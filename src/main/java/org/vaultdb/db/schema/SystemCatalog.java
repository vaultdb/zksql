package org.vaultdb.db.schema;

import org.apache.calcite.schema.SchemaPlus;
import org.apache.calcite.util.Pair;
import org.vaultdb.config.SystemConfiguration;
import org.vaultdb.executor.config.ConnectionManager;
import org.vaultdb.config.WorkerConfiguration;
import org.vaultdb.type.SecureRelDataTypeField;
import org.vaultdb.type.SecureRelDataTypeField.SecurityPolicy;
import org.vaultdb.type.SecureRelRecordType;

import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

// decorator for calcite schema
// adds management of security policy for attributes
// also keeps track of how the tables are laid out - i.e., partitioning keys, replicated status
public class SystemCatalog {

    static SystemCatalog instance;
    Map<Pair<String, String>, Long> operatorCardinalityBounds;
    // <table, <attrName, policy>
    Map<String, Map<String, SecurityPolicy>> accessPolicies;
    // <table, isReplicated>  // isReplicated is true if table is replicated over all data owners
    List<String> replicatedTables;
    // <table, attr> - what attribute is this table partitioned on?
    Map<String, String> partitionBy;
    Map<String, Pair<Long, Long>> tableCardinalities; // size of alice and bob's inputs
    SchemaPlus sharedSchema;
    Map<String, SecureRelRecordType> tableSchemas; // ground truth for our tables and corresponding constraints found via TableDefinition
    ConnectionManager connections;


    protected SystemCatalog(WorkerConfiguration config) throws Exception {
        accessPolicies = new HashMap<String, Map<String, SecurityPolicy>>();

        sharedSchema = SystemConfiguration.getInstance().getPdfSchema();

        String publicQuery = "SELECT table_name, column_name FROM information_schema.column_privileges WHERE grantee='public_attribute'";
        String protectedQuery = "SELECT table_name, column_name FROM information_schema.column_privileges WHERE grantee='protected_attribute'";
        String partitionByQuery = "SELECT table_name, column_name FROM information_schema.column_privileges WHERE grantee='partitioned_on'";
        String replicatedTablesQuery = "SELECT table_name FROM information_schema.table_privileges WHERE grantee='replicated'";

        connections = ConnectionManager.getInstance();

        Connection dbConnection = config.getDbConnection();

        initializeSecurityPolicy(publicQuery, dbConnection, SecurityPolicy.Public);
        initializeSecurityPolicy(protectedQuery, dbConnection, SecurityPolicy.Protected);
        initializeReplicatedTables(replicatedTablesQuery, dbConnection);
        initializePartitioningKeys(partitionByQuery, dbConnection);

        initializeTableCardinalities(dbConnection);

        operatorCardinalityBounds = new HashMap<Pair<String, String>, Long>();

        dbConnection.close();

        tableSchemas = new HashMap<String, SecureRelRecordType>();
    }

    public static SystemCatalog getInstance(SystemConfiguration configuration) throws Exception {
        if (instance == null) {
            instance = new SystemCatalog(configuration.getSchemaConfig());
        }
        return instance;
    }

    public static SystemCatalog getInstance() throws Exception {
        if (instance == null) {
            return getInstance(SystemConfiguration.getInstance());
        }
        return instance;
    }

    public static void resetInstance() {
        instance = null;
    }

    private void initializeTableCardinalities(Connection dbConnection) throws SQLException, ClassNotFoundException {
        // list tables
        String tableListQuery = "SELECT DISTINCT table_name FROM information_schema.tables WHERE table_schema='public'";
        List<String> tables = new ArrayList<String>();
        tableCardinalities = new HashMap<String, Pair<Long, Long>>();

        Connection aliceConnection = null;
        Connection bobConnection = null;


        try {
            aliceConnection = connections.getConnection(connections.getAlice());
            bobConnection = connections.getConnection(connections.getBob());
            Statement st = dbConnection.createStatement();

            ResultSet rs = st.executeQuery(tableListQuery);

            while (rs.next()) {
                String table = rs.getString(1);
                tables.add(table);
            }
            rs.close();
            st.close();

        } catch (Exception e) {

            e.printStackTrace();
            System.exit(-1);
        }

        for (String table : tables) {
            try {
                tableCardinalities.put(table, collectTableCardinalities(table, aliceConnection, bobConnection));
            } catch (Exception e) {
                e.printStackTrace();

                System.exit(-1);
            }

        }


    }

