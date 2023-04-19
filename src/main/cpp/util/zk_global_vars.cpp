#include "zk_global_vars.h"

using namespace vaultdb;



void ZKGlobalVars::initializeZKSet(int party) {
    assert(zkset_ == nullptr);
    zkset_ = std::unique_ptr<ZKSet<BoolIO<NetIO>>>(new ZKSet<BoolIO<NetIO>>(party));
}

ZKSet<BoolIO<NetIO>>* ZKGlobalVars::getZKSet() {
    return zkset_.get();
}


void ZKGlobalVars::reset() {
    zkset_.reset();
}

uint64_t ZKGlobalVars::getAccumulativeCommCostFromAllThreads() {
    uint64_t commCostFromAllThreads = 0;

    for(int i = 0; i < threads; ++i) {
        uint64_t tempCost = netio[i]->counter;
        commCostFromAllThreads += tempCost;
    }

    return commCostFromAllThreads;
}
