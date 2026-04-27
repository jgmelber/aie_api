// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#include <aie_api/aie.hpp>
#include <aie_api/utils.hpp>

constexpr unsigned BUFFER_COUNT = 32;
//![Aligned buffer]
alignas(aie::vector_decl_align) static int16 my_buffer[BUFFER_COUNT];
//![Aligned buffer]

//![Aligned memcpy]
template <typename T>
T* aligned_memcpy(T* __restrict dest, const T* src, unsigned n)
{
    // 256b vectors are used in this example
    static constexpr unsigned Bits  = 256;
    static constexpr unsigned Lanes = Bits / aie::detail::type_bits_v<T>;
    T* p = dest;
    // Assume n is divisible by Lanes
    for (unsigned i = 0; i < n / Lanes; ++i)
    {
        aie::vector<T, Lanes> v = aie::load_v<Lanes>(src);
        aie::store_v(p, v);
        src += Lanes;
        p   += Lanes;
    }
    return dest;
}
//![Aligned memcpy]

//![Unaligned memcpy]
template <typename T>
T* unaligned_memcpy(T* __restrict dest, const T* src, unsigned n, unsigned dest_align = 1, unsigned src_align = 1)
{
    // dest_align, src_align indicate to how many elements their respective pointers are aligned
    // 256b vectors are used in this example
    static constexpr unsigned Bits  = 256;
    static constexpr unsigned Lanes = Bits / aie::detail::type_bits_v<T>;
    T* p = dest;
    // Assume n is divisible by Lanes
    for (unsigned i = 0; i < n / Lanes; ++i)
    {
        aie::vector<T, Lanes> v = aie::load_unaligned_v<Lanes>(src, src_align);
        aie::store_unaligned_v(p, v, dest_align);
        src += Lanes;
        p   += Lanes;
    }
    return dest;
}
//![Unaligned memcpy]

void load_floor() {
    //![Load floor]
    alignas(aie::vector_decl_align) static int16 data[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    
    int16 *ptr = &data[3];
    
    aie::vector<int16, 16> v = aie::load_floor_v<16>(ptr, 16);
    aie::print(v, true); // Prints 0 1 2 ... 13 14 15
    //![Load floor]
}
