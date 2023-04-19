#include "connector_expression_nodes.h"

using namespace vaultdb;

template<typename B>
NotNode<B>::NotNode(std::shared_ptr<ExpressionNode<B>> input): ExpressionNode<B>(input) {}

template<typename B>
Field<B> NotNode<B>::call(const QueryTuple<B> &target) const {
    Field<B> input = ExpressionNode<B>::lhs_->call(target);
    return !input;
}

template<typename B>
ExpressionKind NotNode<B>::kind() const {
    return ExpressionKind::NOT;
}

template<typename B>
void NotNode<B>::accept(ExpressionVisitor<B> *visitor) {
    visitor->visit(*this);
}


template<typename B>
AndNode<B>::AndNode(std::shared_ptr<ExpressionNode<B>> &lhs, std::shared_ptr<ExpressionNode<B>> &rhs) : ExpressionNode<B>(lhs, rhs){ }

template<typename B>
Field<B> AndNode<B>::call(const QueryTuple<B> &target) const {
    Field<B> lhs = ExpressionNode<B>::lhs_->call(target);
    Field<B> rhs = ExpressionNode<B>::rhs_->call(target);

    B result = lhs && rhs;
    return FieldUtilities::getBoolField(result);
}

template<typename B>
ExpressionKind AndNode<B>::kind() const {
    return ExpressionKind::AND;
}

template<typename B>
void AndNode<B>::accept(ExpressionVisitor<B> *visitor) {
    visitor->visit(*this);
}

template<typename B>
OrNode<B>::OrNode(std::shared_ptr<ExpressionNode<B>> &lhs, std::shared_ptr<ExpressionNode<B>> &rhs) : ExpressionNode<B>(lhs, rhs) { }

template<typename B>
Field<B> OrNode<B>::call(const QueryTuple<B> &target) const {
    Field<B> lhs = ExpressionNode<B>::lhs_->call(target);
    Field<B> rhs = ExpressionNode<B>::rhs_->call(target);

    B result = lhs || rhs;
    return FieldUtilities::getBoolField(result);

}

template<typename B>
ExpressionKind OrNode<B>::kind() const {
    return ExpressionKind::OR;
}

template<typename B>
void OrNode<B>::accept(ExpressionVisitor<B> *visitor) {
    visitor->visit(*this);
}

template class vaultdb::NotNode<bool>;
template class vaultdb::NotNode<emp::Bit>;


template class vaultdb::AndNode<bool>;
template class vaultdb::AndNode<emp::Bit>;


template class vaultdb::OrNode<bool>;
template class vaultdb::OrNode<emp::Bit>;
