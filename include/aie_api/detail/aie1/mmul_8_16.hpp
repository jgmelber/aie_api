// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_MMUL_8_16__HPP__
#define __AIE_API_DETAIL_AIE1_MMUL_8_16__HPP__

#include "../vector.hpp"
#include "../ld_st.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_8_16;

template <typename TypeA, unsigned AccumBits>
struct mmul_8_16<4, 4, 4, TypeA, int16, AccumBits> : public C_block<int16, int16, AccumBits, 16, 1>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<int16, 16>;

    using big_vector_A_type = vector<int16, 16>;

    using C_block<int16, int16, AccumBits, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        big_vector_A_type tmp = a.unpack();

        this->data = ::mac16(this->data,
                             tmp.template grow<32>(), 0, 0x02020000, 0x06060404, 0x1010,
                             b,                       0, 0x32103210, 0x32103210, 4);

        this->data = ::mac16(this->data,
                             tmp.template grow<32>(), 2, 0x02020000, 0x06060404, 0x1010,
                             b,                       0, 0xba98ba98, 0xba98ba98, 4);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        big_vector_A_type tmp = a.unpack();

        this->data = ::mul16(tmp.template grow<32>(), 0, 0x02020000, 0x06060404, 0x1010,
                             b,                       0, 0x32103210, 0x32103210, 4);

        this->data = ::mac16(this->data,
                             tmp.template grow<32>(), 2, 0x02020000, 0x06060404, 0x1010,
                             b,                       0, 0xba98ba98, 0xba98ba98, 4);
    }
};

template <typename TypeA, unsigned AccumBits>
struct mmul_8_16<4, 4, 8, TypeA, int16, AccumBits> : public C_block<int16, int16, AccumBits, 32, 2>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<int16, 32>;

    using big_vector_A_type = vector<int16, 16>;

    using C_block<int16, int16, AccumBits, 32, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        big_vector_A_type tmp = a.unpack();

        this->data[0] = ::mac16(this->data[0],
                                tmp.template grow<32>(),   0,  0x00000000, 0x02020202, 0x1010,
                                b.template extract<16>(0), 0,  0x76543210, 0x76543210, 8);

        this->data[0] = ::mac16(this->data[0],
                                tmp.template grow<32>(),   2,  0x00000000, 0x02020202, 0x1010,
                                b.template extract<16>(1), 0,  0x76543210, 0x76543210, 8);

        this->data[1] = ::mac16(this->data[1],
                                tmp.template grow<32>(),   8,  0x00000000, 0x02020202, 0x1010,
                                b.template extract<16>(0), 0,  0x76543210, 0x76543210, 8);

        this->data[1] = ::mac16(this->data[1],
                                tmp.template grow<32>(),   10, 0x00000000, 0x02020202, 0x1010,
                                b.template extract<16>(1), 0,  0x76543210, 0x76543210, 8);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        big_vector_A_type tmp = a.unpack();

        this->data[0] = ::mul16(tmp.template grow<32>(),   0,  0x00000000, 0x02020202, 0x1010,
                                b.template extract<16>(0), 0,  0x76543210, 0x76543210, 8);

        this->data[0] = ::mac16(this->data[0],
                                tmp.template grow<32>(),   2,  0x00000000, 0x02020202, 0x1010,
                                b.template extract<16>(1), 0,  0x76543210, 0x76543210, 8);

        this->data[1] = ::mul16(tmp.template grow<32>(),   8,  0x00000000, 0x02020202, 0x1010,
                                b.template extract<16>(0), 0,  0x76543210, 0x76543210, 8);

        this->data[1] = ::mac16(this->data[1],
                                tmp.template grow<32>(),   10, 0x00000000, 0x02020202, 0x1010,
                                b.template extract<16>(1), 0,  0x76543210, 0x76543210, 8);
    }
};

template <typename TypeA, unsigned AccumBits>
struct mmul_8_16<8, 8, 1, TypeA, int16, AccumBits> : public C_block<int16, int16, AccumBits, 8, 1>
{
    using vector_A_type = vector<TypeA, 64>;
    using vector_B_type = vector<int16, 8>;

    using big_vector_A_type = vector<int16, 64>;

    using C_block<int16, int16, AccumBits, 8, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        big_vector_A_type tmp = a.unpack();

        auto a1 = ::select32(0xffff0000,
                             tmp.template extract<32>(0), 0, 0x0c080400, 0x0,        0x3210,
                             tmp.template extract<32>(1), 0, 0x0,        0x0c080400, 0x3210);
        this->data = ::mac8(this->data,
                            a1,                    0, 0x1c181410, 2, 0x3210,
                            b.template grow<16>(), 0, 0x0,        1);
        auto a2 = ::select32(0xffff0000,
                             tmp.template extract<32>(0), 4, 0x0c080400, 0x0,        0x3210,
                             tmp.template extract<32>(1), 4, 0x0,        0x0c080400, 0x3210);
        this->data = ::mac8(this->data,
                            a2,                    0, 0x1c181410, 2, 0x3210,
                            b.template grow<16>(), 4, 0x0,        1);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        big_vector_A_type tmp = a.unpack();

        auto a1 = ::select32(0xffff0000,
                             tmp.template extract<32>(0), 0, 0x0c080400, 0x0,        0x3210,
                             tmp.template extract<32>(1), 0, 0x0,        0x0c080400, 0x3210);
        this->data = ::mul8(a1,                    0, 0x1c181410, 2, 0x3210,
                            b.template grow<16>(), 0, 0x0,        1);
        auto a2 = ::select32(0xffff0000,
                             tmp.template extract<32>(0), 4, 0x0c080400, 0x0,        0x3210,
                             tmp.template extract<32>(1), 4, 0x0,        0x0c080400, 0x3210);
        this->data = ::mac8(this->data,
                            a2,                    0, 0x1c181410, 2, 0x3210,
                            b.template grow<16>(), 4, 0x0,        1);
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, int8, int16, 48> : public mmul_8_16<M, K, N, int8, int16, 48> { using mmul_8_16<M, K, N, int8, int16, 48>::mmul_8_16; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, uint8, int16, 48> : public mmul_8_16<M, K, N, uint8, int16, 48> { using mmul_8_16<M, K, N, uint8, int16, 48>::mmul_8_16; };

}

#endif
