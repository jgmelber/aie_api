// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_BROADCAST__HPP__
#define __AIE_API_DETAIL_AIE2_BROADCAST__HPP__

#include "../../types.hpp"
#include "../accum.hpp"
#include "../vector.hpp"

namespace aie::detail {

namespace
{
template <unsigned TypeBits, typename T, unsigned Elems> struct native_broadcast;

template <typename T, unsigned Elems>
struct native_broadcast<8, T, Elems>
{
    using type = typename vector<T, Elems>::native_type;

    static auto run(T a) {
        return (type)::broadcast_to_v64int8(__builtin_bit_cast(int8, a));
    }

    static auto run_zeros() {
        return (type)::broadcast_zero_to_v64int8();
    }
};

template <typename T, unsigned Elems>
struct native_broadcast<16, T, Elems>
{
    using type = typename vector<T, Elems>::native_type;

    static auto run(T a) {
        return (type)::broadcast_to_v32int16(__builtin_bit_cast(int16, a));
    }

    static auto run_zeros() {
        return (type)::broadcast_zero_to_v32int16();
    }
};

template <typename T, unsigned Elems>
struct native_broadcast<32, T, Elems>
{
    using type = typename vector<T, Elems>::native_type;

    static auto run(T a) {
        return (type)::broadcast_to_v16int32(__builtin_bit_cast(int32, a));
    }

    static auto run_zeros() {
        return (type)::broadcast_zero_to_v16int32();
    }
};

template <typename T, unsigned Elems>
struct native_broadcast<64, T, Elems>
{
    using type = typename vector<T, Elems>::native_type;

    static auto run(T a) {
        return (type)::broadcast_to_v8cint32(__builtin_bit_cast(cint32, a));
    }

    static auto run_zeros() {
        return (type)::broadcast_zero_to_v8cint32();
    }
};
} // namespace


template <typename T, unsigned Elems>
struct broadcast_bits_impl<4, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const T &a)
    {
        using next_type = utils::get_next_integer_type_t<T>;

        next_type tmp = (uint4) a | ((uint4) a << 4);

        if constexpr (Elems == 32)
            return  broadcast_bits_impl<8, next_type, Elems>::run(tmp).template extract<Elems / 2>(0).template cast_to<T>();
        else
            return  broadcast_bits_impl<8, next_type, Elems / 2>::run(tmp).template cast_to<T>();
    }
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct broadcast_bits_common_impl
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const T &a)
    {
        constexpr unsigned native_broadcast_elems = max_intrinsic_vector_elems<T>::value;
        using native_broadcast_type = broadcast_bits_impl<TypeBits, T, native_broadcast_elems>;

        vector_type ret;

        if constexpr (Elems < native_broadcast_elems) {
            ret = native_broadcast_type::run(a).template extract<Elems>(0);
        }
        else if constexpr (Elems == native_broadcast_elems) {
            ret = native_broadcast<TypeBits, T, native_broadcast_elems>::run(a);
        }
        else {
            const auto tmp = native_broadcast_type::run(a);

            utils::unroll_times<Elems / native_broadcast_elems>([&](unsigned idx) __aie_inline {
                ret.insert(idx, tmp);
            });
        }

        return ret;
    }
};

template <typename T, unsigned Elems> struct broadcast_bits_impl<8,  T, Elems> : public broadcast_bits_common_impl<8,  T, Elems> {};
template <typename T, unsigned Elems> struct broadcast_bits_impl<16, T, Elems> : public broadcast_bits_common_impl<16, T, Elems> {};
template <typename T, unsigned Elems> struct broadcast_bits_impl<32, T, Elems> : public broadcast_bits_common_impl<32, T, Elems> {};
template <typename T, unsigned Elems> struct broadcast_bits_impl<64, T, Elems> : public broadcast_bits_common_impl<64, T, Elems> {};


template <typename T, unsigned Elems>
struct zeros_bits_impl<4, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run()
    {
        using next_type = utils::get_next_integer_type_t<T>;

        if constexpr (Elems == 32)
            return zeros_bits_impl<8, next_type, Elems>::run().template extract<Elems / 2>(0).template cast_to<T>();
        else
            return zeros_bits_impl<8, next_type, Elems / 2>::run().template cast_to<T>();
    }
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct zeros_bits_common_impl
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run()
    {
        constexpr unsigned native_zeros_elems = max_intrinsic_vector_elems<T>::value;
        using native_zeros_type = zeros_bits_impl<TypeBits, T, native_zeros_elems>;

        vector_type ret;

        if constexpr (Elems < native_zeros_elems) {
            ret = native_zeros_type::run().template extract<Elems>(0);
        }
        else if constexpr (Elems == native_zeros_elems) {
            ret = native_broadcast<TypeBits, T, native_zeros_elems>::run_zeros();
        }
        else {
            const auto tmp = native_zeros_type::run();

            utils::unroll_times<Elems / native_zeros_elems>([&](unsigned idx) __aie_inline {
                ret.insert(idx, tmp);
            });
        }

        return ret;
    }
};

