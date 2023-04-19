package org.vaultdb.plan.operator;

import java.util.ArrayList;
import java.util.List;

import org.apache.calcite.rel.RelCollation;
import org.apache.calcite.rel.RelFieldCollation;
import org.apache.calcite.rel.core.Window.Group;
import org.apache.calcite.rel.logical.LogicalWindow;
import org.vaultdb.plan.SecureRelNode;
import org.vaultdb.type.SecureRelDataTypeField;
import org.vaultdb.type.SecureRelRecordType;

public class WindowAggregate extends Operator {

	List<SecureRelDataTypeField> aggFields;
	
	public WindowAggregate(String name, SecureRelNode src, Operator ... children ) throws Exception {
		super(name, src, children);
		aggFields = new ArrayList<>();
	}
	

	
	// computes on PARTITION BY and ORDER BY
	public List<SecureRelDataTypeField> computesOn() {
		List<SecureRelDataTypeField> accessed = new ArrayList<SecureRelDataTypeField>();
		LogicalWindow win = (LogicalWindow) this.getSecureRelNode().getRelNode();
		List<Group> groups = win.groups;
		SecureRelRecordType schema = getSchema();

		for(Group g : groups) {
			accessed =  resolveGroup(g, schema, accessed);
			
		}
		
		return accessed;
	}
	
	public static List<SecureRelDataTypeField> resolveGroup(Group group, SecureRelRecordType schema, List<SecureRelDataTypeField> accessed) {
		RelCollation keys = group.orderKeys;
		List<RelFieldCollation> sortKey = keys.getFieldCollations();
		List<Integer> partitionBy = new ArrayList<Integer>(group.keys.asList());
		
		
		for(Integer idx : partitionBy) {
			if(!accessed.contains(schema.getSecureField(idx)))
				accessed.add(schema.getSecureField(idx));
		}

		for(RelFieldCollation attr : sortKey) {
			int idx = attr.getFieldIndex();
			if(!accessed.contains(schema.getSecureField(idx)))
					accessed.add(schema.getSecureField(idx));
		}
		
		return accessed;
		
	}
	
	
	public List<SecureRelDataTypeField> secureComputeOrder() {
		return this.computesOn();
	}
	
	public List<SecureRelDataTypeField> getAggregateAttributes() {

		List<SecureRelDataTypeField> aggs = new ArrayList<>();
		
		for(SecureRelDataTypeField field : getSchema().getSecureFieldList()) {
			if(field.getBaseField().getName().contains("$")) {
				aggs.add(field);
			}
		}
		return aggs;
	}
	

}
