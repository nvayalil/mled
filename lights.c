#include <ws2811/ws2811.h>
#include <ws2811/clk.h>
#include <ws2811/gpio.h>
#include <ws2811/dma.h>
#include <ws2811/pwm.h>

#include <stdio.h>
#include <stdlib.h>
#include "lights.h"


#define TARGET_FREQ             WS2811_TARGET_FREQ
#define GPIO_PIN                10
#define DMA                     10
#define STRIP_TYPE              WS2812_STRIP

ws2811_t ledstring =
{
    .freq = TARGET_FREQ,
    .dmanum = DMA,
    .channel =
    {
        [0] =
        {
            .gpionum = GPIO_PIN,
            .count = LED_COUNT,
            .invert = 0,
            .brightness = 255,
            .strip_type = STRIP_TYPE,
        },
        [1] =
        {
            .gpionum = 0,
            .count = 0,
            .invert = 0,
            .brightness = 0,
        },
    },
};

int init_lights()
{
    ws2811_return_t ret;
    if ((ret = ws2811_init(&ledstring)) != WS2811_SUCCESS)
    {
        fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
        return ret;
    }

    for (uint16_t i = 0; i < LED_COUNT; i++)
    {
        ledstring.channel[0].leds[i] = 0;
    }
    
    return 0;
}

void free_lights()
{
    uint16_t i;
    ws2811_return_t ret;
    
    for (i = 0; i < LED_COUNT; i++)
        ledstring.channel[0].leds[i] = 0;

    if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS)
    {
        fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
    }

    ws2811_fini(&ledstring);
}

void fill_leds(uint16_t led_count, uint32_t color)
{
    uint16_t i;
    ws2811_return_t ret;
    for(i = 0; i < LED_COUNT; i++)
    {
        if(i < led_count)
            ledstring.channel[0].leds[i] = color;
        else
            ledstring.channel[0].leds[i] = 0;
    }
    if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS)
    {
        fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
    }
}

void set_all(uint8_t r, uint8_t g, uint8_t b)
{
    uint32_t color = (r << 16) | (g << 8) | b;
    for (uint16_t i = 0; i < LED_COUNT; i++)
    {
        ledstring.channel[0].leds[i] = color;
    }
    
}

void set_pixel(uint16_t position, uint8_t r, uint8_t g, uint8_t b)
{
    ledstring.channel[0].leds[position] = (r << 16) | (g << 8) | b;
}

void show_strip()
{
    ws2811_return_t ret;
    if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS)
    {
        fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
    }
}

uint32_t  get_random_color()
{
    return (uint32_t) random() & 0xFFFFFF;
}

uint32_t get_color(int16_t val)
{
    uint8_t r, g, b;
    val *= 2;
    if(val > 255)
    {
        r = val - 255;
        g = (510 - val);
        b = 0;
    }
    else
    {
        r = 0;
        g = val;
        b = (254 - val);
    }
    return (((uint32_t)r) << 16 | ((uint32_t)g) << 8 | ((uint32_t)b));
}

#if 0
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

uint32_t get_color(int16_t val)
{
    /*
        https://www.oreilly.com/library/view/python-cookbook/0596001673/ch09s11.html
    */
    int16_t blue = 764 - (4 * val);

    blue = MIN(blue, 255);
    blue = MAX(blue, 0);

    int16_t red = (4*val) - 255;
    red = MIN(red, 255);
    red = MAX(red, 0);

    if(val > 127)
        val -= 127;
    else
        val = 127 - val;
    int16_t green = 4 * val - 255;

    green = MIN(green, 255);
    green = MAX(green, 0);

    return (((uint32_t)red) << 16 | ((uint32_t)green) << 8 | ((uint32_t)blue));
}
#endif