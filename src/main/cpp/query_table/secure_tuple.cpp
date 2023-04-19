#include "secure_tuple.h"
#include "field/field_factory.h"

using namespace vaultdb;

std::ostream &vaultdb::operator<<(std::ostream &strm,  const SecureTuple &aTuple) {

    strm << aTuple.toString(false);

    return strm;


}

QueryTuple<emp::Bit>::QueryTuple(std::shared_ptr<QuerySchema> &query_schema, emp::Bit *src) : fields_(src), query_schema_(query_schema) {
    assert(fields_ != nullptr);

}

QueryTuple<emp::Bit>::QueryTuple(std::shared_ptr<QuerySchema> &query_schema, int8_t *src) : fields_((emp::Bit *) src), query_schema_(query_schema) {
    assert(fields_ != nullptr);

}

QueryTuple<emp::Bit>::QueryTuple(const std::shared_ptr<QuerySchema> &query_schema, const int8_t *src) :  query_schema_(query_schema) {
    int8_t *tmp = const_cast<int8_t *>(src);
    fields_ = (emp::Bit *) tmp;
    assert(fields_ != nullptr);

}


SecureField QueryTuple<emp::Bit>::getField(const int &ordinal)  {
    size_t field_offset = query_schema_->getFieldOffset(ordinal);
    const emp::Bit *read_ptr = fields_ + field_offset;

    QueryFieldDesc fieldDesc = query_schema_->getField(ordinal);
    return FieldFactory<emp::Bit>::deserialize(fieldDesc.getType(), fieldDesc.getStringLength(),  read_ptr);
}

const SecureField QueryTuple<emp::Bit>::getField(const int &ordinal)  const {
    size_t field_offset = query_schema_->getFieldOffset(ordinal);
    const emp::Bit *read_ptr = fields_ + field_offset;

    QueryFieldDesc fieldDesc = query_schema_->getField(ordinal);
    return  FieldFactory<emp::Bit>::deserialize(fieldDesc.getType(), fieldDesc.getStringLength(),  read_ptr);
}



void QueryTuple<emp::Bit>::setField(const int &idx, const SecureField &f) {
    size_t field_offset = query_schema_->getFieldOffset(idx);
    int8_t *writePos = (int8_t *) (fields_ + field_offset);
    f.serialize(writePos);

}


void QueryTuple<emp::Bit>::setDummyTag(const Bit &d) {
    const emp::Bit *dst = fields_ + query_schema_->getFieldOffset(-1);
    std::memcpy((int8_t *) dst, (int8_t *) &(d.bit), sizeof(emp::block));
}


QueryTuple<bool>
QueryTuple<emp::Bit>::reveal(const int &empParty, std::shared_ptr<QuerySchema> & dst_schema, int8_t *dst) const {
    QueryTuple<bool> dst_tuple(dst_schema, dst);

    for(size_t i = 0; i < dst_schema->getFieldCount(); ++i) {
        SecureField src = getField(i);
        PlainField revealed = src.reveal(empParty);
        dst_tuple.setField(i, revealed);
    }

    emp::Bit dummy_tag = getDummyTag();
    bool plain_dummy_tag = dummy_tag.reveal(empParty);
    dst_tuple.setDummyTag(plain_dummy_tag);

    return dst_tuple;

}

// self-managed tuple
PlainTuple QueryTuple<emp::Bit>::reveal(const int &empParty) const {
    QuerySchema plain_schema = QuerySchema::toPlain(*query_schema_);
    PlainTuple dst_tuple(plain_schema);


    for(size_t i = 0; i < plain_schema.getFieldCount(); ++i) {
        SecureField src = getField(i);
        PlainField revealed = src.reveal(empParty);
        dst_tuple.setField(i, revealed);
    }

    emp::Bit dummy_tag = getDummyTag();
    bool plain_dummy_tag = dummy_tag.reveal(empParty);
    dst_tuple.setDummyTag(plain_dummy_tag);

    return dst_tuple;

}


void QueryTuple<emp::Bit>::compareSwap(const Bit &cmp, SecureTuple  & lhs, SecureTuple  & rhs) {
    size_t tuple_size = lhs.getSchema()->size(); // size in bytes

    emp::Integer lhs_payload(tuple_size, 0, emp::PUBLIC);
    emp::Integer rhs_payload(tuple_size, 0, emp::PUBLIC);

    memcpy(lhs_payload.bits.data(), lhs.getData(), tuple_size * sizeof(emp::block));
    memcpy(rhs_payload.bits.data(), rhs.getData(), tuple_size * sizeof(emp::block));

    emp::swap(cmp, lhs_payload, rhs_payload);

    memcpy(lhs.getData(), lhs_payload.bits.data(), tuple_size * sizeof(emp::block));
    memcpy(rhs.getData(), rhs_payload.bits.data(), tuple_size * sizeof(emp::block));

}

SecureTuple QueryTuple<emp::Bit>::deserialize(emp::Bit *dst_tuple_bits, std::shared_ptr<QuerySchema> &schema,
                                              const emp::Bit *src_tuple_bits) {
    SecureTuple result(schema, dst_tuple_bits);
    size_t tuple_size = schema->size() * sizeof(emp::block);
    memcpy(dst_tuple_bits, src_tuple_bits, tuple_size);
    return result;

}

