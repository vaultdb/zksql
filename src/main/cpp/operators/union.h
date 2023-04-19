#ifndef _UNION_H
#define _UNION_H

#include "operator.h"


// concatenates two input arrays.  Equiavalent to UNION ALL in SQL
namespace  vaultdb {

    class Union : public Operator {


    public:
        Union(Operator *lhs, Operator *rhs, const SortDefinition & sort = SortDefinition()) : Operator(lhs, rhs, sort) {
            Operator::output_schema_ = QuerySchema::toSecure(lhs->getOutputSchema());
        }

        Union(const ZkQueryTable & lhs, const ZkQueryTable &  rhs, const SortDefinition & sort = SortDefinition()) : Operator(lhs, rhs, sort) {
            Operator::output_schema_ = QuerySchema::toSecure(*lhs.getSchema());
        }

        ~Union() = default;


    protected:
        ZkQueryTable runSelf()  override;
        string getOperatorType() const override;
        string getParameters() const override;

    private:
        void copy_plain(const ZkQueryTable &lhs, const ZkQueryTable &rhs) const;
        void copy_secure(const ZkQueryTable &lhs, const ZkQueryTable &rhs) const;


    };


}



#endif //_UNION_H
