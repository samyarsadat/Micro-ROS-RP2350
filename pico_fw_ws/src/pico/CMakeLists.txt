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



# ---- Pico firmware ----
message("Configuring Pico firmware build... (${PICO_NAME})")

# Add source files
add_executable(${PICO_NAME} main.c++
                            ../posix/functions.c)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PICO_NAME})

# Link to libraries
target_link_libraries(${PICO_NAME} micro_ros
                                   pico_stdlib
                                   pico_stdio_usb
                                   pico_stdio_uart
                                   pico_async_context_freertos
                                   FreeRTOS-Kernel
                                   FreeRTOS-Kernel-Heap4
)

# Link to include directories
target_include_directories(${PICO_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# Target-specific compile definitions
target_compile_definitions(${PICO_NAME} PRIVATE
    PICO_STACK_SIZE=0x1000
    PICO_USE_STACK_GUARDS=1
    PICO_DEFAULT_UART=0
    PICO_DEFAULT_UART_TX_PIN=0
    PICO_DEFAULT_UART_RX_PIN=1
)

# PicoTool binary information
pico_set_program_name(${PICO_NAME} ${PICO_NAME})
pico_set_program_version(${PICO_NAME} ${PROJECT_VERSION})
pico_set_program_description(${PICO_NAME} "The ROS Robot Project Research - RP2040 MicroROS Test Firmware")

# Enable STDIO over USB and UART
pico_enable_stdio_usb(${PICO_NAME} 1)
pico_enable_stdio_uart(${PICO_NAME} 1)

# CMake flags
set(FLAGS "${CMAKE_C_FLAGS} -mthumb -ffunction-sections -fdata-sections")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}")

# Compile definitions & options
add_compile_definitions(PICO_UART_ENABLE_CRLF_SUPPORT=0)
add_compile_definitions(PICO_STDIO_ENABLE_CRLF_SUPPORT=0)
add_compile_definitions(PICO_STDIO_DEFAULT_CRLF=0)
add_compile_options(-Wall -Wextra -Wpedantic)