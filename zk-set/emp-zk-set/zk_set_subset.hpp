#pragma once

#include "emp-zk/emp-zk.h"

template<typename IO>
bool ZKSet<IO>::subsetInternal(emp::Bit *lhs, emp::Bit *rhs,
        const int lcount, const int rcount,
        const int block_sz) {

    int mcount = rcount - lcount;
    vector<emp::block> rval;
    vector<emp::block> mval;

    emp::Bit *mhs = new emp::Bit[rcount*block_sz];

    if(party == ALICE) {
        vector<emp::block> lval(lcount);
        rval.resize(rcount);
        mval.resize(rcount);
        packRevealBitBlock(lval, lhs, block_sz, lcount);
        packRevealBitBlock(rval, rhs, block_sz, rcount);

        vector<int> rhs_not_lhs_index;
        std::set<__uint128_t> lval_set;
        for(int i = 0; i < lcount; ++i)
            lval_set.insert((__uint128_t)lval[i]);
        set_minus(rhs_not_lhs_index, rval, lval_set,
                rcount, lcount);

        for(std::size_t i = 0; i < rhs_not_lhs_index.size(); ++i) {
            mval[i] = rval[rhs_not_lhs_index[i]];
            input_bits_block_sz(mhs+i*block_sz, mval[i],
                    block_sz);
        }
	emp::block dummy = rval[rval.size()-1];
        for(int i = rhs_not_lhs_index.size(); i < mcount; ++i) {
            mval[i] = dummy;
            input_bits_block_sz(mhs+i*block_sz, zero_block,
                block_sz);
        }
        memcpy(mval.data()+mcount, lval.data(),
                lcount*sizeof(block));

    } else {
        emp::block dummy = emp::zero_block;
        for(int i = 0; i < mcount; ++i) {
            input_bits_block_sz(mhs+i*block_sz, dummy,
                    block_sz);
        }
    }
    memcpy(mhs+mcount*block_sz, lhs,
            lcount*block_sz*sizeof(emp::Bit));
    bool is_subset = equal(rhs, mhs, rval, mval, rcount, block_sz);

    delete[] mhs;
    return is_subset;
}

template<typename IO>
bool ZKSet<IO>::subsetExtInternal(emp::Bit *lhs, emp::Bit *rhs,
        const int lcount, const int rcount,
        const int block_sz) {
    const int block128_n = (block_sz+8*sizeof(block)-1)/(8*sizeof(block));
    const int block128_n_rem = block_sz % (8*sizeof(block));

    vector<emp::block> lval(lcount);
    vector<emp::block> lmac(lcount);
    vector<emp::block> rval(rcount);
    vector<emp::block> rmac(rcount);
    vector<emp::block> mval(rcount);
    vector<emp::block> mmac(rcount);

    block coefficient[block128_n];
    io->flush();
    block seed = io->get_hash_block();
    uni_hash_coeff_gen(coefficient, seed, block128_n);

    compressBitsToBlock(lval, lmac, lhs, coefficient,
        lcount, block_sz, block128_n, block128_n_rem);
    compressBitsToBlock(rval, rmac, rhs, coefficient,
        rcount, block_sz, block128_n, block128_n_rem);

    emp::Bit *llhs = new emp::Bit[lcount*128];
    emp::Bit *rrhs = new emp::Bit[rcount*128];

    if(party == ALICE) {
        for(int i = 0; i < lcount; ++i)
            input_bits_block_sz(llhs+i*128, lval[i], 128);
        for(int i = 0; i < rcount; ++i)
            input_bits_block_sz(rrhs+i*128, rval[i], 128);
    } else {
        emp::block dummy = emp::zero_block;
        for(int i = 0; i < lcount; ++i)
            input_bits_block_sz(llhs+i*128, dummy, 128);
        for(int i = 0; i < rcount; ++i)
            input_bits_block_sz(rrhs+i*128, dummy, 128);
    }

    bool is_subset = true;
   
    is_subset &= checkCorrectCompression(lval, lmac, llhs, lcount);
    is_subset &= checkCorrectCompression(rval, rmac, rrhs, rcount);

    is_subset &= subsetInternal(llhs, rrhs, lcount, rcount, 128);
   
    delete[] llhs;
    delete[] rrhs;

    return is_subset;
}
