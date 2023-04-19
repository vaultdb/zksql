#ifndef _EXPRESSION_PARSER_H
#define _EXPRESSION_PARSER_H

#include <expression/expression.h>
#include <expression/generic_expression.h>
#include <expression/bool_expression.h>
#include <expression/expression_node.h>
#include <boost/property_tree/ptree.hpp>

using boost::property_tree::ptree;
// converts from Apache Calcite's JSON representation to a GenericExpression or BinaryExpression

namespace vaultdb {
    template<typename B>
    class ExpressionParser {
    public:
        static std::shared_ptr<Expression<B> > parseJSONExpression(const std::string & json, const QuerySchema & input_schema);
        static std::shared_ptr<Expression<B> > parseExpression(const ptree & tree, const QuerySchema & input_schema);
        static BoolExpression<B> parseBoolExpression(const ptree & tree);

    private:
        static std::shared_ptr<ExpressionNode<B> > parseHelper(const ptree & tree);
        static std::shared_ptr<ExpressionNode<B> > parseSubExpression(const ptree & tree);
        static std::shared_ptr<ExpressionNode<B> > parseInput(const ptree & tree);

    };


}

#endif //_EXPRESSION_PARSER_H
