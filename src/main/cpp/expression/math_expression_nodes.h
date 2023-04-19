#ifndef _MATH_EXPRESSION_NODE_H
#define _MATH_EXPRESSION_NODE_H

#include "expression_node.h"
#include <query_table/field/field_factory.h>

namespace vaultdb {

    template<typename B>
    class PlusNode : public ExpressionNode<B>  {
    public:
        PlusNode(std::shared_ptr<ExpressionNode<B> > & lhs, std::shared_ptr<ExpressionNode<B> > & rhs);
        ~PlusNode() = default;
        Field<B> call(const QueryTuple<B> & target) const override;
        void accept(ExpressionVisitor<B> *visitor) override;

        ExpressionKind kind() const override;
    };

    template<typename B>
    class MinusNode : public  ExpressionNode<B>  {
    public:
        MinusNode(std::shared_ptr<ExpressionNode<B> > & lhs, std::shared_ptr<ExpressionNode<B> > & rhs);
        ~MinusNode() = default;
        Field<B> call(const QueryTuple<B> & target) const override;
        void accept(ExpressionVisitor<B> *visitor) override;

        ExpressionKind kind() const override;
    };



    template<typename B>
    class TimesNode : public ExpressionNode<B>  {
    public:
        TimesNode(std::shared_ptr<ExpressionNode<B> > & lhs, std::shared_ptr<ExpressionNode<B> > & rhs);
        ~TimesNode() = default;
        Field<B> call(const QueryTuple<B> & target) const override;
        void accept(ExpressionVisitor<B> *visitor) override;

        ExpressionKind kind() const override;
    };

    template<typename B>
    class DivideNode : public  ExpressionNode<B>  {
    public:
        DivideNode(std::shared_ptr<ExpressionNode<B> > & lhs, std::shared_ptr<ExpressionNode<B> > & rhs);
        ~DivideNode() = default;
        Field<B> call(const QueryTuple<B> & target) const override;
        void accept(ExpressionVisitor<B> *visitor) override;

        ExpressionKind kind() const override;
    };


    template<typename B>
    class ModulusNode : public  ExpressionNode<B>  {
    public:
        ModulusNode(std::shared_ptr<ExpressionNode<B> > & lhs, std::shared_ptr<ExpressionNode<B> > & rhs);
        ~ModulusNode() = default;
        Field<B> call(const QueryTuple<B> & target) const override;
        void accept(ExpressionVisitor<B> *visitor) override;

        ExpressionKind kind() const override;
    };


}


#endif // _MATH_EXPRESSION_NODE_H
