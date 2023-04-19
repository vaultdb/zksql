#ifndef QUERY_TABLE_H
#define QUERY_TABLE_H


#include "query_schema.h"
#include "query_tuple.h"
#include <memory>
#include <ostream>
#include "util/utilities.h"
#include "plain_tuple.h"
#include <emp-tool/emp-tool.h>
#include <boost/iterator/iterator_facade.hpp>



namespace  vaultdb {

    typedef std::pair<std::vector<int8_t>, std::vector<int8_t> > SecretShares;

    template<typename B> class QueryTable;

    typedef  QueryTable<bool> PlainTable;
    typedef  QueryTable<emp::Bit> SecureTable;



    template <typename B>
    class QueryTable {
        private:

        // tuple order
            SortDefinition order_by_;
            std::shared_ptr<QuerySchema> schema_;

    public:
        std::vector<int8_t> tuple_data_;
        // size of each tuple in bytes
        size_t tuple_size_;

        // empty sort definition for default case
            QueryTable(const size_t &num_tuples, const QuerySchema &schema, const SortDefinition & sortDefinition = SortDefinition());
            QueryTable(const QueryTable &src);

            ~QueryTable() {
                tuple_data_.clear(); // manually reset for easier tracing
            }

            void resize(const size_t & tupleCount);
            bool isEncrypted() const;

            void setSchema(const QuerySchema &schema);

            const std::shared_ptr<QuerySchema> getSchema() const;

            QueryTuple<B> getTuple(int idx);
            const QueryTuple<B> getImmutableTuple(int idx) const;

            unsigned int getTupleCount() const;

            std::string toString(const bool &showDummies = false) const;
            std::string toString(const size_t & limit, const bool &showDummies = false) const;

            void putTuple(const int &idx, const QueryTuple<B> &tuple);


            void setSortOrder(const SortDefinition &sortOrder);

            SortDefinition getSortOrder() const;



            std::vector<int8_t> serialize() const;


            static std::shared_ptr<SecureTable> secretShare(const PlainTable & input, emp::NetIO *io, const int &party);
            static std::shared_ptr<SecureTable>
            secret_share_send_table(const std::shared_ptr<PlainTable> &input, const int &sharing_party);
            static std::shared_ptr<SecureTable>
            secret_share_recv_table(const size_t &tuple_cnt, const QuerySchema &src_schema,
                                    const SortDefinition &sortDefinition, const int &sharing_party);


            SecretShares generateSecretShares() const; // generate shares for alice and bob - for data sharing (non-computing) node

            [[nodiscard]] std::unique_ptr<QueryTable<bool> > reveal(int empParty = emp::PUBLIC) const;

            QueryTable<B> &operator=(const QueryTable<B> &src);

            bool operator==(const QueryTable<B> &other) const;

            bool operator!=(const QueryTable &other) const { return !(*this == other); }
            QueryTuple<B> operator[](const int & idx);

            const QueryTuple<B> operator[](const int & idx) const;

            static std::shared_ptr<PlainTable> deserialize(const QuerySchema & schema, const vector<int8_t> &tableBits);

            // encrypted version of deserialization using emp::Bit
            static std::shared_ptr<SecureTable> deserialize(const QuerySchema &schema, vector<Bit> &table_bits);

            size_t getTrueTupleCount() const;

        PlainTuple getPlainTuple(size_t idx) const;

    private:
        static std::unique_ptr<PlainTable> revealTable(const SecureTable & table, const int & party);
        static std::unique_ptr<PlainTable> revealTable(const PlainTable & table, const int & party);
        static void secret_share_send(const int &party, const PlainTable & src_table, SecureTable &dst_table, const int &write_offset,
                               const bool &reverse_read_order);
        static void secret_share_recv(const size_t &tuple_count, const int &dst_party,
                               SecureTable &dst_table, const size_t &write_offset,
                               const bool &reverse_read_order);

    };

    std::ostream &operator<<(std::ostream &os, const PlainTable &table);

    std::ostream &operator<<(std::ostream &os, const SecureTable &table);



}
#endif // _QUERY_TABLE_H
