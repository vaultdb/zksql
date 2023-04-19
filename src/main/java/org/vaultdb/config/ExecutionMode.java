package org.vaultdb.config;

import org.vaultdb.config.SystemConfiguration;

public class ExecutionMode {
		public boolean distributed = true; // || distributed.   Distributed clear usually will be a semi-join or some other rewrite that isolates the public variables
		public boolean oblivious = true;
		public boolean sliced = false; // might get parlayed into something with a fixed output size inferred from schema info, e.g., pkey, # of distinct values, sliced card summed up, this may be an interesting research challenge: optimizing how these knobs play together
		public boolean replicated = false; // when a table is replicated among all data owners, holds true when we join replicated tables too
	

		public ExecutionMode(ExecutionMode executionMode) {
			
			this.distributed = executionMode.distributed;
			this.oblivious = executionMode.oblivious;
			this.sliced = executionMode.sliced;
			this.replicated = executionMode.replicated;
		}


		public ExecutionMode() {
			// initialized to defaults above
		}


		@Override
		public String toString() {

			String descriptor = distributed ?  new String("Distributed")
					:  new String("Local");
			
			descriptor += oblivious ? "Oblivious" : "Clear";
			if(!distributed)
				descriptor += replicated ? "Replicated" : "Partitioned";
			
			return descriptor;
		}
		
		
		@Override
		public boolean equals(Object obj) {
			if(!(obj instanceof ExecutionMode))
				return false;
			
			ExecutionMode other = (ExecutionMode) obj;
			

			try {
				SystemConfiguration config = SystemConfiguration.getInstance();
				if(config.slicingEnabled() && other.sliced != this.sliced)
					return false;
			
			} catch (Exception e) {
				System.out.println("Failed to get system configuration! ");
				e.printStackTrace();
				
			}
			
			if(other.distributed == this.distributed
					&& other.oblivious == this.oblivious 
					&& other.replicated == this.replicated)
				return true;
			
			return false;
			
		}
}
