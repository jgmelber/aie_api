// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_MMUL_8_8__HPP__
#define __AIE_API_DETAIL_AIE1_MMUL_8_8__HPP__

#include "../accum.hpp"
#include "../vector.hpp"
#include "../ld_st.hpp"
#include "../mul.hpp"

namespace aie::detail {

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits>
struct mmul_8_8;

template <typename TypeA, typename TypeB>
struct mmul_8_8<4, 8, 4, TypeA, TypeB, 48> : public C_block<TypeA, TypeB, 48, 16, 1>
{
    using vector_A_type = vector<TypeA, 32>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block<TypeA, TypeB, 48, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac16(this->data,
                             b.template grow<64>(), 0, 0x00000000, 8, 0x3120,
                             a,                     0, 0xcc884400, 2, 0x3210);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul16(b.template grow<64>(), 0, 0x00000000, 8, 0x3120,
                             a,                     0, 0xcc884400, 2, 0x3210);
    }
};

template <typename TypeA, typename TypeB>
struct mmul_8_8<8, 8, 4, TypeA, TypeB, 48> : public C_block<TypeA, TypeB, 48, 32, 1>
{
    using vector_A_type = vector<TypeA, 64>;
    using vector_B_type = vector<TypeB, 32>;

    using C_block<TypeA, TypeB, 48, 32, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data.template insert<16>(0, ::mac16(this->data.template extract<16>(0),
                                                  b.template grow<64>(),     0, 0x00000000, 8, 0x3120,
                                                  a.template extract<32>(0), 0, 0xcc884400, 2, 0x3210));

        this->data.template insert<16>(1, ::mac16(this->data.template extract<16>(1),
                                                  b.template grow<64>(),     0, 0x00000000, 8, 0x3120,
                                                  a.template extract<32>(1), 0, 0xcc884400, 2, 0x3210));
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data.template insert<16>(0, ::mul16(b.template grow<64>(),     0, 0x00000000, 8, 0x3120,
                                                  a.template extract<32>(0), 0, 0xcc884400, 2, 0x3210));

        this->data.template insert<16>(1, ::mul16(b.template grow<64>(),     0, 0x00000000, 8, 0x3120,
                                                  a.template extract<32>(1), 0, 0xcc884400, 2, 0x3210));
    }
};

template <typename TypeA, typename TypeB>
struct mmul_8_8<4, 8, 8, TypeA, TypeB, 48> : public C_block<TypeA, TypeB, 48, 32, 1>
{
    using vector_A_type = vector<TypeA, 32>;
    using vector_B_type = vector<TypeB, 64>;

    using C_block<TypeA, TypeB, 48, 32, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data.template insert<16>(0, ::mac16(this->data.template extract<16>(0),
                                                  b, 0, 0x11101110, 16, 0x3120,
                                                  a, 0, 0x44440000,  2, 0x3210));

        this->data.template insert<16>(1, ::mac16(this->data.template extract<16>(1),
                                                  b, 0, 0x11101110, 16, 0x3120,
                                                  a, 0, 0xcccc8888,  2, 0x3210));
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data.template insert<16>(0, ::mul16(b, 0, 0x11101110, 16, 0x3120,
                                                  a, 0, 0x44440000,  2, 0x3210));

        this->data.template insert<16>(1, ::mul16(b, 0, 0x11101110, 16, 0x3120,
                                                  a, 0, 0xcccc8888,  2, 0x3210));
    }

};

template <typename TypeA, typename TypeB>
struct mmul_8_8<2, 8, 8, TypeA, TypeB, 48> : public C_block<TypeA, TypeB, 48, 16, 1>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 64>;

    using C_block<TypeA, TypeB, 48, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac16(this->data,
                             b,                     0, 0x11101110, 16, 0x3120,
                             a.template grow<32>(), 0, 0x44440000,  2, 0x3210);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul16(b,                     0, 0x11101110, 16, 0x3120,
                             a.template grow<32>(), 0, 0x44440000,  2, 0x3210);
    }
};

template <typename TypeA, typename TypeB>
struct mmul_8_8<1, 16, 8, TypeA, TypeB, 48> : public C_block<TypeA, TypeB, 48, 8, 1>
{
    using vector_A_type = vector<TypeA, 16>;
    using vector_B_type = vector<TypeB, 128>;

    using C_block<TypeA, TypeB, 48, 8, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac8(this->data,
                            b,                     0, 0x11101110, 16, 0x3120,
                            a.template grow<32>(), 0, 0x00000000,  2, 0x3210);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul8(b,                     0, 0x11101110, 16, 0x3120,
                            a.template grow<32>(), 0, 0x00000000,  2, 0x3210);
    }
};

template <typename TypeA, typename TypeB>
struct mmul_8_8<2, 16, 8, TypeA, TypeB, 48> : public C_block<TypeA, TypeB, 48, 16, 1>
{
    using vector_A_type = vector<TypeA, 32>;
    using vector_B_type = vector<TypeB, 128>;

    using C_block<TypeA, TypeB, 48, 16, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mac16(this->data,
                             b.template extract<64>(0), 0, 0x11101110, 16, 0x3120,
                             a,                         0, 0x88880000,  2, 0x3210);

