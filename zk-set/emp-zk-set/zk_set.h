#pragma once

#include <set>
#include "emp-zk/emp-zk.h"

template<typename IO>
class ZKSet {
public:

	int party;
	IO *io;
	block delta;
	F2kOSTriple<IO> *ostriple = nullptr;

	ZKSet(int _party) : party(_party) {
		assert(CircuitExecution::circ_exec != nullptr);
		ZKBoolCircExec<IO> *exec =
			(ZKBoolCircExec<IO>*)(CircuitExecution::circ_exec);
		io = exec->ostriple->io;
		delta = exec->ostriple->delta;
		ostriple = new F2kOSTriple<IO>(party, exec->ostriple->threads,
				exec->ostriple->ios, exec->ostriple->ferret,
				exec->ostriple->pool);
	}

	~ZKSet() {
		if(ostriple != nullptr)
			delete ostriple;
	}

	/*
	 * equality of two sets
	 * call equalInternal or equalExtInternal depends on element size
	 */
	bool equal(emp::Bit *lhs, emp::Bit *rhs,
			const int count, const int block_sz) {
		if(block_sz <= 128) {
			return equalInternal(lhs, rhs, count, block_sz);
		} else {
			return equalExtInternal(lhs, rhs, count, block_sz);
		}
	}

	/*
	* equality of two sets
	* sets elements are 128-bit blocks
	*/
	bool equalInternal(emp::Bit *lhs, emp::Bit *rhs,
        	const int count, const int block_sz);
	/*
	 * equality of two sets
	 * sets elements are 128-bit blocks
	 * prover inputs packed value set
	 */
	bool equal(emp::Bit *lhs, emp::Bit *rhs,
			vector<emp::block> &lval, vector<emp::block> &rval,
			const int count, const int block_sz);

	/*
	 * equality of two sets
	 * sets elements are of arbitrary length
	 */
	bool equalExtInternal(emp::Bit *lhs, emp::Bit *rhs,
			const int count, const int block_sz);

	/*
	 * the set L is the subset of R
	 * sets elements are 128-bit blocks
	 * their sizes are lcount, rcount
	 */

	bool subset(emp::Bit *lhs, emp::Bit *rhs,
			const int lcount, const int rcount,
			const int block_sz) {
		if(block_sz <= 128) {
			return subsetInternal(lhs, rhs, lcount, rcount, block_sz);
		} else {
			return subsetExtInternal(lhs, rhs, lcount, rcount, block_sz);
		}
	}

	bool subsetInternal(emp::Bit *lhs, emp::Bit *rhs,
			const int lcount, const int rcount,
			const int block_sz);

	bool subsetExtInternal(emp::Bit *lhs, emp::Bit *rhs,
			const int lcount, const int rcount,
			const int block_sz);

	/*
	 * the set M is the union of L and R
	 * sets elements are 128-bit blocks
	 * their sizes are lcount, rcount, mcount
	 */
	bool setUnion(emp::Bit *lhs, emp::Bit *rhs, emp::Bit *mhs,
			const int lcount, const int rcount,
			const int mcount,
			const int block_sz) {
		if(block_sz <= 128) {
			return setUnionInternal(lhs, rhs, mhs,
				lcount, rcount, mcount, block_sz);
		} else {
			return setUnionExtInternal(lhs, rhs, mhs,
				lcount, rcount, mcount, block_sz);
		}
	}

	bool setUnionInternal(emp::Bit *lhs, emp::Bit *rhs, emp::Bit *mhs,
			const int lcount, const int rcount,
			const int mcount,
			const int block_sz);

	bool setUnionExtInternal(emp::Bit *lhs, emp::Bit *rhs, emp::Bit *mhs,
			const int lcount, const int rcount,
			const int mcount,
			const int block_sz);

	/*
	 * the sets L and R are disjoint
	 * sets elements are 128-bit blocks
	 * their sizes are lcount, rcount
	 * assume elements in each set are unique
	 */
	bool disjoint(emp::Bit *lhs, emp::Bit *rhs,
			const int lcount, const int rcount,
			const int block_sz) {
		if(block_sz <= 128) {
			return disjointInternal(lhs, rhs, lcount, rcount, block_sz);
		} else {
			return disjointExtInternal(lhs, rhs, lcount, rcount, block_sz);
		}
	}

