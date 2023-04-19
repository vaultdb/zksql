#ifndef _PRINT_EXPRESSION_VISITOR_H
#define _PRINT_EXPRESSION_VISITOR_H

#include "expression_visitor.h"
#include <string>
#include <expression/case_node.h>

namespace vaultdb {
    template<typename B>
    class PrintExpressionVisitor : public ExpressionVisitor<B> {
    public:
        void visit(InputReferenceNode<B> node) override;

        void visit(LiteralNode<B> node) override;

        void visit(AndNode<B> node) override;

        void visit(OrNode<B> node) override;

        void visit(NotNode<B> node) override;

        void visit(PlusNode<B> node) override;

        void visit(MinusNode<B> node) override;

        void visit(TimesNode<B> node) override;

        void visit(DivideNode<B> node) override;

        void visit(ModulusNode<B> node) override;

        void visit(EqualNode<B> node) override;

        void visit(NotEqualNode<B> node) override;

        void visit(GreaterThanNode<B> node) override;

        void visit(LessThanNode<B> node) override;

        void visit(GreaterThanEqNode<B> node) override;

        void visit(LessThanEqNode<B> node) override;

        void visit(CastNode<B> node) override;

        void visit(CaseNode<B> node) override;

        std::string getString() const;

    private:
        std::string last_value_ = "";

        void visitBinaryExpression(ExpressionNode<B> &binary_node, const std::string &connector);
    };


}
#endif //_PRINT_EXPRESSION_VISITOR_H