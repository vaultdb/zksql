#ifndef _TYPE_UTILITIES_H
#define _TYPE_UTILITIES_H

#include <string>

#include "query_table/field/field.h"


// N.B. Do not use namespace std here, it will cause a naming collision in emp

namespace vaultdb {
    class TypeUtilities {

    public:
        static std::string getTypeString(const FieldType & aTypeid);

        // logical size, hence secure bit will be 1 byte (byte-aligned). needs to be unified between encrypted and plain sizes for reveal/secret share methods
        // See FieldType::getPhysicalSize() for physical, allocated size
        static size_t getTypeSize(const FieldType & id);


        // when reading data from ascii sources like csv
        // Moved this to FieldFactory
        // static Field decodeStringValue(const std::string & strValue, const QueryFieldDesc &fieldSpec);

        static FieldType toSecure(const FieldType & plainType);
        static FieldType toPlain(const FieldType & secureType);
        static bool  isEncrypted(const FieldType & type);


        static bool types_equivalent(const FieldType &lhs, const FieldType &rhs);
    };

}

#endif // UTILITIES_H
