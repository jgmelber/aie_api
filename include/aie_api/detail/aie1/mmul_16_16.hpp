// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_MMUL_16_16__HPP__
#define __AIE_API_DETAIL_AIE1_MMUL_16_16__HPP__

#include "../vector.hpp"
#include "../ld_st.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, typename T1, typename T2, unsigned AccumBits>
struct mmul_16_16;

template <unsigned AccumBits>
struct mmul_16_16<4, 4, 4, int16, int16, AccumBits> : public C_block<int16, int16, AccumBits, 16, 1>
{
    using vector_A_type = vector<int16, 16>;
    using vector_B_type = vector<int16, 16>;

    using C_block<int16, int16, AccumBits, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac16(this->data,
                             a.template grow<32>(), 0, 0x02020000, 0x06060404, 0x1010,
                             b,                     0, 0x32103210, 0x32103210, 4);

        this->data = ::mac16(this->data,
                             a.template grow<32>(), 2, 0x02020000, 0x06060404, 0x1010,
                             b,                     8, 0x32103210, 0x32103210, 4);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul16(a.template grow<32>(), 0, 0x02020000, 0x06060404, 0x1010,
                             b,                     0, 0x32103210, 0x32103210, 4);

        this->data = ::mac16(this->data,
                             a.template grow<32>(), 2, 0x02020000, 0x06060404, 0x1010,
                             b,                     8, 0x32103210, 0x32103210, 4);
    }
};

template <unsigned AccumBits>
struct mmul_16_16<2, 4, 8, int16, int16, AccumBits> : public C_block<int16, int16, AccumBits, 16, 1>
{
    using vector_A_type = vector<int16, 8>;
    using vector_B_type = vector<int16, 32>;

    using C_block<int16, int16, AccumBits, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac16(this->data,
                             a.template grow<32>(),     0, 0x00000000, 0x02020202, 0x1010,
                             b.template extract<16>(0), 0, 0x76543210, 0x76543210, 8);

        this->data = ::mac16(this->data,
                             a.template grow<32>(),     2, 0x00000000, 0x02020202, 0x1010,
                             b.template extract<16>(1), 0, 0x76543210, 0x76543210, 8);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul16(a.template grow<32>(),     0, 0x00000000, 0x02020202, 0x1010,
                             b.template extract<16>(0), 0, 0x76543210, 0x76543210, 8);

        this->data = ::mac16(this->data,
                             a.template grow<32>(),     2, 0x00000000, 0x02020202, 0x1010,
                             b.template extract<16>(1), 0, 0x76543210, 0x76543210, 8);
    }
};

template <unsigned AccumBits>
struct mmul_16_16<4, 4, 8, int16, int16, AccumBits> : public C_block<int16, int16, AccumBits, 32, 2>
{
    using vector_A_type = vector<int16, 16>;
    using vector_B_type = vector<int16, 32>;

    using C_block<int16, int16, AccumBits, 32, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mac16(this->data[0],
                                a.template grow<32>(),     0, 0x00000000, 0x02020202, 0x1010,
                                b.template extract<16>(0), 0, 0x76543210, 0x76543210, 8);
        this->data[0] = ::mac16(this->data[0],
                                a.template grow<32>(),     2, 0x00000000, 0x02020202, 0x1010,
                                b.template extract<16>(1), 0, 0x76543210, 0x76543210, 8);

        this->data[1] = ::mac16(this->data[1],
                                a.template grow<32>(),     8, 0x00000000, 0x02020202, 0x1010,
                                b.template extract<16>(0), 0, 0x76543210, 0x76543210, 8);
        this->data[1] = ::mac16(this->data[1],
                                a.template grow<32>(),     10, 0x00000000, 0x02020202, 0x1010,
                                b.template extract<16>(1),  0, 0x76543210, 0x76543210, 8);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mul16(a.template grow<32>(),     0, 0x00000000, 0x02020202, 0x1010,
                                b.template extract<16>(0), 0, 0x76543210, 0x76543210, 8);
        this->data[0] = ::mac16(this->data[0],
                                a.template grow<32>(),     2, 0x00000000, 0x02020202, 0x1010,
                                b.template extract<16>(1), 0, 0x76543210, 0x76543210, 8);

