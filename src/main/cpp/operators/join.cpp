#include <util/type_utilities.h>
#include "join.h"
#include <expression/visitor/plain_to_secure_visitor.h>
#include <operators/sort.h>
#include <operators/union.h>
#include <util/data_utilities.h>
#include <util/zk_global_vars.h>

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

Join::Join(Operator *lhs, Operator *rhs,  const BoolExpression<bool> & predicate, const SortDefinition & sort) : Operator(lhs, rhs, sort), plain_predicate_(predicate),
                                                                                                                 secure_predicate_(nullptr) {
    QuerySchema lhs_schema = lhs->getOutputSchema();
    QuerySchema rhs_schema = rhs->getOutputSchema();
    output_schema_ = QuerySchema::toSecure(concatenateSchemas(lhs_schema, rhs_schema));

    PlainToSecureVisitor visitor(predicate.root_);
    secure_predicate_ = visitor.getSecureExpression();

}

Join::Join(const ZkQueryTable & lhs, const ZkQueryTable &  rhs,  const BoolExpression<bool> & predicate, const SortDefinition & sort) :  Operator(lhs, rhs, sort), plain_predicate_(predicate),
                                                                                                                                         secure_predicate_(nullptr) {
    QuerySchema lhs_schema = *(lhs.getSchema());
    QuerySchema rhs_schema = *(rhs.getSchema());
    output_schema_ = QuerySchema::toSecure(concatenateSchemas(lhs_schema, rhs_schema));
    PlainToSecureVisitor visitor(predicate.root_);
    secure_predicate_ = visitor.getSecureExpression();

}

QuerySchema Join::concatenateSchemas(const QuerySchema &lhs_schema, const QuerySchema &rhs_schema) {
    uint32_t output_cols = lhs_schema.getFieldCount() + rhs_schema.getFieldCount();
    QuerySchema result(output_cols);
    uint32_t cursor = lhs_schema.getFieldCount();

    for(uint32_t i = 0; i < lhs_schema.getFieldCount(); ++i) {
        QueryFieldDesc src_field = lhs_schema.getField(i);
        QueryFieldDesc dst_field(src_field, i);

        size_t srcStringLength = src_field.getStringLength();
        dst_field.setStringLength(srcStringLength);
        result.putField(dst_field);
    }


    for(uint32_t i = 0; i < rhs_schema.getFieldCount(); ++i) {
        QueryFieldDesc src_field = rhs_schema.getField(i);
        QueryFieldDesc dst_field(src_field, cursor);

        size_t srcStringLength = src_field.getStringLength();
        dst_field.setStringLength(srcStringLength);
        result.putField(dst_field);
        ++cursor;
    }


    return result;
}

bool Join::get_dummy_tag(const PlainTuple &lhs, const PlainTuple &rhs, const bool &predicateEval) {
    bool lhsDummyTag = lhs.getDummyTag();
    bool rhsDummyTag = rhs.getDummyTag();

    return (!predicateEval) | lhsDummyTag | rhsDummyTag;
}


Bit Join::get_dummy_tag(const SecureTuple &lhs, const SecureTuple &rhs, const Bit &predicateEval) {
    Bit lhsDummyTag = lhs.getDummyTag();
    Bit rhsDummyTag = rhs.getDummyTag();

    return (!predicateEval) | lhsDummyTag | rhsDummyTag;
}


void Join::write_left(const bool &write, PlainTuple &dst_tuple, const PlainTuple &src_tuple) {
    if(write) {
        size_t tuple_size = src_tuple.getSchema()->size()/8 - 1; // -1 for dummy tag
        memcpy(dst_tuple.getData(), src_tuple.getData(), tuple_size);
    }
}


void Join::write_left(const emp::Bit &write, SecureTuple &dst_tuple, const SecureTuple &src_tuple) {
    size_t write_bit_cnt = src_tuple.getSchema()->size() - 1; // -1 for dummy tag
    size_t write_size = write_bit_cnt * sizeof(emp::block);

    emp::Integer src(write_bit_cnt, 0), dst(write_bit_cnt, 0);

    memcpy(src.bits.data(), src_tuple.getData(), write_size);
    memcpy(dst.bits.data(), dst_tuple.getData(), write_size);

    dst = dst.select(write, src);

    memcpy(dst_tuple.getData(), dst.bits.data(), write_size);

}

