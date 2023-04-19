#include "field_factory.h"

#include "boost/date_time/gregorian/gregorian.hpp"
#include <util/type_utilities.h>
#include <util/field_utilities.h>
#include <float.h>


using namespace vaultdb;

PlainField FieldFactory<bool>::getFieldFromString(const FieldType &type, const size_t &strLength, const std::string &src) {
    switch (type) {
        case FieldType::BOOL: {
            bool boolField = (src == "1") ? true : false;
            return  PlainField(type, boolField);
        }

        case FieldType::INT: {
            int32_t intField = std::atoi(src.c_str());
            return PlainField(type, intField);
        }
        case FieldType::LONG: {
            int64_t intField = std::atol(src.c_str());
            return PlainField(type, intField);
        }
        case FieldType::STRING: {
            std::string fieldStr = src;
            while(fieldStr.length() < strLength) {
                fieldStr += " ";
            }
            return PlainField(type, fieldStr, strLength);

        }
        case FieldType::FLOAT: {
            float_t floatField = std::atof(src.c_str());
            return PlainField(type, floatField);
        }
        case FieldType::DATE: {
            std::tm timeStruct = {};
            strptime(src.c_str(), "%Y-%m-%d", &timeStruct);
            int64_t epoch = mktime(&timeStruct) - 21600; // date time function is 6 hours off from how psql does it
            return  PlainField(FieldType::LONG, epoch);

        }
        default:
            throw std::invalid_argument("Unsupported type for string decoding: " + TypeUtilities::getTypeString(type) + "\n");


    };
}



PlainField FieldFactory<bool>::getZero(const FieldType &aType) {
    switch(aType) {
        case FieldType::BOOL:
            return  Field<bool>(FieldType::BOOL, false);
        case FieldType::INT:
            return  Field<bool>(aType, (int32_t) 0);
        case FieldType::DATE:
        case FieldType::LONG: {
            return  Field<bool>(aType,  (int64_t) 0L);
        }
        case FieldType::FLOAT:
            return Field<bool>(aType, (float_t) 0.0);
        case FieldType::STRING: {
            std::string zero = "0";
            return Field<bool>(aType, zero, 1);
        }
        default:
            throw std::invalid_argument("Type unsupported in getZero(): " + TypeUtilities::getTypeString(aType));

    };
}

PlainField FieldFactory<bool>::getOne(const FieldType &aType) {
    switch(aType) {
        case FieldType::BOOL:
            return  Field<bool>(FieldType::BOOL, true);
        case FieldType::INT:
            return  Field<bool>(aType, (int32_t) 1);
        case FieldType::DATE:
        case FieldType::LONG:
            return  Field<bool>(aType, (int64_t) 1L);
        case FieldType::FLOAT:
            return Field<bool>(aType, (float_t) 1.0);
        case FieldType::STRING: {
            std::string one = "1";
            return Field<bool>(aType, one, 1);
        }
        default:
            throw std::invalid_argument("Type unsupported in getOne(): " + TypeUtilities::getTypeString(aType));

    };
}

PlainField FieldFactory<bool>::toFloat(const PlainField &src) {
    switch(src.getType()) {
        case FieldType::BOOL:
            return PlainField(FieldType::FLOAT, (float_t)  src.getValue<bool>());
        case FieldType::INT:
            return PlainField(FieldType::FLOAT, (float_t)  src.getValue<int32_t>());
        case FieldType::LONG:
            return PlainField(FieldType::FLOAT, (float_t)  src.getValue<int64_t>());
        case FieldType::FLOAT:
            return PlainField(src);

        default:
            throw std::invalid_argument("Cannot convert value of type " +
                                        TypeUtilities::getTypeString(src.getType()) + " to float.");
    }

}

PlainField FieldFactory<bool>::toLong(const PlainField & field) {
    if(field.getType() == FieldType::INT) {
        int32_t payload = field.getValue<int32_t>();
        return PlainField(FieldType::LONG, (int64_t) payload);
    }
    throw std::invalid_argument("toLong not supported for " + TypeUtilities::getTypeString(field.getType()));

}

