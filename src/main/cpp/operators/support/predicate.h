#ifndef _PREDICATE_CLASS_H
#define _PREDICATE_CLASS_H

#include <query_table/query_tuple.h>

using namespace emp;

// predicates will inherit from this class
// it is a shell for creating state for the predicate, e.g., encrypted constants for equality checks
namespace vaultdb {
    template<typename  B>
    class Predicate {

    public:
        Predicate() {}
        virtual ~Predicate() {}
        // override when we instantiate a predicate
        virtual B predicateCall(const QueryTuple<B> & aTuple) const = 0;

    };

    template class Predicate<bool>;
    template class Predicate<emp::Bit>;

}

#endif //_PREDICATE_CLASS_H
