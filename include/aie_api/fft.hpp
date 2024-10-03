// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_FFT__HPP__
#define __AIE_API_FFT__HPP__

#include "concepts.hpp"

namespace aie {

/**
 * @ingroup group_fft
 *
 * Type that encapsulates the functionality for decimation-in-time FFTs.
 *
 * \deprecated
 * The iterator interface is deprecated and the stage-based interface should be preferred.
 * For example, where a user would previously need to define an FFT stage as follows:
 * @code
 * template <unsigned Vectorization>
 * void radix2_dit(const cint32 * __restrict x,
 *                 const cint16 * __restrict tw,
 *                 unsigned n,  unsigned shift_tw, unsigned shift, bool inv,
 *                 cint32 * __restrict y)
 * {
 *     using FFT = aie::fft_dit<Vectorization, 2, cint32>;
 *
 *     FFT fft;
 *
 *     auto it_stage  = fft.begin_stage(x, tw);
 *     auto it_out0 = aie::begin_vector<FFT::out_vector_size>(y);
 *     auto it_out1 = aie::begin_vector<FFT::out_vector_size>(y + n / 2);
 *
 *     for (int j = 0; j < n / (2 * FFT::out_vector_size); ++j)
 *         chess_prepare_for_pipelining
 *         chess_loop_range(1,)
 *     {
 *         const auto out = fft.dit(*it_stage++, shift_tw, shift, inv);
 *         *it_out0++ = out[0];
 *         *it_out1++ = out[1];
 *     }
 * }
 * @endcode
 * the user may now replace all calls to the user-defined function function with the equivalent defined by the API:
 * @code
 * aie::fft_dit_r2_stage<Vectorization>(x, tw, n, shift_tw, shift, inv, y);
 * @endcode
 *
 * @tparam Vectorization  Vectorization of the FFT stage
 * @tparam Radix Number which selects the FFT radix.
 * @tparam Input   Type of the input elements.
 * @tparam Output  Type of the output elements, defaults to input type.
 * @tparam Twiddle Type of the twiddle elements, defaults to cint16 for integral types and cfloat for floating point.
 *
 * @sa fft_dit_r2_stage, fft_dit_r3_stage, fft_dit_r4_stage, fft_dit_r5_stage
 */
template <unsigned Vectorization, unsigned Radix, typename Input, typename Output = Input, typename Twiddle = detail::default_twiddle_type_t<Input, Output>>
    requires(detail::is_valid_fft_op_v<Radix, Input, Output, Twiddle>)
using fft_dit [[deprecated("Use fft_r*_dit_stage function instead")]] =
        detail::fft_dit<Vectorization, detail::fft_get_stage<Input, Output, Twiddle>(Radix, Vectorization), Radix, Input, Output, Twiddle>;

/**
 * @ingroup group_fft
 *
 * A function to perform a single radix 2 FFT stage
 *
 * @param x        Input data pointer
 * @param tw       Twiddle group pointer
 * @param n        Number of samples
 * @param shift_tw Indicates the decimal point of the twiddles (unused for float types)
 * @param shift    Shift applied to apply to dit outputs (unused for float types)
 * @param inv      Run inverse FFT stage
 * @param out      Output data pointer
 *
 * @tparam Vectorization Vectorization of the FFT stage
 * @tparam Input   Type of the input elements.
 * @tparam Output  Type of the output elements, defaults to input type.
 * @tparam Twiddle Type of the twiddle elements, defaults to cint16 for integral types and cfloat for floating point.
 */
template <unsigned Vectorization, typename Input, typename Output, typename Twiddle>
__aie_fft_inline
void fft_dit_r2_stage(const Input * __restrict x,
                      const Twiddle * __restrict tw,
                      unsigned n, unsigned shift_tw, unsigned shift, bool inv, Output * __restrict out)
{
    constexpr unsigned Radix = 2;
    static_assert(detail::is_valid_fft_op_v<Radix, Input, Output, Twiddle>, "Requested FFT mode is not implemented");

    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(x),   "Insufficient input alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw),  "Insufficient twiddle alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(out), "Insufficient output alignment");

