#ifndef _LIGHTS_H_
#define _LIGHTS_H_
#include <stdint.h>

#define LED_COUNT   144


//  F U N C T I O N   P R O T O T Y P E S
int init_lights();
void free_lights();
void fill_leds(uint16_t led_count, uint32_t color);
uint32_t get_color(int16_t val);
uint32_t get_random_color();

void set_all(uint8_t r, uint8_t g, uint8_t b);
void set_pixel(uint16_t position, uint8_t r, uint8_t g, uint8_t b);
void show_strip();

#endif