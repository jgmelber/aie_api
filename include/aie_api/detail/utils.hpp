// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_UTILS__HPP__
#define __AIE_API_DETAIL_UTILS__HPP__

#include <array>
#include <cassert>
#include <tuple>
#include <type_traits>

#include "config.hpp"

namespace aie::detail::utils {

template <typename T>
struct make_signed
{
    using type = T;
};

template <> struct make_signed< uint8_t> { using type = int8_t;  };
template <> struct make_signed<uint16_t> { using type = int16_t; };
template <> struct make_signed<uint32_t> { using type = int32_t; };

#if AIE_API_ML_VERSION >= 200
template <> struct make_signed<uint4_t>  { using type = int4_t;  };
#endif

template <typename T>
struct make_unsigned
{
    using type = T;
};

template <> struct make_unsigned< int8_t> { using type = uint8_t;  };
template <> struct make_unsigned<int16_t> { using type = uint16_t; };
template <> struct make_unsigned<int32_t> { using type = uint32_t; };

#if AIE_API_ML_VERSION >= 200
template <> struct make_unsigned<int4_t>  { using type = uint4_t;  };
#endif

/*
 * Obtain the signed version of the given type. If the type is already signed, the same type is returned
*/
template <typename T>
using make_signed_t = typename make_signed<T>::type;

/*
 * Obtain the unsigned version of the given type. If the type is already unsigned, the same type is returned
*/
template <typename T>
using make_unsigned_t = typename make_unsigned<T>::type;

template <bool Signed, unsigned Bits>
struct get_integer_type;

template <> struct get_integer_type<true, 8>   { using type = int8_t;   };
template <> struct get_integer_type<true, 16>  { using type = int16_t;  };
template <> struct get_integer_type<true, 32>  { using type = int32_t;  };
template <> struct get_integer_type<true, 64>  { using type = int64_t;  };

template <> struct get_integer_type<false, 8>  { using type = uint8_t;  };
template <> struct get_integer_type<false, 16> { using type = uint16_t; };
template <> struct get_integer_type<false, 32> { using type = uint32_t; };
template <> struct get_integer_type<false, 64> { using type = uint64_t; };

#if AIE_API_ML_VERSION >= 200
template <> struct get_integer_type<true,  4>  { using type = int4_t;   };
template <> struct get_integer_type<false, 4>  { using type = uint4_t;  };
#endif

/*
 * Obtain the integer type described by the combination of sign and number of bits. Bits needs to be a power of 2 in
 * the [4 to 64] range
 */
template <bool Signed, unsigned Bits>
using get_integer_type_t = typename get_integer_type<Signed, Bits>::type;

template <typename T>
struct get_next_integer_type
{
    using type = void;
};

template <> struct get_next_integer_type<int8_t>   { using type = int16_t;  };
template <> struct get_next_integer_type<int16_t>  { using type = int32_t;  };
template <> struct get_next_integer_type<int32_t>  { using type = int64_t;  };

template <> struct get_next_integer_type<uint8_t>  { using type = uint16_t; };
template <> struct get_next_integer_type<uint16_t> { using type = uint32_t; };
template <> struct get_next_integer_type<uint32_t> { using type = uint64_t; };

template <> struct get_next_integer_type<cint16>   { using type = cint32;   };

#if AIE_API_ML_VERSION >= 200
template <> struct get_next_integer_type<int4_t>   { using type = int8_t;   };
template <> struct get_next_integer_type<uint4_t>  { using type = uint8_t;  };
#endif

/*
 * Obtain the integer type with the same sign as the given type but with twice as many bits. Bits needs to be a power
 * of 2 in the [4 to 32] range
 */
template <typename T>
using get_next_integer_type_t = typename get_next_integer_type<T>::type;

template <typename T>
struct get_prev_integer_type
{
    using type = void;
};

template <> struct get_prev_integer_type<int16_t>  { using type = int8_t;   };
template <> struct get_prev_integer_type<int32_t>  { using type = int16_t;  };
template <> struct get_prev_integer_type<int64_t>  { using type = int32_t;  };

template <> struct get_prev_integer_type<uint16_t> { using type = uint8_t;  };
template <> struct get_prev_integer_type<uint32_t> { using type = uint16_t; };
template <> struct get_prev_integer_type<uint64_t> { using type = uint32_t; };

template <> struct get_prev_integer_type<cint32>   { using type = cint16;   };

#if AIE_API_ML_VERSION >= 200
template <> struct get_prev_integer_type<int8_t>   { using type = int4_t;   };
template <> struct get_prev_integer_type<uint8_t>  { using type = uint4_t;  };
#endif

/*
 * Obtain the integer type with the same sign as the given type but with half as many bits. Bits needs to be a power
 * of 2 in the [4 to 32] range
 */
template <typename T>
using get_prev_integer_type_t = typename get_prev_integer_type<T>::type;

template <typename T>
struct get_complex_component_type
{
    using type = void;
};

template <> struct get_complex_component_type<cint16>  { using type = int16_t; };
template <> struct get_complex_component_type<cint32>  { using type = int32_t; };

#if AIE_API_MATH_VERSION >= 100 || __AIE_API_COMPLEX_FP32_EMULATION__
#if __AIE_API_CBF16_SUPPORT__
template <> struct get_complex_component_type<cbfloat16> { using type = bfloat16; };
#endif
template <> struct get_complex_component_type<cfloat>    { using type = float;    };
#endif

/*
 * Obtain the type of the real and imaginary components of a given complex type
 */
template <typename T>
using get_complex_component_type_t = typename get_complex_component_type<T>::type;

template <typename T>
struct num_elems
{
    static constexpr unsigned value = 1;
};

template <typename T, unsigned N>
struct num_elems<std::array<T, N>>
{
    static constexpr unsigned value = N;
};

/*
 * Obtain the number of elements of the given type. If an std::array is given, then it returns the number of elements
 * in the array. Otherwise it returns 1.
 */
template <typename T>
constexpr unsigned num_elems_v = num_elems<T>::value;

template <typename T>
struct has_address_space
{
    static constexpr bool value = false;
};

#if !AIE_API_NATIVE

template <typename T, int N> struct has_address_space<               T __attribute__((address_space(N)))> { static constexpr bool value = true; };
template <typename T, int N> struct has_address_space<         const T __attribute__((address_space(N)))> { static constexpr bool value = true; };
template <typename T, int N> struct has_address_space<      volatile T __attribute__((address_space(N)))> { static constexpr bool value = true; };
template <typename T, int N> struct has_address_space<const volatile T __attribute__((address_space(N)))> { static constexpr bool value = true; };

#endif

/*
 * Says whether the given type has defined an address space
 */
template <typename T>
static constexpr bool has_address_space_v = has_address_space<T>::value;

template <typename T>
struct remove_all
{
    using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

/*
 * Obtains a type which would result from applying both std::remove_cv and std::remove_reference to the given type.
 */
template <typename T>
using remove_all_t = typename remove_all<T>::type;

template <typename T, typename... Types>
struct is_one_of
{
    static constexpr bool value = (... || std::is_same_v<T, Types>);
};

/*
 * Says whether the first time matches any of the rest of given types
 */
template <typename T, typename... Types>
static constexpr bool is_one_of_v = is_one_of<T, Types...>::value;

template <size_t Elems, typename T, T Value, T... Generated>
struct make_integer_broadcast_impl
{
    using type = typename make_integer_broadcast_impl<Elems - 1, T, Value, Value, Generated...>::type;
};

template <typename T, T Value, T... Generated>
struct make_integer_broadcast_impl<0, T, Value, Generated...>
{
    using type = std::integer_sequence<T, Generated...>;
};

/*
 * Create a sequence of elements of the given type with the same given value.
 */
template <typename T, size_t Elems, T Value>
using make_integer_broadcast = typename make_integer_broadcast_impl<Elems, T, Value>::type;

/*
 * Create a sequence of elements of type size_t with the same given value.
 */
template <size_t Elems, size_t Value>
using make_index_broadcast = make_integer_broadcast<size_t, Elems, Value>;

template <unsigned Start, typename... T, std::size_t... I>
constexpr auto subtuple_(const std::tuple<T...>& t, std::index_sequence<I...>)
{
  return std::make_tuple(std::get<I + Start>(t)...);
}

/*
 * Return a subtuple with the [Start:End) range elements from the input tuple..
 */
template <unsigned Start, unsigned End, typename... T>
constexpr auto subtuple(const std::tuple<T...>& t)
{
    using subtuple_index = std::make_index_sequence<End - Start>;
    return subtuple_<Start>(t, subtuple_index());
}

template <std::size_t ... Indices>
constexpr auto generate_index_sequence_reverse(const std::index_sequence<Indices...> &)
   -> decltype( std::index_sequence<sizeof...(Indices) - 1U - Indices...>{} );

/*
 * Same as std::make_index_sequence but indices are ordered from large to small.
 */
template <std::size_t N>
using make_reverse_index_sequence = decltype(generate_index_sequence_reverse(std::make_index_sequence<N>{}));

template <typename T, typename... Ts> requires (std::is_same_v<T, Ts> && ...)
std::array<T, 1 + sizeof...(Ts)> tuple_to_array(std::tuple<T, Ts...> tup)
{
    using return_type = std::array<T, 1 + sizeof...(Ts)>;
    auto helper = []<std::size_t... Idx>(auto tup, std::index_sequence<Idx...>) -> return_type {
        return {std::get<Idx>(tup)...};
    };
    return helper(tup, std::make_index_sequence<1 + sizeof...(Ts)>{});
}

/*
 * Return the N-th argument passed into the function
 */
template <unsigned N, typename T, typename... Args>
constexpr auto get_nth(const T &t, Args &&... args)
{
    static_assert(N <= sizeof...(Args));

    if constexpr (N == 0)
        return t;
    else
        return get_nth<N - 1>(args...);
}

/*
 * Says whether the value passed in is a power of 2
 */
template <typename T>
__attribute__((const))
constexpr bool is_powerof2(T v)
{
    return v && ((v & (v - 1)) == 0);
}

/*
 * Returns the logarithm in base 2 of the given value
 */
template <typename T>
constexpr unsigned log2(T v)
{
    return v ? 1 + log2(v >> 1) : -1;
}

/*
 * Helper type provided to pack two elements of the same type. It ensures that the packed type has twice the alignment
 * of the individual values and twice its size.
 */
template <typename T>
struct alignas(alignof(T) * 2) pack2
{
    T a;
    T b;

