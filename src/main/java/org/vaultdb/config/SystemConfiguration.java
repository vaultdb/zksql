package org.vaultdb.config;

import java.io.File;
import java.io.IOException;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.Map.Entry;
import java.util.logging.FileHandler;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.logging.SimpleFormatter;
import java.util.logging.StreamHandler;

import org.apache.calcite.adapter.jdbc.JdbcSchema;
import org.apache.calcite.config.Lex;
import org.apache.calcite.jdbc.CalciteConnection;
import org.apache.calcite.schema.SchemaPlus;
import org.apache.calcite.schema.Table;
import org.apache.calcite.sql.SqlDialect;
import org.apache.calcite.sql.parser.SqlParser;
import org.apache.calcite.sql.parser.SqlParser.Config;
import org.apache.calcite.tools.FrameworkConfig;
import org.apache.calcite.tools.Frameworks;
import org.apache.commons.dbcp2.BasicDataSource;
import org.apache.commons.lang3.StringUtils;
import org.vaultdb.util.FileUtilities;
import org.vaultdb.util.Utilities;

public class SystemConfiguration {
	
	public static final SqlDialect DIALECT = SqlDialect.DatabaseProduct.POSTGRESQL.getDialect();

	public enum Party { ALICE, BOB, COORDINATOR};
	
	static SystemConfiguration instance = null;
	private static final Logger logger =
	        Logger.getLogger(SystemConfiguration.class.getName());
	
	
	private Map<String, String> config;
	// package name --> SQL statement
	

	String configFile = null;

	// calcite parameters
	SchemaPlus pdnSchema;
	CalciteConnection calciteConnection;
	Connection baseConnection;
	FrameworkConfig calciteConfig;
	
	int operatorCounter = -1;
	int queryCounter = -1;
	int portCounter = 54320;

	public Map<String, String> TABLE_DEFINITIONS = null;
	
	
	protected SystemConfiguration() throws Exception {
		config = new HashMap<String, String>();

		String configStr = System.getProperty("vaultdb.setup.str"); // remote case, serialize config and parse this string
		if(configStr != null) {

			List<String> parameters = Arrays.asList(StringUtils.split(configStr, '\n'));
			parseConfiguration(parameters);
			initializeLogger();

			
			return;
		}
		
		// local case, read in a text file
		configFile = System.getProperty("vaultdb.setup");
		
		if(configFile == null) 
			configFile = Utilities.getVaultDBRoot() + "/conf/setup.global";
            
		
		
		File f = new File(configFile); // may not always exist in remote invocations
		if(f.exists()) {
			List<String> parameters = FileUtilities.readFile(configFile);
			parseConfiguration(parameters);
			
		}	

		String deploymentConfigFile = new String(Utilities.getVaultDBRoot() + "/conf/setup.");
		String location = (System.getProperty("vaultdb.location") != null) ? System.getProperty("vaultdb.location") : config.get("location"); // if not given at setup time, use default
		
		// if distributed nodes not set up yet, switch to local mode.  E.g.,
		// distributed-eval-enabled=false
		if(config.get("distributed-eval-enabled") != null && config.get("distributed-eval-enabled").equals("false")) {
			location = "local";
		}
		
		String schemaName = (System.getProperty("vaultdb.schema.name") != null) ? System.getProperty("vaultdb.schema.name") : config.get("schema-name"); // if not given at setup time, use default
		
		
		deploymentConfigFile +=  location + "-" + schemaName;

		File d = new File(deploymentConfigFile); // may not always exist in remote invocations
		if(d.exists()) {
			List<String> parameters = FileUtilities.readFile(deploymentConfigFile);
			parseConfiguration(parameters);		
		}
		else {
			System.out.println("Warning! No deployment file: " + deploymentConfigFile);
		}
		
		
		
		
		initializeLogger();
		initializeCalcite();
		
		// have to do this in system properties because remote instances may not have SystemConfiguration initialized
		setProperty("node-type", "local");

		
	}

	
	protected SystemConfiguration(String schemaName) throws Exception {
		config = new HashMap<String, String>();

		String configStr = System.getProperty("vaultdb.setup.str"); // remote case, serialize config and parse this string
		if(configStr != null) {

			List<String> parameters = Arrays.asList(StringUtils.split(configStr, '\n'));
			parseConfiguration(parameters);
			initializeLogger();

			
			return;
		}
		
		// local case, read in a text file
		configFile = System.getProperty("vaultdb.setup");
		
		if(configFile == null) 
			configFile = Utilities.getVaultDBRoot() + "/conf/setup.global";
		
		
		File f = new File(configFile); // may not always exist in remote invocations
		if(f.exists()) {
			List<String> parameters = FileUtilities.readFile(configFile);
			parseConfiguration(parameters);
			
		}	

		String deploymentConfigFile = new String(Utilities.getVaultDBRoot() + "/conf/setup.");
		String location = (System.getProperty("vaultdb.location") != null) ? System.getProperty("vaultdb.location") : config.get("location"); // if not given at setup time, use default
		
		// if distributed nodes not set up yet, switch to local mode.  E.g.,
		// distributed-eval-enabled=false
		if(config.get("distributed-eval-enabled") != null && config.get("distributed-eval-enabled").equals("false")) {
			location = "local";
		}
		
		if(schemaName == null) 
			schemaName = config.get("schema-name"); // if not given at setup time, use default
		else 
			config.put("schema-name", schemaName);

		deploymentConfigFile +=  location + "-" + schemaName;

		File d = new File(deploymentConfigFile); // may not always exist in remote invocations
		if(d.exists()) {
			List<String> parameters = FileUtilities.readFile(deploymentConfigFile);
			parseConfiguration(parameters);		
		}
		else {
			System.out.println("Warning! No deployment file: " + deploymentConfigFile);
		}

		// reading in table schema definitions from schema definition file
		if (config.containsKey("schema-definition")){
			String schemaDefinition = config.get("schema-definition");
	//		SchemaDefinitionParser schemaDefinitionParser = new SchemaDefinitionParser(schemaDefinition);
	//		TABLE_DEFINITIONS = schemaDefinitionParser.getSchemas();
		}

		initializeLogger();
		initializeCalcite();
		
		// have to do this in system properties because remote instances may not have SystemConfiguration initialized
		setProperty("node-type", "local");
		logger.info("configFile: " + configFile);
		logger.info("Deploying with " + schemaName + " and file " + deploymentConfigFile);
	}

