// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_AIE_DOC__HPP__
#define __AIE_API_AIE_DOC__HPP__

#include "types.hpp"

/**
 * @mainpage Overview
 *
 * AIE API is a portable programming interface for AIE accelerators. It is implemented as a C++ header-only library that
 * provides types and operations that get translated into efficient low-level intrinsics. The API also provides
 * higher-level abstractions such as iterators and multi-dimensional arrays.
 */

/**
 * @defgroup group_basic_types Basic Types
 *
 * The two main types offered by the AIE API are vectors (@ref aie::vector) and accumulators (@ref aie::accum).
 *
 * ### Vector
 * A vector represents a collection of elements of the same type which is transparently mapped to the corresponding
 * vector registers supported on each architecture. Vectors are parametrized by the element type and the number of
 * elements, and any combination that defines a 128b/256b/512b/1024b vector is supported natively. Larger vectors
 * are also supported by transparently maintaining the required number of native vectors.
 *
 * @anchor vector_valid_parameters
 * <table>
 * <caption>Supported vector types and sizes</caption>
 * <tr><th>Arch.    <th>int4          <th>uint4         <th>int8         <th>uint8        <th>int16      <th>uint16     <th>int32     <th>uint32    <th>%bfloat16                 <th>float     <th>cint16    <th>cint32   <th>cbfloat16 <th>cfloat
 * <tr><td>AIE      <td>              <td>              <td>16/32/64/128 <td>16/32/64/128 <td>8/16/32/64 <td>           <td>4/8/16/32 <td>          <td>                          <td>4/8/16/32 <td>4/8/16/32 <td>2/4/8/16 <td>          <td>2/4/8/16
 * <tr><td>AIE-ML   <td>32/64/128/256 <td>32/64/128/256 <td>16/32/64/128 <td>16/32/64/128 <td>8/16/32/64 <td>8/16/32/64 <td>4/8/16/32 <td>4/8/16/32 <td>8/16/32/64                <td>4/8/16/32 <td>4/8/16/32 <td>2/4/8/16 <td>4/8/16/32 <td>2/4/8/16
 * </table>
 *
 * To declare a vector, specify the desired template parameters following the
 * @ref vector_valid_parameters "vector template parameters table". The following example declares a vector variable
 * with 32 elements of type `int16`:
 *
 * @code{.cpp}
 * aie::vector<int16, 32> my_vector;
 * @endcode
 *
 * Refer to @ref group_basic_types_initialization for examples of how to initialize vector objects.
 *
 * ### Accumulator
 *
 * An accumulator represents a collection of elements of the same class, typically obtained as a result of a
 * multiplication operation. They are transparently mapped to the corresponding accumulator registers supported on each
 * architecture. Accumulators commonly provide a large amount of bits, allowing users to perform long chains of
 * operations whose intermediate results would otherwise exceed the range of regular vector types. They are parametrized
 * by the element type (see @ref group_basic_types_accum) and the number of elements. Element types specify the class of
 * an element (e.g. integral, floating point, complex) and its _minimum_ amount of bits required. AIE API then maps it
 * to the nearest accumulator type that is supported natively.
 * Similarly to vectors, accumulators larger than the native sizes are supported by transparently maintaining the
 * required number of native accumulators.
 *
 * @anchor accum_valid_parameters
 * <table>
 * <caption>Supported accumulator types and native sizes</caption>
 * <tr><th>Arch.<th><th>%acc32<th>%acc40<th>%acc48<th>%acc56<th>%acc64<th>%acc72<th>%acc80<th>%accfloat<th>%cacc32<th>%cacc40<th>%cacc48<th>%cacc56<th>%cacc64<th>%cacc72<th>%cacc80<th>%caccfloat
 * <tr><td rowspan=2>AIE
 *     <td>Lanes
 *     <td>8/16/32/64/128
 *     <td>8/16/32/64/128
 *     <td>8/16/32/64/128
 *     <td>4/8/16/32/64
 *     <td>4/8/16/32/64
 *     <td>4/8/16/32/64
 *     <td>4/8/16/32/64
 *     <td>4/8/16/32
 *     <td>4/8/16/32/64
 *     <td>4/8/16/32/64
 *     <td>4/8/16/32/64
 *     <td>2/4/8/16/32
 *     <td>2/4/8/16/32
 *     <td>2/4/8/16/32
 *     <td>2/4/8/16/32
 *     <td>2/4/8/16
 * <tr><td>Native accumulation
 *     <td>48b
 *     <td>48b
 *     <td>48b
 *     <td>80b
 *     <td>80b
 *     <td>80b
 *     <td>80b
 *     <td>32b
 *     <td>48b
 *     <td>48b
 *     <td>48b
 *     <td>80b
 *     <td>80b
 *     <td>80b
 *     <td>80b
 *     <td>32b
 * <tr><td rowspan=2 style="white-space: nowrap">AIE-ML
 *     <td>Lanes
 *     <td>8/16/32/64/128
 *     <td>4/8/16/32/64
 *     <td>4/8/16/32/64
 *     <td>4/8/16/32/64
 *     <td>4/8/16/32/64
 *     <td>
 *     <td>
 *     <td>4/8/16/32/64/128
 *     <td>2/4/8/16/32
 *     <td>2/4/8/16/32
 *     <td>2/4/8/16/32
 *     <td>2/4/8/16/32
 *     <td>2/4/8/16/32
 *     <td>
 *     <td>
 *     <td>2/4/8/16/32/64
 * <tr><td>Native accumulation
 *     <td>32b
 *     <td>64b
 *     <td>64b
 *     <td>64b
 *     <td>64b
 *     <td>
 *     <td>
 *     <td>32b
 *     <td>64b
 *     <td>64b
 *     <td>64b
 *     <td>64b
 *     <td>64b
 *     <td>
 *     <td>
 *     <td>32b
 * </table>
 *
 * Like vectors, declaring an accumulator consists of specifying all its template parameters, following the
 * @ref accum_valid_parameters "accumulator template parameters table" for the accepted combinations when declaring accumulators.
 * For example:
 *
 * @code{.cpp}
 * aie::accum<accfloat, 16> my_accumulator;
 * @endcode
 *
 * Refer to @ref group_basic_types_initialization for examples of how to initialize accumulator objects.
 *
 * ### Mask
 * Some comparison operations return masks. A mask is a collection of values that can be 0 or 1.
 *
 * @code{.cpp}
 * aie::mask<64> my_mask;
 * @endcode
 *
 * Refer to @ref group_basic_types_initialization for examples of how to initialize mask objects.
 */

