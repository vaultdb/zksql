#include "sort.h"

#include "plain_tuple.h"
#include <util/data_utilities.h>
#include <emp-zk-set/zk_set.h>
#include <util/zk_global_vars.h>


using namespace vaultdb;


Sort::Sort(Operator *child, const SortDefinition &aSortDefinition, const int & limit) : Operator(child, aSortDefinition), limit_(limit) {

    for(ColumnSort s : sort_definition_) {
        if(s.second == SortDirection::INVALID)
            throw; // invalid sort definition
    }

    if(limit_ > 0)
        assert(sort_definition_[0].first == -1); // Need to sort on dummy tag to make resizing not delete real tuples


}


Sort::Sort(const ZkQueryTable & child, const SortDefinition &aSortDefinition, const int & limit) : Operator(child, aSortDefinition), limit_(limit) {

    for(ColumnSort s : sort_definition_) {
        if(s.second == SortDirection::INVALID)
            throw; // invalid sort definition
    }

    if(limit_ > 0)
        assert(sort_definition_[0].first == -1); // Need to sort on dummy tag to make resizing not delete real tuples

}


ZkQueryTable Sort::runSelf() {
    ZkQueryTable input = children_[0]->getOutput();

    uint64_t commCostFromAllThreadsBefore = gv.getAccumulativeCommCostFromAllThreads();

    startTime = clock_start();

    size_t tuple_cnt = input.getTupleCount();

    if(input.party_ == ALICE) {
        shared_ptr<PlainTable> sort_input = input.plain_table_;
        shared_ptr<PlainTable> sort_output(new PlainTable(*sort_input));
        QuerySchema plain_schema = *(sort_input->getSchema());

        std::vector<PrimitivePlainTuple> to_sort;
        to_sort.resize(tuple_cnt);

        vector<PrimitivePlainTuple>::iterator tuple_pos = to_sort.begin();

        // copy into PrimitivePlainTuples
        for (size_t i = 0; i < tuple_cnt; i++) {
            *tuple_pos = PrimitivePlainTuple(sort_input->getTuple(i));
            ++tuple_pos;
        }


        std::sort(to_sort.begin(), to_sort.end(), tuple_less(sort_definition_));

        // put into sort_output
        for(size_t i = 0; i < to_sort.size(); i++) {
            sort_output->putTuple(i, to_sort.at(i).getTuple());
        }

        sort_output->setSortOrder(sort_definition_);

        output_ = ZkQueryTable(sort_output, input.netio_, tuple_cnt * input.getSchema()->size(),  ALICE);
    }
    else {
        assert(input.party_ == emp::BOB);
        output_ = ZkQueryTable(*(input.getSchema()), sort_definition_, input.netio_, tuple_cnt * input.getSchema()->size(), BOB);
    }

    if(limit_ > 0 && output_.getTupleCount() > limit_) {
        output_.resize(limit_);
    }

    // verify set equality
    ZKSet<BoolIO<NetIO>> *zkset = gv.getZKSet();
    bool check = zkset->equal(reinterpret_cast<Bit*>(input.secure_table_->tuple_data_.data()),
                              reinterpret_cast<Bit*>(output_.secure_table_->tuple_data_.data()),
                              tuple_cnt,
                              input.getSchema()->size());

    if(output_.party_ == BOB) {
        assert(check);
    }

    // verify order
    Bit correctOrder = true;

    // update a flag by iterating all tuples to check order
    for (int i = 1; i < output_.getTupleCount(); ++i) {
        SecureTuple prevTuple = output_.secure_table_->getTuple(i - 1);
        SecureTuple curTuple = output_.secure_table_->getTuple(i);

        // use checkOrder to determine if a tuple is needed to sort or not.
        // if not, it means the tuple is in the correct order.
        Bit isCorrectOrder = Sort::checkOrder(prevTuple, curTuple, output_.getSortOrder());
        correctOrder = correctOrder & isCorrectOrder;
    }


    bool correct_plain = correctOrder.reveal();
    if(!correct_plain) {
        cout << "Failed verification on sort order for operator " << Operator::operatorId_ << " with "
             << DataUtilities::printSortDefinition(sort_definition_) << endl;
        shared_ptr<PlainTable> revealed = output_.secure_table_->reveal();
        cout << "Ordered table: " << revealed->toString(true) << endl;
    }

    assert(correct_plain);
    if(!correctOrder.reveal())
        throw std::invalid_argument("Sort failed to prove correctness of ordering!");

    uint64_t commCostFromAllThreadsAfter = gv.getAccumulativeCommCostFromAllThreads();
    uint64_t costAfter = commCostFromAllThreadsAfter - commCostFromAllThreadsBefore;
    comm_cost = costAfter;
    gv.setCommCost(gv.getCommCost() + costAfter);

    return output_;

}

Sort::~Sort() {

}

// lhs < rhs in final sort order
bool Sort::lt(const PrimitivePlainTuple & lhs, const PrimitivePlainTuple & rhs, const SortDefinition  & sort_definition)  {
    bool lt_tuple = false;
    bool lt_init = lt_tuple;


    PlainField lhs_dummy_tag = PlainField(lhs.getDummyTag());
    PlainField rhs_dummy_tag = PlainField(rhs.getDummyTag());

    for (size_t i = 0; i < sort_definition.size(); ++i) {

        const PlainField lhs_field = sort_definition[i].first == -1 ? lhs_dummy_tag
                                                                    : lhs.getField(sort_definition[i].first);
        const PlainField rhs_field = sort_definition[i].first == -1 ? rhs_dummy_tag
                                                                    : rhs.getField(sort_definition[i].first);

        bool asc = (sort_definition[i].second == SortDirection::ASCENDING);

        bool neq = lhs_field != rhs_field;
        bool lt_match = ((lhs_field < rhs_field) && asc) ||  ((lhs_field > rhs_field) && !asc);

        lt_tuple = (lt_init) ? lt_tuple : lt_match;
        lt_init = lt_init | neq;
    }

    return lt_tuple;
}

string Sort::getOperatorType() const {
    return "Sort";
}

string Sort::getParameters() const {
    return DataUtilities::printSortDefinition(sort_definition_);
}