    detail::fft_dit_stage<Radix, Vectorization, Input, Output, Twiddle>::run(x, tw, n, shift_tw, shift, inv, out);
}

/**
 * @ingroup group_fft
 *
 * A function to perform a single radix 3 FFT stage
 *
 * Defining the rotation rate of a given twiddle to be `w(tw)`, the relationship between the twiddle groups are
 * @code
 * w(tw0) < w(tw1)
 * @endcode
 *
 * @param x        Input data pointer
 * @param tw0      First twiddle group pointer
 * @param tw1      Second twiddle group pointer
 * @param n        Number of samples
 * @param shift_tw Indicates the decimal point of the twiddles (unused for float types)
 * @param shift    Shift applied to apply to dit outputs (unused for float types)
 * @param inv      Run inverse FFT stage
 * @param out      Output data pointer
 *
 * @tparam Vectorization Vectorization of the FFT stage
 * @tparam Input   Type of the input elements.
 * @tparam Output  Type of the output elements, defaults to input type.
 * @tparam Twiddle Type of the twiddle elements, defaults to cint16 for integral types and cfloat for floating point.
 */
template <unsigned Vectorization, typename Input, typename Output, typename Twiddle>
__aie_fft_inline
void fft_dit_r3_stage(const Input * __restrict x,
                      const Twiddle * __restrict tw0,
                      const Twiddle * __restrict tw1,
                      unsigned n, unsigned shift_tw, unsigned shift, bool inv, Output * __restrict out)
{
    constexpr unsigned Radix = 3;
    static_assert(detail::is_valid_fft_op_v<Radix, Input, Output, Twiddle>, "Requested FFT mode is not implemented");

    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(x),   "Insufficient input alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw0), "Insufficient twiddle 0 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw1), "Insufficient twiddle 1 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(out), "Insufficient output alignment");

    detail::fft_dit_stage<Radix, Vectorization, Input, Output, Twiddle>::run(x, tw0, tw1, n, shift_tw, shift, inv, out);
}

/**
 * @ingroup group_fft
 *
 * A function to perform a single radix 4 FFT stage
 *
 * Defining the rotation rate of a given twiddle to be `w(tw)`, the relationship between the twiddle groups are
 * @code
 * w(tw1) < w(tw0) < w(tw2)
 * @endcode
 *
 * @param x        Input data pointer
 * @param tw0      First twiddle group pointer
 * @param tw1      Second twiddle group pointer
 * @param tw2      Third twiddle group pointer
 * @param n        Number of samples
 * @param shift_tw Indicates the decimal point of the twiddles (unused for float types)
 * @param shift    Shift applied to apply to dit outputs (unused for float types)
 * @param inv      Run inverse FFT stage
 * @param out      Output data pointer
 *
 * @tparam Vectorization Vectorization of the FFT stage
 * @tparam Input   Type of the input elements.
 * @tparam Output  Type of the output elements, defaults to input type.
 * @tparam Twiddle Type of the twiddle elements, defaults to cint16 for integral types and cfloat for floating point.
 */
template <unsigned Vectorization, typename Input, typename Output, typename Twiddle>
__aie_fft_inline
void fft_dit_r4_stage(const Input * __restrict x,
                      const Twiddle * __restrict tw0,
                      const Twiddle * __restrict tw1,
                      const Twiddle * __restrict tw2,
                      unsigned n, unsigned shift_tw, unsigned shift, bool inv, Output * __restrict out)
{
    constexpr unsigned Radix = 4;
    static_assert(detail::is_valid_fft_op_v<Radix, Input, Output, Twiddle>, "Requested FFT mode is not implemented");

    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(x),   "Insufficient input alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw0), "Insufficient twiddle 0 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw1), "Insufficient twiddle 1 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw2), "Insufficient twiddle 2 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(out), "Insufficient output alignment");

    detail::fft_dit_stage<Radix, Vectorization, Input, Output, Twiddle>::run(x, tw0, tw1, tw2, n, shift_tw, shift, inv, out);
}

