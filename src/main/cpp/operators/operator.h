#ifndef _OPERATOR_H
#define _OPERATOR_H

#include <functional>
#include <vector>
#include <query_table/zk_query_table.h>
#include <common/defs.h>
#include <ctime>

using namespace std;



namespace  vaultdb {

    class TableInput;



    class Operator {

    protected:
        Operator *parent_;
        vector<Operator *> children_;
        ZkQueryTable output_; // shared_ptr for payload
        TableInput *lhs_ = nullptr;
        TableInput *rhs_ = nullptr;
        SortDefinition sort_definition_; // start out with empty sort
        QuerySchema output_schema_; // secure schema - plaintext one is stored in output
        int operatorId_ = -1; // id of operator and initialize to -1.
        time_point<high_resolution_clock> startTime = clock_start(); // timer for each operator to record starting time
        uint64_t comm_cost = 0; // comm cost for the operator
        ZKGlobalVars& gv = ZKGlobalVars::getInstance();


    public:


        Operator(const SortDefinition & sorted_on = SortDefinition()) : sort_definition_(sorted_on) {}

        virtual ~Operator();

        Operator(const ZkQueryTable& lhs, const SortDefinition & sorted_on = SortDefinition());
        Operator(const ZkQueryTable& lhs, const ZkQueryTable& rhs, const SortDefinition & sorted_on = SortDefinition());

        Operator(Operator *child, const SortDefinition & sorted_on = SortDefinition());
        Operator(Operator *lhs, Operator *rhs, const SortDefinition & sorted_on = SortDefinition());
        
        // recurses first, then invokes runSelf method
        ZkQueryTable run();
        string printTree() const;
        string toString() const;

        ZkQueryTable getOutput();

        Operator * getParent() const;

        Operator * getChild(int idx = 0) const;

        void setParent(Operator *aParent);

        void setChild(Operator *aChild, int idx = 0);

        SortDefinition  getSortOrder() const;
        void setSortOrder(const SortDefinition & aSortDefinition) { sort_definition_ = aSortDefinition; }

        QuerySchema getOutputSchema() const { return output_schema_; }

        void setOperatorId(int id) { operatorId_ = id; }

        int getOperatorId() {return operatorId_;}

        uint64_t getCommCost() {return comm_cost;}

        void reset();

    protected:
        // to be implemented by the operator classes, e.g., sort, filter, et cetera
        virtual ZkQueryTable runSelf() = 0;
        virtual string getOperatorType() const  = 0;
        virtual string getParameters() const = 0;
        ZkQueryTable zeroOutDummies(ZkQueryTable table);

        bool operator_executed_ = false; // set when runSelf() executed once

    private:
        string printHelper(const string & prefix) const;
    };

    // essentially CommonTableExpression, written here to avoid forward declarations
    class TableInput : public Operator {

    public:
        TableInput(const ZkQueryTable & inputTable) {
            Operator::output_ = move(inputTable);
            Operator::output_schema_ = QuerySchema::toSecure(*(Operator::output_.getSchema())); // should not be necessary
            Operator::sort_definition_ = inputTable.getSortOrder();
            Operator::operator_executed_ = true;
        }

        ZkQueryTable runSelf() override {
            return  Operator::output_;
        }

    protected:
        string getOperatorType() const override {
            return "TableInput";
        }

        string getParameters() const override {
            return string();
        }

    };

    ostream &operator<<(ostream &os, const Operator &op);

}

#endif //_OPERATOR_H
