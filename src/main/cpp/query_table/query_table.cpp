#include <memory>
#include <utility>
#include <util/data_utilities.h>
#include <util/type_utilities.h>
#include <util/field_utilities.h>
#include "query_table.h"
#include "plain_tuple.h"
#include "secure_tuple.h"


using namespace vaultdb;




template <typename B>
void QueryTable<B>::setSchema(const QuerySchema & s) {
    schema_ = std::make_shared<QuerySchema>(s);
    tuple_size_ = schema_->size()/8; // bytes for plaintext

    if(std::is_same_v<emp::Bit, B>) {
        size_t tuple_bits = schema_->size();
        tuple_size_ = tuple_bits * sizeof(emp::block); // bits, one block per bit
    }
}

template <typename B>
const std::shared_ptr<QuerySchema>  QueryTable<B>::getSchema() const { return schema_; }

template <typename B>
unsigned int QueryTable<B>::getTupleCount() const {
    return tuple_data_.size() / tuple_size_;
}


template <typename B>
QueryTable<B>::QueryTable(const size_t &num_tuples, const QuerySchema &schema, const SortDefinition & sortDefinition)
        :  order_by_(std::move(sortDefinition)), schema_(std::make_shared<QuerySchema>(schema)) {

     tuple_size_ = schema_->size()/8; // bytes for plaintext

    if(std::is_same_v<emp::Bit, B>) {
        size_t tuple_bits = schema_->size();
        tuple_size_ = tuple_bits * sizeof(emp::block); // bits, one block per bit
    }

    tuple_data_.resize(num_tuples * tuple_size_);
    if(std::is_same_v<emp::Bit, B>) {
        emp::Integer tmp(schema_->size() * num_tuples, 0, emp::PUBLIC);
        memcpy(tuple_data_.data(), tmp.bits.data(), schema_->size() * num_tuples);
    }
    else {
        std::memset(tuple_data_.data(), 0, tuple_data_.size());
    }

}

template <typename B>
QueryTable<B>::QueryTable(const QueryTable<B> &src) : order_by_(src.getSortOrder()), schema_(src.getSchema()), tuple_size_(src.tuple_size_) {
    size_t alloc_size = tuple_size_ * src.getTupleCount();
    tuple_data_.resize(alloc_size);
    memcpy(tuple_data_.data(), src.tuple_data_.data(), alloc_size);


}




template <typename B>
bool QueryTable<B>::isEncrypted() const {
    return std::is_same_v<B, emp::Bit>;
}



template <typename B>
std::unique_ptr<PlainTable> QueryTable<B>::reveal(int empParty) const  {
   return revealTable(*this, empParty);

}



// iterate over all tuples and produce one long bit array for encrypting/decrypting in emp
// only tested in PUBLIC or XOR mode
template <typename B>
vector<int8_t> QueryTable<B>::serialize() const {
    // copy out our payload
    return vector<int8_t>(tuple_data_);

}

std::ostream &vaultdb::operator<<(std::ostream &os, const PlainTable &table) {
        os << *(table.getSchema()) << " isEncrypted? " << table.isEncrypted() <<  " order by: " << DataUtilities::printSortDefinition(table.getSortOrder()) << endl;

        for(uint32_t i = 0; i < table.getTupleCount(); ++i) {
            PlainTuple tuple = table[i];

            os << tuple;
            const bool is_dummy = tuple.getDummyTag();
            if(!is_dummy)
                os << endl;

        }

        return os;

}

std::ostream &vaultdb::operator<<(std::ostream &os, const SecureTable &table) {


    os << table.getSchema() << " isEncrypted? " << table.isEncrypted() <<  " order by: " << DataUtilities::printSortDefinition(table.getSortOrder()) << endl;

    for(uint32_t i = 0; i < table.getTupleCount(); ++i) {
        os << table[i] << endl;
    }

    return os;

}


template <typename B>
string QueryTable<B>::toString(const bool & showDummies) const {

    std::ostringstream os;

    if(!showDummies) {
        os << *this;
        return os.str();
    }

    // show dummies case
    os << *getSchema() << " isEncrypted? " << isEncrypted() <<  " order by: " << DataUtilities::printSortDefinition(getSortOrder()) << std::endl;


    for(uint32_t i = 0; i < getTupleCount(); ++i) {
        PlainTuple tuple = getPlainTuple(i);
        os << tuple.toString(showDummies) << std::endl;
    }

    return os.str();

}