PlainField FieldFactory<bool>::toInt(const PlainField &src) {
    switch(src.getType()) {
        case FieldType::BOOL: {
            int32_t value = src.getValue<bool>();
            return PlainField(FieldType::INT, value);
        }
        case FieldType::INT: {
            return PlainField(src);
        }
        case FieldType::LONG: {
            int32_t value = src.getValue<int64_t>();
            return PlainField(FieldType::INT, value);
        }
        case FieldType::FLOAT: {
            int32_t value = src.getValue<float_t>();
            return PlainField(FieldType::INT, value);
        }
        default:
            throw std::invalid_argument("Type " + TypeUtilities::getTypeString(src.getType()) + " not supported by FieldFactory<bool>::toInt()");
    }

}


PlainField FieldFactory<bool>::getMin(const FieldType & type) {
    switch(type) {
        case FieldType::BOOL:
            return PlainField(type, false);
        case FieldType::INT:
            return PlainField(type, (int32_t) INT_MIN);
        case FieldType::LONG:
            return  PlainField(type, (int64_t) LONG_MIN);
        case FieldType::FLOAT:
            return PlainField(type, (float_t) FLT_MIN);
        default:
            throw std::invalid_argument("Type " + TypeUtilities::getTypeString(type) + " not supported by FieldFactory<bool>::getMin()");
    }

}

PlainField FieldFactory<bool>::getMax(const FieldType & type) {
    switch(type) {
        case FieldType::BOOL:
            return PlainField(type, true);
        case FieldType::INT:
            return PlainField(type, (int32_t) INT_MAX);
        case FieldType::LONG:
            return  PlainField(type, (int64_t) LONG_MAX);
        case FieldType::FLOAT:
            return PlainField(type, (float_t) FLT_MAX);
        default:
            throw std::invalid_argument("Type " + TypeUtilities::getTypeString(type) + " not supported by FieldFactory<bool>::getMax()");
    }


}

PlainField FieldFactory<bool>::deserialize(const FieldType &type, const size_t &strLength, const int8_t *src) {
    switch (type) {
        case FieldType::BOOL: {
            bool val = *((bool *) src);
            return PlainField(type, val);
        }
        case FieldType::INT: {
            int32_t val = *((int32_t *) src);
            return PlainField(type, val);
        }
        case FieldType::DATE:
        case FieldType::LONG: {
            int64_t val = *((int64_t *) src);
            return PlainField(type, val);
        }
        case FieldType::FLOAT: {
            float_t val = *((float_t *) src);
            return PlainField(type, val);
        }

        case FieldType::STRING: {
            char *val = (char *) src;
            std::string str(val, strLength);
            std::reverse(str.begin(), str.end());
            return PlainField(type, str, strLength);
        }
        default:
            throw std::invalid_argument("Field type " + TypeUtilities::getTypeString(type) + " not supported by FieldFactory<bool>::deserialize()!");
    }
}



// ************  Start SecureBoolField *************/

SecureField FieldFactory<emp::Bit>::getZero(const FieldType &aType) {
    switch(aType) {
        case FieldType::SECURE_BOOL:
            return  SecureField(aType, emp::Bit(false));
        case FieldType::SECURE_INT:
            return  SecureField(aType, emp::Integer(32, 0));
        case FieldType::SECURE_LONG:
            return  SecureField(aType, emp::Integer(64, 0));
        case FieldType::SECURE_FLOAT:
            return SecureField(aType, emp::Float(0.0));
        case FieldType::SECURE_STRING: {
            return SecureField(aType, emp::Integer(8, 0), 1); // single character
        }
        default:
            throw std::invalid_argument("Type unsupported in getZero(): " + TypeUtilities::getTypeString(aType));

    };
}

SecureField FieldFactory<emp::Bit>::getOne(const FieldType &aType) {
    switch(aType) {
        case FieldType::SECURE_BOOL:
            return  SecureField(aType, emp::Bit(true));
        case FieldType::SECURE_INT:
            return  SecureField(aType, emp::Integer(32, 1));
        case FieldType::SECURE_LONG:
            return  SecureField(aType, emp::Integer(64, 1));
        case FieldType::SECURE_FLOAT:
            return SecureField(aType, emp::Float(1.0));
        case FieldType::SECURE_STRING: {
            return SecureField(aType, emp::Integer(8, 1), 1);
        }
        default:
            throw std::invalid_argument("Type unsupported in getOne(): " + TypeUtilities::getTypeString(aType));

    };
}


