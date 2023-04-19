#ifndef VAULTDB_EMP_ZK_GLOBAL_VARS_H
#define VAULTDB_EMP_ZK_GLOBAL_VARS_H

#include <emp-tool/circuits/bit.h>
#include <emp-zk-set/zk_set.h>

// based on https://stackoverflow.com/questions/1008019/c-singleton-design-pattern
namespace vaultdb {
    class ZKGlobalVars
    {
    public:


        static ZKGlobalVars& getInstance() {
            static ZKGlobalVars  instance;
            return instance;
        }

        void initializeZKSet(int party);


        ZKSet<BoolIO<NetIO>>* getZKSet();


        ZKGlobalVars(const ZKGlobalVars&) = delete;
        ZKGlobalVars& operator=(const ZKGlobalVars &) = delete;

        void reset();
        int party = -1;

        void setCommCost(uint64_t c) {comm_cost = c;}
        uint64_t getCommCost() {return comm_cost;}

        void setZKSetCommCost(uint64_t c) {zkset_comm_cost = c;}
        uint64_t getZKSetCommCost() {return zkset_comm_cost;}

        void setThreads(int t) {threads = t;}
        int getThreads() {return threads;}

        void setNetio(BoolIO<NetIO> ** io) {netio = io;}
        BoolIO<NetIO> *getNetio() {return *netio;}

        uint64_t getAccumulativeCommCostFromAllThreads();
    private:
        ZKGlobalVars() {}
        // creating this as a smart pointer to invoke destructor during tear down
        std::unique_ptr<ZKSet<BoolIO<NetIO>> > zkset_;

        uint64_t comm_cost = 0; // communication cost from current party to another party
        uint64_t zkset_comm_cost = 0;
        int threads = -1;
        BoolIO<NetIO> **netio;


    };
}

#endif //VAULTDB_EMP_ZK_GLOBAL_VARS_H
