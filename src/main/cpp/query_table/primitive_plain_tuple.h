#ifndef _PRIMITIVE_PLAIN_TUPLE_H
#define _PRIMITIVE_PLAIN_TUPLE_H

#include <query_table/field/field_type.h>
#include <query_table/plain_tuple.h>

using namespace std;

// just a list of values, takes out memory-management piece for std::sort
namespace vaultdb {
    class PrimitivePlainTuple {

        public:
            vector<PlainField> payload_;
            bool dummy_tag_;
            shared_ptr<QuerySchema> schema_;

            PrimitivePlainTuple() = default;
            PrimitivePlainTuple(const PlainTuple & src);
            PlainField getField(const size_t & i) const;
            bool getDummyTag() const;
            PlainTuple getTuple() const; // generate a tuple with managed storage
            void writeToTuple(PlainTuple & dst) const;
            bool operator==(const PrimitivePlainTuple &other) const;


    };

}

#endif // _PRIMITIVE_PLAIN_TUPLE_H