/**
 *
 * @defgroup group_basic_types_initialization Basic Type Initialization
 * @ingroup group_basic_types
 *
 * ### Vector Initialization
 *
 * The contents of a vector are undefined when default constructed.
 *
 * \warning Operations with undefined input vectors may not produce an error during compilation, but may not work as
 * expected.
 *
 * @code
 * aie::vector<int16, 16> v;
 * @endcode
 *
 * The simplest way of initializing a vector is from another vector of the same type and size.
 *
 * @code
 * aie::vector<int16, 16> v1;
 *
 * aie::vector<int16, 16> v2 = v1;
 * @endcode
 *
 * Or as the result of an operation.
 *
 * @code
 * aie::vector<int16, 16> v = aie::add(v1, v2);
 * @endcode
 *
 * A vector can also be read from memory using the aie::load_v operation or iterators. See @ref group_memory for
 * more details.
 *
 * @code
 *
 * aie::vector<int16, 16> v = aie::load_v<16>(ptr);
 *
 * @endcode
 *
 * Sections of a vector can be modified independently. It can be done in a per-element basis.
 *
 * @code
 *
 * aie::vector<int16, 16> v;
 * for (unsigned i = 0; i < v.size(); ++i)
 *     v[i] = i; // i-th element is updated
 *
 * @endcode
 *
 * Or by writing subvectors.
 *
 * @code
 *
 * aie::vector<int16, 8> v1;
 * aie::vector<int16, 8> v2;
 *
 * aie::vector<int16, 16> v;
 * v.insert(0, v1); // This updates elements 0-7
 * v.insert(1, v2); // This updates elements 8-15
 *
 * @endcode
 *
 * Vectors can also be concatenated into a larger vector.
 *
 * @code
 *
 * aie::vector<int16, 8> v1;
 * aie::vector<int16, 8> v2;
 *
 * aie::vector<int16, 16> v = aie::concat(v1, v2);
 *
 * @endcode
 *
 * ### Accumulator Initialization
 *
 * Accumulators support all the aforementioned vector operations but the individual element update.
 *
 * The contents of an accumulator are undefined when default constructed.
 *
 * \warning Reading an undefined input accumulator may not produce an error during compilation, but the operation may
 * not behave as intended.
 *
 * We can initialize an accumulator from the result of an operation. Some operations such as aie::mul, are able to
 * deduce the \ref DefaultAccumTag "optimal accumulator type from their input arguments". When this is the case, it is
 * not trivial for the developer to know what type each of the template parameters will be used in every situation, but
 * we can let the compiler deduce them instead:
 *
 * @code
 * aie::vector<int16, 16> a = ...
 * aie::vector<int32, 16> b = ...
 * aie::accum result = aie::mul(a, b);
 * @endcode
 * 
 * If we need to retrieve any of the template parameters later on, it is possible to access them by means of
 * class member typedefs:
 *
 * @code
 * using result_tag = decltype(result)::value_type;
 * constexpr size_t result_elems = result.size();
 * @endcode
 *
 * ### Initialization by Stream Read
 *
 * Both vectors and accumulators can also be read from ADF abstractions such as windows and streams. See @ref group_adf
 * for more details.
 *
 * @code
 *
 * aie::vector<int16, 8> v  = readincr_v<8>(input_stream);
 * aie::accum<acc48, 8> acc = readincr_v<8>(input_cascade);
 *
 * @endcode
 *
 * ### Mask Initialization
 *
 * Masks are usually initialized as a result of a comparison using vectors:
 *
 * @code
 *
 * aie::vector<int16, 16> a, b;
 * aie::mask<16> m = aie::lt(a, b);
 *
 * @endcode
 *
 * In addition, it is possible to initialize a mask using a constant value. For example:
 *
 * @code
 *
 * auto m1 = aie::mask<64>::from_uint64(0xaaaabbbbccccddddULL);
 * auto m2 = aie::mask<64>::from_uint32(0xaaaabbbb, 0xccccdddd);
 * auto m3 = aie::mask<16>::from_uint32(0b1010'1010'1011'1011);
 *
 * @endcode
 *
 * @par Tip:
 * You can use the standard C/C++ macros for fixed-sized integers instead of the integer suffix. In the definition of
 * `m1` it would look like `UINT64_C(0xaaaabbbbccccdddd)`.
 *
 * @defgroup group_basic_types_conversion Vector and Accumulator Conversions
 * @ingroup group_basic_types
 *
 * Vectors can be reinterpreted as vectors with a different element type, as long as they have the same total size.
 *
 * @code
 *
 * aie::vector<int16, 32> v;
 * aie::vector<int32, 16> v2;
 * aie::vector<cint16, 16> v3;
 *
 * v2 = v.cast_to<int32>(v);
 *
 * v3 = aie::vector_cast<cint16>(v);
 *
 * @endcode
 *
 * Vectors can be converted into accumulators. Their values can be shifted into a larger magnitude to implement
 * fixed point precision schemes (this does not apply to floating point accumulators).
 *
 * @code
 *
 * aie::vector<int16, 16> v;
 * aie::accum<acc32, 16> acc;
 *
 * acc.from_vector(v, shift);
 *
 * @endcode
 *
 * Conversely, accumulators can be converted into vectors. Their values can be shifted down before rounding and
 * saturation is applied (this does not apply to floating point accumulators).
 *
 * @code
 *
 * aie::accum<acc32, 16> acc;
 * aie::vector<int16, 16> v;
 *
 * v = acc.to_vector<int16>(shift);
 *
 * @endcode
 *
 * @defgroup group_basic_types_concepts Concepts for Basic Types
 * @ingroup group_basic_types
 *
 * @defgroup group_basic_types_accum Accumulator Element Types
 * @ingroup group_basic_types
 *
 * Accumulators in AIE API rely on the aie::accum class template. The first argument of this template is an element type
 * tag that specifies the accumulation class (integer, complex, floating point, ...) and the required accumulation bits.
 */
/**
 * @ingroup group_basic_types_accum
 * \struct acc32
 * \brief Tag used to request an accumulator with at least 32 bit per element.
 */
/**
 * @ingroup group_basic_types_accum
 * \struct acc40
 * \brief Tag used to request an accumulator with at least 40 bit per element.
 */
/**
 * @ingroup group_basic_types_accum
 * \struct acc48
 * \brief Tag used to request an accumulator with at least 48 bit per element.
 */
/**
 * @ingroup group_basic_types_accum
 * \struct acc56
 * \brief Tag used to request an accumulator with at least 56 bit per element.
 */
/**
 * @ingroup group_basic_types_accum
 * \struct acc64
 * \brief Tag used to request an accumulator with at least 64 bit per element.
 */
/**
 * @ingroup group_basic_types_accum
 * \struct acc72
 * \brief Tag used to request an accumulator with at least 72 bit per element.
 * \note Supported in AIE only.
 */
/**
 * @ingroup group_basic_types_accum
 * \struct acc80
 * \brief Tag used to request an accumulator with at least 80 bit per element.
 * \note Supported in AIE only.
 */
/**
 * @ingroup group_basic_types_accum
 * \struct accfloat
 * \brief Tag used to request an accumulator with single precision floating point elements.
 */
/**
 * @ingroup group_basic_types_accum
 * \struct cacc32
 * \brief Tag used to request an accumulator with complex elements of at least 32 bit per component.
 */
/**
 * @ingroup group_basic_types_accum
 * \struct cacc40
 * \brief Tag used to request an accumulator with complex elements of at least 40 bit per component.
 */
/**
 * @ingroup group_basic_types_accum
 * \struct cacc48
 * \brief Tag used to request an accumulator with complex elements of at least 48 bit per component.
 */
/**
 * @ingroup group_basic_types_accum
 * \struct cacc56
 * \brief Tag used to request an accumulator with complex elements of at least 56 bit per component.
 */
/**
 * @ingroup group_basic_types_accum
 * \struct cacc64
 * \brief Tag used to request an accumulator with complex elements of at least 64 bit per component.
 */
/**
 * @ingroup group_basic_types_accum
 * \struct cacc72
 * \brief Tag used to request an accumulator with complex elements of at least 72 bit per component.
 * \note Supported in AIE only.
 */
/**
 * @ingroup group_basic_types_accum
 * \struct cacc80
 * \brief Tag used to request an accumulator with complex elements of at least 80 bit per component.
 * \note Supported in AIE only.
 */
/**
 * @ingroup group_basic_types_accum
 * \struct caccfloat
 * \brief Tag used to request an accumulator with complex elements of single precision floating point components.
 */

 /**
 * @defgroup group_basic_types_ops Lazy Operations
 * @ingroup group_basic_types
 *
 * AIE architectures offer multiplication instructions that can perform additional operations on on the input arguments.
 * Instead of adding one variant for each possible combination, AIE API offers types that can wrap an existing vector,
 * accumulator of element reference and be passed into the multiplication function. Then the API will merge the
 * operations into a single instruction or apply the operation on the vector before the multiplication, depending on the
 * hardware support,
 *
 * The following example performs an element-wise multiplication of the absolute of vector a and the conjugate of
 * vector b.
 *
 * @code
 *
 * aie::accum<cacc48, 16> foo(aie::vector<int16, 16> a, aie::vector<cint16, 16> b)
 * {
 *     aie::accum<cacc48, 16> ret;
 *
 *     ret = aie::mul(aie::op_abs(a), aie::op_conj(b));
 *
 *     return ret;
 * }
 *
 * @endcode
 *
 */

