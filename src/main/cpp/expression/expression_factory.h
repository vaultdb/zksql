#ifndef _EXPRESSION_FACTORY_H
#define _EXPRESSION_FACTORY_H

#include <expression/expression_kind.h>
#include <expression/expression_node.h>
#include <expression/math_expression_nodes.h>
#include <expression/comparator_expression_nodes.h>
#include <expression/connector_expression_nodes.h>
#include <expression/case_node.h>

namespace  vaultdb {

template<typename B>
class ExpressionFactory {
public:
    static std::shared_ptr<ExpressionNode<B> > getExpressionNode(const std::string & expression_kind, std::shared_ptr<ExpressionNode<B> > lhs,   std::shared_ptr<ExpressionNode<B> > rhs);

    static std::shared_ptr<ExpressionNode<B> > getExpressionNode(const std::string & expression_kind, vector<std::shared_ptr<ExpressionNode<B> > >  & operands);

    static std::shared_ptr<ExpressionNode<B> > getExpressionNode(const ExpressionKind & kind, std::shared_ptr<ExpressionNode<B> > lhs,   std::shared_ptr<ExpressionNode<B> > rhs);

    // convert from Calcite JSON's SqlKind format to enum
    static ExpressionKind getKind(const std::string & expression_kind);

};



}


#endif //_EXPRESSION_FACTORY_H
