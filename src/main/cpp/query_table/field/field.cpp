#include "field.h"
#include "util/utilities.h"
#include "field_factory.h"
#include <query_table/field/visitors.h>
#include <util/field_utilities.h>
#include <util/type_utilities.h>

using namespace vaultdb;


std::ostream &vaultdb::operator<<(std::ostream &os, const Field<bool> &aValue) {
    return os << aValue.toString();
}

std::ostream &vaultdb::operator<<(std::ostream &os, const Field<emp::Bit> &aValue) {
    return os << aValue.toString();
}

template<typename B>
Field<B>::Field() :  payload_(false), type_(FieldType::INVALID), string_length_(0) {
    assert(FieldUtilities::validateTypeAlignment(*this));

}

template<typename B>
Field<B>::Field(const FieldType &fieldType, const Value &val, const int &strLength) : payload_(val), type_(fieldType), string_length_(strLength) {
    assert(FieldUtilities::validateTypeAlignment(*this));
    if(type_ == FieldType::STRING || type_ == FieldType::SECURE_STRING)
        assert(string_length_ > 0);
}


template<typename B>
Field<B>::Field(const Field<B> &field) :  payload_(field.payload_), type_(field.type_), string_length_(field.string_length_) {
    assert(FieldUtilities::validateTypeAlignment(*this));
    if(type_ == FieldType::STRING || type_ == FieldType::SECURE_STRING)
        assert(string_length_ > 0);
}

template<typename B>
Field<B>::Field(const B &value) : string_length_(0) {
    payload_ = value;
    type_ = (std::is_same_v<bool, B>)  ? FieldType::BOOL : FieldType::SECURE_BOOL;

}


template<typename B>
Field<B> &Field<B>::operator=(const Field<B> &other) {

    if(type_ == FieldType::INVALID && other.type_ == FieldType::INVALID) return *this; // nothing to do here
    assert(other.type_ != FieldType::INVALID);

    if(other.type_ == FieldType::STRING)
        assert(other.string_length_ > 0);

    if(&other == this)
        return *this;

    payload_ = other.payload_;
    string_length_ = other.string_length_;
    type_ = other.type_;
    return *this;

}

template<typename B>
FieldType Field<B>::getType() const {
    return type_;
}

template<typename B>
size_t Field<B>::getSize() const {

    // serialization size
    return FieldUtilities::getPhysicalSize(type_, string_length_);
}






template<typename B>
B Field<B>::operator==(const Field<B> &cmp) const {

    if(type_ != cmp.type_) return B(false);
    if(getSize() != cmp.getSize()) return B(false);

    EqualityVisitor visitor;
    visitor.rhs = cmp.payload_;
    return boost::get<B>(boost::apply_visitor(visitor, payload_));
}

// has its own visitor because we have different tolerances for float == vs != to make both a little looser
template<typename B>
B Field<B>::operator!=(const Field &cmp) const {
    B eq = *this == cmp;
    return !eq;
}

// only applicable to bool types
template<typename B>
B Field<B>::operator!() const {
    return !(boost::get<B>(payload_));
}

template<typename B>
B Field<B>::operator>=(const Field &r) const {
    GreaterThanEqVisitor visitor;
    visitor.rhs = r.payload_;
    return boost::get<B>(boost::apply_visitor(visitor, payload_));
}

template<typename B>
B Field<B>::operator<(const Field &rhs) const {
    B geq = (*this >= rhs);
    return  !geq;
}

template<typename B>
B Field<B>::operator<=(const Field &rhs) const {
    return rhs >= *this;
}

template<typename B>
B Field<B>::operator>(const Field &rhs) const {
    return  !(rhs >= *this);
}


template<typename B>
PlainField  Field<B>::reveal(const int &party) const {
    RevealVisitor visitor;
    visitor.party = party;
    visitor.type = type_;

    Value revealed = boost::apply_visitor(visitor, payload_);
    FieldType resType = TypeUtilities::toPlain(type_);

    size_t strLength = 0;
    if (type_ == FieldType::SECURE_STRING) {
        strLength = boost::get<emp::Integer>(payload_).size() / 8;
        assert(strLength > 0);

    }
    return PlainField(resType, revealed, strLength);
}


template<typename B>
std::string Field<B>::toString() const {
    if(type_ == FieldType::INVALID)
        return "INVALID";
    ToStringVisitor visitor;
    visitor.type = type_;
    return boost::apply_visitor(visitor, payload_);

}

template<typename B>
void Field<B>::serialize(int8_t *dst) const {
    assert(dst != nullptr);

    SerializeVisitor visitor;
    visitor.dst_ = dst;
    boost::apply_visitor(visitor, payload_);
}


