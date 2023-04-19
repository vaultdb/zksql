#include "comparator_expression_nodes.h"
#include <util/field_utilities.h>

using namespace vaultdb;

template<typename B>
EqualNode<B>::EqualNode(std::shared_ptr<ExpressionNode<B>> lhs, std::shared_ptr<ExpressionNode<B>> rhs)
:ExpressionNode<B>(lhs, rhs) { }

template<typename B>
Field<B> EqualNode<B>::call(const QueryTuple<B> &target) const {
    Field<B> lhs = ExpressionNode<B>::lhs_->call(target);
    Field<B> rhs = ExpressionNode<B>::rhs_->call(target);


    return FieldUtilities::getBoolField(lhs == rhs);
}

template<typename B>
ExpressionKind EqualNode<B>::kind() const {
    return ExpressionKind::EQ;
}

template<typename B>
void EqualNode<B>::accept(ExpressionVisitor<B> *visitor) {
    visitor->visit(*this);
}



template<typename B>
NotEqualNode<B>::NotEqualNode(std::shared_ptr<ExpressionNode<B>> lhs, std::shared_ptr<ExpressionNode<B>> rhs)
:ExpressionNode<B>(lhs, rhs) { }

template<typename B>
Field<B> NotEqualNode<B>::call(const QueryTuple<B> &target) const {
    Field<B> lhs = ExpressionNode<B>::lhs_->call(target);
    Field<B> rhs = ExpressionNode<B>::rhs_->call(target);

    return FieldUtilities::getBoolField(lhs != rhs);
}

template<typename B>
ExpressionKind NotEqualNode<B>::kind() const {
    return ExpressionKind::NEQ;
}

template<typename B>
void NotEqualNode<B>::accept(ExpressionVisitor<B> *visitor) {
    visitor->visit(*this);
}

template<typename B>
LessThanNode<B>::LessThanNode(std::shared_ptr<ExpressionNode<B>> lhs, std::shared_ptr<ExpressionNode<B>> rhs)
        :ExpressionNode<B>(lhs, rhs) {

}

template<typename B>
Field<B> LessThanNode<B>::call(const QueryTuple<B> &target) const {
    Field<B> lhs = ExpressionNode<B>::lhs_->call(target);
    Field<B> rhs = ExpressionNode<B>::rhs_->call(target);

    return FieldUtilities::getBoolField(lhs < rhs);
}

template<typename B>
ExpressionKind LessThanNode<B>::kind() const {
    return ExpressionKind::LT;
}

template<typename B>
void LessThanNode<B>::accept(ExpressionVisitor<B> *visitor) {
    visitor->visit(*this);
}


template<typename B>
GreaterThanNode<B>::GreaterThanNode(std::shared_ptr<ExpressionNode<B>> lhs, std::shared_ptr<ExpressionNode<B>> rhs)
        :ExpressionNode<B>(lhs, rhs) {

}

template<typename B>
Field<B> GreaterThanNode<B>::call(const QueryTuple<B> &target) const {
    Field<B> lhs = ExpressionNode<B>::lhs_->call(target);
    Field<B> rhs = ExpressionNode<B>::rhs_->call(target);

    return FieldUtilities::getBoolField(lhs > rhs);
}

template<typename B>
ExpressionKind GreaterThanNode<B>::kind() const {
    return ExpressionKind::GT;
}

template<typename B>
void GreaterThanNode<B>::accept(ExpressionVisitor<B> *visitor) {
    visitor->visit(*this);
}


template<typename B>
LessThanEqNode<B>::LessThanEqNode(std::shared_ptr<ExpressionNode<B>> lhs, std::shared_ptr<ExpressionNode<B>> rhs)
        :ExpressionNode<B>(lhs, rhs) {

}

template<typename B>
Field<B> LessThanEqNode<B>::call(const QueryTuple<B> &target) const {
    Field<B> lhs = ExpressionNode<B>::lhs_->call(target);
    Field<B> rhs = ExpressionNode<B>::rhs_->call(target);

    return FieldUtilities::getBoolField(lhs <= rhs);
}

template<typename B>
ExpressionKind LessThanEqNode<B>::kind() const {
    return ExpressionKind::LEQ;
}

template<typename B>
void LessThanEqNode<B>::accept(ExpressionVisitor<B> *visitor) {
    visitor->visit(*this);

}

template<typename B>
GreaterThanEqNode<B>::GreaterThanEqNode(std::shared_ptr<ExpressionNode<B>> lhs,
                                        std::shared_ptr<ExpressionNode<B>> rhs) : ExpressionNode<B>(lhs, rhs) {

}

template<typename B>
Field<B> GreaterThanEqNode<B>::call(const QueryTuple<B> &target) const {
    Field<B> lhs = ExpressionNode<B>::lhs_->call(target);
    Field<B> rhs = ExpressionNode<B>::rhs_->call(target);

    return FieldUtilities::getBoolField(lhs >= rhs);
}

template<typename B>
ExpressionKind GreaterThanEqNode<B>::kind() const {
    return ExpressionKind::GEQ;
}

template<typename B>
void GreaterThanEqNode<B>::accept(ExpressionVisitor<B> *visitor) {
    visitor->visit(*this);
}


template class vaultdb::EqualNode<bool>;
template class vaultdb::EqualNode<emp::Bit>;

template class vaultdb::NotEqualNode<bool>;
template class vaultdb::NotEqualNode<emp::Bit>;

template class vaultdb::GreaterThanNode<bool>;
template class vaultdb::GreaterThanNode<emp::Bit>;

template class vaultdb::GreaterThanEqNode<bool>;
template class vaultdb::GreaterThanEqNode<emp::Bit>;


template class vaultdb::LessThanNode<bool>;
template class vaultdb::LessThanNode<emp::Bit>;

template class vaultdb::LessThanEqNode<bool>;
template class vaultdb::LessThanEqNode<emp::Bit>;
