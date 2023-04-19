#include "scalar_aggregate_impl.h"
#include <query_table/plain_tuple.h>
#include <query_table/secure_tuple.h>

using namespace vaultdb;

template<typename B>
ScalarCountImpl<B>::ScalarCountImpl(const uint32_t &ordinal, const FieldType &aggType) : ScalarAggregateImpl<B>(ordinal, aggType) {
    // initialize as long for count regardless of input ordinal

    ScalarAggregateImpl<B>::aggregateType =
            (TypeUtilities::isEncrypted(aggType)) ? FieldType::SECURE_LONG : FieldType::LONG;

    ScalarAggregateImpl<B>::zero = FieldFactory<B>::getZero(ScalarAggregateImpl<B>::aggregateType);
    ScalarAggregateImpl<B>::one = FieldFactory<B>::getOne(ScalarAggregateImpl<B>::aggregateType);
    runningCount = ScalarAggregateImpl<B>::zero;

}

template<typename B>
void ScalarCountImpl<B>::accumulate(const QueryTuple<B> &tuple) {
    runningCount = Field<B>::If(tuple.getDummyTag(), runningCount, runningCount + ScalarAggregateImpl<B>::one);
}

template<typename B>
 Field<B> ScalarCountImpl<B>::getResult() const {
    return runningCount;
}



template<typename B>
void ScalarSumImpl<B>::accumulate(const QueryTuple<B> &tuple) {
    Field<B> aggInput = tuple.getField(ScalarAggregateImpl<B>::aggregateOrdinal);
    runningSum = Field<B>::If(tuple.getDummyTag(), runningSum, runningSum + aggInput);
}

template<typename B>
 Field<B> ScalarSumImpl<B>::getResult() const {
    // extend this to a LONG to keep with PostgreSQL convention
    if(runningSum.getType() == FieldType::INT || runningSum.getType() == FieldType::SECURE_INT)
        return FieldFactory<B>::toLong(runningSum);

    return runningSum;
}

template<typename B>
FieldType ScalarSumImpl<B>::getType() const {
    if(runningSum.getType() == FieldType::INT) return FieldType::LONG;
    if(runningSum.getType() == FieldType::SECURE_INT) return FieldType::SECURE_LONG;

    return ScalarAggregateImpl<B>::aggregateType;

}


template<typename B>
ScalarMinImpl<B>::ScalarMinImpl(const uint32_t &ordinal, const FieldType &aggType):ScalarAggregateImpl<B>(ordinal, aggType) {
    runningMin = FieldFactory<B>::getMax(aggType);

}

template<typename B>
void ScalarMinImpl<B>::accumulate(const QueryTuple<B> &tuple) {
    Field<B> aggInput = tuple.getField(ScalarAggregateImpl<B>::aggregateOrdinal);
    B sameMin = aggInput >= runningMin;
    runningMin = Field<B>::If(sameMin | tuple.getDummyTag(), runningMin, aggInput);
}

template<typename B>
 Field<B> ScalarMinImpl<B>::getResult() const {
    return runningMin;
}


template<typename B>
ScalarMaxImpl<B>::ScalarMaxImpl(const uint32_t &ordinal, const FieldType &aggType):ScalarAggregateImpl<B>(ordinal, aggType) {
    runningMax = FieldFactory<B>::getMin(aggType);

}

template<typename B>
void ScalarMaxImpl<B>::accumulate(const QueryTuple<B> &tuple) {
    Field<B> aggInput = tuple.getField(ScalarAggregateImpl<B>::aggregateOrdinal);
    B sameMax = aggInput <= runningMax;
    runningMax = Field<B>::If(sameMax | tuple.getDummyTag(), runningMax, aggInput);

}

template<typename B>
 Field<B> ScalarMaxImpl<B>::getResult() const {
    return runningMax;
}


template<typename B>
ScalarAvgImpl<B>::ScalarAvgImpl(const uint32_t &ordinal, const FieldType &aggType) :  ScalarAggregateImpl<B>(ordinal, aggType) {
    runningSum = ScalarAggregateImpl<B>::zero;
    runningCount = ScalarAggregateImpl<B>::zero;

}

template<typename B>
void ScalarAvgImpl<B>::accumulate(const QueryTuple<B> &tuple) {
    Field<B> aggInput = tuple.getField(ScalarAggregateImpl<B>::aggregateOrdinal);
    runningSum = Field<B>::If(tuple.getDummyTag(), runningSum, runningSum + aggInput);
    runningCount = Field<B>::If(tuple.getDummyTag(), runningCount, runningCount + ScalarAggregateImpl<B>::one);

}

template<typename B>
 Field<B> ScalarAvgImpl<B>::getResult() const {
    Field<B> quotient = runningSum / runningCount;
    Field<B> remainder = runningSum % runningCount;
    Field<B> quotientPlusOne = quotient + ScalarAggregateImpl<B>::one;
    return Field<B>::If((runningCount - remainder) <= remainder,quotientPlusOne,quotient);

}

template<typename B>
FieldType ScalarAvgImpl<B>::getType() const {
    if(TypeUtilities::isEncrypted(runningSum.getType())) {
        return FieldType::SECURE_LONG;
    }
    return FieldType::LONG;

}


template class vaultdb::ScalarCountImpl<bool>;
template class vaultdb::ScalarCountImpl<emp::Bit>;

template class vaultdb::ScalarSumImpl<bool>;
template class vaultdb::ScalarSumImpl<emp::Bit>;

template class vaultdb::ScalarMinImpl<bool>;
template class vaultdb::ScalarMinImpl<emp::Bit>;

template class vaultdb::ScalarMaxImpl<bool>;
template class vaultdb::ScalarMaxImpl<emp::Bit>;

template class vaultdb::ScalarAvgImpl<bool>;
template class vaultdb::ScalarAvgImpl<emp::Bit>;