	private void initializeLogger() throws SecurityException, IOException  {
		String filename = config.get("log-file");
		if(filename == null)
			filename = "vaultdb.log";

		String logFile = Utilities.getVaultDBRoot() + "/" + filename;

		
		String logLevel = config.get("log-level");
		if(logLevel != null)
			logLevel = logLevel.toLowerCase();

		
		logger.setUseParentHandlers(false);
		
		if(logLevel != null && (logLevel.equals("debug") || logLevel.equals("fine"))) {
				logger.setLevel(Level.FINE);
		}
		else if(logLevel != null && logLevel.equals("off")) {
				logger.setLevel(Level.OFF);
		}
		else {
				logger.setLevel(Level.INFO);
		}

		SimpleFormatter fmt = new SimpleFormatter();
		 StreamHandler sh = new StreamHandler(System.out, fmt);
		 logger.addHandler(sh);
		 
		try {
			FileHandler handler = new FileHandler(logFile);
			
		SimpleFormatter formatter = new SimpleFormatter();
		handler.setFormatter(formatter);

		logger.addHandler(handler);
		} catch (Exception e) { // fall back to home dir
			String homeDirectory = System.getProperty("user.home");
			logFile =  homeDirectory + "/vaultdb.log";
			FileHandler handler = new FileHandler(logFile);
			SimpleFormatter formatter = new SimpleFormatter();
			handler.setFormatter(formatter);

			logger.addHandler(handler);

			// show in console to verify
			logger.setUseParentHandlers(true);
			
		}
		
		
	}

