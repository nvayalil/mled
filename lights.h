#ifndef _LIGHTS_H_
#define _LIGHTS_H_
#define LED_COUNT   144
int init_lights();
void free_lights();
void fill_leds(uint16_t led_count, uint32_t color);
uint32_t get_color(int16_t val);
#endif