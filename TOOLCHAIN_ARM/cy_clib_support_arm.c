/***********************************************************************************************//**
 * \file cy_clib_support_arm.c
 *
 * \brief
 * ARM C library port for RTOS
 *
 ***************************************************************************************************
 * \copyright
 * Copyright 2018-2019 Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **************************************************************************************************/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmsis_compiler.h>
#include "reent.h"
#include "cy_mutex_pool.h"
#include "rt_misc.h"
#if defined(COMPONENT_THREADX)
#include "cy_pdl.h"
#endif // defined(COMPONENT_THREADX)

#if defined(COMPONENT_FREERTOS) && (configUSE_MUTEXES == 0 || configUSE_RECURSIVE_MUTEXES == 0 || \
                                    configSUPPORT_STATIC_ALLOCATION == 0)
#warning \
    configUSE_MUTEXES, configUSE_RECURSIVE_MUTEXES, and configSUPPORT_STATIC_ALLOCATION must be enabled and set to 1 to use clib-support

#else

/* guarantee that no functions using semihosting are included in your application */
__asm(".global __use_no_semihosting\n\t");
#if defined(COMPONENT_CAT5)
/* cat5 provides a thread stack separate from heap, so disable automatic collision detection */
__asm(".global __use_two_region_memory\n\t");
#endif // defined(COMPONENT_CAT5)
#if defined(MUTEX_POOL_AVAILABLE)
cy_mutex_pool_semaphore_t cy_ctor_mutex;
cy_mutex_pool_semaphore_t cy_timer_mutex;
#endif // defined(MUTEX_POOL_AVAILABLE)

#if defined(COMPONENT_CAT5)
extern size_t __rt_heap_extend(size_t /*size*/, void** /*block*/);
extern char Image$$HEAP$$ZI$$Base[];
extern int Image$$HEAP$$ZI$$Limit;
#endif // defined(COMPONENT_CAT5)

#if defined(COMPONENT_THREADX)
static uint16_t _scheduler_suspend_count = 0;
static uint32_t _interrupt_primask = 0;
#endif // defined(COMPONENT_THREADX)
//--------------------------------------------------------------------------------------------------
// _platform_post_stackheap_init
//--------------------------------------------------------------------------------------------------
void _platform_post_stackheap_init(void)
{
    #if defined(COMPONENT_CAT5)
    __rt_lib_init((unsigned)&Image$$HEAP$$ZI$$Base[0], (unsigned)&Image$$HEAP$$ZI$$Limit);
    #endif // defined(COMPONENT_CAT5)
    #if defined(MUTEX_POOL_AVAILABLE)
    cy_ctor_mutex = cy_mutex_pool_create();
    cy_timer_mutex = cy_mutex_pool_create();
    #endif // defined(MUTEX_POOL_AVAILABLE)
}


#if defined(COMPONENT_CAT5)
//--------------------------------------------------------------------------------------------------
// __rt_heap_extend
//--------------------------------------------------------------------------------------------------
size_t __rt_heap_extend(size_t incr, void** block)
{
    static uint8_t* heap_base = (uint8_t*)Image$$HEAP$$ZI$$Base;
    static uint8_t* heap_limit = (uint8_t*)&Image$$HEAP$$ZI$$Limit;
    static uint8_t* heapBrk = (uint8_t*)Image$$HEAP$$ZI$$Base;
    if ((incr > (int32_t)(heap_limit - heapBrk)) ||
        (((int32_t)heapBrk + incr) < (int32_t)heap_base))
    {
        return 0;
    }
    *block = (void*)heapBrk;
    heapBrk += incr;
    return incr;
}


#endif // defined(COMPONENT_CAT5)

// ARM thread-local state

static struct _reent cy_armlib_global_impure;
struct _reent*       _impure_ptr = &cy_armlib_global_impure;

//--------------------------------------------------------------------------------------------------
// __user_perthread_libspace
//--------------------------------------------------------------------------------------------------
__attribute__((used))
struct _reent* __user_perthread_libspace(void)
{
    return _impure_ptr;
}


// ARM library locking

// The ARM library allocates 4 bytes for each mutex.

//--------------------------------------------------------------------------------------------------
// _mutex_initialize
//--------------------------------------------------------------------------------------------------
__attribute__((used))
int _mutex_initialize(cy_mutex_pool_semaphore_t* m)
{
    #if defined(MUTEX_POOL_AVAILABLE)
    *m = cy_mutex_pool_create();
    #else
    (void)m;
    #endif // defined(MUTEX_POOL_AVAILABLE)
    return 1;
}


