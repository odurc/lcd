/*
 * Copyright (c) 2017 Ricardo Crudo <ricardo.crudo@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef LCD_H
#define LCD_H

#ifdef __cplusplus
extern "C"
{
#endif

/*
****************************************************************************************************
*       INCLUDE FILES
****************************************************************************************************
*/

#include <stdint.h>

// adjust the header according your library
#include "gpio.h"


/*
****************************************************************************************************
*       MACROS
****************************************************************************************************
*/

#define LCD_VERSION     "0.1.0"

// useful macro to print at a specific position
#define lcd_print_at(lcd,str,line,col)  lcd_cursor_set(lcd, line, col);\
                                        lcd_print(lcd, str)


/*
****************************************************************************************************
*       CONFIGURATION
****************************************************************************************************
*/

// configure the GPIO functions
#define LCD_GPIO_SET(port,pin,value)    gpio_set(port,pin,value)
#define LCD_GPIO_DIR(port,pin,dir)      gpio_dir(port,pin,dir)
#define LCD_GPIO_GET(port,pin)          gpio_get(port,pin)

// configure the delay function
#define LCD_DELAY_us(time)      delay_us(time)

// maximum of LCD instances
#define LCD_MAX_INSTANCES       1

// uncomment the definition below to enable RW support
//#define LCD_RW_SUPPORT


/*
****************************************************************************************************
*       DATA TYPES
****************************************************************************************************
*/

/** An opaque structure representing a lcd object */
typedef struct lcd_t lcd_t;

/** Configuration flags. */
typedef enum lcd_config_t {
    LCD_8BIT = 0x10,        /**< Use 8bit interface */
    LCD_4BIT = 0x00,        /**< Use 4bit interface */
    LCD_2LINE = 0x08,       /**< Use 2 lines */
    LCD_1LINE = 0x00,       /**< Use 1 line */
    LCD_5x10 = 0x04,        /**< Use 5x10 character size */
    LCD_5x8 = 0x00          /**< Use 5x8 character size */
} lcd_config_t;

/** LCD state flags. */
typedef enum lcd_state_t {
    LCD_OFF = 0x00,         /**< Turn LCD screen off */
    LCD_ON = 0x04           /**< Turn LCD screen on */
} lcd_state_t;

/** Cursor state flags. */
typedef enum lcd_cursor_t {
    LCD_CURSOR_OFF = 0x00,  /**< Turn on cursor */
    LCD_CURSOR_ON = 0x02    /**< Turn off cursor */
} lcd_cursor_t;

/** Cursor blink flags. */
typedef enum lcd_blink_t {
    LCD_BLINK_OFF = 0x00,   /**< Cursor blink off */
    LCD_BLINK_ON = 0x01     /**< Cursor blink on */
} lcd_blink_t;

/** Text direction flags. */
typedef enum lcd_direction_t {
    LCD_RIGHT_TO_LEFT = 0x00,   /**< Text is written from right to left */
    LCD_LEFT_TO_RIGHT = 0x02    /**< Text is written from left to right */
} lcd_direction_t;

/** Autoscroll flags. */
typedef enum lcd_autoscroll_t {
    LCD_AUTOSCROOL_OFF = 0x00,  /**< Disable autoscroll */
    LCD_AUTOSCROOL_ON = 0x01    /**< Enable autoscroll */
} lcd_autoscroll_t;

/** Scroll flags. */
typedef enum lcd_scroll_t {
    LCD_SCROLL_TO_LEFT = 0x00,  /**< Scroll screen to left */
    LCD_SCROLL_TO_RIGHT = 0x04  /**< Scroll screen to right */
} lcd_scroll_t;


/*
****************************************************************************************************
*       FUNCTION PROTOTYPES
****************************************************************************************************
*/

/**
 * @defgroup lcd_ctrl_funcs LCD Control Functions
 * Set of functions to control the LCD
 * @{
 */