    constexpr pack2(T a, T b) :
        a(a),
        b(b)
    {}

    template <typename C>
    constexpr C as() const
    {
        static_assert(sizeof(C) == sizeof(T) * 2);

        return __builtin_bit_cast(C, *this);
    }
};

/*
 * Says whether the two given references refer to the same object
 */
template <typename T1, typename T2>
__attribute__((pure))
constexpr bool is_same_object(const T1 &a, const T2 &b)
{
    if constexpr (std::is_same_v<T1, T2>)
        return &a == &b;
    else
        return false;
}

/*
 * Type that implements an integer value that wraps around when it reaches the given maximum value. The implementation
 * uses addressing intrinsics that efficiently execute this pattern on AIE. The maximum value can be specified as a
 * compile-time template argument, or as a run-time value (Max template argument set to -1).
 */
template <unsigned Max = (unsigned)-1>
struct circular_index;

/*
 * Specialization of circular_index for the case when the maximum value is not known at compile time
 */
template <>
struct circular_index<(unsigned)-1>
{
public:
    __aie_inline
    SCALAR_TYPES_CONSTEXPR circular_index(unsigned max) :
        idx_(0),
        max_(max)
    {
    }

    __aie_inline
    SCALAR_TYPES_CONSTEXPR operator unsigned() const
    {
        return idx_;
    }

