// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

/**
@page changelog Changelog

@tableofcontents{html,latex}

@section jan_2025 January 2025

<h3>Global AIE API changes</h3>

<ul>
<li>Added XDNA2 support</li>
</ul>

@section vitis_2024_2 Vitis 2024.2

<h3>Documentation changes</h3>

<ul>
</ul>

<h3>Global AIE API changes</h3>

<ul>
<li>Implement arbitrary vector and accum size support</li>
<li>Add cbfloat16 support on AIE-ML</li>
</ul>

<h3>Changes to data types</h3>

<ul>
<li>accum: Implement construction from block_vector</li>
<li>accum: Implement grow_replicate</li>
<li>mask: Enable get_submask to work with ElemsOut less than word size</li>
<li>mask: Implement insert and extract</li>
</ul>

<h3>Changes to operations</h3>

<ul>
<li>fft: Implement dynamic vectorization support for AIE</li>
<li>fft: Implement radix 2 and radix 4 cbfloat16 support on AIE-ML</li>
<li>mmul: Implement 8x8x8 bfloat16 mmul mode on AIE-ML</li>
<li>shuffle: Fix optimized code paths for when the shuffle mode is known at compile time</li>
</ul>

<h3>ADF integration</h3>

<ul>
<li>Enable cbfloat16 streams for AIE-ML</li>
<li>Implement vector reads and writes on cascades</li>
</ul>


@section vitis_2024_1 Vitis 2024.1

<h3>Documentation changes</h3>

<ul>
<li>Further refactoring to remove detail namespace from documentation</li>
<li>Document default accumulator type for given mul inputs</li>
<li>Document float conversion implementations and behaviour</li>
</ul>

<h3>Global AIE API changes</h3>

<ul>
<li>Add vector and accum template deduction guides</li>
<li>Leverage aie::zeros in place of aie:broadcast(0) internally to prevent non-trivial conversions</li>
<li>Several refactors to isolate external interfaces from detail namespace</li>
<li>Add clamp function</li>
<li>Add aie::saturating_add aie::saturating_sub functions</li>
<li>Add aie::real and aie::imag helpers to get real and imaginary parts of vectors</li>
</ul>

<h3>Changes to data types</h3>

<ul>
<li>accum: Fix sub-accum insertion</li>
<li>vector: Optimize vector::get for 1K vectors</li>
<li>vector: Extend pack/unpack to work for arbitrary conversions</li>
<li>tensor_buffer_stream: Fix native implementations for non-native vector sizes</li>
</ul>

<h3>Changes to operations</h3>

<ul>
<li>accumulate: Add array-based interface as alternative to variadic interface</li>
<li>compare: Add workaround for IEEE incompliance to equality comparisons for bfloat16 on AIE-ML</li>
<li>compare: Optimize comparisons with zero</li>
<li>compare: Fix scalar/vector comparisons on AIE</li>
<li>concat: Add support for concatenating tuples of vectors/accumulators</li>
<li>elementary: Optimize vector unrolling for scalar functions</li>
<li>fft: Fix vectorization limits for odd-radix implementations</li>
<li>fft: Add radix2 combiner for cint16 input/output with cint32 twiddles</li>
<li>fft: Add radix3 and radix5 stage0 implementations on AIE-ML</li>
<li>fft: Add cfloat radix3 and radix5 stage0 implementations on AIE</li>
<li>fft: Add 32b twiddle up/down dit FFTs</li>
<li>mmul: Add dynamic sign support for sparse_vector</li>
<li>mmul: Implemented additional modes</li>
    <ul>
    <li>AIE-ML: 16b x 8b - 8x4x8 , 32b x 16b - 4x4x8, bf16 x bf16 - 4x8x8</li>
    </ul>
<li>mmul: Add some outer product modes as aie::mmul<M,1,N></li>
<li>mmul: Enable zeroization for fp32 mmuls</li>
<li>mul: Implement cint32 * int16 and int16 * cint32 muls</li>
<li>neg: Add bfloat16 support for AIE-ML</li>
<li>sliding_mul: Optimize complex x real implementations</li>
<li>sliding_mul: Add bfloat16 support on AIE-ML</li>
<li>to_fixed/to_float: Add support for unsigned types on AIE-ML</li>
</ul>

<h3>ADF integration</h3>

<ul>
<li>Fix cfloat stream reads</li>
</ul>

@section vitis_2023_2 Vitis 2023.2

<h3>Documentation changes</h3>

<ul>
<li>Integrate AIE-ML documentation</li>
<li>Document rounding modes</li>
<li>Expand accumulate documentation</li>
<li>Clarify limitations on 8b parallel lookup</li>
<li>Fix mmul member documentation</li>
<li>Clarify requirement for linear_approx step bits</li>
<li>Improve documentation of vector, accum, and mask</li>
<li>Highlight architecture requirements of functions using C++ requires clauses</li>
<li>Document FFT twiddle factor generation</li>
<li>Clarify internal rounding mode for bfloat16 to integer conversion</li>
<li>Clarify native and emulated modes for mmul</li>
<li>Clarify native and emulated modes for sliding_mul</li>
<li>Document sparse_vector_input_buffer_stream with memory layout and GEMM example</li>
<li>Document tensor_buffer_stream with a GEMM example</li>
</ul>

<h3>Global AIE API changes</h3>

<ul>
<li>Add cfloat support for AIE-ML</li>
</ul>

<h3>Changes to data types</h3>

<ul>
<li>vector: Optimize grow_replicate on AIE-ML</li>
<li>mmul: Support reinitialization from an accum</li>
<li>DM resources: Add compound aie_dm_resource variants</li>
<li>streams: Add sparse_vector_input_buffer_stream for loading sparse data on AIE-ML</li>
<li>streams: Add tensor_buffer_stream to handle multi-dimensional addressing for AIE-ML</li>
<li>bfloat16: Add specialization for std::numeric_limits on AIE-ML</li>
</ul>

<h3>Changes to operations</h3>

<ul>
<li>abs: Fix for float input</li>
<li>add_reduce: Optimize for 8b and 16b types on AIE-ML</li>
<li>div: Implement vector-vector and vector-scalar division</li>
<li>downshift: Implement logical_downshift for AIE</li>
<li>fft: Add support for 32 bit twiddles on AIE</li>
<li>fft: Fix for radix-3 and radix-5 FFTs on AIE</li>
<li>fft: Fix radix-5 performance for low vectorizations on AIE</li>
<li>fft: Add stage-based FFT functions and deprecate iterator interface</li>
<li>mul: Fix for vector * vector_elem_ref on AIE</li>
<li>print_fixed: Support printing Q format data</li>
<li>print_matrix: Added accumulator support</li>
<li>sliding_mul: Add support float</li>
<li>sliding_mul: Add support for remaining 32b modes for AIE-ML</li>
<li>sliding_mul: Add support for Points < Native Points</li>
<li>sliding_mul_ch: Fix DataStepX == DataStepY requirement</li>
<li>sincos: Optimize AIE implementation</li>
<li>to_fixed: Fix for AIE-ML</li>
<li>to_fixed/to_float: Add vectorized float conversions for AIE</li>
<li>to_fixed/to_float: Add generic conversions ((int8, int16, int32) <-> (bfloat16, float)) for AIE-ML</li>
</ul>

<h3>ADF integration</h3>

<ul>
<li>Add TLAST support for stream reads on AIE-ML</li>
<li>Add support for input_cascade and output_cascade types</li>
<li>Deprecate accum reads from input_stream and output_stream</li>
</ul>



@section vitis_2023_1 Vitis 2023.1

<h3>Documentation changes</h3>

<ul>
<li>Add explanation of FFT inputs</li>
<li>Use block_size in FFT docs</li>
<li>Clarify matrix data layout expectations</li>
<li>Clarify downshift being arithmetic</li>
<li>Correct description of bfloat16 linear_approx lookup table</li>
</ul>

<h3>Global AIE API changes</h3>

<ul>
<li>Do not explicitly initialize inferred template arguments</li>
<li>More aggressive inlining of internal functions</li>
<li>Avoid using 128b vectors in stream helper functions for AIE-ML</li>
</ul>

<h3>Changes to data types</h3>

<ul>
<li>iterator: Do not declare iterator data members as const</li>
<li>mask: Optimized implementation for 64b masks on AIE-ML</li>
<li>mask: New constructors added to initialize the mask from uint32 or uint64 values</li>
<li>vector: Fix 1024b inserts</li>
<li>vector: Use 128b concats in upd_all</li>
<li>vector: Fix 8b unsigned to_vector for AIE-ML</li>
</ul>

<h3>Changes to operations</h3>

<ul>
<li>add/sub: Support for dynamic accumulator zeroization</li>
<li>begin_restrict_vector: Add implementation for io_buffer</li>
<li>eq: Add support for complex numbers</li>
<li>fft: Correctly set radix configuration in fft::begin_stage calls</li>
<li>inv/invsqrt: Add implementation for AIE-ML</li>
<li>linear_approx: Performance optimization for AIE-ML</li>
<li>logical_downshift: New function that implements a logical downshift (as opposed to aie::downshift, which is arithmetic)</li>
<li>max/min/maxdiff: Add support for dynamic sign</li>
<li>mmul: Implement 16b 8x2x8 mode for AIE-ML</li>
<li>mmul: Implement 8b 8x8x8 mode for AIE-ML</li>
<li>mmul: Implemet missing 16b x 8b and 8b x 4b sparse multiplication modes for AIE-ML</li>
<li>neq: Add support for complex numbers</li>
<li>parallel_lookup: Optimize implementation for signed truncation</li>
<li>print_matrix: New function that prints vectors with the specified matrix shape</li>
<li>shuffle_up/down: Minor optimization for 16b</li>
<li>shuffle_up/down: Optimized implementation for AIE-ML</li>
<li>sliding_mul: Support data_start/coeff_start values larger than vector size</li>
<li>sliding_mul: Add support for 32b modes for AIE-ML</li>
<li>sliding_mul: Add 2 point 16b 16 channel for AIE-ML</li>
<li>sliding_mul_ch: New function for multi-channel multiplication modes for AIE-ML</li>
<li>sliding_mul_sym_uct: Fix for 16b two-buffer implementation</li>
<li>store_unaligned_v: Optimized implementation for AIE-ML</li>
<li>transpose: Add support for 64b and 32b types</li>
<li>transpose: Enable transposition of 256 element 4b vectors (scalar implementation for now)</li>
<li>to_fixed: Add bfloat16 to int32 conversion on AIE-ML</li>
</ul>



@section vitis_2022_2 Vitis 2022.2

<h3>Documentation changes</h3>

<ul>
<li>Add code samples for load_v/store_v and load_unaligned_v/store_unaligned_v</li>
<li>Enhanced documentation for parallel_lookup and linear_approx</li>
<li>Clarify coeff vector size limit on AIE-ML</li>
</ul>

<h3>Global AIE API changes</h3>

<ul>
<li>Remove usage of srs in compare functions, to avoid compilation warnings as it is deprecated</li>
<li>Add support for stream ADF vector types on AIE-ML</li>
</ul>

<h3>Changes to data types</h3>

<ul>
<li>mask: add shift operators</li>
<li>saturation_mode: add saturate value. It was previously named truncate, which is not correct. The old name is also kept until it is deprecated</li>
</ul>

<h3>Changes to operations</h3>

<ul>
<li>add: support accumulator addition on AIE-ML</li>
<li>add_reduce: add optimized implementation for cfloat on AIE</li>
<li>add_reduce: add optimized implementation for bfloat16 on AIE-ML</li>
<li>eq/neq: enhanced implementation on AIE-ML</li>
<li>le: enhanced implementation on AIE-ML</li>
<li>load_unaligned_v: leverage pointer truncation to 128b done by HW on AIE</li>
<li>fft: add support for radix 3/5 on AIE</li>
<li>mmul: add matrix x vector multiplicatio modes on AIE</li>
<li>mmul: add support for dynamic accumulator zeroization</li>
<li>to_fixed: added implementation for AIE-ML</li>
<li>to_fixed: provide a default return type</li>
<li>to_float: added implementation for AIE-ML</li>
<li>reverse: optimized implementation for 32b and 64b on AIE-ML</li>
<li>zeros: include fixes on AIE</li>
</ul>



@section vitis_2022_1 Vitis 2022.1

<h3>Documentation changes</h3>

<ul>
<li>Small documentation fixes for operators</li>
<li>Issues of documentation on msc_square and mmul</li>
<li>Enhance documentation for sliding_mul operations</li>
<li>Change logo in documentation</li>
<li>Add documentation for ADF stream operators</li>
</ul>

<h3>Global AIE API changes</h3>

<ul>
<li>Add support for emulated FP32 data types and operations on AIE-ML</li>
</ul>

<h3>Changes to data types</h3>

<ul>
<li>unaligned_vector_iterator: add new type and helper functions</li>
<li>random_circular_vector_iterator: add new type and helper functions</li>
<li>iterator: add linear iterator type and helper functions for scalar values</li>
<li>accum: add support for dynamic sign in to/from_vector on AIE-ML</li>
<li>accum: add implicit conversion to float on AIE-ML</li>
<li>vector: add support for dynamic sign in pack/unpack</li>
<li>vector: optimization of initialization by value on AIE-ML</li>
<li>vector: add constructor from 1024b native types on AIE-ML</li>
<li>vector: fixes and optimizations for unaligned_load/store</li>
</ul>

<h3>Changes to operations</h3>

<ul>
<li>adf::buffer_port: add many wrapper iterators</li>
<li>adf::stream: annotate read/write functions with stream resource so they can be scheduled in parallel</li>
<li>adf::stream: add stream operator overloading</li>
<li>fft: performance fixes on AIE-ML</li>
<li>max/min/maxdiff: add support for bfloat16 and float on AIE-ML</li>
<li>mul/mmul: add support for bfloat16 and float on AIE-ML</li>
<li>mul/mmul: add support for dynamic sign AIE-ML</li>
<li>parallel_lookup: expanded to int16->bfloat, performance optimisations, and softmax kernel</li>
<li>print: add support to print accumulators</li>
<li>add/max/min_reduce: add support for float on AIE-ML</li>
<li>reverse: add optimized implementation on AIE-ML using matrix multiplications</li>
<li>shuffle_down_replicate: add new function</li>
<li>sliding_mul: add 32b for 8b * 8b and 16b * 16b on AIE-ML</li>
<li>transpose: add new function and implementation for AIE-ML</li>
<li>upshift/downshift: add implementation for AIE-ML</li>
</ul>



@section vitis_2021_2 Vitis 2021.2

<h3>Documentation changes</h3>

<ul>
<li>Fix description of sliding_mul_sym_uct</li>
<li>Make return types explicit for better documentation</li>
<li>Fix documentation for sin/cos so that it says that the input must be in radians</li>
<li>Add support for concepts</li>
<li>Add documenttion for missing arguments and fix wrong argument names</li>
<li>Fixes in documentation for int4/uint4 AIE-ML types</li>
<li>Add documentation for the mmul class</li>
<li>Update documentation about supported accumulator sizes</li>
<li>Update the matrix multiplication example to use the new MxKxN scheme and size_A/size_B/size_C</li>
</ul>

<h3>Global AIE API changes</h3>

<ul>
<li>Make all entry points always_inline</li>
<li>Add declaration macros to aie_declaration.hpp so that they can be used in headers parsed by aiecompiler</li>
</ul>

<h3>Changes to data types</h3>

<ul>
<li>Add support for bfloat16 data type on AIE-ML</li>
<li>Add support for cint16/cint32 data types on AIE-ML</li>
<li>Add an argument to vector::grow, to specify where the input vector will be located in the output vector</li>
<li>Remove copy constructor so that the vector type becomes trivial</li>
<li>Remove copy constructor so that the mask type becomes trivial</li>
<li>Make all member functions in circular_index constexpr</li>
<li>Add tiled_mdspan::begin_vector_dim functions that return vector iterators</li>
<li>Initial support for sparse vectors on AIE-ML, including iterators to read from memory</li>
<li>Make vector methods always_inline</li>
<li>Make vector::push be applied to the object it is called on and return a reference</li>
</ul>

<h3>Changes to operations</h3>

<ul>
<li>add: Implementation optimization on AIE-ML</li>

<li>add_reduce: Implement on AIE-ML</li>

<li>bit/or/xor: Implement scalar x vector variants of bit operations</li>

<li>equal/not_equal: Add fix in which not all lanes were being compared for certain vector sizes.</li>

<li>fft: Interface change to enhance portability across AIE/AIE-ML</li>
<li>fft: Add initial support on AIE-ML</li>
<li>fft: Add alignment checks for x86sim in FFT iterators</li>
<li>fft: Make FFT output interface uniform for radix 2 cint16 upscale version on AIE</li>

<li>filter_even/filter_odd: Functional fixes</li>
<li>filter_even/filter_odd: Performance improvement for 4b/8b/16b implementations</li>
<li>filter_even/filter_odd: Performance optimization on AIE-ML</li>
<li>filter_even/filter_odd: Do not require step argument to be a compile-time constant</li>

<li>interleave_zip/interleave_unzip: Improve performance when configuration is a run-time value</li>
<li>interleave_*: Do not require step argument to be a compile-time constant</li>

<li>load_floor_v/load_floor_bytes_v: New functions that floor the pointer to a requested boundary before performing the load.</li>

<li>load_unaligned_v/store_unaligned_v: Performance optimization on AIE-ML</li>

<li>lut/parallel_lookup/linear_approx: First implementation of look-up based linear functions on AIE-ML.</li>

<li>max_reduce/min_reduce: Add 8b implementation</li>
<li>max_reduce/min_reduce: Implement on AIE-ML</li>

<li>mmul: Implement new shapes for AIE-ML</li>
<li>mmul: Initial support for 4b multiplication</li>
<li>mmul: Add support for 80b accumulation for 16b x 32b / 32b x 16b cases</li>
<li>mmul: Change dimension names from MxNxK to MxKxN</li>
<li>mmul: Add size_A/size_B/size_C data members</li>

<li>mul: Optimized mul+conj operations to merged into a single intrinsic call on AIE-ML</li>

<li>sin/cos/sincos: Fix to avoid int -> unsigned conversions that reduce the range</li>
<li>sin/cos/sincos: Use a compile-time division to compute 1/PI</li>
<li>sin/cos/sincos: Fix floating-point range</li>
<li>sin/cos/sincos: Optimized implementation for float vector</li>

<li>shuffle_up/shuffle_down: Elements don't wrap around anymore. Instead, new elements are undefined.</li>
<li>shuffle_up_rotate/shuffle_down_rotate: New variants added for the cases in which elements need to wrap-around</li>
<li>shuffle_up_replicate: Variant added which replicates the first element.</li>
<li>shuffle_up_fill: Variant added which fills new elements with elements from another vector.</li>
<li>shuffle_*: Optimization in shuffle primitives on AIE, especially for 8b/16b cases</li>

<li>sliding_mul: Fixes to handle larger Step values for cfloat variants</li>
<li>sliding_mul: Initial implementation for 16b x 16b and cint16b x cint16b on AIE-ML</li>
<li>sliding_mul: Optimized mul+conj operations to merged into a single intrinsic call on AIE-ML</li>

<li>sliding_mul_sym: Fixes in start computation for filters with DataStepX > 1</li>
<li>sliding_mul_sym: Add missing int32 x int16 / int16 x int32 type combinations</li>
<li>sliding_mul_sym: Fix two-buffer sliding_mul_sym acc80</li>
<li>sliding_mul_sym: Add support for separate left/right start arguments</li>

<li>store_v: Support pointers annotated with storage attributes</li>
</ul>

*/
