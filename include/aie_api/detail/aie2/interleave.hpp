// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_INTERLEAVE__HPP__
#define __AIE_API_DETAIL_AIE2_INTERLEAVE__HPP__

#include <algorithm>

#include "shuffle_mode.hpp"
#include "../vector.hpp"
#include "../utils.hpp"
#include "../vector_accum_cast.hpp"

namespace aie::detail {

template <unsigned TypeBits, typename T, unsigned Elems>
struct interleave_bits_zip_impl
{
    static constexpr unsigned native_elems = 512 / TypeBits;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);
    using vector_type                      = vector<T, Elems>;
    using native_interleave                = interleave_bits_zip_impl<TypeBits, T, native_elems>;

    __aie_inline
    static std::pair<vector_type, vector_type> run(const vector_type &v, const vector_type &w, unsigned step)
    {
        const shuffle_mode<TypeBits, Elems> mode{step, shuffle_zip_tag{}};
        return run(v, w, mode);
    }

    __aie_inline
    static std::pair<vector_type, vector_type> run(const vector_type &v, const vector_type &w,
                                                   const shuffle_mode<TypeBits, Elems> &mode)
    {
        // Comparing shuffle_mode does not work beyond 512bit vectors
        if (chess_manifest(mode.is_bypass())) {
            return {v, w};
        }

        vector_type ret1, ret2;

        if constexpr (vector_type::bits() < 512) {
            vector<T, native_elems> tmp;

            const shuffle_mode<TypeBits, native_elems> native_mode{mode};
            std::tie(tmp, std::ignore) = native_interleave::run(v.template grow<native_elems>(),
                                                                w.template grow<native_elems>(),
                                                                native_mode);

            ret1 = tmp.template extract<Elems>(0);
            ret2 = tmp.template extract<Elems>(1);
        }
        else if constexpr (vector_type::bits() == 512) {
            ret1 = ::shuffle(v, w, mode.low());
            ret2 = ::shuffle(v, w, mode.high());
        }
        else if constexpr (vector_type::bits() == 1024) {
            if (mode.is_bypass()) {
                return {v, w};
            }

            const shuffle_mode<TypeBits, native_elems> native_mode{mode};

            auto [tmp1, tmp2] = native_interleave::run(v.template extract<Elems / 2>(0),
                                                       w.template extract<Elems / 2>(0),
                                                       native_mode);
            auto [tmp3, tmp4] = native_interleave::run(v.template extract<Elems / 2>(1),
                                                       w.template extract<Elems / 2>(1),
                                                       native_mode);

            ret1 = concat(tmp1, tmp2);
            ret2 = concat(tmp3, tmp4);
        }
        else if constexpr (vector_type::bits() > 1024) {
            if (mode.is_bypass()) {
                return {v, w};
            }

            // handle cases where compound vectors need to be moved
            if (!chess_manifest(mode.step * TypeBits < 512)) {
                if (mode.step * TypeBits >= 1024) {
                    unsigned native_blocks = mode.step / native_elems;

                    unsigned in1_idx = 0, in2_idx = 0;
                    unsigned out_idx = 0;
                    for (unsigned j = 0; j < Elems / mode.step / 2; ++j) {
                        for (unsigned block = 0; block < native_blocks; ++block)
                            ret1.insert(out_idx++, v.template extract<native_elems>(in1_idx++));
                        for (unsigned block = 0; block < native_blocks; ++block)
                            ret1.insert(out_idx++, w.template extract<native_elems>(in2_idx++));
                    }
                    out_idx = 0;
                    for (unsigned j = 0; j < Elems / mode.step / 2; ++j) {
                        for (unsigned block = 0; block < native_blocks; ++block)
                            ret2.insert(out_idx++, v.template extract<native_elems>(in1_idx++));
                        for (unsigned block = 0; block < native_blocks; ++block)
                            ret2.insert(out_idx++, w.template extract<native_elems>(in2_idx++));
                    }
                    return {ret1, ret2};
                }
            }
            
            const shuffle_mode<TypeBits, native_elems> native_mode{mode};

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                auto [tmp1, tmp2] = native_interleave::run(v.template extract<native_elems>(idx),
                                                           w.template extract<native_elems>(idx),
                                                           native_mode);
                if (idx < num_ops / 2)
                    ret1.insert(idx, concat(tmp1, tmp2));
                else
                    ret2.insert(idx - num_ops/2, concat(tmp1, tmp2));
            });
        }

        return {ret1, ret2};
    }
};

