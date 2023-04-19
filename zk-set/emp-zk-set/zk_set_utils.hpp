#pragma once

#include "emp-zk/emp-zk.h"

template<typename IO>
emp::block ZKSet<IO>::packBitBlock(const bool *bits) {
    uint64_t low = bool_to_int<uint64_t>(bits);
    uint64_t high = bool_to_int<uint64_t>(bits+64);
    return emp::makeBlock(high, low);
}

template<typename IO>
emp::block ZKSet<IO>::packBitBlock(const bool *bits, const int block_sz) {
    if(block_sz <= 64) {
        uint64_t low = bool_to_int<uint64_t>(bits);	
        return emp::makeBlock(0LL, low);
    } else {
        uint64_t low = bool_to_int<uint64_t>(bits);
        uint64_t high = bool_to_int<uint64_t>(bits+64);
        return emp::makeBlock(high, low);
    }
}

template<typename IO>
void ZKSet<IO>::packRevealBitBlock(vector<emp::block> &out,
        const emp::Bit *emp_bits,
        const int block_sz, const int count) {
    bool val_bits[block_bit_sz_max];
    for(int i = 0, j = 0; i < count; ++i, j += block_sz) {
        memset(val_bits, 0, block_bit_sz_max*sizeof(bool));
        for(int k = 0; k < block_sz; ++k)
            val_bits[k] = emp_bits[j+k].reveal(ALICE);
        out[i] = packBitBlock(val_bits, block_sz);
    }
}

template<typename IO>
emp::block ZKSet<IO>::packRevealBitBlock(const emp::Bit *emp_bits, const int block_sz) {
    bool val_bits[block_bit_sz_max];
    memset(val_bits, 0, block_bit_sz_max*sizeof(bool));
    for(int k = 0; k < block_sz; ++k)
        val_bits[k] = emp_bits[k].reveal(ALICE);
    return packBitBlock(val_bits, block_sz);
}

template<typename IO>
void ZKSet<IO>::packRevealBitBlockExt(emp::block *val_in_blocks, emp::Bit *emp_bits,
        const int block128_n, const int block128_n_rem) {
    for(int i = 0; i < block128_n - 1; ++i) {
        val_in_blocks[i] = packRevealBitBlock(
                emp_bits+i*8*sizeof(block),
                8*sizeof(block));
    }
    val_in_blocks[block128_n-1] = packRevealBitBlock(
            emp_bits+(block128_n-1)*8*sizeof(block),
            block128_n_rem);
}

template<typename IO>
emp::block ZKSet<IO>::packBlockBlock(const emp::Bit *emp_bits) {
    block ret;
    pack.packing(&ret, (block*)emp_bits);
    return ret;
}

template<typename IO>
emp::block ZKSet<IO>::packBlockBlock(const emp::Bit *emp_bits, const int block_sz) {
    block ret;
    emp::vector_inn_prdt_sum_red(&ret, (block*)emp_bits, pack.base, block_sz);
    return ret;
}

template<typename IO>
void ZKSet<IO>::packBlockBlockExt(emp::block *val_in_blocks, const emp::Bit *emp_bits,
        const int block128_n, const int block128_n_rem) {
    for(int i = 0; i < block128_n-1; ++i)
        pack.packing(val_in_blocks+i, (block*)emp_bits+i*8*sizeof(block));
    emp::vector_inn_prdt_sum_red(val_in_blocks+block128_n-1,
            (block*)emp_bits+(block128_n-1)*8*sizeof(block),
            pack.base, block128_n_rem);
}

template<typename IO>
void ZKSet<IO>::set_minus_origin(vector<int> &out_index, vector<emp::block> &val,
        std::set<__uint128_t> &filter,
        const int vcount, const int fcount) {
    int j = 0;
    for(int i = 0; i < vcount; ++i) {
        __uint128_t v = (__uint128_t)val[i];
        if(filter.find(v) == filter.end()) {
            out_index[j++] = i;
        }
    }
    if(j != (vcount - fcount)) error("set minus origin fails");
}

