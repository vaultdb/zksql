package org.vaultdb.plan.operator;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.SortedSet;

import org.apache.calcite.adapter.jdbc.JdbcTableScan;
import org.apache.calcite.plan.RelOptUtil;
import org.apache.calcite.rel.RelFieldCollation;
import org.apache.calcite.rel.RelNode;
import org.apache.calcite.rel.core.AggregateCall;
import org.apache.calcite.rel.core.Window.Group;
import org.apache.calcite.rel.logical.*;

import org.apache.calcite.rel.type.RelDataTypeField;
import org.apache.calcite.rel.type.RelRecordType;
import org.apache.calcite.rex.RexNode;
import org.apache.calcite.util.Pair;
import org.vaultdb.config.SystemConfiguration;
import org.vaultdb.db.schema.SystemCatalog;
import org.vaultdb.plan.SecureRelNode;
import org.vaultdb.type.SecureRelDataTypeField;
import org.vaultdb.type.SecureRelRecordType;
import org.vaultdb.type.SecureRelDataTypeField.SecurityPolicy;
import org.vaultdb.util.Utilities;

public class AttributeResolver {

	public static SecureRelRecordType resolveNode(SecureRelNode aNode) throws Exception {
		RelNode baseNode = aNode.getRelNode();
		
		if(baseNode instanceof LogicalJoin)
			return resolveJoin(aNode);

		if(baseNode instanceof JdbcTableScan)
			return resolveScan(aNode);
		
		if(baseNode instanceof LogicalProject)
			return resolveProjection(aNode);
		
		if(baseNode instanceof LogicalFilter || baseNode instanceof LogicalSort
				|| (baseNode instanceof LogicalAggregate && ((LogicalAggregate) baseNode).getAggCallList().isEmpty())) // distinct
				return copySchema(aNode);

		if(baseNode instanceof LogicalAggregate)
			return resolveAggregate(aNode);
		
		
		if(baseNode instanceof LogicalWindow) 
			return resolveWinAggregate(aNode);

		if(baseNode instanceof LogicalCorrelate)
			return resolveLogicalCorrelate(aNode);
		
		// unknown type
		return null;
				
		
	}

	private static SecureRelRecordType resolveLogicalCorrelate(SecureRelNode aCorr) throws Exception {

		List<SecureRelDataTypeField> secureFields = new ArrayList<>();
		LogicalCorrelate corr = (LogicalCorrelate) aCorr.getRelNode();
		SecureRelNode lhsChild = aCorr.getChild(0);
		SecureRelNode rhsChild = aCorr.getChild(1);


		SecureRelRecordType lhs = lhsChild.getSchema();
		SecureRelRecordType rhs = rhsChild.getSchema();

		RelRecordType baseType = (RelRecordType) corr.getRowType();
		Iterator<RelDataTypeField> baseItr = baseType.getFieldList().iterator();

		for(SecureRelDataTypeField field : lhs.getSecureFieldList()) {
			RelDataTypeField dstField = baseItr.next();
			secureFields.add(new SecureRelDataTypeField(dstField, field));
		}

		for(SecureRelDataTypeField field : rhs.getSecureFieldList()) {
			RelDataTypeField dstField = baseItr.next();
			secureFields.add(new SecureRelDataTypeField(dstField, field));
		}


		return  new SecureRelRecordType(baseType, secureFields);
	}



