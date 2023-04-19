#ifndef _ZK_BASE_TEST_H
#define _ZK_BASE_TEST_H
#include <gflags/gflags.h>
#include <gtest/gtest.h>
#include <stdexcept>
#include <emp-zk/emp-zk.h>
#include <query_table/query_table.h>
#include <query_table/zk_query_table.h>


namespace vaultdb {

    class ZkTest : public ::testing::Test {
    protected:

        void SetUp() override;

        void TearDown() override;

        void validate(shared_ptr<PlainTable> expected, const ZkQueryTable & observed);

        // *** Change database for experiments here! *** //
        // Alice (prover): tpch_60k, tpch_120k, tpch_240k
        // Bob (verifier): always tpch_zk_bob
        std::string db_name_ = "tpch_60k";
        static const int threads_ = 1;
        BoolIO<NetIO> *ios_[threads_];

        ZKGlobalVars& gv = ZKGlobalVars::getInstance();

    };

}

#endif
