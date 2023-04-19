#include "expression_parser.h"
#include <boost/property_tree/json_parser.hpp>
#include <util/data_utilities.h>
#include <expression/expression_factory.h>

using namespace vaultdb;
using namespace std;

template<typename B>
std::shared_ptr<Expression<B>> ExpressionParser<B>::parseJSONExpression(const string &json, const QuerySchema & input_schema) {
    stringstream ss;
    ss << json << endl;
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(ss, pt);
    return parseExpression(pt, input_schema);
}

// starts with:
// {
//   "op": {
// ...
// handle "input" / raw value copies separately in parse_projection
template<typename B>
shared_ptr<Expression<B>> ExpressionParser<B>::parseExpression(const ptree &tree, const QuerySchema & input_schema) {
   shared_ptr<ExpressionNode<B> > expression_root = parseHelper(tree);
   return shared_ptr<Expression<B> >(new GenericExpression<B>(expression_root, input_schema));
}

template<typename B>
BoolExpression<B> ExpressionParser<B>::parseBoolExpression(const ptree &tree) {
    shared_ptr<ExpressionNode<B> > expression_root = parseHelper(tree);
    return BoolExpression<B>(expression_root);
}


template<typename B>
shared_ptr<ExpressionNode<B>> ExpressionParser<B>::parseHelper(const ptree &tree) {

    if((tree.count("op") > 0 && tree.count("operands") > 0))
       return parseSubExpression(tree);

    return parseInput(tree);

}

template<typename B>
shared_ptr<ExpressionNode<B>> ExpressionParser<B>::parseSubExpression(const ptree &tree) {
    ptree op = tree.get_child("op");
    ptree operands = tree.get_child("operands");

    string op_name = op.get_child("kind").template get_value<string>();

    std::vector<shared_ptr<ExpressionNode<B> > > children;

    // iterate over operands, invoke helper on each one
    for (ptree::const_iterator it = operands.begin(); it != operands.end(); ++it) {
        children.push_back(parseHelper(it->second));
    }
    return ExpressionFactory<B>::getExpressionNode(op_name, children);
}

template<typename B>
shared_ptr<ExpressionNode<B>> ExpressionParser<B>::parseInput(const ptree &tree) {
    if(tree.count("literal")  > 0) {
        ptree literal = tree.get_child("literal");
        std::string type_str = tree.get_child("type").get_child("type").template get_value<std::string>();

        if(type_str == "INTEGER" || type_str == "DATE") { // dates input as epochs
            int32_t literal_int = literal.template get_value<int32_t>();

            Field<B> input_field = (std::is_same_v<B, bool>) ?
                    Field<B>(FieldType::INT, Value((int32_t) literal_int))
                            :  Field<B>(FieldType::SECURE_INT, emp::Integer(32, literal_int));
            return shared_ptr<ExpressionNode<B> > (new LiteralNode<B>(input_field));
        }
        else if(type_str == "LONG") {
            int64_t literal_int = literal.template get_value<int64_t>();

            Field<B> input_field = (std::is_same_v<B, bool>) ?
                                   Field<B>(FieldType::LONG, Value((int64_t) literal_int))
                                                             :  Field<B>(FieldType::SECURE_LONG, emp::Integer(64, literal_int));
            return shared_ptr<ExpressionNode<B> > (new LiteralNode<B>(input_field));
        }
        else if(type_str == "FLOAT" || type_str == "DECIMAL") {
            float_t literal_float = literal.template get_value<float_t>();
            Field<B> input_field = (std::is_same_v<B, bool>) ?
                                   Field<B>(FieldType::FLOAT, Value(literal_float))
                                                             :  Field<B>(FieldType::SECURE_FLOAT, emp::Float(literal_float));

            return shared_ptr<ExpressionNode<B> > (new LiteralNode<B>(input_field));


        }
        else if(type_str == "CHAR" || type_str == "VARCHAR") {
            boost::property_tree::ptree type_tree = tree.get_child("type");
            int length =    type_tree.get_child("precision").get_value<int>();
            std::string literal_string =   literal.template get_value<std::string>();
            Field<B> input_field =  Field<B>(FieldType::STRING, Value(literal_string), length);

            if(std::is_same_v<B, emp::Bit>) {
                SecureField encrypted_field = input_field.secret_share();
                Integer encrypted_string = encrypted_field.template getValue<emp::Integer>();
                return shared_ptr<ExpressionNode<B> > (new LiteralNode<B>(Field<B>(FieldType::SECURE_STRING, encrypted_string, length)));
            }

            return shared_ptr<ExpressionNode<B> > (new LiteralNode<B>(input_field));

        }
        throw std::invalid_argument("Parsing input of type " + type_str + " not yet implemented!");

    }
    // else it is an input
    std::string expr = tree.get_child("input").get_value<std::string>();
    uint32_t src_ordinal;
    if(DataUtilities::isOrdinal(expr)) { // mapping a column from one position to another
        src_ordinal = std::atoi(expr.c_str());
    }
    else {
        throw std::invalid_argument("Expression " + expr + " is not a properly formed input");
    }

    return shared_ptr<ExpressionNode<B> > (new InputReferenceNode<B>(src_ordinal));


}



template class vaultdb::ExpressionParser<bool>;
template class vaultdb::ExpressionParser<emp::Bit>;
