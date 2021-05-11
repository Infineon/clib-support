# CLib FreeRTOS Support Library Release Notes
The CLib FreeRTOS Support Library provides the necessary hooks to make C library functions such as malloc and free thread safe.

### What's Included?
* Thread safety hooks for GCC Newlib
* Thread safety hooks for ARM C library
* Thread safety hooks for IAR C library
* Thread safety hooks for C++
* Hook for the system time() function

### What Changed?
#### v1.1.0
* Add support for using RTC to support time()
#### v1.0.2
* Minor update to work with a wider range of MCUs
#### v1.0.1
* Minor update for documentation & branding
#### v1.0.0
* Initial release

### Supported Software and Tools
This version of the CLib FreeRTOS Support Library was validated for compatibility with the following Software and Tools:

| Software and Tools                        | Version |
| :---                                      | :----:  |
| ModusToolbox Software Environment         | 2.2.1   |
| GCC Compiler                              | 9.3.1   |
| IAR Compiler                              | 8.4     |
| ARM Compiler                              | 6.11    |

Minimum required ModusToolbox Software Environment: v2.0

### More information
Use the following links for more information, as needed:
* [Reference Guide](https://cypresssemiconductorco.github.io/clib-support/html/index.html)
* [Cypress Semiconductor, an Infineon Technologies Company](http://www.cypress.com)
* [Cypress Semiconductor GitHub](https://github.com/cypresssemiconductorco)
* [ModusToolbox](https://www.cypress.com/products/modustoolbox-software-environment)

---
© Cypress Semiconductor Corporation, 2020-2021.