	private static SecureRelRecordType resolveWinAggregate(SecureRelNode aNode) throws Exception {
		SecureRelRecordType inSchema = aNode.getChild(0).getSchema();
		
		LogicalWindow win = (LogicalWindow) aNode.getRelNode();

		RelRecordType outRow = (RelRecordType) win.getRowType();
		
		List<SecureRelDataTypeField> secureFields = new ArrayList<>();
		
		for(RelDataTypeField field : outRow.getFieldList()) {
			String name = field.getName();
			SecurityPolicy policy;
			String storedTable;
			
			if(name.contains("$") && name.startsWith("w")) { // w<winNo>$o<orderByNo> for window aggs
				String[] tokens = name.split("\\$");
				
				String winNoStr = tokens[0].substring(1);
				
				int winNo = Integer.parseInt(winNoStr);
				Group aggregate = win.groups.get(winNo);
				List<Integer> attrsUsed = new ArrayList<Integer>(aggregate.keys.asList());
				List<RelFieldCollation> orderBy = aggregate.orderKeys.getFieldCollations();
				for(RelFieldCollation ref : orderBy) {
					int idx = ref.getFieldIndex();
					if(!attrsUsed.contains(idx)) {
						attrsUsed.add(idx);
					}				
				}
				policy = getFieldPolicy(inSchema, new HashSet<Integer>(attrsUsed));
				storedTable = getStoredTable(inSchema, new HashSet<Integer>(attrsUsed));
				secureFields.add(new SecureRelDataTypeField(field, policy, storedTable));
					
			}
			else { // 1:1 mapping
				List<String> fieldNames = inSchema.getFieldNames();
				int srcIdx = fieldNames.indexOf(name);
				SecureRelDataTypeField srcField = inSchema.getSecureField(srcIdx);
				secureFields.add(new SecureRelDataTypeField(field, srcField));

			}
		}

		SecureRelRecordType schema = new SecureRelRecordType(outRow, secureFields);
		return schema;
	}

	private static SecureRelRecordType resolveAggregate(SecureRelNode aNode) throws Exception {
		SecureRelRecordType inSchema = aNode.getChild(0).getSchema();
		LogicalAggregate agg = (LogicalAggregate) aNode.getRelNode();
		RelRecordType record = (RelRecordType) aNode.getRelNode().getRowType();
		List<SecureRelDataTypeField> secFields = new ArrayList<SecureRelDataTypeField>();
		
		Map<String, AggregateCall> aggMap = new HashMap<String, AggregateCall>();
		Map<String,SecureRelDataTypeField> scalarMap = new HashMap<String, SecureRelDataTypeField>();
		

		Iterator<Pair<AggregateCall, String>> aggItr = agg.getNamedAggCalls().iterator();
		while(aggItr.hasNext()) {
			Pair<AggregateCall, String> entry = aggItr.next();
			aggMap.put(entry.right, entry.left);
		}
		
		for(SecureRelDataTypeField inField : inSchema.getSecureFieldList()) {
			String name = inField.getName();
			scalarMap.put(name, inField);
		}

		for(RelDataTypeField field : record.getFieldList()) {
			String name = field.getName();
			
			if(scalarMap.containsKey(name)) {
				SecureRelDataTypeField prev = scalarMap.get(name);
				SecureRelDataTypeField secField = new SecureRelDataTypeField(field, prev);
				secFields.add(secField);
			}

			// second for loop for the writeDest using logic below

			else if(aggMap.containsKey(name)) {
				AggregateCall call = aggMap.get(name);
				SecurityPolicy policy = AttributeResolver.getAggPolicy(call, inSchema);
				String storedTable = AttributeResolver.getStoredTable(call, inSchema);
				SecureRelDataTypeField secField = new SecureRelDataTypeField(field, policy, storedTable);
				
				secFields.add(secField);
			}
		}
			
		SecureRelRecordType schema =  new SecureRelRecordType(record, secFields);
		schema.setReplicated(inSchema.isReplicated()); // propagate replicated status of child
		return schema;
		
			
	}

	private static SecureRelRecordType resolveProjection(SecureRelNode aProjection) throws Exception {

		LogicalProject projection = (LogicalProject) aProjection.getRelNode();
		SecureRelRecordType srcSchema = aProjection.getChild(0).getSchema();
		List<RexNode> rexNodes = projection.getChildExps();
		RelRecordType outRow = (RelRecordType) projection.getRowType();
		
		Iterator<RelDataTypeField> baseItr = outRow.getFieldList().iterator();

		List<SecureRelDataTypeField> secureFields = new ArrayList<SecureRelDataTypeField>();
		
		for(RexNode r : rexNodes) {
			SecureRelDataTypeField secField = resolveField(r, baseItr.next(), srcSchema);
			secureFields.add(secField);
		}
		
		SecureRelRecordType schema =  new SecureRelRecordType((RelRecordType) projection.getRowType(), secureFields);
		schema.setReplicated(srcSchema.isReplicated()); // propagate replicated status of child
		return schema;

	}
	
