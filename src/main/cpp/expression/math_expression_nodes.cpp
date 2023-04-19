#include "math_expression_nodes.h"

using namespace vaultdb;

template<typename B>
PlusNode<B>::PlusNode(std::shared_ptr <ExpressionNode<B> > &lhs, std::shared_ptr<ExpressionNode<B> > &rhs) : ExpressionNode<B>(lhs, rhs) { }

template<typename B>
Field<B> PlusNode<B>::call(const QueryTuple<B> &target) const {
    Field<B> lhs = ExpressionNode<B>::lhs_->call(target);
    Field<B> rhs = ExpressionNode<B>::rhs_->call(target);
    return lhs + rhs;
}

template<typename B>
ExpressionKind PlusNode<B>::kind() const {
    return ExpressionKind::PLUS;
}

template<typename B>
void PlusNode<B>::accept(ExpressionVisitor<B> *visitor) {
    visitor->visit(*this);
}


template<typename B>
MinusNode<B>::MinusNode(std::shared_ptr<ExpressionNode<B>> &lhs, std::shared_ptr<ExpressionNode<B>> &rhs) : ExpressionNode<B>(lhs, rhs) { }

template<typename B>
Field<B> MinusNode<B>::call(const QueryTuple<B> &target) const {
    Field<B> lhs = ExpressionNode<B>::lhs_->call(target);
    Field<B> rhs = ExpressionNode<B>::rhs_->call(target);
    return lhs - rhs;
}

template<typename B>
ExpressionKind MinusNode<B>::kind() const {
    return ExpressionKind::MINUS;
}

template<typename B>
void MinusNode<B>::accept(ExpressionVisitor<B> *visitor) {
    visitor->visit(*this);
}


template<typename B>
TimesNode<B>::TimesNode(std::shared_ptr<ExpressionNode<B>> &lhs, std::shared_ptr<ExpressionNode<B>> &rhs) : ExpressionNode<B>(lhs, rhs) { }

template<typename B>
Field<B> TimesNode<B>::call(const QueryTuple<B> &target) const {
    Field<B> lhs = ExpressionNode<B>::lhs_->call(target);
    Field<B> rhs = ExpressionNode<B>::rhs_->call(target);
    return lhs * rhs;
}

template<typename B>
ExpressionKind TimesNode<B>::kind() const {
    return ExpressionKind::TIMES;
}

template<typename B>
void TimesNode<B>::accept(ExpressionVisitor<B> *visitor) {
    visitor->visit(*this);
}


template<typename B>
DivideNode<B>::DivideNode(std::shared_ptr<ExpressionNode<B>> &lhs, std::shared_ptr<ExpressionNode<B>> &rhs) : ExpressionNode<B>(lhs, rhs) { }

template<typename B>
Field<B> DivideNode<B>::call(const QueryTuple<B> &target) const {
    Field<B> lhs = ExpressionNode<B>::lhs_->call(target);
    Field<B> rhs = ExpressionNode<B>::rhs_->call(target);
    B condition = rhs == FieldFactory<B>::getZero(rhs.getType());
    Field<B> newRhs = Field<B>::If(condition,FieldFactory<B>::getOne(rhs.getType()), rhs);
    Field<B> temp = lhs / newRhs;
    return Field<B>::If(condition,FieldFactory<B>::getZero(rhs.getType()),temp);
}

template<typename B>
ExpressionKind DivideNode<B>::kind() const {
    return ExpressionKind::DIVIDE;
}

template<typename B>
void DivideNode<B>::accept(ExpressionVisitor<B> *visitor) {
    visitor->visit(*this);
}


template<typename B>
ModulusNode<B>::ModulusNode(std::shared_ptr<ExpressionNode<B>> &lhs, std::shared_ptr<ExpressionNode<B>> &rhs) : ExpressionNode<B>(lhs, rhs) { }

template<typename B>
Field<B> ModulusNode<B>::call(const QueryTuple<B> &target) const {
    Field<B> lhs = ExpressionNode<B>::lhs_->call(target);
    Field<B> rhs = ExpressionNode<B>::rhs_->call(target);
    return lhs / rhs;
}

template<typename B>
ExpressionKind ModulusNode<B>::kind() const {
    return ExpressionKind::MOD;
}

template<typename B>
void ModulusNode<B>::accept(ExpressionVisitor<B> *visitor) {
    visitor->visit(*this);
}


template class vaultdb::PlusNode<bool>;
template class vaultdb::PlusNode<emp::Bit>;

template class vaultdb::MinusNode<bool>;
template class vaultdb::MinusNode<emp::Bit>;

template class vaultdb::TimesNode<bool>;
template class vaultdb::TimesNode<emp::Bit>;

template class vaultdb::DivideNode<bool>;
template class vaultdb::DivideNode<emp::Bit>;

template class vaultdb::ModulusNode<bool>;
template class vaultdb::ModulusNode<emp::Bit>;