/**
 * @ingroup group_fft
 *
 * A function to perform a single radix 5 FFT stage
 *
 * Defining the rotation rate of a given twiddle to be `w(tw)`, the relationship between the twiddle groups are
 * @code
 * w(tw0) < w(tw1) < w(tw2) < w(tw3)
 * @endcode
 *
 * @param x        Input data pointer
 * @param tw0      First twiddle group pointer
 * @param tw1      Second twiddle group pointer
 * @param tw2      Third twiddle group pointer
 * @param tw3      Fourth twiddle group pointer
 * @param n        Number of samples
 * @param shift_tw Indicates the decimal point of the twiddles (unused for float types)
 * @param shift    Shift applied to apply to dit outputs (unused for float types)
 * @param inv      Run inverse FFT stage
 * @param out      Output data pointer
 *
 * @tparam Vectorization Vectorization of the FFT stage
 * @tparam Input   Type of the input elements.
 * @tparam Output  Type of the output elements, defaults to input type.
 * @tparam Twiddle Type of the twiddle elements, defaults to cint16 for integral types and cfloat for floating point.
 */
template <unsigned Vectorization, typename Input, typename Output, typename Twiddle>
__aie_fft_inline
void fft_dit_r5_stage(const Input * __restrict x,
                      const Twiddle * __restrict tw0,
                      const Twiddle * __restrict tw1,
                      const Twiddle * __restrict tw2,
                      const Twiddle * __restrict tw3,
                      unsigned n, unsigned shift_tw, unsigned shift, bool inv, Output * out)
{
    constexpr unsigned Radix = 5;
    static_assert(detail::is_valid_fft_op_v<Radix, Input, Output, Twiddle>, "Requested FFT mode is not implemented");

    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(x),   "Insufficient input alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw0), "Insufficient twiddle 0 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw1), "Insufficient twiddle 1 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw2), "Insufficient twiddle 2 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw3), "Insufficient twiddle 3 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(out), "Insufficient output alignment");

    detail::fft_dit_stage<Radix, Vectorization, Input, Output, Twiddle>::run(x, tw0, tw1, tw2, tw3, n, shift_tw, shift, inv, out);
}

/**
 * @ingroup group_fft
 *
 * A function to perform a single floating point radix 2 FFT stage
 *
 * @param x        Input data pointer
 * @param tw       Twiddle group pointer
 * @param n        Number of samples
 * @param inv      Run inverse FFT stage
 * @param out      Output data pointer
 *
 * @tparam Vectorization Vectorization of the FFT stage
 * @tparam Input   Type of the input elements.
 * @tparam Output  Type of the output elements, defaults to input type.
 * @tparam Twiddle Type of the twiddle elements, defaults to cint16 for integral types and cfloat for floating point.
 */
template <unsigned Vectorization, typename Input, typename Output, typename Twiddle>
    requires(detail::is_floating_point_v<Input>)
__aie_fft_inline
void fft_dit_r2_stage(const Input * __restrict x,
                      const Twiddle * __restrict tw,
                      unsigned n, bool inv, Output * __restrict out)
{
    constexpr unsigned Radix = 2;
    static_assert(detail::is_valid_fft_op_v<Radix, Input, Output, Twiddle>, "Requested FFT mode is not implemented");

    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(x),   "Insufficient input alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw),  "Insufficient twiddle alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(out), "Insufficient output alignment");

    detail::fft_dit_stage<Radix, Vectorization, Input, Output, Twiddle>::run(x, tw, n, 0, 0, inv, out);
}

