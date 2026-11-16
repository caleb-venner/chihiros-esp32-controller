#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "led_driver.h"
#include "safety.h"
#include "watchdog.h"
#include "wifi_manager.h"
#include "web_core.h"
#include "light_api.h"
#include "scheduler.h"

static const char *TAG = "chihiros_main";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting Chihiros ESP32 Controller...");

    /* 1. Initialise NVS (Required for Wi-Fi storage) */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* 2. Start Watchdog & Initialise LED Driver */
    watchdog_driver_init();
    led_driver_init();
    
    /* 3. Start Network, API & Scheduler */
    wifi_manager_init();
    httpd_handle_t server = web_core_start();
    if (server) light_api_register(server);
    scheduler_init();

    ESP_LOGI(TAG, "System running. Awaiting API requests...");

    /* 4. Idle Main Loop */
    while (1) {
        /* The HTTP server and Wi-Fi run on their own FreeRTOS tasks.
         * The main task just needs to keep petting the watchdog. */
        watchdog_pet();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
