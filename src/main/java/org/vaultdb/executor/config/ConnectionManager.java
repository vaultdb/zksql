package org.vaultdb.executor.config;

import java.sql.Connection;
import java.sql.SQLException;
import java.util.*;

import org.apache.commons.lang3.StringUtils;
import org.vaultdb.config.SystemConfiguration;
import org.vaultdb.config.WorkerConfiguration;
import org.vaultdb.util.FileUtilities;
import org.vaultdb.util.Utilities;

// read config and connect to psql instances
public class ConnectionManager {
	
	private Map<String, WorkerConfiguration> workers;
	private Map<Integer, WorkerConfiguration> workersById;
	private List<String> hosts;
	
	private static ConnectionManager instance = null;
	
	
	protected ConnectionManager() throws Exception {

		workers = new LinkedHashMap<String, WorkerConfiguration>();
		hosts = new ArrayList<String>();
		
		
		workersById = new HashMap<Integer, WorkerConfiguration>();
		initialize();
	}
	

	   public static ConnectionManager getInstance() throws Exception {
		      if(instance == null) {
		         instance = new ConnectionManager();
		      }
		   
		      return instance;
		   }

	  

	public void closeConnections() throws Exception {
		

			for(WorkerConfiguration w : workersById.values()) {
				w.closeConnection();

		}
	}
	

	public static void reset() {
		instance = null;

	}

	private void initialize() throws Exception, SQLException {
		List<String> hosts = null;
		SystemConfiguration config = SystemConfiguration.getInstance();
		
		
		String connectionParameters = System.getProperty("vaultdb.connections.str");
		if(connectionParameters != null) {
			hosts = Arrays.asList(StringUtils.split(connectionParameters, '\n'));
		}
		
		else {
			String connectionsFile = config.getProperty("data-providers");		
			String configHosts = Utilities.getVaultDBRoot() + "/" + connectionsFile;

			 hosts = FileUtilities.readFile(configHosts);
		}
		
		for(String h : hosts) {
			parseConnection(h);
		}
		
	}
	
	
	private void parseConnection(String c) throws NumberFormatException, Exception, SQLException {
		if(c.startsWith("#")) { // comment in spec
			return;
		}
     

		WorkerConfiguration worker = new WorkerConfiguration(c);

		if(!hosts.contains(worker.hostname)) 
			hosts.add(worker.hostname);

		workers.put(worker.workerId,  worker);
		workersById.put(worker.dbId, worker);
	}

	
 	// list of all hostnames in VaultDB deployment
	public List<String> getHosts() {
		return hosts;
	}
	
	
	public List<WorkerConfiguration> getWorkerConfigurations() {
		return new ArrayList<WorkerConfiguration>(workers.values());
	}
	
	public Set<String> getDataSources() {
		return  workers.keySet();
	}
	
	public WorkerConfiguration getWorker(String workerId) {
		return workers.get(workerId);
	}

	public List<String> getWorkers() {
		List<String> keys = new ArrayList<String>(workers.keySet());
		return keys;
	}

	// get first connection
	public String getAlice() {
		List<String> keys = new ArrayList<String>(workers.keySet());
		return keys.get(0);
	}
	
	// get second connection
	public String getBob() {
		List<String> keys = new ArrayList<String>(workers.keySet());
		return keys.get(1);
	}

	// get first and second connection (all non-union connections)
	public List<String> getAliceAndBob() {
		List<String> keys = new ArrayList<String>(workers.keySet());
		return keys.subList(0, 2);
	}

	// optional third db that contains the union of the inputs of Alice and Bob for testing
	public String getUnioned() {
		if(workers.size() > 2) {
			List<String> keys = new ArrayList<String>(workers.keySet());
			return keys.get(2);

		}
		return null;
	}
	
	// for C++/pqxx connection
	// 1 = alice, 2 = bob
	public String getConnectionString(int party) {
		WorkerConfiguration config = workersById.get(party);
		String connectionString = new String();
		// "dbname = " + db + " user = " + user_db + " host = " + host_db + " port = " + port_db;
		
		connectionString = "dbname=" + config.dbName + " user=" + config.user
				+ " host=" + config.hostname + " port=" + config.dbPort;
		
		return connectionString;
		
	}


	public Connection getConnection(String workerId) throws Exception {
		return workers.get(workerId).getDbConnection();
	}
	
}

