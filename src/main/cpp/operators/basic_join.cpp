#include "basic_join.h"
#include <operators/project.h>

using namespace vaultdb;


BasicJoin::BasicJoin(Operator *lhs, Operator *rhs, const BoolExpression<bool> & predicate, const SortDefinition & sort)
        : Join(lhs, rhs, predicate, sort) {}


BasicJoin::BasicJoin(const ZkQueryTable & lhs, const ZkQueryTable &  rhs, const BoolExpression<bool> & predicate, const SortDefinition & sort)
        : Join(lhs, rhs, predicate, sort) {}


ZkQueryTable BasicJoin::runSelf() {
    ZkQueryTable lhs = Join::children_[0]->getOutput();
    ZkQueryTable rhs = Join::children_[1]->getOutput();
    uint32_t cursor = 0;

    uint32_t outputTupleCount = lhs.getTupleCount() * rhs.getTupleCount();
    QuerySchema lhs_schema = *lhs.getSchema();
    QuerySchema rhs_schema = *rhs.getSchema();
    QuerySchema outputSchema = Join::concatenateSchemas(lhs_schema, rhs_schema);

    SortDefinition concat_sorts;
    concat_sorts = lhs.getSortOrder();
    SortDefinition  rhs_sort = rhs.getSortOrder();

    for(size_t i = 0; i < rhs_sort.size(); ++i) {
        rhs_sort[i].first += lhs_schema.getFieldCount(); // add offset for lhs tuple
    }
    concat_sorts.insert(concat_sorts.end(),  rhs_sort.begin(), rhs_sort.end());

    Join::output_ = ZkQueryTable(outputTupleCount, outputSchema, lhs.party_, lhs.netio_, concat_sorts);


    for(int i = 0; i < lhs.getTupleCount(); ++i) {
       SecureTuple lhs_secure = (*lhs.secure_table_)[i];
       PlainTuple lhs_plain = (output_.party_ == ALICE) ? (*lhs.plain_table_)[i] : PlainTuple();

        for(int j = 0; j < rhs.getTupleCount(); ++j) {
            if(output_.party_ == ALICE) {
                PlainTuple rhs_plain = (*rhs.plain_table_)[j];
                PlainTuple out_plain = (*output_.plain_table_)[cursor];
                PlainTuple::writeSubset(lhs_plain, out_plain, 0, lhs_schema.getFieldCount(), 0);
                PlainTuple::writeSubset(rhs_plain, out_plain, 0, rhs_schema.getFieldCount(), lhs_schema.getFieldCount());

                bool predicate_eval = plain_predicate_.callBoolExpression(out_plain);
                bool dst_dummy_tag = get_dummy_tag(lhs_plain, rhs_plain, predicate_eval);
                out_plain.setDummyTag(dst_dummy_tag);
            }

            SecureTuple rhs_secure = (*rhs.secure_table_)[j];
            SecureTuple out_secure = (*output_.secure_table_)[cursor];
            SecureTuple::writeSubset(lhs_secure, out_secure, 0, lhs_schema.getFieldCount(), 0);
            SecureTuple::writeSubset(rhs_secure, out_secure, 0, rhs_schema.getFieldCount(), lhs_schema.getFieldCount());

            Bit secure_predicate_eval = secure_predicate_.callBoolExpression(out_secure);
            Bit secure_dummy_tag = get_dummy_tag(lhs_secure, rhs_secure, secure_predicate_eval);
            out_secure.setDummyTag(secure_dummy_tag);

            // Join(R, S, predicate) == filter((cross_product(R, S), predicate)
            // |R| = m, |S| = n therefore m*n output tuples
            // Verification phase
            // check \pi_R(T) \subseteq R
            Bit equal_values_rt(true);
            for(size_t k = 0; k < lhs_secure.getFieldCount(); ++k) {
                equal_values_rt = equal_values_rt & (out_secure.getField(k) == lhs_secure.getField(k));
            }
            bool equality_check_rt = equal_values_rt.reveal(BOB);
            if(output_.party_ == BOB)
                assert(equality_check_rt);

            // check \pi_S(T) \subseteq S
            Bit equal_values_st(true);
            for(size_t k = 0; k < rhs_secure.getFieldCount(); ++k) {
                equal_values_st = equal_values_st & (out_secure.getField(lhs_secure.getFieldCount() + k) == rhs_secure.getField(k));
            }
            bool equality_check_st = equal_values_st.reveal(BOB);
            if(output_.party_ == BOB)
                assert(equality_check_st);

            // verify all of the tuples match the join condition
            Bit result(true);
            Bit check_dummy = (lhs_secure.getDummyTag() | rhs_secure.getDummyTag() | !secure_predicate_eval);
            Bit check_not_dummy = ((!secure_dummy_tag) & secure_predicate_eval & (!check_dummy));
            result = check_not_dummy | check_dummy;
            if(output_.party_ == BOB) {
                bool result_check = result.reveal(BOB);
                assert(result_check);
                if(!result_check)
                    throw std::invalid_argument("Join failed to prove correctness!");
            }

            ++cursor;
        }
    }

    return Join::output_;
}


string BasicJoin::getOperatorType() const {
    return "BasicJoin";
}


