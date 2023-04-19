#pragma once

#include <algorithm>
#include "emp-zk/emp-zk.h"

/*
* the sets L and R are disjoint
* sets elements are 128-bit blocks
* their sizes are lcount, rcount
* assume elements in each set are unique
*/
template<typename IO>
bool ZKSet<IO>::disjointInternal(emp::Bit *lhs, emp::Bit *rhs,
        const int lcount, const int rcount,
        const int block_sz) {
    Integer dummy_high, dummy_low;
    if(block_sz <= 64) {
        uint64_t dummy_int = 1ULL<<(block_sz-8);
        dummy_low = Integer(65, dummy_int, ALICE);
    } else {
        uint64_t dummy_int = 1ULL<<(block_sz-64-8);
        dummy_high = Integer(65, dummy_int, ALICE);
        dummy_low = Integer(65, 0LL, ALICE);
    }
    return disjointInternalImpl(lhs, rhs, lcount, rcount, block_sz,
            dummy_high, dummy_low);
}

struct val_with_sign {
    __uint128_t val;
    bool sig;
};

template<typename IO>
bool ZKSet<IO>::disjointInternalImpl(emp::Bit *lhs, emp::Bit *rhs,
        const int lcount, const int rcount,
        const int block_sz, Integer &dummy_high, Integer &dummy_low) {
    vector<val_with_sign> lval;
    vector<val_with_sign> rval;

    int ucount = lcount + rcount;
    vector<emp::Bit> uhs(ucount*(block_sz+1));
    vector<emp::Bit> ahs(ucount*(block_sz+1));
    if(party == ALICE) {
      vector<emp::block> lval_block(lcount);
      vector<emp::block> rval_block(rcount);
        packRevealBitBlock(lval_block, lhs, block_sz, lcount);
        packRevealBitBlock(rval_block, rhs, block_sz, rcount);

        lval.resize(lcount);
        rval.resize(rcount);
        for(int i = 0; i < lcount; ++i) {
          lval[i].val = (__uint128_t)lval_block[i];
          lval[i].sig = false;
        }
        lval_block.clear();
        for(int i = 0; i < rcount; ++i) {
          rval[i].val = (__uint128_t)rval_block[i];
          rval[i].sig = true;
        }
        rval_block.clear();

        vector<val_with_sign> uval(ucount);
        memcpy(uval.data(), lval.data(), lcount*sizeof(val_with_sign)); // size 32 bytes for alignments?
        memcpy(uval.data()+lcount, rval.data(), rcount*sizeof(val_with_sign));

        struct {
          bool operator()(val_with_sign a, val_with_sign b) const {
            if(a.val < b.val) return true;
            else if(a.val == b.val) {
              if((a.sig == false) && (b.sig == true)) return true;
              else return false;
            }
            else return false;
          }
        } CustomLess;
        std::sort(uval.begin(), uval.end(), CustomLess);
        
        for(int i = 0; i < ucount; ++i) {
            input_bits_block_sz(uhs.data()+i*(block_sz+1),
                    (block)uval[i].val, block_sz);
            uhs[i*(block_sz+1)+block_sz] = Bit(uval[i].sig, ALICE);
        }
    } else {
        emp::block dummy = emp::zero_block;
        for(int i = 0; i < ucount; ++i) {
            input_bits_block_sz(uhs.data()+i*(block_sz+1), dummy, block_sz);
            uhs[i*(block_sz+1)+block_sz] = Bit(false, ALICE);
        }
    }

    for(int i = 0; i < lcount; ++i) {
      memcpy(ahs.data()+i*(block_sz+1), lhs+i*block_sz, block_sz*sizeof(Bit));
      ahs[i*(block_sz+1)+block_sz] = Bit(false, PUBLIC);
    }
    for(int i = 0; i < rcount; ++i) {
      memcpy(ahs.data()+(lcount+i)*(block_sz+1), rhs+i*block_sz, block_sz*sizeof(Bit));
      ahs[(lcount+i)*(block_sz+1)+block_sz] = Bit(true, PUBLIC);
    }
    bool is_disjoint = equal(ahs.data(), uhs.data(), ucount, block_sz+1);

    Integer ah(65, 0LL, ALICE);
    Integer al(65, 0LL, ALICE);
    Integer bh(65, 0LL, ALICE);
    Integer bl(65, 0LL, ALICE);
    Bit res(true, ALICE);
    if(block_sz <= 64) {
        Bit is_equal(true, ALICE);
        for(int i = 0; i < ucount - 1; ++i) {
            // uhs[i] < uhs[i+1]
            memcpy(al.bits.data(),
                    uhs.data()+i*(block_sz+1),
                    block_sz*sizeof(block));
            memcpy(bl.bits.data(),
                    uhs.data()+(i+1)*(block_sz+1),
                    block_sz*sizeof(block));
            Bit a_sig = uhs[i*(block_sz+1)+block_sz];
            Bit b_sig = uhs[(i+1)*(block_sz+1)+block_sz];
            is_equal = bl.equal(al);
            res = res & (
                bl.equal(dummy_low)
                | (bl.geq(al) & (!is_equal))
                | (is_equal & (a_sig == b_sig)));
        }
    } else {
        Bit is_equal_high(true, ALICE);
        Bit is_equal_low(true, ALICE);
        for(int i = 0; i < ucount - 1; ++i) {
            // uhs[i] < uhs[i+1]
            memcpy(ah.bits.data(),
                    uhs.data()+i*(block_sz+1)+64,
                    (block_sz-64)*sizeof(block));
            memcpy(al.bits.data(),
                    uhs.data()+i*(block_sz+1),
                    64*sizeof(block));
            memcpy(bh.bits.data(),
                    uhs.data()+(i+1)*(block_sz+1)+64,
                    (block_sz-64)*sizeof(block));
            memcpy(bl.bits.data(),
                    uhs.data()+(i+1)*(block_sz+1),
                    64*sizeof(block));
            Bit a_sig = uhs[i*(block_sz+1)+block_sz];
            Bit b_sig = uhs[(i+1)*(block_sz+1)+block_sz];
            is_equal_high = bh.equal(ah);
            is_equal_low = bl.equal(al);
            res = res &
                (
                    (
                        bh.equal(dummy_high) & (bl.equal(dummy_low))
                    ) | (
                        (bh.geq(ah) & (!is_equal_high)) |
                        (is_equal_high & (bl.geq(al) & (!is_equal_low)))
                    ) | (
                        is_equal_high & is_equal_low & (a_sig == b_sig)
                    )
                );
        }
    }
    is_disjoint &= res.reveal<bool>(PUBLIC);
    return is_disjoint;
}

