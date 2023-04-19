#include "emp-zk-set/zk_set.h"

int party, port;
const int threads = 4;

using namespace std;
using namespace emp;

void test_set_disjoint(ZKSet<BoolIO<NetIO>> *zkset, const int block_sz) {
	const int lcount = 20000;
	const int rcount = 30000;
	std::vector<emp::Bit> lhs(lcount*block_sz);
	std::vector<emp::Bit> rhs(rcount*block_sz);
	bool *test_bits = new bool[(lcount+rcount)*block_sz];
	PRG prg;
	prg.random_bool(test_bits, (lcount+rcount)*block_sz);

	for(int i = 0; i < lcount*block_sz; ++i) {
		lhs[i] = emp::Bit(test_bits[i], ALICE);
	}
	for(int i = 0; i < rcount*block_sz; ++i) {
		rhs[i] = emp::Bit(test_bits[i+lcount*block_sz], ALICE);
	}

	bool is_disjoint = zkset->disjoint(lhs.data(), rhs.data(),
			lcount, rcount, block_sz);
	std::cout << is_disjoint << std::endl;

	delete[] test_bits;

}

void test_set_disjoint_dummy(ZKSet<BoolIO<NetIO>> *zkset, const int block_sz) {
	const int dummy_count = 10000;
	const int lcount = 20000;
	const int rcount = 30000;
	std::vector<emp::Bit> lhs(lcount*block_sz);
	std::vector<emp::Bit> rhs(rcount*block_sz);
	bool *test_bits = new bool[(lcount+rcount)*block_sz/2];
	PRG prg;
	prg.random_bool(test_bits, (lcount+rcount)*block_sz/2);

    for(int i = lcount-dummy_count; i < lcount; ++i) {
        for(int j = 0; j < block_sz; ++j)
            lhs[i*block_sz+j] = emp::Bit(false, ALICE);
        lhs[(i+1)*block_sz-8] = emp::Bit(true, ALICE);
    }
    for(int i = rcount-dummy_count; i < rcount; ++i) {
        for(int j = 0; j < block_sz; ++j)
            rhs[i*block_sz+j] = emp::Bit(false, ALICE);
        rhs[(i+1)*block_sz-8] = emp::Bit(true, ALICE);
    }
	for(int i = 0; i < (lcount-dummy_count)*block_sz/2; ++i) {
		lhs[i] = emp::Bit(test_bits[i], ALICE);
	}
        memcpy(lhs.data()+(lcount-dummy_count)*block_sz/2, lhs.data(), (lcount-dummy_count)*block_sz/2*sizeof(Bit));
	for(int i = 0; i < (rcount-dummy_count)*block_sz/2; ++i) {
		rhs[i] = emp::Bit(test_bits[i+lcount*block_sz/2], ALICE);
	}
        memcpy(rhs.data()+(rcount-dummy_count)*block_sz/2, rhs.data(), (rcount-dummy_count)*block_sz/2*sizeof(Bit));

	bool is_disjoint = zkset->disjoint(lhs.data(), rhs.data(),
			lcount, rcount, block_sz);
	std::cout << is_disjoint << std::endl;

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

	const int block_sz = 72;
	const int block_sz_ext = 552;
	std::cout << "set disjoint" << std::endl;
        test_set_disjoint(zkset, block_sz);
        test_set_disjoint(zkset, block_sz_ext);
        test_set_disjoint_dummy(zkset, block_sz);
        test_set_disjoint_dummy(zkset, block_sz_ext);

	bool cheated = finalize_zk_bool<BoolIO<NetIO>>();
	if(cheated) error("cheated");
	for(int i = 0; i < threads; ++i) {
		delete ios[i]->io;
		delete ios[i];
	}
	return 0;
}
