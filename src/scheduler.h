#pragma once

#include <stdint.h>
#include <stdbool.h>

#define MAX_KEYFRAMES 24

typedef enum {
    MODE_MANUAL = 0,
    MODE_AUTO = 1
} controller_mode_t;

typedef struct {
    int time_min; /* Minutes since midnight (e.g., 780 = 13:00) */
    int r;
    int g;
    int w;
    int b;
} keyframe_t;

typedef struct {
    controller_mode_t mode;
    
    /* Manual Mode Settings */
    int manual_r;
    int manual_g;
    int manual_w;
    int manual_b;
    
    /* Auto Mode Keyframes */
    int num_keyframes;
    keyframe_t keyframes[MAX_KEYFRAMES];
} system_state_t;

/* Initialise the SNTP time sync and the background scheduler task. */
void scheduler_init(void);

/* Save the new state to NVS and apply it. (Automatically sorts keyframes chronologically) */
void scheduler_set_state(system_state_t *state);

/* Retrieve the current system state. */
void scheduler_get_state(system_state_t *state);