template<typename B>
SecureField Field<B>::secret_share_send(const PlainField & src, const int & dst_party) {
    Value input = src.payload_;

    SecretShareVisitor visitor;
    visitor.dst_party_ = dst_party;
    visitor.send_ = true;
    visitor.string_length_ = src.string_length_;

    Value result = boost::apply_visitor(visitor, input);

    FieldType resType = TypeUtilities::toSecure(src.type_);


    return SecureField(resType, result, src.string_length_);
}

template<typename B>
SecureField Field<B>::secret_share_recv(const FieldType & type, const size_t & str_length, const int & dst_party) {
    assert(TypeUtilities::isEncrypted(type));

    Value input = FieldFactory<bool>::getZero(TypeUtilities::toPlain(type)).payload_;

    SecretShareVisitor visitor;
    visitor.dst_party_ = dst_party;
    visitor.send_ = false;
    visitor.string_length_ = str_length;

    Value result = boost::apply_visitor(visitor, input);


    return SecureField(type, result, str_length);


}


template<typename B>
void Field<B>::compareAndSwap(const B & choice, Field & lhs, Field & rhs) {
   SwapVisitor visitor;
   visitor.lhs = &lhs.payload_;
   visitor.rhs = &rhs.payload_;
   visitor.choiceBit = choice;

    boost::apply_visitor(visitor, lhs.payload_);  // passing in lhs.payload only to establish type
}

template<typename B>
Field<B> Field<B>::If(const B &choice, const Field &lhs, const Field &rhs) {
    assert(lhs.getType() == rhs.getType());
    assert(lhs.string_length_ == rhs.string_length_);

    SelectVisitor visitor;
    visitor.choiceBit = choice;
    visitor.rhs = rhs.payload_;
    return Field(lhs.getType(), boost::apply_visitor(visitor, lhs.payload_), lhs.string_length_);
}

template<typename B>
Field<B> Field<B>::operator+(const Field &rhs) const {
    assert(type_ == rhs.getType());

    PlusVisitor visitor;
    visitor.rhs = rhs.payload_;

    Value result = boost::apply_visitor(visitor, payload_);
    return Field(type_, boost::apply_visitor(visitor, payload_), string_length_);

}

template<typename B>
Field<B> Field<B>::operator-(const Field &rhs) const {
    assert(type_ == rhs.getType());

    MinusVisitor visitor;
    visitor.rhs = rhs.payload_;

    Value result = boost::apply_visitor(visitor, payload_);
    return Field(type_, boost::apply_visitor(visitor, payload_), string_length_);
}


template<typename B>
Field<B> Field<B>::operator*(const Field &rhs) const {
    assert(type_ == rhs.getType());

    MultiplyVisitor visitor;
    visitor.rhs = rhs.payload_;

    Value result = boost::apply_visitor(visitor, payload_);
    return Field(type_, boost::apply_visitor(visitor, payload_), string_length_);
}

template<typename B>
Field<B> Field<B>::operator/(const Field &rhs) const {
    assert(type_ == rhs.getType());

    DivisionVisitor visitor;
    visitor.rhs = rhs.payload_;

    Value result = boost::apply_visitor(visitor, payload_);
    return Field(type_, boost::apply_visitor(visitor, payload_), string_length_);

}

template<typename B>
Field<B> Field<B>::operator%(const Field &rhs) const {
    assert(type_ == rhs.getType());

    ModulusVisitor visitor;
    visitor.rhs = rhs.payload_;

    Value result = boost::apply_visitor(visitor, payload_);
    return Field(type_, result, string_length_);
}

template<typename B>
B Field<B>::operator&&(const Field &cmp) const {
    assert(type_ == cmp.getType());

    AndVisitor visitor;
    visitor.rhs = cmp.payload_;

    Value result = boost::apply_visitor(visitor, payload_);
    return boost::get<B>(result);
}

template<typename B>
B Field<B>::operator||(const Field &cmp) const {
    assert(type_ == cmp.getType());

    OrVisitor visitor;
    visitor.rhs = cmp.payload_;

    Value result = boost::apply_visitor(visitor, payload_);
    return boost::get<B>(result);

}

// secret sharing as public
template<typename B>
SecureField Field<B>::secret_share() const {
    SecretShareVisitor visitor;
    visitor.send_ = true;
    visitor.string_length_ = string_length_;

    Value result = boost::apply_visitor(visitor, payload_);

    FieldType resType = TypeUtilities::toSecure(type_);


    return SecureField(resType, result, string_length_);

}


template class vaultdb::Field<bool>;
template class vaultdb::Field<emp::Bit>;







