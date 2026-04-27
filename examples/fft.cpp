// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#include <aie_api/aie.hpp>

extern const cint16 tw1[], tw2[], tw4[], tw8[], tw16[], tw32[],
                    tw64[], tw128[], tw256[], tw512[],
                    tw2_4[], tw8_16[], tw32_64[], tw128_256[];

//![Staged FFT]
void fft_1024pt(const cint16 * __restrict x,  // Input pointer
                unsigned shift_tw,            // Indicates the decimal point of the twiddles
                                              // e.g. The twiddle 1.0+0.0i can be represented with cint16(32767, 0) and a shift_tw of 15
                unsigned shift,               // Shift applied to apply to dit outputs
                bool inv,                     // Run inverse FFT
                cint16 * __restrict tmp,      // Scratch space for intermediate results
                cint16 * __restrict y         // Output pointer
               )
{
    aie::fft_dit_r2_stage<512>(x,   tw1,   1024, shift_tw, shift, inv, tmp);
    aie::fft_dit_r2_stage<256>(tmp, tw2,   1024, shift_tw, shift, inv, y);
    aie::fft_dit_r2_stage<128>(y,   tw4,   1024, shift_tw, shift, inv, tmp);
    aie::fft_dit_r2_stage<64> (tmp, tw8,   1024, shift_tw, shift, inv, y);
    aie::fft_dit_r2_stage<32> (y,   tw16,  1024, shift_tw, shift, inv, tmp);
    aie::fft_dit_r2_stage<16> (tmp, tw32,  1024, shift_tw, shift, inv, y);
    aie::fft_dit_r2_stage<8>  (y,   tw64,  1024, shift_tw, shift, inv, tmp);
    aie::fft_dit_r2_stage<4>  (tmp, tw128, 1024, shift_tw, shift, inv, y);
    aie::fft_dit_r2_stage<2>  (y,   tw256, 1024, shift_tw, shift, inv, tmp);
    aie::fft_dit_r2_stage<1>  (tmp, tw512, 1024, shift_tw, shift, inv, y);
}
//![Staged FFT]


//![512p FFT]
void fft_512pt(const cint16 * __restrict x,  // Input pointer
               unsigned shift_tw,            // Indicates the decimal point of the twiddles
                                             // e.g. The twiddle 1.0+0.0i can be represented with cint16(32767, 0) and a shift_tw of 15
               unsigned shift,               // Shift applied to apply to dit outputs
               bool inv,                     // Run inverse FFT
               cint16 * __restrict tmp,      // Scratch space for intermediate results
               cint16 * __restrict y         // Output pointer
               )
{
    aie::fft_dit_r2_stage<256>(x,   tw1,                     512, shift_tw, shift, inv, y);
    aie::fft_dit_r4_stage<64> (y,   tw2,   tw4,   tw2_4,     512, shift_tw, shift, inv, tmp);
    aie::fft_dit_r4_stage<16> (tmp, tw8,   tw16,  tw8_16,    512, shift_tw, shift, inv, y);
    aie::fft_dit_r4_stage<4>  (y,   tw32,  tw64,  tw32_64,   512, shift_tw, shift, inv, tmp);
    aie::fft_dit_r4_stage<1>  (tmp, tw128, tw256, tw128_256, 512, shift_tw, shift, inv, y);
}
//![512p FFT]


