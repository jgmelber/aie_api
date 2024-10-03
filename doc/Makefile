GCC_ROOT ?= /tools/batonroot/rodin/devkits/lnx64/gcc-8.3.0
CLANG_ROOT ?= /wrk/hmca/shared/ext/clang-14.0.3
DOXYGEN ?= /wrk/hmca/shared/ext/doxygen-1.9.7/bin/doxygen

LIBS = $(CLANG_ROOT)/lib:$(GCC_ROOT)/lib64

export GCC_ROOT
export CLANG_ROOT

doc:
	LD_LIBRARY_PATH=$(LIBS):$(LD_LIBRARY_PATH) $(DOXYGEN) aie_api.doxygen