/**
 * Create LCD object
 *
 * The \a config flags might be combined using the OR operator.
 *
 * The \a control_pins must be provided in the following sequence: RS, EN, RW (RW only if
 * \a LCD_RW_SUPPORT is defined).
 *
 * The \a data_pins must be provided in the following sequence:
 * - D0, D1, D2, D3, D4, D5, D6, D7 for 8bit; or
 * - D4, D5, D6, D7 for 4bit
 *
 * Example:
 *      \code{.c}
 *      // {RS_port, RS_pin, EN_port, EN_pin}
 *      const int control_pins[] = {0, 1,  0, 2};
 *      // {D4_port, D4_pin, D5_port, D5_pin, D6_port, D6_pin, D7_port, D7_pin}
 *      const int data_pins[] = {0, 3,  0, 4,  0, 5,  0, 6};
 *      // create lcd object
 *      lcd_t *lcd = lcd_create(LCD_4BIT | LCD_2LINE, control_pins, data_pins);
 *      \endcode
 *
 * @param[in]  config        the configuration of the LCD
 * @param[in]  control_pins  integer array with the control pins
 * @param[in]  data_pins     integer array with the data pins
 *
 * @return     pointer to the LCD object or NULL if no more instances are available
 */
lcd_t* lcd_create(lcd_config_t config, const int *control_pins, const int *data_pins);

/**
 * Destroy lcd object
 *
 * @param      lcd    lcd object
 */
void lcd_destroy(lcd_t *lcd);

/**
 * Enable/Disable LCD screen
 *
 * The LCD doesn't lose the written data when disabled.
 *
 * @param      lcd    lcd object
 * @param[in]  state  screen state
 */
void lcd_state(lcd_t *lcd, lcd_state_t state);

/**
 * Clear LCD screen
 *
 * @param      lcd   lcd object
 */
void lcd_clear(lcd_t *lcd);

/**
 * Move cursor at the home position
 *
 * @param      lcd   lcd object
 */
void lcd_home(lcd_t *lcd);

/**
 * Move cursor to a specific position
 *
 * @param      lcd   lcd object
 * @param[in]  line  line number (starting from 0)
 * @param[in]  col   column number (starting from 0)
 */
void lcd_cursor_set(lcd_t *lcd, int line, int col);

/**
 * Enable/Disable cursor
 *
 * @param      lcd    lcd object
 * @param[in]  state  cursor state
 */
void lcd_cursor_state(lcd_t *lcd, lcd_cursor_t state);

/**
 * Enable/Disable cursor blinking
 *
 * @param      lcd    lcd object
 * @param[in]  state  blinking state
 */
void lcd_cursor_blink(lcd_t *lcd, lcd_blink_t state);

/**
 * Set text writing direction
 *
 * @param      lcd        lcd object
 * @param[in]  direction  text direction
 */
void lcd_text_direction(lcd_t *lcd, lcd_direction_t direction);

/**
 * Enable/Disable text autoscroll
 *
 * @param      lcd    lcd object
 * @param[in]  state  autoscroll state
 */
void lcd_autoscroll(lcd_t *lcd, lcd_autoscroll_t state);

/**
 * Set screen scroll direction
 *
 * @param      lcd        lcd object
 * @param[in]  direction  scroll direction
 */
void lcd_scroll(lcd_t *lcd, lcd_scroll_t direction);

/**
 * Create custom character for the LCD
 *
 * @param      lcd      lcd object
 * @param[in]  index    index where to create the character (0 to 7)
 * @param[in]  charmap  array containing the character bits map
 */
void lcd_create_char(lcd_t *lcd, int index, const uint8_t *charmap);

/**
 * @}
 * @defgroup lcd_writing_funcs LCD Writing Functions
 * Set of functions to write on the LCD
 * @{
 */

/**
 * Print a character at the current cursor position of the LCD
 *
 * @param      lcd   lcd object
 * @param[in]  c     the character
 */
void lcd_print_char(lcd_t *lcd, char c);

/**
 * Print a string at the current cursor position of the LCD
 *
 * @param      lcd   lcd object
 * @param[in]  str   the string
 */
void lcd_print_str(lcd_t *lcd, const char *str);

/**
 * Print an integer at the current cursor position of the LCD
 *
 * @param      lcd   lcd object
 * @param[in]  num   the integer
 */
void lcd_print_int(lcd_t *lcd, int num);

/**
 * @}
 */


/*
****************************************************************************************************
*       CONFIGURATION ERRORS
****************************************************************************************************
*/

#ifdef __cplusplus
}
#endif

// LCD_H
#endif
