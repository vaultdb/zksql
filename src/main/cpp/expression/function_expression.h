#ifndef _FUNCTION_EXPRESSION_H
#define _FUNCTION_EXPRESSION_H

#include "expression.h"

namespace vaultdb {

    template<typename B>
    class FunctionExpression : public Expression<B> {

    public:
        FunctionExpression(Field<B> (*funcPtr)(const QueryTuple<B> &), const std::string & anAlias, const FieldType & aType);

        Field<B> call(const QueryTuple<B> & aTuple) const override;
        ExpressionKind kind() const override { return ExpressionKind::FUNCTION; }


        ~FunctionExpression() = default;

        string toString() const override;

    private:
        // function pointer to expression
        Field<B> (*expr_func_)(const QueryTuple<B> &) = nullptr;


    };
}



#endif //VAULTDB_EMP_FUNCTION_EXPRESSION_H
