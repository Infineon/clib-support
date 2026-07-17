# CLib Support Library - Thread Safety Hooks for C Library Functions

## Overview

The **CLib Support Library** provides hooks to make C library functions such as `malloc` and `free` thread-safe for FreeRTOS and ThreadX RTOSes. It supports GCC Newlib, ARM C library, IAR C library, and LLVM ARM picolibc toolchains, and also implements the standard `time()` function backed by the microcontroller Real-Time Clock (RTC).

Most application developers will not interact with this library directly; it is automatically included as a dependency of ModusToolbox™ BSPs. For full API documentation, see the [Reference Guide](https://infineon.github.io/clib-support/html/index.html).

## Public API

### Startup Initialization

For GCC, IAR, and LLVM ARM toolchains, the startup code must call `cy_toolchain_init()` after static data initialization and before static constructors. This is done automatically for PSoC™ devices.

```c
void cy_toolchain_init(void);
```

### Time Functions

When the HAL is present and an RTC-capable device is used, the standard `time()` function returns seconds elapsed since the epoch, sourced from the microcontroller RTC.

#### `mtb_clib_support_init`

```c
/* HAL API version >= 3 */
void mtb_clib_support_init(mtb_hal_rtc_t* rtc);

/* Earlier HAL versions */
void mtb_clib_support_init(cyhal_rtc_t* rtc);
```

Initializes the CLIB support library with an RTC handle.

| Parameter | Type | Description |
|-----------|------|-------------|
| `rtc` | `mtb_hal_rtc_t*` / `cyhal_rtc_t*` | Pointer to the RTC handle. May be `NULL` if `time()` will not be used. |

**Note:** For `MTB_HAL_API_VERSION >= 3`, `mtb_clib_support_init` must be called before `time()` is invoked. If not called, `time()` will assert (when asserts are enabled) and return a default time value.

#### `mtb_clib_support_get_rtc`

```c
/* HAL API version >= 3 */
mtb_hal_rtc_t* mtb_clib_support_get_rtc(void);

/* Earlier HAL versions */
cyhal_rtc_t* mtb_clib_support_get_rtc(void);
```

Returns a pointer to the RTC handle used by the CLIB support library.

| | Type | Description |
|---|------|-------------|
| **Returns** | `mtb_hal_rtc_t*` / `cyhal_rtc_t*` | Pointer to the RTC handle, or `NULL` if not initialized. |

**Deprecated macros:** `cy_set_rtc_instance` is replaced by `mtb_clib_support_init`; `cy_get_rtc_instance` is replaced by `mtb_clib_support_get_rtc`.

#### Setting Up Time Support

To configure `time()` with an existing RTC handle:

1. Initialize the RTC handle using the PDL API.
2. Write the current time to the RTC using the PDL API.
3. Pass the RTC handle to `mtb_clib_support_init()`.
4. Call `time()` to retrieve the current time.

## Features

The library provides the following C library hook implementations:

**GCC Newlib:** `_sbrk`, `__malloc_lock`, `__malloc_unlock`, `__env_lock`, `__env_unlock`

**ARM C library:** `_platform_post_stackheap_init`, `__user_perthread_libspace`, `_mutex_initialize`, `_mutex_acquire`, `_mutex_release`, `_mutex_free`, `_sys_exit`, `$Sub$$_sys_open`, `_ttywrch`, `_sys_command_string`

**IAR C library:** `__aeabi_read_tp` (FreeRTOS), `_reclaim_reent`, `__iar_system_Mtxinit` (FreeRTOS), `__iar_system_Mtxlock` (FreeRTOS), `__iar_system_Mtxunlock` (FreeRTOS), `__iar_system_Mtxdst` (FreeRTOS), `__iar_file_Mtxinit`, `__iar_file_Mtxlock`, `__iar_file_Mtxunlock`, `__iar_file_Mtxdst`, `__iar_Initdynamiclock`, `__iar_Lockdynamiclock`, `__iar_Unlockdynamiclock`, `__iar_Dstdynamiclock`, `__close`, `__lseek`, `remove`

**LLVM ARM picolibc:** `__retarget_lock_init_recursive`, `__retarget_lock_init`, `__retarget_lock_close_recursive`, `__retarget_lock_close`, `__retarget_lock_acquire_recursive`, `__retarget_lock_acquire`, `__retarget_lock_release_recursive`, `__retarget_lock_release`, `__aeabi_errno_addr`, `_exit`

**C++:** `__cxa_guard_acquire`, `__cxa_guard_abort`, `__cxa_guard_release`

**Time:** `time()` from `<time.h>`, backed by the microcontroller RTC (when HAL is present)

For reference documentation on the hook interfaces defined by each toolchain, see:

- **ARM:** [Multithreaded support in ARM C libraries](https://developer.arm.com/docs/100073/0614/the-arm-c-and-c-libraries/multithreaded-support-in-arm-c-libraries/management-of-locks-in-multithreaded-applications)
- **GCC Newlib:** [malloc lock documentation](https://sourceware.org/newlib/libc.html#g_t_005f_005fmalloc_005flock)
- **IAR:** [IAR EWARM Development Guide](http://supp.iar.com/filespublic/updinfo/011261/arm/doc/EWARM_DevelopmentGuide.ENU.pdf)
- **LLVM ARM picolibc:** [Locking documentation](https://github.com/picolibc/picolibc/blob/main/doc/locking.md)

## Requirements

### FreeRTOS

The following options must be enabled in `FreeRTOSConfig.h`:

- `configUSE_MUTEXES`
- `configUSE_RECURSIVE_MUTEXES`
- `configSUPPORT_STATIC_ALLOCATION`

To enable Thread Local Storage for GCC Newlib, also enable `configUSE_NEWLIB_REENTRANT`.

### ThreadX

The following option must be enabled:

- `TX_DISABLE_REDUNDANT_CLEARING`

When building with IAR:

- The `--threaded_lib` linker argument must be provided (done automatically with psoc6make 1.3.1 and later).
- `TX_ENABLE_IAR_LIBRARY_SUPPORT` must be defined in the application to enable the ThreadX IAR-specific port for clib thread safety.

For more information about porting with ThreadX, see the [ThreadX GitHub](https://github.com/azure-rtos/threadx/tree/master/ports).

### LLVM ARM

LLVM ARM (picolibc) is currently supported for use with FreeRTOS only.

**Note:** The picolibc `printf` implementation does not provide thread safety for streams shared between multiple threads. Applications must implement their own synchronization for `printf` in multithreaded contexts.

## Related Middleware and Dependencies

**This library is used by:**

- [secure-sockets](https://github.com/Infineon/secure-sockets) - TLS/TCP socket abstraction; uses clib-support for RTC-backed system time during certificate validation
- [wifi-core-freertos-lwip-mbedtls](https://github.com/Infineon/wifi-core-freertos-lwip-mbedtls) - Wi-Fi middleware core stack (FreeRTOS + lwIP + Mbed TLS)
- [ethernet-core-freertos-lwip-mbedtls](https://github.com/Infineon/ethernet-core-freertos-lwip-mbedtls) - Ethernet middleware core stack (FreeRTOS + lwIP + Mbed TLS)
- [mtb-template-cat5](https://github.com/Infineon/mtb-template-cat5) - ModusToolbox project template for CAT5 devices

**This library depends on:**

- [FreeRTOS](https://github.com/FreeRTOS/FreeRTOS-Kernel) or [ThreadX](https://github.com/azure-rtos/threadx) - RTOS providing mutex and thread management (one required)

## Release Notes and Changelog

- RELEASE.md  - Detailed release notes for all versions

## License

- LICENSE - Apache License 2.0 (applies to all source code)
- EULA - Infineon End User License Agreement

---

## Copyright

(c) 2020-2026, Infineon Technologies AG, or an affiliate of Infineon Technologies AG. All rights reserved.
