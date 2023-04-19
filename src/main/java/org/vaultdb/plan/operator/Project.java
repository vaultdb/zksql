package org.vaultdb.plan.operator;

import org.vaultdb.config.ExecutionMode;
import org.vaultdb.plan.SecureRelNode;

public class Project extends Operator {
	
	public Project(String name, SecureRelNode src, Operator[] childOps) throws Exception {
		super(name, src, childOps);
	}
	
		@Override
	public void inferExecutionMode() throws Exception {
		Operator child = children.get(0);
		child.inferExecutionMode();
		
		// always takes on the execution mode of its child since it runs deterministically
		executionMode = new ExecutionMode(child.executionMode);
	}
	

}
