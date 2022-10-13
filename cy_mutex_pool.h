/***********************************************************************************************//**
 * \file cy_mutex_pool.h
 *
 * \brief
 * Mutex pool implementation for FreeRTOS
 *
 ***************************************************************************************************
 * \copyright
 * Copyright 2018-2022 Cypress Semiconductor Corporation (an Infineon company) or
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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#if defined(COMPONENT_FREERTOS)
#include "FreeRTOS.h"
#include <semphr.h>
#include <task.h>

/** Map cy_mutex_pool_semaphore_t to FreeRTOS specific SemaphoreHandle_t */
typedef SemaphoreHandle_t cy_mutex_pool_semaphore_t;
#elif defined(COMPONENT_THREADX)
#include "tx_api.h"

/** Map cy_mutex_pool_semaphore_t to ThreadX specific TX_MUTEX* */
typedef TX_MUTEX* cy_mutex_pool_semaphore_t;
#else // if defined(COMPONENT_FREERTOS)
#error "Unhandled RTOS type"
#endif // if defined(COMPONENT_FREERTOS)

#if !defined(COMPONENT_FREERTOS) || (configHEAP_ALLOCATION_SCHEME != HEAP_ALLOCATION_TYPE3)
#define MUTEX_POOL_AVAILABLE

/** Internal use only. Initializes the mutex pool. */
void cy_mutex_pool_setup(void);

/** Internal use only. Allocates a recursive mutex. */
/** \return cy_mutex_pool_semaphore_t */
cy_mutex_pool_semaphore_t cy_mutex_pool_create(void);

/** Internal use only. Acquires a recursive mutex. */
/** \param m cy_mutex_pool_semaphore_t */
void cy_mutex_pool_acquire(cy_mutex_pool_semaphore_t m);

/** Internal use only. Releases a recursive mutex. */
/** \param m cy_mutex_pool_semaphore_t */
void cy_mutex_pool_release(cy_mutex_pool_semaphore_t m);

/** Internal use only. Destroys a recursive mutex. */
/** \param m cy_mutex_pool_semaphore_t */
void cy_mutex_pool_destroy(cy_mutex_pool_semaphore_t m);

#else // defined(MUTEX_POOL_AVAILABLE)

/** Internal use only. If the mutex pool is not available, we have to suspend all threads to ensure
 *  exclusive access to resources. */
static inline void cy_mutex_pool_suspend_threads(void)
{
    vTaskSuspendAll();
}


/** Internal use only. Ends an exclusive region and allows other threads to start running again. */
static inline void cy_mutex_pool_resume_threads(void)
{
    xTaskResumeAll();
}


#endif // defined(MUTEX_POOL_AVAILABLE)

#ifdef __cplusplus
}
#endif
