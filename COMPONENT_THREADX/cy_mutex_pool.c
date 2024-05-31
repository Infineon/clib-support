/***********************************************************************************************//**
 * \file COMPONENT_THREADX/cy_mutex_pool.c
 *
 * \brief
 * Mutex pool implementation for ThreadX
 *
 ***************************************************************************************************
 * \copyright
 * Copyright 2021-2022 Cypress Semiconductor Corporation (an Infineon company) or
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
#include <string.h>
#if !defined (COMPONENT_CAT5)
#include <cmsis_compiler.h>
#else
#include "cyhal_system.h"
#endif

#include "cy_mutex_pool.h"
#include "cy_mutex_pool_cfg.h"
#include "tx_api.h"
#include "tx_thread.h"
#include "tx_mutex.h"
#include "tx_initialize.h"

// The standard library requires mutexes in order to ensure thread safety for
// operations such as malloc. The mutexes must be initialized at startup.
// It is not possible to allocate these mutexes on the heap because the heap
// is not initialized until later in the startup process (and because the
// heap requires a mutex). Some of the toolchains require recursive mutexes.

// The mutex functions may be called before tx_kernel_enter. In this case,
// the acquire/release functions will do nothing.

// Ensure that TX_DISABLE_REDUNDANT_CLEARING is defined. Otherwise any mutexes
// which are initialized prior to tx_kernel_enter will be wiped out.

#ifndef TX_DISABLE_REDUNDANT_CLEARING
#error TX_DISABLE_REDUNDANT_CLEARING must be defined!
#endif

//--------------------------------------------------------------------------------------------------
// cy_threadx_kernel_started
//--------------------------------------------------------------------------------------------------
static bool cy_threadx_kernel_started(void)
{
    static bool rtos_started = false;

    if (!rtos_started)
    {
        rtos_started = TX_THREAD_GET_SYSTEM_STATE() == TX_INITIALIZE_IS_FINISHED;
    }

    return rtos_started;
}


// The mutex functions must not be used in interrupt context.
// The acquire function is explicitly designed for waiting which is likely to
// cause a deadlock in an interrupt.
static void cy_threadx_check_in_isr(void)
{
    if (0 != __get_IPSR())
    {
        __BKPT(0);
    }
}


#ifdef __ICCARM__
// For IAR, the mutexes are allocated by __iar_Initlocks before static
// data initialization. Use the __no_init attribute so the mutex data
// will not be overwritten during static data initialization.
#   define CY_ATTR_NO_INIT __no_init
#else
// The other toolchains perform mutex initialization after static data
// initialization so they do not require any special behavior.
#   define CY_ATTR_NO_INIT
#endif

static CY_ATTR_NO_INIT TX_MUTEX cy_mutex_pool_storage[CY_STATIC_MUTEX_MAX];

//--------------------------------------------------------------------------------------------------
// cy_mutex_pool_setup
//--------------------------------------------------------------------------------------------------
void cy_mutex_pool_setup(void)
{
    // Only required if the startup code does not initialize cy_mutex_pool_storage
    memset(cy_mutex_pool_storage, 0, sizeof(cy_mutex_pool_storage));
}


//--------------------------------------------------------------------------------------------------
// cy_mutex_pool_create
//--------------------------------------------------------------------------------------------------
cy_mutex_pool_semaphore_t cy_mutex_pool_create(void)
{
    UINT old_posture;
    cy_mutex_pool_semaphore_t handle = NULL;

    cy_threadx_check_in_isr();

    /*
     * Find a mutex that hasn't been initialized yet.
     */

    old_posture = tx_interrupt_control(TX_INT_DISABLE);
    for (uint16_t i = 0; i < CY_STATIC_MUTEX_MAX; i++)
    {
        if (TX_MUTEX_ID != cy_mutex_pool_storage[i].tx_mutex_id)
        {
            handle = &cy_mutex_pool_storage[i];
            break;
        }
    }
    tx_interrupt_control(old_posture);

    if (NULL != handle)
    {
        if (tx_mutex_create(handle, TX_NULL, TX_NO_INHERIT) != TX_SUCCESS)
        {
            handle = NULL;
        }
    }

    if (NULL == handle)
    {
        __BKPT(0);  // Out of resources
    }

    return handle;
}


//--------------------------------------------------------------------------------------------------
// cy_mutex_pool_acquire
//--------------------------------------------------------------------------------------------------
void cy_mutex_pool_acquire(cy_mutex_pool_semaphore_t m)
{
    cy_threadx_check_in_isr();
    if (cy_threadx_kernel_started())
    {
        static const uint32_t CY_MUTEX_TIMEOUT_TICKS = 10000;

        while (tx_mutex_get(m, CY_MUTEX_TIMEOUT_TICKS) != TX_SUCCESS)
        {
            // Halt here until condition the operation succeeds
        }
    }
}


//--------------------------------------------------------------------------------------------------
// cy_mutex_pool_release
//--------------------------------------------------------------------------------------------------
void cy_mutex_pool_release(cy_mutex_pool_semaphore_t m)
{
    cy_threadx_check_in_isr();
    if (cy_threadx_kernel_started())
    {
        tx_mutex_put(m);
    }
}


//--------------------------------------------------------------------------------------------------
// cy_mutex_pool_destroy
//--------------------------------------------------------------------------------------------------
void cy_mutex_pool_destroy(cy_mutex_pool_semaphore_t m)
{
    UINT old_posture;

    cy_threadx_check_in_isr();

    old_posture = tx_interrupt_control(TX_INT_DISABLE);
    tx_mutex_delete(m);
    tx_interrupt_control(old_posture);
}
