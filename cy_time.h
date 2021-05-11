/***********************************************************************************************//**
 * \file cy_time.h
 *
 * \brief
 * Provides API to read the time, based on the microcontroller
 * Real-Time Clock (RTC). Also provides API to set and get RTC instance.
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

#pragma once

#if defined(CY_USING_HAL)
#include "cyhal.h"

// Make sure the RTC is available on this device
#if defined(CY_RSLT_RTC_NOT_INITIALIZED)
#include "time.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Set the RTC instance for time function.
 *
 * @param[in] rtc_instance RTC instance to be set
 *
 */
void cy_set_rtc_instance(cyhal_rtc_t* rtc_instance);


/** Get the RTC instance.
 *
 * @return  Pointer to cyhal_rtc_t instance
 */
cyhal_rtc_t* cy_get_rtc_instance();

#ifdef __cplusplus
}
#endif

#endif // if defined(CY_RSLT_RTC_NOT_INITIALIZED)
#endif // if defined(CY_USING_HAL)
