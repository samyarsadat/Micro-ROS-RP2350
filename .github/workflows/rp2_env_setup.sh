#!/bin/bash
set -e

# Install dependencies
apt-get update \
&& apt-get install -y cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib build-essential \
&& apt-get install -y git python3 rsync \
&& rm -rf /var/lib/apt/lists/* && apt-get autoremove && apt-get autoclean

# "Install" the Pico SDK
mkdir /pico && cd /pico \
&& git clone https://github.com/raspberrypi/pico-sdk.git \
&& cd pico-sdk \
&& git submodule update --init

# Set Pico SDK path
export PICO_SDK_PATH="/pico/pico-sdk"
export PICO_COMPILER="pico_arm_gcc"