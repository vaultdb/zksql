#ifndef PSQL_DATA_PROVIDER_H
#define PSQL_DATA_PROVIDER_H

#include "query_table/query_table.h"
#include "query_table/query_tuple.h"

#include <pqxx/pqxx>

using namespace pqxx;
using namespace vaultdb;



class  PsqlDataProvider  { // :  DataProvider
public:

    std::shared_ptr<PlainTable> getQueryTable(std::string dbname, std::string query_string, bool hasDummyTag=false);

private:
    void getTuple(pqxx::row row, bool hasDummyTag, PlainTable &dst_table, const size_t &idx);
     PlainField getField(pqxx::field src);
    std::unique_ptr<QuerySchema> getSchema(pqxx::result input, bool hasDummyTag);

     std::string srcTable;
     std::string dbName;
    std::unique_ptr<QuerySchema> tableSchema;

    size_t getVarCharLength(string table, string column) const;

    string getTableName(int oid);
    pqxx::result query(const std::string &  dbname, const std::string  & query_string) const;
};


#endif //PSQL_DATA_PROVIDER_H
