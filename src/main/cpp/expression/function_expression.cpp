#include "function_expression.h"

using namespace vaultdb;

template<typename B>
FunctionExpression<B>::FunctionExpression(Field<B> (*funcPtr)(const QueryTuple<B> &), const string &anAlias,
                                          const FieldType &aType)  : Expression<B>(anAlias, aType) {
    expr_func_ = funcPtr;
}

template<typename B>
Field<B> FunctionExpression<B>::call(const QueryTuple<B> &aTuple) const {
    return expr_func_(aTuple);
}

template<typename B>
string FunctionExpression<B>::toString() const {
    return "FunctionExpression(" + Expression<B>::alias_ + " " + TypeUtilities::getTypeString(Expression<B>::type_) + ")";
}

template class vaultdb::FunctionExpression<bool>;
template class vaultdb::FunctionExpression<emp::Bit>;

