#include <util/type_utilities.h>
#include <util/data_utilities.h>
#include <util/field_utilities.h>

#include "group_by_aggregate.h"
#include <query_table/plain_tuple.h>
#include <query_table/secure_tuple.h>



using namespace vaultdb;
using namespace std;

GroupByAggregate::GroupByAggregate(Operator *child, const vector<int32_t> &groupBys,
                                      const vector<ScalarAggregateDefinition> &aggregates, const SortDefinition &sort) : Operator(child, sort),
                                                                                                                         aggregate_definitions_(aggregates),
                                                                                                                         group_by_(groupBys) {

  setup();
 }


GroupByAggregate::GroupByAggregate(const ZkQueryTable & child, const vector<int32_t> &groupBys,
                                      const vector<ScalarAggregateDefinition> &aggregates, const SortDefinition &sort) : Operator(child, sort),
                                                                                                                         aggregate_definitions_(aggregates),
                                                                                                                         group_by_(groupBys) {

      setup();
 }



ZkQueryTable GroupByAggregate::runSelf() {
    ZkQueryTable input = Operator::children_[0]->getOutput();
    uint64_t commCostFromAllThreadsBefore = gv.getAccumulativeCommCostFromAllThreads();

    startTime = clock_start();

    QuerySchema input_schema = *input.getSchema();
    verifySortOrder(input);


    // output sort order equal to first group-by-col-count entries in input sort order
    vector<ColumnSort>::iterator last_sort_pos = input.getSortOrder().begin() + group_by_.size();
    SortDefinition input_sort = vector<ColumnSort>(input.getSortOrder().begin(), last_sort_pos);
    int party = input.party_;
    SortDefinition output_sort = vector<ColumnSort>(input_sort.begin(), input_sort.begin() + group_by_.size());


    output_ = ZkQueryTable(input.getTupleCount(), output_schema_, party, input.netio_, output_sort);

    if(party == ALICE)
        runSelfPlain();

    // EMP equivalent to runSelfPlain
    verifyGroupByAggregate();

    output_ = zeroOutDummies(output_);

    uint64_t commCostFromAllThreadsAfter = gv.getAccumulativeCommCostFromAllThreads();
    comm_cost = commCostFromAllThreadsAfter - commCostFromAllThreadsBefore;
    gv.setCommCost(gv.getCommCost() + comm_cost);

    return output_;

}


void GroupByAggregate::runSelfPlain() {
    //assert(children_[0]->getOutput().party_ == ALICE);

    shared_ptr<PlainTable> input = children_[0]->getOutput().plain_table_;
    QuerySchema input_schema = *input->getSchema();


    bool real_bin;

    PlainTuple current(input_schema), predecessor(input_schema);

    PlainTuple tuple = (*output_.plain_table_)[0];
    bool dummy_tag(false);
    tuple.setDummyTag(dummy_tag);


    predecessor = (*input)[0];

    for(auto *aggregator : plain_aggregators_) {
        aggregator->initialize(predecessor,false);
    }

    real_bin = !predecessor.getDummyTag();


    for(uint32_t i = 1; i < input->getTupleCount(); ++i) {
        current = (*input)[i];

        real_bin = real_bin | !predecessor.getDummyTag();
        bool group_by_match = groupByMatch<bool>(predecessor, current);

        PlainTuple output_tuple = output_.plain_table_->getTuple(i - 1); // to write to it in place
        generateOutputTuple(output_tuple, predecessor, !group_by_match, real_bin, plain_aggregators_);

        for(auto *aggregator : plain_aggregators_) {
            aggregator->initialize(current, group_by_match);
            aggregator->accumulate(current, !group_by_match);
        }

        predecessor = current;
        // reset the flag at the end of each group-by bin
        // flag denotes if we have one non-dummy tuple in the bin
        real_bin = Field<bool>::If(!group_by_match, false, real_bin).getValue<bool>();

    }

    real_bin = real_bin | !predecessor.getDummyTag();


    // B(true) to make it write out the last entry
    PlainTuple output_tuple = output_.plain_table_->getTuple(input->getTupleCount() - 1);
    generateOutputTuple(output_tuple, predecessor, true, real_bin, plain_aggregators_);




}

void GroupByAggregate::verifyGroupByAggregate() {


    shared_ptr<SecureTable> input = children_[0]->getOutput().secure_table_;
    QuerySchema input_schema = *input->getSchema();


    Bit real_bin;

    SecureTuple current(input_schema), predecessor(input_schema);

    SecureTuple tuple = (*output_.secure_table_)[0];
    Bit dummy_tag(false);
    tuple.setDummyTag(dummy_tag);


    predecessor = (*input)[0];

    for(auto *aggregator : secure_aggregators_) {
        aggregator->initialize(predecessor,false);
    }

    real_bin = !predecessor.getDummyTag();


    for(uint32_t i = 1; i < input->getTupleCount(); ++i) {
        current = (*input)[i];

        real_bin = real_bin | !predecessor.getDummyTag();
        Bit group_by_match = groupByMatch<Bit>(predecessor, current);

        SecureTuple output_tuple = output_.secure_table_->getTuple(i - 1); // to write to it in place
        generateOutputTuple(output_tuple, predecessor, !group_by_match, real_bin, secure_aggregators_);

        for(auto *aggregator : secure_aggregators_) {
            aggregator->initialize(current, group_by_match);
            aggregator->accumulate(current, !group_by_match);
        }

        predecessor = current;
        // reset the flag at the end of each group-by bin
        // flag denotes if we have one non-dummy tuple in the bin
        real_bin = Field<Bit>::If(!group_by_match, Bit(false), real_bin).getValue<Bit>();

    }

    real_bin = real_bin | !predecessor.getDummyTag();


    // B(true) to make it write out the last entry
    SecureTuple output_tuple = output_.secure_table_->getTuple(input->getTupleCount() - 1);
    generateOutputTuple(output_tuple, predecessor, Bit(true), real_bin, secure_aggregators_);

}

