#ifndef _JOIN_H
#define _JOIN_H

#include <expression/bool_expression.h>
#include "operator.h"
#include <query_table/plain_tuple.h>
#include <query_table/secure_tuple.h>
#include <operators/project.h>
#include <unordered_map>
#include <query_table/primitive_plain_tuple.h>

namespace  vaultdb {
    class Join : public Operator {



    public:
        Join(Operator *lhs, Operator *rhs, const BoolExpression<bool> & predicate, const SortDefinition & sort = SortDefinition());
        Join(const ZkQueryTable &  lhs, const ZkQueryTable & rhs,  const BoolExpression<bool> & predicate, const SortDefinition & sort = SortDefinition());
        ~Join()  = default;

        // if B write is true, then write to the left side of an output tuple with src_tuple
        static void write_left(const bool & write, PlainTuple & dst_tuple, const PlainTuple & src_tuple);
        static void write_left(const emp::Bit & write, SecureTuple & dst_tuple, const SecureTuple & src_tuple);

        // if B write is true, then write to the right side of an output tuple with src_tuple
        static void write_right(const bool & write, PlainTuple & dst_tuple, const PlainTuple & src_tuple);
        static void write_right(const emp::Bit & write, SecureTuple & dst_tuple, const SecureTuple & src_tuple);



    protected:
        static QuerySchema concatenateSchemas(const QuerySchema &lhs_schema, const QuerySchema &rhs_schema);

        // current dummy_tag is the output of the current tuple comparison - derived from get_dummy_tag below
        // just splitting this off to make the code modular
        static void update_dummy_tag(QueryTuple<bool> & dst_tuple, const bool & predicate_matched, const bool & current_dummy_tag);
        static void update_dummy_tag(QueryTuple<emp::Bit> & dst_tuple, const emp::Bit & predicate_matched, const emp::Bit & current_dummy_tag);

        static bool get_dummy_tag(const PlainTuple &lhs, const PlainTuple &rhs, const bool & predicateEval);
        static Bit get_dummy_tag(const SecureTuple &lhs, const SecureTuple &rhs, const Bit & predicateEval);

        string getParameters() const override;

        // write unconditionally for the first round of keyed join
        // returns true if predicate is met
        bool join_tuples(const PlainTuple & lhs, const PlainTuple & rhs,  PlainTuple &output) const;
        Bit join_tuples(const SecureTuple & lhs, const SecureTuple & rhs, SecureTuple &output) const;

        ZkQueryTable zeroOutDupes(ZkQueryTable table);

        // helper function for set verification
        ZkQueryTable project(ZkQueryTable src, QuerySchema dstSchema, ExpressionMapBuilder<bool> builder, SortDefinition order, int tupleSize);
        ZkQueryTable hashSetDifference(const ZkQueryTable & lhs, const ZkQueryTable & rhs, int party);
        ZkQueryTable setConcatenation(const ZkQueryTable & lhs, const ZkQueryTable & rhs);

        ZkQueryTable sortByDummyTag(const ZkQueryTable & table);

        // verify join predicates
        void verifyPredicate(const ZkQueryTable & lhs, const ZkQueryTable & rhs, const QuerySchema& outputSchema);

        // verify set equality of concatenation of set difference and projection of joined table w.r.t. lhs or rhs table
        ZkQueryTable verifyEquality(const ZkQueryTable &lhs, const ZkQueryTable &rhs, bool isForeignKey, int offset = 0);

        // verify set disjoint of key projection of set difference
        void verifyDisjoint(const ZkQueryTable & lhs, const ZkQueryTable & rhs, vector<pair<int,int>> keys, int party);

        // predicate function needs aware of encrypted or plaintext state of its inputs
        // B = BoolField || SecureBoolField
        BoolExpression<bool>  plain_predicate_;
        BoolExpression<Bit>  secure_predicate_;


    };

}



#endif //_JOIN_H