template <typename T, unsigned Elems> struct zeros_bits_impl<8,  T, Elems> : public zeros_bits_common_impl<8,  T, Elems> {};
template <typename T, unsigned Elems> struct zeros_bits_impl<16, T, Elems> : public zeros_bits_common_impl<16, T, Elems> {};
template <typename T, unsigned Elems> struct zeros_bits_impl<32, T, Elems> : public zeros_bits_common_impl<32, T, Elems> {};
template <typename T, unsigned Elems> struct zeros_bits_impl<64, T, Elems> : public zeros_bits_common_impl<64, T, Elems> {};


template <AccumClass Class, unsigned AccumBits, unsigned Elems>
struct zeros_acc_bits_impl
{
    using  accum_tag = accum_tag_t<Class, AccumBits>;
    using accum_type = accum<accum_tag, Elems>;

    __aie_inline
    static constexpr auto get_op()
    {
#if __AIE_API_COMPLEX_FP32_EMULATION__
        if constexpr (Class == AccumClass::CFP) {
            return []() {
                // Defer zeroization to the FP variant, then cast the result to a complex accumulator.
                using base             = zeros_acc_bits_impl<AccumClass::FP, AccumBits, Elems * 2>;
                constexpr auto base_op = base::get_op();
                return accum(base_op()).template cast_to<caccfloat>();
            };
        }
#endif
#if __AIE_ARCH__ == 20
        if constexpr (AccumBits == 32) {
            if constexpr (Class == AccumClass::Int)  return []() { return ::broadcast_zero_to_v32acc32();    };
            if constexpr (Class == AccumClass::CInt) return []() { return ::broadcast_zero_to_v8cacc64();    };
            if constexpr (Class == AccumClass::FP)   return []() { return ::broadcast_zero_to_v16accfloat(); };
        }
        else if constexpr (AccumBits <= 64) {
            if constexpr (Class == AccumClass::Int)  return []() { return ::broadcast_zero_to_v16acc64();    };
            if constexpr (Class == AccumClass::CInt) return []() { return ::broadcast_zero_to_v8cacc64();    };
        }
#else
#if __AIE_API_ACC_BROADCAST_NAME__
        if constexpr (AccumBits == 32) {
            if constexpr (Class == AccumClass::Int)               return []() { return ::broadcast_zero_to_v64acc32();    };
            if constexpr (Class == AccumClass::CInt)              return []() { return ::broadcast_zero_to_v16cacc64();   };
            if constexpr (Class == AccumClass::FP && Elems <= 16) return []() { return ::broadcast_zero_to_v16accfloat(); };
            if constexpr (Class == AccumClass::FP && Elems == 32) return []() { return ::broadcast_zero_to_v32accfloat(); };
            if constexpr (Class == AccumClass::FP && Elems >= 64) return []() { return ::broadcast_zero_to_v64accfloat(); };
        }
        else if constexpr (AccumBits <= 64) {
            if constexpr (Class == AccumClass::Int)  return []() { return ::broadcast_zero_to_v32acc64();  };
            if constexpr (Class == AccumClass::CInt) return []() { return ::broadcast_zero_to_v16cacc64(); };
        }
#else
        if constexpr (AccumBits == 32) {
            if constexpr (Class == AccumClass::Int)               return []() { return ::clr64();  };
            if constexpr (Class == AccumClass::CInt)              return []() { return ::clr32c(); };
            if constexpr (Class == AccumClass::FP && Elems <= 32) return []() { return ::clr32f(); };
            if constexpr (Class == AccumClass::FP && Elems >= 64) return []() { return ::clr64f(); };
        }
        else if constexpr (AccumBits <= 64) {
            if constexpr (Class == AccumClass::Int)  return []() { return ::clr32();  };
            if constexpr (Class == AccumClass::CInt) return []() { return ::clr32c();  };
        }
#endif
#endif
    }

    __aie_inline
    static accum_type run()
    {
        auto op = get_op();
        accum tmp = op();
        accum_type ret;
        if constexpr (ret.size() <= tmp.size())
            ret = tmp.template extract<Elems>(0);
        else
            utils::unroll_times<ret.size() / tmp.size()>([&](auto idx) __aie_inline {
                ret.insert(idx, tmp);
            });
        return ret;
    }
};

}

#endif
