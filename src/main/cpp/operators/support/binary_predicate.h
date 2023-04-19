#ifndef _BINARY_PREDICATE_H
#define _BINARY_PREDICATE_H

#include <query_table/query_tuple.h>


using namespace vaultdb;
using namespace emp;

// predicates will inherit from this class
// it is a shell for creating state for the predicate, e.g., encrypted constants for equality checks
template<typename B>
class BinaryPredicate {

public:
    BinaryPredicate() {}
    virtual ~BinaryPredicate() {}

    // override when we instantiate a predicate -- only need to implement one of these per predicate class
    virtual B predicateCall(const QueryTuple<B> & lhs, const QueryTuple<B> & rhs) const = 0;

};

#endif //_BINARY_PREDICATE_H
