// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_BROADCAST__HPP__
#define __AIE_API_DETAIL_AIE2_BROADCAST__HPP__

#include "../../types.hpp"
#include "../accum.hpp"
#include "../vector.hpp"
#include "../scalar_pack.hpp"

namespace aie::detail {

namespace {

template <unsigned Bits> struct integer_bits_type;
template <> struct integer_bits_type<8>  { using type = uint8_t; };
template <> struct integer_bits_type<16> { using type = uint16_t; };
template <> struct integer_bits_type<32> { using type = uint32_t; };
template <> struct integer_bits_type<64> { using type = uint64_t; };

template <typename T, unsigned Bits>
using native_vector_from_bits_t = typename vector<T, Bits / type_bits_v<T>>::native_type;

template <unsigned TypeBits, typename T, unsigned Elems> struct native_broadcast;

template <typename T, unsigned Elems>
struct native_broadcast<8, T, Elems>
{
    using vector_type = vector<T, Elems>;

    template <typename T2>
        requires(sizeof(T2) == 1)
    __aie_inline
    static auto run(T2 a)
    {
        {
            using type = native_vector_from_bits_t<T, 512>;
            return (type)::broadcast_to_v64int8(__builtin_bit_cast(int8, a));
        }
    }

    template <typename T2, unsigned N>
    __aie_inline
    static auto run(const T2 (&a)[N]) requires(type_bits_v<T2> * N == 8)
    {
        using value_t = typename integer_bits_type<8>::type;

        value_t v = 0;
        utils::unroll_times<N>([&](unsigned idx) __aie_inline {
            value_t mask = (1u<<type_bits_v<T2>) -1;
            value_t e = mask & value_t(a[idx]);
            v = v | (e<<(idx*type_bits_v<T>));
        });
        return run(v);
    }

    __aie_inline
    static auto run_zeros()
    {
        {
            using type = native_vector_from_bits_t<T, 512>;
            return (type)::broadcast_zero_to_v64int8();
        }
    }
};

template <typename T, unsigned Elems>
struct native_broadcast<16, T, Elems>
{
    using vector_type = vector<T, Elems>;

    template <typename T2>
        requires(sizeof(T2) == 2)
    __aie_inline
    static auto run(T2 a)
    {
        {
            using type = native_vector_from_bits_t<T, 512>;
        //FIXME: Temporary workaround for CR-1230990
        if constexpr (std::is_same_v<T, bfloat16>) return ::broadcast_to_v32bfloat16(a);
        else
            return (type)::broadcast_to_v32int16(__builtin_bit_cast(int16, a));
        }
    }

    template <typename T2, unsigned N>
    __aie_inline
    static auto run(const T2 (&a)[N]) requires(type_bits_v<T2> * N == 16)
    {
        using value_t = typename integer_bits_type<16>::type;
        using input_t = typename integer_bits_type<type_bits_v<T2>>::type;

        value_t v = 0;
        utils::unroll_times<N>([&](unsigned idx) __aie_inline {
            value_t e = __builtin_bit_cast(input_t, a[idx]);
            v = v | (e<<(idx*type_bits_v<T>));
        });
        return run(v);
    }

    __aie_inline
    static auto run_zeros()
    {
        {
            using type = native_vector_from_bits_t<T, 512>;
        //FIXME: Temporary workaround for CR-1230990
        if constexpr (std::is_same_v<T, bfloat16>) return ::broadcast_zero_to_v32bfloat16();
        else
            return (type)::broadcast_zero_to_v32int16();
        }
    }
};

template <typename T, unsigned Elems>
struct native_broadcast<32, T, Elems>
{
    using vector_type = vector<T, Elems>;

    template <typename T2>
        requires(sizeof(T2) == 4)
    __aie_inline
    static auto run(T2 a)
    {
        {
            using type = native_vector_from_bits_t<T, 512>;
            return (type)::broadcast_to_v16int32(__builtin_bit_cast(int32, a));
        }
    }

    template <typename T2, unsigned N>
    __aie_inline
    static auto run(const T2 (&a)[N]) requires(type_bits_v<T2> * N == 32)
    {
        using value_t = typename integer_bits_type<32>::type;
        using input_t = typename integer_bits_type<type_bits_v<T2>>::type;

        value_t v = 0;
        utils::unroll_times<N>([&](unsigned idx) __aie_inline {
            value_t e = __builtin_bit_cast(input_t, a[idx]);
            v = v | (e<<(idx*type_bits_v<T>));
        });
        return run(v);
    }

