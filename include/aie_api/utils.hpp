// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

/**
 * @file
 * @brief Utilities exposed to users.
 */

/**
 * @defgroup group_utility_functions Uility functions and classes
 */

#pragma once

#ifndef __AIE_API_UTILS__HPP__
#define __AIE_API_UTILS__HPP__

// AIE API currently requires C++20
#define AIE_API_CXX_VERSION 202002L

// TODO: When host/graph compiler is updated to C++20 we will need to change this logic
#if __cplusplus < AIE_API_CXX_VERSION

#ifdef __chess__

#error "C++20 or greater is required to compile AIE API kernels"

#endif

#else

#include <algorithm>
#include <cstdio>
#include <cmath>
#include <limits>
#include <utility>

#include "vector.hpp"
#include "tile.hpp"

namespace aie {

namespace detail {

// An accumulator that stores overlapping ranges of bits
// It requires SRS to add the overlapping bits before shifting, so it
// is not trivial to inspect its contents in memory directly.
template <typename A>
concept OverlappingAcc = is_accum_v<A> && !A::is_floating_point() && A::accum_bits() > 64;

// An accumulator that does not have overlapping ranges of bits
// We can inspect the contents of the accumulator from memory directly
template <typename A>
concept RegularAcc = is_accum_v<A> && !OverlappingAcc<A>;

template <typename T, unsigned Elems>
concept ValidVector = requires { typename native_vector_type_t<T, Elems>; };

// Retrieves the largest size less than or equal the current size that would
// yield a valid aie::vector for type T. If no such vector would exist it returns 0
template <typename T, unsigned Elems>
constexpr unsigned largest_valid_vector_size()
{
    if constexpr (ValidVector<T, Elems>)
        return Elems;
    else if (Elems > 0)
        return largest_valid_vector_size<T, Elems / 2>();
    UNREACHABLE_MSG("No valid vector found");
}

// Interprets an integer as fixed point number and converts it into a floating point number.
template <typename T>
    requires(detail::is_integral_v<T>)
__aie_inline
static double to_floating_point(T value, int scale)
{
    constexpr size_t double_precision = std::numeric_limits<double>::digits;
    if constexpr (detail::type_bits_v<T> > double_precision)
        chess_warning("Type conversion loses precision");
    return std::ldexp(double(value), scale);
}

template <typename T>
__aie_inline
void print_value(const T &e, int scale = 0)
{
    if constexpr (detail::is_complex_v<T>) {
        if constexpr (detail::is_floating_point_v<T>)
            printf("(%f %f) ", (float)e.real, (float)e.imag);
        else if (scale == 0) // Integral
            printf("(%d %d) ", (int)e.real, (int)e.imag);
        else
            printf("(%f %f) ", to_floating_point(e.real, scale), to_floating_point(e.imag, scale));
    }
    else if constexpr (detail::is_floating_point_v<T>) {
        printf("%f ", (float)e);
    }
    else if (scale != 0) {
        printf("%f ", to_floating_point(e, scale));
    }
    else if constexpr (detail::is_signed_v<T>) {
        printf("%d ", (int)e);
    }
    else {
        printf("%u ", (unsigned)e);
    }
}

template <typename T, unsigned Elems>
__aie_inline
void print_elem(const vector<T, Elems> &v, unsigned i)
{
    print_value(T(v[i]));
}

template <RegularAcc Acc>
    requires(Acc::is_floating_point())
__aie_inline
void print_elem(const Acc &acc, unsigned i) {
#if AIE_API_MATH_VERSION >= 100 || __AIE_API_COMPLEX_FP32_EMULATION__
    if constexpr (Acc::is_complex()) {
        const cfloat *ptr = ((const cfloat *)&acc) + i;
        const cfloat e = *ptr;

        detail::print_value(e);
    }
    else
#endif
    {
        const float *ptr = ((const float *)&acc) + i;
        const float e = *ptr;

        detail::print_value(e);
    }
}

// Prints the hex representation of an accumulator's element

// - Helper function used by both integer and complex representations
template <RegularAcc Acc, unsigned memory_bits>
    requires(!Acc::is_floating_point())
__aie_inline
void print_int_component(const char *acc_ptr, const char *prefix = "", const char *suffix = "")
{
    constexpr unsigned representable_bytes = Acc::accum_min_bits() / 8;
    printf("%s0x", prefix);
    for (unsigned j = representable_bytes; j > 0; --j) {
        printf("%02hhx", acc_ptr[j-1]);
    }
    printf("%s", suffix);
}

// - Prints hex representation of an integer element
template <RegularAcc Acc>
    requires(!Acc::is_floating_point() && !Acc::is_complex())
__aie_inline
void print_elem(const Acc &acc, unsigned i)
{
    constexpr unsigned component_bits = Acc::memory_bits();
    const char *ptr = ((const char *)&acc) + i * acc.memory_bits() / 8;
    print_int_component<Acc, component_bits>(ptr, "", " ");
}

// - Prints hex representation of a complex element
template <RegularAcc Acc>
    requires(!Acc::is_floating_point() && Acc::is_complex())
__aie_inline
void print_elem(const Acc &acc, unsigned i)
{
    constexpr unsigned complex_size = Acc::memory_bits() / 8;
    constexpr unsigned component_bits = Acc::memory_bits() / 2;
    const char *real = ((const char *)&acc) + i * complex_size;
    const char *imag = real + component_bits / 8;

    print_int_component<Acc, component_bits>(real, "(", ", ");
    print_int_component<Acc, component_bits>(imag, "", ") ");
}

// Retrieves a sub accumulator and the element index referring to acc[i] that
// can be converted into a vector
// This is because there may not be vectors with the same capacity
template <Accum Acc>
    requires(!Acc::is_floating_point())
__aie_inline
auto get_sub_accum(const Acc &acc, unsigned i)
{
    constexpr unsigned sub_acc_size = largest_valid_vector_size<int32, Acc::size()>();
    if constexpr (sub_acc_size == Acc::size())
        return std::pair{acc, i};

    const unsigned bucket = i / sub_acc_size;
    const unsigned sub_i = i % sub_acc_size;
    const auto sub_acc = acc.template extract<sub_acc_size>(bucket);
    return std::pair{sub_acc, sub_i};
}

// Prints the hex representation of an integer accumulator's element
template <OverlappingAcc Acc>
    requires(!Acc::is_complex())
__aie_inline
void print_elem(const Acc &acc, unsigned i, const char *prefix = "", const char *suffix = " ")
{
    auto [sub_acc, sub_i] = get_sub_accum(acc, i);
    int32 a = Acc::accum_min_bits() <= 64
        ? 0
        : sub_acc.template to_vector<int32>(56)[sub_i];
    int32 b = sub_acc.template to_vector<int32>(32)[sub_i];
    int32 c = sub_acc.template to_vector<int32>(0)[sub_i];

    // For 56b and 64b, ignore 'a' argument.
    // Mask is used to ensure exact number of bits are printed in MSB slots.
    struct Pattern {
        const char *format = nullptr;
        uint32_t mask_a, mask_b;
    };
    constexpr Pattern pattern = []() -> Pattern {
        switch (Acc::accum_min_bits()) {
            case 56: return {"%s0x%06x%08x%s",     0,          0x00ffffff};
            case 64: return {"%s0x%08x%08x%s",     0,          0xffffffff};
            case 72: return {"%s0x%04x%06x%08x%s", 0x0000ffff, 0x00ffffff};
            case 80: return {"%s0x%06x%06x%08x%s", 0x00ffffff, 0x00ffffff};
            default: UNREACHABLE_MSG("Unsupported accum bits");
        }
    }();
    if constexpr (pattern.mask_a == 0)
        printf(pattern.format, prefix, pattern.mask_b & b, c, suffix);
    else
        printf(pattern.format, prefix, pattern.mask_a & a, pattern.mask_b & b, c, suffix);
}

// Prints the hex representation of an complex accumulator's element
template <OverlappingAcc Acc>
    requires(Acc::is_complex())
__aie_inline
void print_elem(const Acc &acc, unsigned i)
{
    // Remove complex tag to access its components like a regular accumulator
    using new_tag = accum_tag_t<AccumClass::Int, Acc::accum_bits()>;
    auto components = accum_cast<new_tag>(acc);

    print_elem(components, 2 * i, "(", " ");
    print_elem(components, 2 * i + 1, "", ") ");
}

} // namespace detail

/**
 * @defgroup group_utility_print Print functions
 * @ingroup group_utility_functions
 *
 * These functions provide an abstraction on top of printf that allow users to
 * display the contents of AIE types in the standard output.
 */

/**
 * @ingroup group_utility_print
 *
 * Displays the contents of a vector or an accumulator.
 * \param[in] c      A vector or an accumulator.
 * \param[in] nl     Set to `true` to print add newline at the end.
 * \param[in] prefix Optional text to print before the first element.
 */
template <typename C>
    requires(Vector<C> || Accum<C>)
__aie_noinline
void print(const C &c, bool nl = false, const char *prefix = nullptr)
{
    if (prefix)
        printf("%s", prefix);

    aie::rounding_mode rnd;
    if constexpr (detail::OverlappingAcc<C>) {
        rnd = aie::tile::current().get_rounding();
        aie::tile::current().set_rounding(aie::rounding_mode::floor);
    }

    for (unsigned i = 0; i < C::size(); ++i) {
        detail::print_elem(c, i);
    }

    if (nl)
        printf("\n");

    if constexpr (detail::OverlappingAcc<C>) {
        aie::tile::current().set_rounding(rnd);
    }
}

/**
 * @ingroup group_utility_print
 *
 * Displays the contents of a vector integers represented as real numbers in fixed point.
 * Applies a scaling factor to each value before printing, which represents the exponent in a multiplication by a power
 * of two: positive scale values will make the numbers larger while negative values will make the numbers smaller.
 * \param[in] v      The vector.
 * \param[in] scale  Power-of-two exponent to apply to each value. Can be positive or negative.
 * \param[in] nl     Set to `true` to print a newline after the last element.
 * \param[in] prefix Optional text to print before the first element.
 */
template <typename T, unsigned Elems>
    requires(!detail::is_floating_point_v<T>)
__aie_noinline
void print_fixed(const aie::vector<T, Elems> &v, int scale, bool nl = false, const char *prefix = nullptr)
{
    if (prefix)
        printf("%s", prefix);

    for (unsigned i = 0; i < Elems; ++i) {
        T e = v[i];
        detail::print_value(e, scale);
    }

    if (nl)
        printf("\n");
}

/**
 * @ingroup group_utility_print
 *
 * Displays the contents of a vector or accumulator, arranged in a matrix layout.
 * \param[in] m       A vector or an accumulator.
 * \param[in] cols    The number of columns in the matrix. Must be an exact divisor of the number of elements in `m`.
 * \param[in] prefix  Optional text to print before the first element.
 */
template <typename Matrix>
    requires(Vector<Matrix> || Accum<Matrix>)
__aie_noinline
void print_matrix(const Matrix &m, unsigned cols, const char *prefix = nullptr)
{
    const unsigned rows = Matrix::size() / cols;
    REQUIRES_MSG(rows > 0, "The number of columns must be, at most, the number of elements in the Vector");
    REQUIRES_MSG(cols > 0 && (cols & (cols - 1)) == 0, "The number of columns must be a power of two");

    unsigned n_white = 0;
    if (prefix) {
        printf("%s", prefix);
        n_white = strlen(prefix);
    }

    aie::rounding_mode rnd;
    if constexpr (detail::OverlappingAcc<Matrix>) {
        rnd = aie::tile::current().get_rounding();
        aie::tile::current().set_rounding(aie::rounding_mode::floor);
    }

    for (unsigned i = 0; i < rows; ++i) {
        for (unsigned j = 0; j < cols; ++j) {
            detail::print_elem(m, i * cols + j);
        }

        printf("\n");

        if (i < rows - 1)
            printf("%*c", n_white, ' ');
    }

    if constexpr (detail::OverlappingAcc<Matrix>) {
        aie::tile::current().set_rounding(rnd);
    }
}

#if AIE_API_ML_VERSION >= 210

/**
 * @ingroup group_utility_print
 *
 * Displays the contents of a block floating point vector.
 * \param[in] v       The vector.
 * \param[in] nl      Set to `true` in order to add a newline at the end.
 * \param[in] prefix  Optional text to print before the first element.
 */
template <typename T, unsigned Elems>
__aie_noinline
void print(const aie::block_vector<T, Elems> &v, bool nl = false, const char *prefix = nullptr)
{
    print(aie::accum<accfloat, Elems>(v) , nl, prefix);
}

/**
 * @ingroup group_utility_print
 *
 * Displays the contents of a vector or accumulator, arranged in a matrix layout.
 * \param[in] v       The vector.
 * \param[in] cols    The number of columns in the matrix. Must be an exact divisor of the number of elements in `m`.
 * \param[in] prefix  Optional text to print before the first element.
 */
template <typename T, unsigned Elems>
__aie_noinline
void print_matrix(const aie::block_vector<T, Elems> &v, unsigned cols, const char *prefix = nullptr)
{
    print_matrix(aie::accum<accfloat, Elems>(v), cols, prefix);
}

#endif

/**
 * @ingroup group_utility_print
 *
 * Displays the contents of a mask.
 * \param[in] m       The mask.
 * \param[in] nl      Set to `true` in order to add a newline at the end.
 * \param[in] prefix  Optional text to print before the first element.
 */
template <unsigned N>
__aie_noinline
void print(const aie::mask<N> &m, bool nl = false, const char *prefix = nullptr)
{
    if (prefix)
        printf("%s", prefix);

    for (unsigned i = 0; i < N; ++i) {
        bool b = m.test(i);

        printf("%d ", b? 1 : 0);
    }

    if (nl)
        printf("\n");
}

using detail::utils::circular_index;

/**
 * @defgroup group_utility_loops Loop functions
 * @ingroup group_utility_functions
 *
 */

/**
 * @defgroup group_utility_unroll Loop unrolling
 * @ingroup group_utility_loops
 *
 * These functions allow users to explicitly unroll the body of a loop.
 * The simplest way to use them is to provide a lambda expression with the loop body. For example:
 *
 * @code{.cpp}
 * aie::vector<int, 16> a;
 * aie::unroll_for<int, 0, 16>([](int i) __aie_inline {
 *    a.push(i);
 * });
 * @endcode
 *
 * It is recommended that any functions or function-like objects used are marked with the `always_inline` attribute.
 * There are different ways you can specify this:
 *
 * 1. Preprocessor macro: `__aie_inline`
 * 2. GNU attribute syntax: `__attribute__((always_inline))`
 * 3. C++11 attribute syntax: the attribute is not C++ standard but it is supported in most common compilers: `[[gnu::always_inline]]`, `[[clang::always_inline]]`.
 *
 * References:
 * - https://clang.llvm.org/docs/AttributeReference.html#always-inline-force-inline
 * - https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-always_005finline-function-attribute
 */

/**
 * @ingroup group_utility_unroll
 *
 * @brief Invokes a function object as many times as specified by a linear index sequence.
 *
 * @tparam T     Type of the sequence values passed to the function.
 * @tparam Start First value in the sequence.
 * @tparam End   Upper limit of the sequence.
 * @tparam Step  Distance between two consecutive values in the sequence.
 * @param fn     \parblock
 *               A function object that takes a sequence value by argument. The signature of the function should be
 *               equivalent to one of the following:
 *
 *               @code{.cpp}
 *               void fun(const Type &i);
 *               void fun();
 *               @endcode
 *
 *               The signature does not need to have `const &` and the type `Type` must be either `T` or implicitly
 *               convertible from `T`.
 *               \endparblock
 */
template <typename T, T Start, T End, T Step =1, typename Fn>
__aie_inline
constexpr void unroll_for(Fn &&fn)
{
    return detail::utils::unroll_for<T, Start, End, Step>(std::forward<Fn>(fn));
}

/**
 * @ingroup group_utility_unroll
 *
 * @brief Invokes a function object as many times as defined by a 2D index sequence. 
 *
 * @tparam T      Type of the sequence values passed to the function.
 * @tparam StartY First value in the sequence (y-axis).
 * @tparam EndY   Upper limit of the sequence(y-axis).
 * @tparam StepY  Distance between two consecutive values in the sequence (y-axis).
 * @tparam StartX First value in the sequence (x-axis).
 * @tparam EndX   Upper limit of the sequence (x-axis).
 * @tparam StepX  Distance between two consecutive values in the sequence (x-axis).
 * @param fn      \parblock
 *                A function object that takes two values of type `T` by argument, one per dimension. The signature of
 *                the function should be equivalent to one the following:
 *
 *                @code{.cpp}
 *                void fun(const Type &y, const Type &x);
 *                void fun();
 *                @endcode
 *
 *                The signature does not need to have `const &` and the type `Type` must be either `T` or implicitly
 *                convertible from `T`.
 *                \endparblock
 */
template <typename T, T StartY, T EndY, T StepY, T StartX, T EndX, T StepX, typename Fn>
__aie_inline
constexpr void unroll_for_2d(Fn &&fn)
{
    return detail::utils::unroll_for_2d<T, StartY, EndY, StepY, StartX, EndX, StepX>(std::forward<Fn>(fn));
}

/**
 * @ingroup group_utility_unroll
 *
 * @brief Invokes a function object a given number of times. The function takes the current count by argument.
 *
 * @tparam Times Total number of function calls.
 * @param fn     \parblock
 *               A function object that takes the current count by argument.  The signature of the function should be
 *               equivalent to one of the following:
 *
 *               @code{.cpp}
 *               void fun(const Type &i);
 *               void fun();
 *               @endcode
 *
 *               The signature does not need to have `const &` and the type `Type` must be either `unsigned` or
 *               implicitly convertible from it.
 *               \endparblock
 */
template <unsigned Times, typename Fn>
__aie_inline
constexpr void unroll_times(Fn &&fn)
{
    unroll_for<unsigned, 0, Times, 1>(std::forward<Fn>(fn));
}

/**
 * @ingroup group_utility_unroll
 *
 * @brief Invokes a function object as many times as defined by a 2D index sequence.
 *
 * @tparam TimesY Total number of function calls (y-axis).
 * @tparam TimesX Total number of function calls (x-axis).
 * @param fn      \parblock
 *                A function object that takes two values by argument, representing the current count for each of the
 *                dimensions. The signature of the function should be equivalent to one of the following:
 *
 *                @code{.cpp}
 *                void fun(const Type &y, const Type &x);
 *                void fun();
 *                @endcode
 *
 *                The signature does not need to have `const &` and the type `Type` must be either `unsigned` or
 *                implicitly convertible from it.
 *                \endparblock
 */
template <unsigned TimesY, unsigned TimesX, typename Fn>
__aie_inline
constexpr void unroll_times_2d(Fn &&fn)
{
    unroll_for_2d<unsigned, 0, TimesY, 1, 0, TimesX, 1>(std::forward<Fn>(fn));
}

/**
 * @defgroup group_utility_pipelining Loop pipelining
 * @ingroup group_utility_loops
 *
 * These functions provide the ability to finely control the pipelining of a loop.
 */

/**
  * @ingroup group_utility_pipelining
  *
  * @brief A structure containing options related to loop iteration peeling
  *
  * To be used with \ref aie::pipelined_loop
  */
struct loop_options
{
    /** @brief The number of iterations to peel at the front of the loop*/
    unsigned peel_front = 0;