/**
 * @defgroup group_memory Memory
 *
 * Each AIE core has access to up to 4 Data Memories (DM). Global variables, graph communication buffers (such as
 * windows), and the stack are placed by the linker on these memories.
 *
 * The AIE API provides a number of functions and types that allow applications to efficiently read and write
 * vector data stored on DM.
 *
 * @section memory_alignment Data Loading And Vector Alignment
 *
 * Applications can load from DM into vector registers and store the contents of vector registers into DM. Memory
 * instructions in the AIE that operate on vectors have alignment requirements as summarised in @ref vector_alignment_requirements "vector alignment requirements table".
 *
 * @anchor vector_alignment_requirements
 * <table>
 * <caption>Required pointer alignment for a given memory access size</caption>
 * <tr><th>Arch.    <th> 128b access  <th> 256b access
 * <tr><td>AIE      <td> 128b         <td>
 * <tr><td>AIE-ML   <td> 128b         <td> 256b
 * </table>
 *
 * Therefore, functions are provided for both aligned and unaligned accesses. Aligned accesses are done using the aie::load_v and aie::store_v functions.
 * Performing memory accesses to unaligned pointers using these functions is undefined behaviour.
 * The following code block shows example usages of the aligned access functions:
 *
 * @code
 *
 * template <typename T>
 * T* aligned_memcpy(T* __restrict dest, const T* src, unsigned n)
 * {
 *     // 256b vectors are used in this example
 *     static constexpr unsigned Bits  = 256;
 *     static constexpr unsigned Lanes = Bits / type_bits_v<T>;
 *     T* p = dest;
 *     // Assume n is divisible by Lanes
 *     for (unsigned i = 0; i < n / Lanes; ++i)
 *     {
 *         aie::vector<T, Lanes> v = aie::load_v<Lanes>(src);
 *         aie::store_v(p, v);
 *         src += Lanes;
 *         p   += Lanes;
 *     }
 *     return dest;
 * }
 *
 * @endcode
 *
 * while unaligned accesses are performed using aie::load_unaligned_v and aie::store_unaligned_v functions.
 * These unaligned access functions are demostrated below.
 *
 * @code
 *
 * template <typename T>
 * T* unaligned_memcpy(T* __restrict dest, const T* src, unsigned n, unsigned dest_align = 1, unsigned src_align = 1)
 * {
 *     // dest_align, src_align indicate to how many elements their respective pointers are aligned
 *     // 256b vectors are used in this example
 *     static constexpr unsigned Bits  = 256;
 *     static constexpr unsigned Lanes = Bits / type_bits_v<T>;
 *     T* p = dest;
 *     // Assume n is divisible by Lanes
 *     for (unsigned i = 0; i < n / Lanes; ++i)
 *     {
 *         aie::vector<T, Lanes> v = aie::load_unaligned_v<Lanes>(src, src_align);
 *         aie::store_unaligned_v(p, v, dest_align);
 *         src += Lanes;
 *         p   += Lanes;
 *     }
 *     return dest;
 * }
 *
 * @endcode
 *
 * Unaligned accesses may incur additional overhead depending on the amount of misalignment.
 *
 * Users can ensure that buffers are aligned using standard C/C++ facilities such as alignas. The API provides a global
 * constant value (aie::vector_decl_align) that can be used to align the buffer to a boundary that works for any vector
 * size.
 *
 * @code
 *
 * alignas(aie::vector_decl_align) static int16 my_buffer[BUFFER_COUNT];
 *
 * @endcode
 *
 * Additional helper functions, aie::load_floor_v, are available that will perform a load at the requested alignment from the given address:
 *
 * @code
 *
 * alignas(aie::vector_decl_align) static int16 data[] = {
 *     0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
 * };
 *
 * int16 *ptr = &data[3];
 *
 * aie::vector<int16, 16> v = aie::load_floor_v(ptr, 16);
 * aie::print(v, true); // Prints 0 1 2 ... 13 14 15
 *
 * @endcode
 *
 * @section memory_banks Memory bank conflicts
 *
 * AIE cores are able to perform several vector load/store operations per instruction. However, in order for them to be
 * executed in parallel, they must target different memory banks. aiecompiler will try to evenly distribute buffers from
 * communication primitives, and users can manually place buffers on specific banks by specifying the address range in
 * the linker script file.
 *
 * In general the compiler will try to schedule many accesses in the same instruction when possible. However, in
 * scenarios in which this would translate into bank conflicts, this behavior might not be desirable. The
 * compiler provides type annotations to associate memory accesses to virtual resources. Accesses using types that are
 * associated to the same virtual resource will not be scheduled in the same instruction.
 *
 * @code
 *
 * void fn(int __aie_dm_resource_a * A,
 *         int                     * B,
 *         int __aie_dm_resource_a * C)
 * {
 *     aie::vector<int, 8> v1 = aie::load_v<8>(A); // Access from A and C are bound to the same virtual resource so they
 *     aie::vector<int, 8> v2 = aie::load_v<8>(B); // are never scheduled on the same instruction. B is not annotated so
 *     aie::vector<int, 8> v3 = aie::load_v<8>(C); // its memory accesses can be scheduled in the same instruction with
 *                                                 // accesses to A or C.
 *     ...
 * }
 *
 * @endcode
 *
 * Also, most memory access functions in the AIE API accept an enum value from @ref aie_dm_resource that can be used to
 * bind individual accesses to a virtual resource.
 *
 * @code
 *
 * void fn(int __aie_dm_resource_a * A,
 *         int                     * B)
 * {
 *     aie::vector<int, 8> v1 = aie::load_v<8>(A);
 *     aie::vector<int, 8> v2 = aie::load_v<8>(B); // This access can be scheduled on the same instruction as the access
 *                                                 // to A since B is not annotated.
 *     aie::vector<int, 8> v3 = aie::load_v<8, aie_dm_resource::a>(B); // This specific access to B is annotated with
 *                                                                     // the same virtual resource as A, so they cannot
 *                                                                     // be scheduled on the same instruction.
 *     ...
 * }
 *
 * @endcode
 *
 * @section iterators Iterators
 *
 * The AIE API provides two kinds of iterators that map semantically to the C++ standard library's
 * [LegacyForwardIterator](https://en.cppreference.com/w/cpp/named_req/ForwardIterator) and
 * [LegacyRandomAccessIterator](https://en.cppreference.com/w/cpp/named_req/RandomAccessIterator).
 * The functionality exposed by these iterators are shown in the table below:
 *
 * <table>
 * <caption id="iterator_kinds">Iterator Kinds</caption>
 * <tr><th>Type                               <th>Operation                     <th>Code Example
 * <tr><td rowspan="3">Forward Iterator       <td>Dereference                   <td>`*it`
 * <tr>                                       <td>Equality/Inequality           <td>`it1 == it2` / `it1 != it2`
 * <tr>                                       <td>Pre/post-increment            <td>`++it` / `it++`
 * <tr><td rowspan="3">Random Access Iterator <td>Pre/post-decrement            <td>`--it` / `it--`
 * <tr>                                       <td>Arbitrary increment/decrement <td>`it += offset` / `it -= offset`
 * <tr>                                       <td>Random Access                 <td>`it[offset]`
 * </table>
 *
 * Note that the forward iterator is a subset of the random access iterator meaning that all operations implemeneted for
 * forward iterators are also implemented for random access iterators.
 * With these iterators defined, the types of iterators offered by the AIE API are outlined below with the iterator kind
 * expressing the operators defined for each iterator:
 *
 * <table>
 * <caption id="iterator_types">AIE API Iterator Types</caption>
 * <tr><th>                   <th>Type            <th>Constructor                         <th>Iterator Kind
 * <tr><td rowspan="4">Scalar <td>Basic           <td>`aie::begin`                        <td>Forward Iterator
 * <tr>                       <td>Circular        <td>`aie::begin_circular`               <td>Forward Iterator
 * <tr>                       <td>Random Circular <td>`aie::begin_random_circular`        <td>Random Access Iterator
 * <tr>                       <td>Pattern         <td>`aie::begin_pattern`                <td>Forward Iterator
 * <tr><td rowspan="5">Vector <td>Basic           <td>`aie::begin_vector`                 <td>Random Access Iterator
 * <tr>                       <td>Circular        <td>`aie::begin_vector_circular`        <td>Forward Iterator
 * <tr>                       <td>Random Circular <td>`aie::begin_vector_random_circular` <td>Random Access Iterator
 * <tr>                       <td>Restrict        <td>`aie::begin_restrict_vector`        <td>Random Access Iterator
 * <tr>                       <td>Unaligned       <td>`aie::begin_unaligned_vector`       <td>Forward Iterator
 * </table>
 *
 * All iterators also have const alternatives, which follow the naming convention laid out by the C++ standard library
 * i.e. the const version of an iterator created with `aie::begin_vector_circular` can be created with
 * `aie::cbegin_vector_circular`.
 *
 * @section buffer_streams Buffer Streams
 *
 * In cases where iterators are inappropriate due to either semantic or performance issues, buffer streams are provided.
 * Buffer streams do not conform to an iterator interface but rather a stream interface, similar to
 * [iostreams](https://en.cppreference.com/w/cpp/io/basic_iostream), which implies that a read or write operation will advance the stream,
 * altering its state.
 *
 * \note If the type name contains `input` or `output` the stream will only support reading or writing respectively.
 *
 * <table>
 * <caption id="stream_ops">AIE API Stream Operations</caption>
 * <tr><th>Operation  <th>Stream operator         <th>Member function
 * <tr><td>Read       <td>operator>>(value_type&) <td>`value_type pop()`
 * <tr><td>Write      <td>operator<<(value_type&) <td>`value_type push()`
 * </table>
 *
 * @subsection tensor_buffer_streams Tensor Buffer Streams
 *
 * Tensor buffer streams are an abtraction provided by the AIE API to handle multi-dimensional addressing.
 *
 * \note Multi-dimensional addressing was introduced on AIE-ML.
 *
 * @subsubsection tensor_buffer_streams_description Tensor Descriptor
 *
 * An aie::tensor_descriptor object serves as a mapping from a multidimensional tensor to a 1-D memory space. A tensor descriptor is
 * constructed from an element type, the number of elements that make up a vector block within the tensor, and a list of aie::tensor_dim
 * objects, which are pairs of size-step pairs that describe each dimension of the tensor.
 *
 * The following illustration shows how a 3-D volume can be described. Here the element type is `int8`, and the number of elements per block
 * is 32, resulting in segments of the tensor being `aie::vector<int8, 32>`. The size of each dimension is given in the first paremeter of each
 * aie::tensor_dim, while the size of the increment required to take a step in each dimension given as the second parameter. Note that this
 * example assumes that the data is laid out in row-major order. This tensor representation allows a subvolume of the tensor to be iterated
 * over a number of times by adding an additional aie::tensor_dim with step set to zero snd the size set to the desired number of iterations.
 * See \ref tensor_buffer_streams_composition for an example.
 *
 * \image html tensor_desc/overview.png width=30%
 *
 * Such a tensor descriptor may be used to construct a tensor buffer stream as shown below. The returned stream serializes the
 * accesses to the buffer as it is read, which is also illustraded below.
 *
 * @code
 * alignas(aie::vector_decl_align) static int16 buff[256];
 *
 * std::iota(buff, buff + 256, 0);
 *
 * auto desc = aie::make_tensor_descriptor<int16, 32>(
 *                      aie::tensor_dim(2u, 4),
 *                      aie::tensor_dim(2u, 2),
 *                      aie::tensor_dim(2u, 1));
 *
 * auto tbs = aie::make_tensor_buffer_stream(ptr, desc);
 *
 * for (unsigned i = 0; i < 8; ++i) {
 *     aie::vector<int16, 32> v;
 *     tbs >> v;
 *     // Alternatively:
 *     //     auto v = tbs.pop(); // will deduce the vector type from the tensor descriptor
 *     printf("%d: {%d, ..., %d\n}", i, v.get(0), v.get(31));
 * }
 *
 * // Prints:
 * // 0: {0, ..., 31}
 * // 1: {32, ..., 63}
 * // 2: {64, ..., 95}
 * // 3: {96, ..., 127}
 * // 4: {128, ..., 159}
 * // 5: {160, ..., 191}
 * // 6: {192, ..., 223}
 * // 7: {224, ..., 255}
 * @endcode
 *
 * \image html tensor_desc/serialize.png width=30%
 *
 * @subsubsection tensor_buffer_streams_composition Tensor Buffer Stream Composition
 *
 * Tensor descriptors and associated buffer streams are composible to arbitrary dimensions; however, the underlying mechanisms on which
 * the abstractions are built upon in AIE-ML are three-dimensional. To overcome this, the tensor buffer streams are defined recursively,
 * decomposing an N-dimensional tensor into `(N-1)/3` nested streams, with a final `N%3` leaf stream. To access an inner stream, the containing
 * outer stream must be read with a `.pop()` call, which will advance the outer stream and return the inner stream. This recursive
 * definition is illustrated below with a corresponding code snippet:
 *
 * \image html tensor_desc/recursion.png width=30%
 *
 * @code
 * auto desc = aie::make_tensor_descriptor<int16, 32>(
 *                      aie::tensor_dim(2u, 4),
 *                      aie::tensor_dim(2u, 0),
 *                      aie::tensor_dim(6u, 8),
 *                      aie::tensor_dim(4u, 1));
 *
 * auto tbs = aie::make_tensor_buffer_stream(ptr, desc);
 *
 * for (unsigned i = 0; i < 2*2*6; ++i) {
 *   auto tsb_inner = tbs.pop(); // Advance outer stream and return inner stream
 *
 *   aie::vector<int16, 32> a, b, c, d;
 *   tbs_inner >> a >> b >> c >> d;
 * }
 * @endcode
 *
 * For a practical example, see \ref group_mmul_page_multidim_gemm.
 *
 * @subsubsection tensor_buffer_streams_native Tensor Descriptor from Native Types
 *
 * In the case that the automatic decomposition described in \ref tensor_buffer_streams_composition is not desired,
 * it is possible to manually decompose the tensor using native integer, aie::dim_2d, and aie::dim_3d descriptions
 * of the increments to be carried out.
 * The arguments to aie::dim_2d and aie::dim_3d differ from the aie::tensor_dim description as the `num` values
 * represent the number of increments to carry out rather than the dimension size, and the increment at each
 * dimension assumes that all previous increments have already been carried out.
 *
 * @code
 * aie::make_tensor_descriptor<int16, 32>(
 *          aie::tensor_dim(2u, 4),
 *          aie::tensor_dim(2u, 2),
 *          aie::tensor_dim(2u, 1));
 *
 * // Is equivalent to:
 *
 * aie::make_tensor_descriptor_from_native<int16, 32>(
 *          aie::dim_3d(1u, 1,   // num1, inc1
 *                      1u, 1,   // num2, inc2
 *                          1)); //       inc3
 * @endcode
 *
 * As with the aie::tensor_dim description, dimensions can be composed arbitrarily using `int`, aie::dim_2d, and aie::dim_3d
 * increments:
 *
 * @code
 * // 6-D tensor decription
 * aie::make_tensor_descriptor_from_native<int16, 32>(
 *          aie::dim_3d(1u, 1,  // num1, inc1
 *                      1u, 1,  // num2, inc2
 *                          1), //       inc3
 *          8,
 *          aie::dim_2d(3u, 4,   // num1, inc1
 *                          4)); //       inc2
 * @endcode
 *
 * The exact increment values may also be set using aie::make_tensor_descriptor_from_native_bytes.
 *
 *
 * @subsection sparse_buffer_streams Sparse Vector Input Buffer Streams
 *
 * A buffer with appropriately prepared sparse data can be read using `aie::sparse_vector_input_buffer_stream`.
 *
 * @code
 *
 * auto vbs = aie::sparse_vector_input_buffer_stream<int8, 128>(ptr);
 *
 * aie::sparse_vector<int8, 128> a, b, c;
 * vbs >> a;           // 1. Single read.
 * vbs >> b >> c;      // 2. Multiple reads.
 * auto d = vbs.pop(); // 3. Identical to 1. but type of d is deduced to be aie::sparse_vector<int8, 128> from
 *                     //    the buffer stream declaration.
 *                     // This is useful as `d` does not need to be declared ahead of time.
 *
 * @endcode
 *
 * @subsection sparse_buffer_streams_data_format Sparse Data Format
 *
 * The supported sparse data layout requires a minimum of 50% sparsity. Specifically, two zero values within
 * each group of four consecutive values. This 50% is a lower bound on the sparsity, meaning that further compression
 * is possible if more zeroes are present.
 * Loading sparse data from memory will interpret the first 64 bits as a mask. If a mask bit is not set, then 8 bits
 * will be initialised to zero at its corresponding position. If the bit is set, then the 8 bits will be loading from
 * the incompressible data that follows the mask. Hence, masks describe the layout of 512 bit after decompression.
 *
 * Each mask must be aligned to a 32b boundary. Failure to meet this requirement will result in the sparse data
 * being parsed incorrectly.
 *
 * When loading sparse data, a partial decompression is carried out to reconstruct the data such that a 64b mask
 * is paired with 256b partially decompressed data. This partial decompression is carried out as described in the
 * following table. Note that 4 bits of the mask are used to represent two elements of the partially decompressed data.
 *
 * <table>
 * <caption>Sparse partial decompression</caption>
 * <tr> <th colspan="4">Mask bits <th colspan="2">Partially decompressed data
 * <tr> <td>0 <td>0 <td>0 <td>0 <td style="text-align:center">0 <td style="text-align:center">0
 * <tr> <td>0 <td>0 <td>0 <td>1 <td style="text-align:center">0 <td style="text-align:center">A
 * <tr> <td>0 <td>0 <td>1 <td>0 <td style="text-align:center">0 <td style="text-align:center">B
 * <tr> <td>0 <td>0 <td>1 <td>1 <td style="text-align:center">B <td style="text-align:center">A
 * <tr> <td>0 <td>1 <td>0 <td>0 <td style="text-align:center">C <td style="text-align:center">0
 * <tr> <td>0 <td>1 <td>0 <td>1 <td style="text-align:center">C <td style="text-align:center">A
 * <tr> <td>0 <td>1 <td>1 <td>0 <td style="text-align:center">C <td style="text-align:center">B
 * <tr> <td>1 <td>0 <td>0 <td>0 <td style="text-align:center">D <td style="text-align:center">0
 * <tr> <td>1 <td>0 <td>0 <td>1 <td style="text-align:center">D <td style="text-align:center">A
 * <tr> <td>1 <td>0 <td>1 <td>0 <td style="text-align:center">D <td style="text-align:center">B
 * <tr> <td>1 <td>1 <td>0 <td>0 <td style="text-align:center">D <td style="text-align:center">C
 * </table>
 *
 * The following example demonstrates how a 128 element sparse vector is read from memory. It requires two sets of sparse
 * data, each comprised of a 64b mask and the associated data.
 *
 * @code
 *
 * constexpr unsigned N = 512;
 * alignas(aie::vector_decl_align) static int8 data[N];
 *
 * void func() {
 *     // Example setup of sparse buffer
 *     // mask:
 *     data[1]  = 0b0000'0000; data[0]  = 0b0000'0011;
 *     data[3]  = 0b0000'0000; data[2]  = 0b0001'0000;
 *     data[5]  = 0b0000'0000; data[4]  = 0b0000'0000;
 *     data[7]  = 0b0000'0000; data[6]  = 0b0000'0000;
 *     // data:
 *     data[8]  = 1;    data[9] = 2;    data[10] = 3;    data[11] = 0;
 *     // Note: data[11] is still considered as data for the previous group to ensure
 *     //       correct alignment of the next group's mask
 *     // mask:
 *     data[13] = 0b0000'0001; data[12] = 0b0001'0001;
 *     data[15] = 0b0000'0000; data[14] = 0b0000'0000;
 *     data[17] = 0b0000'0000; data[16] = 0b0000'0000;
 *     data[19] = 0b1000'0000; data[18] = 0b0000'0000;
 *     // data:
 *     data[20] = 4;    data[21] = 5;    data[22] = 6;    data[23] = 7;
 *
 *     auto vbs = aie::sparse_vector_input_buffer_stream<int8, 128>(data);
 *
 *     aie::sparse_vector<int8, 128> b = vbs.pop();
 * }
 * @endcode
 *
 * For a more comprehensive sparse matrix multiplication example, see \ref group_mmul_page_supported_sparse_modes.
 *
 *
 */

