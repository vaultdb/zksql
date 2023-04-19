#ifndef _FILTER_H
#define _FILTER_H

#include "operator.h"
#include <expression/bool_expression.h>

namespace  vaultdb {

    class Filter : public Operator {

        BoolExpression<bool> plain_predicate_;
        BoolExpression<emp::Bit> secure_predicate_;

    public:
        Filter(Operator *child, const BoolExpression<bool> &predicate);
        Filter(const ZkQueryTable & child, const BoolExpression<bool> &predicate);

        ~Filter() = default;

        ZkQueryTable runSelf()  override;

    protected:
        string getOperatorType() const override;

        string getParameters() const override;


    };

}


#endif // _FILTER_H
