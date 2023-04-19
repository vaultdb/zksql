#ifndef _FIELD_FACTORY_H
#define _FIELD_FACTORY_H

#include <emp-tool/circuits/bit.h>
#include <emp-tool/circuits/integer.h>
#include <emp-tool/circuits/float32.h>
#include "field.h"


namespace vaultdb {


    template<typename B>
    class FieldFactory {

    };


    template<>
    class FieldFactory<bool> {
    public:
        static PlainField getFieldFromString(const FieldType &type, const size_t &strLength, const std::string &src);
        static PlainField getZero(const FieldType & type);
        static PlainField getOne(const FieldType & type);


        static PlainField toFloat(const PlainField &src);
        static PlainField toInt(const PlainField &src);
        static PlainField toLong(const PlainField &src);


        static PlainField getMin(const FieldType & type);
        static PlainField getMax(const FieldType & type);

        static PlainField getInt(const int32_t & src) { return PlainField(FieldType::INT, src); }
        static PlainField deserialize(const FieldType &type, const size_t &strLength, const int8_t *src);

    };


    template<>
    class FieldFactory<emp::Bit> {
    public:
        static SecureField getZero(const FieldType & type);
        static SecureField getOne(const FieldType & type);

        static SecureField toFloat(const SecureField &src);
        static SecureField toInt(const SecureField &src);
        static SecureField toLong(const SecureField &field);

        static SecureField getMin(const FieldType & type);
        static SecureField getMax(const FieldType & type);

        static SecureField getInt(const int32_t & src) { return  SecureField(FieldType::SECURE_INT, emp::Integer(32, src));   }
        static SecureField deserialize(const FieldType &type, const size_t &strLength, const emp::Bit *src);



    };

}

#endif
