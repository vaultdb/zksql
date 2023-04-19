#ifndef _QUERY_SCHEMA_H
#define _QUERY_SCHEMA_H

#include "query_field_desc.h"
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <ostream>

namespace  vaultdb {
    class QuerySchema {

    protected:

        std::vector<QueryFieldDesc> fields_;

    public:
        explicit QuerySchema(const size_t &num_fields);

        QuerySchema() {} // empty setup
        QuerySchema(const QuerySchema &s);

        size_t getFieldCount() const;

        void putField(const QueryFieldDesc &fd);

        // returns size in bits
        size_t size() const;


        QueryFieldDesc getField(const int &i) const;

        QueryFieldDesc getField(const std::string & fieldName) const;


        QuerySchema &operator=(const QuerySchema &other);

        size_t getFieldOffset(const int32_t idx) const;


        bool operator==(const QuerySchema &other) const;

        inline bool operator!=(const QuerySchema &other) const { return !(*this == other); }

        static QuerySchema toSecure(const QuerySchema &plainSchema);

        static QuerySchema toPlain(const QuerySchema &secureSchema);


    };

    std::ostream &operator<<(std::ostream &os, const QuerySchema &schema);

}

#endif // _QUERY_SCHEMA_H
