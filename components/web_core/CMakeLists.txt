idf_component_register(SRCS "web_core.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_http_server esp_https_ota app_update spi_flash)
