package org.vaultdb.plan.operator;

import java.util.List;

import org.vaultdb.plan.SecureRelNode;
import org.vaultdb.type.SecureRelDataTypeField;

public class Distinct extends Operator {

	public Distinct(String name, SecureRelNode src, Operator ...children ) throws Exception {
		super(name, src, children);
		
	}
	

	
	public List<SecureRelDataTypeField> computesOn() {
		return getSchema().getSecureFieldList();
	}



	public List<SecureRelDataTypeField> secureComputeOrder() {


		return getSchema().getSecureFieldList(); // all inputs to distinct are in order-by
	}
	
}