    __aie_inline
    static auto run_zeros()
    {
        {
            using type = native_vector_from_bits_t<T, 512>;
            return (type)::broadcast_zero_to_v16int32();
        }
    }
};

template <typename T, unsigned Elems>
struct native_broadcast<64, T, Elems>
{
    using vector_type = vector<T, Elems>;

    template <typename T2>
        requires(sizeof(T2) == 8)
    __aie_inline
    static auto run(T2 a)
    {
#if !__AIE_API_COMPLEX_VECTOR_SUPPORT__
        // AIE2 64bit broadcast takes 'unsigned long long' as argument, AIE2p and later take v2int32
#   if __AIE_ARCH__ == 20
        using bcast_arg_t = unsigned long long;
#   else
        using bcast_arg_t = v2int32;
#   endif
        auto bcast_512  = [](bcast_arg_t i) __aie_inline { return ::broadcast_to_v16int32(i); };
#else
        // Casting from cint32 to v2int32/mask64 produces redundant copies that are not optimized correctly by the
        // compiler, so stick to cint32 for now.
        using bcast_arg_t = cint32;
        auto bcast_512  = [](bcast_arg_t i) __aie_inline { return ::broadcast_to_v8cint32(i); };
#endif

        bcast_arg_t value;
        if constexpr (std::is_same_v<T2, bcast_arg_t>)
            value = a;
        else
            value = __builtin_bit_cast(bcast_arg_t, a);
        {
            using type = native_vector_from_bits_t<T, 512>;
            return (type)bcast_512(value);
        }
    }

    template <typename T2, unsigned N>
    __aie_inline
    static auto run(const T2 (&a)[N]) requires(type_bits_v<T2> * N == 64)
    {
        constexpr unsigned elems_per_word = 32u / type_bits_v<T2>;

        using input_t = typename integer_bits_type<type_bits_v<T2>>::type;

        alignas(v2int32) unsigned int w[2] = {0, 0};
        utils::unroll_times_2d<2, elems_per_word>([&](unsigned y, unsigned x) __aie_inline {
            w[y] = w[y] | unsigned(__builtin_bit_cast(input_t, a[y * elems_per_word + x])) << type_bits_v<T2> * x;
        });

        // set_u64/set_uint64 intrinsics are not portable across architectures
        alignas(v2int32) unsigned long long v;
#if __AIE_ARCH__ == 20
        v = ::set_u64(0, w[0]);
#else
        v = ::set_uint64(0, w[0]);
#endif
        v = ::insert(v, 1, w[1]);

        return run(v);
    }

    static auto run_zeros()
    {
        {
            using type = native_vector_from_bits_t<T, 512>;
            return (type)::broadcast_zero_to_v16int32();
        }
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
        vector native_result = native_broadcast<8, T, Elems>::run({a, a});
        if constexpr (Elems <= native_result.size())
            return  native_result.template extract<Elems>(0);
        else
            return  native_result.template grow_replicate<Elems>();
    }

    template <unsigned N>
    __aie_inline
    static vector_type run(const T (&a)[N]) requires(N * 4 <= 64)
    {
        auto tmp = pack_to_scalar(a);

        vector native_result = native_broadcast<8 * sizeof(tmp), T, Elems>::run(tmp);
        if constexpr (Elems <= native_result.size())
            return  native_result.template extract<Elems>(0);
        else
            return  native_result.template grow_replicate<Elems>();
    }
};

template <unsigned TypeBits, typename T, unsigned Elems>
struct broadcast_bits_common_impl
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const T &a)
    {
        using broadcast_type = native_broadcast<TypeBits, T, Elems>;

        vector native_result = broadcast_type::run(a);

        if constexpr (Elems <= native_result.size()) {
            return native_result.template extract<Elems>(0);
        }
        else {
            return native_result.template grow_replicate<Elems>();
        }
    }

