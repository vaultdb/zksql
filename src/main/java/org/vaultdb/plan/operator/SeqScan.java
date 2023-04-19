package org.vaultdb.plan.operator;

import java.util.ArrayList;
import java.util.List;

import org.apache.calcite.adapter.jdbc.JdbcTableScan;
import org.apache.calcite.sql.type.SqlTypeName;
import org.apache.calcite.util.Pair;
import org.vaultdb.db.schema.SystemCatalog;
import org.vaultdb.config.ExecutionMode;
import org.vaultdb.plan.SecureRelNode;
import org.vaultdb.type.SecureRelDataTypeField;
import org.vaultdb.type.SecureRelRecordType;

public class SeqScan extends Operator {
	
	String tableName;
	SystemCatalog catalog;

	public SeqScan(String name, SecureRelNode src, Operator ... children ) throws Exception {
		super(name, src, children);

		catalog = SystemCatalog.getInstance();
		
		JdbcTableScan scan = (JdbcTableScan) src.getPhysicalNode().baseRelNode.getRelNode();
		tableName = scan.getTable().getQualifiedName().get(0);

		executionMode = new ExecutionMode();
		executionMode.distributed = false;
		executionMode.oblivious = false;
		executionMode.replicated = catalog.isReplicated(tableName);
		
	}
	
	public SecureRelRecordType getInSchema() {
		return baseRelNode.getSchema();
	}
	
	@Override
	public SecureRelRecordType getSchema(boolean isSecureLeaf) {
		return baseRelNode.getSchema();
	}
	


	@Override
	public void inferExecutionMode() {
	    // done in constructor
	}


	private List<String> getOrderableFields() {
		List<String> fieldNames = new ArrayList<String>();
		Operator parent = this.parent;
		while (parent != null) {
			if (parent instanceof Project) {
				for (SecureRelDataTypeField field : parent.getSchema().getAttributes()) {
					SqlTypeName type = field.getType().getSqlTypeName();

					if ((SqlTypeName.NUMERIC_TYPES.contains(type) || SqlTypeName.DATETIME_TYPES.contains(type)) && !fieldNames.contains(field.getName()))
						fieldNames.add(field.getName());
				}
			}

			parent = parent.getParent();
		}
		return fieldNames;
	}

	@Override
	public List<SecureRelDataTypeField> secureComputeOrder() {
		List<SecureRelDataTypeField> result = new ArrayList<SecureRelDataTypeField>();
		for (String name: getOrderableFields()) {
			for (SecureRelDataTypeField field : this.getSchema().getAttributes()) {
				if (field.getName().equals(name))
					result.add(field);
			}
		}

		return result;
	}
}
