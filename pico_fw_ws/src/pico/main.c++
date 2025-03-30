/*
    Raspberry Pi RP2350 MicroROS tests - The ROS Robot Project Research.
    Basic dual-core publisher/subscriber example with FreeRTOS for RP2350.
    Copyright 2025 Samyar Sadat Akhavi
    Written by Samyar Sadat Akhavi, 2025.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https: www.gnu.org/licenses/>.
*/

#define PRINTF_BUFFER_SIZE  256
#define TASK_STACK_SIZE     1024
#define USE_UART_TRANSPORT  0   // Set to 1 to use UART transport, 0 for USB transport.

// ---- Libraries ----
#include <stdio.h>
#include <random>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/int32.h>
#include <rmw_microros/rmw_microros.h>
#include "pico_uart_transport.c"
#include "uros_allocators.c"
#include "pico/stdio/driver.h"
#include <stdarg.h>
#include <ctime>
#include <hardware/watchdog.h>


// ---- Logging ----
void log_printf(const char *format, ...) {
    char buffer[PRINTF_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    
    int length = vsnprintf(buffer, PRINTF_BUFFER_SIZE, format, args);
    if (length > 0) {
        #if USE_UART_TRANSPORT
            stdio_usb.out_chars(buffer, length);
        #else
            stdio_uart.out_chars(buffer, length);
        #endif
    }
    
    va_end(args);
}

#define LOG_DEBUG(format, ...) log_printf("[DEBUG][%s:%d] " format "\r\n", __func__, __LINE__, ##__VA_ARGS__)


// ---- Return Checkers ----
#define RCCHECK(fn) { \
    rcl_ret_t temp_rc = fn; \
    if((temp_rc != RCL_RET_OK)) { \
        LOG_DEBUG("Failed status (RCL) with error code %d", (int)temp_rc); \
        return false; \
    } \
}

#define RMWCHECK(fn) { \
    rmw_ret_t temp_rc = fn; \
    if((temp_rc != RMW_RET_OK)) { \
        LOG_DEBUG("Failed status (RMW) with error code %d", (int)temp_rc); \
        return false; \
    } \
}

#define RCSOFTCHECK(fn) { \
    rcl_ret_t temp_rc = fn; \
    if((temp_rc != RCL_RET_OK)) { \
        LOG_DEBUG("Non-critical failure (RCL) with error code %d", (int)temp_rc); \
    } \
}


// ---- Global Variables ----

// Agent state enum
enum AGENT_STATE {
    WAITING_FOR_AGENT,
    AGENT_AVAILABLE,
    AGENT_CONNECTED,
    AGENT_DISCONNECTED
};

// RCL variables
rcl_publisher_t publisher_core0, publisher_core1;
rcl_subscription_t subscriber_core0, subscriber_core1;
std_msgs__msg__Int32 msg_core0, msg_core1;
std_msgs__msg__Int32 sub_msg_core0, sub_msg_core1;
rclc_executor_t executor_core0, executor_core1;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;

// FreeRTOS task handles
TaskHandle_t task_handle_core0 = NULL, task_handle_core1 = NULL, task_handle_uros_state = NULL;


// ---- Functions ----
void sub_cb_core0(const void * msgin) {
    const std_msgs__msg__Int32 * msg = (const std_msgs__msg__Int32 *)msgin;
    LOG_DEBUG("Core %d received: %d", get_core_num(), msg->data);
}

void sub_cb_core1(const void * msgin) {
    const std_msgs__msg__Int32 * msg = (const std_msgs__msg__Int32 *)msgin;
    LOG_DEBUG("Core %d received: %d", get_core_num(), msg->data);
}