	void initializeCalcite() throws Exception {
		WorkerConfiguration honestBroker = getSchemaConfig();
		String host = honestBroker.hostname;
		int port = honestBroker.dbPort;
		String db = honestBroker.dbName;
		String user = honestBroker.user;
		String pass = honestBroker.password;
		
		String url = "jdbc:postgresql://" + host + ":" + port + "/" + db;

		Properties props = new Properties();
		props.setProperty("caseSensitive", "false");
		
		 baseConnection = DriverManager.getConnection("jdbc:calcite:", props);
	     calciteConnection = baseConnection.unwrap(CalciteConnection.class);
	        
	        Class.forName("org.postgresql.Driver");
	        BasicDataSource dataSource = new BasicDataSource();
	        dataSource.setUrl(url);
	        dataSource.setUsername(user);
	        dataSource.setPassword(pass);
	        
	        
	        JdbcSchema schema = JdbcSchema.create(calciteConnection.getRootSchema(), "name", dataSource,
	        	    null, null);
	        
	        for(String tableName : schema.getTableNames()) {
	        	Table table = schema.getTable(tableName);
	        	
	        	calciteConnection.getRootSchema().add(tableName, table);
	        }
	        
	        
	        
	        
	    	pdnSchema = calciteConnection.getRootSchema();
	 		
	 		Config parserConf = SqlParser.configBuilder().setCaseSensitive(false).setLex(Lex.MYSQL).build();
			calciteConfig = Frameworks.newConfigBuilder().defaultSchema(pdnSchema).parserConfig(parserConf).build();

	}
	
	
	public static SystemConfiguration getInstance() throws Exception {
		if(instance == null) {
			instance = new SystemConfiguration();

		}
		return instance;
	}

	
	
	public static SystemConfiguration getInstance(String schemaName) throws Exception {
		if(instance == null) {
			instance = new SystemConfiguration(schemaName);
		}
		return instance;
	}
	

		
	public static  void resetConfiguration() {
		instance = null;
	}

	
	public  Logger getLogger() {
		return logger;
	}
	
	public void setProperty(String key, String value) {
		config.put(key, value);
	}
	
	

	public String getConfigFile() {
		return configFile;
	}
	
	private void parseConfiguration(List<String> parameters) {
		String prefix = null;

		for(String p : parameters) {
			if(!p.startsWith("#")) { // skip comments
				if(p.startsWith("[") && p.endsWith("]")) {
					prefix = p.substring(1, p.length() - 1);
				}
				else if(p.contains("=")){
					String[] tokens = p.split("=");
					String key = tokens[0];
					String value = (tokens.length > 1) ? tokens[1] : null;
					if(prefix != null) {
						key = prefix + "-" + key;
					}

					config.put(key, value);

				}
				
			}
		}
	
	}

	
	public String getProperty(String p) {
		if(config.containsKey(p)) 
			return config.get(p);
		
		return null;
	}
	
	public String getOperatorId() {
        ++operatorCounter;
        return String.valueOf(operatorCounter);
	}
	
	public String getQueryName() {
		++queryCounter;
		
		return "query" + queryCounter;
	}
	
	public WorkerConfiguration getSchemaConfig() throws Exception {
		String host = config.get("psql-host");
		int port = Integer.parseInt(config.get("psql-port"));
		String dbName = config.get("psql-db");
		String user = config.get("psql-user");
		String pass = config.get("psql-password");
		String empBridgePath = Utilities.getVaultDBRoot() + "/deps/emp/emp-bridge";


		return new WorkerConfiguration("honest-broker", host, port, dbName, user, pass, empBridgePath);
	}
	
	public FrameworkConfig getCalciteConfiguration() {
		return calciteConfig;
	}
	
	public SchemaPlus getPdfSchema() {
		return pdnSchema;
	}
	
	public boolean slicingEnabled() {
		if(getProperty("sliced-execution") != null)
			return !getProperty("sliced-execution").equals("false");
		return false;
		
	}

	public CalciteConnection getCalciteConnection() {
		return calciteConnection;
	}

	public int readAndIncrementPortCounter() {
		++portCounter;
		return portCounter;
	}
	
		

	public void resetCounters() {
		operatorCounter = -1;
		queryCounter = -1;
		portCounter = 54320;
	}


	public Map<String, String> getProperties() {
		return config;
	}
	
	// all info needed to initialize remote host
    public String getSetupParameters() throws Exception {
		
		
		Map<String, String> propertiesMap = SystemConfiguration.getInstance().getProperties();
		String properties = new String();
		
		for(Entry<String, String> param : propertiesMap.entrySet()) {
			properties += param.getKey() + "=" + param.getValue() + "\n";
		}
		
		return properties;
	}
	public float getLocalCostDiscount() {
		String localDiscountStr = getProperty("local-discount");
		if(localDiscountStr == null) {
			localDiscountStr = "1.0"; // ignore this factor otherwise
		}

		return Float.valueOf(localDiscountStr);

	}

	public boolean distributedEvaluationEnabled() {
		if(config.get("distributed-eval-enabled") != null && config.get("distributed-eval-enabled").equals("false")) 
			return false;
		
		return true;
	}


	public void closeCalciteConnection() throws SQLException {
		calciteConnection.close();
		baseConnection.close();
		
	}
	

}
