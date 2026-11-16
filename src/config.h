#pragma once

/* ── Boot-Safe GPIO Pin assignments (ESP32-S3) ──────────────────────────────
 *
 * It is critical to avoid ESP32-S3 "strapping pins" (0, 3, 45, 46) and pins
 * that might output high/PWM at boot to prevent the LEDs flashing at 
 * full brightness the moment power is applied.
 *
 * The pins on the right edge (7-13) are safe output pins that float or 
 * remain low during the bootloader sequence.
 */
#define PIN_GREEN  7
#define PIN_BLUE   9
#define PIN_RED    11
#define PIN_WHITE  13

/* ── LEDC (PWM) parameters ──────────────────────────────────────────────────
 *
 * 1 kHz frequency (within PT4121 100 Hz–20 kHz spec).
 * 13-bit resolution gives 8192 steps (0–8191), which is plenty for smooth fades.
 */
#define PWM_FREQ_HZ   1000
#define PWM_RES_BITS  13
#define PWM_WRAP      ((1 << PWM_RES_BITS) - 1)  /* 8191 */