template <typename B>
QueryTable<B> & QueryTable<B>::operator=(const QueryTable<B> & src) {
    if(&src == this)
        return *this;

    setSchema(*src.getSchema());

    tuple_data_.resize(src.tuple_data_.size());
    memcpy(tuple_data_.data(), src.tuple_data_.data(), tuple_data_.size());
    return *this;
}

template <typename B>
void QueryTable<B>::putTuple(const int &idx, const QueryTuple<B> & tuple) {
    assert(*tuple.getSchema() == *schema_);

    size_t tuple_offset = idx * tuple_size_;
    memcpy(tuple_data_.data() + tuple_offset, tuple.getData(), tuple_size_);


}




template <typename B>
bool QueryTable<B>::operator==(const QueryTable<B> &other) const {

    assert(!isEncrypted()); // reveal this for tables in the clear

    if(*getSchema() != *other.getSchema()) {
        std::cout << "Failed to match on schema: \n" << *getSchema() << "\n  == \n" << *other.getSchema() << std::endl;
        return false;
    }

    if(this->getSortOrder() != other.getSortOrder()) {
        std::cout << "Failed to match on sort order expected="  << DataUtilities::printSortDefinition(this->getSortOrder())
                  << "observed=" << DataUtilities::printSortDefinition(other.getSortOrder()) <<  std::endl;
        return false;
    }

    if(this->getTupleCount() != other.getTupleCount()) {
        std::cout << "Failed to match on tuple count " << this->getTupleCount() << " vs " << other.getTupleCount() << std::endl;
        return false;
    }


    size_t tuple_offset = 0;
    std::shared_ptr<QuerySchema> q = schema_;

    for(uint32_t i = 0; i < getTupleCount(); ++i) {
        PlainTuple this_tuple(q, (int8_t *) (tuple_data_.data() + tuple_offset));
        PlainTuple other_tuple(q, (int8_t *) (other.tuple_data_.data() + tuple_offset));

        if(this_tuple != other_tuple) {
            std::cout << "Comparing on idx " << i << " with " << this_tuple.toString(true) << "\n          !=            " << other_tuple.toString(true) << endl;
            std::cout << "    Failed to match!" << std::endl;
           return false;
        }

        tuple_offset += tuple_size_;
    }


    return true;
}



// BOB inputs empty table for ZK
template<typename B>
std::shared_ptr<SecureTable> QueryTable<B>::secretShare(const PlainTable & input, emp::NetIO *netio, const int & party)  {

    size_t tuple_cnt = input.getTupleCount();


    if (party == ALICE) {
        netio->send_data(&tuple_cnt, 4);
        //netio->flush();
    } else if (party == BOB) {
        netio->recv_data(&tuple_cnt, 4);
        //netio->flush();
    }



    QuerySchema dst_schema = QuerySchema::toSecure(*input.getSchema());

    std::shared_ptr<SecureTable> dst_table(new SecureTable(tuple_cnt, dst_schema, input.getSortOrder()));

    // preserve sort order - reverse input order for latter half to do bitonic merge
        if (party == emp::ALICE) {
            secret_share_send(emp::ALICE, input, *dst_table, 0, false);
        } else { // bob
            secret_share_recv(tuple_cnt, emp::ALICE, *dst_table, 0, false);
        }

    return dst_table;

}



template<typename B>
std::shared_ptr<SecureTable>
QueryTable<B>::secret_share_send_table(const std::shared_ptr<PlainTable> &input, const int &sharing_party) {
    QuerySchema dst_schema = QuerySchema::toSecure(*input->getSchema());

    std::shared_ptr<SecureTable> shared = std::make_shared<SecureTable>(input->getTupleCount(), dst_schema, input->getSortOrder());
    secret_share_send(sharing_party, *input, *shared, 0, false);
    return shared;
}