    __aie_inline
    circular_index &operator++()
    {
#if __AIE_ARCH__ == 10
        idx_ = (unsigned)uintptr_t(::cyclic_add((char *)uintptr_t(idx_), 1, (char *)uintptr_t(0), max_));
#elif __AIE_ARCH__ == 20
        ::add_2d_ptr((char *)uintptr_t(0), -int(max_ - 1), max_ - 1, idx_, 1);
#endif
        return *this;
    }

    __aie_inline
    SCALAR_TYPES_CONSTEXPR circular_index operator++(int)
    {
        const circular_index ret = *this;
        ++(*this);
        return ret;
    }

private:
#if __AIE_ARCH__ == 10
    unsigned idx_;
#elif __AIE_ARCH__ == 20
    addr_t idx_;
#endif
    const unsigned max_;
};

/*
 * Specialization of circular_index for the case when the maximum value is known at compile time
 */
template <unsigned Max>
struct circular_index
{
public:
    __aie_inline
    SCALAR_TYPES_CONSTEXPR circular_index() :
        idx_(0)
    {
    }

    __aie_inline
    SCALAR_TYPES_CONSTEXPR operator unsigned() const
    {
        return idx_;
    }

    __aie_inline
    SCALAR_TYPES_CONSTEXPR circular_index &operator++()
    {
        if constexpr (Max == 2)
            idx_ = 1 - idx_;
        else if constexpr (Max > 2)
#if __AIE_ARCH__ == 10
            idx_ = (unsigned)uintptr_t(::cyclic_add((char *)uintptr_t(idx_), 1, (char *)uintptr_t(0), Max));
#else
            ::add_2d_ptr((char *)uintptr_t(0), -int(Max - 1), Max - 1, idx_, 1);
#endif

        return *this;
    }

