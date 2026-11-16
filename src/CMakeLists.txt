idf_component_register(SRCS "main.c" "light_api.c" "led_driver.c" "scheduler.c" "safety.c"
                       INCLUDE_DIRS "."
                       REQUIRES cjson wifi_manager web_core watchdog)
