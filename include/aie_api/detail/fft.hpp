// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_FFT_HPP__
#define __AIE_API_DETAIL_FFT_HPP__

#include "array_helpers.hpp"
#include "utils.hpp"

/**
 * @ingroup group_fft
 * @page group_fft_page_supported_modes Supported Fast Fourier Transform Modes
 *
 * <table>
 * <caption>Supported FFT/IFFT Modes</caption>
 * <tr>
 *   <th>Input Type <th>Output Type <th>Twiddle Type <th>AIE Supported Radices      <th>AIE-ML/XDNA 1 Supported Radices   <th>XDNA 2 Supported Radices
 * <tr>
 *   <td>c16b       <td>c16b        <td>c16b         <td align="center"> 2, 3, 4, 5 <td align="center"> 2, 3, 4, 5        <td align="center"> 2, 4
 * <tr>
 *   <td>c16b       <td>c32b        <td>c16b         <td align="center"> 2, 3, 4, 5 <td align="center"> 2, 3, 4, 5        <td align="center"> 2, 4
 * <tr>
 *   <td>c32b       <td>c16b        <td>c16b         <td align="center"> 2, 3, 4, 5 <td align="center"> 2, 3, 4, 5        <td align="center"> 2, 4
 * <tr>
 *   <td>c32b       <td>c32b        <td>c16b         <td align="center"> 2, 3, 4, 5 <td align="center"> 2, 3, 4, 5        <td align="center"> 2, 4
 * <tr>
 *   <td>c16b       <td>c32b        <td>c32b         <td align="center"> 2          <td align="center">                   <td align="center">
 * <tr>
 *   <td>c32b       <td>c16b        <td>c32b         <td align="center"> 2          <td align="center">                   <td align="center">
 * <tr>
 *   <td>c32b       <td>c32b        <td>c32b         <td align="center"> 2, 3, 4, 5 <td align="center">                   <td align="center">
 * <tr>
 *   <td>%cbfloat16 <td>%cbfloat16  <td>%cbfloat16   <td align="center">            <td align="center"> 2, 4              <td align="center">
 * <tr>
 *   <td>cfloat     <td>cfloat      <td>cfloat       <td align="center"> 2, 3, 5    <td align="center">                   <td align="center">
 * </table>
 *
 * \note
 * Odd-radix FFT stages are only available for vectorization values greater than or equal to the underlying output vector sizes.
 * <table>
 * <caption>Underlying output vector sizes</caption>
 * <tr>
 *   <th>Input Type <th>Output Type <th>Twiddle Type <th>AIE Output Vector Size            <th>AIE-ML/XDNA 1 Output Vector Size  <th>XDNA 2 Output Vector Size
 * <tr>
 *   <td>c16b       <td>c16b        <td>c16b         <td align="center"> 4 (8 for radix 2) <td align="center"> 8                 <td align="center"> 16
 * <tr>
 *   <td>c16b       <td>c32b        <td>c16b         <td align="center"> 4 (8 for radix 2) <td align="center"> 8                 <td align="center"> 16
 * <tr>
 *   <td>c32b       <td>c16b        <td>c16b         <td align="center"> 4                 <td align="center"> 8                 <td align="center"> 16
 * <tr>
 *   <td>c32b       <td>c32b        <td>c16b         <td align="center"> 4                 <td align="center"> 8                 <td align="center"> 16
 * <tr>
 *   <td>c16b       <td>c32b        <td>c32b         <td align="center"> 4                 <td align="center">                   <td align="center">
 * <tr>
 *   <td>c32b       <td>c16b        <td>c32b         <td align="center"> 4                 <td align="center">                   <td align="center">
 * <tr>
 *   <td>c32b       <td>c32b        <td>c32b         <td align="center"> 2                 <td align="center">                   <td align="center">
 * <tr>
 *   <td>%cbfloat16 <td>%cbfloat16  <td>%cbfloat16   <td align="center">                   <td align="center"> 8                 <td align="center">
 * <tr>
 *   <td>cfloat     <td>cfloat      <td>cfloat       <td align="center"> 4                 <td align="center">                   <td align="center">
 * </table>
 *
 * \note The minimum point size supported by an FFT is given by the product of the radix with the underlying output vector size.
 * This is due to the fact that a radix R FFT will use R output pointers, each writing an amount of data equal to the underlying output vector size.
 * For example, a radix 4 FFT with cint16 input data, cint32 output data, and cint16 twiddles will have a minimum point size of 16 (4 * 4) on AIE
 * while the minimum point size on AIE-ML/XDNA 1 will be 32 (4 * 8).
 */

