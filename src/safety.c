#include "safety.h"

static const uint8_t s_max_pct[LED_CH_COUNT] = {
    [LED_CH_RED]   = SAFETY_MAX_RED_PCT,
    [LED_CH_GREEN] = SAFETY_MAX_GREEN_PCT,
    [LED_CH_WHITE] = SAFETY_MAX_WHITE_PCT,
    [LED_CH_BLUE]  = SAFETY_MAX_BLUE_PCT,
};

uint8_t safety_clamp(led_channel_t channel, uint8_t duty_pct)
{
    if (channel >= LED_CH_COUNT) {
        return 0u;
    }
    uint8_t ceiling = s_max_pct[channel];
    return (duty_pct > ceiling) ? ceiling : duty_pct;
}
