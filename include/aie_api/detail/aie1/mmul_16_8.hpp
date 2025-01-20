// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_MMUL_16_8__HPP__
#define __AIE_API_DETAIL_AIE1_MMUL_16_8__HPP__

#include "../vector.hpp"
#include "../ld_st.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_16_8;

template <typename TypeB, unsigned AccumBits>
struct mmul_16_8<4, 4, 4, int16, TypeB, AccumBits> : public C_block<int16, TypeB, AccumBits, 16, 1>
{
    using vector_A_type = vector<int16, 16>;
    using vector_B_type = vector<TypeB, 16>;

    using C_block<int16, TypeB, AccumBits, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac16(this->data,
                             a.template grow<32>(), 0, 0x02020000, 0x06060404, 2, 0x1010,
                             b.template grow<32>(), 0, 0x31203120, 0x31203120, 8, 0x3120);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul16(a.template grow<32>(), 0, 0x02020000, 0x06060404,  2, 0x1010,
                             b.template grow<32>(), 0, 0x31203120, 0x31203120,  8, 0x3120);
    }
};

template <typename TypeB, unsigned AccumBits>
struct mmul_16_8<4, 8, 4, int16, TypeB, AccumBits> : public C_block<int16, TypeB, AccumBits, 16, 1>
{
    using vector_A_type = vector<int16, 32>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block<int16, TypeB, AccumBits, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac16(this->data,
                             a, 0, 0x04040000, 0x0c0c0808, 2, 0x1010,
                             b, 0, 0x31203120, 0x31203120, 8, 0x3120);

        this->data = ::mac16(this->data,
                             a, 4, 0x04040000, 0x0c0c0808, 2, 0x1010,
                             b, 0, 0xb9a8b9a8, 0xb9a8b9a8, 8, 0x3120);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul16(a, 0, 0x04040000, 0x0c0c0808, 2, 0x1010,
                             b, 0, 0x31203120, 0x31203120, 8, 0x3120);

        this->data = ::mac16(this->data,
                             a, 4, 0x04040000, 0x0c0c0808, 2, 0x1010,
                             b, 0, 0xb9a8b9a8, 0xb9a8b9a8, 8, 0x3120);
    }
};

template <typename TypeB, unsigned AccumBits>
struct mmul_16_8<4, 4, 8, int16, TypeB, AccumBits> : public C_block<int16, TypeB, AccumBits, 32, 2>
{
    using vector_A_type = vector<int16, 16>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block<int16, TypeB, AccumBits, 32, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mac16(this->data[0],
                                a.template grow<32>(), 0, 0x00000000, 0x02020202,   2, 0x1010,
                                b,                     0, 0x73625140, 0x73625140,  16, 0x3120);

        this->data[1] = ::mac16(this->data[1],
                                a.template grow<32>(), 8, 0x00000000, 0x02020202,   2, 0x1010,
                                b,                     0, 0x73625140, 0x73625140,  16, 0x3120);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mul16(a.template grow<32>(), 0, 0x00000000, 0x02020202,   2, 0x1010,
                                b,                     0, 0x73625140, 0x73625140,  16, 0x3120);

        this->data[1] = ::mul16(a.template grow<32>(), 8, 0x00000000, 0x02020202,   2, 0x1010,
                                b,                     0, 0x73625140, 0x73625140,  16, 0x3120);
    }
};

template <typename TypeB, unsigned AccumBits>
struct mmul_16_8<8, 4, 4, int16, TypeB, AccumBits> : public C_block<int16, TypeB, AccumBits, 32, 2>
{
    using vector_A_type = vector<int16, 32>;
    using vector_B_type = vector<TypeB, 16>;

    using C_block<int16, TypeB, AccumBits, 32, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mac16(this->data[0],
                                a,                      0, 0x02020000, 0x06060404,  2, 0x1010,
                                b.template grow<32>(),  0, 0x31203120, 0x31203120,  8, 0x3120);

        this->data[1] = ::mac16(this->data[1],
                                a,                     16, 0x02020000, 0x06060404,  2, 0x1010,
                                b.template grow<32>(),  0, 0x31203120, 0x31203120,  8, 0x3120);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mul16(a,                      0, 0x02020000, 0x06060404,  2, 0x1010,
                                b.template grow<32>(),  0, 0x31203120, 0x31203120,  8, 0x3120);

        this->data[1] = ::mul16(a,                     16, 0x02020000, 0x06060404,  2, 0x1010,
                                b.template grow<32>(),  0, 0x31203120, 0x31203120,  8, 0x3120);
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, int16, int8, 48> : public mmul_16_8<M, K, N, int16, int8, 48>   { using mmul_16_8<M, K, N, int16, int8, 48>::mmul_16_8; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, int16, uint8, 48> : public mmul_16_8<M, K, N, int16, uint8, 48> { using mmul_16_8<M, K, N, int16, uint8, 48>::mmul_16_8; };

}

#endif
