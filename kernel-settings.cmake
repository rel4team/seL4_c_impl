#
# Copyright 2023, Colias Group, LLC
#
# SPDX-License-Identifier: BSD-2-Clause
#

# Basis for seL4 kernel configuration

set(KernelArch riscv64 CACHE STRING "")
set(KernelMaxNumNodes 4 CACHE STRING "")
set(KernelIsUintr ON CACHE BOOL "")
set(KernelPlatform spike CACHE STRING "")
set(KernelSel4Arch riscv64 CACHE STRING "")
set(KernelVerificationBuild OFF CACHE BOOL "")
set(KernelWordSize 64 CACHE STRING "")