/**
 * @ingroup group_fft
 *
 * A function to perform a single floating point radix 3 FFT stage
 *
 * Defining the rotation rate of a given twiddle to be `w(tw)`, the relationship between the twiddle groups are
 * @code
 * w(tw0) < w(tw1)
 * @endcode
 *
 * @param x        Input data pointer
 * @param tw0      First twiddle group pointer
 * @param tw1      Second twiddle group pointer
 * @param n        Number of samples
 * @param inv      Run inverse FFT stage
 * @param out      Output data pointer
 *
 * @tparam Vectorization Vectorization of the FFT stage
 * @tparam Input   Type of the input elements.
 * @tparam Output  Type of the output elements, defaults to input type.
 * @tparam Twiddle Type of the twiddle elements, defaults to cint16 for integral types and cfloat for floating point.
 */
template <unsigned Vectorization, typename Input, typename Output, typename Twiddle>
    requires(detail::is_floating_point_v<Input>)
__aie_fft_inline
void fft_dit_r3_stage(const Input * __restrict x,
                      const Twiddle * __restrict tw0,
                      const Twiddle * __restrict tw1,
                      unsigned n, bool inv, Output * __restrict out)
{
    constexpr unsigned Radix = 3;
    static_assert(detail::is_valid_fft_op_v<Radix, Input, Output, Twiddle>, "Requested FFT mode is not implemented");

    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(x),   "Insufficient input alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw0), "Insufficient twiddle 0 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw1), "Insufficient twiddle 1 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(out), "Insufficient output alignment");

    detail::fft_dit_stage<Radix, Vectorization, Input, Output, Twiddle>::run(x, tw0, tw1, n, 0, 0, inv, out);
}

#if __AIE_API_CBF16_SUPPORT__
/**
 * @ingroup group_fft
 *
 * A function to perform a single floating point radix 4 FFT stage
 *
 * Defining the rotation rate of a given twiddle to be `w(tw)`, the relationship between the twiddle groups are
 * @code
 * w(tw1) < w(tw0) < w(tw2)
 * @endcode
 *
 * @param x        Input data pointer
 * @param tw0      First twiddle group pointer
 * @param tw1      Second twiddle group pointer
 * @param tw2      Third twiddle group pointer
 * @param n        Number of samples
 * @param inv      Run inverse FFT stage
 * @param out      Output data pointer
 *
 * @tparam Vectorization Vectorization of the FFT stage
 * @tparam Input   Type of the input elements.
 * @tparam Output  Type of the output elements, defaults to input type.
 * @tparam Twiddle Type of the twiddle elements, defaults to cint16 for integral types and cfloat for floating point.
 */
template <unsigned Vectorization, typename Input, typename Output, typename Twiddle>
    requires(arch::is(arch::AIE_ML) && std::is_same_v<Input, cbfloat16>)
__aie_fft_inline
void fft_dit_r4_stage(const Input * __restrict x,
                      const Twiddle * __restrict tw0,
                      const Twiddle * __restrict tw1,
                      const Twiddle * __restrict tw2,
                      unsigned n, bool inv, Output * __restrict out)
{
    constexpr unsigned Radix = 4;
    static_assert(detail::is_valid_fft_op_v<Radix, Input, Output, Twiddle>, "Requested FFT mode is not implemented");

    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(x),   "Insufficient input alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw0), "Insufficient twiddle 0 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw1), "Insufficient twiddle 1 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw2), "Insufficient twiddle 2 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(out), "Insufficient output alignment");

    detail::fft_dit_stage<Radix, Vectorization, Input, Output, Twiddle>::run(x, tw0, tw1, tw2, n, 0, 0, inv, out);
}
#endif

