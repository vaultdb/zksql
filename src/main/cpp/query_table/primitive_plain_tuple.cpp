#include "primitive_plain_tuple.h"

using namespace vaultdb;

PrimitivePlainTuple::PrimitivePlainTuple(const PlainTuple &src)  : schema_(src.getSchema()){
    dummy_tag_ = src.getDummyTag();
    size_t field_cnt = src.getFieldCount();
    payload_.resize(field_cnt);

    for(size_t i = 0; i  < field_cnt; ++i) {
        payload_[i] = src.getField(i);
    }

}


PlainField PrimitivePlainTuple::getField(const size_t &i) const {
    assert(i < payload_.size());
    return payload_.at(i);
}

PlainTuple PrimitivePlainTuple::getTuple() const {
    PlainTuple  dst(*schema_);
    writeToTuple(dst);
    return dst;
}

void PrimitivePlainTuple::writeToTuple(PlainTuple &dst) const {
    for(size_t i = 0; i < schema_->getFieldCount(); ++i) {
        dst.setField(i, payload_.at(i));
    }

    dst.setDummyTag(dummy_tag_);
}

bool PrimitivePlainTuple::getDummyTag() const {
    return dummy_tag_;
}


bool PrimitivePlainTuple::operator==(const PrimitivePlainTuple &other) const {
    return this->getTuple() == other.getTuple();
}
