#pragma once

#include "emp-zk/emp-zk.h"

/*
* the set M is the intersection of L and R
* sets elements are 128-bit blocks
* their sizes are lcount, rcount and mcount
*/
template<typename IO>
bool ZKSet<IO>::intersectionInternal(emp::Bit *lhs, emp::Bit *rhs, emp::Bit *mhs,
        const int lcount, const int rcount,
        const int mcount,
        const int block_sz) {

    // size of L\M and R\M
    int lhs_not_mhs_count = lcount - mcount;
    int rhs_not_mhs_count = rcount - mcount;
    // set L\M + M and R\M + M
    vector<emp::Bit> lhs_not_mhs(lcount*block_sz);
    vector<emp::Bit> rhs_not_mhs(rcount*block_sz);

    vector<emp::block> lval;
    vector<emp::block> rval;
    vector<emp::block> mval;
    vector<emp::block> lhs_not_mhs_val;
    vector<emp::block> rhs_not_mhs_val;

    bool is_set_intersection = true;

    if(party == ALICE) {
        // 1. Prover input L\M and R\M
        // pack values into 128-bit blocks
        lval.resize(lcount);
        rval.resize(rcount);
        mval.resize(mcount);
        lhs_not_mhs_val.resize(lcount);
        rhs_not_mhs_val.resize(rcount);
        vector<int> lhs_not_mhs_index(lhs_not_mhs_count);
        vector<int> rhs_not_mhs_index(rhs_not_mhs_count);
        packRevealBitBlock(lval, lhs, block_sz, lcount);
        packRevealBitBlock(rval, rhs, block_sz, rcount);
        packRevealBitBlock(mval, mhs, block_sz, mcount);

        // store the intersection into 
        std::set<__uint128_t> mval_set;
        for(int i = 0; i < mcount; ++i)
            mval_set.insert((__uint128_t)mval[i]);
        set_minus_origin(lhs_not_mhs_index, lval, mval_set,
                lcount, mcount);
        set_minus_origin(rhs_not_mhs_index, rval, mval_set,
                rcount, mcount);

        // input
        for(int i = 0; i < lhs_not_mhs_count; ++i) {
            lhs_not_mhs_val[i] = lval[lhs_not_mhs_index[i]];
            input_bits_block_sz(
                    lhs_not_mhs.data()+i*block_sz,
                    lhs_not_mhs_val[i], block_sz);
        }
        memcpy(lhs_not_mhs.data()+(lhs_not_mhs_count)*block_sz, mhs,
                mcount*block_sz*sizeof(emp::Bit));
        memcpy(lhs_not_mhs_val.data()+lhs_not_mhs_count, mval.data(),
                mcount*sizeof(emp::block));
        for(int i = 0; i < rhs_not_mhs_count; ++i) {
            rhs_not_mhs_val[i] = rval[rhs_not_mhs_index[i]];
            input_bits_block_sz(
                    rhs_not_mhs.data()+i*block_sz,
                    rhs_not_mhs_val[i], block_sz);
        }
        memcpy(rhs_not_mhs.data()+(rhs_not_mhs_count)*block_sz, mhs,
                mcount*block_sz*sizeof(emp::Bit));
        memcpy(rhs_not_mhs_val.data()+rhs_not_mhs_count, mval.data(),
                mcount*sizeof(emp::block));

        // 2. Prove lhs\mhs + mhs = lhs
        is_set_intersection = equal(lhs, lhs_not_mhs.data(), lval,
                lhs_not_mhs_val, lcount, block_sz);

        // 3. Prove rhs\mhs + mhs = rhs	
        is_set_intersection &= equal(rhs, rhs_not_mhs.data(), rval,
                rhs_not_mhs_val, rcount, block_sz);
    } else {
        // 1. Prover input L\M and R\M
        emp::block dummy = emp::zero_block;
        for(int i = 0; i < lhs_not_mhs_count; ++i) {
            input_bits_block_sz(
                    lhs_not_mhs.data()+i*block_sz,
                    dummy, block_sz);
        }
        memcpy(lhs_not_mhs.data()+(lhs_not_mhs_count)*block_sz, mhs,
                mcount*block_sz*sizeof(emp::Bit));
        for(int i = 0; i < rhs_not_mhs_count; ++i) {
            input_bits_block_sz(
                    rhs_not_mhs.data()+i*block_sz,
                    dummy, block_sz);
        }
        memcpy(rhs_not_mhs.data()+(rhs_not_mhs_count)*block_sz, mhs,
                mcount*block_sz*sizeof(emp::Bit));

        // 2. Prove lhs\mhs + mhs = lhs
        is_set_intersection = equal(lhs, lhs_not_mhs.data(), lval,
                lhs_not_mhs_val, lcount, block_sz);

        // 3. Prove rhs\mhs + mhs = rhs	
        is_set_intersection &= equal(rhs, rhs_not_mhs.data(), rval,
                rhs_not_mhs_val, rcount, block_sz);
    }
    return is_set_intersection;
}

