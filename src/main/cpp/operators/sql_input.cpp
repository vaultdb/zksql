#include <data/psql_data_provider.h>
#include "sql_input.h"
#include <boost/algorithm/string/replace.hpp>


using namespace vaultdb;

// run this eagerly to get the schema, needed to propogate to parent operators
SqlInput::SqlInput(std::string db, std::string sql,  const int & party, BoolIO<NetIO> *netio,  bool dummyTag) :   input_query_(sql), db_name_(db), dummy_tagged_(dummyTag), party_(party),  netio_(netio), tuple_limit_(0) {
    runQuery();
    operator_executed_ = true;
    output_schema_ = *(output_.getSchema());

}

SqlInput::SqlInput(std::string db, std::string sql, bool dummyTag, const SortDefinition &sortDefinition,  const int & party, BoolIO<NetIO> *netio, const int & tuple_limit) :
        Operator(sortDefinition), input_query_(sql), db_name_(db), dummy_tagged_(dummyTag),  party_(party), netio_(netio), tuple_limit_(tuple_limit) {

    runQuery();
    operator_executed_ = true;
    output_schema_ = *(output_.getSchema());
}



// read in the data from supplied SQL query
ZkQueryTable SqlInput::runSelf() {

    if(!operator_executed_) {
        runQuery();
        operator_executed_ = true;
        output_schema_ = *(output_.getSchema());

    }
    return output_;


}

void SqlInput::runQuery() {
    uint64_t commCostFromAllThreadsBefore = gv.getAccumulativeCommCostFromAllThreads();

    startTime = clock_start();

    PsqlDataProvider dataProvider;

    if(tuple_limit_ > 0) { // truncate inputs
        boost::replace_all(input_query_, ";", "");

        input_query_ = "SELECT * FROM (" + input_query_ + ") input LIMIT " + std::to_string(tuple_limit_);
    }

    std::shared_ptr<PlainTable> local_output = dataProvider.getQueryTable(db_name_, input_query_, dummy_tagged_);
    local_output->setSortOrder(sort_definition_);

    if(party_ == ALICE) {

        if(local_output->getTupleCount() <= 0) {
            throw std::invalid_argument("read empty input from \"" + input_query_ + "\"");
        }

        output_ = ZkQueryTable(local_output, netio_, party_);
    }
    else {
        assert(party_ == BOB);
        output_ = ZkQueryTable(*(local_output->getSchema()), sort_definition_, netio_, party_);
    }

    double duration = time_from(startTime) / 1e6;

    uint64_t commCostFromAllThreadsAfter = gv.getAccumulativeCommCostFromAllThreads();
    uint64_t totalCommCost = commCostFromAllThreadsAfter - commCostFromAllThreadsBefore;
    gv.setCommCost(gv.getCommCost() + totalCommCost);

}

string SqlInput::getOperatorType() const {
    return "SqlInput";
}

string SqlInput::getParameters() const {
    string str = input_query_;
    std::replace(str.begin(), str.end(), '\n', ' ');
    string tuple_cnt = (output_.initialized()) ? ", tuple_count=" + std::to_string(output_.getTupleCount()) : "";
    return "\"" + str + "\"" + tuple_cnt;


}

void SqlInput::truncateInput(const size_t &limit) {
    output_.resize(limit);
}


