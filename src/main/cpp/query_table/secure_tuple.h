#ifndef _SECURE_TUPLE_H
#define _SECURE_TUPLE_H

#include "query_tuple.h"
#include "plain_tuple.h"

namespace vaultdb {

    typedef QueryTuple<emp::Bit> SecureTuple;

    // secure specialization
    template<>
    class QueryTuple<emp::Bit> {
    protected:

        emp::Bit *fields_; // has dummy tag at end, serialized representation, points to an offset in parent QueryTable
        std::shared_ptr<QuerySchema> query_schema_; // pointer to enclosing table
        std::unique_ptr<emp::Bit []> managed_data_;

    public:
        QueryTuple() {};
        ~QueryTuple() { managed_data_.reset();  }
        QueryTuple(const QueryTuple & src);

        QueryTuple(std::shared_ptr<QuerySchema> & query_schema, int8_t *src);
        QueryTuple(std::shared_ptr<QuerySchema> & query_schema, emp::Bit *src);
        // constructor for immutable tuple
        QueryTuple(const std::shared_ptr<QuerySchema> &query_schema, const int8_t *src);
        explicit QueryTuple(const QuerySchema & schema); // self-managed memory

        emp::Bit *getData() const  { return fields_; }

        inline bool  isEncrypted() const { return true; }

        bool hasManagedStorage() const { return managed_data_.get() != nullptr; }

        SecureField getField(const int &ordinal);
        const SecureField getField(const int & ordinal) const;


        void setField(const int &idx, const SecureField &f);
        void setDummyTag(const emp::Bit & d);

        void setDummyTag(const bool & b);

        void setDummyTag(const Field<emp::Bit> & d);

        void setSchema(std::shared_ptr<QuerySchema> q);


        emp::Bit getDummyTag() const;

        std::shared_ptr<QuerySchema> getSchema() const;

        QueryTuple<bool> reveal(const int &empParty,  std::shared_ptr<QuerySchema> & dst_schema, int8_t *dst) const;
        // self-managed storage
        QueryTuple<bool> reveal(const int &empParty = emp::PUBLIC) const;

        string toString(const bool &showDummies = false) const;

        void serialize(int8_t *dst);

        size_t getFieldCount() const;


        QueryTuple& operator=(const SecureTuple& other);

        emp::Bit operator==(const SecureTuple & other) const;
        emp::Bit operator!=(const SecureTuple & other) const;

        SecureField operator[](const int32_t & idx);
        const SecureField operator[](const int32_t & idx) const;

        static void compareSwap(const emp::Bit & cmp, QueryTuple<emp::Bit> & lhs, QueryTuple<emp::Bit> & rhs);


        static SecureTuple
        deserialize(emp::Bit *dst_tuple_bits, std::shared_ptr<QuerySchema> &schema, const emp::Bit *src_tuple_bits);

        static void writeSubset(const SecureTuple & src_tuple, const SecureTuple & dst_tuple, uint32_t src_start_idx, uint32_t src_attr_cnt, uint32_t dst_start_idx);


        void clear(const Bit &zeroOutData);





    };


    std::ostream &operator<<(std::ostream &os, const SecureTuple &tuple);




}

#endif // _SECURE_TUPLE_H
