#include "emp-zk-set/zk_set.h"

int party, port;
const int threads = 4;

using namespace std;
using namespace emp;

void test_set_intersection(ZKSet<BoolIO<NetIO>> *zkset, const int block_sz) {
	const int lcount = 20000;
	const int rcount = 30000;
	const int mcount = 10000;
	std::vector<emp::Bit> lhs(lcount*block_sz);
	std::vector<emp::Bit> rhs(rcount*block_sz);
	std::vector<emp::Bit> mhs(mcount*block_sz);
	bool *test_bits = new bool[(lcount+rcount-mcount)*block_sz];
	PRG prg;
	prg.random_bool(test_bits, (lcount+rcount-mcount)*block_sz);

	for(int i = 0; i < lcount*block_sz; ++i) {
		lhs[i] = emp::Bit(test_bits[i], ALICE);
	}
	for(int i = 0; i < rcount*block_sz; ++i) {
		rhs[i] = emp::Bit(test_bits[i+(lcount-mcount)*block_sz], ALICE);
	}
	for(int i = 0; i < mcount*block_sz; ++i) {
		mhs[i] = emp::Bit(test_bits[i+(lcount-mcount)*block_sz], ALICE);
	}

	bool is_intsect = zkset->intersection(lhs.data(), rhs.data(),
			mhs.data(), lcount, rcount, mcount, block_sz);
	std::cout << is_intsect << std::endl;

	delete[] test_bits;

}

void test_set_union(ZKSet<BoolIO<NetIO>> *zkset, const int block_sz) {
	const int lcount = 20000;
	const int rcount = 30000;
	const int mcount = 40000;
	std::vector<emp::Bit> lhs(lcount*block_sz);
	std::vector<emp::Bit> rhs(rcount*block_sz);
	std::vector<emp::Bit> mhs(mcount*block_sz);
	bool *test_bits = new bool[mcount*block_sz];
	PRG prg;
	prg.random_bool(test_bits, mcount*block_sz);

	for(int i = 0; i < lcount*block_sz; ++i) {
		lhs[i] = emp::Bit(test_bits[i], ALICE);
	}
	for(int i = 0; i < rcount*block_sz; ++i) {
		rhs[i] = emp::Bit(test_bits[i+(lcount+rcount-mcount)*block_sz],
			       ALICE);
	}
	for(int i = 0; i < mcount*block_sz; ++i) {
		mhs[i] = emp::Bit(test_bits[i], ALICE);
	}

	bool is_union = zkset->setUnion(lhs.data(), rhs.data(), mhs.data(),
			lcount, rcount, mcount, block_sz);
	std::cout << is_union << std::endl;

	delete[] test_bits;
}

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
	const int block_sz_128 = 128;
	const int block_sz_ext = 567;
	std::cout << "set intersection" << std::endl;
	for(int i = 0; i < 5; ++i) {
		test_set_intersection(zkset, block_sz);
		test_set_intersection(zkset, block_sz_128);
		test_set_intersection(zkset, block_sz_ext);
    }
	/*for(int i = 0; i < 5; ++i)
		test_set_intersection(zkset2, block_sz);*/
	std::cout << "set union" << std::endl;
	for(int i = 0; i < 5; ++i) {
		test_set_union(zkset, block_sz);
		test_set_union(zkset, block_sz_128);
		test_set_union(zkset, block_sz_ext);
    }
	/*for(int i = 0; i < 5; ++i)
		test_set_union(zkset2, block_sz);*/
	std::cout << "set disjoint" << std::endl;
	for(int i = 0; i < 5; ++i) {
		test_set_disjoint(zkset, block_sz);
		test_set_disjoint(zkset, block_sz_128);
		test_set_disjoint(zkset, block_sz_ext);
    }
	/*for(int i = 0; i < 5; ++i)
		test_set_disjoint(zkset2, block_sz);*/

	bool cheated = finalize_zk_bool<BoolIO<NetIO>>();
	if(cheated) error("cheated");
	for(int i = 0; i < threads; ++i) {
		delete ios[i]->io;
		delete ios[i];
	}
	return 0;
}
