#ifndef _JOIN_EQUALITY_PREDICATE_H
#define _JOIN_EQUALITY_PREDICATE_H

#include "binary_predicate.h"

// ordinals with respect to position in each source tuple
// lhs.ordinal == rhs.ordinal
typedef std::pair<size_t, size_t> EqualityPredicate;
typedef std::vector<EqualityPredicate> ConjunctiveEqualityPredicate;

// conjunctive equality predicate
// e.g., partsupp.suppkey = lineitem.suppkey AND partsupp.partkey = lineitem.partkey

namespace vaultdb {
    template<typename B>
    class JoinEqualityPredicate : public BinaryPredicate<B> {
    public:
        JoinEqualityPredicate(const ConjunctiveEqualityPredicate &srcPredicates);

        B predicateCall(const QueryTuple<B> & lhs, const QueryTuple<B> & rhs) const override;

    private:
        ConjunctiveEqualityPredicate predicate;


    };

}

#endif //_JOIN_EQUALITY_PREDICATE_H