template<typename IO>
void ZKSet<IO>::set_minus(vector<int> &out_index, vector<emp::block> &val,
        std::set<__uint128_t> &filter,
        const int vcount, const int fcount) {
    //int j = 0;
    ///__uint128_t zero = (__uint128_t)0;
    __uint128_t zero = (__uint128_t)val[val.size()-1];
    for(int i = 0; i < vcount; ++i) {
        __uint128_t v = (__uint128_t)val[i];
        if(v != zero) {
            if(filter.find(v) == filter.end()) {
                out_index.push_back(i);
            }
        }
    }
    //assert(j == (vcount - fcount));
}

template<typename IO>
void ZKSet<IO>::intersection_local(vector<emp::block> &mhs,
        const vector<emp::block> &lhs,
        const vector<emp::block> &rhs,
        const int lcount, const int rcount) {

    vector<__uint128_t> llhs(lcount);
    memcpy(llhs.data(), lhs.data(), lcount*sizeof(block));
    vector<__uint128_t> rrhs(rcount);
    memcpy(rrhs.data(), rhs.data(), rcount*sizeof(block));
    sort(llhs.begin(), llhs.end());
    sort(rrhs.begin(), rrhs.end());
    int i = 0, j = 0, k = 0;
    while(i < lcount && j < rcount) {
        if(llhs[i] < rrhs[j]) {
            i++;
        } else if(llhs[i] == rrhs[j]) {
            mhs[k++] = (emp::block)llhs[i];
            i++;
            j++;
        } else {
            j++;
        }
    }
}

template<typename IO>
void ZKSet<IO>::input_bits_128(emp::Bit *bits, emp::block input) {
    bool val_bits[128];
    uint64_t half = _mm_extract_epi64(input, 0);
    emp::int_to_bool<uint64_t>(val_bits, half, 64);
    half = _mm_extract_epi64(input, 1);
    emp::int_to_bool<uint64_t>(val_bits+64, half, 64);
    for(int i = 0; i < 128; ++i)
        bits[i] = Bit(val_bits[i], ALICE);
}

template<typename IO>
void ZKSet<IO>::input_bits_block_sz(emp::Bit *bits, emp::block input, const int block_sz) {
    bool val_bits[128];
    uint64_t half = _mm_extract_epi64(input, 0);
    emp::int_to_bool<uint64_t>(val_bits, half, 64);
    half = _mm_extract_epi64(input, 1);
    emp::int_to_bool<uint64_t>(val_bits+64, half, 64);
    for(int i = 0; i < block_sz; ++i)
        bits[i] = Bit(val_bits[i], ALICE);
}

template<typename IO>
void ZKSet<IO>::input_bits_from_bit_block_sz(emp::Bit *bits, emp::Bit *input,
        const int block_sz, const int block128_n, const int block128_n_rem) {
    block val_in_blocks[block128_n];
    packRevealBitBlockExt(val_in_blocks, input, block128_n, block128_n_rem);
    for(int i = 0; i < block128_n - 1; ++i)
        input_bits_block_sz(bits+i*128, val_in_blocks[i], 128);
    input_bits_block_sz(bits+(block128_n-1)*128,
        val_in_blocks[block128_n-1], block128_n_rem);
}