template <unsigned AccumBits, typename AccumTag, unsigned Elems>
struct interleave_accum_bits_zip_impl
{
    static constexpr unsigned native_elems = 512 / AccumBits;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);
    using accum_type                       = accum<AccumTag, Elems>;
    using vector_type                      = vector<int32, accum_type::bits() / type_bits_v<int32>>;
    using to_vector_cast                   = accum_to_vector_cast<int32, AccumTag, Elems>; 
    using native_interleave                = interleave_accum_bits_zip_impl<AccumBits, AccumTag, native_elems>;

    __aie_inline
    static std::pair<accum_type, accum_type> run(const accum_type &v, const accum_type &w, unsigned step)
    {
        const shuffle_mode<AccumBits, Elems> mode{step, shuffle_zip_tag{}};
        return run(v, w, mode);
    }

    __aie_inline
    static std::pair<accum_type, accum_type> run(const accum_type &v, const accum_type &w,
                                                 const shuffle_mode<AccumBits, Elems> &mode)
    {
        accum_type ret1, ret2;

        if constexpr (vector_type::bits() < 512) {
            using accum_t = typename native_interleave::accum_type;

            const shuffle_mode<AccumBits, native_elems> native_mode{mode};

            accum_t tmp;
            std::tie(tmp, std::ignore) = native_interleave::run(v.template grow<native_elems>(),
                                                                w.template grow<native_elems>(),
                                                                native_mode);

            ret1 = tmp.template extract<Elems>(0);
            ret2 = tmp.template extract<Elems>(1);
        }
        else if constexpr (vector_type::bits() == 512) {
            using storage_t = typename accum_type::storage_t;
            using to_vector = to_vector_cast;
            ret1 = (storage_t)::shuffle(to_vector::run(v), to_vector::run(w), mode.low());
            ret2 = (storage_t)::shuffle(to_vector::run(v), to_vector::run(w), mode.high());
        }
        else if constexpr (vector_type::bits() == 1024) {
            if (mode.is_bypass()) {
                return {v, w};
            }
            auto v0 = v.template extract<native_elems>(0),
                 v1 = v.template extract<native_elems>(1);
            auto w0 = w.template extract<native_elems>(0),
                 w1 = w.template extract<native_elems>(1);

            auto [tmp1, tmp2] = native_interleave::run(v0, w0, mode);
            auto [tmp3, tmp4] = native_interleave::run(v1, w1, mode);
#if __AIE_ARCH__ == 20
            ret1 = concat(tmp1, tmp2);
            ret2 = concat(tmp3, tmp4);
#else
            ret1 = tmp1.template grow<Elems>();
            ret1.insert(1, tmp2);

            ret2 = tmp3.template grow<Elems>();
            ret2.insert(1, tmp4);
#endif
        }
        else if constexpr (vector_type::bits() > 1024) {
            if (mode.is_bypass()) {
                return {v, w};
            }

            // handle cases where compound vectors need to be moved
            if (!chess_manifest(mode.step * AccumBits < 512)) {
                if (mode.step * AccumBits >= 1024) {
                    unsigned native_blocks = mode.step / native_elems;

                    unsigned in1_idx = 0, in2_idx = 0;
                    unsigned out_idx = 0;
                    for (unsigned j = 0; j < Elems / mode.step / 2; ++j) {
                        for (unsigned block = 0; block < native_blocks; ++block)
                            ret1.insert(out_idx++, v.template extract<native_elems>(in1_idx++));
                        for (unsigned block = 0; block < native_blocks; ++block)
                            ret1.insert(out_idx++, w.template extract<native_elems>(in2_idx++));
                    }
                    out_idx = 0;
                    for (unsigned j = 0; j < Elems / mode.step / 2; ++j) {
                        for (unsigned block = 0; block < native_blocks; ++block)
                            ret2.insert(out_idx++, v.template extract<native_elems>(in1_idx++));
                        for (unsigned block = 0; block < native_blocks; ++block)
                            ret2.insert(out_idx++, w.template extract<native_elems>(in2_idx++));
                    }
                    return {ret1, ret2};
                }
            }
            
            const shuffle_mode<AccumBits, native_elems> native_mode{mode};

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                auto [tmp1, tmp2] = native_interleave::run(v.template extract<native_elems>(idx),
                                                           w.template extract<native_elems>(idx),
                                                           native_mode);
                unsigned insert_idx = idx % (num_ops / 2);
                auto &r = idx < num_ops / 2? ret1 : ret2;
#if __AIE_ARCH__ == 20
                r.insert(insert_idx, concat(tmp1, tmp2));
#else
                r.insert(2 * insert_idx + 0, tmp1);
                r.insert(2 * insert_idx + 1, tmp2);
#endif
            });
        }

        return {ret1, ret2};
    }
};

