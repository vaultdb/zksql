#include <util/data_utilities.h>
#include "operator.h"


using namespace vaultdb;


Operator::Operator(const ZkQueryTable& lhsSrc, const SortDefinition & sorted_on) : sort_definition_(sorted_on) {
    lhs_ = new TableInput(lhsSrc);
    children_.push_back((Operator *) lhs_);
    output_schema_ = QuerySchema::toSecure(*lhsSrc.getSchema());
    if(sorted_on.empty())
        sort_definition_ = lhsSrc.getSortOrder();
}


Operator::Operator(const ZkQueryTable& lhsSrc, const ZkQueryTable& rhsSrc, const SortDefinition & sorted_on) : sort_definition_(sorted_on) {
    lhs_ = new TableInput(lhsSrc);
    children_.push_back((Operator *) lhs_);

    rhs_ = new TableInput(rhsSrc);
    children_.push_back((Operator *) rhs_);

}


Operator::Operator(Operator *child, const SortDefinition & sorted_on) : sort_definition_(sorted_on)  {

     child->setParent(this);
    children_.push_back(child);
    output_schema_ = QuerySchema::toSecure(child->output_schema_);
    if(sorted_on.empty())
        sort_definition_ = child->sort_definition_;

}


Operator::Operator(Operator *lhs, Operator *rhs, const SortDefinition & sorted_on) : sort_definition_(sorted_on)  {

    lhs->setParent(this);
    rhs->setParent(this);
    children_.push_back(lhs);
    children_.push_back(rhs);

}



ZkQueryTable Operator::run() {
    if(operator_executed_) // prevent duplicate executions of operator
        return output_;

    for(Operator *op : children_) {
        op->run();
    }

    clock_t secureStartClock = clock();
    output_ = runSelf(); // delegated to children
    double secureClockTicks = (double) (clock() - secureStartClock);
    double secureClockTicksPerSecond = secureClockTicks / ((double) CLOCKS_PER_SEC);

    double duration = time_from(startTime) / 1e6;


    operator_executed_ = true;
    sort_definition_  = output_.getSortOrder(); // update this if needed
    size_t resident_memory = Utilities::residentMemoryUtilization();
    if(getOperatorId() > 0) {
        size_t peak_memory = Utilities::checkMemoryUtilization();
        cout << "Time: " << duration << "s,resident memory: " <<  resident_memory << " bytes, peak memory = "  << peak_memory <<  " bytes, communication cost: " << comm_cost << " bytes, CPU clock ticks: " << secureClockTicks << ",CPU clock ticks per second: " << secureClockTicksPerSecond << ",Validating " << toString();
        cout << "...success!" << endl;
    }


#ifndef DEBUG
    if(this->operatorId_ > -1) {   // don't delete sub-operators like Projection within Join
        for (Operator *op: children_) {

            op->reset();

        }

    }
#endif

    return output_;
}



string Operator::printTree() const {
    return printHelper("");

}


string Operator::printHelper(const string &prefix) const {
    stringstream  ss;
    ss << prefix << toString() << endl;
    string indent = prefix + "    ";
    for(Operator *op : children_) {
        ss << op->printHelper(indent);
    }

    return ss.str();
}



string Operator::toString() const {
    stringstream  ss;

    ss <<   getOperatorType() << " ";

    if(operatorId_ >= 0) {
        ss << "(" << operatorId_ << ")";
    }

    string params = getParameters();
    if(!params.empty())
        ss << "(" << params << ") ";

    ss << ": " <<  output_schema_ <<    " order by: " << DataUtilities::printSortDefinition(sort_definition_);


    if(output_.initialized())
        ss << " tuple count: " << output_.getTupleCount();

    return ss.str();

}




 ZkQueryTable Operator ::getOutput()  {
    if(!operator_executed_) { // if we haven't run it yet
        output_ = run();
    }

    return output_;
}


Operator * Operator::getParent() const {
    return parent_;
}


 Operator * Operator::getChild(int idx) const {
    return children_[idx];
}


void Operator::setParent(Operator *aParent) {
    parent_ = aParent;
}


void Operator::setChild(Operator *aChild, int idx) {
    children_[idx] = aChild;
}


 Operator::~Operator() {
     delete lhs_;
    delete rhs_;

}


SortDefinition Operator::getSortOrder() const {
    if(output_.initialized())
        assert(sort_definition_ == output_.getSortOrder()); // check that output table is aligned with operator's sort order
    return sort_definition_;
}


ostream &vaultdb::operator<<(ostream &os, const Operator &op) {
    os << op.printTree();
    return os;
}

ZkQueryTable Operator::zeroOutDummies(ZkQueryTable table) {
    ZkQueryTable result(table); // deep copy
    int party = table.party_;

    for(int i = 0; i < result.getTupleCount(); ++i) {
        result.secure_table_->getTuple(i).clear( (*result.secure_table_)[i].getDummyTag());


        if(party == ALICE)
            result.plain_table_->getTuple(i).clear( (*result.plain_table_)[i].getDummyTag());


    }

    //result.netio_->flush();
    return result;
}




void Operator::reset() {

    output_.reset();
    assert(!output_.initialized());
    operator_executed_ = false;



}



