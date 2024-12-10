/***********************************************************************************************//**
 * \file cy_time.c
 *
 * \brief
 * Implementation of the time() function provided by time.h. Provide APIs to init &
 * get CLIB support RTC
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

#include "cy_time.h"

// Make sure the RTC is available on this device.
#if defined(_MTB_CLIB_SUPPORT_RTC_AVAILABLE)
#include "cy_mutex_pool.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#define SUCCESS 0
#define ERROR   -1

#if defined(MTB_HAL_API_VERSION) && ((MTB_HAL_API_VERSION) >= 3)
static mtb_hal_rtc_t*      cy_time = NULL;
#else
static cyhal_rtc_t*        cy_time = NULL;
static cyhal_rtc_t         cy_time_rtc_inst;
#endif


#if defined(MUTEX_POOL_AVAILABLE)
extern cy_mutex_pool_semaphore_t cy_timer_mutex;
#endif

//--------------------------------------------------------------------------------------------------
// Acquire a lock to ensure exclusive access
//--------------------------------------------------------------------------------------------------
static void mutex_acquire(void)
{
    #if defined(MUTEX_POOL_AVAILABLE)
    cy_mutex_pool_acquire(cy_timer_mutex);
    #else
    cy_mutex_pool_suspend_threads();
    #endif
}


//--------------------------------------------------------------------------------------------------
// Release the exclusivity lock
//--------------------------------------------------------------------------------------------------
static void mutex_release(void)
{
    #if defined(MUTEX_POOL_AVAILABLE)
    cy_mutex_pool_release(cy_timer_mutex);
    #else
    cy_mutex_pool_resume_threads();
    #endif
}


//--------------------------------------------------------------------------------------------------
// Get the current time
//--------------------------------------------------------------------------------------------------
time_t time(time_t* _timer)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    struct tm rtc_time;
    #if defined(COMPONENT_CAT5)
    /* For CAT5, default time is set as January 1, 2011 12:00:00 AM.
       Epoch timestamp: 1293840000 */
    time_t seconds = 1293840000;
    #else
    time_t seconds = 0;
    #endif

    /* If CLIB support RTC is not init using mtb_clib_support_init() API at first */
    #if defined(MTB_HAL_DISABLE_ERR_CHECK)
    CY_ASSERT_AND_RETURN(cy_time != NULL, seconds);
    #else
    if (cy_time == NULL)
    {
        /* HAL API version 2 and lower support dynamically allocating and initializing
         * an RTC instance if one is not already set. HAL API version 3 requires that
         * the RTC instance be allocated in the configurator and configured prior to this
         * function being called */
        #if defined(MTB_HAL_API_VERSION) && ((MTB_HAL_API_VERSION) >= 3)
        CY_ASSERT(false);
        return seconds;
        #else /* Older HAL versions define CYHAL_API_VERSION */
        result = cyhal_rtc_init(&cy_time_rtc_inst);
        CY_ASSERT(CY_RSLT_SUCCESS == result);
        if (CY_RSLT_SUCCESS == result)
        {
            cy_time = &cy_time_rtc_inst;

            /* Write default time to RTC */
            result = cyhal_rtc_write(cy_time, localtime(&seconds));
        }
        #endif // if defined(MTB_HAL_API_VERSION) && ((MTB_HAL_API_VERSION) >= 3)
    }
    #endif // defined(MTB_HAL_DISABLE_ERR_CHECK)

    mutex_acquire();

    /* Read current time from RTC */
    #if defined(MTB_HAL_API_VERSION) && ((MTB_HAL_API_VERSION) >= 3)
    result = mtb_hal_rtc_read(cy_time, &rtc_time);
    #else
    result = cyhal_rtc_read(cy_time, &rtc_time);
    #endif

    if (result == CY_RSLT_SUCCESS)
    {
        /* Convert tm format to time_t */
        seconds = mktime(&rtc_time);
    }

    mutex_release();
    if (result != CY_RSLT_SUCCESS)
    {
        if (_timer != NULL)
        {
            *_timer = ERROR;
        }
        return ERROR;
    }

    if (_timer != NULL)
    {
        *_timer = seconds;
    }

    return seconds;
}


//--------------------------------------------------------------------------------------------------
// Init the CLIB support RTC (HAL API >= 3.0)
//--------------------------------------------------------------------------------------------------
#if defined(MTB_HAL_API_VERSION) && ((MTB_HAL_API_VERSION) >= 3)
void mtb_clib_support_init(mtb_hal_rtc_t* rtc)
#else
//--------------------------------------------------------------------------------------------------
// Init the CLIB support RTC (HAL API <= 2.0)
//--------------------------------------------------------------------------------------------------
void mtb_clib_support_init(cyhal_rtc_t* rtc)
#endif
{
    mutex_acquire();

    cy_time = rtc;

    mutex_release();
}


//--------------------------------------------------------------------------------------------------
// Get the CLIB support RTC (HAL API >= 3.0)
//--------------------------------------------------------------------------------------------------
#if defined(MTB_HAL_API_VERSION) && ((MTB_HAL_API_VERSION) >= 3)
mtb_hal_rtc_t* mtb_clib_support_get_rtc(void)
#else
//--------------------------------------------------------------------------------------------------
// Get the CLIB support RTC (HAL API <= 2.0)
//--------------------------------------------------------------------------------------------------
cyhal_rtc_t* mtb_clib_support_get_rtc(void)
#endif
{
    return cy_time;
}


#if defined(__cplusplus)
}
#endif

#endif // if defined(_MTB_CLIB_SUPPORT_RTC_AVAILABLE)
