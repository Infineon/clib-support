# CLib Support Library

## Overview

The CLib Support Library provides the necessary hooks to make C library functions such as malloc and free thread safe. This implementation supports FreeRTOS and ThreadX and requires the appropriate RTOS to be present to build. For details on what this library provides see the toolchain specific documentation at:
* ARM: https://developer.arm.com/docs/100073/0614/the-arm-c-and-c-libraries/multithreaded-support-in-arm-c-libraries/management-of-locks-in-multithreaded-applications
* GCC: https://sourceware.org/newlib/libc.html#g_t_005f_005fmalloc_005flock
* IAR: http://supp.iar.com/filespublic/updinfo/011261/arm/doc/EWARM_DevelopmentGuide.ENU.pdf

The startup code must call cy_toolchain_init (for GCC and IAR). This must occur after static data initialization and before static constructors. This is done automatically for PSoC™ devices. See the PSoC™ startup files for an example.

In FreeRTOS to enable Thread Local Storage, configUSE_NEWLIB_REENTRANT must be enabled.

While this is specific to FreeRTOS and ThreadX, it can be used as a basis for supporting other RTOSes as well.

## FreeRTOS Requirements
To use this library, the following configuration options must be enabled in FreeRTOSConfig.h:
* configUSE_MUTEXES
* configUSE_RECURSIVE_MUTEXES
* configSUPPORT_STATIC_ALLOCATION

## ThreadX Requirements
To use this library, the following configuration option must be enabled:
* TX_DISABLE_REDUNDANT_CLEARING

When building with IAR, the '--threaded_lib' argument must be provided when linking. This is done automatically with psoc6make 1.3.1 and later.
Also, 'TX_ENABLE_IAR_LIBRARY_SUPPORT' must be defined in the application when building with IAR. This enables the ThreadX IAR-specific port for clib thread safety.
For more information about porting with ThreadX, see the ThreadX GitHub: https://github.com/azure-rtos/threadx/tree/master/ports

## Features
* GCC Newlib implementations for:
    * _sbrk
    * __malloc_lock
    * __malloc_unlock
    * __env_lock
    * __env_unlock
* ARM C library implementations for:
    * _platform_post_stackheap_init
    * __user_perthread_libspace
    * _mutex_initialize
    * _mutex_acquire
    * _mutex_release
    * _mutex_free
    * _sys_exit
    * $Sub$$_sys_open
    * _ttywrch
    * _sys_command_string
* IAR C library implementations for:
    * __aeabi_read_tp (FreeRTOS)
    * _reclaim_reent
    * __iar_system_Mtxinit (FreeRTOS)
    * __iar_system_Mtxlock (FreeRTOS)
    * __iar_system_Mtxunlock (FreeRTOS)
    * __iar_system_Mtxdst (FreeRTOS)
    * __iar_file_Mtxinit
    * __iar_file_Mtxlock
    * __iar_file_Mtxunlock
    * __iar_file_Mtxdst
    * __iar_Initdynamiclock
    * __iar_Lockdynamiclock
    * __iar_Unlockdynamiclock
    * __iar_Dstdynamiclock
    * __close
    * __lseek
    * remove
* C++ implementations for:
    * __cxa_guard_acquire
    * __cxa_guard_abort
    * __cxa_guard_release
* time function implementation from time.h

### Time Support Details
When using the HAL (CY_USING_HAL defined) the **time** function returns the time in seconds from microcontroller Real-Time Clock (RTC). Additionally, functions  **cy_set_rtc_instance** and **cy_get_rtc_instance** are provided to interact with the RTC instance used. There are two ways to set this up.
1. Setup using an existing RTC instance:
    * Application initializes RTC using **cyhal_rtc_init** API
    * Application writes the current time to RTC using **cyhal_rtc_write** API
    * Application sets the RTC instance using **cy_set_rtc_instance** API
    * Application calls **time** function to get the time from RTC
2. Allow time to initialize the RTC instance:
    * Application calls **time** function to get the time from RTC. The time function will initializes RTC
    * Application can get the RTC instance using **cy_get_rtc_instance** API
    * Application can write the current time to RTC using **cyhal_rtc_write** API

NOTE: In either flow, until the application calls **cyhal_rtc_write** the RTC will only be initialized with a default time. This means that time deltas will be accurate, but absolute times will not have any relation to the real time.

## More information
Use the following links for more information, as needed:
* [Reference Guide](https://infineon.github.io/clib-support/html/index.html)
* [Cypress Semiconductor, an Infineon Technologies Company](http://www.cypress.com)
* [Infineon GitHub](https://github.com/infineon)
* [ModusToolbox™](https://www.cypress.com/products/modustoolbox-software-environment)
* [RTC APIs](https://github.com/infineon/mtb-hal-cat1/blob/master/include/cyhal_rtc.h)

---
© Cypress Semiconductor Corporation (an Infineon company) or an affiliate of Cypress Semiconductor Corporation, 2020-2021.