/**
 * @ingroup group_fft
 *
 * A function to perform a single floating point radix 5 FFT stage
 *
 * Defining the rotation rate of a given twiddle to be `w(tw)`, the relationship between the twiddle groups are
 * @code
 * w(tw0) < w(tw1) < w(tw2) < w(tw3)
 * @endcode
 *
 * @param x        Input data pointer
 * @param tw0      First twiddle group pointer
 * @param tw1      Second twiddle group pointer
 * @param tw2      Third twiddle group pointer
 * @param tw3      Fourth twiddle group pointer
 * @param n        Number of samples
 * @param inv      Run inverse FFT stage
 * @param out      Output data pointer
 *
 * @tparam Vectorization Vectorization of the FFT stage
 * @tparam Input   Type of the input elements.
 * @tparam Output  Type of the output elements, defaults to input type.
 * @tparam Twiddle Type of the twiddle elements, defaults to cint16 for integral types and cfloat for floating point.
 */
template <unsigned Vectorization, typename Input, typename Output, typename Twiddle>
    requires(arch::is(arch::AIE) && detail::is_floating_point_v<Input>)
__aie_fft_inline
void fft_dit_r5_stage(const Input * __restrict x,
                      const Twiddle * __restrict tw0,
                      const Twiddle * __restrict tw1,
                      const Twiddle * __restrict tw2,
                      const Twiddle * __restrict tw3,
                      unsigned n, bool inv, Output * __restrict out)
{
    constexpr unsigned Radix = 5;
    static_assert(detail::is_valid_fft_op_v<Radix, Input, Output, Twiddle>, "Requested FFT mode is not implemented");

    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(x),   "Insufficient input alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw0), "Insufficient twiddle 0 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw1), "Insufficient twiddle 1 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw2), "Insufficient twiddle 2 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw3), "Insufficient twiddle 3 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(out), "Insufficient output alignment");

    detail::fft_dit_stage<Radix, Vectorization, Input, Output, Twiddle>::run(x, tw0, tw1, tw2, tw3, n, 0, 0, inv, out);
}


// Dynamic vectorization

/**
 * @ingroup group_fft
 *
 * A function to perform a single radix 2 FFT stage with dynamic vectorization.
 * This will incur additional overhead compared to the static vectorization implementation.
 *
 * @param x             Input data pointer
 * @param tw            Twiddle group pointer
 * @param n             Number of samples
 * @param vectorization Vectorization of the FFT stage
 * @param shift_tw      Indicates the decimal point of the twiddles (unused for float types)
 * @param shift         Shift applied to apply to dit outputs (unused for float types)
 * @param inv           Run inverse FFT stage
 * @param out           Output data pointer
 *
 * @tparam Input   Type of the input elements.
 * @tparam Output  Type of the output elements, defaults to input type.
 * @tparam Twiddle Type of the twiddle elements, defaults to cint16 for integral types and cfloat for floating point.
 */
template <typename Input, typename Output, typename Twiddle>
    requires(arch::is(arch::AIE))
__aie_fft_inline
void fft_dit_r2_stage(const Input * __restrict x,
                      const Twiddle * __restrict tw,
                      unsigned n, unsigned vectorization,
                      unsigned shift_tw, unsigned shift, bool inv, Output * __restrict out)
{
    constexpr unsigned Radix = 2;
    static_assert(detail::is_valid_fft_op_v<Radix, Input, Output, Twiddle>, "Requested FFT mode is not implemented");

    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(x),   "Insufficient input alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw),  "Insufficient twiddle alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(out), "Insufficient output alignment");

    detail::fft_dit_stage_dyn_vec<Radix, Input, Output, Twiddle>::run(x, tw, n, vectorization, shift_tw, shift, inv, out);
}

