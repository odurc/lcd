lcd
===

Library to control character LCDs featured with HD44780 or compatibles controllers.
Written in C, the library is designed to use only static memory allocation and it can be
used with any microcontroller.

Features
---

* CPU agnostic implementation
* use only static memory allocation
* full implementation of HD44780 controller features
* configurable via macros

How to install
---

Simply copy the content of src directory to your work directory.


Configuration
---

The configuration of the library is done by setting 'define' macros in the header file,
under the configuration section.

There are some mandatory configurations before the use, the GPIO include file and the setting
of the *LCD_GPIO_XXX* and *LCD_DELAY_us* macros. In the include files section replace the line
`#include "gpio.h"` with the include of your GPIO library. Then, in the configuration
section, adjust the macro *LCD_GPIO_XXX* to the GPIO functions of your library.

To use the library with an Arduino the configuration would be:

    #include <Arduino.h>
    // ...
    #define LCD_GPIO_SET(port,pin,value)    digitalWrite(pin,value)
    #define LCD_GPIO_DIR(port,pin,dir)      pinMode(pin,dir)
    #define LCD_GPIO_GET(port,pin)          digitalRead(pin)
    #define LCD_DELAY_us(time)              delayMicroseconds(time)

The other settings are preset and can be adjusted as needed. The macro *LCD_MAX_INSTANCES*
is used to define the total amount of LCDs being controlled by the library.

How to use
---

Once the configuration is done start by creating an instance of a LCD object (`lcd_create`) and
use the LCD control functions to configure the LCD as you wish. After the configuration use the
writing functions to print data to the LCD.

To see more details how to use the library, please check the online
[API documentation](http://ricardocrudo.github.io/lcd).

License
---

MIT
