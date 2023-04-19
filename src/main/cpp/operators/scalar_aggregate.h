#ifndef _SCALAR_AGGREGATE_H
#define _SCALAR_AGGREGATE_H


#include <operators/support/aggregate_id.h>
#include <operators/support/scalar_aggregate_impl.h>
#include "operator.h"


namespace vaultdb {
    class ScalarAggregate : public Operator {
    public:
        // aggregates are sorted by their output order in aggregate's output schema
        ScalarAggregate(Operator *child, const std::vector<ScalarAggregateDefinition> &aggregates, const SortDefinition & sort = SortDefinition());;

        ScalarAggregate(const ZkQueryTable & child, const std::vector<ScalarAggregateDefinition> &aggregates, const SortDefinition & sort = SortDefinition());;
        ~ScalarAggregate();


    private:
        std::vector<ScalarAggregateDefinition> aggregate_definitions_;

        std::vector<ScalarAggregateImpl<Bit> *> secure_aggregators_;
        std::vector<ScalarAggregateImpl<bool> *> plain_aggregators_;

        void aggregateFactory(const AggregateId &aggregateType, const uint32_t &ordinal,
                                                   const FieldType &aggregateValueType);

        void setup();

    protected:
        ZkQueryTable runSelf() override;
        string getOperatorType() const override;
        string getParameters() const override;

    };

}
#endif //_SCALAR_AGGREGATE_H