template<typename IO>
bool ZKSet<IO>::disjointExtInternal(emp::Bit *lhs, emp::Bit *rhs,
        const int lcount, const int rcount,
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

    compressBitsToBlock(lval, lmac, lhs, coefficient,
        lcount, block_sz, block128_n, block128_n_rem);
    compressBitsToBlock(rval, rmac, rhs, coefficient,
        rcount, block_sz, block128_n, block128_n_rem);

    vector<emp::block> dummy_comp_val(1);
    vector<emp::block> dummy_comp_mac(1);
    emp::Bit *dummy_raw = new emp::Bit[block_sz];
    int i = 0;
    for(; i < block_sz-8; ++i)
        dummy_raw[i] = emp::Bit(false, ALICE);
    dummy_raw[i] = emp::Bit(true, ALICE);
    ++i;
    for(; i < block_sz; ++i)
        dummy_raw[i] = emp::Bit(false, ALICE);
    compressBitsToBlock(dummy_comp_val, dummy_comp_mac,
            dummy_raw, coefficient, 1, block_sz,
            block128_n, block128_n_rem);
    delete[] dummy_raw;
    Integer dummy_high(65, 0LL, ALICE);
    Integer dummy_low(65, 0LL, ALICE);

    emp::Bit *llhs = new emp::Bit[lcount*128];
    emp::Bit *rrhs = new emp::Bit[rcount*128];
    emp::Bit *ddhs = new emp::Bit[128];

    if(party == ALICE) {
        for(int i = 0; i < lcount; ++i)
            input_bits_block_sz(llhs+i*128, lval[i], 128);
        for(int i = 0; i < rcount; ++i)
            input_bits_block_sz(rrhs+i*128, rval[i], 128);
        input_bits_block_sz(ddhs, dummy_comp_val[0], 128);
    } else {
        emp::block dummy = emp::zero_block;
        for(int i = 0; i < lcount; ++i)
            input_bits_block_sz(llhs+i*128, dummy, 128);
        for(int i = 0; i < rcount; ++i)
            input_bits_block_sz(rrhs+i*128, dummy, 128);
        input_bits_block_sz(ddhs, dummy, 128);
    }

    memcpy(dummy_high.bits.data(), ddhs+64, 64*sizeof(emp::Bit));
    memcpy(dummy_low.bits.data(), ddhs, 64*sizeof(emp::Bit));

    bool is_disjoint = true;

    is_disjoint &= checkCorrectCompression(lval, lmac, llhs, lcount);
    is_disjoint &= checkCorrectCompression(rval, rmac, rrhs, rcount);
    is_disjoint &= disjointInternalImpl(llhs, rrhs, lcount, rcount, 128,
            dummy_high, dummy_low);

    delete[] llhs;
    delete[] rrhs;

    return is_disjoint;
}
