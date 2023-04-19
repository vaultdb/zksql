#include "union.h"

using namespace vaultdb;


ZkQueryTable Union::runSelf() {
    ZkQueryTable lhs = Operator::children_[0]->run();
    ZkQueryTable rhs = Operator::children_[1]->run();

    uint64_t commCostFromAllThreadsBefore = gv.getAccumulativeCommCostFromAllThreads();

    startTime = clock_start();

    assert(*lhs.getSchema() == *rhs.getSchema()); // union compatible
    assert(lhs.party_ == rhs.party_);

    QuerySchema output_schema = *lhs.getSchema();

    size_t tuple_count = lhs.getTupleCount() + rhs.getTupleCount();
    Operator::output_ = ZkQueryTable(tuple_count, output_schema, lhs.party_, lhs.netio_); // intentionally empty sort definition
    copy_plain(lhs, rhs);
    copy_secure(lhs, rhs);

    uint64_t commCostFromAllThreadsAfter = gv.getAccumulativeCommCostFromAllThreads();
    comm_cost = commCostFromAllThreadsAfter - commCostFromAllThreadsBefore;
    gv.setCommCost(gv.getCommCost() + comm_cost);

    return Operator::output_;
}




string Union::getOperatorType() const {
    return "Union";
}


string Union::getParameters() const {
    return std::string();
}

void Union::copy_plain(const ZkQueryTable &lhs, const ZkQueryTable &rhs) const {
    if(lhs.party_ == emp::BOB || rhs.party_ == emp::BOB) // BOB does not have plaintext
        return;


    size_t tuple_size_bytes = lhs.plain_table_->tuple_size_;

    int8_t *write_ptr = Operator::output_.plain_table_->tuple_data_.data();
    int8_t *read_ptr = lhs.plain_table_->tuple_data_.data();

    memcpy(write_ptr, read_ptr, tuple_size_bytes * lhs.getTupleCount());
    write_ptr += tuple_size_bytes * lhs.getTupleCount();
    read_ptr = rhs.plain_table_->tuple_data_.data();

    memcpy(write_ptr, read_ptr, tuple_size_bytes * rhs.getTupleCount());

}
void Union::copy_secure(const ZkQueryTable &lhs, const ZkQueryTable &rhs) const {
    if(lhs.secure_table_ == nullptr || rhs.secure_table_ == nullptr)
        return; // don't copy

    size_t tuple_size_bytes = lhs.secure_table_->tuple_size_;


    int8_t *write_ptr = Operator::output_.secure_table_->tuple_data_.data();
    int8_t *read_ptr = lhs.secure_table_->tuple_data_.data();

    memcpy(write_ptr, read_ptr, tuple_size_bytes * lhs.getTupleCount());
    write_ptr += tuple_size_bytes * lhs.getTupleCount();
    read_ptr = rhs.secure_table_->tuple_data_.data();

    memcpy(write_ptr, read_ptr, tuple_size_bytes * rhs.getTupleCount());

}


