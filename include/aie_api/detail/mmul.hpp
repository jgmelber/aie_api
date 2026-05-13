// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_MMUL__HPP__
#define __AIE_API_DETAIL_MMUL__HPP__

#include "vector.hpp"
#include "accum.hpp"
#include "vector_accum_cast.hpp"
/**
 * @ingroup group_mmul
 * @page group_mmul_page_supported_modes Matrix Multiplication Modes
 *
 * The following matrix multiplication shapes are supported.
 *
 * @section group_mmul_page_supported_regular_modes Supported Matrix Multiplication Modes
 *
 * @note
 * In the following table, the following abbreviations are used to denote low precision floating point multiplications:
 * - f8 is used to denote multiplication with 8 bit low precission floating point formats
 * (%bfloat8 x %bfloat8, %bfloat8 x %float8, %float8 x %bfloat8, %float8 x %float8).
 * - f16 is used to denote multiplication with 16 bit %floating point formats
 * (%bfloat16 x %bfloat16, %bfloat16 x %float16, %float16 x %bfloat16, %float16 x %float16).
 * If a multiplication mode is restricted to a specific combination, then the unabbreviated format name is used.
 *
 * <table>
 * <caption>Matrix multiplication modes for real types</caption>
 * <tr><th>Arch.<th>8b x 4b<th>8b x 8b<th>16b x 8b<th>8b x 16b<th>16b x 16b<th>32b x 16b<th>16b x 32b<th>32b x 32b<sup>c</sup>
 * <th>%f8 x %f8
 * <th>%bfloat16 x %bfloat16
 * <th>%f16 x %f16
 * <th>float x float<sup>d</sup>
 * <th>bfp16 x bfp16 <th>%mx4 x %mx4 <th>%mx6 x %mx6 <th>%mx9 x %mx9
 * <tr><td>
 *         AIE
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *         4x8x4<br> 4x16x4<sup>a</sup><br> 8x8x4<sup>a</sup><br> 2x8x8<br> 4x8x8<sup>a</sup><br> 1x16x8<br> 2x16x8<sup>a</sup><br> 4x16x8<sup>a</sup>
 *     <td style="vertical-align:top">
 *         4x4x4<br> 8x4x4<sup>a</sup><br>  4x8x4<sup>a</sup><br> 4x4x8<sup>a</sup>
 *     <td style="vertical-align:top">
 *         4x4x8<sup>a</sup><br> 4x4x4<sup>a</sup><br> 8x8x1<sup>ab</sup>
 *     <td style="vertical-align:top">
 *         4x4x4<sup>a</sup><br> 2x4x8<sup>a</sup><br> 4x4x8<sup>a</sup><br> 4x2x8<sup>a</sup><br> 8x8x1<sup>ab</sup>
 *     <td style="vertical-align:top">
 *         2x4x8<sup>a</sup><br> 4x4x4<sup>a</sup><br> 4x2x4<sup>a</sup><br> 2x2x4<br> 2x4x4<sup>a</sup><br> 4x4x2<sup>a</sup><br> 2x2x8<sup>a</sup>
 *     <td style="vertical-align:top">
 *         4x2x2<br> 2x4x8<sup>a</sup><br> 4x4x4<sup>a</sup>
 *     <td style="vertical-align:top">
 *         4x2x4<sup>a</sup><br> 2x2x2<br> 2x4x2<sup>a</sup><br> 2x8x2<sup>a</sup><br> 4x2x2<sup>a</sup><br> 4x4x2<sup>a</sup><br> 2x4x4<sup>a</sup><br> 4x4x1<sup>a</sup>
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *         4x2x4<sup>a</sup><br> 2x2x2<sup>a</sup><br> 2x4x2<sup>ab</sup><br> 2x8x2<sup>ab</sup><br> 4x2x2<sup>a</sup><br> 4x4x2<sup>a</sup><br> 2x4x4<sup>a</sup><br> 4x4x1<sup>ab</sup>
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 * <tr><td>
 *         AIE-ML/XDNA1
 *     <td style="vertical-align:top">
 *         4x16x8<br>8x16x8<sup>a</sup><br>4x32x8<sup>ab</sup>
 *     <td style="vertical-align:top">
 *         4x8x4<sup>ab</sup><br> 4x16x4<sup>ab</sup><br> 8x8x4<sup>ab</sup><br> 2x8x8<br> 4x8x8<br> 8x8x8<sup>a</sup><br> 1x16x8<sup>ab</sup><br> 2x16x8<sup>ab</sup><br> 4x16x8<sup>ab</sup>
 *     <td style="vertical-align:top">
 *         4x4x4<sup>ab</sup><br> 8x4x4<sup>ab</sup><br>  4x8x4<br> 4x4x8 <br> 8x4x8<sup>ab</sup><br> 2x8x8
 *     <td style="vertical-align:top">
 *         4x4x8<sup>ab</sup><br> 4x4x4<sup>ab</sup>
 *     <td style="vertical-align:top">
 *         4x4x4<br> 2x4x8<br> 4x4x8<sup>ab</sup><br> 4x2x8<br> 8x2x8<sup>a</sup><br> 8x1x8<sup>ab</sup>
 *     <td style="vertical-align:top">
 *         2x4x8<br> 4x4x8<sup>ab</sup><br> 4x4x4<br> 4x2x4<br> 4x1x8<sup>ab</sup>
 *     <td style="vertical-align:top">
 *         2x4x8<br> 4x4x4
 *     <td style="vertical-align:top">
 *         4x2x4<sup>a</sup><br> 4x4x4<sup>ab</sup><br> 8x2x4<sup>a</sup><br> 4x1x8<sup>ab</sup><br> 8x1x8<sup>ab</sup>
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *         4x8x4<br> 8x8x4<sup>a</sup><br> 4x16x8<sup>ab</sup><br> 8x8x8<sup>ab</sup>
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *         4x8x4<br> 4x1x4<sup>b</sup><br> 4x1x8<sup>ab</sup>
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 * <tr><td>
 *         XDNA2
 *     <td style="vertical-align:top">
 *         4x16x16
 *     <td style="vertical-align:top">
 *         4x8x8<br> 8x8x8
 *     <td style="vertical-align:top">
 *         4x4x8<br> 8x4x8<br> 4x8x8<br> 2x8x8<sup>b</sup>
 *     <td style="vertical-align:top">
 *         8x2x8<sup>b</sup><br> 4x4x8<sup>b</sup>
 *     <td style="vertical-align:top">
 *         4x2x8<sup>32</sup><br> 8x2x8<sup>32</sup><br> 2x4x8<sup>64</sup><br> 4x4x8<sup>64</sup><br> 8x1x8<sup>b, 32</sup>
 *     <td style="vertical-align:top">
 *         4x2x8<br> 2x4x8<sup>ab</sup><br> 4x4x8<sup>ab</sup><br> 4x1x8<sup>b</sup>
 *     <td style="vertical-align:top">
 *         4x4x8<sup>ab</sup>
 *     <td style="vertical-align:top">
 *         4x2x8<sup>ab</sup><br> 4x4x4<sup>ab</sup><br> 4x4x8<sup>ab</sup><br> 8x2x8<sup>ab</sup><br> 4x1x8<sup>b</sup>
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *         8x8x4<sup>ab</sup><br> 4x8x8<sup>abc</sup><br> 4x8x4<sup>ab</sup><br> 8x8x8<sup>e</sup><br> 8x1x8<sup>b</sup>
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *         4x8x4<sup>ab</sup>
 *     <td style="vertical-align:top">
 *         8x8x8<br> 8x8x16<sup>ab</sup>
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 * <tr><td>
 *         AIE-MLv2
 *     <td style="vertical-align:top">
 *         4x16x16<sup>ab</sup><br> 8x8x8
 *     <td style="vertical-align:top">
 *         4x8x8<sup>32</sup><br> 8x8x8<sup>32</sup>
 *     <td style="vertical-align:top">
 *         8x2x8<sup>b</sup><br> 8x4x8<sup>ab</sup><br> 4x4x8<sup>b</sup> <br> 4x8x8<sup>ab</sup>
 *     <td style="vertical-align:top">
 *         8x2x8<sup>b</sup><br> 4x4x8<sup>b</sup>
 *     <td style="vertical-align:top">
 *         4x2x8<sup>c, 32</sup><br> 8x2x8<sup>32</sup><br> 2x4x8<sup>64</sup><br> 4x4x8<sup>64</sup><br> 8x1x8<sup>b, 32</sup>
 *     <td style="vertical-align:top">
 *         4x2x8<br> 2x4x8<sup>ab</sup><br> 4x4x8<sup>ab</sup><br> 4x1x8<sup>b</sup>
 *     <td style="vertical-align:top">
 *         4x4x8<sup>ab</sup>
 *     <td style="vertical-align:top">
 *         4x2x8<sup>ab</sup><br> 4x4x4<sup>ab</sup><br> 4x4x8<sup>ab</sup><br> 8x2x8<sup>ab</sup><br> 4x1x8<sup>b</sup>
 *     <td style="vertical-align:top">
 *         8x8x8
 *     <td style="vertical-align:top">
 *         4x8x4<sup>ab</sup><br> 4x8x8<br> 8x8x8<sup>a</sup><br> 8x1x8<sup>b</sup>
 *     <td style="vertical-align:top">
 *         4x8x4<sup>ab</sup><br> 4x8x8<br> 8x8x8<sup>a</sup><br> 8x1x8<sup>b</sup>
 *     <td style="vertical-align:top">
 *         4x8x4<sup>ab</sup>
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *         4x16x16<br> 8x16x16<sup>a</sup>
 *     <td style="vertical-align:top">
 *         4x16x16<br> 8x16x16<sup>a</sup>
 *     <td style="vertical-align:top">
 *         4x16x16<sup>b</sup>
 * </table>
 *
 * <table>
 * <caption>Matrix multiplication modes for complex types (c16b/c32b/cfloat represent complex types)</caption>
 * <tr><th>Arch.<th>16b x c16b<th>16b x c32b<th>c16b x 16b<th>c16b x c16b<th>c16b x 32b
 *     <th>c16b x c32b<th>32b x c16b<th>32b x c32b<sup>c</sup><th>c32b x 16b<th>c32b x c16b
 *     <th>c32b x 32b<sup>c</sup><th>c32b x c32b<sup>c</sup>
 *     <th>%bfloat16 x %cbfloat16<th>%cbfloat16 x %bfloat16<th>%cbfloat16 x %cbfloat16
 *     <th>float x cfloat<sup>d</sup><th>cfloat x float<sup>d</sup><th>cfloat x cfloat<sup>d</sup>
 * <tr><td>
 *         AIE
 *     <td style="vertical-align:top">
 *         4x2x2<br> 4x4x4<sup>a</sup><br> 4x4x1
 *     <td style="vertical-align:top">
 *         2x4x2<sup>a</sup><br> 2x4x4<sup>a</sup><br> 2x8x2<sup>a</sup><br> 4x4x2<sup>a</sup><br> 4x4x1<sup>a</sup>
 *     <td style="vertical-align:top">
 *         2x2x4<br> 2x2x8<sup>a</sup><br> 2x4x4<sup>a</sup><br> 2x4x8<sup>a</sup><br> 4x2x4<sup>a</sup><br> 4x4x2<sup>a</sup><br> 4x4x4<sup>a</sup>
 *     <td style="vertical-align:top">
 *         2x2x2<br> 2x4x2<sup>a</sup><br> 2x8x2<sup>a</sup><br> 2x4x4<sup>a</sup><br> 4x2x2<sup>a</sup><br> 4x4x2<sup>a</sup><br> 4x2x4<sup>a</sup><br> 4x4x1<sup>a</sup>
 *     <td style="vertical-align:top">
 *         2x2x2<br> 2x4x2<sup>a</sup><br> 2x8x2<sup>a</sup><br> 2x4x4<sup>a</sup><br> 4x2x2<sup>a</sup><br> 4x4x2<sup>a</sup><br> 4x2x4<sup>a</sup><br> 4x4x1<sup>a</sup>
 *     <td style="vertical-align:top">
 *         2x2x2<sup>a</sup><br> 2x4x2<sup>a</sup><br> 4x2x1<sup>a</sup>
 *     <td style="vertical-align:top">
 *         2x2x2<br> 2x4x2<sup>a</sup><br> 2x8x2<sup>a</sup><br> 2x4x4<sup>a</sup><br> 4x2x2<sup>a</sup><br> 4x4x2<sup>a</sup><br> 4x2x4<sup>a</sup><br> 4x4x1<sup>a</sup>
 *     <td style="vertical-align:top">
 *         2x2x2<sup>a</sup><br> 2x4x2<sup>a</sup><br> 4x2x1<sup>a</sup>
 *     <td style="vertical-align:top">
 *         2x4x2<sup>a</sup><br> 2x8x2<sup>a</sup><br> 2x4x4<sup>a</sup><br> 4x4x2<sup>a</sup>
 *     <td style="vertical-align:top">
 *         2x2x2<sup>a</sup><br> 2x4x2<sup>a</sup><br> 4x4x1<sup>a</sup>
 *     <td style="vertical-align:top">
 *         1x2x2<br> 2x2x2<sup>a</sup><br> 2x4x2<sup>a</sup><br> 4x4x1<sup>a</sup>
 *     <td style="vertical-align:top">
 *         1x2x2<sup>a</sup><br> 2x2x1<sup>a</sup><br> 2x2x1
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *         2x2x2<sup>a</sup><br> 2x4x2<sup>a</sup><br> 4x2x1<sup>a</sup>
 *     <td style="vertical-align:top">
 *         2x2x2<sup>a</sup><br> 2x4x2<sup>a</sup><br> 4x4x1<sup>a</sup><br> 2x4x1<sup>ab</sup>
 *     <td style="vertical-align:top">
 *         2x2x2<sup>a</sup><br> 2x2x4<sup>a</sup><br> 2x4x2<sup>a</sup><br> 4x2x2<sup>a</sup><br> 4x2x1<sup>a</sup>
 * <tr><td>
 *         AIE-ML/XDNA1
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *         2x4x8<sup>ab</sup><br> 4x4x4<sup>ab</sup>
 *     <td style="vertical-align:top">
 *         1x4x8<sup>ab</sup><br> 2x4x8<sup>ab</sup>
 *     <td style="vertical-align:top">
 *         4x4x4<sup>ab</sup>
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *         4x2x4<sup>ab</sup><br> 4x4x4<sup>ab</sup>
 *     <td style="vertical-align:top">
 *         1x2x4<sup>ab</sup><br> 1x2x8<sup>ab</sup><br> 2x2x8<sup>ab</sup><br> 1x4x8<sup>ab</sup><br> 2x4x8<sup>ab</sup>
 *     <td style="vertical-align:top">
 *         4x2x4<sup>ab</sup>
 *     <td style="vertical-align:top">
 *         1x2x8<sup>ab</sup>
 *     <td style="vertical-align:top">
 *         2x8x2<sup>ab</sup>
 *     <td style="vertical-align:top">
 *         2x8x2<sup>ab</sup>
 *     <td style="vertical-align:top">
 *         2x8x2<sup>ab</sup>
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 * <tr><td>
 *         XDNA2
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *         4x4x8<sup>ab</sup><br> 2x4x8<sup>ab</sup>
 *     <td style="vertical-align:top">
 *         1x4x8<sup>ab</sup><br> 2x2x16<sup>ab</sup>
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *         1x2x4<sup>ab</sup><br> 1x2x8<sup>ab</sup><br> 1x2x16<sup>ab</sup>
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *         1x2x8<sup>ab</sup>
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 * <tr><td>
 *         AIE-MLv2
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *         4x4x8<sup>abd</sup><br> 2x4x8<sup>abd</sup>
 *     <td style="vertical-align:top">
 *         1x4x8<sup>ab</sup><br> 2x2x16<sup>ab</sup>
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *         1x2x4<sup>ab</sup><br> 1x2x8<sup>ab</sup><br> 1x2x16<sup>ab</sup>
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *         1x2x8<sup>ab</sup>
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 * </table>
 *
 * \note
 * <dl class="footnote">
 * <dt>a</dt><dd>Emulated using multiple intrinsic calls.</dd>
 * <dt>b</dt><dd>Require additional data manipulation.</dd>
 * <dt>c</dt><dd>32b * 16b multiplications are emulated on AIE-ML/XDNA1, XDNA2, and AIE-MLv2.</dd>
 * <dt>d</dt><dd>float multiplications are emulated on AIE-ML/XDNA1, XDNA2, and AIE-MLv2 using native %bfloat16 multiplications.</dd>
 * <dt>e</dt><dd>Mode available through block-floating-point emulation to increase throughput at the cost of accuracy. Enabled by defining AIE_API_EMULATE_BFLOAT16_MMUL_WITH_BFP16 at compile time.</dd>
 * <dt>32</dt><dd>Supported for %acc32 accumulators only.</dd>
 * <dt>64</dt><dd>Supported for %acc64 accumulators only.</dd>
 * </dl>
 *
 * @section group_mmul_page_multidim_gemm GEMM leveraging multidimensional addressing
 *
 * \note Multi-dimensional addressing and the corresponding tensor buffer streams were introduced with AIE-ML/XDNA1
 *
 * Below is an example of an optimized bfloat16 GEMM kernel in which both input matrices, A and B, are addressed in the following 4D patterns (see \ref tensor_buffer_streams):
 *
 * \image html tensor_desc/mmul.png width=50%
 *
 * It is assumed that the data for both input matrices are pre-tiled and that the tiles are laid out in column-major order in memory.
 *
 * @snippet gemm_bf16xbf16.cpp Matrix multiplication
 *
 * @section group_mmul_page_supported_sparse_modes Supported Sparse Matrix Multiplication Modes
 *
 * AIE-ML/XDNA1 introduced hardware support for sparse matrix multiplication. For an M x K x N matrix multiplication with
 * A being M x K, B being K x N, and C being M x N, a sparse B matrix may be stored in memory using a data layout
 * which avoids storing zero values.
 *
 * \note Sparse matrix multiplications require that the sparse data be stored in column major layout.
 * An internal transpose of the partially decompressed data is required by the underlying intrinsics
 * and is carried out automatically by the API.
 *
 * <table>
 * <caption>Matrix multiplication modes for real types (sparse B matrix)</caption>
 * <tr><th>Arch.<th>8b x 4b<th>8b x 8b<th>16b x 8b<th>16b x 16b<th>%bfloat16 x %bfloat16
 * <tr><td>
 *         AIE-ML/XDNA1
 *     <td style="vertical-align:top">
 *         4x32x8
 *     <td style="vertical-align:top">
 *         4x16x8<br> 8x16x8<sup>a</sup><br> 4x16x16<sup>ab</sup>
 *     <td style="vertical-align:top">
 *         2x16x8<br> 4x16x8<sup>a</sup>
 *     <td style="vertical-align:top">
 *         2x8x8<br> 4x8x8<sup>a</sup><br> 2x8x16<sup>ab</sup>
 *     <td style="vertical-align:top">
 *         4x16x4<br> 4x16x8<sup>ab</sup>
 * <tr><td>
 *         XDNA2
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *         4x16x8<br> 8x16x8
 *     <td style="vertical-align:top">
 *         2x16x8<br> 4x16x8
 *     <td style="vertical-align:top">
 *         2x8x8<br> 4x8x8
 *     <td style="vertical-align:top">
 * <tr><td>
 *         AIE-MLv2
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *         4x16x8<br> 8x16x8
 *     <td style="vertical-align:top">
 *     <td style="vertical-align:top">
 *         2x8x8<br> 4x8x8
 *     <td style="vertical-align:top">
 *         4x16x8
 * </table>
 *
 * \note
 * <dl class="footnote">
 * <dt>a</dt><dd>Emulated using multiple intrinsic calls</dd>
 * <dt>b</dt><dd>Require additional data manipulation</dd>
 * </dl>
 *
 * The following example shows an optimized `int8 * sparse int8` GEMM:
 * 
 * @snippet gemm_int8xint8_sparse.cpp Sparse matrix multiplication
 */

