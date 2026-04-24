// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_BLEND__HPP__
#define __AIE_API_DETAIL_AIE2_BLEND__HPP__

#include <algorithm>

#include "../vector.hpp"
#include "../mask.hpp"

namespace aie::detail {

template <unsigned Elems>
struct mask_backing_storage;

consteval unsigned select_native_elems(unsigned elems, unsigned elem_bits)
{
    const unsigned bits = elems * elem_bits;
    const unsigned native_lo = 512u;
    const unsigned native_hi = 512u;
    const unsigned native_bits = std::clamp(bits, native_lo, native_hi);
    return native_bits / elem_bits;
}

template <typename T, unsigned Elems>
struct select_bits_impl<4, T, Elems>
{
     static constexpr unsigned native_elems = select_native_elems(Elems, 8);
     static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

     using vector_type = vector<T, Elems>;

     __aie_inline
     static vector_type run(const vector_type &v1, const vector_type &v2, const mask<Elems> &m)
     {
         using next_type = utils::get_next_integer_type_t<T>;
         using select_op = select_bits_impl<8, next_type, std::min(Elems, native_elems)>;

         if constexpr (Elems <= native_elems) {
             return select_op::run(v1.unpack(), v2.unpack(), m).pack();
         }
         else {
            vector_type ret;
            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                auto chunk = select_op::run(v1.template extract<native_elems>(idx).unpack(),
                                            v2.template extract<native_elems>(idx).unpack(),
                                            m.template extract<native_elems>(idx));
                ret.insert(idx, chunk.pack());
            });
            return ret;
         }
     }

     template <unsigned Elems2>
     __aie_inline
     static vector_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v, const mask<Elems> &m)
     {
         return run((T)a, v, m);
     }

     template <unsigned Elems2>
     __aie_inline
     static vector_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a, const mask<Elems> &m)
     {
         return run(v, (T)a, m);
     }

     __aie_inline
     static vector_type run(const T &a, const vector_type &v, const mask<Elems> &m)
     {
         return run(broadcast<T, Elems>::run(a), v, m);
     }

     __aie_inline
     static vector_type run(const vector_type &v, const T &a, const mask<Elems> &m)
     {
         return run(v, broadcast<T, Elems>::run(a), m);
     }

     __aie_inline
     static vector_type run(T a, T b, const mask<Elems> &m)
     {
         return run(broadcast<T, Elems>::run(a), broadcast<T, Elems>::run(b), m);
     }
};

template <typename T, unsigned Elems>
struct select_bits_common
{
    static constexpr unsigned native_elems = select_native_elems(Elems, type_bits_v<T>);
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using native_select = select_bits_common<T, native_elems>;
    using vector_type = vector<T, Elems>;

     __aie_inline
    static vector_type run(const vector_type &v1, const vector_type &v2, const mask<Elems> &m)
    {
        auto select_mask = [&](unsigned i) __aie_inline {
            if constexpr (native_elems <= 32)
                return m.to_uint32(i);
#if !__AIECC__ // Peano does not expose mask64 intrinsic type: use unsigned long long instead
            else if constexpr (std::is_same_v<v2uint32, typename mask_backing_storage<Elems>::type>) {
                mask64 a = __builtin_bit_cast(mask64, m);
                return a;
            }
#endif
            else if constexpr (native_elems == 64) {
                return m.to_uint64(i);
            }
            static_assert(native_elems <= 64, "Not supported for 128bit or larger masks");
        };

        if constexpr (Elems < native_elems) {
            return native_select::run(v1.template grow<native_elems>(),
                                      v2.template grow<native_elems>(),
                                      m.template grow<native_elems>())
                        .template extract<Elems>(0);
        }
        else if constexpr (Elems == native_elems) {
            const vector_type ret = ::sel(v1, v2, select_mask(0));
            return ret;

        }
        else {
            vector_type ret;
            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                auto chunk = native_select::run(v1.template extract<native_elems>(idx),
                                                v2.template extract<native_elems>(idx),
                                                m.template extract<native_elems>(idx));
                ret.insert(idx, chunk);
            });
            return ret;
        }
    }

    template <unsigned Elems2>
     __aie_inline
    static vector_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v, const mask<Elems> &m)
    {
        return run((T)a, v, m);
    }

    template <unsigned Elems2>
     __aie_inline
    static vector_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a, const mask<Elems> &m)
    {
        return run(v, (T)a, m);
    }

     __aie_inline
    static vector_type run(const T &a, const vector_type &v, const mask<Elems> &m)
    {
        if constexpr (Elems <= native_elems) {
            return run(broadcast<T, Elems>::run(a), v, m);
        }
        else {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            vector_type ret;
            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                auto chunk = native_select::run(vals,
                                                v.template extract<native_elems>(idx),
                                                m.template extract<native_elems>(idx));

                ret.insert(idx, chunk);
            });

            return ret;
        }
    }

     __aie_inline
    static vector_type run(const vector_type &v, const T &a, const mask<Elems> &m)
    {
        if constexpr (Elems <= native_elems) {
            return run(v, broadcast<T, Elems>::run(a), m);
        }
        else {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            vector_type ret;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                auto chunk = native_select::run(v.template extract<native_elems>(idx),
                                                vals,
                                                m.template extract<native_elems>(idx));
                ret.insert(idx, chunk);
            });

            return ret;
        }
    }

     __aie_inline
    static vector_type run(T a, T b, const mask<Elems> &m)
    {
        if constexpr (Elems <= native_elems) {
            return run(broadcast<T, Elems>::run(a),
                       broadcast<T, Elems>::run(b),
                       m);
        }
        else {
            const auto va = broadcast<T, native_elems>::run(a),
                       vb = broadcast<T, native_elems>::run(b);

            vector_type ret;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                auto chunk = native_select::run(va, vb,
                                                m.template extract<native_elems>(idx));
                ret.insert(idx, chunk);
            });

            return ret;
        }
    }
};

template <typename T, unsigned Elems> struct select_bits_impl< 8, T, Elems> : public select_bits_common<T, Elems> {};
template <typename T, unsigned Elems> struct select_bits_impl<16, T, Elems> : public select_bits_common<T, Elems> {};
template <typename T, unsigned Elems> struct select_bits_impl<32, T, Elems> : public select_bits_common<T, Elems> {};
template <typename T, unsigned Elems> struct select_bits_impl<64, T, Elems> : public select_bits_common<T, Elems> {};

}

#endif