//--------------------------------------------------------------------------------------------------
// _mutex_acquire
//--------------------------------------------------------------------------------------------------
__attribute__((used))
void _mutex_acquire(cy_mutex_pool_semaphore_t* m)
{
    #if defined(MUTEX_POOL_AVAILABLE)
    cy_mutex_pool_acquire(*m);
    #else
    #if defined(COMPONENT_FREERTOS)
    (void)m;
    cy_mutex_pool_suspend_threads();
    #elif defined(COMPONENT_THREADX)
    ++_scheduler_suspend_count;
    /* ThreadX scheduler suspend for other threads to do not interfere */
    if (_scheduler_suspend_count == 1)
    {
        _interrupt_primask = Cy_SysLib_EnterCriticalSection();
    }
    #endif // defined(COMPONENT_FREERTOS)
    #endif // defined(MUTEX_POOL_AVAILABLE)
}


//--------------------------------------------------------------------------------------------------
// _mutex_release
//--------------------------------------------------------------------------------------------------
__attribute__((used))
void _mutex_release(cy_mutex_pool_semaphore_t* m)
{
    #if defined(MUTEX_POOL_AVAILABLE)
    cy_mutex_pool_release(*m);
    #else
    #if defined(COMPONENT_FREERTOS)
    (void)m;
    cy_mutex_pool_resume_threads();
    #elif defined(COMPONENT_THREADX)
    /* ThreadX scheduler resume , allows other threads to run */
    if (_scheduler_suspend_count > 0)
    {
        if (_scheduler_suspend_count == 1)
        {
            Cy_SysLib_ExitCriticalSection(_interrupt_primask);
        }
        --_scheduler_suspend_count;
    }
    #endif // defined(COMPONENT_FREERTOS)
    #endif // defined(MUTEX_POOL_AVAILABLE)
}


//--------------------------------------------------------------------------------------------------
// _mutex_free
//--------------------------------------------------------------------------------------------------
__attribute__((used))
void _mutex_free(cy_mutex_pool_semaphore_t* m)
{
    #if defined(MUTEX_POOL_AVAILABLE)
    cy_mutex_pool_destroy(*m);
    #endif //defined(MUTEX_POOL_AVAILABLE)
    *m = NULL;
}


// The __cxa_guard_acquire, __cxa_guard_release, and __cxa_guard_abort
// functions ensure that constructors for static local variables are
// executed exactly once. For more information, see:
// https://itanium-cxx-abi.github.io/cxx-abi/abi.html#obj-ctor

typedef struct
{
    uint8_t initialized;
    uint8_t acquired;
} cy_cxa_guard_object_t;

// Use custom routines for atomic load/store as the ARM Compiler
// doesn't provide the required builtins for ARMv6-M architecture.
__STATIC_INLINE uint8_t cy_atomic_load_1(uint8_t* address)
{
    __DMB();
    uint8_t result = *address;
    __DMB();
    return result;
}


//--------------------------------------------------------------------------------------------------
// cy_atomic_store_1
//--------------------------------------------------------------------------------------------------
__STATIC_INLINE void cy_atomic_store_1(uint8_t* address, uint8_t value)
{
    __DMB();
    *address = value;
    __DMB();
}


//--------------------------------------------------------------------------------------------------
// __cxa_guard_acquire
//--------------------------------------------------------------------------------------------------
int __cxa_guard_acquire(cy_cxa_guard_object_t* guard_object)
{
    int acquired = 0;
    if (0 == cy_atomic_load_1(&guard_object->initialized))
    {
        #if defined(MUTEX_POOL_AVAILABLE)
        cy_mutex_pool_acquire(cy_ctor_mutex);
        #if defined(COMPONENT_THREADX)
        ++_scheduler_suspend_count;
        /* ThreadX scheduler suspend for other threads to do not interfere */
        if (_scheduler_suspend_count == 1)
        {
            _interrupt_primask = Cy_SysLib_EnterCriticalSection();
        }
        #endif // defined(COMPONENT_THREADX)
        #else
        #if defined(COMPONENT_FREERTOS)
        cy_mutex_pool_suspend_threads();
        #endif // defined(COMPONENT_FREERTOS)
        #endif // defined(MUTEX_POOL_AVAILABLE)
        if (0 == cy_atomic_load_1(&guard_object->initialized))
        {
            acquired = 1;
            #ifndef NDEBUG
            if (guard_object->acquired)
            {
                __BKPT(0);  // acquire called again without release/abort
            }
            #endif
            guard_object->acquired = 1;
        }
        else
        {
            #if defined(MUTEX_POOL_AVAILABLE)
            cy_mutex_pool_release(cy_ctor_mutex);
            #if defined(COMPONENT_THREADX)
            /* ThreadX scheduler resume , allows other threads to run */
            if (_scheduler_suspend_count > 0)
            {
                if (_scheduler_suspend_count == 1)
                {
                    Cy_SysLib_ExitCriticalSection(_interrupt_primask);
                }
                --_scheduler_suspend_count;
            }
            #endif // defined(COMPONENT_THREADX)
            #else // if defined(MUTEX_POOL_AVAILABLE)
            #if defined(COMPONENT_FREERTOS)
            cy_mutex_pool_resume_threads();
            #endif // defined(COMPONENT_FREERTOS)
            #endif // defined(MUTEX_POOL_AVAILABLE)
        }
    }
    return acquired;
}


