#ifndef _EXPRESSION_NODE_H
#define _EXPRESSION_NODE_H

#include <query_table/query_tuple.h>
#include <expression/expression_kind.h>
#include "visitor/expression_visitor.h"
#include <expression/bool_expression.h>

// building blocks for composing arbitrary expressions

namespace vaultdb {

    // all expressions implement the call method in this
    template<typename B>
    class ExpressionNode {
    public:
        ExpressionNode(std::shared_ptr<ExpressionNode<B> > lhs);
        ExpressionNode(std::shared_ptr<ExpressionNode<B> > lhs, std::shared_ptr<ExpressionNode<B> > rhs);

        virtual Field<B> call(const QueryTuple<B> & target) const = 0;
        virtual ExpressionKind kind() const = 0;
        virtual void accept(ExpressionVisitor<B> * visitor) = 0;
        virtual ~ExpressionNode() {}
        std::string toString();

        std::shared_ptr<ExpressionNode<B> > lhs_;
        std::shared_ptr<ExpressionNode<B> > rhs_;

    };

    std::ostream &operator<<(std::ostream &os,  ExpressionNode<bool> &expression);
    std::ostream &operator<<(std::ostream &os,  ExpressionNode<emp::Bit> &expression);


    // read a field from a tuple
    template<typename B>
    class InputReferenceNode : public ExpressionNode<B> {
    public:
        InputReferenceNode(const uint32_t & read_idx);
        ~InputReferenceNode() = default;

        Field<B> call(const QueryTuple<B> & target) const override;

        ExpressionKind kind() const override;

        void accept(ExpressionVisitor<B> *visitor) override;

        uint32_t read_idx_;
    };

    template<typename B>
    class LiteralNode : public ExpressionNode<B> {
    public:
        LiteralNode(const Field<B> & literal);
        ~LiteralNode() = default;

        Field<B> call(const QueryTuple<B> & target) const override;

        ExpressionKind kind() const override;
        void accept(ExpressionVisitor<B> *visitor) override;
        std::shared_ptr<LiteralNode<emp::Bit> > toSecure() const;


        Field<B> payload_;

    private:
        SecureField toSecureImpl(const SecureField & f);
        SecureField toSecureImpl(const PlainField & f);

    };



    template<typename B>
    class CastNode : public ExpressionNode<B> {
    public:
        CastNode(std::shared_ptr<ExpressionNode<B> > & input, const FieldType & dst_type);
        ~CastNode() = default;

        Field<B> call(const QueryTuple<B> & target) const override;

        ExpressionKind kind() const override;

        void accept(ExpressionVisitor<B> *visitor) override;

        FieldType dst_type_;
    };


}



#endif //_EXPRESSION_NODE_H
