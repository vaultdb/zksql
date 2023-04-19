#include "hash_keyed_join.h"

using namespace vaultdb;

namespace std {
    template<>
    struct hash<PrimitivePlainTuple> {
        size_t operator()(const PrimitivePlainTuple &tuple) const {
            PlainTuple plainTuple = tuple.getTuple();

            size_t hashValue;

            int8_t tupleFields = *plainTuple.getData();
            hashValue = hash<int>()(tupleFields);
            return hashValue;
        }
    };
}

HashKeyedJoin::HashKeyedJoin(Operator *foreignKey, Operator *primaryKey, const BoolExpression<bool> & predicate, const vector<pair<int,int>> & joinKeys, const int & fkey, const SortDefinition & sort)
        : Join(foreignKey, primaryKey, predicate, sort),  foreign_key_input_((int32_t)fkey), keys(joinKeys) {
    assert(fkey == 0 || fkey == 1);
}

ZkQueryTable HashKeyedJoin::runSelf() {
    if(foreign_key_input_ == 0) {
        return foreignKeyPrimaryKeyHashJoin();
    }
    return primaryKeyForeignKeyHashJoin();
}

ZkQueryTable HashKeyedJoin::foreignKeyPrimaryKeyHashJoin() {
    ZkQueryTable lhs = children_[0]->getOutput();   // foreign key
    ZkQueryTable rhs = children_[1]->getOutput();  // primary key

    uint64_t commCostFromAllThreadsBefore = gv.getAccumulativeCommCostFromAllThreads();
    uint64_t commCostBefore = gv.getCommCost();

    startTime = clock_start();

    uint32_t outputTupleCount = lhs.getTupleCount() > rhs.getTupleCount() ? lhs.getTupleCount() : rhs.getTupleCount(); // foreignKeyTable = foreign key
    QuerySchema lhs_schema = *lhs.getSchema(); // secure schema
    QuerySchema rhs_schema = *rhs.getSchema();
    QuerySchema outputSchema = Join::concatenateSchemas(lhs_schema, rhs_schema);

    SortDefinition concatSorts;
    concatSorts = lhs.getSortOrder();
    SortDefinition  rhsSort = rhs.getSortOrder();

    for(size_t i = 0; i < rhsSort.size(); ++i) {
        rhsSort[i].first += lhs_schema.getFieldCount(); // add offset for lhs tuple
    }
    concatSorts.insert(concatSorts.end(),  rhsSort.begin(), rhsSort.end());

    assert(lhs.party_ == rhs.party_);
    ZkQueryTable tempTable = ZkQueryTable(outputTupleCount, outputSchema, lhs.party_, lhs.netio_, concatSorts);

    // temp table of lhs and rhs ready to join for predicate check computed locally by prover
    ZkQueryTable tempLhs = ZkQueryTable(outputTupleCount, lhs_schema, lhs.party_, lhs.netio_, lhs.getSortOrder());
    ZkQueryTable tempRhs = ZkQueryTable(outputTupleCount, rhs_schema, rhs.party_, rhs.netio_, rhs.getSortOrder());

    if(tempTable.party_ == ALICE) {
        // initialize all dummyTag in plaintable to 1 (all dummy)
        for(size_t i = 0; i < tempTable.plain_table_->getTupleCount(); ++i) {
            tempTable.plain_table_->getTuple(i).setDummyTag(true);
            tempLhs.plain_table_->getTuple(i).setDummyTag(true);
            tempRhs.plain_table_->getTuple(i).setDummyTag(true);
        }

        int tempTablePos = 0;

        int fieldOffset = lhs.plain_table_->getSchema()->getFieldCount();

        QuerySchema keySchema = QuerySchema(keys.size());
        for(size_t i = 0; i < keys.size(); ++i) {
            pair<int,int> keyPair = keys.at(i);
            QueryFieldDesc keyFieldDesc = rhs.plain_table_->getSchema()->getField(keyPair.second - fieldOffset);
            QueryFieldDesc dstKeyFieldDesc(keyFieldDesc, i);
            keySchema.putField(dstKeyFieldDesc);
        }

        unordered_map<PrimitivePlainTuple,PrimitivePlainTuple> lookup;

        // Build hash table
        for(int i = 0; i < rhs.getTupleCount(); ++i) {
            PlainTuple rhsTuple = rhs.plain_table_->getTuple(i);

            if(!rhsTuple.getDummyTag()) {
                PrimitivePlainTuple key = PrimitivePlainTuple(getJoinKeys(keySchema, rhsTuple, fieldOffset, false));
                PrimitivePlainTuple value = PrimitivePlainTuple(rhsTuple);
                lookup.insert(pair<PrimitivePlainTuple,PrimitivePlainTuple>(key,value));
            }
        }

        // Probe hash table
        for(int i = 0; i < lhs.getTupleCount(); ++i) {
            PlainTuple lhsTuple = lhs.plain_table_->getTuple(i);

            if(!lhsTuple.getDummyTag()) {
                PrimitivePlainTuple key = PrimitivePlainTuple(getJoinKeys(keySchema, lhsTuple, 0, true));

                auto matched = lookup.find(key);
                if (matched != lookup.end()) {
                    PlainTuple matchedTuple = matched->second.getTuple();

                    tempLhs.plain_table_->putTuple(tempTablePos, lhsTuple);
                    tempRhs.plain_table_->putTuple(tempTablePos, matchedTuple);
                    tempTablePos++;
                }
            }
        }

        output_ = ZkQueryTable(tempTable.plain_table_,lhs.netio_,outputTupleCount * outputSchema.size(),ALICE);
        tempLhs = ZkQueryTable(tempLhs.plain_table_, lhs.netio_, outputTupleCount * lhs_schema.size(), ALICE);
        tempRhs = ZkQueryTable(tempRhs.plain_table_, rhs.netio_, outputTupleCount * rhs_schema.size(), ALICE);
    }
    else {
        assert(tempTable.party_ == BOB);
        output_ = ZkQueryTable(*(tempTable.getSchema()),concatSorts,lhs.netio_,outputTupleCount * outputSchema.size(),BOB);
        tempLhs = ZkQueryTable(lhs_schema, lhs.getSortOrder(), lhs.netio_, outputTupleCount * lhs_schema.size(), BOB);
        tempRhs = ZkQueryTable(rhs_schema, rhs.getSortOrder(), rhs.netio_, outputTupleCount * rhs_schema.size(), BOB);
    }


    // Verification phase
    // this also writes the plain hash join output to output_
    verifyPredicate(tempLhs, tempRhs, outputSchema);


    ZkQueryTable padded_lhs = lhs;
    ZkQueryTable padded_rhs = rhs;

    if(lhs.getTupleCount() > rhs.getTupleCount()) {
        padded_rhs = DataUtilities::padZeros(rhs, lhs.getTupleCount() - rhs.getTupleCount());
    }

    if(lhs.getTupleCount() < rhs.getTupleCount()) {
        padded_lhs = DataUtilities::padZeros(lhs, rhs.getTupleCount() - lhs.getTupleCount());
    }

    // check ((R - \pi_R(T)) || \pi_R(T)) \equal R
    ZkQueryTable diff_rt = verifyEquality(output_, padded_lhs,true);

    // check ((S - \pi_S(T)) || \pi_S(T)) \equal S
    ZkQueryTable diff_st = verifyEquality(output_, padded_rhs, false, padded_lhs.getSchema()->getFieldCount());

    // check disjoint of \pi_KR(R - \pi_R(T)) and \pi_KS(S - \pi_S(T))
    verifyDisjoint(diff_rt,diff_st,keys,output_.party_);

    uint64_t commCostFromAllThreadsAfter = gv.getAccumulativeCommCostFromAllThreads();
    uint64_t costForJoin = commCostFromAllThreadsAfter - commCostFromAllThreadsBefore;
    comm_cost = costForJoin;
    gv.setCommCost(commCostBefore + costForJoin);

    return output_;

}