/**
 * @ingroup group_fft
 *
 * A function to perform a single radix 3 FFT stage with dynamic vectorization.
 * This will incur additional overhead compared to the static vectorization implementation.
 *
 * Defining the rotation rate of a given twiddle to be `w(tw)`, the relationship between the twiddle groups are
 * @code
 * w(tw0) < w(tw1)
 * @endcode
 *
 * @param x             Input data pointer
 * @param tw0           First twiddle group pointer
 * @param tw1           Second twiddle group pointer
 * @param n             Number of samples
 * @param vectorization Vectorization of the FFT stage
 * @param shift_tw      Indicates the decimal point of the twiddles (unused for float types)
 * @param shift         Shift applied to apply to dit outputs (unused for float types)
 * @param inv           Run inverse FFT stage
 * @param out           Output data pointer
 *
 * @tparam Input   Type of the input elements.
 * @tparam Output  Type of the output elements, defaults to input type.
 * @tparam Twiddle Type of the twiddle elements, defaults to cint16 for integral types and cfloat for floating point.
 */
template <typename Input, typename Output, typename Twiddle>
    requires(arch::is(arch::AIE))
__aie_fft_inline
void fft_dit_r3_stage(const Input * __restrict x,
                      const Twiddle * __restrict tw0,
                      const Twiddle * __restrict tw1,
                      unsigned n, unsigned vectorization,
                      unsigned shift_tw, unsigned shift, bool inv, Output * __restrict out)
{
    constexpr unsigned Radix = 3;
    static_assert(detail::is_valid_fft_op_v<Radix, Input, Output, Twiddle>, "Requested FFT mode is not implemented");

    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(x),   "Insufficient input alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw0), "Insufficient twiddle 0 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw1), "Insufficient twiddle 1 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(out), "Insufficient output alignment");

    detail::fft_dit_stage_dyn_vec<Radix, Input, Output, Twiddle>::run(x, tw0, tw1, n, vectorization, shift_tw, shift, inv, out);
}

/**
 * @ingroup group_fft
 *
 * A function to perform a single radix 4 FFT stage with dynamic vectorization.
 * This will incur additional overhead compared to the static vectorization implementation.
 *
 * Defining the rotation rate of a given twiddle to be `w(tw)`, the relationship between the twiddle groups are
 * @code
 * w(tw1) < w(tw0) < w(tw2)
 * @endcode
 *
 * @param x             Input data pointer
 * @param tw0           First twiddle group pointer
 * @param tw1           Second twiddle group pointer
 * @param tw2           Third twiddle group pointer
 * @param n             Number of samples
 * @param vectorization Vectorization of the FFT stage
 * @param shift_tw      Indicates the decimal point of the twiddles (unused for float types)
 * @param shift         Shift applied to apply to dit outputs (unused for float types)
 * @param inv           Run inverse FFT stage
 * @param out           Output data pointer
 *
 * @tparam Input   Type of the input elements.
 * @tparam Output  Type of the output elements, defaults to input type.
 * @tparam Twiddle Type of the twiddle elements, defaults to cint16 for integral types and cfloat for floating point.
 */
template <typename Input, typename Output, typename Twiddle>
    requires(arch::is(arch::AIE))
__aie_fft_inline
void fft_dit_r4_stage(const Input * __restrict x,
                      const Twiddle * __restrict tw0,
                      const Twiddle * __restrict tw1,
                      const Twiddle * __restrict tw2,
                      unsigned n, unsigned vectorization,
                      unsigned shift_tw, unsigned shift, bool inv, Output * __restrict out)
{
    constexpr unsigned Radix = 4;
    static_assert(detail::is_valid_fft_op_v<Radix, Input, Output, Twiddle>, "Requested FFT mode is not implemented");

    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(x),   "Insufficient input alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw0), "Insufficient twiddle 0 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw1), "Insufficient twiddle 1 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw2), "Insufficient twiddle 2 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(out), "Insufficient output alignment");

    detail::fft_dit_stage_dyn_vec<Radix, Input, Output, Twiddle>::run(x, tw0, tw1, tw2, n, vectorization, shift_tw, shift, inv, out);
}