/**
 * @defgroup group_init Initialization
 *
 * Operations to initialize vectors and accumulators.
 */

/**
 * @defgroup group_arithmetic Arithmetic
 *
 * AIE API provides a set of functions that implement arithmetic operations on vector types. Operands may be vectors,
 * values or vector element references and the supported operand combinations are:
 * - Vector / Vector: the type and the size of the vectors must match. The operation is performed element-wise between
 *   the corresponding elements in each vector.
 * - Value / Vector: the type of the value and the type of the elements of the vector must match. The operation has the
 *   same result as if the value was broadcast to a vector and then operated with the vector argument.
 * - Vector element reference / Vector. Similar as Value / Vector, but using an element reference, the AIE API may
 *   optimize the operation by accessing the element directly from its original location.
 *
 * The following code snippet shows an example that adds two input arrays into an output array using vectors. For
 * simplicity count must be divisible by 8.
 *
 * @code
 *
 * void add(int32 * __restrict out,
 *          const int32 * __restrict in1, const int32 * __restrict in2, unsigned count)
 * {
 *     for (unsigned i = 8; i < count; i += 8) {
 *         aie::vector<int32, 8> vec = aie::add(aie::load_v<8>(in1 + i),
 *                                              aie::load_v<8>(in2 + i));
 *
 *         aie::store_v(out, vec);
 *     }
 * }
 *
 * @endcode
 *
 * Operations that include a multiplication return an accumulator. The API defines a default accumulation, shown below,
 * for each combination of types. Note that the input types are unorderded.
 *
 * \anchor DefaultAccumTag
 * <table>
 * <caption>Default accumulator tag for different integral factor types</caption>
 * <tr>	<th>Type1    <td>int4   <td>int8   <td>int8   <td>int16  <td>int16  <td>int32              <td>cint16  <td>cint16  <td>cint16  <td>cint16  <td>cint32  <td>cint32              <td>cint32
 * <tr>	<th>Type2    <td>int8   <td>int8   <td>int16  <td>int16  <td>int32  <td>int32              <td>int16   <td>int32   <td>cint16  <td>cint32  <td>int16   <td>int32               <td>cint32
 * <tr>	<th>AIE      <td>       <td>%acc48 <td>%acc48 <td>%acc48 <td>%acc48 <td>%acc80             <td>%cacc48 <td>%cacc48 <td>%cacc48 <td>%cacc48 <td>%cacc48 <td>%cacc80             <td>%cacc80
 * <tr>	<th>AIE-ML   <td>%acc32 <td>%acc32 <td>%acc32 <td>%acc32 <td>%acc64 <td>%acc64<sup>a</sup> <td>%cacc64 <td>%cacc64 <td>%cacc64 <td>%cacc64 <td>%cacc64 <td>%cacc64 <sup>a</sup><td>%cacc64<sup>a</sup>
 * </table>
 *
 * \note
 * <sup>a</sup> - 32b x 32b multiplication is not natively supported on AIE-ML and is emulated using two 32b x 16b muls.<br/>
 *
 * <table>
 * <caption>Default accumulator tag for different floating point factor types</caption>
 * <tr>	<th>Type1    <td>bfloat16  <td>bfloat16   <td>%cbfloat16 <td>float     <td>float      <td>cfloat
 * <tr>	<th>Type2    <td>bfloat16  <td>%cbfloat16 <td>%cbfloat16 <td>float     <td>cfloat     <td>cfloat
 * <tr>	<th>AIE      <td>          <td>           <td>           <td>%accfloat <td>%caccfloat <td>%caccfloat
 * <tr>	<th>AIE-ML   <td>%accfloat <td>%caccfloat <td>%caccfloat <td>%accfloat <td>%caccfloat <td>%caccfloat
 * </table>
 *  
 * Users may specify a larger number of accumulation bits by explicitly passing an accumulator tag:
 *  
 * @code
 *
 * // Default accumulation will be used
 * auto acc = aie::mul(v1, v2);
 *
 * // 64b accumulation, at least, will be used
 * auto acc = aie::mul<acc64>(v1, v2);
 *
 * // For multiply-add operations, the API uses the same accumulation as the given accumulator (cannot be overriden)
 * auto acc2 = aie::mac(acc, v1, v2);
 *
 * @endcode
 */

