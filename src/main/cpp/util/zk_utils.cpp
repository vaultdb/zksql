#include "util/zk_utils.h"

using namespace vaultdb;

vector<emp::Bit> ZKUtils::tupleDataToBits(vector<int8_t> tupleData) {
    vector<emp::Bit> result;

    for(auto it = tupleData.begin(); it != tupleData.end(); ++it) {
        int8_t curInt = *it;
        for(int i = 0; i < 8; ++i) {
            int bit = curInt & 1;
            result.push_back(Bit((bool) bit, ALICE));
            curInt = curInt >> 1;
        }
    }

    return result;
}