template <typename T, unsigned Elems> struct interleave_bits_zip< 4, T, Elems> : public interleave_bits_zip_impl< 4, T, Elems> {};
template <typename T, unsigned Elems> struct interleave_bits_zip< 8, T, Elems> : public interleave_bits_zip_impl< 8, T, Elems> {};
template <typename T, unsigned Elems> struct interleave_bits_zip<16, T, Elems> : public interleave_bits_zip_impl<16, T, Elems> {};
template <typename T, unsigned Elems> struct interleave_bits_zip<32, T, Elems> : public interleave_bits_zip_impl<32, T, Elems> {};
template <typename T, unsigned Elems> struct interleave_bits_zip<64, T, Elems> : public interleave_bits_zip_impl<64, T, Elems> {};
template <unsigned Elems>             struct interleave_bits_zip<32, acc32, Elems> : public interleave_accum_bits_zip_impl<32, acc32, Elems> {};
template <unsigned Elems>             struct interleave_bits_zip<64, acc64, Elems> : public interleave_accum_bits_zip_impl<64, acc64, Elems> {};


template <unsigned TypeBits, typename T, unsigned Elems>
struct interleave_bits_unzip_impl
{
    static constexpr unsigned native_elems = 512 / TypeBits;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);
    using vector_type                      = vector<T, Elems>;
    using native_interleave                = interleave_bits_unzip_impl<TypeBits, T, native_elems>;

    __aie_inline
    static std::pair<vector_type, vector_type> run(const vector_type &v, const vector_type &w, unsigned step)
    {
        const shuffle_mode<TypeBits, Elems> mode{step, shuffle_unzip_tag{}};
        return run(v, w, mode);
    }

    __aie_inline
    static std::pair<vector_type, vector_type> run(const vector_type &v, const vector_type &w,
                                                   const shuffle_mode<TypeBits, Elems> &mode)
    {
        if (chess_manifest(mode.is_bypass())) {
            return {v, w};
        }

        vector_type ret1, ret2;

        if constexpr (vector_type::bits() < 512) {
            vector<T, native_elems> tmp;
#if __AIE_ARCH__ == 20
            tmp = concat(v, w).template grow<native_elems>();
#else
            tmp.insert(0, v);
            tmp.insert(1, w);
#endif

            const shuffle_mode<TypeBits, native_elems> native_mode{mode};
            auto [even, odd] = native_interleave::run(tmp, {}, native_mode);
            ret1 = even.template extract<Elems>(0);
            ret2 = odd .template extract<Elems>(0);
        }
        else if constexpr (vector_type::bits() == 512) {
            ret1 = ::shuffle(v, w, mode.low());
            ret2 = ::shuffle(v, w, mode.high());
        }
        else if constexpr (vector_type::bits() == 1024) {
            auto v0 = v.template extract<native_elems>(0),
                 v1 = v.template extract<native_elems>(1);
            auto w0 = w.template extract<native_elems>(0),
                 w1 = w.template extract<native_elems>(1);

            if (chess_manifest(mode.is_swap())) {
                ret1 = concat(v0, w0);
                ret2 = concat(v1, w1);
            }
            else if (mode.is_bypass()) {
                return {v, w};
            }
            else {
                const shuffle_mode<TypeBits, native_elems> native_mode{mode};
                auto [even0, odd0] = native_interleave::run(v0, v1, native_mode);
                auto [even1, odd1] = native_interleave::run(w0, w1, native_mode);

                ret1 = concat(even0, even1);
                ret2 = concat(odd0, odd1);
            }
        }
        else if constexpr (vector_type::bits() > 1024) {
            if (mode.is_bypass()) {
                return {v, w};
            }

            // handle cases where compound vectors need to be moved
            if (!chess_manifest(mode.step * TypeBits <= 512)) {
                if (mode.step * TypeBits >= 1024) {
                    unsigned native_blocks = mode.step / native_elems;

                    unsigned in_idx = 0;
                    unsigned out1_idx = 0, out2_idx = 0;
                    for (unsigned j = 0; j < Elems / mode.step / 2; ++j) {
                        for (unsigned block = 0; block < native_blocks; ++block)
                            ret1.insert(out1_idx++, v.template extract<native_elems>(in_idx++));
                        for (unsigned block = 0; block < native_blocks; ++block)
                            ret2.insert(out2_idx++, v.template extract<native_elems>(in_idx++));
                    }
                    in_idx = 0;
                    for (unsigned j = 0; j < Elems / mode.step / 2; ++j) {
                        for (unsigned block = 0; block < native_blocks; ++block)
                            ret1.insert(out1_idx++, w.template extract<native_elems>(in_idx++));
                        for (unsigned block = 0; block < native_blocks; ++block)
                            ret2.insert(out2_idx++, w.template extract<native_elems>(in_idx++));
                    }
                    return {ret1, ret2};
                }
            }
            
            const shuffle_mode<TypeBits, native_elems> native_mode{mode};

            utils::unroll_times<num_ops / 2>([&](unsigned idx) __aie_inline {
                auto [even, odd] = native_interleave::run(v.template extract<native_elems>(2 * idx + 0),
                                                          v.template extract<native_elems>(2 * idx + 1),
                                                          native_mode);
                ret1.insert(idx, even);
                ret2.insert(idx, odd);
            });
            utils::unroll_times<num_ops / 2>([&](unsigned idx) __aie_inline {
                auto [even, odd] = native_interleave::run(w.template extract<native_elems>(2 * idx + 0),
                                                          w.template extract<native_elems>(2 * idx + 1),
                                                          native_mode);
                ret1.insert(num_ops / 2 + idx, even);
                ret2.insert(num_ops / 2 + idx, odd);
            });
        }

        return {ret1, ret2};
    }

    __aie_inline
    static std::pair<vector<T, Elems/2>, vector<T, Elems/2>> run(const vector_type &v, unsigned step)
    {
        if constexpr (Elems <= native_elems) {
            const shuffle_mode<TypeBits, native_elems> mode{step, shuffle_unzip_tag{}};
            // Avoid concat overhead when input vector is smaller than native.
            // We use ::shuffle intrinsic with single argument
            // This is equivalent to setting the second argument as undefined
            // The compiler may be able to perform some optimizations when the second argument is undefined
            using vector_t  = typename native_interleave::vector_type;

            vector_t v2 = v.template grow<native_elems>();

            vector_t lo, hi;
            lo = ::shuffle(v2, mode.low());
            hi = ::shuffle(v2, mode.high());

            return {lo.template extract<Elems / 2>(0), hi.template extract<Elems / 2>(0)};
        }
        else {
            auto [lo, hi] = v.template split<Elems / 2>();
            return interleave_bits_unzip_impl<TypeBits, T, Elems / 2>::run(lo, hi, step);
        }
    }
};

