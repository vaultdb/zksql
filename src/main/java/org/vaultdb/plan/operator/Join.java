package org.vaultdb.plan.operator;

import org.apache.calcite.rel.logical.LogicalJoin;
import org.apache.calcite.rex.*;
import org.vaultdb.plan.SecureRelNode;
import org.vaultdb.type.SecureRelDataTypeField;
import org.vaultdb.type.SecureRelDataTypeField.SecurityPolicy;
import org.vaultdb.type.SecureRelRecordType;

import java.util.List;

public class Join extends Operator {

	
	public Join(String name, SecureRelNode src, Operator ...children ) throws Exception {
		super(name, src, children);
		
	}
	
	

	@Override
	public void inferExecutionMode()  throws Exception {
		SecurityPolicy maxAccess = maxAccessLevel();

		super.inferExecutionMode();
		
		if(!children.get(0).executionMode.oblivious  && 
				!children.get(1).executionMode.oblivious && maxAccess == SecurityPolicy.Public) { // results are replicated
				executionMode.oblivious = false;
		}
	}
	



	public SecureRelRecordType getInSchema() {
		
		return  getSchema();
	}
	
	public List<SecureRelDataTypeField> computesOn() {

		LogicalJoin join = (LogicalJoin) this.getSecureRelNode().getRelNode();
		RexNode joinOn = join.getCondition();
		
		return AttributeResolver.getAttributes(joinOn, getSchema());
		
	}

	@Override
	public String toString() {
		
		LogicalJoin join = (LogicalJoin) this.getSecureRelNode().getRelNode();
		RexNode predicate = join.getCondition();
		return baseRelNode.getRelNode().getRelTypeName() + "-" + executionMode + ", schema:" + getSchema() + ", Predicate: " + predicate;
	}

	

}
