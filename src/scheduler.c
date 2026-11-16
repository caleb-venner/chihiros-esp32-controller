#include "scheduler.h"
#include "led_driver.h"
#include "config.h"
#include "shared_config.h"

#include <time.h>
#include <sys/time.h>
#include "esp_sntp.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "scheduler";
static system_state_t current_state;
static bool state_loaded = false;

static void save_state_to_nvs(void) {
    nvs_handle_t handle;
    if (nvs_open("storage", NVS_READWRITE, &handle) == ESP_OK) {
        nvs_set_blob(handle, "sys_state", &current_state, sizeof(system_state_t));
        nvs_commit(handle);
        nvs_close(handle);
    }
}

static void load_state_from_nvs(void) {
    nvs_handle_t handle;
    if (nvs_open("storage", NVS_READONLY, &handle) == ESP_OK) {
        size_t len = sizeof(system_state_t);
        if (nvs_get_blob(handle, "sys_state", &current_state, &len) == ESP_OK) {
            state_loaded = true;
            ESP_LOGI(TAG, "Loaded system state from NVS");
        }
        nvs_close(handle);
    }
    
    if (!state_loaded) {
        // Safe default if boot for the first time
        current_state.mode = MODE_MANUAL;
        current_state.manual_r = 0; current_state.manual_g = 0; 
        current_state.manual_w = 0; current_state.manual_b = 0;
        current_state.num_keyframes = 0;
        state_loaded = true;
    }
}

void scheduler_set_state(system_state_t *state) {
    /* Bubble sort keyframes chronologically to guarantee robust math */
    for (int i = 0; i < state->num_keyframes - 1; i++) {
        for (int j = 0; j < state->num_keyframes - i - 1; j++) {
            if (state->keyframes[j].time_min > state->keyframes[j+1].time_min) {
                keyframe_t temp = state->keyframes[j];
                state->keyframes[j] = state->keyframes[j+1];
                state->keyframes[j+1] = temp;
            }
        }
    }
    
    current_state = *state;
    save_state_to_nvs();
    ESP_LOGI(TAG, "New state applied and saved.");
}

void scheduler_get_state(system_state_t *state) {
    *state = current_state;
}

static void time_sync_init(void) {
    ESP_LOGI(TAG, "Initialising SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
    
    setenv("TZ", TIMEZONE, 1);
    tzset();
}

static void scheduler_task(void *arg) {
    while (1) {
        if (!state_loaded) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        if (current_state.mode == MODE_MANUAL) {
            /* Manual Mode: Instantly apply statics */
            led_set_all(current_state.manual_r, current_state.manual_g, 
                        current_state.manual_w, current_state.manual_b);
        } 
        else {
            /* Auto Mode: Keyframe Interpolation */
            time_t now;
            struct tm timeinfo;
            time(&now);
            localtime_r(&now, &timeinfo);

            /* Only execute auto if SNTP has synced the time */
            if (timeinfo.tm_year > (2020 - 1900)) {
                if (current_state.num_keyframes == 0) {
                    led_set_all(0, 0, 0, 0);
                } 
                else if (current_state.num_keyframes == 1) {
                    led_set_all(current_state.keyframes[0].r, current_state.keyframes[0].g,
                                current_state.keyframes[0].w, current_state.keyframes[0].b);
                } 
                else {
                    /* Calculate time in minutes with fractional seconds for buttery smooth fading */
                    float current_time = (timeinfo.tm_hour * 60) + timeinfo.tm_min + (timeinfo.tm_sec / 60.0f);
                    
                    keyframe_t *kf_prev = &current_state.keyframes[current_state.num_keyframes - 1];
                    keyframe_t *kf_next = &current_state.keyframes[0];

                    for (int i = 0; i < current_state.num_keyframes - 1; i++) {
                        if (current_time >= current_state.keyframes[i].time_min && 
                            current_time < current_state.keyframes[i+1].time_min) {
                            kf_prev = &current_state.keyframes[i];
                            kf_next = &current_state.keyframes[i+1];
                            break;
                        }
                    }
                    /* If not found, it means we are spanning midnight. kf_prev is the last frame of the day, kf_next is the first frame. */

                    /* Calculate interpolation progress handling midnight wrap-around */
                    float time_diff = kf_next->time_min - kf_prev->time_min;
                    if (time_diff < 0) time_diff += 1440.0f;

                    float time_passed = current_time - kf_prev->time_min;
                    if (time_passed < 0) time_passed += 1440.0f;

                    float progress = (time_diff == 0) ? 0 : (time_passed / time_diff);

                    int r = kf_prev->r + (int)((kf_next->r - kf_prev->r) * progress);
                    int g = kf_prev->g + (int)((kf_next->g - kf_prev->g) * progress);
                    int w = kf_prev->w + (int)((kf_next->w - kf_prev->w) * progress);
                    int b = kf_prev->b + (int)((kf_next->b - kf_prev->b) * progress);

                    led_set_all(r, g, w, b);
                }
            }
        }

        /* Evaluate every 2 seconds */
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void scheduler_init(void) {
    load_state_from_nvs();
    time_sync_init();
    xTaskCreate(scheduler_task, "scheduler_task", 4096, NULL, 5, NULL);
}
