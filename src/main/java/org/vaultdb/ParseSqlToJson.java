package org.vaultdb;

import org.apache.calcite.plan.RelOptUtil;
import org.apache.calcite.rel.RelRoot;
import org.apache.calcite.sql.SqlDialect;
import org.apache.calcite.sql.SqlExplainFormat;
import org.apache.calcite.sql.SqlExplainLevel;
import org.vaultdb.codegen.JSONGenerator;
import org.vaultdb.config.SystemConfiguration;
import org.vaultdb.config.WorkerConfiguration;
import org.vaultdb.db.schema.SystemCatalog;
import org.vaultdb.executor.config.ConnectionManager;
import org.vaultdb.parser.SqlStatementParser;
import org.vaultdb.plan.SecureRelRoot;
import org.vaultdb.util.FileUtilities;
import org.vaultdb.util.Utilities;

import java.util.logging.Level;
import java.util.logging.Logger;

public class ParseSqlToJson {



    // usage:
    // mvn compile exec:java -Dexec.mainClass="org.vaultdb.ParseSqlToJson" -Dexec.args="<schema name> <src sql file> <dst path for query plan>"
    // Example:
    // mvn compile exec:java -Dexec.mainClass="org.vaultdb.ParseSqlToJson" -Dexec.args="tpch   conf/workload/tpch/queries/01.sql  conf/workload/tpch/plans"
    public static void main(String[] args) throws Exception {


        if(args.length != 3) {
            System.out.println("usage: ParseSqlToJson <source sql file> <dst path for JSON>");
            System.exit(-1);
        }



        String schema = args[0];
        String srcSql = args[1];
        String dstPath = args[2];

        SystemConfiguration config =  SystemConfiguration.getInstance(schema);
        Logger logger = SystemConfiguration.getInstance().getLogger();
        logger.setLevel(Level.INFO);

        Utilities.mkdir(dstPath); // ensure destination exists


        String sql = FileUtilities.readSQL(srcSql);
        logger.info("Received SQL statement: \n" + sql);


        // extract test name as filename
        String segments[] = srcSql.split("/");
        String testName =  segments[segments.length - 1];
        if(testName.contains(".")) {
            testName = testName.substring(0, testName.indexOf('.'));
        }







        SecureRelRoot root = new SecureRelRoot(testName, sql);

        String logicalPlan = RelOptUtil.dumpPlan("", root.getRelRoot().rel, SqlExplainFormat.TEXT, SqlExplainLevel.ALL_ATTRIBUTES);
        logger.info("\n\nLogical plan: " + logicalPlan);

        String plan = JSONGenerator.exportGenericQueryPlan(root.getPlanRoot().getSecureRelNode(), testName, dstPath);
//        logger.info("Parsed plan for " + testName + ":\n" + plan);

        logger.info("Wrote parsed query execution plan to " + dstPath);




        // tear down DB state
        ConnectionManager connections = ConnectionManager.getInstance();
        if(connections != null) {
            connections.closeConnections();
            ConnectionManager.reset();
        }

        SystemConfiguration.getInstance().closeCalciteConnection();
        SystemConfiguration.resetConfiguration();
        SystemCatalog.resetInstance();

    }



}
