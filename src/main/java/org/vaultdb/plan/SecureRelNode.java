package org.vaultdb.plan;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import org.apache.calcite.plan.RelOptUtil;
import org.apache.calcite.rel.RelNode;
import org.apache.calcite.rel.logical.LogicalValues;
import org.apache.calcite.rel.rel2sql.RelToSqlConverter;
import org.apache.calcite.sql.SqlExplainFormat;
import org.apache.calcite.sql.SqlExplainLevel;
import org.apache.calcite.sql.SqlSelect;
import org.apache.calcite.tools.FrameworkConfig;
import org.apache.calcite.tools.RelBuilder;
import org.vaultdb.config.ExecutionMode;
import org.vaultdb.config.SystemConfiguration;
import org.vaultdb.plan.operator.AttributeResolver;
import org.vaultdb.plan.operator.Operator;
import org.vaultdb.type.SecureRelRecordType;

// decorator for calcite's RelNode
// for attribute-level security policy inference 
// and cardinality resolution
public class SecureRelNode implements Serializable {
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	transient RelNode baseNode;
	Operator physicalNode; 
	List<SecureRelNode> children;
	
	SecureRelNode parent;
	
	
	SecureRelRecordType schema; // out schema of this node
	
	
	public SecureRelNode(RelNode base, SecureRelNode ... childNodes) throws Exception {
		baseNode = base;
		children = new ArrayList<SecureRelNode>();
		
		if(childNodes != null)
			for(SecureRelNode child : childNodes) 
				addChild(child);
		
		schema = AttributeResolver.resolveNode(this);
	}
	
	
	public void addChild(SecureRelNode op) {
		children.add(op);
		op.setParent(this);
	}
	
	public void addChildren(List<SecureRelNode> ops) {
		children.addAll(ops);	
		for(SecureRelNode op : ops)
			op.setParent(this);
	}
	
	public void setParent(SecureRelNode op) {
		parent = op;
	}


	public RelNode getRelNode() {
		return baseNode;
	}


	public SecureRelNode getChild(int i) {
		return children.get(i);
	}

	public List<SecureRelNode> getChildren() {
		return children;
	}

	public SecureRelRecordType getSchema() {
		return schema;
	}

	public void setPhysicalNode(Operator op) {
		physicalNode = op;
	}
	
	
	public Operator getPhysicalNode() {
		return physicalNode;
	}



}