/**
 * @ingroup group_fft
 *
 * A function to perform a single radix 5 FFT stage with dynamic vectorization.
 * This will incur additional overhead compared to the static vectorization implementation.
 *
 * Defining the rotation rate of a given twiddle to be `w(tw)`, the relationship between the twiddle groups are
 * @code
 * w(tw0) < w(tw1) < w(tw2) < w(tw3)
 * @endcode
 *
 * @param x             Input data pointer
 * @param tw0           First twiddle group pointer
 * @param tw1           Second twiddle group pointer
 * @param tw2           Third twiddle group pointer
 * @param tw3           Fourth twiddle group pointer
 * @param n             Number of samples
 * @param vectorization Vectorization of the FFT stage
 * @param shift_tw      Indicates the decimal point of the twiddles (unused for float types)
 * @param shift         Shift applied to apply to dit outputs (unused for float types)
 * @param inv           Run inverse FFT stage
 * @param out           Output data pointer
 *
 * @tparam Input   Type of the input elements.
 * @tparam Output  Type of the output elements, defaults to input type.
 * @tparam Twiddle Type of the twiddle elements, defaults to cint16 for integral types and cfloat for floating point.
 */
template <typename Input, typename Output, typename Twiddle>
    requires(arch::is(arch::AIE))
__aie_fft_inline
void fft_dit_r5_stage(const Input * __restrict x,
                      const Twiddle * __restrict tw0,
                      const Twiddle * __restrict tw1,
                      const Twiddle * __restrict tw2,
                      const Twiddle * __restrict tw3,
                      unsigned n, unsigned vectorization,
                      unsigned shift_tw, unsigned shift, bool inv, Output * out)
{
    constexpr unsigned Radix = 5;
    static_assert(detail::is_valid_fft_op_v<Radix, Input, Output, Twiddle>, "Requested FFT mode is not implemented");

    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(x),   "Insufficient input alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw0), "Insufficient twiddle 0 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw1), "Insufficient twiddle 1 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw2), "Insufficient twiddle 2 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw3), "Insufficient twiddle 3 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(out), "Insufficient output alignment");

    detail::fft_dit_stage_dyn_vec<Radix, Input, Output, Twiddle>::run(x, tw0, tw1, tw2, tw3, n, vectorization, shift_tw, shift, inv, out);
}

/**
 * @ingroup group_fft
 *
 * A function to perform a single floating point radix 2 FFT stage with dynamic vectorization.
 * This will incur additional overhead compared to the static vectorization implementation.
 *
 * @param x             Input data pointer
 * @param tw            Twiddle group pointer
 * @param n             Number of samples
 * @param vectorization Vectorization of the FFT stage
 * @param inv           Run inverse FFT stage
 * @param out           Output data pointer
 *
 * @tparam Input   Type of the input elements.
 * @tparam Output  Type of the output elements, defaults to input type.
 * @tparam Twiddle Type of the twiddle elements, defaults to cint16 for integral types and cfloat for floating point.
 */
template <typename Input, typename Output, typename Twiddle>
    requires(arch::is(arch::AIE) && detail::is_floating_point_v<Input>)
__aie_fft_inline
void fft_dit_r2_stage(const Input * __restrict x,
                      const Twiddle * __restrict tw,
                      unsigned n, unsigned vectorization,
                      bool inv, Output * __restrict out)
{
    constexpr unsigned Radix = 2;
    static_assert(detail::is_valid_fft_op_v<Radix, Input, Output, Twiddle>, "Requested FFT mode is not implemented");

    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(x),   "Insufficient input alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw),  "Insufficient twiddle alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(out), "Insufficient output alignment");

    detail::fft_dit_stage_dyn_vec<Radix, Input, Output, Twiddle>::run(x, tw, n, vectorization, 0, 0, inv, out);
}