namespace aie::detail {

#if __AIE_ARCH__ == 10

template <typename Input, typename Output, typename Twiddle>
static constexpr unsigned fft_get_stage(unsigned Radix, unsigned Vectorization)
{
    //TODO: check legal vectorization value? Also refactor to compute this directly from datatype bits?
    if      constexpr (std::is_same_v<Twiddle, cint16>) {
        if constexpr (std::is_same_v<Input, Output>) {
            if (Radix == 2 && std::is_same_v<Input, cint16>) {
                if      (Vectorization == 1) { return 3; }
                else if (Vectorization == 2) { return 2; }
                else if (Vectorization == 4) { return 1; }
                else                         { return 0; }
            }
            else if (Radix == 2 && std::is_same_v<Input, cint32>) {
                if      (Vectorization == 1) { return 2; }
                else if (Vectorization == 2) { return 1; }
                else                         { return 0; }
            }
            else if (Radix == 4) {
                if      (Vectorization == 1) { return 1; }
                else if (Vectorization >= 4) { return 0; }
                else                         { UNREACHABLE_MSG("Requested vectorization not supported for Radix 4\n"); }
            }
            //Only plans to support stage 0 for odd radix at this time
            else if (Radix == 3 || Radix == 5) {
                if   (Vectorization >= 4) { return 0; }
                else                      { UNREACHABLE_MSG("Only vectorization of 4 or greater is supported in Radix 3 and 5\n"); }
            }
        }
        else {
            if (Vectorization == 1) {
                if      (Radix == 2) { return 2; }
                else if (Radix == 4) { return 1; }
            }
            else { return 0; }
        }
    }
    else if constexpr (std::is_same_v<Twiddle, cint32>) {
        if constexpr (std::is_same_v<Input, Output>) {
            if (Radix == 2 && std::is_same_v<Input, cint16>) {
                if   (Vectorization == 1) { return 2; }
                else                      { UNREACHABLE_MSG("Requested vectorization not supported for Radix 2\n"); }
            }
            else if (Radix == 2 && std::is_same_v<Input, cint32>) {
                if   (Vectorization == 1) { return 1; }
                else                      { return 0; }
            }
            else if (Radix == 4 && std::is_same_v<Input, cint32>) {
                if      (Vectorization == 1) { return 1; }
                else if (Vectorization >= 4) { return 0; }
                else                         { UNREACHABLE_MSG("Requested vectorization not supported for Radix 4\n"); }
            }
            else if (Radix == 3 || Radix == 5) {
                if   (Vectorization >= 2) { return 0; }
                else                      { UNREACHABLE_MSG("Only vectorization of 2 or greater is supported in Radix 3 and 5\n"); }
            }
        }
        else {
            if (Vectorization == 1) {
                if      (Radix == 2) { return 1; }
                else if (Radix == 4) { return 1; }
            }
            else { return 0; }
        }
    }
    else if constexpr (std::is_same_v<Twiddle, cfloat>) {
        if constexpr (std::is_same_v<Input, cfloat>) {
            if (Radix == 2) {
                if      (Vectorization == 1) { return 2; }
                else if (Vectorization == 2) { return 1; }
                else                         { return 0; }
            }
            else if (Radix == 3 || Radix == 5) {
                if   (Vectorization >= 4) { return 0; }
                else                      { UNREACHABLE_MSG("Only vectorization of 4 or greater is supported in Radix 3 and 5\n"); }
            }
        }
    }

    UNREACHABLE_MSG("Unreachable\n");
}

template <typename Input, typename Output, typename Twiddle>
static constexpr unsigned fft_get_out_vector_size(unsigned Radix, unsigned Vectorization)
{
    if      constexpr (std::is_same_v<Twiddle, cint16>) {
        if   (std::is_same_v<Input, cint16> && Radix == 2 ) { return 8; } //cint16 radix2 has vectors of 8 elems unless it is a stage 0 upscaling to cint32
        else                                                { return 4; }
    }
    else if constexpr (std::is_same_v<Twiddle, cint32>) {
        if      constexpr (std::is_same_v<Input, cint16> && std::is_same_v<Output, cint16>) { return 4; }
        else if constexpr (std::is_same_v<Input, cint16> && std::is_same_v<Output, cint32>) { return 4; }
        else if constexpr (std::is_same_v<Input, cint32> && std::is_same_v<Output, cint16>) { return 4; }
        else if constexpr (std::is_same_v<Input, cint32> && std::is_same_v<Output, cint32>) { return 2; }
    }
    else if constexpr (std::is_same_v<Twiddle, cfloat>) {
        return 4;
    }
}

#elif __AIE_ARCH__ == 20

template <typename Input, typename Output, typename Twiddle>
static constexpr unsigned fft_get_stage(unsigned Radix, unsigned Vectorization)
{
    //TODO: check legal vectorization value? Also could probably refactor to an expression compute this directly from datatype bits, fft_get_out_vector_number and fft_get_out_vector_size
    if      constexpr (std::is_same_v<Twiddle, cint16>) {
        if (Radix == 2) {
            if      (Vectorization == 1) { return 3; }
            else if (Vectorization == 2) { return 2; }
            else if (Vectorization == 4) { return 1; }
            else                         { return 0; }
        }
        else if (Radix == 4) {
            if      (Vectorization == 1) { return 2; }
            else if (Vectorization == 4) { return 1; }
            else                         { return 0; }
        }
        else if (Radix == 3 || Radix == 5) {
            if   (Vectorization >= 8) { return 0; }
            else                      { UNREACHABLE_MSG("Only vectorization of 8 or greater is supported in Radix 3 and 5\n"); }
        }
    }
    else if constexpr (std::is_same_v<Twiddle, cbfloat16>) {
        if (Radix == 2) {
            if      (Vectorization == 1) { return 3; }
            else if (Vectorization == 2) { return 2; }
            else if (Vectorization == 4) { return 1; }
            else                         { return 0; }
        }
        else if (Radix == 4) {
            if      (Vectorization == 1) { return 2; }
            else if (Vectorization == 4) { return 1; }
            else                         { return 0; }
        }
    }


    UNREACHABLE_MSG("Unreachable\n");
}

template <typename Input, typename Output, typename Twiddle>
static constexpr unsigned fft_get_out_vector_size(unsigned Radix, unsigned Vectorization)
{
    return 8;
}

#else

template <typename Input, typename Output, typename Twiddle>
static constexpr unsigned fft_get_stage(unsigned Radix, unsigned Vectorization)
{
    //TODO: check legal vectorization value? Also could probably refactor to an expression compute this directly from datatype bits, fft_get_out_vector_number and fft_get_out_vector_size
    if      constexpr (std::is_same_v<Twiddle, cint16>) {
        if constexpr (std::is_same_v<Input, Output>) {
            if (Radix == 2) {
                if      (Vectorization == 1) { return 4; }
                else if (Vectorization == 2) { return 3; }
                else if (Vectorization == 4) { return 2; }
                else if (Vectorization == 8) { return 1; }
                else                         { return 0; }
            }
            else if (Radix == 4) {
                if      (Vectorization == 1) { return 2; }
                else if (Vectorization == 4) { return 1; }
                else                         { return 0; }
            }
        }
        else {
            //TODO: this only covers the up/down levels we currently implement
            if      constexpr (std::is_same_v<Input, cint16>) { return 0; }
            else if constexpr (std::is_same_v<Input, cint32>) {
                if (Radix == 2) {
                    if (Vectorization == 1) { return 4; }
                }
                else if (Radix == 4) {
                    if      (Vectorization == 1) { return 2; }
                    else if (Vectorization == 4) { return 1; }
                    else                         { return 0; }
                }
            }
        }
    }

    UNREACHABLE_MSG("Unreachable\n");
}

template <typename Input, typename Output, typename Twiddle>
static constexpr unsigned fft_get_out_vector_size(unsigned Radix, unsigned Vectorization)
{
    if   (std::is_same_v<Input, cint32> && Radix == 4 && Vectorization == 1) return 8;
    else                                                                     return 16;
}

#endif

template <typename Input, typename Output>
struct default_twiddle_type
{
    using type = cint16;
};

#if __AIE_ARCH__ == 10
template <>
struct default_twiddle_type<cfloat, cfloat>
{
    using type = cfloat;
};
#elif __AIE_API_CBF16_SUPPORT__
template <>
struct default_twiddle_type<cbfloat16, cbfloat16>
{
    using type = cbfloat16;
};
#endif

template <typename Input, typename Output = Input>
using default_twiddle_type_t = typename default_twiddle_type<Input, Output>::type;

template <unsigned Vectorization, unsigned Stage, unsigned Radix,
          typename Input, typename Output = Input, typename Twiddle = default_twiddle_type_t<Input, Output>>
struct fft_dit_common
{
    static constexpr unsigned stage = Stage;
    static constexpr unsigned radix = Radix;