    __aie_inline
    SCALAR_TYPES_CONSTEXPR circular_index operator++(int)
    {
        const circular_index ret = *this;
        ++(*this);
        return ret;
    }

private:
#if __AIE_ARCH__ == 10
    unsigned idx_;
#elif __AIE_ARCH__ == 20
    addr_t idx_;
#endif
};

circular_index(unsigned max) -> circular_index<(unsigned)-1>;

template <typename T, T Start, T End, T It>
struct iteration_dim
{
    constexpr operator T() const
    {
        return current();
    }

    constexpr T min() const
    {
        return Start;
    }

    constexpr T max() const
    {
        return End;
    }

    constexpr T current() const
    {
        return It;
    }
};

template <typename T, T Start, T End, T It, T Step>
struct unroll_for_helper
{
    static_assert(Step != 0, "0 is not a valid step");

    template <typename Fn>
    __aie_inline
    static void execute(Fn && fn)
    {
        if constexpr ((Step > 0 && It < End) || (Step < 0 && It > End)) {
            constexpr iteration_dim<T, Start, End, It> it{};

            static_assert(std::is_invocable_v<Fn, decltype(it)> ||
                          std::is_invocable_v<Fn, std::integral_constant<T, It>> ||
                          std::is_invocable_v<Fn>);

            if constexpr (std::is_invocable_v<Fn, decltype(it)>)
                fn(it);
            else if constexpr (std::is_invocable_v<Fn, std::integral_constant<T, It>>)
                fn(std::integral_constant<T, It>{});
            else
                fn();

            constexpr T next_it = It + Step;

            // Check for wrap-around
            static_assert((Step > 0) || (next_it < It), "The unrolled loop range wraps around");
            static_assert((Step < 0) || (next_it > It), "The unrolled loop range wraps around");

            unroll_for_helper<T, Start, End, next_it, Step>::execute(std::forward<Fn>(fn));
        }
    }
};

template <typename T, T StartY, T EndY, T ItY, T StepY, T StartX, T EndX, T ItX, T StepX>
struct unroll_for_helper_2d
{
    static_assert(StepY != 0, "0 is not a valid step");
    static_assert(StepX != 0, "0 is not a valid step");

    static constexpr auto compute_next()
    {
        std::pair<T, T> next_it;

        // Check for wrap-around on X
        static_assert((StepX > 0) || (ItX + StepX < ItX), "The unrolled loop range wraps around");
        static_assert((StepX < 0) || (ItX + StepX > ItX), "The unrolled loop range wraps around");

        next_it.first  = ItY;
        next_it.second = ItX + StepX;

        if (StepX > 0 && next_it.second >= EndX) {
            next_it.first  += StepY;
            next_it.second  = StartX;
        }
        else if (StepX < 0 && next_it.second <= EndX) {
            next_it.first  += StepY;
            next_it.second  = StartX;
        }

        return next_it;
    }