bool init_microros() {
    LOG_DEBUG("Initializing MicroROS...");
    
    allocator = rcl_get_default_allocator();
    LOG_DEBUG("Allocator initialized.");

    RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
    RCCHECK(rclc_node_init_default(&node, "pico_node", "", &support));
    LOG_DEBUG("ROS node and context initialized successfully.");

    RCCHECK(rclc_executor_init(&executor_core0, &support.context, 1, &allocator));
    RCCHECK(rclc_executor_init(&executor_core1, &support.context, 1, &allocator));
    LOG_DEBUG("Executors initialized successfully.");

    const rosidl_message_type_support_t *int32_type_supp = ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32);

    RCCHECK(rclc_publisher_init_default(&publisher_core0, &node, int32_type_supp, "core0_pub"));
    RCCHECK(rclc_publisher_init_default(&publisher_core1, &node, int32_type_supp, "core1_pub"));
    LOG_DEBUG("Publishers created successfully.");

    RCCHECK(rclc_subscription_init_default(&subscriber_core0, &node, int32_type_supp, "core0_sub"));
    RCCHECK(rclc_subscription_init_default(&subscriber_core1, &node, int32_type_supp, "core1_sub"));
    LOG_DEBUG("Subscriptions created successfully.");

    RCCHECK(rclc_executor_add_subscription(&executor_core0, &subscriber_core0, &sub_msg_core0, &sub_cb_core0, ON_NEW_DATA));
    RCCHECK(rclc_executor_add_subscription(&executor_core1, &subscriber_core1, &sub_msg_core1, &sub_cb_core1, ON_NEW_DATA));
    LOG_DEBUG("Subscriptions added to executors successfully.");

    return true;
}

bool ping_agent() {
    return rmw_uros_ping_agent(100, 10) == RMW_RET_OK;
}

void reset() {
    watchdog_disable();
    watchdog_enable(1, true);
    while(1);  // Wait for the watchdog to reset the system.
}


// ---- FreeRTOS Hook Callbacks ----
void vApplicationMallocFailedHook(void) {
    LOG_DEBUG("ERROR: Malloc failed! Resetting...");
    configASSERT(false);
    reset();
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    LOG_DEBUG("ERROR: Stack overflow in task: '%s'! Resetting...", pcTaskName);
    configASSERT(false);
    reset();
}


// ---- Task Functions ----
void task_core0(void *params) {
    LOG_DEBUG("Core %d task starting...", get_core_num());
    uint32_t iter = 0;

    while (true) {
        if (iter % 10 == 0) {
            msg_core0.data = rand() % 100;
            rcl_ret_t pub_ret = rcl_publish(&publisher_core0, &msg_core0, NULL);
            
            if (pub_ret == RCL_RET_OK) {
                LOG_DEBUG("Core %d published: %d", get_core_num(), msg_core0.data);
            } else {
                LOG_DEBUG("Core %d failed to publish with error: %d", get_core_num(), (int)pub_ret);
            }
        }

        RCSOFTCHECK(rclc_executor_spin_some(&executor_core0, RCL_MS_TO_NS(20)));
        
        iter++;
        vTaskDelay(pdMS_TO_TICKS(80));
    }
}

void task_core1(void *params) {
    LOG_DEBUG("Core %d task starting...", get_core_num());
    uint32_t iter = 0;

    while (true) {
        if (iter % 5 == 0) {
            msg_core1.data = rand() % 100;
            rcl_ret_t pub_ret = rcl_publish(&publisher_core1, &msg_core1, NULL);
            
            if (pub_ret == RCL_RET_OK) {
                LOG_DEBUG("Core %d published: %d", get_core_num(), msg_core1.data);
            } else {
                LOG_DEBUG("Core %d failed to publish with error: %d", get_core_num(), (int)pub_ret);
            }
        }

        RCSOFTCHECK(rclc_executor_spin_some(&executor_core1, RCL_MS_TO_NS(20)));
        
        iter++;
        vTaskDelay(pdMS_TO_TICKS(80));
    }
}