template<typename B>
std::shared_ptr<SecureTable>
QueryTable<B>::secret_share_recv_table(const size_t &tuple_cnt, const QuerySchema &src_schema,
                                       const SortDefinition &sort_definition, const int &sharing_party)
{
    QuerySchema dst_schema = QuerySchema::toSecure(src_schema);
    std::shared_ptr<SecureTable> shared = std::make_shared<SecureTable>(tuple_cnt, dst_schema, sort_definition);
    secret_share_recv(tuple_cnt, sharing_party, *shared, 0, false);

    return shared;
}


// use this for acting as a data sharing party in the PDF
// generates alice and bob's shares and returns the pair
template<typename B>
SecretShares QueryTable<B>::generateSecretShares() const {
    vector<int8_t> serialized = this->serialize();
    int8_t *secrets = serialized.data();
    size_t sharesSize = serialized.size();

    vector<int8_t> aliceShares, bobShares;
    aliceShares.resize(sharesSize);
    bobShares.resize(sharesSize);
    int8_t *alice = aliceShares.data();
    int8_t *bob = bobShares.data();

    emp::PRG prg; // initializes with a random seed


    prg.random_data(alice, sharesSize);

    for(size_t i = 0; i < sharesSize; ++i) {
        bob[i] = alice[i] ^ secrets[i];
    }


    return SecretShares(aliceShares, bobShares);
}


template <typename B>
void QueryTable<B>::setSortOrder(const SortDefinition &sortOrder) {
    order_by_ = sortOrder;

}

template <typename B>
SortDefinition QueryTable<B>::getSortOrder() const {
    return order_by_;
}

template <typename B>
std::shared_ptr<PlainTable> QueryTable<B>::deserialize(const QuerySchema &schema, const vector<int8_t> & tableBits) {

    uint32_t tableSize = tableBits.size(); // in bytes
    uint32_t tupleSize = schema.size() / 8; // in bytes
    uint32_t tupleCount = tableSize / tupleSize;
    SortDefinition emptySortDefinition;

    std::shared_ptr<PlainTable > result(new PlainTable(tupleCount, schema));
    result->tuple_data_ = tableBits;

    return result;


}

template<typename B>
std::shared_ptr<SecureTable>
QueryTable<B>::deserialize(const QuerySchema &schema, vector<Bit> &table_bits) {
    QuerySchema encrypted_schema = QuerySchema::toSecure(schema);
    uint32_t table_size = table_bits.size(); // in bits
    uint32_t tuple_size = encrypted_schema.size(); // in bits
    assert(table_size % tuple_size == 0);
    uint32_t tuple_cnt = table_size / tuple_size;

    std::shared_ptr<SecureTable> result(new SecureTable(tuple_cnt, encrypted_schema));
    assert(result->tuple_data_.size() / sizeof(emp::block) == table_bits.size());

    memcpy(result->tuple_data_.data(), table_bits.data(), table_size * sizeof(emp::block));

    return result;

}

template<typename B>
void QueryTable<B>::resize(const size_t &tupleCount) {
    tuple_data_.resize(tupleCount * tuple_size_);
}

template<typename B>
QueryTuple<B> QueryTable<B>::getTuple(int idx)   {
    size_t offset_bytes = tuple_size_ * idx;
    int8_t *dst = tuple_data_.data();
    dst += offset_bytes;
    return QueryTuple<B>(schema_,  dst);
}

template<typename B>
const QueryTuple<B> QueryTable<B>::getImmutableTuple(int idx)  const  {
    int8_t *write_ptr = (int8_t *) (tuple_data_.data() + tuple_size_ * idx);
    return QueryTuple<B>(schema_,  write_ptr);
}

template<typename B>
size_t QueryTable<B>::getTrueTupleCount() const {
    assert(!isEncrypted());
    size_t count = 0;

    for(size_t i = 0; i < tuple_data_.size() / tuple_size_; ++i) {
        PlainTuple p = getPlainTuple(i);
        if(!p.getDummyTag())
            ++count;
    }

    return count;
}

template<typename B>
PlainTuple QueryTable<B>::getPlainTuple(size_t idx) const {
    assert(!isEncrypted()); // B == bool
    size_t tuple_offset = tuple_size_ * idx;
    std::shared_ptr<QuerySchema> q = schema_;
    int8_t *tuple_pos =  (int8_t *) tuple_data_.data() + tuple_offset;
    return vaultdb::PlainTuple(q,tuple_pos);
}