// TODO: document table with default accumulator for each type combination and what combinations support user selection

/**
 * @defgroup group_bit Bits
 *
 * Bitwise logical operations.
 */

/**
 * @defgroup group_compare Comparison
 *
 * Vector comparison operations.
 */

/**
 * @defgroup group_reduce Reduction
 *
 * Vector reduction operations.
 */

/**
 * @defgroup group_reshape Reshaping
 *
 * AIE API provides operations to change the location of the elements within a vector and to combine the elements from
 * two or more vectors.
 */

/**
 * @defgroup group_fp_conversion Floating-point Conversion
 *
 * <table>
 * <caption>Supported float to fixed conversions</caption>
 * <tr><th>               <th>                 <th>          <th colspan=2> bfloat16                                                        <th colspan=2> float
 * <tr><th> Output bits   <th> Type            <th> Arch.    <th> Implementation                                <th> Notes                                        <th> Implementation           <th> Notes
 * <tr><td rowspan=4> 4b  <td rowspan=2>Scalar <td> AIE      <td>                                               <td>                                              <td>                          <td>
 * <tr>                                        <td> AIE-ML   <td> Runs on vector unit                           <td>                                              <td> Runs on vector unit      <td>
 * <tr>                   <td rowspan=2>Vector <td> AIE      <td>                                               <td>                                              <td>                          <td>
 * <tr>                                        <td> AIE-ML   <td> Emulated                                      <td> Uses @ref aie::rounding_mode::symmetric_zero <td> Emulated                 <td> Uses @ref aie::rounding_mode::symmetric_zero
 * <tr><td rowspan=4> 8b  <td rowspan=2>Scalar <td> AIE      <td>                                               <td>                                              <td> Native to 32b + cast     <td>
 * <tr>                                        <td> AIE-ML   <td> Runs on vector unit                           <td>                                              <td> Runs on vector unit      <td>
 * <tr>                   <td rowspan=2>Vector <td> AIE      <td>                                               <td>                                              <td> Element-wise scalar      <td>
 * <tr>                                        <td> AIE-ML   <td> Emulated                                      <td> Uses @ref aie::rounding_mode::symmetric_zero <td> Emulated                 <td> Uses @ref aie::rounding_mode::symmetric_zero
 * <tr><td rowspan=4> 16b <td rowspan=2>Scalar <td> AIE      <td>                                               <td>                                              <td> Native to 32b + cast     <td>
 * <tr>                                        <td> AIE-ML   <td> Runs on vector unit                           <td>                                              <td> Runs on vector unit      <td>
 * <tr>                   <td rowspan=2>Vector <td> AIE      <td>                                               <td>                                              <td> Vectorized emulated impl <td>
 * <tr>                                        <td> AIE-ML   <td> Native to 32b + extract lower 16b<sup>1</sup> <td> Uses @ref aie::rounding_mode::floor          <td> Emulated                 <td> Uses @ref aie::rounding_mode::symmetric_zero
 * <tr><td rowspan=4> 32b <td rowspan=2>Scalar <td> AIE      <td>                                               <td>                                              <td> Native                   <td>
 * <tr>                                        <td> AIE-ML   <td> Runs on vector unit                           <td>                                              <td> Runs on vector unit      <td>
 * <tr>                   <td rowspan=2>Vector <td> AIE      <td>                                               <td>                                              <td> Vectorized emulated impl <td>
 * <tr>                                        <td> AIE-ML   <td> Native                                        <td> Uses @ref aie::rounding_mode::floor          <td> Emulated                 <td> Uses @ref aie::rounding_mode::symmetric_zero
 * </table>
 *
 * \note
 * <sup>1</sup> - Unsigned conversions are emulated and use @ref aie::rounding_mode::symmetric_zero.<br/>
 *
 * <table>
 * <caption>Supported fixed to float conversions</caption>
 * <tr><th>Output type         <th>Type             <th>Arch.     <th>int4                    <th>int8                    <th>int16                   <th>int32
 * <tr><td rowspan=4>bfloat16  <td rowspan=2>Scalar <td> AIE      <td>                        <td>                        <td>                        <td>
 * <tr>                                             <td> AIE-ML   <td> Runs on vector unit    <td> Runs on vector unit    <td> Runs on vector unit    <td> Runs on vector unit
 * <tr>                        <td rowspan=2>Vector <td> AIE      <td>                        <td>                        <td>                        <td> 
 * <tr>                                             <td> AIE-ML   <td> Emulated               <td> Emulated               <td> Emulated               <td> Emulated
 * <tr><td rowspan=4>float     <td rowspan=2>Scalar <td> AIE      <td>                        <td> Cast to int32 + native <td> Cast to int32 + native <td> Native
 * <tr>                                             <td> AIE-ML   <td> Runs on vector unit    <td> Runs on vector unit    <td> Runs on vector unit    <td> Runs on vector unit
 * <tr>                        <td rowspan=2>Vector <td> AIE      <td>                        <td> Element-wise scalar    <td> Emulated               <td> Element-wise scalar
 * <tr>                                             <td> AIE-ML   <td> Emulated               <td> Emulated               <td> Emulated               <td> Emulated
 * </table>
 */

