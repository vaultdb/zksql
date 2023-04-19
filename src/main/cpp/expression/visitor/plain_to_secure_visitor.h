#ifndef _SECURE_TO_PLAIN_VISITOR_H
#define _SECURE_TO_PLAIN_VISITOR_H

#include "expression_visitor.h"
#include <expression/connector_expression_nodes.h>
#include <expression/comparator_expression_nodes.h>
#include <expression/math_expression_nodes.h>
#include <expression/case_node.h>


namespace  vaultdb {

    class PlainToSecureVisitor : public ExpressionVisitor<bool> {

    private:
        std::shared_ptr<ExpressionNode<emp::Bit> > root_;


    public:
        PlainToSecureVisitor(std::shared_ptr<ExpressionNode<bool> > root);

        std::shared_ptr<ExpressionNode<emp::Bit> > getSecureExpression() {
            return root_;
        }

        void visit(InputReferenceNode<bool> node) override;

        void visit(LiteralNode<bool> node) override;

        void visit(AndNode<bool> node) override;

        void visit(OrNode<bool> node) override;

        void visit(NotNode<bool> node) override;

        void visit(PlusNode<bool> node) override;

        void visit(MinusNode<bool> node) override;

        void visit(TimesNode<bool> node) override;

        void visit(DivideNode<bool> node) override;

        void visit(ModulusNode<bool> node) override;

        void visit(EqualNode<bool> node) override;

        void visit(NotEqualNode<bool> node) override;

        void visit(GreaterThanNode<bool> node) override;

        void visit(LessThanNode<bool> node) override;

        void visit(GreaterThanEqNode<bool> node) override;

        void visit(LessThanEqNode<bool> node) override;

        void visit(CastNode<bool> node) override;

        void visit(CaseNode<bool> node) override;

    };


}



#endif //_SECURE_TO_PLAIN_VISITOR_H