void Join::write_right(const bool &write, PlainTuple &dst_tuple, const PlainTuple &src_tuple) {
    if(write) {
        size_t write_size = src_tuple.getSchema()->size()/8 - sizeof(bool); // don't overwrite dummy tag
        size_t dst_byte_cnt = dst_tuple.getSchema()->size()/8;
        size_t write_offset = dst_byte_cnt - write_size - 1;

        memcpy(dst_tuple.getData() + write_offset, src_tuple.getData(), write_size);
    }
}


void Join::write_right(const emp::Bit &write, SecureTuple &dst_tuple, const SecureTuple &src_tuple) {
    size_t write_bit_cnt = src_tuple.getSchema()->size() - 1; // don't overwrite dummy tag
    size_t dst_bit_cnt = dst_tuple.getSchema()->size() - 1;
    size_t write_offset = dst_bit_cnt - write_bit_cnt;
    size_t write_size = write_bit_cnt * sizeof(emp::block);


    emp::Integer src(write_bit_cnt, 0), dst(write_bit_cnt, 0);

    memcpy(src.bits.data(), src_tuple.getData(), write_size);
    memcpy(dst.bits.data(), dst_tuple.getData() + write_offset, write_size);


    dst = dst.select(write, src);

    memcpy(dst_tuple.getData() + write_offset, dst.bits.data(), write_size);



}


string Join::getParameters() const {
    return plain_predicate_.root_->toString();

}

void Join::update_dummy_tag(QueryTuple<bool> &dst_tuple, const bool &predicate_matched, const bool & current_dummy_tag) {
    if(predicate_matched)
        dst_tuple.setDummyTag(current_dummy_tag);
}


void Join::update_dummy_tag(QueryTuple<emp::Bit> &dst_tuple, const emp::Bit &predicate_matched, const emp::Bit & current_dummy_tag) {
    emp::Bit dummy_tag = emp::If(predicate_matched, current_dummy_tag, dst_tuple.getDummyTag());
    dst_tuple.setDummyTag(dummy_tag);

}


bool Join::join_tuples(const PlainTuple & lhs, const PlainTuple & rhs,  PlainTuple &output) const {

    PlainTuple::writeSubset(lhs, output, 0, lhs.getFieldCount(), 0);
    PlainTuple::writeSubset(rhs, output, 0, rhs.getFieldCount(), lhs.getFieldCount());

    bool predicate_result = plain_predicate_.callBoolExpression(output);
    bool dummy_tag = get_dummy_tag(lhs, rhs, predicate_result);

    output.setDummyTag(dummy_tag);

    return predicate_result;
}


Bit Join::join_tuples(const SecureTuple & lhs, const SecureTuple & rhs, SecureTuple &output) const {

    SecureTuple::writeSubset(lhs, output, 0, lhs.getFieldCount(), 0);
    SecureTuple::writeSubset(rhs, output, 0, rhs.getFieldCount(), lhs.getFieldCount());

    Bit predicate_result = secure_predicate_.callBoolExpression(output);
    Bit dummy_tag = get_dummy_tag(lhs, rhs, predicate_result);

    output.setDummyTag(dummy_tag);
    return predicate_result;
}

template<class B>
void zeroOutDupesSortedArray(shared_ptr<QueryTable<B> > input) {
    if(input.get() == nullptr) return; // empty means that it's likely BOB/verifier
    // check that it has been sorted
    SortDefinition sort_def = input->getSortOrder();
    size_t col_count = input->getSchema()->getFieldCount();
    assert(sort_def.size() == col_count + 1); // inc. dummy tag
    assert(sort_def[0].first == -1);

    // 0, 1, ..., col_count - 1
    for(int i = 1; i < (int) col_count; ++i) {
        assert(sort_def[i].first == (i-1));
    }

    QueryTuple<B> last_row(*input->getSchema()); // set up managed storage, deep copies
    last_row = (*input)[0]; // copy to managed storage

    for(size_t i =1; i < input->getTupleCount(); ++i) {
        QueryTuple<B> current_row((*input)[i]);
        B eq = (last_row == current_row);
        B dt = current_row.getDummyTag();
        B to_clear = eq | dt;
        last_row = current_row;
        current_row.clear(to_clear);
    }

}