/**
 * @defgroup group_elementary Elementary Functions
 */

/**
 * @ingroup group_elementary
 * @defgroup group_fp_scalar Floating-point Scalar Operations
 */

/**
 * @defgroup group_mmul Matrix Multiplication
 *
 * The AIE API encapsulates the matrix multiplication functionality in the @ref aie::mmul class template. This class
 * template is parametrized with the matrix multiplication shape (MxKxN), the data types and, optionally, the
 * requested accmululation precision. The resulting class defines a function that performs the multiplication and
 * a data type for the result that can be converted to an accumulator/vector. The function interprets the input vectors
 * as matrices as described by the shape parameters.
 *
 * The following code snippet shows a portable sample blocked multiplication using the @ref aie::mmul class. The matrices are
 * assumed to be pre-tiled as defined by the mmul shape (MxK for A, KxN for B, and MxN for C).
 *
 * @code
 * template <unsigned M, unsigned K, unsigned N>
 * void mmul_blocked(unsigned rowA, unsigned colA, unsigned colB,
 *                   const int16 * __restrict pA, const int16 * __restrict pB, int16 * __restrict pC)
 * {
 *    using MMUL = aie::mmul<M, K, N, int16, int16>;
 *
 *    for (unsigned z = 0; z < rowA; z += 2) chess_loop_range(2,) {
 *        int16 * __restrict pC1 = pC + (      z * colB +       0) * MMUL::size_C;
 *        int16 * __restrict pC2 = pC + ((z + 1) * colB +       0) * MMUL::size_C;
 *
 *        for (unsigned j = 0; j < colB; j += 2) chess_loop_range(2,) {
 *            const int16 * __restrict pA1 = pA + (      z * colA +       0) * MMUL::size_A;
 *            const int16 * __restrict pA2 = pA + ((z + 1) * colA +       0) * MMUL::size_A;
 *            const int16 * __restrict pB1 = pB + (      0 * colB +       j) * MMUL::size_B;
 *            const int16 * __restrict pB2 = pB + (      0 * colB + (j + 1)) * MMUL::size_B;
 *
 *            aie::vector<int16, MMUL::size_A> A0 = aie::load_v<MMUL::size_A>(pA1); pA1 += MMUL::size_A;
 *            aie::vector<int16, MMUL::size_A> A1 = aie::load_v<MMUL::size_A>(pA2); pA2 += MMUL::size_A;
 *            aie::vector<int16, MMUL::size_B> B0 = aie::load_v<MMUL::size_B>(pB1); pB1 += MMUL::size_B * colB;
 *            aie::vector<int16, MMUL::size_B> B1 = aie::load_v<MMUL::size_B>(pB2); pB2 += MMUL::size_B * colB;
 *
 *            MMUL C00; C00.mul(A0, B0);
 *            MMUL C01; C01.mul(A0, B1);
 *            MMUL C10; C10.mul(A1, B0);
 *            MMUL C11; C11.mul(A1, B1);
 *
 *            for (unsigned i = 1; i < colA; ++i) chess_prepare_for_pipelining chess_loop_range(3,) {
 *                A0 = aie::load_v<MMUL::size_A>(pA1); pA1 += MMUL::size_A;
 *                A1 = aie::load_v<MMUL::size_A>(pA2); pA2 += MMUL::size_A;
 *                B0 = aie::load_v<MMUL::size_B>(pB1); pB1 += MMUL::size_B * colB;
 *                B1 = aie::load_v<MMUL::size_B>(pB2); pB2 += MMUL::size_B * colB;
 *
 *                C00.mac(A0, B0);
 *                C01.mac(A0, B1);
 *                C10.mac(A1, B0);
 *                C11.mac(A1, B1);
 *            }
 *
 *            aie::store_v(pC1, C00.template to_vector<int16>()); pC1 += MMUL::size_C;
 *            aie::store_v(pC1, C01.template to_vector<int16>()); pC1 += MMUL::size_C;
 *            aie::store_v(pC2, C10.template to_vector<int16>()); pC2 += MMUL::size_C;
 *            aie::store_v(pC2, C11.template to_vector<int16>()); pC2 += MMUL::size_C;
 *        }
 *    }
 * }
 *
 * @endcode
 */

