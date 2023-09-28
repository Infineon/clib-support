/***********************************************************************************************//**
 * \file cy_clib_support_iar.c
 *
 * \brief
 * IAR C library port for clib-support library
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <DLib_Threads.h>
#include "reent.h"
#include <cmsis_compiler.h>
#include "cy_mutex_pool.h"

#if defined(COMPONENT_FREERTOS) && (configUSE_MUTEXES == 0 || configUSE_RECURSIVE_MUTEXES == 0 || \
                                    configSUPPORT_STATIC_ALLOCATION == 0)
#warning \
    configUSE_MUTEXES, configUSE_RECURSIVE_MUTEXES, and configSUPPORT_STATIC_ALLOCATION must be enabled and set to 1 to use clib-support

#elif defined(COMPONENT_THREADX) && !defined(TX_ENABLE_IAR_LIBRARY_SUPPORT)
#error \
    "In order to enable thread safe library support on IAR, add TX_ENABLE_IAR_LIBRARY_SUPPORT to defines"

#else

// IAR thread-local state

#pragma section="__iar_tls$$DATA"
struct _reent  cy_iar_global_impure = { __section_begin("__iar_tls$$DATA") };
struct _reent* _impure_ptr          = &cy_iar_global_impure;

#if defined(MUTEX_POOL_AVAILABLE)
cy_mutex_pool_semaphore_t cy_timer_mutex;
#endif

#if defined(COMPONENT_THREADX)
cy_mutex_pool_semaphore_t cy_malloc_mutex;
#endif // if defined(COMPONENT_THREADX)

//--------------------------------------------------------------------------------------------------
// __aeabi_read_tp
//--------------------------------------------------------------------------------------------------
#if !defined(COMPONENT_THREADX)
// __aeabi_read_tp functions is defined in the tx_iar.c port provided by ThreadX
void* __aeabi_read_tp(void)
{
    return _impure_ptr->ptr;
}


#endif

//--------------------------------------------------------------------------------------------------
// cy_iar_init_reent
//--------------------------------------------------------------------------------------------------
#if defined(COMPONENT_FREERTOS)
void cy_iar_init_reent(struct _reent* r)
{
    size_t tls_size = __iar_tls_size();
    if (0 == tls_size)
    {
        tls_size = sizeof(void*);
    }
    r->ptr = pvPortMalloc(tls_size);
    if (NULL == r->ptr)
    {
        __BKPT(0);  // Failed to allocate memory for TLS
    }
    __iar_tls_init(r->ptr);
}


#endif //#if defined(COMPONENT_FREERTOS)


//--------------------------------------------------------------------------------------------------
// _reclaim_reent
//--------------------------------------------------------------------------------------------------
void _reclaim_reent(struct _reent* r)
{
    // __call_thread_dtors must be called from the thread that is being destroyed. This will not
    // work with FreeRTOS.
    // The effect of omitting it is that destructors for thread-local C++ objects will not be
    // executed.
    //__call_thread_dtors();
    if (r != &cy_iar_global_impure)
    {
        #if defined(COMPONENT_FREERTOS)
        vPortFree(r->ptr);
        #elif defined(COMPONENT_THREADX)
        cy_mutex_pool_acquire(cy_malloc_mutex);
        free(r->ptr);
        cy_mutex_pool_release(cy_malloc_mutex);
        #endif // if defined(COMPONENT_FREERTOS)
        r->ptr = NULL;
    }
}


// IAR library locking

void cy_toolchain_init(void)
{
    #if !defined(COMPONENT_THREADX)
    extern void __iar_Initlocks(void);
    #endif
    #if defined(MUTEX_POOL_AVAILABLE)
    cy_mutex_pool_setup();
    cy_timer_mutex   = cy_mutex_pool_create();
    #endif
    #if defined(COMPONENT_THREADX)
    cy_malloc_mutex = cy_mutex_pool_create();
    #endif
    // __iar_Initlocks is called in ThreadX during setup; calling it here would lead
    // to (invalid) double initialization of the mutexes
    #if !defined(COMPONENT_THREADX)
    __iar_Initlocks();
    #endif
}


#if !defined(COMPONENT_THREADX)
// __iar_system_* functions are defined in the tx_iar.c port provided by ThreadX

//--------------------------------------------------------------------------------------------------
// __iar_system_Mtxinit
//--------------------------------------------------------------------------------------------------
void __iar_system_Mtxinit(__iar_Rmtx* arg)
{
    #if defined(MUTEX_POOL_AVAILABLE)
    *(cy_mutex_pool_semaphore_t*)arg = cy_mutex_pool_create();
    #else
    (void)arg;
    #endif
}


//--------------------------------------------------------------------------------------------------
// __iar_system_Mtxlock
//--------------------------------------------------------------------------------------------------
void __iar_system_Mtxlock(__iar_Rmtx* m)
{
    #if defined(MUTEX_POOL_AVAILABLE)
    cy_mutex_pool_acquire(*(cy_mutex_pool_semaphore_t*)m);
    #else
    (void)m;
    cy_mutex_pool_suspend_threads();
    #endif
}


//--------------------------------------------------------------------------------------------------
// __iar_system_Mtxunlock
//--------------------------------------------------------------------------------------------------
void __iar_system_Mtxunlock(__iar_Rmtx* m)
{
    #if defined(MUTEX_POOL_AVAILABLE)
    cy_mutex_pool_release(*(cy_mutex_pool_semaphore_t*)m);
    #else
    (void)m;
    cy_mutex_pool_resume_threads();
    #endif
}


//--------------------------------------------------------------------------------------------------
// __iar_system_Mtxdst
//--------------------------------------------------------------------------------------------------
void __iar_system_Mtxdst(__iar_Rmtx* arg)
{
    #if defined(MUTEX_POOL_AVAILABLE)
    cy_mutex_pool_semaphore_t* m = (cy_mutex_pool_semaphore_t*)arg;
    cy_mutex_pool_destroy(*m);
    *m = NULL;
    #else
    (void)arg;
    #endif
}


#endif // if !defined(COMPONENT_THREADX)

#if ((!_DLIB_FILE_DESCRIPTOR) || (!defined(COMPONENT_THREADX)))

//--------------------------------------------------------------------------------------------------
// __iar_file_Mtxinit
//--------------------------------------------------------------------------------------------------
void __iar_file_Mtxinit(__iar_Rmtx* m)
{
    __iar_system_Mtxinit(m);
}


//--------------------------------------------------------------------------------------------------
// __iar_file_Mtxlock
//--------------------------------------------------------------------------------------------------
void __iar_file_Mtxlock(__iar_Rmtx* m)
{
    __iar_system_Mtxlock(m);
}


//--------------------------------------------------------------------------------------------------
// __iar_file_Mtxunlock
//--------------------------------------------------------------------------------------------------
void __iar_file_Mtxunlock(__iar_Rmtx* m)
{
    __iar_system_Mtxunlock(m);
}


//--------------------------------------------------------------------------------------------------
// __iar_file_Mtxdst
//--------------------------------------------------------------------------------------------------
void __iar_file_Mtxdst(__iar_Rmtx* m)
{
    __iar_system_Mtxdst(m);
}


#endif // if ((!_DLIB_FILE_DESCRIPTOR) || (!defined(COMPONENT_THREADX)))


#if configSUPPORT_DYNAMIC_ALLOCATION

//--------------------------------------------------------------------------------------------------
// __iar_Initdynamiclock
//--------------------------------------------------------------------------------------------------
void __iar_Initdynamiclock(__iar_Rmtx* m)
{
    #if defined(MUTEX_POOL_AVAILABLE)
    cy_mutex_pool_semaphore_t handle = xSemaphoreCreateRecursiveMutex();
    *(cy_mutex_pool_semaphore_t*)m = handle;
    if (NULL == handle)
    {
        __BKPT(0);  // Failed to allocate C++ dynamic lock
    }
    #else
    (void)m;
    #endif
}


//--------------------------------------------------------------------------------------------------
// __iar_Lockdynamiclock
//--------------------------------------------------------------------------------------------------
void __iar_Lockdynamiclock(__iar_Rmtx* m)
{
    __iar_system_Mtxlock(m);
}


//--------------------------------------------------------------------------------------------------
// __iar_Unlockdynamiclock
//--------------------------------------------------------------------------------------------------
void __iar_Unlockdynamiclock(__iar_Rmtx* m)
{
    __iar_system_Mtxunlock(m);
}


//--------------------------------------------------------------------------------------------------
// __iar_Dstdynamiclock
//--------------------------------------------------------------------------------------------------
void __iar_Dstdynamiclock(__iar_Rmtx* m)
{
    #if defined(MUTEX_POOL_AVAILABLE)
    vSemaphoreDelete(*(cy_mutex_pool_semaphore_t*)m);
    *(cy_mutex_pool_semaphore_t*)m = NULL;
    #else
    (void)m;
    #endif
}


#endif // if configSUPPORT_DYNAMIC_ALLOCATION

//--------------------------------------------------------------------------------------------------
// __close
//--------------------------------------------------------------------------------------------------
__weak int __close(int fd)
{
    return 0;
}


//--------------------------------------------------------------------------------------------------
// __lseek
//--------------------------------------------------------------------------------------------------
__weak long __lseek(int fd, long offset, int whence)
{
    return -1;
}


//--------------------------------------------------------------------------------------------------
// remove
//--------------------------------------------------------------------------------------------------
__weak int remove(const char* path)
{
    return 0;
}


#endif \
    // defined(COMPONENT_FREERTOS) && (configUSE_MUTEXES == 0 ||
    //     configUSE_RECURSIVE_MUTEXES == 0 || configSUPPORT_STATIC_ALLOCATION == 0)
