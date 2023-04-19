#ifndef _SCALAR_AGGREGATE_IMPL_H
#define _SCALAR_AGGREGATE_IMPL_H


#include <query_table/query_tuple.h>
#include <query_table/field/field_factory.h>
#include <climits>
#include <cfloat>

namespace vaultdb {
    template<typename B>
    class ScalarAggregateImpl {
    public:
        ScalarAggregateImpl(const uint32_t & ordinal, const FieldType & aggType) : aggregateOrdinal(ordinal), aggregateType(aggType),
                                                                                       zero(FieldFactory<B>::getZero(aggregateType)),
                                                                                       one(FieldFactory<B>::getOne(aggregateType)){};
        virtual ~ScalarAggregateImpl() = default;
        virtual void accumulate(const QueryTuple<B> & tuple) = 0;
        virtual  Field<B> getResult() const = 0;
        virtual FieldType getType() const { return aggregateType; }

    protected:

        uint32_t aggregateOrdinal;
        FieldType aggregateType;
        Field<B> zero;
        Field<B> one;

    };

    template<typename B>
    class ScalarCountImpl : public ScalarAggregateImpl<B> {
    public:
        ScalarCountImpl(const uint32_t & ordinal, const FieldType & aggType);
        ~ScalarCountImpl() = default;
        void accumulate(const QueryTuple<B> & tuple) override;
         Field<B> getResult() const override;

    private:
        Field<B> runningCount;

    };


    template<typename B>
    class ScalarSumImpl : public ScalarAggregateImpl<B> {
    public:
        ScalarSumImpl(const uint32_t & ordinal, const FieldType & aggType) :  ScalarAggregateImpl<B>(ordinal, aggType), runningSum(ScalarAggregateImpl<B>::zero) {}
        ~ScalarSumImpl() = default;
        void accumulate(const QueryTuple<B> & tuple) override;
         Field<B> getResult() const override;
        FieldType getType() const override;

    private:
        Field<B> runningSum;

    };



    template<typename B>
    class ScalarMinImpl : public ScalarAggregateImpl<B> {
    public:
      ScalarMinImpl(const uint32_t & ordinal, const FieldType & aggType);
      ~ScalarMinImpl()  = default;
      void accumulate(const QueryTuple<B> & tuple) override;
       Field<B> getResult() const override;

    private:
      Field<B> runningMin;

    };



    template<typename B>
    class ScalarMaxImpl : public ScalarAggregateImpl<B> {
      public:
        ScalarMaxImpl(const uint32_t & ordinal, const FieldType & aggType);
        ~ScalarMaxImpl() = default;
        void accumulate(const QueryTuple<B> & tuple) override;
         Field<B> getResult() const override;

      private:
        Field<B> runningMax;
    };


    template<typename B>
    class ScalarAvgImpl : public ScalarAggregateImpl<B> {
    public:
        // following psql convention, only outputs floats
        ScalarAvgImpl(const uint32_t & ordinal, const FieldType & aggType);
        ~ScalarAvgImpl()  = default;
        void accumulate(const QueryTuple<B> & tuple) override;
         Field<B> getResult() const override;
        FieldType getType() const override;

    private:
        Field<B> runningSum;
        Field<B> runningCount;

    };

}

#endif
