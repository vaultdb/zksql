#pragma once

#include "emp-zk/emp-zk.h"

/*
* equality of two sets
* sets elements are 128-bit blocks
*/
template<typename IO>
bool ZKSet<IO>::equalInternal(emp::Bit *lhs, emp::Bit *rhs,
        const int count, const int block_sz) {
    vector<emp::block> lval(count); 
    vector<emp::block> lmac(count); 
    vector<emp::block> rval(count); 
    vector<emp::block> rmac(count);
    // 1. for each block, pack 128 bits cleartext into a block
    // 2. for each block, pack 128 authenticated MAC/keys into a block
    if(party == ALICE) {
        for(int i = 0,j = 0; i < count; ++i, j += block_sz) {
            lval[i] = packRevealBitBlock(lhs+j, block_sz);
            lmac[i] = packBlockBlock(lhs+j, block_sz);
        }
        for(int i = 0,j = 0; i < count; ++i, j += block_sz) {
            rval[i] = packRevealBitBlock(rhs+j, block_sz);
            rmac[i] = packBlockBlock(rhs+j, block_sz);
        }
    } else {
        for(int i = 0,j = 0; i < count; ++i, j += block_sz) {
            lmac[i] = packBlockBlock(lhs+j, block_sz);
        }
        for(int i = 0,j = 0; i < count; ++i, j += block_sz) {
            rmac[i] = packBlockBlock(rhs+j, block_sz);
        }
    }
    // 3. check set equality
    bool isequal = check_set_equality(lval, lmac, rval, rmac, count);
    return isequal;
}

/*
* equality of two sets
* sets elements are 128-bit blocks
* prover inputs packed value set
*/
template<typename IO>
bool ZKSet<IO>::equal(emp::Bit *lhs, emp::Bit *rhs,
        vector<emp::block> &lval, vector<emp::block> &rval,
        const int count, const int block_sz) {
    vector<emp::block> lmac(count); 
    vector<emp::block> rmac(count); 
    // 1. for each block, pack 128 authenticated MAC/keys into a block
    for(int i = 0,j = 0; i < count; ++i, j += block_sz) {
        lmac[i] = packBlockBlock(lhs+j, block_sz);
    }
    for(int i = 0,j = 0; i < count; ++i, j += block_sz) {
        rmac[i] = packBlockBlock(rhs+j, block_sz);
    }
    // 2. check set equality
    bool isequal = check_set_equality(lval, lmac, rval, rmac, count);
    return isequal;
}

/*
* equality of two sets
* sets elements are of arbitrary length
*/
template<typename IO>
bool ZKSet<IO>::equalExtInternal(emp::Bit *lhs, emp::Bit *rhs,
        const int count, const int block_sz) {
    const int block128_n = (block_sz+8*sizeof(block)-1)/(8*sizeof(block));
    const int block128_n_rem = block_sz % (8*sizeof(block));

    vector<emp::block> lval(count); 
    vector<emp::block> lmac(count); 
    vector<emp::block> rval(count); 
    vector<emp::block> rmac(count);

    block coefficient[block128_n];
    io->flush();
    block seed = io->get_hash_block();
    uni_hash_coeff_gen(coefficient, seed, block128_n);

    // 1. for each block, pack 128 bits cleartext into a block
    // 2. for each block, pack 128 authenticated MAC/keys into a block
    compressBitsToBlock(lval, lmac, lhs, coefficient,
        count, block_sz, block128_n, block128_n_rem);
    compressBitsToBlock(rval, rmac, rhs, coefficient,
        count, block_sz, block128_n, block128_n_rem);

    // 3. check set equality
    bool isequal = check_set_equality(lval, lmac, rval, rmac, count);
    return isequal;
}