/***********************************************************************************************//**
 * \file cy_time.h
 *
 * \brief
 * Provides API to read the time, based on the microcontroller
 * Real-Time Clock (RTC). Also provides API to init and get CLIB support RTC.
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

#pragma once

#if defined(COMPONENT_MTB_HAL)
#include "mtb_hal.h"
#elif defined(CY_USING_HAL)
#include "cyhal.h"
#endif

// Make sure the RTC is available on this device
#if (defined(MTB_HAL_DRIVER_AVAILABLE_RTC) && (MTB_HAL_DRIVER_AVAILABLE_RTC)) \
    || (defined(CYHAL_DRIVER_AVAILABLE_RTC) && (CYHAL_DRIVER_AVAILABLE_RTC)) \
    /* Early HAL versions did not define CYHAL_DRIVER_AVAILABLE macros */ \
    || (!defined(CYHAL_DRIVER_AVAILABLE_RTC) && defined(CY_RSLT_RTC_NOT_INITIALIZED))
#define _MTB_CLIB_SUPPORT_RTC_AVAILABLE

#include "time.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize the CLIB support library.
 *
 * @param[in] rtc Pointer to the RTC handle
 * \note This argument may be NULL if the time() function will not be used.
 */
#if defined(MTB_HAL_API_VERSION) && ((MTB_HAL_API_VERSION) >= 3)
void mtb_clib_support_init(mtb_hal_rtc_t* rtc);
#else
void mtb_clib_support_init(cyhal_rtc_t* rtc);
#endif

/** Deprecated, replaced by @ref mtb_clib_support_init */
#define cy_set_rtc_instance mtb_clib_support_init

/** Deprecated, replaced by @ref mtb_clib_support_init */
#define cy_get_rtc_instance mtb_clib_support_get_rtc

/** Get the CLIB support RTC.
 *
 * @return  Pointer to RTC handle
 */
#if defined(MTB_HAL_API_VERSION) && ((MTB_HAL_API_VERSION) >= 3)
mtb_hal_rtc_t* mtb_clib_support_get_rtc(void);
#else
cyhal_rtc_t* mtb_clib_support_get_rtc(void);
#endif

#ifdef __cplusplus
}
#endif

#endif // if defined(MTB_HAL_DRIVER_AVAILABLE_RTC)