void GroupByAggregate::aggregateFactory(const AggregateId &aggregateType, const int32_t &ordinal,
                                                         const FieldType &aggregateValueType)  {

    switch (aggregateType) {
            case AggregateId::COUNT: {
                plain_aggregators_.push_back(new GroupByCountImpl<bool>(ordinal, TypeUtilities::toPlain(aggregateValueType)));
                secure_aggregators_.push_back(new GroupByCountImpl<Bit>(ordinal, aggregateValueType) );
                break;
            }
            case AggregateId::SUM: {
                plain_aggregators_.push_back(new GroupBySumImpl<bool>(ordinal, TypeUtilities::toPlain(aggregateValueType)));
                secure_aggregators_.push_back(new GroupBySumImpl<Bit>(ordinal, aggregateValueType) );
                break;
            }
            case AggregateId::AVG: {
                plain_aggregators_.push_back(new GroupByAvgImpl<bool>(ordinal, TypeUtilities::toPlain(aggregateValueType)));
                secure_aggregators_.push_back(new GroupByAvgImpl<Bit>(ordinal, aggregateValueType) );
                break;
            }
            case AggregateId::MIN:{
                {
                    plain_aggregators_.push_back(new GroupByMinImpl<bool>(ordinal, TypeUtilities::toPlain(aggregateValueType)));
                    secure_aggregators_.push_back(new GroupByMinImpl<Bit>(ordinal, aggregateValueType) );
                    break;
                }
            }
            case AggregateId::MAX:  {
                plain_aggregators_.push_back(new GroupByMaxImpl<bool>(ordinal, TypeUtilities::toPlain(aggregateValueType)));
                secure_aggregators_.push_back(new GroupByMaxImpl<Bit>(ordinal, aggregateValueType) );
                break;
            }
        default:
            throw;
        };
    }


bool GroupByAggregate::verifySortOrder(const ZkQueryTable &table) const {
    SortDefinition sortedOn = table.getSortOrder();
    return sortCompatible(sortedOn, group_by_);
}



QuerySchema GroupByAggregate::generateOutputSchema(const QuerySchema & srcSchema, const vector<GroupByAggregateImpl<bool> *> & aggregators) const {
    QuerySchema outputSchema(group_by_.size() + aggregate_definitions_.size());
    size_t i;

    for(i = 0; i < group_by_.size(); ++i) {
        QueryFieldDesc srcField = srcSchema.getField(group_by_[i]);
        QueryFieldDesc dstField(i, srcField.getName(), srcField.getTableName(), srcField.getType());
        dstField.setStringLength(srcField.getStringLength());
        outputSchema.putField(dstField);
    }

    for(i = 0; i < aggregate_definitions_.size(); ++i) {
        QueryFieldDesc fieldDesc(i + group_by_.size(), aggregate_definitions_[i].alias, "", aggregators[i]->getType());
        outputSchema.putField(fieldDesc);
    }

    return outputSchema;

}



bool GroupByAggregate::sortCompatible(const SortDefinition & sorted_on, const vector<int32_t> &group_by_idxs) {
    if(sorted_on.size() < group_by_idxs.size())
        return false;

    for(size_t idx = 0; idx < group_by_idxs.size(); ++idx) {
        // ASC || DESC does not matter here
        if(group_by_idxs[idx] != sorted_on[idx].first) {
            return false;
        }
    }

    return true;

}


void GroupByAggregate::setup() {
    QuerySchema input_schema = Operator::getChild(0)->getOutputSchema();
    SortDefinition input_sort = Operator::getChild(0)->getSortOrder();

    for(ScalarAggregateDefinition agg : aggregate_definitions_) {
        // for most aggs the output type is the same as the input type
        // for COUNT(*) and others with an ordinal of < 0, then we set it to an INTEGER instead
        FieldType aggValueType = (agg.ordinal >= 0) ?
                                 input_schema.getField(agg.ordinal).getType() : FieldType::SECURE_LONG ;
          aggregateFactory(agg.type, agg.ordinal, aggValueType);
    }


    // sorted on group-by cols
    assert(sortCompatible(input_sort, group_by_));


    Operator::output_schema_ = QuerySchema::toSecure(generateOutputSchema(input_schema, plain_aggregators_));

}


string GroupByAggregate::getOperatorType() const {
    return "GroupByAggregate";
}


string GroupByAggregate::getParameters() const {
    stringstream  ss;
   ss << "group-by: (" << group_by_[0];
   for(uint32_t i = 1; i < group_by_.size(); ++i)
       ss << ", " << group_by_[i];

   ss << ") aggs: (" << aggregate_definitions_[0].toString();

   for(uint32_t i = 1; i < aggregate_definitions_.size(); ++i) {
       ss << ", " << aggregate_definitions_[i].toString();
   }

   ss << ")";
   return ss.str();
}

GroupByAggregate::~GroupByAggregate() {
    for(auto *agg : secure_aggregators_) {
        delete agg;
    }

    for(auto *agg : plain_aggregators_) {
        delete agg;
    }


}