namespace aie {

enum class accum_ownership {
    owned,
    ref
};

}

namespace aie::detail {

template <typename TypeA, typename TypeB>
struct compute_C_type;

template <> struct compute_C_type<int8,  int8>  { using type = int8; };
template <> struct compute_C_type<int8,  uint8> { using type = int8; };
template <> struct compute_C_type<uint8, int8>  { using type = int8; };
template <> struct compute_C_type<uint8, uint8> { using type = uint8; };

template <typename TypeA, typename TypeB>
using compute_C_type_t = typename compute_C_type<TypeA, TypeB>::type;

template <unsigned M, unsigned K, unsigned N, typename TypeA, typename TypeB, unsigned AccumBits, accum_ownership Ownership = accum_ownership::owned>
struct mmul;

template <typename TypeA, typename TypeB, unsigned AccumBits, unsigned Elems, unsigned NumAccum, accum_ownership Ownership = accum_ownership::owned>
struct C_block;

template <typename TypeA, typename TypeB, unsigned AccumBits, unsigned Elems>
struct C_block<TypeA, TypeB, AccumBits, Elems, 1, accum_ownership::owned>
{
    using  accum_tag = accum_tag_for_mul_types<TypeA, TypeB, AccumBits>;
    using accum_type = accum<accum_tag, Elems>;

