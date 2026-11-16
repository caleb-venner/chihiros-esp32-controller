#pragma once

#include <stdint.h>
#include "led_driver.h"

/* 
 * Limits: All channels confirmed to have hardware current regulation.
 * Firmware maps 100% here to the safe physical maximum (71.4% PWM).
 */
#define SAFETY_MAX_RED_PCT    100u
#define SAFETY_MAX_GREEN_PCT  100u
#define SAFETY_MAX_WHITE_PCT  100u
#define SAFETY_MAX_BLUE_PCT   100u

/* Returns the lower of `duty_pct` and the configured ceiling. */
uint8_t safety_clamp(led_channel_t channel, uint8_t duty_pct);