    /** @brief The number of iterations to peel at the back of the loop*/
    unsigned peel_back  = 0;

    /** @brief Adjust the preamble */
    int preamble_offset = 0;
};

using LoopOptions [[deprecated("Use loop_options instead")]] = loop_options;

// Base template for peeling index structure
template <unsigned peel_front, unsigned peel_back>
struct peel_idx {
    // Indicates if the current iteration is in the front peeled section
    constexpr bool in_front() { return false; }

    // Indicates if the current iteration is in the main loop
    constexpr bool in_loop()  { return false; }

    // Indicates if the current iteration is in the back peeled section
    constexpr bool in_back()  { return false; }

    // Indicates if the current iteration is the first iteration in the front peeled section
    constexpr bool first_iter() requires(peel_front > 0) { 
        return false; 
    }

    // Indicates if the current iteration is the last iteration in the back peeled section
    constexpr bool last_iter() requires(peel_back > 0) { 
        return false; 
    }
};

// Specialization for front peeled iterations
template<unsigned peel_front, unsigned peel_back, unsigned N> 
struct front_idx : peel_idx<peel_front, peel_back> {
    // Indicates that the current iteration is in the front peeled section
    constexpr bool in_front() { return true; }

    // Indicates if the current iteration is the first iteration in the front peeled section
    constexpr bool first_iter() { return (N == 0); }

