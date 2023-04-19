#ifndef _PROPERTY_TREE_EXPRESSION_H
#define _PROPERTY_TREE_EXPRESSION_H

#include <boost/property_tree/ptree.hpp>
#include "expression.h"

namespace vaultdb {


// takes in a boost::propertytree - most likely from JSON - and traverses it to evaluate expression on a tuple's fields.
    template<typename B>
    class PropertyTreeExpression : public Expression<B> {
        PropertyTreeExpression() : Expression<B>(FieldType::INVALID, "anonymous") {}
        PropertyTreeExpression(const boost::property_tree::ptree  & expression, const FieldType & type, const std::string & alias);

    private:
        boost::property_tree::ptree expression_tree_;

    };

    template<typename B>
    PropertyTreeExpression<B>::PropertyTreeExpression(const boost::property_tree::ptree &expression,
                                                      const FieldType &type, const string &alias) : Expression<B>(type, alias), expression_tree_(expression) {

    }
}

#endif //VAULTDB_EMP_PROPERTY_TREE_EXPRESSION_H
