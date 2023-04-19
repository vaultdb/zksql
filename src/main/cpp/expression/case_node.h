#ifndef _CASE_NODE_H
#define _CASE_NODE_H

#include <expression/expression_node.h>


namespace  vaultdb {
    template<typename B>
    class CaseNode :  public ExpressionNode<B>  {
    public:
        CaseNode(BoolExpression<B> & conditional, std::shared_ptr<ExpressionNode<B> > & lhs, std::shared_ptr<ExpressionNode<B> > & rhs);
        ~CaseNode() = default;
        Field<B> call(const QueryTuple<B> & target) const override;
        void accept(ExpressionVisitor<B> *visitor) override;

        ExpressionKind kind() const override;

        BoolExpression<B> conditional_;
    };

}

#endif //_CASE_NODE_H
