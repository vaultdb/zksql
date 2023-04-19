#ifndef _CONNECTOR_EXPRESSION_NODES_H
#define _CONNECTOR_EXPRESSION_NODES_H

#include <util/field_utilities.h>
#include "expression_node.h"


namespace vaultdb {

    template<typename B>
    class  NotNode : public ExpressionNode<B> {
    public:
        NotNode( std::shared_ptr<ExpressionNode<B> > input);
        ~NotNode() = default;
        Field<B> call(const QueryTuple<B> & target) const override;
        void accept(ExpressionVisitor<B> *visitor) override;

        ExpressionKind kind() const override;
    };




    template<typename B>
    class AndNode : public ExpressionNode<B>  {
    public:

        AndNode(std::shared_ptr<ExpressionNode<B> > & lhs, std::shared_ptr<ExpressionNode<B> > & rhs);
        ~AndNode() = default;
        Field<B> call(const QueryTuple<B> & target) const override;
        void accept(ExpressionVisitor<B> *visitor) override;

        ExpressionKind kind() const override;
    };



    template<typename B>
    class OrNode : public  ExpressionNode<B>  {
    public:
        OrNode(std::shared_ptr<ExpressionNode<B> > & lhs, std::shared_ptr<ExpressionNode<B> > & rhs);
        ~OrNode() = default;
        Field<B> call(const QueryTuple<B> & target) const override;
        void accept(ExpressionVisitor<B> *visitor) override;

        ExpressionKind kind() const override;
    };



}

#endif //_CONNECTOR_EXPRESSION_NODES_H