    template <typename Fn>
    __aie_inline
    static void execute(Fn && fn)
    {

        if constexpr (((StepY > 0 && ItY < EndY) || (StepY < 0 && ItY > EndY)) &&
                      ((StepX > 0 && ItX < EndX) || (StepX < 0 && ItX > EndX))) {
            constexpr iteration_dim<T, StartY, EndY, ItY> itY{};
            constexpr iteration_dim<T, StartX, EndX, ItX> itX{};

            static_assert(std::is_invocable_v<Fn, decltype(itY), decltype(itX)> ||
                          std::is_invocable_v<Fn, std::integral_constant<T, ItY>, std::integral_constant<T, ItX>> ||
                          std::is_invocable_v<Fn>);

            if constexpr (std::is_invocable_v<Fn, decltype(itY), decltype(itX)>)
                fn(itY, itX);
            else if constexpr (std::is_invocable_v<Fn, std::integral_constant<T, ItY>, std::integral_constant<T, ItX>>)
                fn(std::integral_constant<T, ItY>{}, std::integral_constant<T, ItX>{});
            else
                fn();

            constexpr auto next_it = compute_next();

            // Check for wrap-around on Y
            static_assert((StepY > 0) || (next_it.first + StepY < ItY), "The unrolled loop range wraps around");
            static_assert((StepY < 0) || (next_it.first + StepY > ItY), "The unrolled loop range wraps around");

            unroll_for_helper_2d<T, StartY, EndY, next_it.first,  StepY,
                                    StartX, EndX, next_it.second, StepX>::execute(std::forward<Fn>(fn));
        }
    }
};

/*
 * Creates an index sequence defined by Start, End, Step, and executes the given function many times, passing to each
 * instance one index from the sequence
 */
template <typename T, T Start, T End, T Step = 1, typename Fn>
__aie_inline
void unroll_for(Fn &&fn)
{
    unroll_for_helper<T, Start, End, Start, Step>::execute(std::forward<Fn>(fn));
}

/*
 * Creates a 2D index sequence defined by Start, End, Step, for each dimension and executes the given function many
 * times, passing to each instance the two indices from the sequence
 */
template <typename T, T StartY, T EndY, T StepY, T StartX, T EndX, T StepX, typename Fn>
__aie_inline
void unroll_for_2d(Fn &&fn)
{
    unroll_for_helper_2d<T, StartY, EndY, StartY, StepY, StartX, EndX, StartX, StepX>::execute(std::forward<Fn>(fn));
}

/*
 * Creates the [0, Times) index sequence and executes the given function many times, function passing to each instance
 * an index from the sequence
 */
template <unsigned Times, typename Fn>
__aie_inline
void unroll_times(Fn &&fn)
{
    unroll_for<unsigned, 0, Times, 1>(std::forward<Fn>(fn));
}

/*
 * Creates the [0, TimesY) x [0, TimesX) 2D index sequence and executes the given function many times, function passing
 * to each instance the two indices from the sequence
 */
template <unsigned TimesY, unsigned TimesX, typename Fn>
__aie_inline
void unroll_times_2d(Fn &&fn)
{
    unroll_for_2d<unsigned, 0, TimesY, 1, 0, TimesX, 1>(std::forward<Fn>(fn));
}

template <typename Callable, std::size_t... Idx, typename... Args>
__aie_inline
auto make_array_helper(Callable&& callable, std::index_sequence<Idx...>, Args&&... args)
    -> std::array<std::invoke_result_t<Callable, std::tuple_element_t<0, Args>...>, sizeof...(Idx)>
{
    return {[&]{ constexpr unsigned I = Idx; return callable(std::get<I>(args)...); }()...};
}

template <unsigned N, typename Callable, typename... Args>
__aie_inline
auto make_array(Callable&& callable, Args... args)
{
    using seq = std::make_index_sequence<N>;
    return make_array_helper(std::forward<Callable&&>(callable), seq{}, std::forward<Args&&>(args)...);
}

__aie_inline
static constexpr unsigned clz_impl(unsigned n)
{
    unsigned ret = 0;

    for (int i = 31; i >= 0; --i) {
        if ((1 << i) & n) break;
        ++ret;
    }

    return ret;
}

// Helper to compute the number of leading zeros of a value. When the input value is known at compile time it calls a
// constexpr function. Otherwise, it uses the intrinc functions for AIE.
__aie_inline
static constexpr inline unsigned clz(unsigned n)
{
    if (std::is_constant_evaluated()) {
        return clz_impl(n);
    }
    else {
        if (chess_const(n) && chess_const(0))
            return clz_impl(n);
        else
            return ::clb(n);
    }
}

// Helper to the index of the last set bit in the given a value. When the input value is known at compile time it calls
// a constexpr function. Otherwise, it uses the intrinc functions for AIE.
__aie_inline
static constexpr inline unsigned fls(unsigned n)
{
    return 32 - clz(n) - 1;
}

// Helper to the index of the first set bit in the given a value. When the input value is known at compile time it calls
// a constexpr function. Otherwise, it uses the intrinc functions for AIE.
__aie_inline
static constexpr inline unsigned ffs(unsigned n)
{
    return 32 - clz(n & -n) - 1;
}

__aie_inline
static constexpr inline unsigned ceildiv(unsigned num, unsigned div)
{
    return (num - 1) / div + 1;
}

//
// Helper macros for run-time and compile-time checks
//

// Helper required to get the string of a macro argument
#define STRINGIFY_(a) #a
#define STRINGIFY(a) STRINGIFY_(a)

#if AIE_API_NATIVE

// Compile-time assertions to check that an expression is constant. This check is *disabled* on Native
#define STATIC_ASSERT_CONSTANT(e, m)
#define STATIC_ASSERT_CONSTANT_EXPRESSION(e, m)

// Run-time assertion to check that an expression evaluates to true
#define RUNTIME_ASSERT(e, m)           assert(e && m)
#define RUNTIME_ASSERT_NO_ASSUME(e, m) assert(e && m)

// Assertion to check that an expression evaluates to true. On Native it is always evaluated at run time.
#define REQUIRES_MSG(a, m)          RUNTIME_ASSERT(a, m)
#define REQUIRES(a)                 REQUIRES_MSG(a, "Requirement " STRINGIFY(a) " not met")

// Assertions to check that an expression can be evaluated at compile time. This check is *disabled* on Native.
#define REQUIRES_CONSTANT_MSG(a, m)
#define REQUIRES_CONSTANT(a)

// Assertions to check that the program flow should not reach a specific point.
// On Native it is evaluated at run time.
#define UNREACHABLE_MSG(m)                                                                                             \
    do {                                                                                                               \
        assert(false && "Unreachable; " m);                                                                            \
        __builtin_unreachable();                                                                                       \
    } while (0)

#else

// Compile-time assertions to check that an expression is constant
#define STATIC_ASSERT_CONSTANT(e, m)            do { if (!chess_const(e))    chess_error(m); } while (0)
#define STATIC_ASSERT_CONSTANT_EXPRESSION(e, m) do { if (!chess_manifest(e)) chess_error(m); } while (0)

// Run-time assertion to check that an expression evaluates to true. This check is *disabled* on LLVM. Instead, an
// annotation is given for the compiler to assume that the exression evaluates to true. The compiler can use
// this information to introduce more optimizations.
#define RUNTIME_ASSERT(e, m)           __builtin_assume(e)

// Same as RUNTIME_ASSERT, but no annotation is given to the compiler
#define RUNTIME_ASSERT_NO_ASSUME(e, m)

// Assertions to check that an expression evaluates to true. If possible, the expresion is evaluated at compile time.
#define REQUIRES_MSG(a, m)                           \
    do {                                             \
        if (chess_const(a)) {                        \
            STATIC_ASSERT_CONSTANT_EXPRESSION(a, m); \
        } else {                                     \
            __builtin_assume(a);                     \
        }                                            \
    } while (0)

#define REQUIRES(a)                 REQUIRES_MSG(a, "Requirement " STRINGIFY(a) " not met")

// Assertions to check that an expression can be evaluated at compile time.
#define REQUIRES_CONSTANT_MSG(a, m) STATIC_ASSERT_CONSTANT(a, m)
#define REQUIRES_CONSTANT(a)        REQUIRES_CONSTANT_MSG(a, "Expression " STRINGIFY(a) " is not constant")

// Assertions to check that the program flow should not reach a specific point.
// This check is *disabled* on LLVM. Instead, an annotation is given to the compiler, which can use this information to
// introduce more optimizations.
#define UNREACHABLE_MSG(m)              \
    do {                                \
        chess_error("Unreachable; " m); \
        __builtin_unreachable();        \
    } while (0)

#endif

template <typename Function, typename TupleLike, size_t... Is>
__aie_inline
auto apply_tuple_helper(Function &&f, TupleLike &&t, std::index_sequence<Is...>)
{
    return std::forward<Function>(f)(std::get<Is>(std::forward<TupleLike>(t))...);
}

template <typename Function, typename TupleLike>
__aie_inline
auto apply_tuple(Function &&f, TupleLike &&t)
{
    constexpr size_t N = std::tuple_size_v<std::decay_t<TupleLike>>;
    return apply_tuple_helper(std::forward<Function>(f),
                              std::forward<TupleLike>(t),
                              std::make_index_sequence<N>{});
}

}

#if AIE_API_ML_VERSION >= 200 && !defined(__STDCPP_BFLOAT16_T__)

namespace std {
    template<>
    class numeric_limits<::bfloat16> {
        using T = ::bfloat16;

