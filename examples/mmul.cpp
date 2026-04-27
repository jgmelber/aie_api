// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#include <aie_api/aie.hpp>

//![Blocked matrix multiplication]
template <unsigned M, unsigned K, unsigned N>
void mmul_blocked(unsigned rowA, unsigned colA, unsigned colB,
                  const int16 * __restrict pA, const int16 * __restrict pB, int16 * __restrict pC)
{
   using MMUL = aie::mmul<M, K, N, int16, int16>;

   for (unsigned z = 0; z < rowA / M; z += 2) chess_loop_range(2,) {
       int16 * __restrict pC1 = pC + (      z * colB +       0) * MMUL::size_C;
       int16 * __restrict pC2 = pC + ((z + 1) * colB +       0) * MMUL::size_C;

       for (unsigned j = 0; j < colB / N; j += 2) chess_loop_range(2,) {
           const int16 * __restrict pA1 = pA + (      z * colA +       0) * MMUL::size_A;
           const int16 * __restrict pA2 = pA + ((z + 1) * colA +       0) * MMUL::size_A;
           const int16 * __restrict pB1 = pB + (      0 * colB +       j) * MMUL::size_B;
           const int16 * __restrict pB2 = pB + (      0 * colB + (j + 1)) * MMUL::size_B;

           aie::vector<int16, MMUL::size_A> A0 = aie::load_v<MMUL::size_A>(pA1); pA1 += MMUL::size_A;
           aie::vector<int16, MMUL::size_A> A1 = aie::load_v<MMUL::size_A>(pA2); pA2 += MMUL::size_A;
           aie::vector<int16, MMUL::size_B> B0 = aie::load_v<MMUL::size_B>(pB1); pB1 += MMUL::size_B * colB;
           aie::vector<int16, MMUL::size_B> B1 = aie::load_v<MMUL::size_B>(pB2); pB2 += MMUL::size_B * colB;

           MMUL C00; C00.mul(A0, B0);
           MMUL C01; C01.mul(A0, B1);
           MMUL C10; C10.mul(A1, B0);
           MMUL C11; C11.mul(A1, B1);

           for (unsigned i = 1; i < colA / K; ++i) chess_prepare_for_pipelining chess_loop_range(3,) {
               A0 = aie::load_v<MMUL::size_A>(pA1); pA1 += MMUL::size_A;
               A1 = aie::load_v<MMUL::size_A>(pA2); pA2 += MMUL::size_A;
               B0 = aie::load_v<MMUL::size_B>(pB1); pB1 += MMUL::size_B * colB;
               B1 = aie::load_v<MMUL::size_B>(pB2); pB2 += MMUL::size_B * colB;

               C00.mac(A0, B0);
               C01.mac(A0, B1);
               C10.mac(A1, B0);
               C11.mac(A1, B1);
           }

           aie::store_v(pC1, C00.template to_vector<int16>()); pC1 += MMUL::size_C;
           aie::store_v(pC1, C01.template to_vector<int16>()); pC1 += MMUL::size_C;
           aie::store_v(pC2, C10.template to_vector<int16>()); pC2 += MMUL::size_C;
           aie::store_v(pC2, C11.template to_vector<int16>()); pC2 += MMUL::size_C;
       }
   }
}
//![Blocked matrix multiplication]