ZkQueryTable Join::zeroOutDupes(ZkQueryTable table) {
    auto sorted = sortByDummyTag(table);

    if(table.party_ == ALICE) {
        zeroOutDupesSortedArray(sorted.plain_table_);
    }
    zeroOutDupesSortedArray(sorted.secure_table_);

    return sorted;

}

ZkQueryTable Join::project(ZkQueryTable src, QuerySchema dstSchema, ExpressionMapBuilder<bool> builder, SortDefinition order, int tupleSize) {
    Project proj(src, builder.getExprs());
    proj.setOperatorId(-1);

    return proj.run();

}

ZkQueryTable Join::hashSetDifference(const ZkQueryTable &lhs, const ZkQueryTable &rhs, int party) {
    ZkQueryTable deltaTable;

    if(party == ALICE) {
        shared_ptr<PlainTable> delta(new PlainTable (lhs.getTupleCount(), *lhs.plain_table_->getSchema(), lhs.getSortOrder()));
        int deltaCursor = 0;

        // initialize all dummyTag in plaintable to 1 (all dummy)
        for(unsigned int i = 0; i < delta->getTupleCount(); ++i) {
            delta->getTuple(i).setDummyTag(true);
        }

        // build hash table for rhs
        unordered_map<PrimitivePlainTuple,bool> lookup;

        for(int i = 0; i < rhs.getTupleCount(); ++i) {
            PlainTuple curRhsPlainTuple = rhs.plain_table_->getTuple(i);

            if(!curRhsPlainTuple.getDummyTag()) {
                PrimitivePlainTuple key = PrimitivePlainTuple(curRhsPlainTuple);

                lookup.insert(pair<PrimitivePlainTuple, bool>(key, true));
            }
        }

        // probe the hash table to check if lhs tuples can find corresponding rhs tuples
        for(int i = 0; i < lhs.getTupleCount(); ++i) {
            PlainTuple curLhsPlainTuple = lhs.plain_table_->getTuple(i);

            if(!curLhsPlainTuple.getDummyTag()) {
                auto matched = lookup.find(PrimitivePlainTuple(curLhsPlainTuple));

                if(matched == lookup.end()) {
                    delta->putTuple(deltaCursor++, curLhsPlainTuple);
                }
            }
        }

        deltaTable = ZkQueryTable(delta, lhs.netio_, lhs.getTupleCount() * lhs.getSchema()->size(), ALICE);
    }
    else {
        assert(party == BOB);
        deltaTable = ZkQueryTable(*lhs.getSchema(), lhs.getSortOrder(), lhs.netio_, lhs.getTupleCount() * lhs.getSchema()->size(), BOB);
    }

    return deltaTable;
}

ZkQueryTable Join::setConcatenation(const ZkQueryTable &lhs, const ZkQueryTable &rhs) {
    Union unioner(lhs, rhs);
    unioner.setOperatorId(-1);
    ZkQueryTable unioned = unioner.run();
    return unioned;

}

ZkQueryTable Join::sortByDummyTag(const ZkQueryTable &table) {
    SortDefinition table_sort_def;
    table_sort_def.push_back(pair<int32_t, SortDirection>(-1, SortDirection::ASCENDING));
    for(size_t i = 0; i < table.getSchema()->getFieldCount(); i++) {
        table_sort_def.push_back(pair<int32_t, SortDirection>(i, SortDirection::ASCENDING));
    }
    Sort table_sort(table, table_sort_def);
    table_sort.setOperatorId(-1);
    return  table_sort.run();
}

void Join::verifyPredicate(const ZkQueryTable &lhs, const ZkQueryTable &rhs, const QuerySchema& outputSchema) {
    Bit matched(true);

    for(int i = 0; i < lhs.getTupleCount(); i++) {
        if(lhs.party_ == ALICE) {
            PlainTuple lhsPlainTuple = lhs.plain_table_->getTuple(i);
            PlainTuple rhsPlainTuple = rhs.plain_table_->getTuple(i);
            PlainTuple outPlainTuple = (*output_.plain_table_)[i];

           join_tuples(lhsPlainTuple, rhsPlainTuple, outPlainTuple);
        }
        SecureTuple lhsSecureTuple = lhs.secure_table_->getTuple(i);
        SecureTuple rhsSecureTuple = rhs.secure_table_->getTuple(i);
        SecureTuple outputTuple = (*output_.secure_table_)[i];

        Bit joinedInSecure = join_tuples(lhsSecureTuple, rhsSecureTuple, outputTuple);
        matched = matched & joinedInSecure;
    }

    if(lhs.party_ == BOB) {
        bool check = matched.reveal(BOB);
        assert(check);
        if(!check)
            throw std::invalid_argument("Join failed to prove correctness!");

    }
    else {
        matched.reveal(BOB);
    }
}