    accum_type data;
    bool       zero;

    __aie_inline
    C_block() : zero(true)
    {}

    __aie_inline
    C_block(const accum_type &acc, bool to_zero = false) : data(acc), zero(to_zero)
    {}

    template <typename T>
    __aie_inline
    C_block(const vector<T, Elems> &v, int shift = 0) : C_block(accum_type(v, shift))
    {}

    __aie_inline
    accum_type to_accum() const
    {
        return data;
    }

    __aie_inline
    operator accum_type() const
    {
        return to_accum();
    }

    template <typename T>
    __aie_inline
    auto to_vector_sign(bool v_sign, int shift = 0) const
    {
        return data.template to_vector_sign<T>(v_sign, shift);
    }

    template <typename T>
    __aie_inline
    auto to_vector(int shift = 0) const
    {
        return to_vector_sign<T>(is_signed_v<T>, shift);
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits, unsigned Elems, unsigned NumAccums>
    requires (NumAccums > 1)
struct C_block<TypeA, TypeB, AccumBits, Elems, NumAccums>
{
    using           accum_tag = accum_tag_for_mul_types<TypeA, TypeB, AccumBits>;
    using          accum_type = accum<accum_tag, Elems>;
    using internal_accum_type = accum<accum_tag, Elems / NumAccums>;

    std::array<internal_accum_type, NumAccums> data;
    bool       zero;

    __aie_inline
    C_block() : zero(true)
    {}

    __aie_inline
    C_block(const accum_type &acc, bool to_zero = false) : zero(to_zero)
    {
        utils::unroll_times<NumAccums>([&](unsigned idx){ ;
            data[idx] = acc.template extract<Elems / NumAccums>(idx);
        });
    }

    template <typename T>
    __aie_inline
    C_block(const vector<T, Elems> &v, int shift = 0) : C_block(accum_type(v, shift))
    {
    }

    __aie_inline
    accum_type to_accum() const
    {
        return utils::apply_tuple([](auto&&... ts) __aie_inline {
            return concat_accum(std::forward<decltype(ts)>(ts)...);
        }, data);
    }

    __aie_inline
    operator accum_type() const
    {
        return to_accum();
    }

    template <typename T>
    __aie_inline
    auto to_vector_sign(bool v_sign, int shift = 0) const
    {
        return to_accum().template to_vector_sign<T>(v_sign, shift);
    }

    template <typename T>
    __aie_inline
    auto to_vector(int shift = 0) const
    {
        return to_vector_sign<T>(is_signed_v<T>, shift);
    }
};

template <typename TypeA, typename TypeB, unsigned AccumBits, unsigned Elems, unsigned Ratio>
struct C_block_larger_internal
{
    using           accum_tag = accum_tag_for_mul_types<TypeA, TypeB, AccumBits>;
    using          accum_type = accum<accum_tag, Elems>;
    using internal_accum_type = accum<accum_tag, Elems * Ratio>;

    internal_accum_type data;
    bool       zero;

    __aie_inline
    C_block_larger_internal() : zero(true)
    {}

    __aie_inline
    C_block_larger_internal(const accum_type &acc, bool to_zero = false) : data(acc.template grow<Elems * Ratio>()), zero(to_zero)
    {}

    template <typename T>
    __aie_inline
    C_block_larger_internal(const vector<T, Elems> &v, int shift = 0) : C_block_larger_internal(accum_type(v, shift))
    {}

    __aie_inline
    accum_type to_accum() const
    {
        return data.template extract<Elems>(0);
    }

    __aie_inline
    operator accum_type() const
    {
        return to_accum();
    }

    template <typename T>
    __aie_inline
    vector<T, Elems> to_vector_sign(bool v_sign, int shift = 0) const
    {
        return data.template extract<Elems>(0).template to_vector_sign<T>(v_sign, shift);
    }

    template <typename T>
    __aie_inline
    vector<T, Elems> to_vector(int shift = 0) const
    {
        return to_vector_sign<T>(is_signed_v<T>, shift);
    }
};

#if __AIE_ARCH__ >= 20
// C_block_interleave_cols implementation requires accumulator shuffles
// which aren't supported on AIE1 due to loss of precision
template <typename TypeA, typename TypeB, unsigned AccumBits, unsigned Elems, unsigned NumCols, unsigned NumAccums = 1>
struct C_block_interleave_cols;

template <typename TypeA, typename TypeB, unsigned AccumBits, unsigned Elems, unsigned NumCols>
struct C_block_interleave_cols<TypeA, TypeB, AccumBits, Elems, NumCols, 1>
{
    using           accum_tag = accum_tag_for_mul_types<TypeA, TypeB, AccumBits>;
    using          accum_type = accum<accum_tag, Elems>;
    using internal_accum_type = accum<accum_tag, Elems / 2>;

    internal_accum_type data[2];
    bool                zero;

    __aie_inline
    C_block_interleave_cols() : zero(true)
    {}

    __aie_inline
    C_block_interleave_cols(const accum_type &acc, bool to_zero = false)
    {
        accum_type ret;
        zero = to_zero;

        constexpr unsigned factor = AccumBits == 64 ? 2 : 1;

        const auto [tmp1, tmp2] = interleave_unzip<interleave_t, Elems / 2>::run(acc_to_vec::run(acc.template extract<Elems / 2>(0)),
                                                                                 acc_to_vec::run(acc.template extract<Elems / 2>(1)),
                                                                                 (NumCols / 2) * factor);

        data[0] = vector_to_accum_cast<accum_tag, interleave_t, Elems / 2>::run(tmp1);
        data[1] = vector_to_accum_cast<accum_tag, interleave_t, Elems / 2>::run(tmp2);
    }

    template <typename T>
    __aie_inline
    C_block_interleave_cols(const vector<T, Elems> &v, int shift = 0)
    {
        accum_type ret;

        const auto [tmp1, tmp2] = interleave_unzip<T, Elems / 2>::run(v.template extract<Elems / 2>(0),
                                                                      v.template extract<Elems / 2>(1),
                                                                      NumCols / 2);

        data[0].from_vector(tmp1, shift);
        data[1].from_vector(tmp2, shift);
        zero = false;
    }

    __aie_inline
    accum_type to_accum() const
    {
        accum_type ret;

        const auto [tmp1, tmp2] = interleave_zip<interleave_t, Elems / 2>::run(acc_to_vec::run(data[0]),
                                                                               acc_to_vec::run(data[1]),
                                                                               NumCols / 2);

        ret.insert(0, vec_to_acc::run(tmp1));
        ret.insert(1, vec_to_acc::run(tmp2));

        return ret;
    }

    __aie_inline
    operator accum_type() const
    {
        return to_accum();
    }

    template <typename T>
    __aie_inline
    vector<T, Elems> to_vector_sign(bool v_sign, int shift = 0) const
    {
        return to_accum().template to_vector_sign<T>(v_sign, shift);
    }

    template <typename T>
    __aie_inline
    vector<T, Elems> to_vector(int shift = 0) const
    {
        return to_vector_sign<T>(is_signed_v<T>, shift);
    }

private:
    using interleave_t = int32;
    using acc_to_vec   = accum_to_vector_cast<interleave_t, accum_tag,    Elems / 2>;
    using vec_to_acc   = vector_to_accum_cast<accum_tag,    interleave_t, Elems / 2>;
};

template <typename TypeA, typename TypeB, unsigned AccumBits, unsigned Elems, unsigned NumCols, unsigned NumAccums>
    requires (NumAccums > 1)
struct C_block_interleave_cols<TypeA, TypeB, AccumBits, Elems, NumCols, NumAccums>
{
    static constexpr unsigned lanes_per_internal_acc = Elems / NumAccums;
    static constexpr unsigned lanes_per_shuffled_acc = Elems / NumAccums / 2;
    using           accum_tag = accum_tag_for_mul_types<TypeA, TypeB, AccumBits>;
    using          accum_type = accum<accum_tag, Elems>;
    using internal_accum_type = accum<accum_tag, lanes_per_internal_acc>;
    using shuffled_accum_type = accum<accum_tag, lanes_per_shuffled_acc>;

    shuffled_accum_type data[NumAccums][2];
    bool                zero;

    __aie_inline
    C_block_interleave_cols() : zero(true)
    {}

    __aie_inline
    C_block_interleave_cols(const accum_type &acc, bool to_zero = false)
    {
        accum_type ret;
        zero = to_zero;

        constexpr unsigned factor = AccumBits == 64 ? 2 : 1;

        utils::unroll_times<NumAccums>([&](unsigned idx) __aie_inline {
            const auto [tmp1, tmp2] = interleave_unzip<interleave_t, lanes_per_shuffled_acc>::run(
                    acc_to_vec::run(acc.template extract<lanes_per_shuffled_acc>(2 * idx + 0)),
                    acc_to_vec::run(acc.template extract<lanes_per_shuffled_acc>(2 * idx + 1)),
                    (NumCols / 2) * factor);

            data[idx][0] = vector_to_accum_cast<accum_tag, interleave_t, lanes_per_shuffled_acc>::run(tmp1);
            data[idx][1] = vector_to_accum_cast<accum_tag, interleave_t, lanes_per_shuffled_acc>::run(tmp2);
        });
    }

    template <typename T>
    __aie_inline
    C_block_interleave_cols(const vector<T, Elems> &v, int shift = 0)
    {
        accum_type ret;

        utils::unroll_times<NumAccums>([&](unsigned idx) __aie_inline {
            const auto [tmp1, tmp2] = interleave_unzip<T, lanes_per_shuffled_acc>::run(
                    v.template extract<lanes_per_shuffled_acc>(2 * idx + 0),
                    v.template extract<lanes_per_shuffled_acc>(2 * idx + 1),
                    NumCols / 2);

            data[idx][0].from_vector(tmp1, shift);
            data[idx][1].from_vector(tmp2, shift);
        });

        zero = false;
    }

    __aie_inline
    accum_type to_accum() const
    {
        accum_type ret;

        utils::unroll_times<NumAccums>([&](unsigned idx) __aie_inline {
            const auto [tmp1, tmp2] = interleave_zip<interleave_t, lanes_per_shuffled_acc>::run(
                    acc_to_vec::run(data[idx][0]),
                    acc_to_vec::run(data[idx][1]),
                    NumCols / 2);

            ret.insert(2 * idx + 0, vec_to_acc::run(tmp1));
            ret.insert(2 * idx + 1, vec_to_acc::run(tmp2));
        });

        return ret;
    }

    __aie_inline
    operator accum_type() const
    {
        return to_accum();
    }

    template <typename T>
    __aie_inline
    vector<T, Elems> to_vector_sign(bool v_sign, int shift = 0) const
    {
        return to_accum().template to_vector_sign<T>(v_sign, shift);
    }

    template <typename T>
    __aie_inline
    vector<T, Elems> to_vector(int shift = 0) const
    {
        return to_vector_sign<T>(is_signed_v<T>, shift);
    }

private:
    using interleave_t = int32;
    using acc_to_vec   = accum_to_vector_cast<interleave_t, accum_tag,    lanes_per_shuffled_acc>;
    using vec_to_acc   = vector_to_accum_cast<accum_tag,    interleave_t, lanes_per_shuffled_acc>;
};

#endif //__AIE_ARCH__ >= 20

}

#if __AIE_ARCH__ == 10

#include "aie1/mmul.hpp"

#elif __AIE_ARCH__ == 20

#include "aie2/mmul.hpp"

#elif __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22

#include "aie2p/mmul.hpp"

#endif

#endif