SecureField FieldFactory<emp::Bit>::toFloat(const SecureField &src) {

    switch (src.getType()) {
        case FieldType::SECURE_INT:
        case FieldType::SECURE_LONG: {
            emp::Integer i = src.getValue<emp::Integer>();
            emp::Float f = FieldUtilities::toFloat(i);
            return SecureField(FieldType::SECURE_FLOAT, f);
        }
        case FieldType::SECURE_FLOAT:
            return SecureField(src); // copy constructor
        default:
            throw std::invalid_argument("Cannot convert value of type " +
                                        TypeUtilities::getTypeString(src.getType()) + " to emp::Float.");


    }
}

SecureField FieldFactory<emp::Bit>::toLong(const SecureField & field) {
    if(field.getType() == FieldType::SECURE_INT) {
        emp::Integer payload = field.getValue<emp::Integer>();
        payload.resize(64);
        return SecureField(FieldType::SECURE_LONG, payload);
    }
    throw std::invalid_argument("toLong not supported for " + TypeUtilities::getTypeString(field.getType()));

}

SecureField FieldFactory<emp::Bit>::toInt(const SecureField &src) {
    switch(src.getType()) {
        case FieldType::SECURE_BOOL: {
            emp::Integer dst(32, 0);
            dst[0] = src.getValue<emp::Bit>();
            return SecureField(FieldType::SECURE_INT, dst);
        }
        case FieldType::SECURE_INT: {
            return SecureField(FieldType::SECURE_INT, src.getValue<emp::Integer>());
        }
        case FieldType::SECURE_LONG: {
            emp::Integer value = src.getValue<emp::Integer>();
            value.resize(32);
            return SecureField(FieldType::SECURE_INT, value);
        }
        default:
            throw std::invalid_argument("Type " + TypeUtilities::getTypeString(src.getType()) + " not supported by FieldFactory<bool>::toInt()");
    }

}

SecureField FieldFactory<emp::Bit>::deserialize(const FieldType &type, const size_t &strLength, const emp::Bit *src) {
    {
        switch (type) {
            case FieldType::SECURE_BOOL: {
                emp::Bit myBit(*(emp::block *) src);
                return SecureField(type, myBit);
            }
            case FieldType::SECURE_INT: {
                emp::Integer payload(32, 0);
                memcpy(payload.bits.data(), src, 32*sizeof(emp::Bit));
                return SecureField(type, payload);
            }
            case FieldType::SECURE_LONG: {
                emp::Integer payload(64, 0);
                memcpy(payload.bits.data(), src, 64*sizeof(emp::Bit));
                return SecureField(type, payload);
            }
            case FieldType::SECURE_FLOAT: {
                emp::Float v(0, emp::PUBLIC);
                memcpy(v.value.data(), src, 32*sizeof(emp::Bit) );
                return SecureField(type, v);
            }

            case FieldType::SECURE_STRING: {
                size_t bitCount = strLength * 8;

                emp::Integer v(bitCount, 0, emp::PUBLIC);
                memcpy(v.bits.data(), src, sizeof(emp::Bit)*bitCount);
                return SecureField(type, v, bitCount / 8);

            }
            default:
                throw std::invalid_argument("Field type " + TypeUtilities::getTypeString(type) + " not supported by FieldFactory<emp::Bit>::deserialize()!");
        }

    }
}


SecureField FieldFactory<emp::Bit>::getMin(const FieldType & type) {
    switch (type) {
        case FieldType::SECURE_BOOL:
            return SecureField(type, emp::Bit(false));
        case FieldType::SECURE_INT:
            return SecureField(type, emp::Integer(32, INT_MIN));
        case FieldType::SECURE_LONG:
            return SecureField(type, emp::Integer(64, LONG_MIN));
        case FieldType::SECURE_FLOAT:
            return SecureField(type, emp::Float(FLT_MIN));
        default:
            throw std::invalid_argument("Type " + TypeUtilities::getTypeString(type) +
                                        " not supported by FieldFactory<emp::Bit>::getMin()");
    }
}


SecureField FieldFactory<emp::Bit>::getMax(const FieldType & type) {
    switch (type) {
        case FieldType::SECURE_BOOL:
            return SecureField(type, emp::Bit(true));
        case FieldType::SECURE_INT:
            return SecureField(type, emp::Integer(32, INT_MAX));
        case FieldType::SECURE_LONG:
            return SecureField(type, emp::Integer(64, LONG_MAX));
        case FieldType::SECURE_FLOAT:
            return SecureField(type, emp::Float(FLT_MAX));
        default:
            throw std::invalid_argument("Type " + TypeUtilities::getTypeString(type) +
                                        " not supported by FieldFactory<emp::Bit>::getMax()");
    }
}





