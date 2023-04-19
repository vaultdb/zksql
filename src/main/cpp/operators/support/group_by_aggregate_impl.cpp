#include "group_by_aggregate_impl.h"
#include <query_table/secure_tuple.h>
#include <query_table/plain_tuple.h>

using namespace vaultdb;



template<typename B>
GroupByCountImpl<B>::GroupByCountImpl(const int32_t &ordinal, const FieldType &aggType) : GroupByAggregateImpl<B>(ordinal, aggType)
{
    GroupByAggregateImpl<B>::aggregateType =
            (TypeUtilities::isEncrypted(aggType)) ? FieldType::SECURE_LONG : FieldType::LONG;

    GroupByAggregateImpl<B>::zero = FieldFactory<B>::getZero(GroupByAggregateImpl<B>::aggregateType);
    GroupByAggregateImpl<B>::one = FieldFactory<B>::getOne(GroupByAggregateImpl<B>::aggregateType);
    runningCount = GroupByAggregateImpl<B>::zero;

}



template<typename B>
void GroupByCountImpl<B>::initialize(const QueryTuple<B> &tuple, const B &isGroupByMatch) {
    Field<B> resetValue = Field<B>::If(tuple.getDummyTag(), GroupByAggregateImpl<B>::zero, GroupByAggregateImpl<B>::one);
    runningCount = Field<B>::If(isGroupByMatch, runningCount, resetValue);

}



template<typename B>
void GroupByCountImpl<B>::accumulate(const QueryTuple<B> &tuple, const B &isGroupByMatch) {

    B toUpdate = (!isGroupByMatch) & !(tuple.getDummyTag());
    runningCount = runningCount + Field<B>::If(toUpdate, GroupByCountImpl<B>::one, GroupByCountImpl<B>::zero);

}

template<typename B>
Field<B> GroupByCountImpl<B>::getResult() {
    return runningCount;
}

template<typename B>
FieldType GroupByCountImpl<B>::getType() const {
    if(TypeUtilities::isEncrypted(GroupByAggregateImpl<B>::aggregateType))
        return FieldType::SECURE_LONG;
    return FieldType::LONG;  // count always returns a long
}


template<typename B>
GroupBySumImpl<B>::GroupBySumImpl(const int32_t &ordinal, const FieldType &aggType) : GroupByAggregateImpl<B>(ordinal, aggType) {
    runningSum = GroupByAggregateImpl<B>::zero;
    assert(aggType != FieldType::STRING && aggType != FieldType::SECURE_STRING);
}


template<typename B>
void GroupBySumImpl<B>::initialize(const QueryTuple<B> &tuple, const B &isGroupByMatch) {

   const Field<B> aggInput = tuple.getField(GroupByAggregateImpl<B>::aggregateOrdinal);
   Field<B> resetValue = Field<B>::If(tuple.getDummyTag(), GroupByAggregateImpl<B>::zero, aggInput);
   runningSum = Field<B>::If(isGroupByMatch, runningSum, resetValue);

}


template<typename B>
void GroupBySumImpl<B>::accumulate(const QueryTuple<B> &tuple, const B &isGroupByMatch) {
    Field<B> aggInput = tuple.getField(GroupByAggregateImpl<B>::aggregateOrdinal);
    B toUpdate = (!isGroupByMatch) & !tuple.getDummyTag();
    runningSum = runningSum + Field<B>::If(toUpdate, aggInput, GroupByAggregateImpl<B>::zero);
}

template<typename B>
Field<B> GroupBySumImpl<B>::getResult() {
    // extend this to a LONG to keep with PostgreSQL convention
   if(runningSum.getType() == FieldType::INT || runningSum.getType() == FieldType::SECURE_INT)
        return FieldFactory<B>::toLong(runningSum);

    return runningSum;
}

template<typename B>
FieldType GroupBySumImpl<B>::getType() const {
    if(runningSum.getType() == FieldType::INT) return FieldType::LONG;
    if(runningSum.getType() == FieldType::SECURE_INT) return FieldType::SECURE_LONG;

    return GroupByAggregateImpl<B>::aggregateType;
}


template<typename B>
GroupByAvgImpl<B>::GroupByAvgImpl(const int32_t &ordinal, const FieldType &aggType) : GroupByAggregateImpl<B>(ordinal, aggType)  {
    runningSum = GroupByAggregateImpl<B>::zero;
    runningCount = GroupByAggregateImpl<B>::zero;
}


