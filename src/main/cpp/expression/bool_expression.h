#ifndef _BOOL_EXPRESSION_H
#define _BOOL_EXPRESSION_H
#include "expression.h"
#include "expression_node.h"

namespace  vaultdb {
    template<typename B>

    // wrapper for ExpressionNode
    class BoolExpression : public Expression<B> {
    public:
        BoolExpression(std::shared_ptr<ExpressionNode<B> > root);

        Field<B> call(const QueryTuple<B> &aTuple) const override;

        B callBoolExpression(const QueryTuple<B> &tuple) const;

        // for joins
        B call(const QueryTuple<B> & lhs, const QueryTuple<B> & rhs, const QuerySchema & output_schema) const;

        ExpressionKind kind() const override { return root_->kind(); }


        ~BoolExpression() = default;

        string toString() const override;

        std::shared_ptr<ExpressionNode<B> > root_;
    };

    std::ostream &operator<<(std::ostream &os,  BoolExpression<bool> &expression);
    std::ostream &operator<<(std::ostream &os,  BoolExpression<emp::Bit> &expression);


}

#endif //_BOOL_EXPRESSION_H
