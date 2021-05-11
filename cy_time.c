/***********************************************************************************************//**
 * \file cy_time.c
 *
 * \brief
 * Implementation of the time() function provided by time.h. Provide APIs to set &
 * get RTC instance
 *
 ***************************************************************************************************
 * \copyright
 * Copyright 2021 Cypress Semiconductor Corporation
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
#if defined(CY_USING_HAL)
#include "cy_time.h"

// Make sure the RTC is available on this device
#if defined(CY_RSLT_RTC_NOT_INITIALIZED)
#include "cy_mutex_pool.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#define SUCCESS 0
#define ERROR   -1

static cyhal_rtc_t*      cy_time = NULL;
static cyhal_rtc_t       cy_timer_rtc;

#if (configHEAP_ALLOCATION_SCHEME != HEAP_ALLOCATION_TYPE3)
extern SemaphoreHandle_t cy_timer_mutex;
#endif

//--------------------------------------------------------------------------------------------------
// Acquire a lock to ensure exclusive access
//--------------------------------------------------------------------------------------------------
static void mutex_acquire()
{
    #if (configHEAP_ALLOCATION_SCHEME != HEAP_ALLOCATION_TYPE3)
    cy_mutex_pool_acquire(cy_timer_mutex);
    #else
    vTaskSuspendAll();
    #endif
}


//--------------------------------------------------------------------------------------------------
// Release the exclusivity lock
//--------------------------------------------------------------------------------------------------
static void mutex_release()
{
    #if (configHEAP_ALLOCATION_SCHEME != HEAP_ALLOCATION_TYPE3)
    cy_mutex_pool_release(cy_timer_mutex);
    #else
    xTaskResumeAll();
    #endif
}


//--------------------------------------------------------------------------------------------------
// Get the current time
//--------------------------------------------------------------------------------------------------
time_t time(time_t* timer)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    struct tm rtc_time;
    time_t seconds = 0;

    mutex_acquire();

    /* If RTC instance is not set using cy_rtc_set_instance() API, Initialize RTC
     * and write the default time to RTC */
    if (cy_time == NULL)
    {
        result = cyhal_rtc_init(&cy_timer_rtc);
        CY_ASSERT(CY_RSLT_SUCCESS == result);

        if (result == CY_RSLT_SUCCESS)
        {
            cy_time = &cy_timer_rtc;

            /* Write time to RTC */
            result = cyhal_rtc_write(cy_time, localtime(&seconds));
        }
    }

    if (result == CY_RSLT_SUCCESS)
    {
        /* Read current time from RTC */
        result = cyhal_rtc_read(cy_time, &rtc_time);
        if (result == CY_RSLT_SUCCESS)
        {
            /* Convert tm format to time_t */
            seconds = mktime(&rtc_time);
        }
    }

    mutex_release();
    if (result != CY_RSLT_SUCCESS)
    {
        if (timer != NULL)
        {
            *timer = ERROR;
        }
        return ERROR;
    }

    if (timer != NULL)
    {
        *timer = seconds;
    }

    return seconds;
}


//--------------------------------------------------------------------------------------------------
// Set the RTC instance.
//--------------------------------------------------------------------------------------------------
void cy_set_rtc_instance(cyhal_rtc_t* rtc_instance)
{
    mutex_acquire();

    cy_time = rtc_instance;

    mutex_release();
}


//--------------------------------------------------------------------------------------------------
// Get the RTC instance.
//--------------------------------------------------------------------------------------------------
cyhal_rtc_t* cy_get_rtc_instance()
{
    return cy_time;
}


#if defined(__cplusplus)
}
#endif

#endif // if defined(CY_RSLT_RTC_NOT_INITIALIZED)
#endif // if defined(CY_USING_HAL)
