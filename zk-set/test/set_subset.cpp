#include "emp-zk-set/zk_set.h"

int party, port;
const int threads = 4;

using namespace std;
using namespace emp;

void test_set_subset(ZKSet<BoolIO<NetIO>> *zkset, const int block_sz) {
	const int lcount = 20000;
	const int rcount = 30000;
	std::vector<emp::Bit> lhs(lcount*block_sz);
	std::vector<emp::Bit> rhs(rcount*block_sz);
	bool *test_bits = new bool[rcount*block_sz];
	PRG prg;
	prg.random_bool(test_bits, rcount*block_sz);

	for(int i = 0; i < lcount*block_sz; ++i) {
		lhs[i] = emp::Bit(test_bits[i], ALICE);
	}
	for(int i = 0; i < rcount*block_sz; ++i) {
		rhs[i] = emp::Bit(test_bits[i], ALICE);
	}

	bool is_subset = zkset->subset(lhs.data(), rhs.data(),
			lcount, rcount, block_sz);
	std::cout << is_subset << std::endl;

	delete[] test_bits;

}

void test_set_subset_dummy(ZKSet<BoolIO<NetIO>> *zkset, const int block_sz) {
	const int dummy_count = 10000;
	const int lcount = 20000;
	const int rcount = 30000;
	std::vector<emp::Bit> lhs(lcount*block_sz);
	std::vector<emp::Bit> rhs(rcount*block_sz);
	bool *test_bits = new bool[rcount*block_sz];
	PRG prg;
	prg.random_bool(test_bits, rcount*block_sz);

	for(int i = 0; i < dummy_count*block_sz; ++i) {
		lhs[i+lcount-dummy_count] = emp::Bit(false, ALICE);
		rhs[i+rcount-dummy_count] = emp::Bit(false, ALICE);
	}
	for(int i = 0; i < 500*block_sz; ++i) {
		lhs[i] = emp::Bit(false, ALICE);
	}
	for(int i = 500*block_sz; i < (lcount-dummy_count+500)*block_sz; ++i) {
		lhs[i] = emp::Bit(test_bits[i], ALICE);
	}
	for(int i = 0; i < (rcount-dummy_count)*block_sz; ++i) {
		rhs[i] = emp::Bit(test_bits[i], ALICE);
	}

	bool is_subset = zkset->subset(lhs.data(), rhs.data(),
			lcount, rcount, block_sz);
	std::cout << is_subset << std::endl;

	delete[] test_bits;

}

int main(int argc, char **argv) {
	parse_party_and_port(argv, &party, &port);
	BoolIO<NetIO>* ios[threads];
	for(int i = 0; i < threads; ++i)
		ios[i] = new BoolIO<NetIO>(
				new NetIO(party==ALICE?nullptr:"127.0.0.1",port)
				, party==ALICE);

	setup_zk_bool<BoolIO<NetIO>>(ios, threads, party);
	ZKSet<BoolIO<NetIO>> *zkset = new ZKSet<BoolIO<NetIO>>(party);
	//ZKSet<BoolIO<NetIO>> *zkset2 = new ZKSet<BoolIO<NetIO>>(party);

	const int block_sz = 73;
	const int block_sz_ext = 567;
	std::cout << "set subset" << std::endl;
    for(int i = 0; i < 10; ++i) {
        test_set_subset(zkset, block_sz);
        test_set_subset(zkset, block_sz_ext);
        test_set_subset_dummy(zkset, block_sz);
        test_set_subset_dummy(zkset, block_sz_ext);
    }

	bool cheated = finalize_zk_bool<BoolIO<NetIO>>();
	if(cheated) error("cheated");
	for(int i = 0; i < threads; ++i) {
		delete ios[i]->io;
		delete ios[i];
	}
	return 0;
}
