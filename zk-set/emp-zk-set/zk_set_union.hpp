#pragma once

#include "emp-zk/emp-zk.h"

/*
* the set M is the union of L and R
* sets elements are 128-bit blocks
* their sizes are lcount, rcount, mcount
*/
template<typename IO>
bool ZKSet<IO>::setUnionInternal(emp::Bit *lhs, emp::Bit *rhs, emp::Bit *mhs,
        const int lcount, const int rcount,
        const int mcount,
        const int block_sz) {

    // set intersection
    int scount = lcount + rcount - mcount;
    vector<emp::block> S_block(scount);
    emp::Bit *shs = new emp::Bit[scount*block_sz];

    bool is_set_union = true;

    if(party == ALICE) {
        // 1. locally compute set-intersection S
        vector<emp::block> lval;
        vector<emp::block> rval;
        vector<emp::block> mval;
        lval.resize(lcount);
        rval.resize(rcount);
        mval.resize(mcount);
        packRevealBitBlock(lval, lhs, block_sz, lcount);
        packRevealBitBlock(rval, rhs, block_sz, rcount);
        packRevealBitBlock(mval, mhs, block_sz, mcount);

        intersection_local(S_block, lval, rval, lcount, rcount);

        assert(S_block.size() == (std::size_t)scount);

        for(int i = 0; i < scount; ++i) {
            input_bits_block_sz(shs+i*block_sz, S_block[i],
                    block_sz);
        }
    } else {
        emp::block dummy = emp::zero_block;
        for(int i = 0; i < scount; ++i) {
            input_bits_block_sz(shs+i*block_sz, dummy,
                    block_sz);
        }
    }

    // 2. prove S is the intersection of L and R
    is_set_union &= intersection(lhs, rhs, shs,
            lcount, rcount, scount, block_sz);

    // 3. prove L + R == M + S
    vector<emp::Bit> lset((lcount + rcount) * block_sz);
    vector<emp::Bit> rset((mcount + scount) * block_sz);
    memcpy(lset.data(), lhs, lcount*block_sz*sizeof(emp::Bit));
    memcpy(lset.data()+lcount*block_sz, rhs,
            rcount*block_sz*sizeof(emp::Bit));
    memcpy(rset.data(), mhs, mcount*block_sz*sizeof(emp::Bit));
    memcpy(rset.data()+mcount*block_sz, shs,
            scount*block_sz*sizeof(emp::Bit));
    is_set_union &= equal(lset.data(), rset.data(), lcount+rcount, block_sz);

    delete[] shs;

    return is_set_union;
}

template<typename IO>
bool ZKSet<IO>::setUnionExtInternal(emp::Bit *lhs, emp::Bit *rhs, emp::Bit *mhs,
        const int lcount, const int rcount,
        const int mcount,
        const int block_sz) {
    const int block128_n = (block_sz+8*sizeof(block)-1)/(8*sizeof(block));
    const int block128_n_rem = block_sz % (8*sizeof(block));
   
    //int scount = lcount + rcount - mcount;
    vector<emp::block> lval(lcount);
    vector<emp::block> lmac(lcount);
    vector<emp::block> rval(rcount);
    vector<emp::block> rmac(rcount);
    vector<emp::block> mval(mcount);
    vector<emp::block> mmac(mcount);

    block coefficient[block128_n];
    io->flush();
    block seed = io->get_hash_block();
    uni_hash_coeff_gen(coefficient, seed, block128_n);

    compressBitsToBlock(lval, lmac, lhs, coefficient,
        lcount, block_sz, block128_n, block128_n_rem);
    compressBitsToBlock(rval, rmac, rhs, coefficient,
        rcount, block_sz, block128_n, block128_n_rem);
    compressBitsToBlock(mval, mmac, mhs, coefficient,
        mcount, block_sz, block128_n, block128_n_rem);


    // set intersection
    //vector<emp::block> S_block(scount);
    emp::Bit *llhs = new emp::Bit[lcount*128];
    emp::Bit *rrhs = new emp::Bit[rcount*128];
    //emp::Bit *shs = new emp::Bit[scount*128];
    emp::Bit *mmhs = new emp::Bit[mcount*128];
 
    bool is_set_union = true;

    if(party == ALICE) {
        // 1. locally compute set-intersection S
        /*intersection_local(S_block, lval, rval, lcount, rcount);

        assert(S_block.size() == scount);

        vector<__uint128_t> a(lcount+rcount);
        vector<__uint128_t> b(lcount+rcount);
        memcpy(a.data(), lval.data(), lcount*sizeof(block));
        memcpy(a.data()+lcount, rval.data(), rcount*sizeof(block));
        memcpy(b.data(), mval.data(), mcount*sizeof(block));
        memcpy(b.data()+mcount, S_block.data(), scount*sizeof(block));
        sort(a.begin(), a.end());
        sort(b.begin(), b.end());
        for(int i = 0; i < lcount+rcount; ++i) {
            if(a[i] != b[i])
                error("set union error");
        }*/

        for(int i = 0; i < lcount; ++i)
            input_bits_block_sz(llhs+i*128, lval[i], 128);
        for(int i = 0; i < rcount; ++i)
            input_bits_block_sz(rrhs+i*128, rval[i], 128);
        for(int i = 0; i < mcount; ++i)
            input_bits_block_sz(mmhs+i*128, mval[i], 128);
        //for(int i = 0; i < scount; ++i)
            //input_bits_block_sz(shs+i*128, S_block[i], 128);
    } else {
        emp::block dummy = emp::zero_block;

        for(int i = 0; i < lcount; ++i)
            input_bits_block_sz(llhs+i*128, dummy, 128);
        for(int i = 0; i < rcount; ++i)
            input_bits_block_sz(rrhs+i*128, dummy, 128);
        for(int i = 0; i < mcount; ++i)
            input_bits_block_sz(mmhs+i*128, dummy, 128);
        //for(int i = 0; i < scount; ++i)
            //input_bits_block_sz(shs+i*128, dummy, 128);
    }

    is_set_union &= checkCorrectCompression(lval, lmac, llhs, lcount);
    is_set_union &= checkCorrectCompression(rval, rmac, rrhs, rcount);
    is_set_union &= checkCorrectCompression(mval, mmac, mmhs, mcount);

    is_set_union &= setUnionInternal(llhs, rrhs, mmhs, lcount, rcount, mcount, 128);

    //delete[] shs;
    delete[] llhs;
    delete[] rrhs;
    delete[] mmhs;

    return is_set_union;
}
