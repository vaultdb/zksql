#ifndef _PLAN_READER_H
#define _PLAN_READER_H

#include <string>
#include <map>
#include <boost/property_tree/ptree.hpp>
#include <emp-tool/emp-tool.h>
#include <operators/operator.h>


// parse this from 1) list of SQL statements, and 2) Apache Calcite JSON for secure plan
// plan generator from SQL is in vaultdb-mock repo

namespace vaultdb {

    class PlanParser {
    public:
       PlanParser(const std::string &db_name, std::string plan_name, BoolIO<NetIO>  * netio, const int & party, const int & limit = -1);
        shared_ptr<Operator> getRoot() const { return root_; }
        shared_ptr<Operator> getOperator(const int & op_id);

        static shared_ptr<Operator > parse(const std::string & db_name, const std::string & plan_name, BoolIO<NetIO>  * netio, const int & party, const int & limit = -1);
        static pair<int, SortDefinition> parseSqlHeader(const string & header);
    protected:
        std::string db_name_;
        BoolIO<NetIO> *netio_ = nullptr;
        int party_ = emp::PUBLIC;
        shared_ptr<Operator > root_;
        int input_limit_ = -1; // to add a limit clause to SQL statements for efficient testing

        std::map<int, shared_ptr<Operator > > operators_; // op ID --> operator instantiation
        std::vector<std::shared_ptr<Operator > > support_ops_; // these ones don't get an operator ID from the JSON plan

        void parseSqlInputs(const std::string & input_file);
        void parseSecurePlan(const std::string & plan_file);

        // operator parsers
        void parseOperator(const int & operator_id, const std::string & op_name, const  boost::property_tree::ptree &pt);
        std::shared_ptr<Operator> parseSort(const int &operator_id, const boost::property_tree::ptree &pt);
        std::shared_ptr<Operator> parseAggregate(const int & operator_id, const boost::property_tree::ptree &pt);
        std::shared_ptr<Operator> parseJoin(const int & operator_id, const boost::property_tree::ptree &pt);
        std::shared_ptr<Operator> parseFilter(const int & operator_id, const boost::property_tree::ptree &pt);
        std::shared_ptr<Operator> parseProjection(const int & operator_id, const boost::property_tree::ptree &project_tree);
        std::shared_ptr<Operator> parseSeqScan(const int & operator_id, const boost::property_tree::ptree &seq_scan_tree);

        shared_ptr<Operator> createInputOperator(const string &sql, const SortDefinition &collation, const bool &has_dummy_tag);

        // utils
        const std::shared_ptr<Operator>
        getChildOperator(const int &my_operator_id, const boost::property_tree::ptree &pt) const;
        const std::string truncateInput(const std::string sql) const;



        void print(const boost::property_tree::ptree &pt, const std::string &prefix) const;

    };
}



#endif // _PLAN_READER_H