	public static SecureRelDataTypeField resolveField(RexNode rex, RelDataTypeField baseField, SecureRelRecordType inSchema) throws Exception {
		final RelOptUtil.InputReferencedVisitor shuttle = new RelOptUtil.InputReferencedVisitor();
		rex.accept(shuttle);
		SortedSet<Integer> ordinalsAccessed = shuttle.inputPosReferenced;
		if(ordinalsAccessed.size() ==  1) { // can preserve stored source info
			return new SecureRelDataTypeField(baseField, inSchema.getSecureField(ordinalsAccessed.first()));
		}
		else {
			SecurityPolicy policy  = getFieldPolicy(inSchema, ordinalsAccessed);
			String storedTable = getStoredTable(inSchema, ordinalsAccessed);
		
			return new SecureRelDataTypeField(baseField, policy, storedTable);
		}
	}


	// simple copy of permissions, accept any new aliases
	public static SecureRelRecordType copySchema(SecureRelNode aNode) throws Exception {
		SecureRelRecordType srcSchema = aNode.getChild(0).getSchema();
		RelRecordType dstRecord = (RelRecordType) aNode.getRelNode().getRowType();
		
		List<SecureRelDataTypeField> dstFields = new ArrayList<SecureRelDataTypeField>();
		
		assert(srcSchema.getFieldCount() == dstRecord.getFieldCount());
		
		Iterator<RelDataTypeField> baseItr = dstRecord.getFieldList().iterator();
		Iterator<SecureRelDataTypeField> srcItr = srcSchema.getSecureFieldList().iterator();
		
		while(baseItr.hasNext() && srcItr.hasNext()) {
			SecureRelDataTypeField secureField = new SecureRelDataTypeField(baseItr.next(), srcItr.next());
			dstFields.add(secureField);
		}

		SecureRelRecordType schema =  new SecureRelRecordType(dstRecord, dstFields);
		schema.setReplicated(srcSchema.isReplicated()); // propagate replicated status of child

		return schema;
	}

	public static SecurityPolicy getFieldPolicy(SecureRelRecordType srcSchema, Set<Integer> ordinalsAccessed) {
		SecurityPolicy maxPolicy = SecurityPolicy.Public;
		
		for(Integer i : ordinalsAccessed) {
			SecureRelDataTypeField field = srcSchema.getSecureField(i);
			SecurityPolicy attrPolicy = field.getSecurityPolicy();
			if(attrPolicy.compareTo(maxPolicy) > 0) {
				maxPolicy = attrPolicy;
			}
		}
		return maxPolicy;
	}
	
	public static SecurityPolicy getAggPolicy(AggregateCall agg, SecureRelRecordType inSchema) {
		List<Integer> ordinalsAccessed = agg.getArgList();
		if(ordinalsAccessed.isEmpty()) {
			// get max over all attributes
			SecurityPolicy maxPolicy = SecurityPolicy.Public;
			for(SecureRelDataTypeField field : inSchema.getSecureFieldList()) {
				SecurityPolicy fieldPolicy = field.getSecurityPolicy();
				if(fieldPolicy.compareTo(maxPolicy) > 0) 
					maxPolicy = fieldPolicy;
			}
		
			return maxPolicy;
			
		}
		else 
			return getFieldPolicy(inSchema, new HashSet<Integer>(agg.getArgList()));
		
	}
	
	
	public static String getStoredTable(AggregateCall agg, SecureRelRecordType inSchema) {
		List<Integer> ordinalsAccessed = agg.getArgList();
		String storedTable = null;
		
		if(ordinalsAccessed.isEmpty()) {
			// see if all attrs have same src table
			
			for(SecureRelDataTypeField field : inSchema.getSecureFieldList()) {
				if(storedTable == null) {
					storedTable = field.getStoredTable();
				}
				else if(!field.getStoredTable().equals(storedTable)) { // no single source
					return null;
				}
			}
		
			return storedTable;
			
		}
		else 
			return getStoredTable(inSchema, new HashSet<Integer>(agg.getArgList()));
		
	}
	
	
	public static String getStoredTable(SecureRelRecordType srcSchema, Set<Integer> ordinalsAccessed) {
		String storedTable = null;
		
		for(Integer i : ordinalsAccessed) {
			SecureRelDataTypeField field = srcSchema.getSecureField(i);
			if(storedTable == null) {
				storedTable = field.getStoredTable();
			}
			else if(!storedTable.equals(field.getStoredTable())) {
				return null;
			}
		}
		return storedTable;
	}


