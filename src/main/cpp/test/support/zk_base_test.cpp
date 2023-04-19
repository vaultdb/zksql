#include "zk_base_test.h"
#include <util/data_utilities.h>
#include <operators/sql_input.h>
#include <util/zk_global_vars.h>

using namespace vaultdb;

DECLARE_int32(party);
DECLARE_int32(port);
DECLARE_string(alice_host);



void ZkTest::SetUp() {

    for(int i = 0; i < threads_; ++i)
        ios_[i] = new BoolIO<NetIO>(new NetIO(FLAGS_party == ALICE ? nullptr : FLAGS_alice_host.c_str(),FLAGS_port+i), FLAGS_party==ALICE);

    setup_zk_bool<BoolIO<NetIO>>(ios_, threads_, FLAGS_party);

    gv.initializeZKSet(FLAGS_party);
    gv.party = FLAGS_party;
    gv.setThreads(threads_);
    gv.setNetio(ios_);

    Utilities::mkdir("data");

}

void ZkTest::TearDown() {

    ZKGlobalVars &  gv = ZKGlobalVars::getInstance();
    gv.reset();

    bool cheat = finalize_zk_bool<BoolIO<NetIO>>();
    ASSERT_FALSE(cheat);

    for(int i = 0; i < threads_; ++i) {
        delete ios_[i]->io;
        delete ios_[i];
    }



}

void ZkTest::validate(shared_ptr<PlainTable> expected, const ZkQueryTable & observed) {
    shared_ptr<PlainTable> result = observed.secure_table_->reveal(PUBLIC);

    if(FLAGS_party == emp::ALICE) {
        ASSERT_EQ(*expected, *result);  // are Alice's plain and encrypted versions of the table in sync?
        ASSERT_EQ(*expected, *(observed.plain_table_));

    }

    ASSERT_EQ(*expected,  *result);



}