/**
 * @ingroup group_fft
 *
 * A function to perform a single floating point radix 3 FFT stage with dynamic vectorization.
 * This will incur additional overhead compared to the static vectorization implementation.
 *
 * Defining the rotation rate of a given twiddle to be `w(tw)`, the relationship between the twiddle groups are
 * @code
 * w(tw0) < w(tw1)
 * @endcode
 *
 * @param x             Input data pointer
 * @param tw0           First twiddle group pointer
 * @param tw1           Second twiddle group pointer
 * @param n             Number of samples
 * @param vectorization Vectorization of the FFT stage
 * @param inv           Run inverse FFT stage
 * @param out           Output data pointer
 *
 * @tparam Input   Type of the input elements.
 * @tparam Output  Type of the output elements, defaults to input type.
 * @tparam Twiddle Type of the twiddle elements, defaults to cint16 for integral types and cfloat for floating point.
 */
template <typename Input, typename Output, typename Twiddle>
    requires(arch::is(arch::AIE) && detail::is_floating_point_v<Input>)
__aie_fft_inline
void fft_dit_r3_stage(const Input * __restrict x,
                      const Twiddle * __restrict tw0,
                      const Twiddle * __restrict tw1,
                      unsigned n, unsigned vectorization,
                      bool inv, Output * __restrict out)
{
    constexpr unsigned Radix = 3;
    static_assert(detail::is_valid_fft_op_v<Radix, Input, Output, Twiddle>, "Requested FFT mode is not implemented");

    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(x),   "Insufficient input alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw0), "Insufficient twiddle 0 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw1), "Insufficient twiddle 1 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(out), "Insufficient output alignment");

    detail::fft_dit_stage_dyn_vec<Radix, Input, Output, Twiddle>::run(x, tw0, tw1, n, vectorization, 0, 0, inv, out);
}

/**
 * @ingroup group_fft
 *
 * A function to perform a single floating point radix 5 FFT stage with dynamic vectorization.
 * This will incur additional overhead compared to the static vectorization implementation.
 *
 * Defining the rotation rate of a given twiddle to be `w(tw)`, the relationship between the twiddle groups are
 * @code
 * w(tw0) < w(tw1) < w(tw2) < w(tw3)
 * @endcode
 *
 * @param x             Input data pointer
 * @param tw0           First twiddle group pointer
 * @param tw1           Second twiddle group pointer
 * @param tw2           Third twiddle group pointer
 * @param tw3           Fourth twiddle group pointer
 * @param n             Number of samples
 * @param vectorization Vectorization of the FFT stage
 * @param inv           Run inverse FFT stage
 * @param out           Output data pointer
 *
 * @tparam Input   Type of the input elements.
 * @tparam Output  Type of the output elements, defaults to input type.
 * @tparam Twiddle Type of the twiddle elements, defaults to cint16 for integral types and cfloat for floating point.
 */
template <typename Input, typename Output, typename Twiddle>
    requires(arch::is(arch::AIE) && detail::is_floating_point_v<Input>)
__aie_fft_inline
void fft_dit_r5_stage(const Input * __restrict x,
                      const Twiddle * __restrict tw0,
                      const Twiddle * __restrict tw1,
                      const Twiddle * __restrict tw2,
                      const Twiddle * __restrict tw3,
                      unsigned n, unsigned vectorization,
                      bool inv, Output * __restrict out)
{
    constexpr unsigned Radix = 5;
    static_assert(detail::is_valid_fft_op_v<Radix, Input, Output, Twiddle>, "Requested FFT mode is not implemented");

    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(x),   "Insufficient input alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw0), "Insufficient twiddle 0 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw1), "Insufficient twiddle 1 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw2), "Insufficient twiddle 2 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(tw3), "Insufficient twiddle 3 alignment");
    RUNTIME_ASSERT_NO_ASSUME(detail::check_vector_alignment(out), "Insufficient output alignment");

    detail::fft_dit_stage_dyn_vec<Radix, Input, Output, Twiddle>::run(x, tw0, tw1, tw2, tw3, n, vectorization, 0, 0, inv, out);
}

} // namespace aie

#endif