	private static SecureRelRecordType resolveJoin(SecureRelNode aJoin) throws Exception {
		
		List<SecureRelDataTypeField> secureFields = new ArrayList<SecureRelDataTypeField>();
		LogicalJoin join = (LogicalJoin) aJoin.getRelNode();
		SecureRelNode lhsChild = aJoin.getChild(0);
		SecureRelNode rhsChild = aJoin.getChild(1);
		
		
		SecureRelRecordType lhs = lhsChild.getSchema();
		SecureRelRecordType rhs = rhsChild.getSchema();
		
		RelRecordType baseType = (RelRecordType) join.getRowType();
		Iterator<RelDataTypeField> baseItr = baseType.getFieldList().iterator();

		for(SecureRelDataTypeField field : lhs.getSecureFieldList()) {
			RelDataTypeField dstField = baseItr.next();
			SecureRelDataTypeField secureRelDataTypeField = new SecureRelDataTypeField(dstField, field);
			secureFields.add(secureRelDataTypeField);
		}
		
		for(SecureRelDataTypeField field : rhs.getSecureFieldList()) {
			RelDataTypeField dstField = baseItr.next();
			SecureRelDataTypeField secureRelDataTypeField = new SecureRelDataTypeField(dstField, field);
			secureFields.add(secureRelDataTypeField);
		}

		SecureRelRecordType schema =  new SecureRelRecordType(baseType, secureFields);
		
		if(lhs.isReplicated() && rhs.isReplicated())
			schema.setReplicated(true);


		return schema;
	}

	/**
	 * Adds Column Definitions to TableDefinition in TableConstraints inside SystemCatalog
	 * Also adds constraints to SecureRelRecordType
	 * @param aScan
	 * @return
	 * @throws Exception
	 */
	public static SecureRelRecordType resolveScan(SecureRelNode aScan) throws Exception {
		List<SecureRelDataTypeField > secureFields = new ArrayList<SecureRelDataTypeField>();
		
		JdbcTableScan rel = (JdbcTableScan) aScan.getRelNode();
		RelRecordType record = (RelRecordType) rel.getRowType();
		String table = rel.getTable().getQualifiedName().get(0);
		SystemCatalog catalog = SystemCatalog.getInstance();
		SystemConfiguration configuration = SystemConfiguration.getInstance();

		// the parser pushes all projections out of the table scan, but let's construct this as if that's not a given.

		for(RelDataTypeField relField : record.getFieldList()) {
			String attr = relField.getName();
			SecureRelDataTypeField schemaField = Utilities.lookUpAttribute(table, attr, catalog, configuration);

			// copy, but don't collect statistics for ColumnDefinition - necessary to prevent dependency loop with ColumnDefinition constructor on public attrs
			SecureRelDataTypeField secField =  new SecureRelDataTypeField(relField, schemaField); // deep copy
			secureFields.add(secField);
		}

		SecureRelRecordType schema = new SecureRelRecordType(record, secureFields);
		schema.setReplicated(catalog.isReplicated(table));

		return schema;
	}





	public static List<SecureRelDataTypeField> getAttributes(RexNode aNode, SecureRelRecordType schema) {
		List<SecureRelDataTypeField> accessed = new ArrayList<SecureRelDataTypeField>();

		final RelOptUtil.InputReferencedVisitor shuttle = new RelOptUtil.InputReferencedVisitor();
		aNode.accept(shuttle);
		SortedSet<Integer> ordinalsAccessed = shuttle.inputPosReferenced;

		for(Integer i : ordinalsAccessed) {
			accessed.add(schema.getSecureField(i));
		}

		return accessed;

	}
}