template<typename IO>
void ZKSet<IO>::inn_prdt_bch4(block &val, block &mac,
        const vector<emp::block> &X, const vector<emp::block> &MAC,
        const block r, const int count) {
    block x[4], m[4];
    int i = 1;
    if(party == ALICE) {
        ostriple->compute_add_const(val, mac, X[0], MAC[0], r);
        while(i < count-3 && count > 4) {
            for(int j = 0; j < 4; ++j)
                ostriple->compute_add_const(x[j], m[j], X[i+j], MAC[i+j], r);
            ostriple->compute_mul5(val, mac, val, mac, x[0], m[0],
                    x[1], m[1], x[2], m[2], x[3], m[3]);
            i += 4;
        }
        while(i < count) {
            ostriple->compute_add_const(x[0], m[0], X[i], MAC[i], r);
            ostriple->compute_mul(val, mac, val, mac, x[0], m[0]);
            ++i;
        }
    } else {
        block dummy = zero_block;
        ostriple->compute_add_const(val, mac, dummy, MAC[0], r);
        while(i < count-3 && count > 4) {
            for(int j = 0; j < 4; ++j)
                ostriple->compute_add_const(x[j], m[j], dummy, MAC[i+j], r);
            ostriple->compute_mul5(val, mac, val, mac, x[0], m[0],
                    x[1], m[1], x[2], m[2], x[3], m[3]);
            i += 4;
        }
        while(i < count) {
            ostriple->compute_add_const(x[0], m[0], dummy, MAC[i], r);
            ostriple->compute_mul(val, mac, val, mac, x[0], m[0]);
            ++i;
        }
    }
}

template<typename IO>
bool ZKSet<IO>::check_set_equality(const vector<emp::block> &lval, const vector<emp::block> &lmac,
        const vector<emp::block> &rval, const vector<emp::block> &rmac, const int count) {
    emp::block r, val[2], mac[2];
    io->flush();
    r = io->get_hash_block();
    inn_prdt_bch4(val[0], mac[0], lval, lmac, r, count);
    inn_prdt_bch4(val[1], mac[1], rval, rmac, r, count);

    if(party == ALICE) {
        io->send_data(mac, 2*sizeof(block));
        io->flush();
        return true;
    } else {
        block macrecv[2];
        io->recv_data(macrecv, 2*sizeof(block));
        mac[0] ^= macrecv[0];
        mac[1] ^= macrecv[1];
        if(memcmp(mac, mac+1, 16) != 0) {
            //error("check set equality failed!\n");
            return false;
        }
    }
    return true;
}

template<typename IO>
void ZKSet<IO>::compressBitsToBlock(vector<emp::block> &val, vector<emp::block> &mac,
        emp::Bit *hs,
        block *coefficient,
        const int count, const int block_sz,
        const int block128_n, const int block128_n_rem) {
    if(party == ALICE) {
        block val_in_blocks[block128_n];
        block mac_in_blocks[block128_n];
        for(int i = 0,j = 0; i < count; ++i, j += block_sz) {
            packRevealBitBlockExt(val_in_blocks, hs+j, block128_n, block128_n_rem);
            packBlockBlockExt(mac_in_blocks, hs+j, block128_n, block128_n_rem);
            emp::vector_inn_prdt_sum_red(val.data()+i, val_in_blocks, coefficient, block128_n);
            emp::vector_inn_prdt_sum_red(mac.data()+i, mac_in_blocks, coefficient, block128_n);
        }
    } else {
        block mac_in_blocks[block128_n];
        for(int i = 0,j = 0; i < count; ++i, j += block_sz) {
            packBlockBlockExt(mac_in_blocks, hs+j, block128_n, block128_n_rem);
            emp::vector_inn_prdt_sum_red(mac.data()+i, mac_in_blocks, coefficient, block128_n);
        }
    }
}

template<typename IO>
bool ZKSet<IO>::checkCorrectCompression(vector<emp::block> &val,
        vector<emp::block> &mac, emp::Bit *auth_bits, int count) {
    vector<emp::block> rval(count);
    vector<emp::block> rmac(count);
    if(party == ALICE) {
        for(int i = 0, j = 0; i < count; ++i, j+=128) {
            rval[i] = packRevealBitBlock(auth_bits+j, 128);
            rmac[i] = packBlockBlock(auth_bits+j);
        }
    } else {
        for(int i = 0, j = 0; i < count; ++i, j+=128)
            rmac[i] = packBlockBlock(auth_bits+j);
    }
    return check_set_equality(val, mac, rval, rmac, count);
}
