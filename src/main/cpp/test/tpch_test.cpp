#include <gflags/gflags.h>
#include <gtest/gtest.h>
#include <operators/sql_input.h>
#include <parser/plan_parser.h>
#include "support/tpch_queries.h"
#include <test/support/zk_base_test.h>
#include <ctime>


using namespace emp;
using namespace vaultdb;

DEFINE_int32(party, 1, "party for EMP execution");
DEFINE_int32(port, 54321, "port for EMP execution");
DEFINE_string(alice_host, "127.0.0.1", "alice hostname for execution");

// to reproduce experiments run these tests one program execution at a time
// otherwise the memory utilization measurements will be the "high watermark"
// of all tests run so far
class TpcHTest : public ZkTest {


protected:
    // limit input to first N tuples per SQL statement
    int limit_ = -1;

    void runTest(const int &test_id, const string & test_name, const SortDefinition &expected_sort);
};

void TpcHTest::runTest(const int &test_id, const string & test_name, const SortDefinition &expected_sort) {
    string query = tpch_queries[test_id];
    ZKGlobalVars & globalVars = ZKGlobalVars::getInstance();
    cout << "Initial memory util: " << Utilities::checkMemoryUtilization() << " bytes." << endl;

    auto plainStartTime = clock_start();
    shared_ptr<PlainTable> expected = DataUtilities::getExpectedResults(db_name_, query, false, 0);
    double plainDuration = time_from(plainStartTime) / 1e6;
    cout << "Plaintext runtime: " << plainDuration << "s\n";
    expected->setSortOrder(expected_sort);

    PlanParser plan_reader(db_name_, test_name, ios_[0], FLAGS_party, limit_);
    shared_ptr<Operator> root = plan_reader.getRoot();



    auto secureStartTime = clock_start();
    clock_t secureStartClock = clock();
    ZkQueryTable result = root->run();
    double secureClockTicks = (double) (clock() - secureStartClock);
    double secureClockTicksPerSecond = secureClockTicks / ((double) CLOCKS_PER_SEC);
    double secureDuration = time_from(secureStartTime) / 1e6;

    cout << "Total communication cost: " << gv.getCommCost() << " bytes\n";
    cout << "ZK runtime: " << secureDuration << "s\n";
    cout << "CPU clock ticks: " << secureClockTicks << "\n";
    cout << "CPU clock ticks per second: " << secureClockTicksPerSecond << "\n";

    size_t peak_memory =  Utilities::checkMemoryUtilization(true);
    cout << "Memory: " << peak_memory  << " bytes\n";

    shared_ptr<PlainTable> observed = result.secure_table_->reveal(PUBLIC);

    observed = DataUtilities::removeDummies(observed);
    expected = DataUtilities::removeDummies(expected);

    if(FLAGS_party == ALICE) {
        ASSERT_EQ(*expected, *observed);
    }
}

TEST_F(TpcHTest, tpch_q1) {
    SortDefinition expected_sort = DataUtilities::getDefaultSortDefinition(2);
    runTest(1, "q1", expected_sort);
}

TEST_F(TpcHTest, tpch_q3) {
    SortDefinition expected_sort{ColumnSort(-1, SortDirection::ASCENDING),
                                 ColumnSort(1, SortDirection::DESCENDING),
                                 ColumnSort(2, SortDirection::ASCENDING)};

    runTest(3, "q3", expected_sort);
}

TEST_F(TpcHTest, tpch_q5) {
    SortDefinition  expected_sort{ColumnSort(1, SortDirection::DESCENDING)};

    runTest(5, "q5", expected_sort);
}

TEST_F(TpcHTest, tpch_q8) {
    SortDefinition expected_sort = DataUtilities::getDefaultSortDefinition(1);

    runTest(8,"q8", expected_sort);
}

TEST_F(TpcHTest, tpch_q9) {
    SortDefinition  expected_sort{ColumnSort(0, SortDirection::ASCENDING), ColumnSort(1, SortDirection::DESCENDING)};

    runTest(9,"q9", expected_sort);

}

TEST_F(TpcHTest, tpch_q18) {
    SortDefinition expected_sort{ColumnSort(-1, SortDirection::ASCENDING),
                                 ColumnSort(4, SortDirection::DESCENDING),
                                 ColumnSort(3, SortDirection::ASCENDING)};

    runTest(18, "q18", expected_sort);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    return RUN_ALL_TESTS();
}

