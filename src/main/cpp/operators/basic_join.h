#ifndef _BASIC_JOIN_H
#define _BASIC_JOIN_H

// creates a cross product of its input relations, sets the dummy tag in output to reflect selected tuples

#include "join.h"


namespace vaultdb {

    class BasicJoin : public Join {

    public:
        BasicJoin(Operator *lhs, Operator *rhs, const BoolExpression<bool> & predicate, const SortDefinition & sort = SortDefinition());
        BasicJoin(const ZkQueryTable & lhs, const ZkQueryTable & rhs, const BoolExpression<bool> & predicate, const SortDefinition & sort = SortDefinition());
        ~BasicJoin() = default;

    protected:
        ZkQueryTable runSelf() override;
        string getOperatorType() const override;

    };
}

#endif //_BASIC_JOIN_H