//--------------------------------------------------------------------------------------------------
// __cxa_guard_abort
//--------------------------------------------------------------------------------------------------
void __cxa_guard_abort(cy_cxa_guard_object_t* guard_object)
{
    if (guard_object->acquired)
    {
        guard_object->acquired = 0;
        #if defined(MUTEX_POOL_AVAILABLE)
        cy_mutex_pool_release(cy_ctor_mutex);
        #if defined(COMPONENT_THREADX)
        /* ThreadX scheduler resume , allows other threads to run */
        if (_scheduler_suspend_count > 0)
        {
            if (_scheduler_suspend_count == 1)
            {
                Cy_SysLib_ExitCriticalSection(_interrupt_primask);
            }
            --_scheduler_suspend_count;
        }
        #endif // defined(COMPONENT_THREADX)
        #else // if defined(MUTEX_POOL_AVAILABLE)
        #if defined(COMPONENT_FREERTOS)
        cy_mutex_pool_resume_threads();
        #endif // defined(COMPONENT_FREERTOS)
        #endif // defined(MUTEX_POOL_AVAILABLE)
    }
    #ifndef NDEBUG
    else
    {
        __BKPT(0);  // __cxa_guard_abort called when not acquired
    }
    #endif
}


//--------------------------------------------------------------------------------------------------
// __cxa_guard_release
//--------------------------------------------------------------------------------------------------
void __cxa_guard_release(cy_cxa_guard_object_t* guard_object)
{
    cy_atomic_store_1(&guard_object->initialized, 1);
    __cxa_guard_abort(guard_object);    // Release mutex
}


// Replace functions that depend on semihosting

//--------------------------------------------------------------------------------------------------
// _sys_exit
//--------------------------------------------------------------------------------------------------
__attribute__((used))
void _sys_exit(int unused __attribute__((unused)))
{
    __disable_irq();
    while (1)
    {
        __WFI();
    }
}


//--------------------------------------------------------------------------------------------------
// _ttywrch
//--------------------------------------------------------------------------------------------------
__attribute__((used))
void _ttywrch(int ch)
{
    fputc(ch, stdout);
}


extern int $Super$$_sys_open(const char*, int);

//--------------------------------------------------------------------------------------------------
// _sys_open
//--------------------------------------------------------------------------------------------------
__attribute__((used))
int $Sub$$_sys_open(const char* name, int openmode __attribute__((unused)))
{
    extern const char __stdin_name, __stdout_name, __stderr_name;
    int               fd = -1;
    if (0 == strcmp(name, &__stdin_name))
    {
        fd = 0; // STDIN_FILENO
    }
    else if (0 == strcmp(name, &__stdout_name))
    {
        fd = 1; // STDOUT_FILENO
    }
    else if (0 == strcmp(name, &__stderr_name))
    {
        fd = 2; // STDERR_FILENO
    }
    return fd;
}


//--------------------------------------------------------------------------------------------------
// _sys_command_string
//--------------------------------------------------------------------------------------------------
__attribute__((used))
char* _sys_command_string(char* unused1 __attribute__((unused)),
                          int unused2 __attribute__((unused)))
{
    return NULL;
}


#endif \
    // defined(COMPONENT_FREERTOS) && (configUSE_MUTEXES == 0 ||
    //     configUSE_RECURSIVE_MUTEXES == 0 || configSUPPORT_STATIC_ALLOCATION == 0)
