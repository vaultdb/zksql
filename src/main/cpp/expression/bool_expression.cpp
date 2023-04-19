#include "bool_expression.h"
#include <util/field_utilities.h>
#include <query_table/secure_tuple.h>
#include <query_table/plain_tuple.h>

using namespace vaultdb;

std::ostream &vaultdb::operator<<(std::ostream &os, BoolExpression<bool> &expression) {
    os << *(expression.root_);
    return os;
}

std::ostream &vaultdb::operator<<(std::ostream &os, BoolExpression<emp::Bit> &expression) {
    os << *(expression.root_);
    return os;
}

template<typename B>
Field<B> BoolExpression<B>::call(const QueryTuple<B> &aTuple) const {
    return root_->call(aTuple);
}

template<typename B>
B BoolExpression<B>::callBoolExpression(const QueryTuple<B> &tuple) const {
    Field<B> output = root_->call(tuple);
    return FieldUtilities::getBoolPrimitive(output);
}

template<typename B>
BoolExpression<B>::BoolExpression(std::shared_ptr<ExpressionNode<B>> root) : Expression<B>(), root_(root){
    Expression<B>::alias_ = "predicate";
    Expression<B>::type_ = (std::is_same_v<B, bool>) ? FieldType::BOOL : FieldType::SECURE_BOOL;

}

template<typename B>
B BoolExpression<B>::call(const QueryTuple<B> &lhs, const QueryTuple<B> &rhs, const QuerySchema & output_schema) const {

        QueryTuple<B> tmp(output_schema); // create a tuple with self-managed storage

        size_t lhs_attribute_cnt = lhs.getSchema()->getFieldCount();
        size_t rhs_attribute_cnt = rhs.getSchema()->getFieldCount();

        QueryTuple<B>::writeSubset(lhs, tmp, 0, lhs_attribute_cnt, 0);
        QueryTuple<B>::writeSubset(rhs, tmp, 0, rhs_attribute_cnt, lhs_attribute_cnt);

        
    return callBoolExpression(tmp);
}

template<typename B>
string BoolExpression<B>::toString() const {
    return root_->toString() + " " +  TypeUtilities::getTypeString(Expression<B>::type_);
}


template class vaultdb::BoolExpression<bool>;
template class vaultdb::BoolExpression<emp::Bit>;


