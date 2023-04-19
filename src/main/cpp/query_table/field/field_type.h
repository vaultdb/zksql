#ifndef _FIELDTYPE_H
#define _FIELDTYPE_H
#include <boost/variant.hpp>
#include <emp-tool/circuits/bit.h>
#include <emp-tool/circuits/integer.h>
#include <emp-tool/circuits/float32.h>


namespace vaultdb {
    enum class FieldType {
        INVALID = 0,
        BOOL,
        DATE, // DATE is just a stand-in for long
        INT,
        LONG,
        FLOAT,
        STRING,
        SECURE_INT,
        SECURE_LONG,
        SECURE_BOOL,
        SECURE_FLOAT,
        SECURE_STRING, // need all types to have encrypted counterpart so that we can translate them back to query tables when we decrypt the results
        UNKNOWN // for lazy schema evaluation after we evaluate an expression for the first time
    };


    typedef boost::variant<bool, int32_t, int64_t, float_t, std::string, emp::Bit, emp::Integer, emp::Float> Value;
}

#endif //_FIELDTYPE_H
