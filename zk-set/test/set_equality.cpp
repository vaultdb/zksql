#include "emp-zk-set/zk_set.h"

int party, port;
const int threads = 4;

using namespace std;
using namespace emp;

void test_set_equality_small(ZKSet<BoolIO<NetIO>> *zkset, const int block_sz, int test_n) {
	std::vector<emp::Bit> lhs(test_n*block_sz);
	std::vector<emp::Bit> rhs(test_n*block_sz);
	bool *test_bits = new bool[test_n*block_sz];
	PRG prg;
	prg.random_bool(test_bits, test_n*block_sz);

	for(int i = 0; i < test_n*block_sz; ++i) {
		lhs[i] = emp::Bit(test_bits[i], ALICE);
	}
	for(int i = 0; i < test_n*block_sz; ++i) {
		int index = (i+block_sz)>=(test_n*block_sz)?(i+block_sz-test_n*block_sz):i+block_sz;
		rhs[i] = emp::Bit(test_bits[index], ALICE);
	}

	bool isequal = true;
	for(int i = 0; i < 1000; ++i) {
		isequal &= zkset->equal(lhs.data(), rhs.data(), test_n, block_sz);
	}
	std::cout << isequal << std::endl;

	delete[] test_bits;

}

void test_set_equality(ZKSet<BoolIO<NetIO>> *zkset, const int block_sz) {
	const int test_n = 10000;
	std::vector<emp::Bit> lhs(test_n*block_sz);
	std::vector<emp::Bit> rhs(test_n*block_sz);
	bool *test_bits = new bool[test_n*block_sz];
	PRG prg;
	prg.random_bool(test_bits, test_n*block_sz);

	for(int i = 0; i < test_n*block_sz; ++i) {
		lhs[i] = emp::Bit(test_bits[i], ALICE);
	}
	for(int i = 0; i < test_n*block_sz; ++i) {
		int index = (i+block_sz)>=(test_n*block_sz)?(i+block_sz-test_n*block_sz):i+block_sz;
		rhs[i] = emp::Bit(test_bits[index], ALICE);
	}

	bool isequal = zkset->equal(lhs.data(), rhs.data(), test_n, block_sz);
	std::cout << isequal << std::endl;

	delete[] test_bits;

}

/*void test_set_equality_ext(ZKSet<BoolIO<NetIO>> *zkset, const int block_sz) {
	const int test_n = 10000;
	std::vector<emp::Bit> lhs(test_n*block_sz);
	std::vector<emp::Bit> rhs(test_n*block_sz);
	bool *test_bits = new bool[test_n*block_sz];
	PRG prg;
	prg.random_bool(test_bits, test_n*block_sz);

	for(int i = 0; i < test_n*block_sz; ++i) {
		lhs[i] = emp::Bit(test_bits[i], ALICE);
	}
	for(int i = 0; i < test_n*block_sz; ++i) {
		int index = (i+block_sz)>=(test_n*block_sz)?(i+block_sz-test_n*block_sz):i+block_sz;
		rhs[i] = emp::Bit(test_bits[index], ALICE);
	}

	bool isequal = zkset->equalExt(lhs.data(), rhs.data(), test_n, block_sz);
	std::cout << isequal << std::endl;

	delete[] test_bits;

}*/

int main(int argc, char **argv) {
	parse_party_and_port(argv, &party, &port);
	BoolIO<NetIO>* ios[threads];
	for(int i = 0; i < threads; ++i)
		ios[i] = new BoolIO<NetIO>(
				new NetIO(party==ALICE?nullptr:"127.0.0.1",port)
				, party==ALICE);

	setup_zk_bool<BoolIO<NetIO>>(ios, threads, party);
	ZKSet<BoolIO<NetIO>> *zkset = new ZKSet<BoolIO<NetIO>>(party);

    for(int i = 0; i < 10; ++i) {
        test_set_equality_small(zkset, 16, 10);
        test_set_equality_small(zkset, 16, 5);
        test_set_equality_small(zkset, 16, 3);
        test_set_equality_small(zkset, 16, 1);
        test_set_equality(zkset, 35);
        test_set_equality(zkset, 97);
        test_set_equality(zkset, 732);
        test_set_equality(zkset, 4098);
    }

	bool cheated = finalize_zk_bool<BoolIO<NetIO>>();
    if(cheated) error("zk proof: prover cheating");
	for(int i = 0; i < threads; ++i) {
		delete ios[i]->io;
		delete ios[i];
	}
	return 0;
}
