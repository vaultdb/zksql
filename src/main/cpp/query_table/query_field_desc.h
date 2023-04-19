#ifndef _QUERY_FIELD_DESC_H
#define _QUERY_FIELD_DESC_H

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include <ostream>
#include <query_table/field/field_type.h>

namespace  vaultdb {
    class QueryFieldDesc {

    protected:
        std::string name_;
        std::string table_name;
        size_t string_length_; // for varchars
        FieldType type_;
        int ordinal_;
        
    public:
        QueryFieldDesc();

        [[nodiscard]] int getOrdinal() const;


        [[nodiscard]] const std::string &getName() const;

        [[nodiscard]] FieldType getType() const;

        [[nodiscard]] const std::string &getTableName() const;

        [[nodiscard]] size_t size() const;

        QueryFieldDesc(const QueryFieldDesc &f) = default;

        QueryFieldDesc(const QueryFieldDesc &f, const FieldType & type)
                : name_(f.name_), table_name(f.table_name),
                  string_length_(f.getStringLength()), type_(type), ordinal_(f.ordinal_) {}

        QueryFieldDesc(const QueryFieldDesc &f, int col_num)
                : name_(f.name_), table_name(f.table_name), string_length_(f.string_length_), type_(f.type_), ordinal_(col_num) {}

        QueryFieldDesc(uint32_t anOrdinal, const std::string &n, const std::string &tab, const FieldType &aType, const size_t & stringLength = 0)
                : name_(n),
                  table_name(tab),  string_length_(stringLength), type_(aType), ordinal_(anOrdinal) {
            // since we convert DATEs to int32_t in both operator land and in our verification pipeline,
            // i.e., we compare the output of our queries to SELECT EXTRACT(EPOCH FROM date_)
            // fields of type date have no source table
            if (type_ == FieldType::DATE) {
                table_name = "";
                type_ = FieldType::LONG; // we actually store it as an INT32, this is the result of EXTRACT(EPOCH..)
            }
            string_length_ = stringLength;
        }

        void setStringLength(size_t i);

        size_t getStringLength() const { return string_length_; }

        void setOrdinal(const size_t &  ordinal) { ordinal_ = ordinal; }


        QueryFieldDesc &operator=(const QueryFieldDesc &other);

        bool operator==(const QueryFieldDesc &other);

        inline bool operator!=(const QueryFieldDesc &other) { return !(*this == other); }



    };

         std::ostream& operator<<(std::ostream &strm, const QueryFieldDesc &desc);

}

#endif // _QUERY_FIELD_DESC_H
