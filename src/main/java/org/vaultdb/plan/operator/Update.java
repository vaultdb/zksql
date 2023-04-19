package org.vaultdb.plan.operator;

import org.vaultdb.plan.SecureRelNode;
import org.vaultdb.type.SecureRelRecordType;
import org.vaultdb.config.ExecutionMode;
import org.vaultdb.type.SecureRelDataTypeField.SecurityPolicy;
import org.apache.calcite.sql.SqlUpdate;

public class Update extends Operator {
    public Update(String name, SecureRelNode src, Operator ...children ) throws Exception{
        super(name, src, children); 
    }

    @Override
    public void inferExecutionMode() throws Exception {
        Operator child = children.get(0);
        child.inferExecutionMode();

        // always takes on the execution mode of its child since it runs deterministically
        executionMode = new ExecutionMode(child.executionMode);
    }
}
