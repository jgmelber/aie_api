# SPDX-License-Identifier: MIT
# Copyright (C) 2022 Xilinx, Inc.
# Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

GCC_ROOT ?=
CLANG_ROOT ?=
DOXYGEN ?=

LIBS = $(CLANG_ROOT)/lib:$(GCC_ROOT)/lib64

doc:
	LD_LIBRARY_PATH=$(LIBS):$(LD_LIBRARY_PATH) $(DOXYGEN) aie_api.doxygen
