#ifndef _ZK_QUERY_TABLE_H
#define _ZK_QUERY_TABLE_H

#include <query_table/query_table.h>
#include <emp-zk/emp-zk.h>
#include <util/zk_utils.h>


using namespace std;

// This wrapper encapsulates a plaintext query table and a secret-shared version
// Alice has both the secret-shared and plain table
// Bob only sees the secret-shared version
//  a thin wrapper around QueryTable API

namespace vaultdb {
    class ZkQueryTable {

    public:
        shared_ptr<PlainTable> plain_table_;
        shared_ptr<SecureTable> secure_table_;
        int party_ = emp::PUBLIC;
        BoolIO<NetIO> *netio_;

        ZkQueryTable() = default;
        ZkQueryTable(const ZkQueryTable & src);
        ZkQueryTable(shared_ptr<PlainTable> plain, shared_ptr<SecureTable> secure, const int &party,
                     BoolIO<NetIO> *netio);
        ZkQueryTable(const size_t &num_tuples, const QuerySchema &schema, const int &party, BoolIO<NetIO> *netio,
                     const SortDefinition &sort_definition = SortDefinition());


        // setup for Bob/verifier, just receiving secret-shared secure_table_
        explicit ZkQueryTable(const QuerySchema &schema, const SortDefinition &order_by, BoolIO<NetIO> *netio,
                              const int &party);

        // primarily for Alice, secret shares plaintable
        ZkQueryTable(shared_ptr<PlainTable>  plain, BoolIO<NetIO> *netio,  const int & party);

        ZkQueryTable(shared_ptr<PlainTable> input, BoolIO<NetIO> *netio, int sentBitsSize, const int &party);
        ZkQueryTable(const QuerySchema &schema, const SortDefinition &order_by, BoolIO<NetIO> *netio, int receivedBitsSize,
                     const int &party);


        const shared_ptr<QuerySchema> getSchema() const;

        SortDefinition  getSortOrder() const;
        void setSortOrder(const SortDefinition & s);

        bool initialized() const;

        int getTupleCount() const;

        void resize(const size_t & limit);

        bool encrypted() const;

        void validate_tables() const;

        string toString(bool showDummies = false) const;

        // free up memory for tuples.
        // Leave schema and sort order unchanged - they are features of the query plan / op order too
        void reset();
    };

}

#endif // _ZK_QUERY_TABLE_H
