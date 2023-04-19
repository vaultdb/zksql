#include "generic_expression.h"
#include <expression/visitor/type_inference_visitor.h>

using namespace vaultdb;

template<typename B>
GenericExpression<B>::GenericExpression(std::shared_ptr<ExpressionNode<B> > root, const QuerySchema & input_schema)
        : Expression<B>("anonymous", GenericExpression<B>::inferFieldType(root, input_schema)), root_(root) {}


template<typename B>
GenericExpression<B>::GenericExpression(std::shared_ptr<ExpressionNode<B>> root, const string &alias,
                                        const FieldType &output_type) : Expression<B>(alias, output_type), root_(root) {

}


template<typename B>
Field<B> GenericExpression<B>::call(const QueryTuple<B> &aTuple) const {
    return root_->call(aTuple);
}

template<typename B>
string GenericExpression<B>::toString() const {
   return root_->toString() + " " +  TypeUtilities::getTypeString(Expression<B>::type_);
}

template<typename B>
FieldType GenericExpression<B>::inferFieldType(std::shared_ptr<ExpressionNode<B> > node, const QuerySchema & input_schema) {
    TypeInferenceVisitor<B> visitor(node, input_schema);
    node->accept(&visitor);
    return visitor.getExpressionType();
}


template class vaultdb::GenericExpression<bool>;
template class vaultdb::GenericExpression<emp::Bit>;