template<typename B>
void GroupByAvgImpl<B>::initialize(const QueryTuple<B> &tuple, const B &isGroupByMatch) {
    Field<B> resetValue = GroupByAggregateImpl<B>::one;
    runningCount = Field<B>::If(isGroupByMatch, runningCount, resetValue);


    Field<B> aggInput = tuple.getField(GroupByAggregateImpl<B>::aggregateOrdinal);
    resetValue = Field<B>::If(tuple.getDummyTag(), GroupByAggregateImpl<B>::zero, aggInput);
    runningSum = Field<B>::If(isGroupByMatch, runningSum, resetValue);

}

template<typename B>
void GroupByAvgImpl<B>::accumulate(const QueryTuple<B> &tuple, const B &isGroupByMatch) {

    Field<B> aggInput = tuple.getField(GroupByAggregateImpl<B>::aggregateOrdinal);
    B toUpdate = (!isGroupByMatch) & !(tuple.getDummyTag());
    runningCount = runningCount + Field<B>::If(toUpdate, GroupByAggregateImpl<B>::one, GroupByAggregateImpl<B>::zero);
    runningSum = runningSum +  Field<B>::If(toUpdate, aggInput, GroupByAggregateImpl<B>::zero);
}

template<typename B>
Field<B> GroupByAvgImpl<B>::getResult() {
    return runningSum / runningCount;
}

// always returns a float
template<typename B>
FieldType GroupByAvgImpl<B>::getType() const {
    if(TypeUtilities::isEncrypted(runningSum.getType())) {
        return FieldType::SECURE_LONG;
    }
    return FieldType::LONG;
}


template<typename B>
GroupByMinImpl<B>::GroupByMinImpl(const int32_t &ordinal, const FieldType &aggType) : GroupByAggregateImpl<B>(ordinal, aggType) {
    runningMin = FieldFactory<B>::getMax(aggType);
    resetMin = runningMin;
}


template<typename B>
void GroupByMinImpl<B>::initialize(const QueryTuple<B> &tuple, const B &isGroupByMatch) {

    Field<B> aggInput = tuple.getField(GroupByAggregateImpl<B>::aggregateOrdinal);
    Field<B> newValue = Field<B>::If(tuple.getDummyTag(), resetMin, aggInput);
    runningMin = Field<B>::If(isGroupByMatch, runningMin, newValue);
}

template<typename B>
void GroupByMinImpl<B>::accumulate(const QueryTuple<B> &tuple, const B &isGroupByMatch) {
    Field<B> aggInput = tuple.getField(GroupByAggregateImpl<B>::aggregateOrdinal);
    B toUpdate = (!isGroupByMatch) & !(tuple.getDummyTag()) & (aggInput < runningMin);
    runningMin = Field<B>::If(toUpdate, aggInput, runningMin);

}

// if not initialized the value will get discarded later because the whole group-by bin will have had dummies
template<typename B>
Field<B> GroupByMinImpl<B>::getResult() {
    return runningMin;
}



template<typename B>
GroupByMaxImpl<B>::GroupByMaxImpl(const int32_t &ordinal, const FieldType &aggType) : GroupByAggregateImpl<B>(ordinal, aggType) {
    runningMax = FieldFactory<B>::getMin(aggType);
    resetMax = runningMax;
}


template<typename B>
void GroupByMaxImpl<B>::initialize(const QueryTuple<B> &tuple, const B &isGroupByMatch) {

    Field<B> aggInput = tuple.getField(GroupByAggregateImpl<B>::aggregateOrdinal);
    Field<B> newValue = Field<B>::If(tuple.getDummyTag(), resetMax, aggInput);
    runningMax = Field<B>::If(isGroupByMatch, runningMax, newValue);

}

template<typename B>
void GroupByMaxImpl<B>::accumulate(const QueryTuple<B> &tuple, const B &isGroupByMatch) {
    Field<B> aggInput = tuple.getField(GroupByAggregateImpl<B>::aggregateOrdinal);
    B toUpdate = (!isGroupByMatch) & !(tuple.getDummyTag()) & (aggInput > runningMax);
    runningMax = Field<B>::If(toUpdate, aggInput, runningMax);
}


template<typename B>
Field<B> GroupByMaxImpl<B>::getResult() {

    return runningMax;
}

template class vaultdb::GroupByAggregateImpl<bool>;
template class vaultdb::GroupByAggregateImpl<emp::Bit>;

template class vaultdb::GroupByCountImpl<bool>;
template class vaultdb::GroupByCountImpl<emp::Bit>;

template class vaultdb::GroupBySumImpl<bool>;
template class vaultdb::GroupBySumImpl<emp::Bit>;

template class vaultdb::GroupByMinImpl<bool>;
template class vaultdb::GroupByMinImpl<emp::Bit>;

template class vaultdb::GroupByMaxImpl<bool>;
template class vaultdb::GroupByMaxImpl<emp::Bit>;

template class vaultdb::GroupByAvgImpl<bool>;
template class vaultdb::GroupByAvgImpl<emp::Bit>;

