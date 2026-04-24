// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_CONFIG__HPP__
#define __AIE_API_DETAIL_CONFIG__HPP__

#if defined(__chess__) || defined(__AIECC__)
#define AIE_API_NATIVE 0
#else
#define AIE_API_NATIVE 1
#endif

#ifndef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
#define __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__ 1
#endif

#if __chess__
#define __AIE_API_FUNDAMENTAL_TYPE__ [[chess::behave_as_fundamental_type]]
#define __AIE_API_KEEP_IN_REGISTERS__ [[chess::keep_in_registers]]
#else
#define __AIE_API_FUNDAMENTAL_TYPE__
#define __AIE_API_KEEP_IN_REGISTERS__
#endif

#ifdef AIE_DEBUG_NOINLINE
#define __aie_inline
#else
#define __aie_inline __attribute__((always_inline))
#endif
#define __aie_noinline __attribute__((noinline))

#ifndef AIE_API_FFT_STAGE_INLINE
#define __aie_fft_inline __aie_inline
#else
#if AIE_API_FFT_STAGE_INLINE
#define __aie_fft_inline __aie_inline
#else
#define __aie_fft_inline __aie_noinline
#endif
#endif

#if __chess__
#define __aie_api_duplicate(x) chess_duplicate((x))
#else
#define __aie_api_duplicate(x) (x)
#endif

#ifndef __AIE_API_COMPLEX_VECTOR_SUPPORT__
#if __AIECC__
#define __AIE_API_COMPLEX_VECTOR_SUPPORT__ 0
#else
#define __AIE_API_COMPLEX_VECTOR_SUPPORT__ 1
#endif
#endif

#if __AIE_ARCH__ == 10

#include "aie1/config.hpp"

#elif __AIE_ARCH__ == 20

#include "aie2/config.hpp"

#elif __AIE_ARCH__ == 21

#include "aie2p/config.hpp"

#elif __AIE_ARCH__ == 22

#include "aie2ps/config.hpp"

#endif

#if (AIE_API_NATIVE == 0)
#define SCALAR_TYPES_CONSTEXPR constexpr
#else
#define SCALAR_TYPES_CONSTEXPR
#endif

#if (AIE_API_NATIVE == 0) && __AIE_API_CONSTEXPR_BFLOAT16__
//TODO: Pending CRVO-7585
#define BFLOAT16_CONSTEXPR constexpr
#else
#define BFLOAT16_CONSTEXPR
#endif

#if (AIE_API_NATIVE == 0)
#define FLOAT16_CONSTEXPR constexpr
#else
#define FLOAT16_CONSTEXPR
#endif

#if (AIE_API_NATIVE == 0)
#define BFLOAT8_CONSTEXPR constexpr
#else
#define BFLOAT8_CONSTEXPR
#endif

#if (AIE_API_NATIVE == 0)
#define FLOAT8_CONSTEXPR constexpr
#else
#define FLOAT8_CONSTEXPR
#endif

namespace aie {

/**
 * @ingroup group_utility_functions
 * @defgroup group_utility_arch Architecture helpers
 */

/**
 * @ingroup group_utility_arch
 *
 * Structure used to represent the AIE architecture being compiled against.
 */
struct arch {
    /**
     * An enum defining available AIE architectures.
     */
    enum ArchVersion : unsigned {
        AIE      = 10,
        AIE_ML   = 20,
        XDNA2    = 21,
        AIE_MLv2 = 22
    };

    /**
     * An enum defining available AIE generations, which are defined as:
     */
    enum ArchGeneration : unsigned {
        Gen1 = 1, //!< AIE
        Gen2 = 2  //!< AIE-ML/XDNA1, XDNA2, AIE-MLv2
    };

    /**
     * Represents the current AIE architecture version.
     */
    static constexpr ArchVersion version = ArchVersion(__AIE_ARCH__);

    /**
     * Represents the current AIE generation.
     */
    static constexpr ArchGeneration generation = ArchGeneration(__AIE_ARCH__ / 10);

    /**
     * Checks if the current AIE architecture version against the supplied pack.
     *
     * @param vs A pack of ArchVersions to test the current version against
     */
    template <typename... T> requires (std::is_same_v<T, ArchVersion> && ...)
    static constexpr bool is(T... vs) { return ((version == vs) || ...); }

    /**
     * Checks if the current AIE architecture version against the supplied generation pack.
     *
     * @param gens A pack of ArchGenerations to test the current version against
     */
    template <typename... T> requires (std::is_same_v<T, ArchGeneration> && ...)
    static constexpr bool is(T... gens) { return ((generation == gens) || ...); }
};

}

#endif
