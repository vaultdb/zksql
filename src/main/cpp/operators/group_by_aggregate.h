#ifndef _GROUP_BY_AGGREGATE_H
#define _GROUP_BY_AGGREGATE_H


#include <operators/support/aggregate_id.h>
#include <operators/support/group_by_aggregate_impl.h>
#include "operator.h"

namespace vaultdb {
    class GroupByAggregate : public Operator {

        std::vector<ScalarAggregateDefinition> aggregate_definitions_;
        std::vector<int32_t> group_by_;

        vector<GroupByAggregateImpl<emp::Bit> *> secure_aggregators_;
        vector<GroupByAggregateImpl<bool> *> plain_aggregators_;

    public:
        GroupByAggregate(Operator *child, const vector<int32_t> &groupBys,
                         const vector<ScalarAggregateDefinition> &aggregates, const SortDefinition & sort = SortDefinition());

        GroupByAggregate(const ZkQueryTable & child, const vector<int32_t> &groupBys,
                         const vector<ScalarAggregateDefinition> &aggregates, const SortDefinition & sort = SortDefinition());;
        ~GroupByAggregate();
        static bool sortCompatible(const SortDefinition & lhs, const vector<int32_t> &group_by_idxs);

    protected:
        ZkQueryTable runSelf() override;
        string getOperatorType() const override;
        string getParameters() const override;

    private:
        void aggregateFactory(const AggregateId &aggregateType, const int32_t &ordinal,
                                               const FieldType &aggregateValueType);

        // checks that input table is sorted by group-by cols
        bool verifySortOrder(const ZkQueryTable & table) const;

        // returns boolean for whether two tuples are in the same group-by bin
        bool groupByMatch(const PlainTuple & lhs, const PlainTuple & rhs) const;
        emp::Bit groupByMatch(const SecureTuple & lhs, const SecureTuple & rhs) const;

        QuerySchema generateOutputSchema(const QuerySchema & srcSchema,
                                         const std::vector<GroupByAggregateImpl<bool> *> & aggregators) const;

        template<typename B>
        inline void generateOutputTuple(QueryTuple<B> &dstTuple, const QueryTuple<B> &lastTuple,
                                        const B &lastEntryGroupByBin, const B &realBin,
                                        const vector<GroupByAggregateImpl<B> *> &aggregators) const {
            size_t i;

            // write group-by ordinals
            for(i = 0; i < group_by_.size(); ++i) {
                const Field<B> srcField = lastTuple.getField(group_by_[i]);
                dstTuple.setField(i, srcField);
            }

            // write partial aggs
            for(GroupByAggregateImpl<B> *aggregator : aggregators) {
                Field<B> currentResult = aggregator->getResult();
                dstTuple.setField(i, currentResult);
                ++i;
            }


            B dummyTag = Field<B>::If(lastEntryGroupByBin, Field<B>(!realBin), B(true)).template getValue<B>();
            dstTuple.setDummyTag(dummyTag);


        }

        template<typename B>
        inline B groupByMatch(const QueryTuple<B> &lhs, const QueryTuple<B> &rhs) const {

            B result = (lhs.getField(group_by_[0]) == rhs.getField(group_by_[0]));
            size_t cursor = 1;

            while(cursor < group_by_.size()) {
                result = result &
                         (lhs.getField(group_by_[cursor]) == rhs.getField(group_by_[cursor]));
                ++cursor;
            }

            return result;
        }



        void setup();


        void runSelfPlain();

        void verifyGroupByAggregate();
    };
}

#endif //_GROUP_BY_AGGREGATE_H