    public:
        static constexpr bool is_specialized           = true;
        static constexpr int  digits                   = 8;
        static constexpr int  digits10                 = 2;
        static constexpr int  max_digits10             = 2 + (digits * 30103l) / 100000l;
        static constexpr bool is_signed                = true;
        static constexpr bool is_integer               = false;
        static constexpr bool is_exact                 = false;
        static constexpr int  radix                    = 2;
        static constexpr int  min_exponent             = -125;
        static constexpr int  min_exponent10           = -37;
        static constexpr int  max_exponent             = 128;
        static constexpr int  max_exponent10           = 38;
        static constexpr bool has_infinity             = true;
        static constexpr bool has_quiet_NaN            = true;
        static constexpr bool has_signaling_NaN        = false;
        static constexpr float_denorm_style has_denorm = denorm_absent;
        static constexpr bool has_denorm_loss          = false;
        static constexpr bool is_iec559                = false;
        static constexpr bool is_bounded               = false;
        static constexpr bool is_modulo                = false;
        static constexpr bool traps                    = false;
        static constexpr bool tinyness_before          = false;
        //static constexpr float_round_style round_style = round_toward_zero; // Set by rounding mode

        // Only the upper 2 bytes of these literals are of interest as the lower two are removed
        static BFLOAT16_CONSTEXPR T min()           { return __builtin_bit_cast(float, 0x0080'0000); }
        static BFLOAT16_CONSTEXPR T max()           { return __builtin_bit_cast(float, 0x7f7f'0000); }
        static BFLOAT16_CONSTEXPR T lowest()        { return -max(); }
        static BFLOAT16_CONSTEXPR T epsilon()       { return __builtin_bit_cast(float, ((0x7f - (digits - 1)) << (digits - 1)) << 16); }
        static BFLOAT16_CONSTEXPR T round_error()   { return __builtin_bit_cast(float, 0x3f00'0000); }
        static BFLOAT16_CONSTEXPR T infinity()      { return __builtin_bit_cast(float, 0x7f80'0000); }
        static BFLOAT16_CONSTEXPR T quiet_NaN()     { return __builtin_bit_cast(float, 0x7fc0'0000); }
        static BFLOAT16_CONSTEXPR T signaling_NaN() { return 0.0; }
        static BFLOAT16_CONSTEXPR T denorm_min()    { return 0.0; }
    };
}

#endif

#endif