    template <unsigned Elems2>
        requires(Elems2 * TypeBits <= 64)
    __aie_inline
    static vector_type run(const T (&a)[Elems2])
    {
        using broadcast_type = native_broadcast<TypeBits * Elems2, T, Elems>;

        vector native_result = broadcast_type::run(a);

        if constexpr (Elems <= native_result.size()) {
            return native_result.template extract<Elems>(0);
        }
        else {
            return native_result.template grow_replicate<Elems>();
        }
    }
};

template <typename T, unsigned Elems> struct broadcast_bits_impl<8,  T, Elems> : public broadcast_bits_common_impl<8,  T, Elems> {};
template <typename T, unsigned Elems> struct broadcast_bits_impl<16, T, Elems> : public broadcast_bits_common_impl<16, T, Elems> {};
template <typename T, unsigned Elems> struct broadcast_bits_impl<32, T, Elems> : public broadcast_bits_common_impl<32, T, Elems> {};
template <typename T, unsigned Elems> struct broadcast_bits_impl<64, T, Elems> : public broadcast_bits_common_impl<64, T, Elems> {};

template <unsigned TypeBits, typename T, unsigned Elems, unsigned N>
struct broadcast_vector_bits_common_impl
{
    using vector_type = vector<T, Elems>;
    using native_vector_type = vector<T, 512 / type_bits_v<T>>;

    static constexpr unsigned broadcast_vector_bits = N * type_bits_v<T>;

    template <unsigned Elems2>
    __aie_inline
    static vector_type run(const vector<T, Elems2> &v, unsigned index)
    {
        native_vector_type native_result;

        if constexpr (broadcast_vector_bits >= 128) {
            return v.template extract<N>(index).template grow_replicate<Elems>();
        }
        else if constexpr (broadcast_vector_bits == 64) {
            constexpr unsigned input_vector_bits = vector<T, Elems2>::bits();

            // Use v8int8 as cint32 is not supported in some architectures
            auto v_int8 = v.template cast_to<int8>();
            v8int8 broadcast_vector;
            if constexpr (input_vector_bits == 1024) {
                broadcast_vector = ::extract_v8int8(v_int8.template extract<64>(index / 8), index % 8);
            }
            else {
                broadcast_vector = ::extract_v8int8(v_int8.template grow<64>(), index);
            }
            vector<int8, 64> res = ::broadcast_to_v64int8(broadcast_vector);

            native_result = res.template cast_to<T>();
        }
        else if constexpr (broadcast_vector_bits == 32) {
            int32 broadcast_vector = v.template cast_to<int32>().get(index);
            vector<int32, 16> res = ::broadcast_to_v16int32(broadcast_vector);

            native_result = res.template cast_to<T>();
        }
        else if constexpr (broadcast_vector_bits == 16) {
            int16 broadcast_vector = v.template cast_to<int16>().get(index);
            vector<int16, 32> res = ::broadcast_to_v32int16(broadcast_vector);

            native_result = res.template cast_to<T>();
        }
        else if constexpr (broadcast_vector_bits == 8) {
            int8 broadcast_vector = v.template cast_to<int8>().get(index);
            vector<int8, 64> res = ::broadcast_to_v64int8(broadcast_vector);

            native_result = res.template cast_to<T>();
        }

        if constexpr (Elems <= native_result.size()) {
            return native_result.template extract<Elems>(0);
        }
        else {
            return native_result.template grow_replicate<Elems>();
        }
    }
};

template <typename T, unsigned Elems, unsigned N> struct broadcast_vector_bits_impl<4,  T, Elems, N> : public broadcast_vector_bits_common_impl<4,  T, Elems, N> {};
template <typename T, unsigned Elems, unsigned N> struct broadcast_vector_bits_impl<8,  T, Elems, N> : public broadcast_vector_bits_common_impl<8,  T, Elems, N> {};
template <typename T, unsigned Elems, unsigned N> struct broadcast_vector_bits_impl<16, T, Elems, N> : public broadcast_vector_bits_common_impl<16, T, Elems, N> {};
template <typename T, unsigned Elems, unsigned N> struct broadcast_vector_bits_impl<32, T, Elems, N> : public broadcast_vector_bits_common_impl<32, T, Elems, N> {};
template <typename T, unsigned Elems, unsigned N> struct broadcast_vector_bits_impl<64, T, Elems, N> : public broadcast_vector_bits_common_impl<64, T, Elems, N> {};

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
