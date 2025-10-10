/***********************************************************************************************//**
 * \file cy_clib_support_llvm_arm_freertos.c
 *
 * \brief
 * LLVM ARM C library port for FreeRTOS
 *
 ***************************************************************************************************
 * \copyright
 * Copyright 2018-2024 Cypress Semiconductor Corporation (an Infineon company) or
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

#if defined(COMPONENT_FREERTOS)
#include <sys/errno.h>
#include <sys/unistd.h>
#include <cmsis_compiler.h>
#include "cy_mutex_pool.h"
#include "cy_utils.h"

// Picolibc specific
// Per README.md, this is initialized after .bss and .data and before static constructors.
__NO_INIT cy_mutex_pool_semaphore_t __lock___libc_recursive_mutex;
typedef cy_mutex_pool_semaphore_t* _LOCK_T;

#if defined(MUTEX_POOL_AVAILABLE)
__NO_INIT cy_mutex_pool_semaphore_t cy_timer_mutex;
#endif // defined(MUTEX_POOL_AVAILABLE)

//--------------------------------------------------------------------------------------------------
// cy_toolchain_init
//--------------------------------------------------------------------------------------------------
void cy_toolchain_init(void)
{
    #if defined(MUTEX_POOL_AVAILABLE)
    cy_mutex_pool_setup();
    __lock___libc_recursive_mutex = cy_mutex_pool_create();
    cy_timer_mutex = cy_mutex_pool_create();
    #endif // defined(MUTEX_POOL_AVAILABLE)
}


//--------------------------------------------------------------------------------------------------
// __retarget_lock_init_recursive
//--------------------------------------------------------------------------------------------------
void __retarget_lock_init_recursive(_LOCK_T* lock)
{
    #if defined(MUTEX_POOL_AVAILABLE)
    **(cy_mutex_pool_semaphore_t**)lock = cy_mutex_pool_create();
    #else
    CY_UNUSED_PARAMETER(lock);
    #endif
}


//--------------------------------------------------------------------------------------------------
// __retarget_lock_init
//--------------------------------------------------------------------------------------------------
void __retarget_lock_init(_LOCK_T* lock)
{
    __retarget_lock_init_recursive(lock);
}


//--------------------------------------------------------------------------------------------------
// __retarget_lock_close_recursive
//--------------------------------------------------------------------------------------------------
void __retarget_lock_close_recursive(_LOCK_T lock)
{
    #if defined(MUTEX_POOL_AVAILABLE)
    cy_mutex_pool_semaphore_t* toClose = (cy_mutex_pool_semaphore_t*)lock;
    cy_mutex_pool_destroy(*toClose);
    *toClose = NULL;
    #else
    CY_UNUSED_PARAMETER(lock);
    #endif
}


//--------------------------------------------------------------------------------------------------
// __retarget_lock_close
//--------------------------------------------------------------------------------------------------
void __retarget_lock_close(_LOCK_T lock)
{
    __retarget_lock_close_recursive(lock);
}


//--------------------------------------------------------------------------------------------------
// __retarget_lock_acquire_recursive
//--------------------------------------------------------------------------------------------------
void __retarget_lock_acquire_recursive(_LOCK_T lock)
{
    #if defined(MUTEX_POOL_AVAILABLE)
    cy_mutex_pool_acquire(*(cy_mutex_pool_semaphore_t*)lock);
    #else
    CY_UNUSED_PARAMETER(lock);
    cy_mutex_pool_suspend_threads();
    #endif
}


//--------------------------------------------------------------------------------------------------
// __retarget_lock_acquire
//--------------------------------------------------------------------------------------------------
void __retarget_lock_acquire(_LOCK_T lock)
{
    __retarget_lock_acquire_recursive(lock);
}


//--------------------------------------------------------------------------------------------------
// __retarget_lock_release_recursive
//--------------------------------------------------------------------------------------------------
void __retarget_lock_release_recursive(_LOCK_T lock)
{
    #if defined(MUTEX_POOL_AVAILABLE)
    cy_mutex_pool_release(*(cy_mutex_pool_semaphore_t*)lock);
    #else
    CY_UNUSED_PARAMETER(lock);
    cy_mutex_pool_resume_threads();
    #endif
}


//--------------------------------------------------------------------------------------------------
// __retarget_lock_release
//--------------------------------------------------------------------------------------------------
void __retarget_lock_release(_LOCK_T lock)
{
    __retarget_lock_release_recursive(lock);
}


#endif // defined(COMPONENT_FREERTOS)
