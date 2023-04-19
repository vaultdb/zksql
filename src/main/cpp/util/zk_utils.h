#ifndef _ZK_UTILS_H
#define _ZK_UTILS_H

#include <emp-tool/circuits/bit.h>
#include <emp-zk-set/zk_set.h>
#include <query_table/query_table.h>
#include <util/zk_global_vars.h>

// utilities for zk proof of vaultdb

namespace vaultdb {
    class ZKUtils {
    private:
        ZKSet<BoolIO<NetIO>> *zkset_;
        int party_ = PUBLIC;
    public:
        ZKUtils(int party) {
            zkset_ = new ZKSet<BoolIO<NetIO>>(party);
            party_ = party;
        }

        static vector<emp::Bit> tupleDataToBits(vector<int8_t> tupleData);

    };
}

#endif //_ZK_UTILS_H
