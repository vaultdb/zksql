package org.vaultdb.plan.operator;

import java.util.ArrayList;
import java.util.List;

import org.apache.calcite.rel.RelNode;
import org.apache.calcite.rel.core.AggregateCall;
import org.apache.calcite.rel.logical.LogicalAggregate;
import org.apache.calcite.util.ImmutableBitSet;
import org.apache.calcite.util.Pair;
import org.vaultdb.config.ExecutionMode;
import org.vaultdb.plan.SecureRelNode;
import org.vaultdb.type.SecureRelDataTypeField;
import org.vaultdb.type.SecureRelDataTypeField.SecurityPolicy;
import org.vaultdb.type.SecureRelRecordType;
import org.vaultdb.util.Utilities;


public class Aggregate extends Operator {

	LogicalAggregate agg;
	
	public Aggregate(String name, SecureRelNode src, Operator ...children) throws Exception {
		super(name, src, children);
		
		agg = (LogicalAggregate) this.getSecureRelNode().getRelNode();


	}


	public List<SecureRelDataTypeField> getGroupByAttributes() {
		List<Integer> groupBy = agg.getGroupSet().asList();
		
		SecureRelRecordType schema = this.getSchema();
		List<SecureRelDataTypeField> result = new ArrayList<>();
		for (Integer i : groupBy)
			result.add(schema.getSecureField(i));

		return result;
	}

	public List<SecureRelDataTypeField> getAggCallAttributes() {
		List<Integer> groupBy = agg.getGroupSet().asList();

		SecureRelRecordType schema = this.getSchema();
		List<SecureRelDataTypeField> result = new ArrayList<SecureRelDataTypeField>();

		for(SecureRelDataTypeField field : schema.getSecureFieldList()) {
			int idx = field.getIndex();
			if(!groupBy.contains(idx)) {
				result.add(field);
			}
		}
		return result;
	}



	public List<SecureRelDataTypeField> computesOn() {
		List<SecureRelDataTypeField> attrs = this.getGroupByAttributes(); // group by only
		
		// don't need the rest of this because evaluation of aggregate is done deterministically
		
		List<SecureRelDataTypeField> allFields = getSchema().getSecureFieldList();
		
		List<AggregateCall> aggregates = agg.getAggCallList();
		for(AggregateCall aggCall : aggregates) {
			List<Integer> args  =  aggCall.getArgList();
			// compute over entire row
			if(args.isEmpty()) return baseRelNode.getSchema().getSecureFieldList();

			for(Integer i : args) {
				SecureRelDataTypeField lookup = allFields.get(i);
				
				if(!attrs.contains(lookup)) attrs.add(lookup);
			}
		}
		
		for(ImmutableBitSet bits : agg.groupSets.asList()) {
			for(Integer i : bits.asList()) {
				SecureRelDataTypeField lookup = allFields.get(i);
				
				if(!attrs.contains(lookup)) attrs.add(lookup);
			}
		}
		
		return attrs;
	}


	// is this a scalar aggregate with no distributed children?
	public boolean splitAggregate() {

		if(executionMode.distributed  && executionMode.oblivious && (agg.getGroupCount() == 0)
				 && childrenLocal()) { 
			return true;
		}
		return false;
		
	}
	
	
	@Override
	public void inferExecutionMode() throws Exception {
	
		Operator child = children.get(0);
		child.inferExecutionMode();
		
		executionMode = new ExecutionMode(child.executionMode);

		// we already know inputs are partitioned-alike
		// see if we are partitioned on group-by or a subset thereof
		boolean partitioned = false;
		// if child runs locally
		if(!child.executionMode.distributed) {
			List<SecureRelDataTypeField> attrs = this.getGroupByAttributes();

			
		}
		
		SecurityPolicy maxExecutionMode = super.maxAccessLevel();
		if(maxExecutionMode != SecurityPolicy.Public)
			executionMode.oblivious = true;
		
		if(!partitioned)
			executionMode.distributed = true;
	}
		
	
	private boolean childrenLocal() {
		
		for(Operator op : children) {
			if(op.getExecutionMode().distributed) {
				return false;
			}
			
		}
		return true;
		
	}


	

	protected String getAggregateString() {
		RelNode relNode = this.baseRelNode.getRelNode();
		LogicalAggregate logicalAggregate = ((LogicalAggregate) relNode);

		List<String> statements = new ArrayList<>();
		// groups
		List<ImmutableBitSet> groups = logicalAggregate.getGroupSets();
		if (groups.size() > 0) {
			String groupString = "Group-by: " + groups.toString();
			statements.add(groupString);
		}
		// aggregate
		List<Pair<AggregateCall, String>> aggCalls = logicalAggregate.getNamedAggCalls();
		List<String> aggregateStrings = new ArrayList<>();
		for (Pair<AggregateCall, String> call : aggCalls) {
			aggregateStrings.add(call.left.toString() + " as " + call.right);
		}

		if (!aggregateStrings.isEmpty()) {
			String aggregateString = "Aggs: " + String.join(" & ", aggregateStrings);
			statements.add(aggregateString);
		}

		if (statements.isEmpty()) {
			return null;
		}
		return String.join(" & ", statements);
	}


	public List<SecureRelDataTypeField> secureComputeOrder() {
		List<Integer> groupBy = agg.getGroupSet().asList();
		List<SecureRelDataTypeField> orderBy = new ArrayList<SecureRelDataTypeField>();
		SecureRelRecordType inSchema = this.getInSchema();

		for (Integer i : groupBy)
			orderBy.add(inSchema.getSecureField(i));

		return orderBy;
	}
}