    // Allows implicit conversion to an integer representing the iteration index
    constexpr operator int() { return N; }
};

// Specialization for main loop iterations
template <unsigned peel_front = 0, unsigned peel_back = 0>
struct loop_idx : peel_idx<peel_front, peel_back> {
    // Constructor to initialize the iteration index
    loop_idx(unsigned n_) : n(n_) {}

    // Indicates that the current iteration is in the main loop
    constexpr bool in_loop() { return true; }

    // Allows implicit conversion to an integer representing the iteration index
    operator int() { return n; }

private:
    unsigned n; // Stores the current iteration index
};

// Specialization for back peeled iterations
template<unsigned peel_front, unsigned peel_back, unsigned BACK_N> 
struct back_idx : peel_idx<peel_front, peel_back> {
    // Constructor to initialize the iteration index
    back_idx(unsigned n_) : n(n_) {}

    // Indicates that the current iteration is in the back peeled section
    constexpr bool in_back() { return true; }

    // Allows implicit conversion to an integer representing the iteration index
    operator int() { return n; }

    // Indicates if the current iteration is the last iteration in the back peeled section
    constexpr bool last_iter()  { return (BACK_N == 0); }

private:
    unsigned n; // Stores the current iteration index
};

// Implementation of the peeled/pipelined loop
template<unsigned MinIters, loop_options Opts = loop_options{}, bool use_pipelining, typename Fn>
__aie_inline
void peeled_pipelined_loop(unsigned count, Fn &&fn)
{
    constexpr unsigned total_peel_iters = Opts.peel_front + Opts.peel_back;

    static_assert(total_peel_iters <= MinIters, "Requested peeling exceeds loop range");

    REQUIRES_MSG(count >= total_peel_iters, "Cannot peel more iterations than the loop will be executed");

    if constexpr(MinIters > 1)
    {
        constexpr unsigned peel_front = Opts.peel_front;
        constexpr unsigned peel_back  = Opts.peel_back;

        if constexpr(peel_front > 0)
        {
            unroll_times<peel_front>([&](auto i) __aie_inline {
                fn(front_idx<peel_front, peel_back, i>{});
            });
            chess_separator();
        }

        if constexpr(use_pipelining) {
#if !AIE_API_NATIVE
            [[using chess: prepare_for_pipelining,
                        min_loop_count(MinIters - total_peel_iters),
                        pipeline_adjust_preamble(Opts.preamble_offset)]]
#endif
            for (unsigned i = peel_front; i < count - peel_back; i++)
            {
                fn(loop_idx<peel_front, peel_back>{i});
            }
        } else {
            for (unsigned i = peel_front; i < count - peel_back; i++)
            {
                fn(loop_idx<peel_front, peel_back>{i});
            }
        }

        if constexpr(peel_back > 0)
        {
            chess_separator();
            unroll_times<peel_back>([&](auto i) __aie_inline {
                fn(back_idx<peel_front, peel_back, peel_back - 1 - i>{count - peel_back + i});
            });
        }
    }
    else
    {
#if !AIE_API_NATIVE
        [[using chess: min_loop_count(MinIters)]]
#endif
        for (unsigned i = 0; i < count; i++) {
            fn(loop_idx{i});
        }
    }
}

/**
 * @ingroup group_utility_pipelining
 *
 * @brief Invokes a function object a given number of times.
 *        The pipelining can be controlled by optionally peeling iterations.
 *
 * @code{.cpp}
 * constexpr unsigned MinIters = 8;
 * auto loop_body = [&](unsigned idx){ ... };
 * aie::pipelined_loop<MinIters, aie::loop_options{.peel_front = 2, .peel_back = 1}>(n, loop_body);
 * @endcode
 *
 * @tparam MinIters Lower bound on the number of iterations of the loop body
 * @tparam Opts     Options related to peeling loop iterations
 * @param count     Number of iterations
 * @param fn        The callable to pipeline
 *
 */
template<unsigned MinIters, loop_options Opts = loop_options{}, typename Fn>
__aie_inline
void pipelined_loop(unsigned count, Fn &&fn)
{
    constexpr bool use_pipelining = true;
    peeled_pipelined_loop<MinIters, Opts, use_pipelining>(count, std::forward<Fn>(fn));
}

/**
 * @ingroup group_utility_pipelining
 *
 * @brief Invokes a function object a given number of times, applying
 *        peeling at the front and back of the loop.
 *
 * @code{.cpp}
 * constexpr unsigned MinIters = 8;
 * auto loop_body = [&](unsigned idx){ ... };
 * aie::peeled_loop<MinIters, aie::loop_options{.peel_front = 2, .peel_back = 1}>(n, loop_body);
 * @endcode
 *
 * @tparam MinIters Lower bound on the number of iterations of the loop body
 * @tparam Opts     Options related to peeling loop iterations
 * @param count     Number of iterations
 * @param fn        The callable to the function
 *
 */
template<unsigned MinIters, loop_options Opts = loop_options{}, typename Fn>
__aie_inline
void peeled_loop(unsigned count, Fn &&fn)
{
    constexpr bool use_pipelining = false;
    peeled_pipelined_loop<MinIters, Opts, use_pipelining>(count, std::forward<Fn>(fn));
}

/**
 * @ingroup group_utility_pipelining
 *
 * @brief Invokes two function objects a given number of times.
 *        The pipelining of each can be controlled by optionally peeling iterations.
 *
 * @tparam MinIters Lower bound on the number of iterations of the loop body
 * @tparam OptsFn1  Options related to peeling loop iterations of the first function
 * @tparam OptsFn2  Options related to peeling loop iterations of the second function
 * @param count     Number of iterations
 * @param fn1       The first callable to pipeline
 * @param fn2       The second callable to pipeline
 *
 * For example, take the following
 *
 * @code{.cpp}
 * constexpr unsigned MinIters = 4;
 * auto loop_body1 = [&](unsigned idx){ ... };
 * auto loop_body2 = [&](unsigned idx){ ... };
 * unsigned n = 16;
 * aie::pipelined_loops<MinIters, aie::loop_options{.peel_front = 5, .peel_back = 2},
 *                                aie::loop_options{.peel_front = 3, .peel_back = 4}>(n, loop_body1, loop_body2);
 * @endcode
 *
 * This will result in the following execution:
 *
 * @code{.unparsed}
 *     |stage0 |stage1     |stage2 (main loop)                 |stage3 |stage4 |
 * ----|-------|-----------|-----------------------------------|-------|-------|
 * fn1 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10| 11| 12| 13| 14| 15| - | - |
 * fn2 | - | - | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10| 11| 12| 13| 14| 15|
 * @endcode
 */
template<unsigned MinIters, loop_options OptsFn1 = loop_options{}, loop_options OptsFn2 = loop_options{}, typename Fn1, typename Fn2>
__aie_inline
void pipelined_loops(unsigned count, Fn1 &&fn1, Fn2 &&fn2)
{
    constexpr unsigned total_peel_iters     = OptsFn1.peel_front + OptsFn1.peel_back;
    constexpr unsigned total_peel_iters_fn2 = OptsFn2.peel_front + OptsFn2.peel_back;

    static_assert(total_peel_iters == total_peel_iters_fn2, "Both functions require the same total peeled iterations");
    static_assert(OptsFn1.preamble_offset == OptsFn2.preamble_offset, "Both functions require the same preamble_offset");

    static_assert(total_peel_iters <= MinIters, "Requested peeling exceeds loop range");

    REQUIRES_MSG(count >= total_peel_iters, "Cannot peel more iterations than the loop will be executed");

    constexpr unsigned peel_front1 = OptsFn1.peel_front;
    constexpr unsigned peel_back1  = OptsFn1.peel_back;
    constexpr unsigned peel_front2 = OptsFn2.peel_front;
    constexpr unsigned peel_back2  = OptsFn2.peel_back;

    constexpr auto absdiff = [](auto x, auto y) { return x > y ? x - y : y - x; };

    // There are 5 stage of peeling:
    //   0. Running a single function as its requested peeling is greater than the other
    //   1. Running both functions remaining peeled iterations
    //   2. Running the main loop with both functions
    //   3. Running both functions rotated peeled iterations
    //   4. Running a single function as its requested rotated peeling exceeds the other

    constexpr unsigned stage0_count      = absdiff(peel_front1, peel_front2);
    constexpr unsigned stage0_fn1_offset = 0;
    constexpr unsigned stage0_fn2_offset = 0;

    constexpr unsigned stage1_count      = std::min(peel_front1, peel_front2);
    constexpr unsigned stage1_fn1_offset = peel_front1 - stage1_count;
    constexpr unsigned stage1_fn2_offset = peel_front2 - stage1_count;
    
              unsigned stage2_count      = count - total_peel_iters;
    constexpr unsigned stage2_fn1_offset = peel_front1;
    constexpr unsigned stage2_fn2_offset = peel_front2;
    
    constexpr unsigned stage3_count      = std::min(peel_back1, peel_back2);
              unsigned stage3_fn1_offset = stage2_count + stage2_fn1_offset;
              unsigned stage3_fn2_offset = stage2_count + stage2_fn2_offset;
    
    constexpr unsigned stage4_count      = absdiff(peel_back1, peel_back2);
              unsigned stage4_fn1_offset = stage3_count + stage3_fn1_offset;
              unsigned stage4_fn2_offset = stage3_count + stage3_fn2_offset;

    // e.g. count = 16
    //       OptsFn1 = loop_options{.peel_front = 5, .peel_back = 2}
    //       OptsFn2 = loop_options{.peel_front = 3, .peel_back = 4}
    //
    //       peel_front1 = 5, peel_back1 = 2
    //       peel_front2 = 3, peel_back2 = 4
    //
    // Stage      | 0 | 1 | 2  | 3  | 4  |
    // -----------|---|---|----|----|----|
    // count      | 2 | 3 | 9  | 2  | 2  |
    // fn1_offset | 0 | 2 | 5  | 14 | 16 |
    // fn2_offset | 0 | 0 | 3  | 12 | 14 |
    //
    //     |stage0 |stage1     |stage2 (main loop)                 |stage3 |stage4 |
    // ----|-------|-----------|-----------------------------------|-------|-------|
    // fn1 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10| 11| 12| 13| 14| 15| - | - |
    // fn2 | - | - | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10| 11| 12| 13| 14| 15|


    if constexpr (stage0_count > 0) {
        if constexpr (peel_front1 > peel_front2) {
            unroll_times<stage0_count>([&](auto i) __aie_inline {
                fn1(front_idx<peel_front1, peel_back1, stage0_fn1_offset + i>{});
            });
        } else {
            unroll_times<stage0_count>([&](auto i) __aie_inline {
                fn2(front_idx<peel_front2, peel_back2, stage0_fn2_offset + i>{});
            });
        }
    }

    unroll_times<stage1_count>([&](auto i) __aie_inline {
        fn1(front_idx<peel_front1, peel_back1, stage1_fn1_offset + i>{});
        fn2(front_idx<peel_front2, peel_back2, stage1_fn2_offset + i>{});
    });

#if !AIE_API_NATIVE
    [[using chess: prepare_for_pipelining,
                   min_loop_count(MinIters - total_peel_iters),
                   pipeline_adjust_preamble(OptsFn1.preamble_offset)]]
#endif
    for (unsigned i = 0; i < stage2_count; ++i) {
        fn1(loop_idx<peel_front1, peel_back1>{stage2_fn1_offset + i});
        fn2(loop_idx<peel_front2, peel_back2>{stage2_fn2_offset + i});
    }

    unroll_times<stage3_count>([&](auto i) __aie_inline {
        fn1(back_idx<peel_front1, peel_back1, peel_back1 - 1 - i>{stage3_fn1_offset + i});
        fn2(back_idx<peel_front2, peel_back2, peel_back2 - 1 - i>{stage3_fn2_offset + i});
    });

    if constexpr (stage4_count > 0) {
        if constexpr (peel_back1 > peel_back2) {
            unroll_times<stage4_count>([&](auto i) __aie_inline {
                fn1(back_idx<peel_front1, peel_back1, stage4_count - 1 - i>{stage4_fn1_offset + i});
            });
        } else {
            unroll_times<stage4_count>([&](auto i) __aie_inline {
                fn2(back_idx<peel_front2, peel_back2, stage4_count - 1 - i>{stage4_fn2_offset + i});
            });
        }
    }
}

/**
 * @ingroup group_utility_pipelining
 *
 * @brief Invokes 3 function objects a given number of times.
 *        The pipelining of each can be controlled by optionally peeling iterations.
 *
 * @tparam MinIters Lower bound on the number of iterations of the loop body
 * @tparam OptsFn1  Options related to peeling loop iterations of the first function
 * @tparam OptsFn2  Options related to peeling loop iterations of the second function
 * @tparam OptsFn3  Options related to peeling loop iterations of the third function
 * @param count     Number of iterations
 * @param fn1       The first callable to pipeline
 * @param fn2       The second callable to pipeline
 * @param fn3       The second callable to pipeline
 *
 */
 template<unsigned MinIters, loop_options OptsFn1 = loop_options{}, loop_options OptsFn2 = loop_options{}, loop_options OptsFn3 = loop_options{}, typename Fn1, typename Fn2, typename Fn3>
 __aie_inline
 void pipelined_loops(unsigned count, Fn1 &&fn1, Fn2 &&fn2, Fn3 &&fn3)
 {
    constexpr unsigned total_peel_iters     = OptsFn1.peel_front + OptsFn1.peel_back;
    constexpr unsigned total_peel_iters_fn2 = OptsFn2.peel_front + OptsFn2.peel_back;
    constexpr unsigned total_peel_iters_fn3 = OptsFn3.peel_front + OptsFn3.peel_back;

    static_assert(total_peel_iters == total_peel_iters_fn2 && total_peel_iters == total_peel_iters_fn3, "All 3 functions require the same total peeled iterations");
    static_assert(OptsFn1.preamble_offset == OptsFn2.preamble_offset && OptsFn1.preamble_offset == OptsFn3.preamble_offset, "All 3 functions require the same preamble_offset");

    static_assert(total_peel_iters <= MinIters, "Requested peeling exceeds loop range");

    REQUIRES_MSG(count >= total_peel_iters, "Cannot peel more iterations than the loop will be executed");

    constexpr unsigned peel_front1 = OptsFn1.peel_front;
    constexpr unsigned peel_back1  = OptsFn1.peel_back;
    constexpr unsigned peel_front2 = OptsFn2.peel_front;
    constexpr unsigned peel_back2  = OptsFn2.peel_back;
    constexpr unsigned peel_front3 = OptsFn3.peel_front;
    constexpr unsigned peel_back3  = OptsFn3.peel_back;

    constexpr unsigned max_front_peeling = std::max({peel_front1, peel_front2, peel_front3});
    constexpr unsigned mid_front_peeling = std::max(
        std::min(peel_front1, peel_front2),
        std::max(std::min(peel_front1, peel_front3), std::min(peel_front2, peel_front3))
    );
    constexpr unsigned min_front_peeling = std::min({peel_front1, peel_front2, peel_front3});

    constexpr unsigned min_back_peeling = std::min({peel_back1, peel_back2, peel_back3});
    constexpr unsigned mid_back_peeling = std::max(
        std::min(peel_back1, peel_back2),
        std::max(std::min(peel_back1, peel_back3), std::min(peel_back2, peel_back3)));
    constexpr unsigned max_back_peeling = std::max({peel_back1, peel_back2, peel_back3});

    constexpr unsigned stage0_count = max_front_peeling - mid_front_peeling;
    constexpr unsigned stage1_count = mid_front_peeling - min_front_peeling;
    constexpr unsigned stage2_count = min_front_peeling;

    unsigned stage3_count = count - total_peel_iters;

    constexpr unsigned stage4_count = min_back_peeling;
    constexpr unsigned stage5_count = mid_back_peeling - min_back_peeling;
    constexpr unsigned stage6_count = max_back_peeling - mid_back_peeling;

    // front peeling
    constexpr unsigned stage0_fn1_offset = 0;
    constexpr unsigned stage0_fn2_offset = 0;
    constexpr unsigned stage0_fn3_offset = 0;

    constexpr unsigned stage1_fn1_offset = (peel_front1 == max_front_peeling)? stage0_count : 0;
    constexpr unsigned stage1_fn2_offset = (peel_front2 == max_front_peeling)? stage0_count : 0;
    constexpr unsigned stage1_fn3_offset = (peel_front3 == max_front_peeling)? stage0_count : 0;

    constexpr unsigned stage2_fn1_offset = (peel_front1 >= mid_front_peeling)? stage1_fn1_offset + stage1_count : stage1_fn1_offset;
    constexpr unsigned stage2_fn2_offset = (peel_front2 >= mid_front_peeling)? stage1_fn2_offset + stage1_count : stage1_fn2_offset;
    constexpr unsigned stage2_fn3_offset = (peel_front3 >= mid_front_peeling)? stage1_fn2_offset + stage1_count : stage1_fn3_offset;

    // non-peeled iterations
    constexpr unsigned stage3_fn1_offset = peel_front1;
    constexpr unsigned stage3_fn2_offset = peel_front2;
    constexpr unsigned stage3_fn3_offset = peel_front3;

    // back peeling
    unsigned stage4_fn1_offset = stage3_count + stage3_fn1_offset;
    unsigned stage4_fn2_offset = stage3_count + stage3_fn2_offset;
    unsigned stage4_fn3_offset = stage3_count + stage3_fn3_offset;

    unsigned stage5_fn1_offset = stage4_count + stage4_fn1_offset;
    unsigned stage5_fn2_offset = stage4_count + stage4_fn2_offset;
    unsigned stage5_fn3_offset = stage4_count + stage4_fn3_offset;

    unsigned stage6_fn1_offset = stage5_count + stage5_fn1_offset;
    unsigned stage6_fn2_offset = stage5_count + stage5_fn2_offset;
    unsigned stage6_fn3_offset = stage5_count + stage5_fn3_offset;

    // front peeling stages
    if constexpr (stage0_count > 0) {
        unroll_times<stage0_count>([&](auto i) __aie_inline {
            if constexpr (peel_front1 == max_front_peeling) {
                fn1(front_idx<peel_front1, peel_back1, stage0_fn1_offset + i>{});
            }
            if constexpr (peel_front2 == max_front_peeling) {
                fn2(front_idx<peel_front2, peel_back2, stage0_fn2_offset + i>{});
            }
            if constexpr (peel_front3 == max_front_peeling) {
                fn3(front_idx<peel_front3, peel_back3, stage0_fn3_offset + i>{});
            }
        });
    }

    if constexpr (stage1_count > 0) {
        unroll_times<stage1_count>([&](auto i) __aie_inline {
            if constexpr (peel_front1 >= mid_front_peeling) {
                fn1(front_idx<peel_front1, peel_back1, stage1_fn1_offset + i>{});
            }
            if constexpr (peel_front2 >= mid_front_peeling) {
                fn2(front_idx<peel_front2, peel_back2, stage1_fn2_offset + i>{});
            }
            if constexpr (peel_front3 >= mid_front_peeling) {
                fn3(front_idx<peel_front3, peel_back3, stage1_fn3_offset + i>{});
            }
        });
    }

    unroll_times<stage2_count>([&](auto i) __aie_inline {
        fn1(front_idx<peel_front1, peel_back1, stage2_fn1_offset + i>{});
        fn2(front_idx<peel_front2, peel_back2, stage2_fn2_offset + i>{});
        fn3(front_idx<peel_front3, peel_back3, stage2_fn3_offset + i>{});
    });

    // non-peeled stage
 #if !AIE_API_NATIVE
     [[using chess: prepare_for_pipelining,
                    min_loop_count(MinIters - total_peel_iters),
                    pipeline_adjust_preamble(OptsFn1.preamble_offset)]]
 #endif
    for (unsigned i = 0; i < stage3_count; ++i) {
        fn1(loop_idx<peel_front1, peel_back1>{stage3_fn1_offset + i});
        fn2(loop_idx<peel_front2, peel_back2>{stage3_fn2_offset + i});
        fn3(loop_idx<peel_front3, peel_back3>{stage3_fn3_offset + i});
    }

    // back peeling stages
    unroll_times<stage4_count>([&](auto i) __aie_inline {
         fn1(back_idx<peel_front1, peel_back1, peel_back1 - 1 - i>{stage4_fn1_offset + i});
         fn2(back_idx<peel_front2, peel_back2, peel_back2 - 1 - i>{stage4_fn2_offset + i});
         fn3(back_idx<peel_front3, peel_back3, peel_back3 - 1 - i>{stage4_fn3_offset + i});
    });

    if constexpr (stage5_count > 0) {
        unroll_times<stage5_count>([&](auto i) __aie_inline {
            if constexpr (peel_back1 >= mid_back_peeling) {
                fn1(back_idx<peel_front1, peel_back1, stage5_count - 1 - i>{stage5_fn1_offset + i});
            }
            if constexpr (peel_back2 >= mid_back_peeling) {
                fn2(back_idx<peel_front2, peel_back2, stage5_count - 1 - i>{stage5_fn2_offset + i});
            }
            if constexpr (peel_back3 >= mid_back_peeling) {
                fn3(back_idx<peel_front3, peel_back3, stage5_count - 1 - i>{stage5_fn3_offset + i});
            }
        });
    }

    if constexpr (stage6_count > 0) {
        unroll_times<stage6_count>([&](auto i) __aie_inline {
            if constexpr (peel_back1 == max_back_peeling) {
                fn1(back_idx<peel_front1, peel_back1, stage6_count - 1 - i>{stage6_fn1_offset + i});
            }
            if constexpr (peel_back2 == max_back_peeling) {
                fn2(back_idx<peel_front2, peel_back2, stage6_count - 1 - i>{stage6_fn2_offset + i});
            }
            if constexpr (peel_back3 == max_back_peeling) {
                fn3(back_idx<peel_front3, peel_back3, stage6_count - 1 - i>{stage6_fn3_offset + i});
            }
        });
    }
}

namespace utils {

/**
 * @defgroup group_utility_non_portable Non-portable optimizations
 * @ingroup group_utility_functions
 *
 * These functions provide non-portable or semi-portable functionality that can
 * be used to apply low level optimizations.
 */

/**
 * @defgroup group_utility_pinning Register pinning
 * @ingroup group_utility_non_portable
 *
 * These functions provide a semi-portable interface to pin variables, such as vectors, accumulators,
 * masks, and pointers, to a given register/set of registers.
 */

using AIE_RegFile = detail::utils::AIE_RegFile;

/**
 * @ingroup group_utility_pinning
 *
 * @brief Binds a value to a specified register.
 * The type and size of the value determines the type of register and the template parameter `Reg` selects which
 * specific register to bind it to.
 *
 * @tparam Reg     Numeric identifier of the register the value will be bound to.
 * @tparam RegFile Register file to locate the value.
 * @param val      The value to be pinned. This may be an aie::vector, aie::accum, or scalar value.
 *
 * Example:
 * @code{.cpp}
 * aie::vector<int32, 16> v;
 * aie::utils::locate_in_register<4>(v)
 * @endcode
 *
 * This will bind `v` to register `x4` on AIE-ML:
 * - `Reg` template parameter is set to `4`.
 * - `v` is a vector of 512bit, so it will bind to a `x` 512bit vector register.
 */
template<unsigned Reg = (unsigned)-1, AIE_RegFile RegFile = AIE_RegFile::Default, typename T>
__aie_inline
auto locate_in_register(T&& val)
{
    detail::utils::locate_in_register<Reg, RegFile>(val);
    return val;
}

template<unsigned Reg = (unsigned)-1, AIE_RegFile RegFile = AIE_RegFile::Default, typename T>
__aie_inline
auto locate_in_register(const T& val)
{
    T v = val;
    detail::utils::locate_in_register<Reg, RegFile>(v);
    return v;
}

/**
 * @ingroup group_utility_pinning
 *
 * @brief Binds each variable in an array to sequential registers, starting from the specified register.
 *
 * @tparam Reg     Numeric identifier of register to bind the first variable to.
 * @tparam RegFile Which register file to locate the variable in.
 * @param arr      The value to be placed. This may be an aie::vector, aie::accum, or scalar value.
 *
 * Example:
 * @code{.cpp}
 * aie::vector<int32, 16> vs[3];
 * aie::utils::locate_in_register<4>(vs)
 * @endcode
 *
 * This will bind `vs[0]` to `x4`, `vs[1]` to `x5` and `vs[2]` to `x6` on AIE-ML.
 *
 * @sa locate_in_register(T&&)
 */
template<unsigned StartIdx = 0, unsigned N, AIE_RegFile RegFile = AIE_RegFile::Default, typename T>
__aie_inline
void locate_in_register(T (&arr)[N])
{
    detail::utils::locate_in_register<StartIdx, N, RegFile>(arr);
}

} // namespace utils

} // namespace aie

#endif

#endif
