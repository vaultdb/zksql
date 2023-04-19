package org.vaultdb.plan;

import org.apache.calcite.rel.RelRoot;
import org.vaultdb.config.SystemConfiguration;
import org.vaultdb.parser.SqlStatementParser;
import org.vaultdb.parser.TreeBuilder;
import org.vaultdb.plan.operator.Operator;

public class SecureRelRoot {
	RelRoot baseRoot;
	Operator treeRoot;
	String name;
	
	public SecureRelRoot(String queryName, String sql) throws Exception {
		SqlStatementParser parser = new SqlStatementParser();


	
		baseRoot = parser.convertSqlToRelMinFields(sql);
		baseRoot = parser.optimize(baseRoot); // optimized to represent in a fine granularity for more smc avoidance

		baseRoot = parser.trimFields(baseRoot); // use minimal set of fields to avoid triggering unnecessary SMC

		baseRoot = parser.mergeProjects(baseRoot); // drop any unnecessary steps in the plan

		name = (queryName == null) ? SystemConfiguration.getInstance().getQueryName() : queryName.replaceAll("-", "_");
		treeRoot = TreeBuilder.create(name, baseRoot);
	}
	
	
	public RelRoot getRelRoot() {
		return baseRoot;
	}
	
	public Operator getPlanRoot() {
		return treeRoot;
	}
	
	public String toString() {
		return appendOperator(treeRoot, new String(), "");
	}
	
	public String getName() {
		return name;
	}


	String appendOperator(Operator op, String src, String linePrefix) {
		src += linePrefix + op + "\n";
		linePrefix += "    ";
		for(Operator child : op.getChildren()) {
			src = appendOperator(child, src, linePrefix);
		}
		return src;
	}


}
