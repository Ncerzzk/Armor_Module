#ifndef __SK8612__
#define __SK6812__
#include "stdint.h"
uint16_t * RGB2PWM_duty_array(uint32_t color);
void set_leds_color(uint32_t * colors,uint8_t led_cnt);
void leds_color_left_shift(uint32_t * colors,uint8_t led_cnt);
#endif