ZkQueryTable Join::verifyEquality(const ZkQueryTable &lhs, const ZkQueryTable &rhs, bool isForeignKey, int offset) {
    QuerySchema lhsSchema = *lhs.getSchema();
    QuerySchema rhsSchema = *rhs.getSchema();

    // Build \pi_R(T)
    ExpressionMapBuilder<bool> builder_rt(lhsSchema);
    for(size_t i = 0; i < rhsSchema.getFieldCount(); ++i) {
        builder_rt.addMapping(offset + i, i);
    }

    ZkQueryTable projected_t = project(lhs, rhsSchema, builder_rt, rhs.getSortOrder(), lhs.getTupleCount());

    projected_t = isForeignKey ? projected_t : zeroOutDupes(projected_t);

    // Delta_R = R - \pi_R(T)
    ZkQueryTable delta_rhs = hashSetDifference(rhs, projected_t, projected_t.party_);

    // (R - \pi_R(T)) || \pi_R(T)
    // there should be no duplicates in this set
    ZkQueryTable concated_rhs = setConcatenation(delta_rhs, projected_t);

    ZkQueryTable padded_rhs = DataUtilities::padZeros(rhs, rhs.getTupleCount());

    ZKGlobalVars& gv = ZKGlobalVars::getInstance();
    ZKSet<BoolIO<NetIO>> *zkset = gv.getZKSet();
    bool check = zkset->equal(reinterpret_cast<Bit*>(concated_rhs.secure_table_->tuple_data_.data()),
                              reinterpret_cast<Bit*>(padded_rhs.secure_table_->tuple_data_.data()),
                              padded_rhs.getTupleCount(),rhsSchema.size());

    if(lhs.party_ == BOB) {
        assert(check);
        if(!check)
            throw std::invalid_argument("Join failed to prove correctness!");

    }

    return delta_rhs;
}

void Join::verifyDisjoint(const ZkQueryTable &lhs, const ZkQueryTable &rhs, vector<pair<int, int>> keys, int party) {
    int offset = lhs.secure_table_->getSchema()->getFieldCount();

    QuerySchema lhsSchema = *lhs.getSchema();
    QuerySchema rhsSchema = *rhs.getSchema();

    QuerySchema leftKeySchema = QuerySchema(keys.size());
    QuerySchema rightKeySchema = QuerySchema(keys.size());

    ExpressionMapBuilder<bool> lhs_builder(lhsSchema);
    ExpressionMapBuilder<bool> rhs_builder(rhsSchema);

    for(unsigned int i = 0; i < keys.size(); ++i) {
        pair<int,int> keyPair = keys.at(i);
        leftKeySchema.putField(QueryFieldDesc(lhsSchema.getField(keyPair.first), i));
        rightKeySchema.putField(QueryFieldDesc(rhsSchema.getField(keyPair.second - offset), i));

        lhs_builder.addMapping(keyPair.first, i);
        rhs_builder.addMapping(keyPair.second - offset, i);
    }

    ZkQueryTable projected_lhs = project(lhs, leftKeySchema, lhs_builder, lhs.getSortOrder(), lhs.getTupleCount());

    ZkQueryTable projected_rhs = project(rhs, rightKeySchema, rhs_builder, rhs.getSortOrder(), rhs.getTupleCount());

    ZKGlobalVars& gv = ZKGlobalVars::getInstance();
    ZKSet<BoolIO<NetIO>> *zkset = gv.getZKSet();
    bool check = zkset->disjoint(reinterpret_cast<Bit*>(projected_lhs.secure_table_->tuple_data_.data()),
                                 reinterpret_cast<Bit*>(projected_rhs.secure_table_->tuple_data_.data()),
                                 projected_lhs.getTupleCount(),
                                 projected_rhs.getTupleCount(),
                                 projected_lhs.getSchema()->size());

    if(lhs.party_ == BOB) {
        assert(check);
        if(!check)
            throw std::invalid_argument("Join failed to prove correctness!");

    }
}
