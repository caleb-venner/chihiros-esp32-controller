#pragma once

/* 
 * FreeRTOS Task Watchdog Timer (TWDT) wrapper.
 * Resets the ESP32 if the main task stops responding.
 */
#define WATCHDOG_TIMEOUT_MS  2000

void watchdog_driver_init(void);
void watchdog_pet(void);
