#  Raspberry Pi RP2350 MicroROS tests (RP2040 backport) - The ROS Robot Project Research.
#  Copyright 2025 Samyar Sadat Akhavi
#  Written by Samyar Sadat Akhavi, 2025.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https: www.gnu.org/licenses/>.


include($ENV{PICO_SDK_PATH}/cmake/preload/toolchains/util/find_compiler.cmake)
set(PICO_GCC_TRIPLE "arm-none-eabi")

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CROSSCOMPILING 1)
set(CMAKE_SYSTEM_PROCESSOR cortex-m0plus)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

if (NOT PICO_GCC_TRIPLE)
    if (DEFINED ENV{PICO_GCC_TRIPLE})
        set(PICO_GCC_TRIPLE $ENV{PICO_GCC_TRIPLE})
        message("PICO_GCC_TRIPLE set from environment: $ENV{PICO_GCC_TRIPLE}")
    else()
        set(PICO_GCC_TRIPLE arm-none-eabi)
        message("PICO_GCC_TRIPLE defaulted to arm-none-eabi")
    endif()
endif()

pico_find_compiler(PICO_COMPILER_CC ${PICO_GCC_TRIPLE}-gcc)
pico_find_compiler(PICO_COMPILER_CXX ${PICO_GCC_TRIPLE}-g++)
set(CMAKE_C_COMPILER ${PICO_COMPILER_CC} CACHE FILEPATH "C compiler")
set(CMAKE_CXX_COMPILER ${PICO_COMPILER_CXX} CACHE FILEPATH "C++ compiler")

set(CMAKE_C_COMPILER_WORKS 1 CACHE INTERNAL "")
set(CMAKE_CXX_COMPILER_WORKS 1 CACHE INTERNAL "")
    
set(FLAGS "-O2 -mcpu=cortex-m0plus -mthumb -ffunction-sections -fdata-sections -fno-exceptions -nostdlib -D'RCUTILS_LOG_MIN_SEVERITY=RCUTILS_LOG_MIN_SEVERITY_NONE'" CACHE STRING "" FORCE)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS} -DCLOCK_MONOTONIC=0 -D'__attribute__(x)='" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS} -fno-rtti -DCLOCK_MONOTONIC=0 -D'__attribute__(x)='" CACHE STRING "" FORCE)
add_compile_options(
    "$<$<COMPILE_LANGUAGE:C>:-std=c11>"
    "$<$<COMPILE_LANGUAGE:CXX>:-std=c++14>"
)

# FreeRTOS Kernel
add_compile_definitions(PLATFORM_NAME_FREERTOS)
set(FREERTOS_KERNEL_PATH "${CMAKE_CURRENT_LIST_DIR}/../libfreertos/FreeRTOS-Kernel")
include_directories("${FREERTOS_KERNEL_PATH}/include") 
include_directories("${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/include")

# FreeRTOS config
set(FREERTOS_CONFIG_DIR "${CMAKE_CURRENT_LIST_DIR}/../libfreertos/Config")
include_directories("${FREERTOS_CONFIG_DIR}")

# Raspberry Pi Pico SDK headers
set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
add_compile_definitions(PICO_RP2040)
include_directories("${PICO_SDK_PATH}/src/common/pico_base_headers/include")
include_directories("${PICO_SDK_PATH}/src/rp2_common/hardware_base/include")
include_directories("${PICO_SDK_PATH}/src/rp2_common/hardware_sync/include")
include_directories("${PICO_SDK_PATH}/src/rp2_common/hardware_sync_spin_lock/include")
include_directories("${PICO_SDK_PATH}/src/rp2_common/pico_platform_compiler/include")
include_directories("${PICO_SDK_PATH}/src/rp2_common/pico_platform_sections/include")
include_directories("${PICO_SDK_PATH}/src/rp2_common/pico_platform_panic/include")
include_directories("${PICO_SDK_PATH}/src/rp2040/hardware_regs/include")
include_directories("${PICO_SDK_PATH}/src/rp2040/pico_platform/include")
include_directories("${CMAKE_CURRENT_LIST_DIR}/../build/generated/pico_base")   # Auto-generated headers

# Ignore all warnings. This is not recommended practice, but most warnings here are
# non-critical or false positives, and there's not much I can do about them anyway.
add_definitions(-w)