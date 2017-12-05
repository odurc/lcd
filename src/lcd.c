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

/*
****************************************************************************************************
*       INCLUDE FILES
****************************************************************************************************
*/

#include "lcd.h"


/*
****************************************************************************************************
*       INTERNAL MACROS
****************************************************************************************************
*/

#ifndef GPIO_INPUT
#define GPIO_INPUT      0
#endif

#ifndef GPIO_OUTPUT
#define GPIO_OUTPUT     1
#endif

#define GPIO_SET(...)   LCD_GPIO_SET(__VA_ARGS__)
#define GPIO_GET(...)   LCD_GPIO_GET(__VA_ARGS__)
#define GPIO_DIR(...)   LCD_GPIO_DIR(__VA_ARGS__)

// commands
#define LCD_CLEAR_DISPLAY   0x01
#define LCD_RETURN_HOME     0x02
#define LCD_ENTRY_MODE_SET  0x04
#define LCD_DISPLAY_CONTROL 0x08
#define LCD_CURSOR_SHIFT    0x10
#define LCD_FUNCTION_SET    0x20
#define LCD_SET_CGRAM_ADDR  0x40
#define LCD_SET_DDRAM_ADDR  0x80

// flags for display/cursor shift
#define LCD_DISPLAY_MOVE    0x08
#define LCD_CURSOR_MOVE     0x00

// macros to select pins
#define RS_PIN(lcd)     lcd->control_pins[0], lcd->control_pins[1]
#define EN_PIN(lcd)     lcd->control_pins[2], lcd->control_pins[3]
#define RW_PIN(lcd)     lcd->control_pins[4], lcd->control_pins[5]
#define DATA_PIN(lcd,d) lcd->data_pins[d*2], lcd->data_pins[d*2+1]


/*
****************************************************************************************************
*       INTERNAL CONSTANTS
****************************************************************************************************
*/


/*
****************************************************************************************************
*       INTERNAL DATA TYPES
****************************************************************************************************
*/

struct lcd_t {
    const int *control_pins, *data_pins;
    int interface, control, mode;
};

enum {LCD_CMD, LCD_DATA};
enum {LCD_WRITE, LCD_READ};


/*
****************************************************************************************************
*       INTERNAL GLOBAL VARIABLES
****************************************************************************************************
*/

static lcd_t g_lcd[LCD_MAX_INSTANCES];


/*
****************************************************************************************************
*       INTERNAL FUNCTIONS
****************************************************************************************************
*/

static inline lcd_t* lcd_instantiate(void)
{
    static int counter;

    if (counter < LCD_MAX_INSTANCES)
        return &g_lcd[counter++];

    // iterate array searching for a free spot
    for (int i = 0; i < LCD_MAX_INSTANCES; i++)
    {
        lcd_t *lcd = &g_lcd[i];

        if (lcd->control_pins == 0)
            return lcd;
    }

    return 0;
}

// enable pulse
static void lcd_enable(lcd_t *lcd)
{
    GPIO_SET(EN_PIN(lcd), 1);
    LCD_DELAY_us(1);
    GPIO_SET(EN_PIN(lcd), 0);
}

// check busy flag
#ifdef LCD_RW_SUPPORT
static void lcd_busy(lcd_t *lcd)
{
    GPIO_SET(RS_PIN(lcd), LCD_CMD);
    GPIO_SET(RW_PIN(lcd), LCD_READ);

    // configure pin d7 as input
    int d7 = lcd->interface - 1;
    GPIO_DIR(DATA_PIN(lcd, d7), GPIO_INPUT);

    int busy = 1;
    while (busy)
    {
        GPIO_SET(EN_PIN(lcd), 1);
        LCD_DELAY_us(1);
        busy = GPIO_GET(DATA_PIN(lcd, d7));
        GPIO_SET(EN_PIN(lcd), 0);

        if (lcd->interface == 4)
            lcd_enable(lcd);
    }

    // configure pin d7 as output and disable reading
    GPIO_DIR(DATA_PIN(lcd, d7), GPIO_OUTPUT);
    GPIO_SET(RW_PIN(lcd), LCD_WRITE);
}
#endif

// write to lcd GPIOs
static void lcd_write(lcd_t *lcd, uint8_t value)
{
    for (int i = 0; i < (int) lcd->interface; i++)
        GPIO_SET(DATA_PIN(lcd, i), (value >> i) & 1);

    lcd_enable(lcd);

#ifndef LCD_RW_SUPPORT
    // uses hardcoded delay if RW pin is not enabled
    LCD_DELAY_us(50);
#endif
}

// send command or data to lcd
static void lcd_send(lcd_t *lcd, uint8_t value, uint8_t cmd_data)
{
#ifdef LCD_RW_SUPPORT
    lcd_busy(lcd);
#endif

    GPIO_SET(RS_PIN(lcd), cmd_data);

    if (lcd->interface == 4)
        lcd_write(lcd, value >> 4);

    lcd_write(lcd, value);
}


/*
****************************************************************************************************
*       GLOBAL FUNCTIONS
****************************************************************************************************
*/