void uros_state_task(void *params) {
    AGENT_STATE current_agent_state = WAITING_FOR_AGENT;
    LOG_DEBUG("Waiting for agent...");

    while (true) {
        switch (current_agent_state) {
            case WAITING_FOR_AGENT:
                if (ping_agent()) {
                    current_agent_state = AGENT_AVAILABLE;
                }
                break;

            case AGENT_AVAILABLE:
                LOG_DEBUG("Agent available!");

                if (init_microros()) {
                    current_agent_state = AGENT_CONNECTED;
                    LOG_DEBUG("MicroROS initialized successfully.");

                    BaseType_t task0_created = xTaskCreate(task_core0, "core0_task", TASK_STACK_SIZE, NULL, 1, &task_handle_core0);
                    BaseType_t task1_created = xTaskCreate(task_core1, "core1_task", TASK_STACK_SIZE, NULL, 1, &task_handle_core1);
                    vTaskCoreAffinitySet(task_handle_core0, (1 << 0));
                    vTaskCoreAffinitySet(task_handle_core1, (1 << 1));

                    if (task0_created != pdPASS || task1_created != pdPASS) {
                        LOG_DEBUG("Failed to create tasks!");
                        current_agent_state = AGENT_DISCONNECTED;
                    }
                } else {
                    current_agent_state = AGENT_DISCONNECTED;
                }
                break;

            case AGENT_CONNECTED:
                if (!ping_agent()) {
                    current_agent_state = AGENT_DISCONNECTED;
                    LOG_DEBUG("Agent disconnected!");
                }
                break;

            case AGENT_DISCONNECTED:
                LOG_DEBUG("Cleaning up resources and preparing for reset...");
                
                if (task_handle_core0 != NULL) {
                    vTaskDelete(task_handle_core0);
                    task_handle_core0 = NULL;
                }

                if (task_handle_core1 != NULL) {
                    vTaskDelete(task_handle_core1);
                    task_handle_core1 = NULL;
                }

                portDISABLE_INTERRUPTS();
                vTaskSuspendAll();
                LOG_DEBUG("Scheduler suspended.");

                // Here we have to set-up a watchdog timer to reset the system if any of the
                // _fini() calls hang for more than 1000ms. This is because these functions will
                // hang if they are called after the trasnport has disconnected from the micro-ROS agent.
                watchdog_enable(1000, true);

                RCSOFTCHECK(rcl_publisher_fini(&publisher_core0, &node)); watchdog_update();
                RCSOFTCHECK(rcl_publisher_fini(&publisher_core1, &node)); watchdog_update();
                RCSOFTCHECK(rcl_subscription_fini(&subscriber_core0, &node)); watchdog_update();
                RCSOFTCHECK(rcl_subscription_fini(&subscriber_core1, &node)); watchdog_update();
                RCSOFTCHECK(rclc_executor_fini(&executor_core0)); watchdog_update();
                RCSOFTCHECK(rclc_executor_fini(&executor_core1)); watchdog_update();
                RCSOFTCHECK(rcl_node_fini(&node)); watchdog_update();
                RCSOFTCHECK(rclc_support_fini(&support)); watchdog_update();
                
                LOG_DEBUG("Cleanup completed, resetting...");
                reset();
        }
    
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


// ---- Main Function ----
int main() 
{
    stdio_usb_init();
    while (!stdio_usb_connected());
    stdio_uart_init();

    #if USE_UART_TRANSPORT
        stdio_filter_driver(&stdio_uart);
    #else
        stdio_filter_driver(&stdio_usb);
    #endif

    LOG_DEBUG("STDIO initialized.");

    srand(to_us_since_boot(get_absolute_time()));

    rcl_allocator_t rtos_allocators = rcutils_get_zero_initialized_allocator();
    rtos_allocators.allocate = uros_rtos_allocate;
    rtos_allocators.deallocate = uros_rtos_deallocate;
    rtos_allocators.reallocate = uros_rtos_reallocate;
    rtos_allocators.zero_allocate = uros_rtos_zero_allocate;
    
    if (!rcutils_set_default_allocator(&rtos_allocators)) {
        LOG_DEBUG("Failed to set allocators!");
        return 0;
    }

    RMWCHECK(rmw_uros_set_custom_transport(
        true,
        NULL,
        pico_serial_transport_open,
        pico_serial_transport_close,
        pico_serial_transport_write,
        pico_serial_transport_read
    ));

    BaseType_t task_created = xTaskCreate(uros_state_task, "uros_state_task", TASK_STACK_SIZE, NULL, 1, &task_handle_uros_state);
    vTaskCoreAffinitySet(task_handle_uros_state, (1 << 0));

    if (task_created != pdPASS) {
        LOG_DEBUG("Failed to create MicroROS state task!");
    } else {
        LOG_DEBUG("Starting FreeRTOS scheduler...");
        vTaskStartScheduler();
    }

    return 0;
}