	bool disjointInternal(emp::Bit *lhs, emp::Bit *rhs,
			const int lcount, const int rcount,
			const int block_sz);

	bool disjointInternalImpl(emp::Bit *lhs, emp::Bit *rhs,
			const int lcount, const int rcount,
			const int block_sz, Integer &dummy_high, Integer &dummy_low);

	bool disjointExtInternal(emp::Bit *lhs, emp::Bit *rhs,
			const int lcount, const int rcount,
			const int block_sz);

	/*
	 * the set M is the intersection of L and R
	 * sets elements are 128-bit blocks
	 * their sizes are lcount, rcount and mcount
	 */
	bool intersection(emp::Bit *lhs, emp::Bit *rhs, emp::Bit *mhs,
			const int lcount, const int rcount,
			const int mcount,
			const int block_sz) {
		if(block_sz <= 128) {
			return intersectionInternal(lhs, rhs, mhs,
				lcount, rcount, mcount, block_sz);
		} else {
			return intersectionExtInternal(lhs, rhs, mhs,
				lcount, rcount, mcount, block_sz);
		}
	}

	bool intersectionInternal(emp::Bit *lhs, emp::Bit *rhs, emp::Bit *mhs,
			const int lcount, const int rcount,
			const int mcount,
			const int block_sz);

	bool intersectionExtInternal(emp::Bit *lhs, emp::Bit *rhs, emp::Bit *mhs,
        const int lcount, const int rcount,
        const int mcount,
        const int block_sz);

private:
	static const int block_bit_sz_max = 128;
	GaloisFieldPacking pack;

	emp::block packBitBlock(const bool *bits);

	emp::block packBitBlock(const bool *bits, const int block_sz);

	void packRevealBitBlock(vector<emp::block> &out,
			const emp::Bit *emp_bits,
			const int block_sz, const int count);

	emp::block packRevealBitBlock(const emp::Bit *emp_bits, const int block_sz);

	void packRevealBitBlockExt(emp::block *val_in_blocks, emp::Bit *emp_bits,
			const int block128_n, const int block128_n_rem);

	emp::block packBlockBlock(const emp::Bit *emp_bits);

	emp::block packBlockBlock(const emp::Bit *emp_bits, const int block_sz);

	void packBlockBlockExt(emp::block *val_in_blocks, const emp::Bit *emp_bits,
			const int block128_n, const int block128_n_rem);

	void compressBitsToBlock(vector<emp::block> &val, vector<emp::block> &mac,
			emp::Bit *hs,
			block *coefficient,
			const int count, const int block_sz,
			const int block128_n, const int block128_n_rem);

	void set_minus(vector<int> &out_index, vector<emp::block> &val,
			std::set<__uint128_t> &filter,
			const int vcount, const int fcount);

	void set_minus_origin(vector<int> &out_index, vector<emp::block> &val,
			std::set<__uint128_t> &filter,
			const int vcount, const int fcount);

	void intersection_local(vector<emp::block> &mhs,
			const vector<emp::block> &lhs,
			const vector<emp::block> &rhs,
			const int lcount, const int rcount);

    bool checkCorrectCompression(vector<emp::block> &val,
        vector<emp::block> &mac, emp::Bit *auth_bits, int count);

	void input_bits_128(emp::Bit *bits, emp::block input);

	void input_bits_block_sz(emp::Bit *bits, emp::block input, const int block_sz);

	void input_bits_from_bit_block_sz(emp::Bit *bits, emp::Bit *input,
			const int block_sz, const int block128_n, const int block128_n_rem);
	
	void inn_prdt_bch4(block &val, block &mac,
			const vector<emp::block> &X, const vector<emp::block> &MAC,
			const block r, const int count);

	bool check_set_equality(const vector<emp::block> &lval, const vector<emp::block> &lmac,
			const vector<emp::block> &rval, const vector<emp::block> &rmac, const int count);
};

#include "emp-zk-set/zk_set_equal.hpp"
#include "emp-zk-set/zk_set_subset.hpp"
#include "emp-zk-set/zk_set_union.hpp"
#include "emp-zk-set/zk_set_disjoint.hpp"
#include "emp-zk-set/zk_set_intersection.hpp"
#include "emp-zk-set/zk_set_utils.hpp"