template<typename IO>
bool ZKSet<IO>::intersectionExtInternal(emp::Bit *lhs, emp::Bit *rhs, emp::Bit *mhs,
        const int lcount, const int rcount,
        const int mcount,
        const int block_sz) {
    const int block128_n = (block_sz+8*sizeof(block)-1)/(8*sizeof(block));
    const int block128_n_rem = block_sz % (8*sizeof(block));
    block coefficient[block128_n];
    io->flush();
    block seed = io->get_hash_block();
    uni_hash_coeff_gen(coefficient, seed, block128_n);

    vector<emp::block> lval(lcount);
    vector<emp::block> lmac(lcount);
    vector<emp::block> rval(rcount);
    vector<emp::block> rmac(rcount);
    vector<emp::block> mval(mcount);
    vector<emp::block> mmac(mcount);

    compressBitsToBlock(lval, lmac, lhs, coefficient,
        lcount, block_sz, block128_n, block128_n_rem);
    compressBitsToBlock(rval, rmac, rhs, coefficient,
        rcount, block_sz, block128_n, block128_n_rem);
    compressBitsToBlock(mval, mmac, mhs, coefficient,
        mcount, block_sz, block128_n, block128_n_rem);

    emp::Bit *llhs = new emp::Bit[lcount*128];
    emp::Bit *rrhs = new emp::Bit[rcount*128];
    emp::Bit *mmhs = new emp::Bit[mcount*128];

    bool is_set_intersection = true;

    if(party == ALICE) {
        for(int i = 0; i < lcount; ++i)
            input_bits_block_sz(llhs+i*128, lval[i], 128);
        for(int i = 0; i < rcount; ++i)
            input_bits_block_sz(rrhs+i*128, rval[i], 128);
        for(int i = 0; i < mcount; ++i)
            input_bits_block_sz(mmhs+i*128, mval[i], 128);
    } else {
        emp::block dummy = emp::zero_block;
        for(int i = 0; i < lcount; ++i)
            input_bits_block_sz(llhs+i*128, dummy, 128);
        for(int i = 0; i < rcount; ++i)
            input_bits_block_sz(rrhs+i*128, dummy, 128);
        for(int i = 0; i < mcount; ++i)
            input_bits_block_sz(mmhs+i*128, dummy, 128);
    }

    is_set_intersection &= checkCorrectCompression(lval, lmac, llhs, lcount);
    is_set_intersection &= checkCorrectCompression(rval, rmac, rrhs, rcount);
    is_set_intersection &= checkCorrectCompression(mval, mmac, mmhs, mcount);

    is_set_intersection &= intersectionInternal(llhs, rrhs, mmhs,
        lcount, rcount, mcount, 128);

    delete[] llhs;
    delete[] rrhs;
    delete[] mmhs;

    return is_set_intersection;
}
