#ifndef _EXPRESSION_H
#define _EXPRESSION_H


#include <query_table/query_tuple.h>
#include <query_table/query_schema.h>
#include "expression_kind.h"

namespace vaultdb {
    template<typename B> class Expression;
    typedef Expression<bool> PlainExpression;
    typedef Expression<emp::Bit> SecureExpression;


    template<typename B>
    class Expression {

    protected:
        std::string alias_;
        FieldType type_;


    public:
        Expression() : alias_("anonymous"), type_(FieldType::INVALID) {}
        Expression(const std::string & alias, const FieldType & type) : alias_(alias), type_(type) {}
        Expression(const Expression & src) : alias_(src.alias_), type_(src.type_) { }

        virtual ~Expression() {}



        virtual Field<B> call(const QueryTuple<B> & aTuple) const = 0;
        virtual ExpressionKind kind() const = 0;
        virtual std::string toString() const = 0;

        FieldType getType() const { return type_; }

        std::string getAlias() const { return alias_; }


        // for lazy schema evaluation, e.g., Project
        void setType(const FieldType & type) {type_ = type; }
        void setAlias(const std::string & alias) {alias_ = alias; }

    };
}





template class vaultdb::Expression<bool>;
template class vaultdb::Expression<emp::Bit>;


#endif //_EXPRESSION_H