        this->data = ::mac16(this->data,
                             b.template extract<64>(1), 0, 0x11101110, 16, 0x3120,
                             a,                         8, 0x88880000,  2, 0x3210);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data = ::mul16(b.template extract<64>(0), 0, 0x11101110, 16, 0x3120,
                             a,                         0, 0x88880000,  2, 0x3210);

        this->data = ::mac16(this->data,
                             b.template extract<64>(1), 0, 0x11101110, 16, 0x3120,
                             a,                         8, 0x88880000,  2, 0x3210);
    }
};

template <typename TypeA, typename TypeB>
struct mmul_8_8<4, 16, 4, TypeA, TypeB, 48> : public C_block<TypeA, TypeB, 48, 16, 2>
{
    using vector_A_type = vector<TypeA, 64>;
    using vector_B_type = vector<TypeB, 64>;

    using C_block<TypeA, TypeB, 48, 16, 2>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mac8(this->data[0],
                               b,                         0, 0x00000000, 8, 0x3120,
                               a.template extract<32>(0), 0, 0x00008800, 2, 0x3210);

        this->data[1] = ::mac8(this->data[1],
                               b,                         0, 0x00000000, 8, 0x3120,
                               a.template extract<32>(1), 0, 0x00008800, 2, 0x3210);
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data[0] = ::mul8(b,                         0, 0x00000000, 8, 0x3120,
                               a.template extract<32>(0), 0, 0x00008800, 2, 0x3210);

        this->data[1] = ::mul8(b,                         0, 0x00000000, 8, 0x3120,
                               a.template extract<32>(1), 0, 0x00008800, 2, 0x3210);
    }
};

template <typename TypeA, typename TypeB>
struct mmul_8_8<4, 16, 8, TypeA, TypeB, 48> : public C_block<TypeA, TypeB, 48, 32, 1>
{
    using vector_A_type = vector<TypeA, 64>;
    using vector_B_type = vector<TypeB, 128>;

    using C_block<TypeA, TypeB, 48, 32, 1>::C_block;

    __aie_inline void mac(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data.template insert<16>(0, ::mac16(this->data.template extract<16>(0),
                                                  b.template extract<64>(0), 0, 0x11101110, 16, 0x3120,
                                                  a.template extract<32>(0), 0, 0x88880000,  2, 0x3210));

        this->data.template insert<16>(1, ::mac16(this->data.template extract<16>(1),
                                                  b.template extract<64>(0), 0, 0x11101110, 16, 0x3120,
                                                  a.template extract<32>(1), 0, 0x88880000,  2, 0x3210));

        this->data.template insert<16>(0, ::mac16(this->data.template extract<16>(0),
                                                  b.template extract<64>(1), 0, 0x11101110, 16, 0x3120,
                                                  a.template extract<32>(0), 8, 0x88880000,  2, 0x3210));

        this->data.template insert<16>(1, ::mac16(this->data.template extract<16>(1),
                                                  b.template extract<64>(1), 0, 0x11101110, 16, 0x3120,
                                                  a.template extract<32>(1), 8, 0x88880000,  2, 0x3210));
    }

    __aie_inline void mul(const vector_A_type &a, bool a_sign, const vector_B_type &b, bool b_sign)
    {
        this->data.template insert<16>(0, ::mul16(b.template extract<64>(0), 0, 0x11101110, 16, 0x3120,
                                                  a.template extract<32>(0), 0, 0x88880000,  2, 0x3210));

        this->data.template insert<16>(1, ::mul16(b.template extract<64>(0), 0, 0x11101110, 16, 0x3120,
                                                  a.template extract<32>(1), 0, 0x88880000,  2, 0x3210));

        this->data.template insert<16>(0, ::mac16(this->data.template extract<16>(0),
                                                  b.template extract<64>(1), 0, 0x11101110, 16, 0x3120,
                                                  a.template extract<32>(0), 8, 0x88880000,  2, 0x3210));

        this->data.template insert<16>(1, ::mac16(this->data.template extract<16>(1),
                                                  b.template extract<64>(1), 0, 0x11101110, 16, 0x3120,
                                                  a.template extract<32>(1), 8, 0x88880000,  2, 0x3210));
    }
};

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, int8, int8, 48>   : public mmul_8_8<M, K, N, int8,  int8, 48>  { using mmul_8_8<M, K, N, int8,  int8, 48>::mmul_8_8; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, uint8, uint8, 48> : public mmul_8_8<M, K, N, uint8, uint8, 48> { using mmul_8_8<M, K, N, uint8, uint8, 48>::mmul_8_8; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, int8, uint8, 48>  : public mmul_8_8<M, K, N, int8,  uint8, 48> { using mmul_8_8<M, K, N, int8,  uint8, 48>::mmul_8_8; };

template <unsigned M, unsigned K, unsigned N>
struct mmul<M, K, N, uint8, int8, 48>  : public mmul_8_8<M, K, N, uint8, int8, 48>  { using mmul_8_8<M, K, N, uint8, int8, 48>::mmul_8_8; };

}

#endif