bool fft_complete() {
    //![FFT complete example]
    constexpr unsigned        n = 128;
    constexpr unsigned shift_tw = 15;
    constexpr unsigned    shift = 15;
    constexpr bool          inv = false;
    
    alignas(aie::vector_decl_align) static cint32 x[n];
    alignas(aie::vector_decl_align) static cint32 tmp[n];
    alignas(aie::vector_decl_align) static cint32 y[n];
    
    alignas(aie::vector_decl_align) static cint16 tw1    [] = {{ 32767,      0}};
    alignas(aie::vector_decl_align) static cint16 tw2    [] = {{ 32767,      0}, {     0, -32768}};
    alignas(aie::vector_decl_align) static cint16 tw4    [] = {{ 32767,      0}, { 23170, -23170}};
    alignas(aie::vector_decl_align) static cint16 tw2_4  [] = {{ 32767,      0}, {-23170, -23170}};
    alignas(aie::vector_decl_align) static cint16 tw8    [] = {{ 32767,      0}, { 30273, -12539}, { 23170, -23170}, { 12539, -30273},
                                                               {     0, -32768}, {-12539, -30273}, {-23170, -23170}, {-30273, -12539}};
    alignas(aie::vector_decl_align) static cint16 tw16   [] = {{ 32767,      0}, { 32138,  -6392}, { 30273, -12539}, { 27245, -18204},
                                                               { 23170, -23170}, { 18204, -27245}, { 12539, -30273}, {  6392, -32138}};
    alignas(aie::vector_decl_align) static cint16 tw8_16 [] = {{ 32767,      0}, { 27245, -18204}, { 12539, -30273}, { -6392, -32138},
                                                               {-23170, -23170}, {-32138,  -6392}, {-30273,  12539}, {-18204,  27245}};
    alignas(aie::vector_decl_align) static cint16 tw32   [] = {{ 32767,      0}, { 32610,  -3211}, { 32138,  -6392}, { 31357,  -9512},
                                                               { 30273, -12539}, { 28898, -15446}, { 27245, -18204}, { 25330, -20787},
                                                               { 23170, -23170}, { 20787, -25330}, { 18204, -27245}, { 15446, -28898},
                                                               { 12539, -30273}, {  9512, -31357}, {  6392, -32138}, {  3211, -32610},
                                                               {     0, -32768}, { -3211, -32610}, { -6392, -32138}, { -9512, -31357},
                                                               {-12539, -30273}, {-15446, -28898}, {-18204, -27245}, {-20787, -25330},
                                                               {-23170, -23170}, {-25330, -20787}, {-27245, -18204}, {-28898, -15446},
                                                               {-30273, -12539}, {-31357,  -9512}, {-32138,  -6392}, {-32610,  -3211}};
    alignas(aie::vector_decl_align) static cint16 tw64   [] = {{ 32767,      0}, { 32728,  -1607}, { 32610,  -3211}, { 32413,  -4808},
                                                               { 32138,  -6392}, { 31785,  -7961}, { 31357,  -9512}, { 30852, -11039},
                                                               { 30273, -12539}, { 29621, -14010}, { 28898, -15446}, { 28106, -16846},
                                                               { 27245, -18204}, { 26319, -19519}, { 25330, -20787}, { 24279, -22005},
                                                               { 23170, -23170}, { 22005, -24279}, { 20787, -25330}, { 19519, -26319},
                                                               { 18204, -27245}, { 16846, -28106}, { 15446, -28898}, { 14010, -29621},
                                                               { 12539, -30273}, { 11039, -30852}, {  9512, -31357}, {  7961, -31785},
                                                               {  6392, -32138}, {  4808, -32413}, {  3211, -32610}, {  1607, -32728}};
    alignas(aie::vector_decl_align) static cint16 tw32_64[] = {{ 32767,      0}, { 32413,  -4808}, { 31357,  -9512}, { 29621, -14010},
                                                               { 27245, -18204}, { 24279, -22005}, { 20787, -25330}, { 16846, -28106},
                                                               { 12539, -30273}, {  7961, -31785}, {  3211, -32610}, { -1607, -32728},
                                                               { -6392, -32138}, {-11039, -30852}, {-15446, -28898}, {-19519, -26319},
                                                               {-23170, -23170}, {-26319, -19519}, {-28898, -15446}, {-30852, -11039},
                                                               {-32138,  -6392}, {-32728,  -1607}, {-32610,   3211}, {-31785,   7961},
                                                               {-30273,  12539}, {-28106,  16846}, {-25330,  20787}, {-22005,  24279},
                                                               {-18204,  27245}, {-14010,  29621}, { -9512,  31357}, { -4808,  32413}};
    
    // Constant value input
    std::fill(std::begin(x), std::end(x), cint32(1, 0));

    aie::set_rounding(aie::rounding_mode::positive_inf);
    aie::set_saturation(aie::saturation_mode::saturate);
    
    aie::fft_dit_r2_stage<64>(x,   tw1,                   n, shift_tw, shift, inv, tmp);
    aie::fft_dit_r4_stage<16>(tmp, tw2,   tw4,   tw2_4,   n, shift_tw, shift, inv, y);
    aie::fft_dit_r4_stage<4> (y,   tw8,   tw16,  tw8_16,  n, shift_tw, shift, inv, tmp);
    aie::fft_dit_r4_stage<1> (tmp, tw32,  tw64,  tw32_64, n, shift_tw, shift, inv, y);
    
    for (unsigned i = 0; i < n; ++i) {
        printf("{%d, %d} ", y[i].real, y[i].imag);
    }
    //![FFT complete example]
    printf("\n");
    return y[0] == cint32(128, 0)
        && std::all_of(std::next(std::begin(y)), std::end(y), [](cint32 v) { return v == cint32(0, 0); });
}

int main() {
    bool pass = fft_complete();
    return pass? 0 : 1;
}
