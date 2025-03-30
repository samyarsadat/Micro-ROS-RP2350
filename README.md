<h1 align="center">Micro-ROS RP2350</h1>

<p align="center">
	<br>
	<a href="https://www.ros.org"><img src="https://github.com/samyarsadat/ROS-Robot/raw/stage-1/assets/logos/ROS_logo.svg"></a><br>
	<br>
	<br>
	<a href="LICENSE"><img src="https://img.shields.io/github/license/samyarsadat/Micro-ROS-RP2350?color=blue"></a>
	|
	<a href="../../issues"><img src="https://img.shields.io/github/issues/samyarsadat/Micro-ROS-RP2350"></a>
	<br><br>
</p>

----
This repository contains test firmware and build configuration for building and running micro-ROS on the Raspberry Pi RP2350 (ARM Cortex-M33 cores) microcontroller, with multi-threading and Symmetric Multi Processing (SMP) support.


## Overview

### Comparison to micro-ROS on the RP2040
I used code and configuration files I had prepared for running micro-ROS on the RP2040 as a base for the RP2350. I did however have to make a number of changes, not only because of the new chip, but also because I had to finally migrate away from version `1.5.1` of the Pico SDK to version `2.x.x`.

Some header files were moved around, and some were separated and re-structured. This is relevant for the micro-ROS build configuration, as because we're building with FreeRTOS support (and the FreeRTOS port `#include`s several Pico SDK headers), we have to manually include specific directories from the Pico SDK in the CMake configuration.

Other than that, most other changes to the micro-ROS build configuration were ones made as a result of the different CPU architecture used by the RP2350's ARM cores (ARM Cortex-M0+ for RP2040 vs. ARM Cortex-M33 for RP2350), we're not using the Hazard 3 Risc-V cores here.

#### Dual-core operation & SMP support
When testing micro-ROS with FreeRTOS on the RP2040 months ago, I was unable to get multi-core operation to work. Multi-threading on core 0 worked fine, if I called any RCL functions from core 1, the processor would deadlock. 

I did not spend much effort to try to fix this issue, though I suspect it was a bug in some locking code somewhere, so I created an abstraction that allowed me to "publish" messages from both cores, whilst only calling `rcl_publish()` from core 0. This was not ideal (and resulted in some message lifetime issues, as message publication was asynchronous), but it worked.

I am happy to report, however, that everything functions perfectly on the new RP2350, and that there is no need for such an abstraction. The example provided in this repository publishes from, and runs an executor on both cores.

## The Example
The example program provided in this repository is meant to be a starting point, and a demonstration of multi-threaded and multi-core operations using micro-ROS and FreeRTOS on the RP2350. It doesn't rely on any external libraries (other than the FreeRTOS Kernel, micro-ROS, and the standard Pico C SDK libraries, of course).

### What it does
The example program publishes a randomly generated, 2-digit integer from both cores to two topics (`core0_pub` and `core1_pub`, with message type `std_msgs/msg/Int32`). It also subscribes to two topics, one for each core (`core0_sub` and `core1_sub`, with message type `std_msgs/msg/Int32`). The data of any messages published on `core0_sub` and `core1_sub` are displayed using the debug log output.

#### Executors
Subscription callbacks (and service server callbacks) are called from the executor `spin` functions, so which core the callbacks are called from depends on which core the executor is being spun from. For this reason, we have two executor instances in this program. One is spun by the core 0 task, and the other by the core 1 task. Each executor is subscribed to one topic (`core0_sub` and `core1_sub` for core 0 and core 1, respectively).

#### Debug log output
By default, micro-ROS data is sent using serial over the Pico's USB port, and debug log output is sent over `UART 0` (pins `GP0` and `GP1`). This behavior can be changed if needed by setting `USE_UART_TRANSPORT` to `1`.

### Micro-ROS agent
There will be two scripts in the working directory (well, they're located in a folder called `uros_agent`) of the devcontainer. You must first run the agent creation script (`bash uros_agent_create.sh`), and then to run the agent you can use the run script (`bash uros_agent_run.sh ttyACM0`, replace `ttyACM0` with whichever port your Pico is connected to).

<br>

## Contact
You can contact me via e-mail.<br>
E-mail: samyarsadat@gigawhat.net<br>
<br>
If you think that you have found a bug or issue please report it <a href="../../issues">here</a>.

<br>


## Credits
| Role           | Name                                                             |
| -------------- | ---------------------------------------------------------------- |
| Lead Developer | <a href="https://github.com/samyarsadat">Samyar Sadat Akhavi</a> |

<br>

Copyright © 2025 Samyar Sadat Akhavi.