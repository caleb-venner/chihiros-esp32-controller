#include "watchdog.h"
#include "esp_task_wdt.h"
#include "esp_log.h"

static const char* TAG = "watchdog";

void watchdog_driver_init(void)
{
    /*
     * Initialise the ESP-IDF Task Watchdog Timer (TWDT).
     * If the task fails to feed the watchdog within WATCHDOG_TIMEOUT_MS,
     * the ESP32 will panic and reboot, returning LEDs to their 0% boot state.
     */
    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = WATCHDOG_TIMEOUT_MS,
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1, 
        .trigger_panic = true
    };
    
    esp_err_t err = esp_task_wdt_init(&twdt_config);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_ERROR_CHECK(err);
    }
    
    /* Subscribe the current task (app_main) to the watchdog */
    esp_task_wdt_add(NULL);
    
    ESP_LOGI(TAG, "Watchdog initialised (2000ms timeout)");
}

void watchdog_pet(void)
{
    esp_task_wdt_reset();
}
