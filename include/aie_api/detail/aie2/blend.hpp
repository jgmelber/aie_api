// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_BLEND__HPP__
#define __AIE_API_DETAIL_AIE2_BLEND__HPP__

#include "../vector.hpp"

namespace aie::detail {

template <typename T, unsigned Elems>
struct select_bits_impl<4, T, Elems>
{
     static constexpr unsigned native_elems = native_vector_length_v<T>;
     static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

     using vector_type = vector<T, Elems>;

     __aie_inline
     static vector_type run(const vector_type &v1, const vector_type &v2, const mask<Elems> &m)
     {
         using next_type = utils::get_next_integer_type_t<T>;

         if constexpr (Elems <= native_elems) {
             return select_bits_impl<8, next_type, Elems>::run(v1.unpack(), v2.unpack(), m).pack();
         }
         else {
            using native_op = select_bits_impl<8, next_type, native_elems>;

            vector_type ret;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                auto submask = m.template extract<native_elems>(idx);

                vector<T, native_elems> tmp = native_op::run(v1.template extract<native_elems>(idx).unpack(),
                                                             v2.template extract<native_elems>(idx).unpack(), submask).pack();
                ret.insert(idx, tmp);
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
struct select_bits_impl<8, T, Elems>
{
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;

     __aie_inline
    static vector_type run(const vector_type &v1, const vector_type &v2, const mask<Elems> &m)
    {
        if constexpr (Elems < native_elems) {
            const vector<T, native_elems> tmp = ::sel(v1.template grow<native_elems>(),
                                                      v2.template grow<native_elems>(),
                                                      uint64_t(m.to_uint32(0)));

            return tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == native_elems) {
            const vector_type ret = ::sel(v1, v2, m.to_uint64(0));

            return ret;
        }
        else {
            vector_type ret;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                vector<T, native_elems> tmp = ::sel(v1.template extract<native_elems>(idx),
                                                    v2.template extract<native_elems>(idx),
                                                    m.to_uint64(idx));
                ret.insert(idx, tmp);
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
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;

     __aie_inline
    static vector_type run(const vector_type &v1, const vector_type &v2, const mask<Elems> &m)
    {
        if constexpr (Elems < native_elems) {
            const vector<T, native_elems> tmp = ::sel(v1.template grow<native_elems>(),
                                                      v2.template grow<native_elems>(),
                                                      m.to_uint32(0));

            return tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == native_elems) {
            const vector_type ret = ::sel(v1, v2, m.to_uint32(0));

            return ret;

        }
        else {
            vector_type ret;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                vector<T, native_elems> tmp = ::sel(v1.template extract<native_elems>(idx),
                                                    v2.template extract<native_elems>(idx),
                                                    m.template extract<native_elems>(idx).to_uint32());
                ret.insert(idx, tmp);
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
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            vector_type ret;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, ::sel(vals,
                                                             v.template extract<native_elems>(idx),
                                                             m.template extract<native_elems>(idx).to_uint32()));
            });

            return ret;
        }
        else {
            return run(broadcast<T, Elems>::run(a), v, m);
        }
    }

     __aie_inline
    static vector_type run(const vector_type &v, const T &a, const mask<Elems> &m)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            vector_type ret;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, ::sel(v.template extract<native_elems>(idx),
                                                             vals,
                                                             m.template extract<native_elems>(idx).to_uint32()));
            });

            return ret;
        }
        else {
            return run(v, broadcast<T, Elems>::run(a), m);
        }
    }

     __aie_inline
    static vector_type run(T a, T b, const mask<Elems> &m)
    {
        return run(broadcast<T, Elems>::run(a), broadcast<T, Elems>::run(b), m);
    }
};

template <typename T, unsigned Elems> struct select_bits_impl<16, T, Elems> : public select_bits_common<T, Elems> {};
template <typename T, unsigned Elems> struct select_bits_impl<32, T, Elems> : public select_bits_common<T, Elems> {};
template <typename T, unsigned Elems> struct select_bits_impl<64, T, Elems> : public select_bits_common<T, Elems> {};

}

#endif