template <unsigned AccumBits, typename AccumTag, unsigned Elems>
struct interleave_accum_bits_unzip_impl
{
    static constexpr unsigned native_elems = 512 / AccumBits;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);
    using accum_type                       = accum<AccumTag, Elems>;
    using native_interleave                = interleave_accum_bits_unzip_impl<AccumBits, AccumTag, native_elems>;
    using to_vector_cast                   = accum_to_vector_cast<int32, AccumTag, Elems>;

    __aie_inline
    static std::pair<accum_type, accum_type> run(const accum_type &v, const accum_type &w, unsigned step)
    {
        const shuffle_mode<AccumBits, Elems> mode{step, shuffle_unzip_tag{}};
        return run(v, w, mode);
    }

    __aie_inline
    static std::pair<accum_type, accum_type> run(const accum_type &v, const accum_type &w,
                                                 const shuffle_mode<AccumBits, Elems> &mode)
    {
        if (chess_manifest(mode.is_bypass()))
            return {v, w};

        accum_type ret1, ret2;

        if constexpr (accum_type::bits() < 512) {
            accum<AccumTag, native_elems> tmp;
#if __AIE_ARCH__ == 20
            tmp = concat(v, w).template grow<native_elems>();
#else
            tmp.insert(0, v);
            tmp.insert(1, w);
#endif

            const shuffle_mode<AccumBits, native_elems> native_mode{mode};
            auto [even, odd] = native_interleave::run(tmp, {}, native_mode);

            ret1 = even.template extract<Elems>(0);
            ret2 = odd .template extract<Elems>(0);
        }
        else if constexpr (accum_type::bits() == 512) {
            using storage_t = typename accum_type::storage_t;
            using to_vector = to_vector_cast;
            ret1 = (storage_t)::shuffle(to_vector::run(v), to_vector::run(w), mode.low());
            ret2 = (storage_t)::shuffle(to_vector::run(v), to_vector::run(w), mode.high());
        }
        else if constexpr (accum_type::bits() == 1024) {
            if (mode.is_bypass()) {
                return {v, w};
            }

            auto v0 = v.template extract<native_elems>(0),
                 v1 = v.template extract<native_elems>(1);
            auto w0 = w.template extract<native_elems>(0),
                 w1 = w.template extract<native_elems>(1);

            auto [lo1, hi1] = native_interleave::run(v0, v1, mode);
            auto [lo2, hi2] = native_interleave::run(w0, w1, mode);
#if __AIE_ARCH__ == 20
            ret1 = concat(lo1, lo2);
            ret2 = concat(hi1, hi2);
#else
            ret1.insert(0, lo1);
            ret1.insert(1, lo2);
            ret2.insert(0, hi1);
            ret2.insert(1, hi2);
#endif
        }
        else if constexpr (accum_type::bits() > 1024) {
            if (mode.is_bypass()) {
                return {v, w};
            }

            // handle cases where compound vectors need to be moved
            if (!chess_manifest(mode.step * AccumBits < 512)) {
                if (mode.step * AccumBits >= 1024) {
                    unsigned native_blocks = mode.step / native_elems;

                    unsigned in_idx = 0;
                    unsigned out1_idx = 0, out2_idx = 0;
                    for (unsigned j = 0; j < Elems / mode.step / 2; ++j) {
                        for (unsigned block = 0; block < native_blocks; ++block)
                            ret1.insert(out1_idx++, v.template extract<native_elems>(in_idx++));
                        for (unsigned block = 0; block < native_blocks; ++block)
                            ret2.insert(out2_idx++, v.template extract<native_elems>(in_idx++));
                    }
                    in_idx = 0;
                    for (unsigned j = 0; j < Elems / mode.step / 2; ++j) {
                        for (unsigned block = 0; block < native_blocks; ++block)
                            ret1.insert(out1_idx++, w.template extract<native_elems>(in_idx++));
                        for (unsigned block = 0; block < native_blocks; ++block)
                            ret2.insert(out2_idx++, w.template extract<native_elems>(in_idx++));
                    }
                    return {ret1, ret2};
                }
            }
            
            const shuffle_mode<AccumBits, native_elems> native_mode{mode};

            utils::unroll_times<num_ops / 2>([&](unsigned idx) __aie_inline {
                auto [lo, hi] = native_interleave::run(v.template extract<native_elems>(2 * idx + 0),
                                                       v.template extract<native_elems>(2 * idx + 1),
                                                       native_mode);
                ret1.insert(idx, lo);
                ret2.insert(idx, hi);
            });
            utils::unroll_times<num_ops / 2>([&](unsigned idx) __aie_inline {
                auto [lo, hi] = native_interleave::run(w.template extract<native_elems>(2 * idx + 0),
                                                       w.template extract<native_elems>(2 * idx + 1),
                                                       native_mode);
                ret1.insert(num_ops / 2 + idx, lo);
                ret2.insert(num_ops / 2 + idx, hi);
            });
        }

        return {ret1, ret2};
    }

    __aie_inline
    static std::pair<accum<AccumTag, Elems / 2>, accum<AccumTag, Elems / 2>> run(const accum_type &a, unsigned step)
    {
        const shuffle_mode<AccumBits, Elems> mode{step, shuffle_unzip_tag{}};
        if constexpr (Elems <= native_elems) {
            const shuffle_mode<AccumBits, native_elems> native_mode{mode};

            // Avoid concat overhead when input vector is smaller than native.
            // We use ::shuffle intrinsic with single argument
            // This is equivalent to setting the second argument as undefined
            // The compiler may be able to perform some optimizations when the second argument is undefined
            using accum_t   = typename native_interleave::accum_type;
            using storage_t = typename accum_t::storage_t;
            using to_vector = typename native_interleave::to_vector_cast;

            accum_t a2 = a.template grow<native_elems>();

            accum_t odd, even;
            odd  = (storage_t)::shuffle(to_vector::run(a2), native_mode.low());
            even = (storage_t)::shuffle(to_vector::run(a2), native_mode.high());

            return {odd.template extract<Elems / 2>(0), even.template extract<Elems / 2>(0)};
        }
        else {
            const shuffle_mode<AccumBits, Elems / 2> lower_mode{mode};
            auto lo = a.template extract<Elems / 2>(0);
            auto hi = a.template extract<Elems / 2>(1);
            return interleave_accum_bits_unzip_impl<AccumBits, AccumTag, Elems / 2>::run(lo, hi, lower_mode);
        }
    }
};

template <ElemBaseType T, unsigned Elems>      struct interleave_bits_unzip< 4, T, Elems> : public interleave_bits_unzip_impl< 4, T, Elems> {};
template <ElemBaseType T, unsigned Elems>      struct interleave_bits_unzip< 8, T, Elems> : public interleave_bits_unzip_impl< 8, T, Elems> {};
template <ElemBaseType T, unsigned Elems>      struct interleave_bits_unzip<16, T, Elems> : public interleave_bits_unzip_impl<16, T, Elems> {};
template <ElemBaseType T, unsigned Elems>      struct interleave_bits_unzip<32, T, Elems> : public interleave_bits_unzip_impl<32, T, Elems> {};
template <ElemBaseType T, unsigned Elems>      struct interleave_bits_unzip<64, T, Elems> : public interleave_bits_unzip_impl<64, T, Elems> {};
template <AccumElemBaseType A, unsigned Elems> struct interleave_bits_unzip<32, A, Elems> : public interleave_accum_bits_unzip_impl<32, A, Elems> {};
template <AccumElemBaseType A, unsigned Elems> struct interleave_bits_unzip<64, A, Elems> : public interleave_accum_bits_unzip_impl<64, A, Elems> {};

}

#endif
