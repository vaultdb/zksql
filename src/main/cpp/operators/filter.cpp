#include "filter.h"
#include <query_table/plain_tuple.h>
#include <query_table/secure_tuple.h>
#include <expression/visitor/plain_to_secure_visitor.h>

using namespace vaultdb;



Filter::Filter(Operator *child, const BoolExpression<bool> & predicate) :
     Operator(child, child->getSortOrder()), plain_predicate_(predicate), secure_predicate_(nullptr) {
        PlainToSecureVisitor visitor(predicate.root_);
        secure_predicate_ = visitor.getSecureExpression();
     }


Filter::Filter(const ZkQueryTable  & child, const BoolExpression<bool> & predicate) :
     Operator(child, child.getSortOrder()), plain_predicate_(predicate), secure_predicate_(nullptr) {
        PlainToSecureVisitor visitor(predicate.root_);
        secure_predicate_ = visitor.getSecureExpression();

}


ZkQueryTable Filter::runSelf() {
    ZkQueryTable input = Operator::children_[0]->getOutput();

    uint64_t commCostFromAllThreadsBefore = gv.getAccumulativeCommCostFromAllThreads();

    startTime = clock_start();

    output_ = ZkQueryTable(input);

    for(int i = 0; i < Operator::output_.getTupleCount(); ++i) {
        // Evaluate the predicate in the clear when possible
        if(output_.party_ == emp::ALICE) {
            PlainTuple plain = output_.plain_table_->getTuple(i);
            bool plain_matches = plain_predicate_.callBoolExpression(plain);
            bool dummy_tag = plain.getDummyTag();
            dummy_tag =  ((!plain_matches) | dummy_tag); // (!) because dummyTag is false if our selection criteria is satisfied
            plain.setDummyTag(dummy_tag);
            plain.clear(dummy_tag);
        }

        SecureTuple secure = Operator::output_.secure_table_->getTuple(i);
        emp::Bit secure_matches = secure_predicate_.callBoolExpression(secure);
        emp::Bit secure_dummy_tag = secure.getDummyTag() | (!secure_matches);
        secure.setDummyTag(secure_dummy_tag);
        secure.clear(secure_dummy_tag);
    }

    uint64_t commCostFromAllThreadsAfter = gv.getAccumulativeCommCostFromAllThreads();
    comm_cost = commCostFromAllThreadsAfter - commCostFromAllThreadsBefore;
    gv.setCommCost(gv.getCommCost() + comm_cost);

    return output_;

}


string Filter::getOperatorType() const {
    return "Filter";
}


string Filter::getParameters() const {

    return plain_predicate_.root_->toString();
}

