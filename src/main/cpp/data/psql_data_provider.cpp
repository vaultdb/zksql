#include "psql_data_provider.h"
#include "pq_oid_defs.h"
#include <time.h>
#include <typeinfo>

#include <chrono>
#include <plain_tuple.h>
#include "query_table/field/field.h"

// if hasDummyTag == true, then last column needs to be a boolean that denotes whether the tuple was selected
// tableName == nullptr if query result from more than one table
std::shared_ptr<PlainTable>
PsqlDataProvider::getQueryTable(std::string dbname, std::string query_string, bool hasDummyTag) {

    dbName = dbname;
    pqxx::result pqxxResult;
    pqxx::connection dbConn("dbname=" + dbname);

    try {
        pqxx::work txn(dbConn);
        pqxxResult = txn.exec(query_string);
        txn.commit();


    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;

        throw e;
    }


    pqxx::row firstRow = *(pqxxResult.begin());
    int colCount = firstRow.size();
    if(hasDummyTag)
        --colCount;

    size_t rowCount = pqxxResult.size();


    tableSchema = getSchema(pqxxResult, hasDummyTag);
    std::shared_ptr<PlainTable > dst_table = std::make_shared<PlainTable>(rowCount, *tableSchema);


    int counter = 0;
    for(result::const_iterator resultPos = pqxxResult.begin(); resultPos != pqxxResult.end(); ++resultPos) {
        getTuple(*resultPos, hasDummyTag, *dst_table, counter);
        ++counter;
    }

    dbConn.disconnect();

    return dst_table;
}

std::unique_ptr<QuerySchema> PsqlDataProvider::getSchema(pqxx::result input, bool hasDummyTag) {
    pqxx::row firstRow = *(input.begin());
    size_t colCount = firstRow.size();
    if(hasDummyTag)
        --colCount; // don't include dummy tag as a separate column


    std::unique_ptr<QuerySchema> result(new QuerySchema(colCount));

    for(uint32_t i = 0; i < colCount; ++i) {
       string colName =  input.column_name(i);
       FieldType type = getFieldTypeFromOid(input.column_type(i));
        int tableId = input.column_table(i);

        srcTable = getTableName(tableId); // once per col in case of joins

        QueryFieldDesc fieldDesc(i, colName, srcTable, type, 0);

       if(type == FieldType::STRING) {

           size_t stringLength = getVarCharLength(srcTable, colName);
            fieldDesc.setStringLength(stringLength);

       }


        result->putField(fieldDesc);
    }

   if(hasDummyTag) {
        pqxx::oid dummyTagOid = input.column_type((int) colCount);
        FieldType dummyType = getFieldTypeFromOid(dummyTagOid);
        assert(dummyType == FieldType::BOOL); // check that dummy tag is a boolean
    }

    return result;
}

// queries the psql data definition to find the max length of each string in our column definitions
size_t PsqlDataProvider::getVarCharLength(std::string tableName, std::string columnName) const {
    // query the schema to get the column width
    //  SELECT character_maximum_length FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name='<table>' AND column_name='<col name>';
    std::string queryCharWidth( "SELECT character_maximum_length FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name=\'");
    queryCharWidth += tableName;
    queryCharWidth += "\' AND column_name=\'";
    queryCharWidth += columnName;
    queryCharWidth += "\'";

    pqxx::result pqxxResult = query("dbname=" + dbName, queryCharWidth);
    // read single row, single val

    row aRow = pqxxResult[0];
    field aField = aRow.at(0);

    return aField.as<size_t>();

}



string PsqlDataProvider::getTableName(int oid) {
    std::string queryTableName = "SELECT relname FROM pg_class WHERE oid=" + std::to_string(oid);
    pqxx::result pqxxResult = query("dbname=" + dbName, queryTableName);
    // read single row, single val

    if(pqxxResult.empty())
        return "";

    row aRow = pqxxResult[0];
    field aField = aRow.at(0);

    return aField.as<string>();

}

void
PsqlDataProvider::getTuple(pqxx::row row, bool hasDummyTag, PlainTable &dst_table, const size_t &idx) {
        int colCount = row.size();


        if(hasDummyTag) {
            --colCount;
        }



        PlainTuple dst_tuple = dst_table.getTuple(idx); // writes in place
        for (int i=0; i < colCount; i++) {
            const pqxx::field srcField = row[i];

           PlainField  parsedField = getField(srcField);
            dst_tuple.setField(i, parsedField);
        }

        if(hasDummyTag) {

                PlainField parsedField = getField(row[colCount]); // get the last col
                dst_tuple.setDummyTag(parsedField);
                dst_tuple.clear(dst_tuple.getDummyTag());
        }

    }


    PlainField PsqlDataProvider::getField(pqxx::field src) {

        int ordinal = src.num();
        pqxx::oid oid = src.type();
        FieldType colType = getFieldTypeFromOid(oid);

        switch (colType) {
            case FieldType::INT:
            {
                int32_t intVal = src.as<int32_t>();
                return  PlainField(colType, intVal);
            }
            case FieldType::LONG:
            {
                int64_t intVal = src.as<int64_t>();
                return  PlainField(colType, intVal);
            }

           case FieldType::DATE:
            {
                std::string dateStr = src.as<std::string>(); // YYYY-MM-DD
                std::tm timeStruct = {};
                strptime(dateStr.c_str(), "%Y-%m-%d", &timeStruct);
                int64_t epoch = mktime(&timeStruct) - 21600; // date time function is 6 hours off from how psql does it.
                return  PlainField(FieldType::LONG, epoch);

            }
            case FieldType::BOOL:
            {
                bool boolVal = src.as<bool>();
                return  PlainField(colType, boolVal);
            }
            case FieldType::FLOAT:
            {
                float floatVal = src.as<float>();
                return  PlainField(colType, floatVal);
            }

            case FieldType::STRING:
            {
                string stringVal = src.as<string>();
                size_t strLength = tableSchema->getField(ordinal).getStringLength();

                while(stringVal.size() != strLength) {
                    stringVal += " ";
                }


                return  PlainField(colType, stringVal, strLength);
            }
            default:
                throw std::invalid_argument("Unsupported column type " + std::to_string(oid));

        };

    }

pqxx::result PsqlDataProvider::query(const string &dbname, const string &query_string) const {
    pqxx::result res;
    try {
        pqxx::connection c(dbname);
        pqxx::work txn(c);

        res = txn.exec(query_string);
        txn.commit();


    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;

        throw e;
    }

    return res;
}



