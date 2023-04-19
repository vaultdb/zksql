#include <operators/support/scalar_aggregate_impl.h>
#include "scalar_aggregate.h"
#include <query_table/plain_tuple.h>
#include <query_table/secure_tuple.h>


using namespace vaultdb;

ScalarAggregate::ScalarAggregate(Operator *child, const vector<ScalarAggregateDefinition> &aggregates,
                                    const SortDefinition &sort)
        : Operator(child, sort), aggregate_definitions_(aggregates) {
            setup();

        }

ScalarAggregate::ScalarAggregate(const ZkQueryTable & child,
                                    const vector<ScalarAggregateDefinition> &aggregates, const SortDefinition &sort)
        : Operator(child, sort), aggregate_definitions_(aggregates) {
            setup();
        }


ZkQueryTable ScalarAggregate::runSelf() {
    ZkQueryTable input = ScalarAggregate::children_[0]->getOutput();

    startTime = clock_start();

    PlainTuple  tuple_plain(QuerySchema::toPlain(*input.getSchema()));
    SecureTuple  tuple_secure(*input.getSchema());

    int party = input.party_;
    output_ = ZkQueryTable(1, output_schema_, party, input.netio_, SortDefinition());


    SecureTuple dst_secure = output_.secure_table_->getTuple(0);
    PlainTuple dst_plain =  (party == ALICE) ? output_.plain_table_->getTuple(0) : PlainTuple(QuerySchema::toPlain(output_schema_));


    for(int i = 0; i < input.getTupleCount(); ++i) {
        tuple_secure = input.secure_table_->getTuple(i);
        for(ScalarAggregateImpl<Bit> *aggregator : secure_aggregators_) {
            aggregator->accumulate(tuple_secure);
        }

        if(party == ALICE) {
            tuple_plain = input.plain_table_->getTuple(i);
            for(ScalarAggregateImpl<bool> *aggregator : plain_aggregators_) {
                aggregator->accumulate(tuple_plain);
            }

        }

    }

    for(size_t i = 0; i < secure_aggregators_.size(); ++i) {
        Field f = secure_aggregators_[i]->getResult();
        dst_secure.setField(i, f);
    }
    // dummy tag is always false in our setting, e.g., if we count a set of nulls/dummies, then our count is zero - not dummy
    dst_secure.setDummyTag(false);

    if(party == ALICE) {
        for(size_t i = 0; i < plain_aggregators_.size(); ++i) {
            Field f = plain_aggregators_[i]->getResult();
            dst_plain.setField(i, f);
        }
        dst_plain.setDummyTag(false);
    }

    return output_;
}


void ScalarAggregate::aggregateFactory(const AggregateId &aggregateType, const uint32_t &ordinal,
                                                            const FieldType &aggregateValueType)  {
        switch (aggregateType) {
            case AggregateId::COUNT: {
                plain_aggregators_.push_back(new ScalarCountImpl<bool>(ordinal, TypeUtilities::toPlain(aggregateValueType)));
                secure_aggregators_.push_back(new ScalarCountImpl<Bit>(ordinal, aggregateValueType));
                break;
            }
            case AggregateId::SUM:{
                plain_aggregators_.push_back(new ScalarSumImpl<bool>(ordinal, TypeUtilities::toPlain(aggregateValueType)));
                secure_aggregators_.push_back(new ScalarSumImpl<Bit>(ordinal, aggregateValueType));
                break;
            }
            case AggregateId::AVG:{
                plain_aggregators_.push_back(new ScalarAvgImpl<bool>(ordinal, TypeUtilities::toPlain(aggregateValueType)));
                secure_aggregators_.push_back(new ScalarAvgImpl<Bit>(ordinal, aggregateValueType));
                break;
            }
            case AggregateId::MIN: {
                plain_aggregators_.push_back(new ScalarMinImpl<bool>(ordinal, TypeUtilities::toPlain(aggregateValueType)));
                secure_aggregators_.push_back(new ScalarMinImpl<Bit>(ordinal, aggregateValueType));
                break;
            }
            case AggregateId::MAX: {
                plain_aggregators_.push_back(new ScalarMaxImpl<bool>(ordinal, TypeUtilities::toPlain(aggregateValueType)));
                secure_aggregators_.push_back(new ScalarMaxImpl<Bit>(ordinal, aggregateValueType));
                break;
            }
            default:
                throw std::invalid_argument("Not yet implemented!");
        };
    }

    void  ScalarAggregate::setup() {

    QuerySchema input_schema = getChild()->getOutputSchema();

    for(ScalarAggregateDefinition agg : aggregate_definitions_) {

        // -1 ordinal for COUNT(*)
        FieldType aggValueType = (agg.ordinal == -1) ?
                                  FieldType::SECURE_LONG :
                                 input_schema.getField(agg.ordinal).getType();
        aggregateFactory(agg.type, agg.ordinal, aggValueType);
    }

        // generate output schema
        output_schema_ = QuerySchema(secure_aggregators_.size());

        for(size_t i = 0; i < secure_aggregators_.size(); ++i) {
            QueryFieldDesc fieldDesc(i, aggregate_definitions_[i].alias, "", TypeUtilities::toPlain(secure_aggregators_[i]->getType()));
            output_schema_.putField(fieldDesc);
        }



    }

string ScalarAggregate::getOperatorType() const {
    return "ScalarAggregate";
}

string ScalarAggregate::getParameters() const {
    stringstream ss;
    ss << "aggs: (" << aggregate_definitions_[0].toString();

    for(uint32_t i = 1; i < aggregate_definitions_.size(); ++i) {
        ss << ", " << aggregate_definitions_[i].toString();
    }

    ss << ")";
    return ss.str();
}

ScalarAggregate::~ScalarAggregate() {
    for(ScalarAggregateImpl<Bit> *agg : secure_aggregators_) {
        delete agg;
    }

    for(ScalarAggregateImpl<bool> *agg : plain_aggregators_) {
        delete agg;
    }

}



