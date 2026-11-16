#include "led_driver.h"
#include "safety.h"
#include "config.h"

#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_err.h"

static uint8_t s_duty_pct[LED_CH_COUNT] = {0};

static const int s_pin[LED_CH_COUNT] = {
    [LED_CH_RED]   = PIN_RED,
    [LED_CH_GREEN] = PIN_GREEN,
    [LED_CH_WHITE] = PIN_WHITE,
    [LED_CH_BLUE]  = PIN_BLUE,
};

static const ledc_channel_t s_ledc_ch[LED_CH_COUNT] = {
    [LED_CH_RED]   = LEDC_CHANNEL_0,
    [LED_CH_GREEN] = LEDC_CHANNEL_1,
    [LED_CH_WHITE] = LEDC_CHANNEL_2,
    [LED_CH_BLUE]  = LEDC_CHANNEL_3,
};

/* Convert 0-100% to 13-bit LEDC duty, capped at 100/140th of max */
static uint32_t duty_to_level(uint8_t duty_pct)
{
    if (duty_pct == 0u) return 0;
    if (duty_pct >= 100u) duty_pct = 100u;
    
    /* Cap physical PWM wrap to 100/140 of the absolute maximum */
    uint32_t physical_max = ((uint32_t)PWM_WRAP * 100u) / 140u;
    
    return (((uint32_t)duty_pct * physical_max) / 100u);
}

static void write_pwm_level(led_channel_t channel, uint32_t level)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, s_ledc_ch[channel], level);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, s_ledc_ch[channel]);
}

void led_driver_init(void)
{
    /* 1. Configure LEDC Timer */
    ledc_timer_config_t timer_conf = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = PWM_RES_BITS,
        .freq_hz          = PWM_FREQ_HZ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_conf);

    /* 2. Configure Channels */
    for (led_channel_t i = 0; i < LED_CH_COUNT; i++) {
        ledc_channel_config_t ch_conf = {
            .gpio_num       = s_pin[i],
            .speed_mode     = LEDC_LOW_SPEED_MODE,
            .channel        = s_ledc_ch[i],
            .intr_type      = LEDC_INTR_DISABLE,
            .timer_sel      = LEDC_TIMER_0,
            .duty           = 0, /* Start strictly at 0 */
            .hpoint         = 0
        };
        ledc_channel_config(&ch_conf);
        s_duty_pct[i] = 0;
    }
}

void led_set_duty(led_channel_t channel, uint8_t duty_pct)
{
    if (channel >= LED_CH_COUNT) return;

    uint8_t clamped = safety_clamp(channel, duty_pct);
    write_pwm_level(channel, duty_to_level(clamped));
    s_duty_pct[channel] = clamped;
}

void led_set_all(uint8_t red_pct, uint8_t green_pct,
                 uint8_t white_pct, uint8_t blue_pct)
{
    led_set_duty(LED_CH_RED,   red_pct);
    led_set_duty(LED_CH_GREEN, green_pct);
    led_set_duty(LED_CH_WHITE, white_pct);
    led_set_duty(LED_CH_BLUE,  blue_pct);
}

uint8_t led_get_duty(led_channel_t channel)
{
    if (channel >= LED_CH_COUNT) return 0;
    return s_duty_pct[channel];
}

void led_emergency_off(void)
{
    for (led_channel_t i = 0; i < LED_CH_COUNT; i++) {
        write_pwm_level(i, 0);
        s_duty_pct[i] = 0;
    }
}
