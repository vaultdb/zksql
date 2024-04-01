#include "zk_query_table.h"

#include <util/data_utilities.h>

using namespace vaultdb;

ZkQueryTable::ZkQueryTable(shared_ptr<PlainTable> plain, shared_ptr<SecureTable> secure, const int &party,
                           BoolIO<NetIO> *netio) :
        plain_table_(plain), secure_table_(secure), party_(party) { }

ZkQueryTable::ZkQueryTable(const QuerySchema &schema, const SortDefinition &order_by, BoolIO<NetIO> *netio,
                           const int &party) : party_(party), netio_(netio) {
    if(party == emp::BOB) {

        uint32_t tuple_cnt;


        netio->recv_data(&tuple_cnt, 4);

        secure_table_ = SecureTable::secret_share_recv_table(tuple_cnt, schema, order_by, emp::ALICE);
    }
    else
        throw std::invalid_argument("Undefined behavior for party " + std::to_string(party));

}

ZkQueryTable::ZkQueryTable(const ZkQueryTable &src) {
    // deep copy
    if(src.plain_table_ != nullptr)
        plain_table_ = shared_ptr<PlainTable>(new PlainTable(*src.plain_table_));

    if(src.secure_table_ != nullptr)
        secure_table_ = shared_ptr<SecureTable>(new SecureTable(*src.secure_table_));

    party_ = src.party_;
    netio_ = src.netio_;

}



ZkQueryTable::ZkQueryTable(shared_ptr<PlainTable> input, BoolIO<NetIO> *netio, const int &party) : plain_table_(input), party_(party), netio_(netio) {

    if(party == emp::ALICE) {
        netio_ = netio;
        uint32_t tuple_cnt = input->getTupleCount();

        netio->send_data(&tuple_cnt, 4);
        secure_table_ = PlainTable::secret_share_send_table(input, party_);
    }
    else
        throw std::invalid_argument("Undefined behavior for party " + std::to_string(party));

}

ZkQueryTable::ZkQueryTable(shared_ptr<PlainTable> input, BoolIO<NetIO> *netio, int sentBitsSize, const int &party) : plain_table_(input), party_(party), netio_(netio) {

    if(party == emp::ALICE) {
        vector<int8_t> inputTupleData = input->tuple_data_;

        vector<emp::Bit> bitsTupleData = ZKUtils::tupleDataToBits(inputTupleData);

        secure_table_ = SecureTable::deserialize(*input->getSchema(), bitsTupleData);
        secure_table_->setSortOrder(input->getSortOrder());
    }
}

ZkQueryTable::ZkQueryTable(const QuerySchema &schema, const SortDefinition &order_by, BoolIO<NetIO> *netio, int receivedBitsSize,
                           const int &party) : party_(party), netio_(netio) {

    if(party == emp::BOB) {

        vector<emp::Bit> bitsTupleData;

        for(int i = 0; i < receivedBitsSize; ++i) {
            bitsTupleData.push_back(emp::Bit(false,ALICE));
        }

        secure_table_ = SecureTable::deserialize(schema,bitsTupleData);
        secure_table_->setSortOrder(order_by);
    }
}

ZkQueryTable::ZkQueryTable(const size_t &num_tuples, const QuerySchema &schema, const int &party, BoolIO<NetIO> *netio,
                           const SortDefinition &sort_definition) : party_(party), netio_(netio) {

    if(party_ == ALICE)
        plain_table_ = shared_ptr<PlainTable>(new PlainTable(num_tuples, QuerySchema::toPlain(schema), sort_definition));
    secure_table_ = shared_ptr<SecureTable>(new SecureTable(num_tuples, QuerySchema::toSecure(schema), sort_definition));

}


const shared_ptr<QuerySchema>  ZkQueryTable::getSchema() const {
    assert(secure_table_ != nullptr);

    return secure_table_->getSchema();
}

SortDefinition ZkQueryTable::getSortOrder() const {
    assert(secure_table_ != nullptr);

    return secure_table_->getSortOrder();
}

bool ZkQueryTable::initialized() const {
    if(party_ == emp::BOB)
        return secure_table_.get() != nullptr;

    // it's only initialized if we are running as ALICE or BOB
     if(party_ != emp::ALICE)
         return false;

    return plain_table_.get() != nullptr && secure_table_.get() != nullptr;
}

int ZkQueryTable::getTupleCount() const {
    assert(initialized());

    return secure_table_->getTupleCount();
}

void ZkQueryTable::setSortOrder(const SortDefinition &s) {
    if(plain_table_ != nullptr) plain_table_->setSortOrder(s);
    if(secure_table_ != nullptr) secure_table_->setSortOrder(s);
}

void ZkQueryTable::resize(const size_t & limit) {
    if(plain_table_ != nullptr) plain_table_->resize(limit);
    if(secure_table_ != nullptr) secure_table_->resize(limit);

}

// for use in debugging, verify that plain table and secure table have the same values
void ZkQueryTable::validate_tables() const {
    shared_ptr<PlainTable> revealed = secure_table_->reveal();
    if(party_ == ALICE) {
        assert(*revealed == *plain_table_);
    }
}

string ZkQueryTable::toString(bool showDummies) const {
    return secure_table_->reveal()->toString(showDummies);
}

void ZkQueryTable::reset() {
    plain_table_.reset();
    secure_table_.reset();
}