/**
 * @defgroup group_fft Fast Fourier Transform (FFT)
 *
 * @subsection stage_based_fft Stage-based FFT APIs
 *
 * The AIE API offers a stage-based interface for carrying out decimation-in-time FFTs.
 * For example, assuming twiddle pointer visibility (see \ref twiddle_generation "Twiddle Generation" below),
 * a 1024 point FFT can be computed as follows:
 *
 * @code
 * void fft_1024pt(const cint16 * __restrict x,  // Input pointer
 *                 unsigned shift_tw,            // Indicates the decimal point of the twiddles
 *                                               // e.g. The twiddle 1.0+0.0i can be represented with cint16(32767, 0) and a shift_tw of 15
 *                 unsigned shift,               // Shift applied to apply to dit outputs
 *                 bool inv,                     // Run inverse FFT
 *                 cint16 * __restrict tmp,      // Scratch space for intermediate results
 *                 cint16 * __restrict y         // Output pointer
 *                )
 * {
 *     aie::fft_dit_r2_stage<512>(x,   tw1,   1024, shift_tw, shift, inv, tmp);
 *     aie::fft_dit_r2_stage<256>(tmp, tw2,   1024, shift_tw, shift, inv, y);
 *     aie::fft_dit_r2_stage<128>(y,   tw4,   1024, shift_tw, shift, inv, tmp);
 *     aie::fft_dit_r2_stage<64> (tmp, tw8,   1024, shift_tw, shift, inv, y);
 *     aie::fft_dit_r2_stage<32> (y,   tw16,  1024, shift_tw, shift, inv, tmp);
 *     aie::fft_dit_r2_stage<16> (tmp, tw32,  1024, shift_tw, shift, inv, y);
 *     aie::fft_dit_r2_stage<8>  (y,   tw64,  1024, shift_tw, shift, inv, tmp);
 *     aie::fft_dit_r2_stage<4>  (tmp, tw128, 1024, shift_tw, shift, inv, y);
 *     aie::fft_dit_r2_stage<2>  (y,   tw256, 1024, shift_tw, shift, inv, tmp);
 *     aie::fft_dit_r2_stage<1>  (tmp, tw512, 1024, shift_tw, shift, inv, y);
 * }
 * @endcode
 *
 * Similarly, a 512 point FFT can be implemented, using a mix of radix-2 and radix-4 stages, as follows:
 *
 * @code
 * void fft_512pt(const cint16 * __restrict x,  // Input pointer
 *                unsigned shift_tw,            // Indicates the decimal point of the twiddles
 *                                              // e.g. The twiddle 1.0+0.0i can be represented with cint16(32767, 0) and a shift_tw of 15
 *                unsigned shift,               // Shift applied to apply to dit outputs
 *                bool inv,                     // Run inverse FFT
 *                cint16 * __restrict tmp,      // Scratch space for intermediate results
 *                cint16 * __restrict y         // Output pointer
 *                )
 * {
 *     aie::fft_dit_r2_stage<256>(x,   tw1,                     512, shift_tw, shift, inv, y);
 *     aie::fft_dit_r4_stage<64> (y,   tw2,   tw4,   tw2_4,     512, shift_tw, shift, inv, tmp);
 *     aie::fft_dit_r4_stage<16> (tmp, tw8,   tw16,  tw8_16,    512, shift_tw, shift, inv, y);
 *     aie::fft_dit_r4_stage<4>  (y,   tw32,  tw64,  tw32_64,   512, shift_tw, shift, inv, tmp);
 *     aie::fft_dit_r4_stage<1>  (tmp, tw128, tw256, tw128_256, 512, shift_tw, shift, inv, y);
 * }
 * @endcode
 *
 * \note For an odd number of stages the input buffer may be used in place of the `tmp`, which could be of benefit for large FFTs.
 *
 * \note The order of the twiddle arguments are outlined in the description of each FFT stage function:
 * \ref aie::fft_dit_r2_stage, \ref aie::fft_dit_r3_stage, \ref aie::fft_dit_r4_stage, \ref aie::fft_dit_r5_stage
 *
 *
 * @subsection twiddle_generation Twiddle Generation
 *
 * An R-Radix, N-point FFT requires R-1 twiddle tables per stage.
 *
 * Each of the tables are of length `(n_stage / R)`, where `n_stage` is the local number of samples of the current radix stage.
 * The local number of samples is given as the total point size, N, divided by the Vectorization, which is the template parameter of the
 * `fft_dit_r*_stage` function calls. This is due to the fact that earlier stages of an N-point FFT are smaller, batched FFTs.
 *
 * For each stage, the twiddle tables can be computed, in floating point, as:
 *
 * @code
 * int n_stage = N / Vectorization;
 * int n_tws   = n_stage / Radix;
 * for (unsigned r = 1; r < Radix; ++r) {
 *     for (unsigned i = 0; i < n_tws; ++i) {
 *         tw[r-1][i] = exp(-2j * pi * r * i / n);
 *     }
 * }
 * @endcode
 *
 * and the equivalent python code:
 *
 * @code
 * import numpy as np
 *
 * def tw(n, radix, vec):
 *     n_stage = n / vec
 *     points = n_stage / radix
 *     return np.exp(-2j * np.pi * np.arange(1, radix).reshape(-1,1) * np.arange(0, points) / n_stage)
 * @endcode
 *
 *
 * For fixed point implementations, the twiddle values should be multiplied by `(1 << shift_tw)` before converting to the output type.
 * For example,
 *
 * @code
 * template <typename TR, typename T>
 * TR convert_twiddle_to_fixed_point(T val, unsigned shift_tw) {
 *     return aie::to_fixed<TR>(val * (1 << shift_tw));
 * }
 *
 * // Required to prevent overflow on conversions
 * aie::set_rounding(aie::rounding_mode::positive_inf);
 * aie::set_saturation(aie::saturation_mode::saturate);
 *
 * unsigned shift_tw = 15;
 * cfloat tw = cfloat(1.0f, 0.0f);
 * cint16 tw_fixed = convert_twiddle_to_fixed_point<cint16>(tw, shift_tw);
 * // tw_fixed = 32767 + 0j
 * @endcode
 *
 * @subsection fft_example Full FFT Example
 *
 * Using the method of generating twiddles outlined in \ref twiddle_generation, a 128pt FFT can be computed as follows:
 *
 * \note Note the order of the twiddles to the radix 4 stages are not in increasing order; i.e. defining the rotation rate of
 * a given twiddle to be `w(tw)`, the relationship between the twiddle groups are `w(tw1) < w(tw0) < w(tw2)`.
 * This is not the case for other radix stages, where the rotation rate is monotonically increasing; i.e. `w(tw1) < ... < w(twN)`.
 *
 * @code
 * constexpr unsigned        n = 128;
 * constexpr unsigned shift_tw = 15;
 * constexpr unsigned    shift = 15;
 * constexpr bool          inv = false;
 *
 * alignas(aie::vector_decl_align) static cint32 x[n];
 * alignas(aie::vector_decl_align) static cint32 tmp[n];
 * alignas(aie::vector_decl_align) static cint32 y[n];
 * 
 * alignas(aie::vector_decl_align) static cint16 tw1    [] = {{ 32767,      0}};
 * alignas(aie::vector_decl_align) static cint16 tw2    [] = {{ 32767,      0}, {     0, -32768}};
 * alignas(aie::vector_decl_align) static cint16 tw4    [] = {{ 32767,      0}, { 23170, -23170}};
 * alignas(aie::vector_decl_align) static cint16 tw2_4  [] = {{ 32767,      0}, {-23170, -23170}};
 * alignas(aie::vector_decl_align) static cint16 tw8    [] = {{ 32767,      0}, { 30273, -12539}, { 23170, -23170}, { 12539, -30273},
 *                                                            {     0, -32768}, {-12539, -30273}, {-23170, -23170}, {-30273, -12539}};
 * alignas(aie::vector_decl_align) static cint16 tw16   [] = {{ 32767,      0}, { 32138,  -6392}, { 30273, -12539}, { 27245, -18204},
 *                                                            { 23170, -23170}, { 18204, -27245}, { 12539, -30273}, {  6392, -32138}};
 * alignas(aie::vector_decl_align) static cint16 tw8_16 [] = {{ 32767,      0}, { 27245, -18204}, { 12539, -30273}, { -6392, -32138},
 *                                                            {-23170, -23170}, {-32138,  -6392}, {-30273,  12539}, {-18204,  27245}};
 * alignas(aie::vector_decl_align) static cint16 tw32   [] = {{ 32767,      0}, { 32610,  -3211}, { 32138,  -6392}, { 31357,  -9512},
 *                                                            { 30273, -12539}, { 28898, -15446}, { 27245, -18204}, { 25330, -20787},
 *                                                            { 23170, -23170}, { 20787, -25330}, { 18204, -27245}, { 15446, -28898},
 *                                                            { 12539, -30273}, {  9512, -31357}, {  6392, -32138}, {  3211, -32610},
 *                                                            {     0, -32768}, { -3211, -32610}, { -6392, -32138}, { -9512, -31357},
 *                                                            {-12539, -30273}, {-15446, -28898}, {-18204, -27245}, {-20787, -25330},
 *                                                            {-23170, -23170}, {-25330, -20787}, {-27245, -18204}, {-28898, -15446},
 *                                                            {-30273, -12539}, {-31357,  -9512}, {-32138,  -6392}, {-32610,  -3211}};
 * alignas(aie::vector_decl_align) static cint16 tw64   [] = {{ 32767,      0}, { 32728,  -1607}, { 32610,  -3211}, { 32413,  -4808},
 *                                                            { 32138,  -6392}, { 31785,  -7961}, { 31357,  -9512}, { 30852, -11039},
 *                                                            { 30273, -12539}, { 29621, -14010}, { 28898, -15446}, { 28106, -16846},
 *                                                            { 27245, -18204}, { 26319, -19519}, { 25330, -20787}, { 24279, -22005},
 *                                                            { 23170, -23170}, { 22005, -24279}, { 20787, -25330}, { 19519, -26319},
 *                                                            { 18204, -27245}, { 16846, -28106}, { 15446, -28898}, { 14010, -29621},
 *                                                            { 12539, -30273}, { 11039, -30852}, {  9512, -31357}, {  7961, -31785},
 *                                                            {  6392, -32138}, {  4808, -32413}, {  3211, -32610}, {  1607, -32728}};
 * alignas(aie::vector_decl_align) static cint16 tw32_64[] = {{ 32767,      0}, { 32413,  -4808}, { 31357,  -9512}, { 29621, -14010},
 *                                                            { 27245, -18204}, { 24279, -22005}, { 20787, -25330}, { 16846, -28106},
 *                                                            { 12539, -30273}, {  7961, -31785}, {  3211, -32610}, { -1607, -32728},
 *                                                            { -6392, -32138}, {-11039, -30852}, {-15446, -28898}, {-19519, -26319},
 *                                                            {-23170, -23170}, {-26319, -19519}, {-28898, -15446}, {-30852, -11039},
 *                                                            {-32138,  -6392}, {-32728,  -1607}, {-32610,   3211}, {-31785,   7961},
 *                                                            {-30273,  12539}, {-28106,  16846}, {-25330,  20787}, {-22005,  24279},
 *                                                            {-18204,  27245}, {-14010,  29621}, { -9512,  31357}, { -4808,  32413}};
 *
 * // Constant value input
 * for (unsigned i = 0; i < n; ++i) {
 *     x[i] = cint32(1, 0);
 * }
 *
 * aie::fft_dit_r2_stage<64>(x,   tw1,                   n, shift_tw, shift, inv, tmp);
 * aie::fft_dit_r4_stage<16>(tmp, tw2,   tw4,   tw2_4,   n, shift_tw, shift, inv, y);
 * aie::fft_dit_r4_stage<4> (y,   tw8,   tw16,  tw8_16,  n, shift_tw, shift, inv, tmp);
 * aie::fft_dit_r4_stage<1> (tmp, tw32,  tw64,  tw32_64, n, shift_tw, shift, inv, y);
 *
 * for (unsigned i = 0; i < n; ++i) {
 *     printf("{%d, %d} ", y[i].real, y[i].imag);
 * }
 * // Will print:
 * // {128, 0} {0, 0} {0, 0} ... {0, 0}
 *
 * @endcode
 *
 */

