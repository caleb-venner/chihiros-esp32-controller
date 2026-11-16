#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    LED_CH_RED   = 0,
    LED_CH_GREEN = 1,
    LED_CH_WHITE = 2,
    LED_CH_BLUE  = 3,
    LED_CH_COUNT = 4,
} led_channel_t;

/* 
 * Initialise the ESP32 LEDC peripheral.
 * Pins are held low before the timer is started.
 */
void led_driver_init(void);

/* 
 * Set the duty cycle for a single channel.
 * @param duty_pct Requested duty cycle, 0–100 (%).
 * Scaled internally so 100% matches the Chihiros 140% hardware physical limit.
 */
void led_set_duty(led_channel_t channel, uint8_t duty_pct);

/* Convenience wrapper for all 4 channels. */
void led_set_all(uint8_t red_pct, uint8_t green_pct,
                 uint8_t white_pct, uint8_t blue_pct);

/* Read back the current (clamped) duty cycle. */
uint8_t led_get_duty(led_channel_t channel);

/* Bypass the clamp and immediately turn off all channels (fault path). */
void led_emergency_off(void);