    static constexpr unsigned out_vector_size = fft_get_out_vector_size<Input, Output, Twiddle>(Radix, Vectorization);
    static constexpr unsigned num_out_vector  = Radix;
    static constexpr unsigned min_point_size  = out_vector_size * num_out_vector;

    using output_data = std::array<vector<Output, out_vector_size>, num_out_vector>;

    __aie_inline
    static constexpr int block_size(unsigned n)
    {
        RUNTIME_ASSERT((n % min_point_size) == 0, "FFT point size not a multiple of min_point_size");

        unsigned size;
        if constexpr (Radix == 3) {
            size = ((n * 10923) >> 15) / out_vector_size;
        } else if constexpr (Radix == 5) {
            size = ((n * 6554) >> 15) / out_vector_size;
        } else {
            size = n / (Radix * out_vector_size);
        }
        return size;
    }
};

template <unsigned Vectorization, unsigned Stage, unsigned Radix,
          typename Input, typename Output = Input, typename Twiddle = default_twiddle_type_t<Input, Output>>
struct fft_dit;

template <unsigned Radix, typename Input, typename Output, typename Twiddle>
struct is_valid_fft_op
{
    static constexpr bool value()
    {
#if __AIE_ARCH__ == 10
        if      constexpr (std::is_same_v<Twiddle, cint16>) {
            if      constexpr (std::is_same_v<Input, cint32> && std::is_same_v<Output, cint32> && (Radix == 2 || Radix == 3 || Radix == 4 || Radix == 5))
                return true;
            else if constexpr (std::is_same_v<Input, cint16> && std::is_same_v<Output, cint32> && (Radix == 2 || Radix == 3 || Radix == 4 || Radix == 5))
                return true;
            else if constexpr (std::is_same_v<Input, cint32> && std::is_same_v<Output, cint16> && (Radix == 2 || Radix == 3 || Radix == 4 || Radix == 5))
                return true;
            else if constexpr (std::is_same_v<Input, cint16> && std::is_same_v<Output, cint16> && (Radix == 2 || Radix == 3 || Radix == 4 || Radix == 5))
                return true;
        }
        else if constexpr (std::is_same_v<Twiddle, cint32>) {
            if      constexpr (std::is_same_v<Input, cint16> && std::is_same_v<Output, cint16> && Radix == 2)
                return true;
            else if constexpr (std::is_same_v<Input, cint16> && std::is_same_v<Output, cint32> && (Radix == 2 || Radix == 4))
                return true;
            else if constexpr (std::is_same_v<Input, cint32> && std::is_same_v<Output, cint16> && (Radix == 2 || Radix == 4))
                return true;
            else if constexpr (std::is_same_v<Input, cint32> && std::is_same_v<Output, cint32> && (Radix == 2 || Radix == 3 || Radix == 4 || Radix == 5))
                return true;
        }
        else if constexpr (std::is_same_v<Twiddle, cfloat>) {
            if      constexpr (std::is_same_v<Input, cfloat> && std::is_same_v<Output, cfloat> && (Radix == 2 || Radix == 3 || Radix == 5))
                return true;
        }
#elif __AIE_ARCH__ == 20
        if      constexpr (std::is_same_v<Twiddle, cint16>) {
            if      constexpr (utils::is_one_of_v<Input, cint16, cint32> && utils::is_one_of_v<Output, cint16, cint32> && (Radix == 2 || Radix == 4))
                return true;
            if      constexpr (utils::is_one_of_v<Input, cint16, cint32> && utils::is_one_of_v<Output, cint16, cint32> && (Radix == 3 || Radix == 5))
                return true;
        }
        else if constexpr (std::is_same_v<Twiddle, cbfloat16>) {
            if      constexpr (std::is_same_v<Input, cbfloat16> && std::is_same_v<Output, cbfloat16> && (Radix == 2 || Radix == 4))
                return true;
        }
#elif __AIE_ARCH__ == 21
        if      constexpr (std::is_same_v<Twiddle, cint16>) {
            if      constexpr (utils::is_one_of_v<Input, cint16, cint32> && utils::is_one_of_v<Output, cint16, cint32> && (Radix == 2 || Radix == 4))
                return true;
        }
#endif

        return false;
    }
};

template <unsigned Radix, typename Input, typename Output, typename Twiddle>
static constexpr bool is_valid_fft_op_v = is_valid_fft_op<Radix, Input, Output, Twiddle>::value();

template <unsigned Radix, unsigned Vectorization, typename Input, typename Output, typename Twiddle>
struct fft_dit_stage;

template <unsigned Radix, typename Input, typename Output, typename Twiddle>
struct fft_dit_stage_dyn_vec;

}

#if __AIE_ARCH__ == 10

#include "aie1/fft_dit.hpp"

#elif __AIE_ARCH__ == 20

#include "aie2/fft_dit.hpp"

#elif __AIE_ARCH__ == 21

#include "aie2p/fft_dit.hpp"

#endif

#endif