lcd_t* lcd_create(lcd_config_t config, const int *control_pins, const int *data_pins)
{
    lcd_t *lcd = lcd_instantiate();

    if (!lcd || !control_pins || !data_pins)
        return 0;

    // initialize variables
    lcd->control_pins = control_pins;
    lcd->data_pins = data_pins;
    lcd->interface = (config & LCD_8BIT ? 8 : 4);

    // configure GPIOs as output
    GPIO_DIR(RS_PIN(lcd), GPIO_OUTPUT);
    GPIO_DIR(EN_PIN(lcd), GPIO_OUTPUT);

#ifdef LCD_RW_SUPPORT
    GPIO_DIR(RW_PIN(lcd), GPIO_OUTPUT);
    GPIO_SET(RW_PIN(lcd), LCD_WRITE);
#endif

    for (int i = 0; i < (int) lcd->interface; i++)
        GPIO_DIR(DATA_PIN(lcd, i), GPIO_OUTPUT);

    // see datasheet pages 45, 46 for initialization proceeding
    // https://www.sparkfun.com/datasheets/LCD/HD44780.pdf

    // display initialization time
    LCD_DELAY_us(50000);

    GPIO_SET(RS_PIN(lcd), 0);
    GPIO_SET(EN_PIN(lcd), 0);

    // initialization in 4 bits interface
    if (lcd->interface == 4)
    {
        // function set
        lcd_write(lcd, 0x03);
        LCD_DELAY_us(4500);

        // function set
        lcd_write(lcd, 0x03);
        LCD_DELAY_us(150);

        // function set
        lcd_write(lcd, 0x03);
        LCD_DELAY_us(150);

        lcd_write(lcd, 0x02);
    }
    else
    {
        lcd_send(lcd, config, LCD_CMD);
        LCD_DELAY_us(4500);

        lcd_send(lcd, config, LCD_CMD);
        LCD_DELAY_us(150);

        lcd_send(lcd, config, LCD_CMD);
    }

    // set interface, number of lines and font size
    config |= LCD_FUNCTION_SET;
    lcd_send(lcd, config, LCD_CMD);

    // display control
    // display on, cursor off and blinking off by default
    lcd->control = LCD_DISPLAY_CONTROL | LCD_ON;
    lcd_send(lcd, lcd->control, LCD_CMD);

    // clear display
    lcd_clear(lcd);

    // entry mode
    // no scrolling and left to right shifting by default
    lcd->mode = LCD_ENTRY_MODE_SET | LCD_LEFT_TO_RIGHT;
    lcd_send(lcd, lcd->mode, LCD_CMD);

    return lcd;
}

inline void lcd_destroy(lcd_t *lcd)
{
    if (lcd)
        lcd->control_pins = 0;
}

void lcd_state(lcd_t *lcd, lcd_state_t state)
{
    lcd->control &= ~LCD_ON;
    lcd->control |= state;
    lcd_send(lcd, lcd->control, LCD_CMD);
}

void lcd_clear(lcd_t *lcd)
{
    lcd_send(lcd, LCD_CLEAR_DISPLAY, LCD_CMD);
    delay_us(2000);
}

void lcd_home(lcd_t *lcd)
{
    lcd_send(lcd, LCD_RETURN_HOME, LCD_CMD);
    delay_us(2000);
}

void lcd_cursor_set(lcd_t *lcd, int line, int col)
{
    static const int lines_offset[] = {0x00, 0x40, 0x10, 0x50};
    uint8_t address = LCD_SET_DDRAM_ADDR | (col + lines_offset[line]);
    lcd_send(lcd, address, LCD_CMD);
}

void lcd_cursor_state(lcd_t *lcd, lcd_cursor_t state)
{
    lcd->control &= ~LCD_CURSOR_ON;
    lcd->control |= state;
    lcd_send(lcd, lcd->control, LCD_CMD);
}

void lcd_cursor_blink(lcd_t *lcd, lcd_blink_t state)
{
    lcd->control &= ~LCD_BLINK_ON;
    lcd->control |= state;
    lcd_send(lcd, lcd->control, LCD_CMD);
}

void lcd_text_direction(lcd_t *lcd, lcd_direction_t direction)
{
    lcd->mode &= ~LCD_LEFT_TO_RIGHT;
    lcd->mode |= direction;
    lcd_send(lcd, lcd->mode, LCD_CMD);
}

void lcd_autoscroll(lcd_t *lcd, lcd_autoscroll_t state)
{
    lcd->mode &= ~LCD_AUTOSCROOL_ON;
    lcd->mode |= state;
    lcd_send(lcd, lcd->mode, LCD_CMD);
}

void lcd_scroll(lcd_t *lcd, lcd_scroll_t direction)
{
    lcd_send(lcd, LCD_CURSOR_SHIFT | LCD_DISPLAY_MOVE | direction, LCD_CMD);
}

void lcd_create_char(lcd_t *lcd, int index, const uint8_t *charmap)
{
    index &= 0x07;

    lcd_send(lcd, LCD_SET_CGRAM_ADDR | (index << 3), LCD_CMD);

    for (int i = 0; i < 8; i++)
        lcd_send(lcd, charmap[i], LCD_DATA);
}

void lcd_print_char(lcd_t *lcd, char c)
{
    lcd_send(lcd, c, LCD_DATA);
}

void lcd_print_str(lcd_t *lcd, const char *str)
{
    const char *pstr = str;

    while (*pstr)
        lcd_send(lcd, *pstr++, LCD_DATA);
}

void lcd_print_int(lcd_t *lcd, int num)
{
    if (num < 0)
    {
        lcd_send(lcd, '-', LCD_DATA);
        num = -num;
    }

    int i = 0;
    char buffer[12];
    while (num > 0)
    {
        buffer[i++] = (num % 10) + '0';
        num /= 10;
    }

    while (i > 0)
        lcd_send(lcd, buffer[--i], LCD_DATA);
}