std::shared_ptr<QuerySchema> QueryTuple<emp::Bit>::getSchema() const {
    return query_schema_;
}

emp::Bit QueryTuple<emp::Bit>::getDummyTag() const {
    const emp::Bit *src = fields_ + query_schema_->getFieldOffset(-1);
    return emp::Bit(*((emp::Bit *) src));
}

void QueryTuple<emp::Bit>::setSchema(std::shared_ptr<QuerySchema> q) {
    query_schema_ = q;
}

void QueryTuple<emp::Bit>::setDummyTag(const Field<emp::Bit> &d) {
    setDummyTag(d.getValue<emp::Bit>());
}

void QueryTuple<emp::Bit>::setDummyTag(const bool &b) {
    emp::Bit bit(b);
    setDummyTag(bit);
}

QueryTuple<emp::Bit> &QueryTuple<emp::Bit>::operator=(const SecureTuple &other) {
    assert(*this->getSchema() == *(other.getSchema()));
    if(&other == this)
        return *this;

    emp::Bit *dst = this->fields_;
    emp::Bit *src = other.fields_;

    memcpy(dst, src, query_schema_->size() * sizeof(emp::block));

    return *this;

}

size_t QueryTuple<emp::Bit>::getFieldCount() const {
    return query_schema_->getFieldCount();
}

void QueryTuple<emp::Bit>::serialize(int8_t *dst) {
    memcpy(dst, (int8_t *) fields_, query_schema_->size());
}

string QueryTuple<emp::Bit>::toString(const bool &showDummies) const {
    std::stringstream sstream;


    sstream <<   "(" <<  getField(0).toString();

    for (size_t i = 1; i < getFieldCount(); ++i)
        sstream << ", " << getField(i);

    sstream << ")";

    if(showDummies) {
        sstream <<  " (dummy=" << "SECRET BIT" << ")";
    }

    return sstream.str();
}

SecureField QueryTuple<emp::Bit>::operator[](const int32_t &idx)  {
    return getField(idx);
}

const SecureField QueryTuple<emp::Bit>::operator[](const int32_t &idx)  const {
    return getField(idx);
}


emp::Bit QueryTuple<emp::Bit>::operator==(const SecureTuple &other) const {
    emp::Bit matched(true);
    assert(*this->getSchema() == *other.getSchema() );

    for (size_t i = 0; i < query_schema_->getFieldCount(); ++i) {
        auto l = getField(i);
        auto r = other.getField(i);
        matched = matched & (l == r);
    }

    return matched;
}


emp::Bit QueryTuple<emp::Bit>::operator!=(const SecureTuple &other) const {
    return !(*this == other);
}

QueryTuple<emp::Bit>::QueryTuple(const QuerySchema &schema) {
    size_t tuple_bit_cnt = schema.size();
    managed_data_ = std::unique_ptr<emp::Bit[]>(new emp::Bit[tuple_bit_cnt]);

    fields_ = managed_data_.get();
    query_schema_ = std::make_shared<QuerySchema>(schema);
}

QueryTuple<emp::Bit>::QueryTuple(const QueryTuple & src) {
        query_schema_ = std::shared_ptr<QuerySchema>(new QuerySchema(*(src.getSchema())));
        if(src.hasManagedStorage()) { // allocate storage for this copy
            size_t tuple_bit_cnt = query_schema_->size();
            managed_data_ = std::unique_ptr<emp::Bit[]>(new emp::Bit[tuple_bit_cnt]);
            fields_ = managed_data_.get();
        }

}

void QueryTuple<emp::Bit>::writeSubset(const SecureTuple &src_tuple, const SecureTuple &dst_tuple, uint32_t src_start_idx,
                                       uint32_t src_attr_cnt, uint32_t dst_start_idx) {

    size_t src_field_offset = src_tuple.getSchema()->getFieldOffset(src_start_idx);
    size_t dst_field_offset = dst_tuple.getSchema()->getFieldOffset(dst_start_idx);

    size_t write_size = 0;
    for(uint32_t i = src_start_idx; i < src_start_idx + src_attr_cnt; ++i) {
        write_size += src_tuple.getSchema()->getField(i).size();
    }

    assert(dst_field_offset + write_size <= dst_tuple.query_schema_->size());

    emp::Bit *read_pos = src_tuple.fields_ + src_field_offset;
    emp::Bit *write_pos = dst_tuple.fields_ + dst_field_offset;

    memcpy(write_pos, read_pos, write_size * sizeof(emp::Bit));

}

void QueryTuple<emp::Bit>::clear(const Bit &zeroOutData) {
    // conditional write
    Bit *cursor = fields_;
    Bit zero(false);

    for(size_t i = 0; i <  query_schema_->size(); ++i) {
        *cursor = emp::If(zeroOutData, zero, fields_[i]);
        ++cursor;
    }

    Bit dummy_tag = emp::If(zeroOutData, Bit(true), getDummyTag());
    setDummyTag(dummy_tag);


}






