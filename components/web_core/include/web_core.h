#pragma once
#include "esp_http_server.h"

/* Start the ESP32 HTTP web server and register shared endpoints like OTA. */
httpd_handle_t web_core_start(void);