        this->data[1] = ::mul16(a.template grow<32>(),     8, 0x00000000, 0x02020202, 0x1010,
                                b.template extract<16>(0), 0, 0x76543210, 0x76543210, 8);
        this->data[1] = ::mac16(this->data[1],
                                a.template grow<32>(),     10, 0x00000000, 0x02020202, 0x1010,
                                b.template extract<16>(1),  0, 0x76543210, 0x76543210, 8);
    }
};

template <unsigned AccumBits>
struct mmul_16_16<4, 2, 8, int16, int16, AccumBits> : public C_block<int16, int16, AccumBits, 32, 2>
{
    using vector_A_type = vector<int16, 8>;
    using vector_B_type = vector<int16, 16>;

    using C_block<int16, int16, AccumBits, 32, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mac16(this->data[0],
                                a.template grow<32>(), 0, 0x00000000, 0x01010101, 0x1010,
                                b,                     0, 0x76543210, 0x76543210, 8);

        this->data[1] = ::mac16(this->data[1],
                                a.template grow<32>(), 4, 0x00000000, 0x01010101, 0x1010,
                                b,                     0, 0x76543210, 0x76543210, 8);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mul16(a.template grow<32>(), 0, 0x00000000, 0x01010101, 0x1010,
                                b,                     0, 0x76543210, 0x76543210, 8);

        this->data[1] = ::mul16(a.template grow<32>(), 4, 0x00000000, 0x01010101, 0x1010,
                                b,                     0, 0x76543210, 0x76543210, 8);
    }
};

template <unsigned AccumBits>
struct mmul_16_16<8, 8, 1, int16, int16, AccumBits> : public C_block<int16, int16, AccumBits, 8, 1>
{
    using vector_A_type = vector<int16, 64>;
    using vector_B_type = vector<int16, 8>;

    using C_block<int16, int16, AccumBits, 8, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        auto a1 = ::select32(0xffff0000,
                             a.template extract<32>(0), 0, 0x0c080400, 0x0,        0x3210,
                             a.template extract<32>(1), 0, 0x0,        0x0c080400, 0x3210);
        this->data = ::mac8(this->data,
                            a1,                    0, 0x1c181410, 2, 0x3210,
                            b.template grow<16>(), 0, 0x0,        1);
        auto a2 = ::select32(0xffff0000,
                             a.template extract<32>(0), 4, 0x0c080400, 0x0,        0x3210,
                             a.template extract<32>(1), 4, 0x0,        0x0c080400, 0x3210);
        this->data = ::mac8(this->data,
                            a2,                    0, 0x1c181410, 2, 0x3210,
                            b.template grow<16>(), 4, 0x0,        1);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        auto a1 = ::select32(0xffff0000,
                             a.template extract<32>(0), 0, 0x0c080400, 0x0,        0x3210,
                             a.template extract<32>(1), 0, 0x0,        0x0c080400, 0x3210);
        this->data = ::mul8(a1,                    0, 0x1c181410, 2, 0x3210,
                            b.template grow<16>(), 0, 0x0,        1);
        auto a2 = ::select32(0xffff0000,
                             a.template extract<32>(0), 4, 0x0c080400, 0x0,        0x3210,
                             a.template extract<32>(1), 4, 0x0,        0x0c080400, 0x3210);
        this->data = ::mac8(this->data,
                            a2,                    0, 0x1c181410, 2, 0x3210,
                            b.template grow<16>(), 4, 0x0,        1);
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, int16, int16, 48> : public mmul_16_16<M, K, N, int16, int16, 48> { using mmul_16_16<M, K, N, int16, int16, 48>::mmul_16_16; };

}

#endif
