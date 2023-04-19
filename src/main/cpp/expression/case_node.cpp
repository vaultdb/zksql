#include "case_node.h"

using namespace vaultdb;

template<typename B>
CaseNode<B>::CaseNode(BoolExpression<B> & conditional, std::shared_ptr<ExpressionNode<B> > & lhs, std::shared_ptr<ExpressionNode<B> > & rhs) :
ExpressionNode<B>(lhs, rhs), conditional_(conditional){ }


template<typename B>
Field<B> CaseNode<B>::call(const QueryTuple<B> &target) const {
    B result = conditional_.callBoolExpression(target);
    Field<B> lhs_eval = ExpressionNode<B>::lhs_->call(target);
    Field<B> rhs_eval = ExpressionNode<B>::rhs_->call(target);

    return Field<B>::If(result, lhs_eval, rhs_eval);
}

template<typename B>
void CaseNode<B>::accept(ExpressionVisitor<B> *visitor) {
    visitor->visit(*this);

}
template<typename B>
ExpressionKind CaseNode<B>::kind() const {
    return ExpressionKind::CASE;
}



template class vaultdb::CaseNode<bool>;
template class vaultdb::CaseNode<emp::Bit>;
