package org.vaultdb.codegen.sql;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.apache.calcite.rel.RelNode;
import org.apache.calcite.sql.SqlDialect;
import org.apache.calcite.sql.SqlIdentifier;
import org.apache.calcite.sql.SqlNodeList;
import org.apache.calcite.sql.SqlSelect;
import org.apache.calcite.sql.parser.SqlParserPos;
import org.vaultdb.plan.operator.Filter;
import org.vaultdb.plan.operator.Operator;
import org.vaultdb.plan.operator.Project;
import org.vaultdb.type.SecureRelDataTypeField;

public class SecureRelToSqlConverter extends ExtendedRelToSqlConverter {

    private Operator scanOperator;
    private boolean needsRewrite;

    public SecureRelToSqlConverter(SqlDialect dialect, Operator operator) {
        super(dialect);
        scanOperator = operator;
        while (scanOperator instanceof Project || scanOperator instanceof Filter) {
            scanOperator = scanOperator.getChild(0);
        }
       // needsRewrite = !scanOperator.secureComputeOrder().isEmpty();
        // no longer needed unconditionally because we do dummy padding in java instead of RDBMS
        needsRewrite = true; // do this just once to add order by clause for dummy_tag and any other columns needed
    }

    private List<SqlIdentifier> getOrderAttrs(SqlParserPos sqlParserPos) {
        List<SqlIdentifier> result = new ArrayList<SqlIdentifier>();

        for (SecureRelDataTypeField field : scanOperator.secureComputeOrder()) {
            result.add(new SqlIdentifier(Arrays.asList(field.getName()), sqlParserPos));
        }

        return result;
    }

    private Result addOrderBy(RelNode e) {
        Result x = dispatch(e);

        final Builder builder = x.builder(e, Clause.ORDER_BY);
        SqlSelect select = x.asSelect();
        SqlNodeList list = new SqlNodeList(select.getParserPosition());

        for (SqlIdentifier iden : getOrderAttrs(select.getParserPosition()))
            list.add(iden);
        select.setOrderBy(list);

        builder.setOrderBy(list);
        Result res = builder.result();

        return res;
    }

    @Override
    public Result visitChild(int i, RelNode e, boolean isAnon) {
        if (needsRewrite) {
            needsRewrite = false;
            return addOrderBy(e);
        }

        return dispatch(e);
    }
}
