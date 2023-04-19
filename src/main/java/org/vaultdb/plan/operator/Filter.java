package org.vaultdb.plan.operator;

import java.util.List;

import org.apache.calcite.rel.logical.LogicalFilter;
import org.apache.calcite.rex.RexNode;
import org.vaultdb.plan.SecureRelNode;
import org.vaultdb.type.SecureRelDataTypeField;
import org.vaultdb.type.SecureRelRecordType;

public class Filter extends Operator {
	

	public Filter(String name, SecureRelNode src, Operator[] childOps) throws Exception {
		super(name, src, childOps);
	}
	

	public List<SecureRelDataTypeField> computesOn() {
		LogicalFilter filter = (LogicalFilter) baseRelNode.getRelNode();
		RexNode predicate = filter.getCondition();
		SecureRelRecordType schema = getSchema();
		
		return AttributeResolver.getAttributes(predicate, schema);
	}





}