    private Pair<Long, Long> collectTableCardinalities(String table, Connection aliceConnection, Connection bobConnection) throws Exception {

        String countQuery = "SELECT COUNT(*) FROM " + table;
        Long aliceCount, bobCount;

        Statement stA = aliceConnection.createStatement();
        ResultSet rsA = stA.executeQuery(countQuery);
        rsA.next();
        aliceCount = rsA.getLong(1);
        rsA.close();
        stA.close();

        Statement stB = bobConnection.createStatement();
        ResultSet rsB = stB.executeQuery(countQuery);
        rsB.next();
        bobCount = rsB.getLong(1);
        rsB.close();
        stB.close();


        Pair<Long, Long> counts = new Pair<Long, Long>(aliceCount, bobCount);

        return counts;
    }

    private void initializePartitioningKeys(String sql, Connection c) throws SQLException {

        Statement st = c.createStatement();
        ResultSet rs = st.executeQuery(sql);
        partitionBy = new HashMap<String, String>();


        while (rs.next()) {
            String table = rs.getString(1);
            String attr = rs.getString(2);
            partitionBy.put(table, attr);
        }

        rs.close();
        st.close();
    }

    // cardinalities for Alice and Bob
    public void putOperatorcardinality(String queryName, String operatorId, long cardinality) {
        Pair operator = new Pair(queryName, operatorId);
        try {
            operatorCardinalityBounds.put(operator, cardinality);
        } catch (Exception e) {
            e.printStackTrace();

            System.exit(-1);
        }

    }

    public Long getOperatorcardinality(String queryName, String operatorId) {
        Pair operator = new Pair(queryName, operatorId);
        Long cardinality = 0L;
        try {
            cardinality = operatorCardinalityBounds.get(operator);
            System.out.println("Getting operator cardinality for " + queryName + " " + operatorId + ": " + cardinality);
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(-1);
        }
        return cardinality;
    }

    public Pair<Long, Long> getTableCardinalities(String tableName) {

        return tableCardinalities.get(tableName);
    }

    // cardinality of union
    public Long getTableCardinality(String tableName) {
        Pair<Long, Long> providerCardinalities = getTableCardinalities(tableName);
        if (isReplicated(tableName))
            return providerCardinalities.left; // no double counting!
        return providerCardinalities.left + providerCardinalities.right;
    }

    private void initializeReplicatedTables(String sql, Connection c) throws SQLException {
        Statement st = c.createStatement();
        ResultSet rs = st.executeQuery(sql);

        replicatedTables = new ArrayList<String>();

        while (rs.next()) {
            String table = rs.getString(1);
            replicatedTables.add(table);
        }

        rs.close();
        st.close();

    }

    void initializeSecurityPolicy(String sql, Connection c, SecurityPolicy policy) throws SQLException {
        Statement st = c.createStatement();
        ResultSet rs = st.executeQuery(sql);

        while (rs.next()) {
            String table = rs.getString(1);
            String attr = rs.getString(2);
            if (accessPolicies.containsKey(table)) {
                accessPolicies.get(table).put(attr, policy);
            } else {
                Map<String, SecurityPolicy> tableEntries = new HashMap<String, SecurityPolicy>();
                tableEntries.put(attr, policy);
                accessPolicies.put(table, tableEntries);
            }
        }

        rs.close();
        st.close();

    }


    public Map<String, SecurityPolicy> tablePolicies(String tableName) {
        return accessPolicies.get(tableName);
    }

    public SecurityPolicy getPolicy(String table, String attr) {
        if (!accessPolicies.containsKey(table) || !accessPolicies.get(table).containsKey(attr))
            return SecurityPolicy.Private;
        return accessPolicies.get(table).get(attr);
    }

    public boolean isReplicated(String tableName) {
        return replicatedTables.contains(tableName);
    }


    public String getPartitionKey(String tableName) {
        return partitionBy.get(tableName);
    }


    public SecureRelRecordType getTableSchema(String tableName) {

        return tableSchemas.get(tableName);
    }

    public SecureRelDataTypeField getAttribute(String tableName, String attrName) {
        SecureRelRecordType relRecordType = tableSchemas.get(tableName);
        if (relRecordType != null) {
            return relRecordType.getAttribute(attrName);
        }

        return null;
    }

}