template<typename B>
QueryTuple<B> QueryTable<B>::operator[](const int &idx) {
    return this->getTuple(idx);
}

template<typename B>
const QueryTuple<B> QueryTable<B>::operator[](const int &idx) const {
    int8_t *read_ptr = (int8_t *) (tuple_data_.data() + tuple_size_ * idx);

    const QueryTuple<B> tuple(schema_, read_ptr);
    return tuple;
}

template<typename B>
std::unique_ptr<PlainTable> QueryTable<B>::revealTable(const SecureTable &table, const int & party) {
    uint32_t tuple_cnt = table.getTupleCount();


    QuerySchema dst_schema = QuerySchema::toPlain(*table.getSchema());

    std::unique_ptr<PlainTable > dst_table(new PlainTable(tuple_cnt, dst_schema, table.getSortOrder()));


    for(uint32_t i = 0; i < tuple_cnt; ++i)  {
        const SecureTuple tuple = table.getImmutableTuple(i);
        PlainTuple dst_tuple = tuple.reveal(party);
        dst_table->putTuple(i, dst_tuple);
    }

    return dst_table;
}

template<typename B>
std::unique_ptr<PlainTable> QueryTable<B>::revealTable(const PlainTable & table, const int & party) {
    return std::make_unique<PlainTable>(table);
}

template<typename B>
void
QueryTable<B>::secret_share_send(const int &party, const PlainTable & src_table, SecureTable & dst_table, const int &write_offset,
                                 const bool &reverse_read_order)  {

    int32_t cursor = (int32_t) write_offset;

    if(reverse_read_order) {
        for(int32_t i = src_table.getTupleCount() - 1; i >= 0; --i) {
            SecureTuple dst_tuple = dst_table.getTuple(cursor);
            PlainTuple src_tuple = src_table.getPlainTuple(i);
            FieldUtilities::secret_share_send(src_tuple, dst_tuple, party);
            ++cursor;
        }

        return;

    }

    // else
    for(size_t i = 0; i < src_table.getTupleCount(); ++i) {
        SecureTuple dst_tuple = dst_table.getTuple(cursor);
        PlainTuple src_tuple = src_table.getPlainTuple(i);
        FieldUtilities::secret_share_send(src_tuple, dst_tuple, party);
        ++cursor;
    }

}

template<typename B>
void QueryTable<B>::secret_share_recv(const size_t &tuple_count, const int &dst_party,
                                      SecureTable &dst_table, const size_t &write_offset,
                                      const bool &reverse_read_order)  {

    int32_t cursor = (int32_t) write_offset;

    if(reverse_read_order) {

        for(int32_t i = tuple_count - 1; i >= 0; --i) {
            SecureTuple dst_tuple = dst_table.getTuple(cursor);
            FieldUtilities::secret_share_recv(dst_tuple, dst_party);
            ++cursor;
        }



        return;
    }

    // else
    for(size_t i = 0; i < tuple_count; ++i) {
        SecureTuple dst_tuple = dst_table.getTuple(cursor);
        FieldUtilities::secret_share_recv(dst_tuple, dst_party);
        ++cursor;
    }



}

template<typename B>
std::string QueryTable<B>::toString(const size_t &limit, const bool &show_dummies) const {
    std::ostringstream os;
    size_t tuples_printed = 0;
    size_t cursor = 0;
    size_t tuple_cnt = getTupleCount();

    assert(!isEncrypted());

    os << *getSchema() <<  " order by: " << DataUtilities::printSortDefinition(getSortOrder()) << std::endl;
    while(cursor < tuple_cnt && tuples_printed < limit) {
        PlainTuple tuple = getPlainTuple(cursor);
        if(show_dummies  // print unconditionally
            || !tuple.getDummyTag()) {
            os << cursor << ": " << tuple.toString(show_dummies) << std::endl;
            ++tuples_printed;
        }
        ++cursor;
    }

    return os.str();

}


template class vaultdb::QueryTable<bool>;
template class vaultdb::QueryTable<emp::Bit>;