/**
 * @defgroup group_mul_special Special Multiplications
 *
 * AIE provides hardware support to accelerate special multiplications that can be used to accelerate specific
 * application use cases like (but not limited to) signal processing.
 */

/**
 * @defgroup group_lut Lookup Tables
 *
 * \note
 * Lookup table functionality is only available from AIE-ML
 *
 * Two abstractions are provided to represent lookup tables on AIE architectures:
 *
 * 1. \ref aie::parallel_lookup which provides a direct lookup
 * 2. \ref aie::linear_approx which provides a linear approximation for non-linear functions
 *
 * The primary purpose of these abstractions is to leverage hardware support for parallel accesses on certain AIE architectures.
 *
 * Both of these abstractions are built upon the \ref aie::lut type that is used to encapsulate the raw LUT data.
 * This encapsulation is implemented in an attempt to ensure correct data layout for a given lookup type.
 * Specifically, to achieve a given level of access parallelism, the LUT values are required to have a specific layout in memory,
 * which is dependent on the required number of parallel loads.
 * For details on the memory layout requirements, see the \ref aie::lut documentation.
 *
 * Example implementations of parallel lookup and linear approximation functions are given below:
 *
 * @code
 * template <typename Value>
 * void parallel_lookup(const int8* pIn, Value* pOut, const aie::lut<4, Value>& my_lut,
 *                      int samples, int step_bits, int bias, int LUT_elems)
 * {
 *     aie::parallel_lookup<int8, aie::lut<4, Value>> lookup(my_lut, step_bits, bias);
 *
 *     auto it_in  = aie::begin_vector<32>(pIn);
 *     auto it_out = aie::begin_vector<32>(pOut);
 *
 *     for (unsigned l = 0; l < samples / 32; ++l)
 *         *it_out++ = lookup.fetch(*it_in++);
 * }
 *
 * template <typename OffsetType, typename SlopeType>
 * void linear_approx(const int8* pIn, OffsetType* pOut, const aie::lut<4, OffsetType, SlopeType>& my_lut,
 *                    int samples, int step_bits, int bias, int LUT_elems, int shift_offset, int shift_out)
 * {
 *     aie::linear_approx<int8, aie::lut<4, OffsetType, SlopeType>> lin_approx(my_lut, step_bits, bias, shift_offset);
 *
 *     auto it_in  = aie::begin_vector<32>(pIn);
 *     auto it_out = aie::begin_vector<32>(pOut);
 *
 *     for (unsigned l = 0; l < samples / 32; ++l)
 *         *it_out++ = lin_approx.compute(*it_in++).to_vector<LUT_T>(shift_out);
 * }
 * @endcode
 *
 */

/**
 * @defgroup group_operators Operator Overloading
 *
 * The AIE API provides overloading for most of the available operators. In order to use them, you need to include a
 * special header file and use the aie::operators namespace.
 *
 * @code
 *
 * #include <aie_api/operators.hpp>
 *
 * using namespace aie::operators;
 *
 * aie::mask<16> less_than_add(aie::vector<int32, 16> a, aie::vector<int32, 16> b, aie::vector<int32, 16> c)
 * {
 *     return c < (a + b);
 * }
 *
 * @endcode
 */

/**
 * @defgroup group_adf Interoperability with Adaptive Data Flow (ADF) Graph Abstractions
 *
 * ADF graphs use data flow abstractions to read input data and write output data. AIE API extends such abstractions
 * to work with its @ref aie::vector and @ref aie::accum data types.
 *
 * \note ADF abstractions related to accumulator types require that the accumulator tags are natively supported on the target architecture
 * as the graph does not support non-native accumulator types. See \ref accum_valid_parameters "Valid Accumulator Parameters" for natively supported accum tags.
 */

#endif
