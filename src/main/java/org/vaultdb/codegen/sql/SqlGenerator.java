package org.vaultdb.codegen.sql;


import org.apache.calcite.rel.RelNode;
import org.apache.calcite.rel.RelRoot;
import org.apache.calcite.rel.rel2sql.RelToSqlConverter;
import org.apache.calcite.rex.RexNode;
import org.apache.calcite.sql.*;
import org.apache.calcite.sql.fun.SqlStdOperatorTable;
import org.apache.calcite.sql.parser.SqlParserPos;
import org.apache.calcite.sql.validate.SqlValidatorUtil;
import org.apache.calcite.sql2rel.SqlNodeToRexConverter;
import org.vaultdb.config.SystemConfiguration;
import org.vaultdb.plan.SecureRelNode;
import org.vaultdb.plan.operator.Operator;

import static org.apache.calcite.sql.validate.SqlValidatorUtil.*;


public class SqlGenerator {


    public static String getSql(RelRoot root, SqlDialect dialect) {
        return getSql(root.rel, dialect);
    }

    public static String getSql(RelNode rel, SqlDialect dialect) {
        RelToSqlConverter converter = new ExtendedRelToSqlConverter(SystemConfiguration.DIALECT);
        return getStringFromNode(rel, converter, dialect);
    }

    public static String getSourceSql(Operator node) {
        SecureRelNode secNode = node.getSecureRelNode();
        RelNode rel = secNode.getRelNode();
        RelToSqlConverter converter = new SecureRelToSqlConverter(SystemConfiguration.DIALECT, secNode.getPhysicalNode());
        return getStringFromNode(rel, converter, SystemConfiguration.DIALECT);
    }




    public static String getSourceSql(Operator node, SqlDialect dialect) {
        SecureRelNode secNode = node.getSecureRelNode();
        RelNode rel = secNode.getRelNode();
        RelToSqlConverter converter = new SecureRelToSqlConverter(dialect, secNode.getPhysicalNode());



        return getStringFromNode(rel, converter, dialect);
    }


    public static String getStringFromNode(RelNode rel, RelToSqlConverter converter, SqlDialect dialect) {
        SqlSelect sql = converter.visitChild(0, rel).asSelect();



	// move up filter for union/merge input as needed


		// create list for dummyTags regardless of value ( both true and false will be represented)
        SqlNodeList selections = sql.getSelectList();
		SqlNodeList newSelection = sql.getSelectList();

		SqlNode where = sql.getWhere();
		SqlNode dummyTag = null;
		if(where != null) {
			// if selection list is empty, then add wildcard selection
			if (newSelection == null) {
				SqlParserPos pos = sql.getParserPosition();
				SqlNode star = (SqlNode) SqlIdentifier.star(pos); // check to see what is being inserted
				newSelection = new SqlNodeList(pos);
				newSelection.add(star);

			}

			SqlBasicCall notWhere = new SqlBasicCall(SqlStdOperatorTable.NOT, new SqlNode[]{where}, newSelection.getParserPosition());
			dummyTag = addAlias(notWhere, "dummy_tag");
			sql.setWhere(null);
            newSelection.add(dummyTag);
            sql.setSelectList(newSelection);

        }
		//else { // where is null, still want dummy tag
		//		SqlLiteral dummy = SqlLiteral.createBoolean(false, newSelection.getParserPosition());
		//		dummyTag = addAlias(dummy, "dummy_tag");
		//}



		String sqlOut = sql.toSqlString(dialect).getSql();
		sqlOut = sqlOut.replace("\"", ""); // correct quotes
        sqlOut = sqlOut.replace("$", "");


		return sqlOut;

    }


	/*	public static String getDistributedSql(SecureRelRoot root, SqlDialect dialect) throws Exception {

	Operator rootOp = root.getPlanRoot();
	RelNode distributed = rewriteForDistributed(rootOp);
	RelToSqlConverter converter = new ExtendedRelToSqlConverter(dialect);
	return getStringFromNode(distributed, converter, dialect, false);



}

// new tactic: create a new tree of relnodes with the use of copy for everything except the leafs
private static RelNode rewriteForDistributed(Operator op) throws Exception {
	List<RelNode> newChildren = new ArrayList<RelNode>();
	RelNode node = op.getSecureRelNode().getRelNode();

	// if it has children, recurse to them first
	if(!op.getChildren().isEmpty()) {
		for(Operator childOp : op.getChildren()) {
			newChildren.add(rewriteForDistributed(childOp));
		}

		return node.copy(node.getTraitSet(), newChildren);
	}

	// else if it is a leaf
	assert(op instanceof SeqScan);
	FrameworkConfig config = SystemConfiguration.getInstance().getCalciteConfiguration();
	RelBuilder relBuilder = RelBuilder.create(config);
	JdbcTableScan scan =  (JdbcTableScan) node;
	String tableName = scan.getTable().getQualifiedName().get(0);

	String remoteTableName = "remote_" + tableName;
	return relBuilder.scan(tableName).scan(remoteTableName).union(true).build();

}

*/
}
