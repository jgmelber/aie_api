.. _README.rst:

..
    comment:: SPDX-License-Identifier: MIT
    comment:: Copyright (C) 2022 Xilinx, Inc.
    comment:: Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

=================================
AIE API
=================================

AIE API is a portable programming interface for AI Engine accelerator cores, implemented as a C++ header-only library.
It defines types and operations that get translated into efficient low-level intrinsics. This library also provides
higher-level abstractions, such as iterators and multi-dimensional arrays.

Using AIE API in your application
=================================

Since AIE API is a header-only library, you only need to include the corresponding headers in your application.

.. code-block:: cpp

    #include <aie_api/aie.hpp>

This should be sufficient to enable most the functionality in the library. Some utility functions, such as `aie::print` or `aie::unroll_for`, require including `aie_api/utils.hpp` header. This header file should only be used to compile kernel code and thus shouldn't be included by ADF graph code.

aiecompiler
-----------

AIE API include paths are added automatically when compiling a design using `aiecompiler`. If you need to use a
different implementation in an external directory, you can add the following option to the aiecompiler command-line.

.. code-block::

   --aie-api-path=/path/to/include/aie_api

Vitis GUI
---------

In order to add the AIE API headers to the include path of your application follow the next steps:

- Right-click on the aiengine application item in the Explorer.
- Go to C/C++ Build | Settings.
- In Tool Settings, click on Input Spec
- Add a new entry for the aie_api/include subfolder.
- Click on Apply and Close

Documentation
=============

You can browse the Doxygen documentation under the doc folder.