ZkQueryTable HashKeyedJoin::primaryKeyForeignKeyHashJoin() {
    ZkQueryTable lhs = children_[0]->getOutput();   // primary key
    ZkQueryTable rhs = children_[1]->getOutput();  // foreign key

    uint64_t commCostFromAllThreadsBefore = gv.getAccumulativeCommCostFromAllThreads();
    uint64_t commCostBefore = gv.getCommCost();

    startTime = clock_start();

    uint32_t outputTupleCount = rhs.getTupleCount() > lhs.getTupleCount() ? rhs.getTupleCount() : lhs.getTupleCount(); // foreignKeyTable = foreign key
    QuerySchema lhs_schema = *lhs.getSchema(); // secure schema
    QuerySchema rhs_schema = *rhs.getSchema();
    QuerySchema outputSchema = Join::concatenateSchemas(lhs_schema, rhs_schema);

    SortDefinition concatSorts;
    concatSorts = lhs.getSortOrder();
    SortDefinition rhsSort = rhs.getSortOrder();

    for(size_t i = 0; i < rhsSort.size(); ++i) {
        rhsSort[i].first += lhs_schema.getFieldCount(); // add offset for lhs tuple
    }
    concatSorts.insert(concatSorts.end(),  rhsSort.begin(), rhsSort.end());

    assert(lhs.party_ == rhs.party_);
    ZkQueryTable tempTable = ZkQueryTable(outputTupleCount, outputSchema, rhs.party_, rhs.netio_, concatSorts);

    // temp table of lhs and rhs ready to join for predicate check computed locally by prover
    ZkQueryTable tempLhs = ZkQueryTable(outputTupleCount, lhs_schema, lhs.party_, lhs.netio_, lhs.getSortOrder());
    ZkQueryTable tempRhs = ZkQueryTable(outputTupleCount, rhs_schema, rhs.party_, rhs.netio_, rhs.getSortOrder());

    if(tempTable.party_ == ALICE) {
        // initialize all dummyTag in plaintable to 1 (all dummy)
        for(size_t i = 0; i < tempTable.plain_table_->getTupleCount(); ++i) {
            tempTable.plain_table_->getTuple(i).setDummyTag(true);
            tempLhs.plain_table_->getTuple(i).setDummyTag(true);
            tempRhs.plain_table_->getTuple(i).setDummyTag(true);
        }

        int tempTablePos = 0;

        int fieldOffset = lhs.plain_table_->getSchema()->getFieldCount();

        QuerySchema keySchema = QuerySchema(keys.size());
        for(size_t i = 0; i < keys.size(); ++i) {
            pair<int,int> keyPair = keys.at(i);
            QueryFieldDesc keyFieldDesc = lhs.plain_table_->getSchema()->getField(keyPair.first);
            QueryFieldDesc dstKeyFieldDesc(keyFieldDesc, i);
            keySchema.putField(dstKeyFieldDesc);
        }

        unordered_map<PrimitivePlainTuple,PrimitivePlainTuple> lookup;

        // Build hash table
        for(int i = 0; i < lhs.getTupleCount(); ++i) {
            PlainTuple lhsTuple = lhs.plain_table_->getTuple(i);

            if(!lhsTuple.getDummyTag()) {
                PrimitivePlainTuple key = PrimitivePlainTuple(getJoinKeys(keySchema, lhsTuple, 0, true));
                PrimitivePlainTuple value = PrimitivePlainTuple(lhsTuple);
                lookup.insert(pair<PrimitivePlainTuple,PrimitivePlainTuple>(key,value));
            }
        }

        // Probe hash table
        for(int i = 0; i < rhs.getTupleCount(); ++i) {
            PlainTuple rhsTuple = rhs.plain_table_->getTuple(i);

            if(!rhsTuple.getDummyTag()) {
                PrimitivePlainTuple key = PrimitivePlainTuple(getJoinKeys(keySchema, rhsTuple, fieldOffset, false));

                auto matched = lookup.find(key);
                if (matched != lookup.end()) {
                    PlainTuple matchedTuple = matched->second.getTuple();

                    tempLhs.plain_table_->putTuple(tempTablePos, matchedTuple);
                    tempRhs.plain_table_->putTuple(tempTablePos, rhsTuple);
                    tempTablePos++;
                }
            }
        }

        output_ = ZkQueryTable(tempTable.plain_table_,rhs.netio_,outputTupleCount * outputSchema.size(),ALICE);
        tempLhs = ZkQueryTable(tempLhs.plain_table_, lhs.netio_, outputTupleCount * lhs_schema.size(), ALICE);
        tempRhs = ZkQueryTable(tempRhs.plain_table_, rhs.netio_, outputTupleCount * rhs_schema.size(), ALICE);
    }
    else {
        assert(tempTable.party_ == BOB);
        output_ = ZkQueryTable(*(tempTable.getSchema()),concatSorts,rhs.netio_,outputTupleCount * outputSchema.size(),BOB);
        tempLhs = ZkQueryTable(lhs_schema, lhs.getSortOrder(), lhs.netio_, outputTupleCount * lhs_schema.size(), BOB);
        tempRhs = ZkQueryTable(rhs_schema, rhs.getSortOrder(), rhs.netio_, outputTupleCount * rhs_schema.size(), BOB);
    }

    // Verification phase
    verifyPredicate(tempLhs, tempRhs, outputSchema);

    ZkQueryTable padded_lhs = lhs;
    ZkQueryTable padded_rhs = rhs;

    if(lhs.getTupleCount() > rhs.getTupleCount()) {
        padded_rhs = DataUtilities::padZeros(rhs, lhs.getTupleCount() - rhs.getTupleCount());
    }

    if(lhs.getTupleCount() < rhs.getTupleCount()) {
        padded_lhs = DataUtilities::padZeros(lhs, rhs.getTupleCount() - lhs.getTupleCount());
    }

    // check ((R - \pi_R(T)) || \pi_R(T)) \equal R
    ZkQueryTable diff_rt = verifyEquality(output_, padded_lhs, false);

    // check ((S - \pi_S(T)) || \pi_S(T)) \equal S
    ZkQueryTable diff_st = verifyEquality(output_, padded_rhs,true, padded_lhs.getSchema()->getFieldCount());

    // check disjoint of \pi_KR(R - \pi_R(T)) and \pi_KS(S - \pi_S(T))
    verifyDisjoint(diff_rt,diff_st,keys,output_.party_);

    uint64_t commCostFromAllThreadsAfter = gv.getAccumulativeCommCostFromAllThreads();
    uint64_t costForJoin = commCostFromAllThreadsAfter - commCostFromAllThreadsBefore;
    comm_cost = costForJoin;
    gv.setCommCost(commCostBefore + costForJoin);

    return output_;
}

PlainTuple HashKeyedJoin::getJoinKeys(QuerySchema &keySchema, PlainTuple &tuple, int offset, bool left) {
    PlainTuple joinKeyTuple = PlainTuple(keySchema);
    joinKeyTuple.setDummyTag(false);

    int dstIndex = 0;

    for(auto it = keys.begin(); it != keys.end(); ++it) {
        pair<int,int> keyPair = *it;
        PlainTuple::writeSubset(tuple,joinKeyTuple,left ? keyPair.first : (keyPair.second - offset),1,dstIndex++);
    }

    return joinKeyTuple;
}

string HashKeyedJoin::getOperatorType() const {
    return "HashKeyedJoin";
}