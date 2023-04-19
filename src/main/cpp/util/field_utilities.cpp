#include <query_schema.h>
#include "field_utilities.h"
#include <query_table/plain_tuple.h>
#include <query_table/secure_tuple.h>

using namespace vaultdb;
using namespace  emp;

size_t FieldUtilities::getPhysicalSize(const FieldType &id, const size_t &strLength) {
    switch (id) {
        case FieldType::BOOL:
            return sizeof(bool); // stored size when we serialize it
        case FieldType::INT:
            return sizeof(int32_t);
        case FieldType::LONG:
            return sizeof(int64_t);
        case FieldType::FLOAT:
            return sizeof(float_t);
        case FieldType::STRING:
            return strLength;
        case FieldType::SECURE_BOOL:
            return sizeof(emp::block);
        case FieldType::SECURE_INT:
        case FieldType::SECURE_FLOAT:
            return sizeof(emp::block) * 32;
        case FieldType::SECURE_LONG:
            return sizeof(emp::block) * 64;

        case FieldType::SECURE_STRING:
            return sizeof(emp::block) * strLength * 8;
        case FieldType::INVALID:
        default: // unsupported type
            throw;

    }
}

emp::Float FieldUtilities::toFloat(const emp::Integer &input) {
    const Integer zero(32, 0, PUBLIC);
    const Integer one(32, 1, PUBLIC);
    const Integer maxInt(32, 1 << 24, PUBLIC); // 2^24
    const Integer minInt = Integer(32, -1 * (1 << 24), PUBLIC); // -2^24
    const Integer twentyThree(32, 23, PUBLIC);

    Float output(0.0, PUBLIC);

    Bit signBit = input.bits[31];
    Integer unsignedInput = input.abs();

    Integer firstOneIdx = Integer(32, 31, PUBLIC) - unsignedInput.leading_zeros().resize(32);

    Bit leftShift = firstOneIdx >= twentyThree;
    Integer shiftOffset = emp::If(leftShift, firstOneIdx - twentyThree, twentyThree - firstOneIdx);
    Integer shifted = emp::If(leftShift, unsignedInput >> shiftOffset, unsignedInput << shiftOffset);

    // exponent is biased by 127
    Integer exponent = firstOneIdx + Integer(32, 127, PUBLIC);
    // move exp to the right place in final output
    exponent = exponent << 23;

    Integer coefficient = shifted;
    // clear leading 1 (bit #23) (it will implicitly be there but not stored)
    coefficient.bits[23] = Bit(false, PUBLIC);


    // bitwise OR the sign bit | exp | coeff
    Integer outputInt(32, 0, PUBLIC);
    outputInt.bits[31] = signBit; // bit 31 is sign bit

    outputInt =  coefficient | exponent | outputInt;
    memcpy(&(output.value[0]), &(outputInt.bits[0]), 32 * sizeof(Bit));

    // cover the corner cases
    output = emp::If(input == zero, Float(0.0, PUBLIC), output);
    output = emp::If(input < minInt, Float(INT_MIN, PUBLIC), output);
    output = emp::If(input > maxInt, Float(INT_MAX, PUBLIC), output);

    return output;
}


void FieldUtilities::secret_share_send(const PlainTuple & src_tuple, SecureTuple & dst_tuple, const int & dst_party) {
    size_t field_count = dst_tuple.getSchema()->getFieldCount();

    for (size_t i = 0; i < field_count; ++i) {
        PlainField src_field = src_tuple.getField(i);
        SecureField dst_field = SecureField::secret_share_send(src_field, dst_party);
        dst_tuple.setField(i, dst_field);
    }

    PlainField plain_dummy_tag =  PlainField(src_tuple.getDummyTag());
    SecureField dummy_tag = SecureField::secret_share_send(plain_dummy_tag, dst_party);
    dst_tuple.setDummyTag(dummy_tag);


}

void FieldUtilities::secret_share_recv(SecureTuple &dst_tuple, const int &dst_party) {
    QuerySchema schema = *dst_tuple.getSchema();
    size_t field_count = schema.getFieldCount();

    for(size_t i = 0;  i < field_count; ++i) {
        QueryFieldDesc field_desc = schema.getField(i);
        SecureField  dst_field = SecureField::secret_share_recv(field_desc.getType(), field_desc.getStringLength(), dst_party);
        dst_tuple.setField(i, dst_field);
    }

    SecureField d = SecureField::secret_share_recv(FieldType::SECURE_BOOL, 0, dst_party);
    dst_tuple.setDummyTag(d);

}

// to compute sort
bool FieldUtilities::select(const bool &choice, const bool &lhs, const bool &rhs) {
    return choice ? lhs : rhs;
}

emp::Bit FieldUtilities::select(const Bit &choice, const Bit &lhs, const Bit &rhs) {
    return emp::If(choice, lhs, rhs);
}



