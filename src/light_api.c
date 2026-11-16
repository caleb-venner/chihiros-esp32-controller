#include "light_api.h"

#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "cJSON.h"
#include "led_driver.h"
#include "scheduler.h"

static const char *TAG = "light_api";

/* Reject out-of-range input at the API boundary: channel levels are 0-100 %,
 * keyframe times are minutes since midnight (0-1439). */
static int clamp_int(int v, int lo, int hi) {
    return (v < lo) ? lo : (v > hi) ? hi : v;
}

static esp_err_t api_status_handler(httpd_req_t *req) {
    system_state_t state;
    scheduler_get_state(&state);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "red", led_get_duty(LED_CH_RED));
    cJSON_AddNumberToObject(root, "green", led_get_duty(LED_CH_GREEN));
    cJSON_AddNumberToObject(root, "blue", led_get_duty(LED_CH_BLUE));
    cJSON_AddNumberToObject(root, "white", led_get_duty(LED_CH_WHITE));
    cJSON_AddStringToObject(root, "mode", (state.mode == MODE_MANUAL) ? "manual" : "auto");
    
    cJSON *man = cJSON_CreateObject();
    cJSON_AddNumberToObject(man, "red", state.manual_r);
    cJSON_AddNumberToObject(man, "green", state.manual_g);
    cJSON_AddNumberToObject(man, "blue", state.manual_b);
    cJSON_AddNumberToObject(man, "white", state.manual_w);
    cJSON_AddItemToObject(root, "manual", man);
    
    cJSON *kfs = cJSON_CreateArray();
    for (int i = 0; i < state.num_keyframes; i++) {
        cJSON *kf = cJSON_CreateObject();
        cJSON_AddNumberToObject(kf, "time_min", state.keyframes[i].time_min);
        cJSON_AddNumberToObject(kf, "r", state.keyframes[i].r);
        cJSON_AddNumberToObject(kf, "g", state.keyframes[i].g);
        cJSON_AddNumberToObject(kf, "b", state.keyframes[i].b);
        cJSON_AddNumberToObject(kf, "w", state.keyframes[i].w);
        cJSON_AddItemToArray(kfs, kf);
    }
    cJSON_AddItemToObject(root, "keyframes", kfs);
    
    const char *sys_info = cJSON_Print(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, sys_info, strlen(sys_info));
    free((void *)sys_info);
    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t api_state_post_handler(httpd_req_t *req) {
    char buf[1024];
    int ret, remaining = req->content_len;
    
    if (remaining >= sizeof(buf)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Payload too large");
        return ESP_FAIL;
    }
    if ((ret = httpd_req_recv(req, buf, remaining)) <= 0) return ESP_FAIL;
    buf[ret] = '\0';
    
    cJSON *root = cJSON_Parse(buf);
    if (!root) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }
    
    system_state_t state;
    scheduler_get_state(&state);
    
    cJSON *mode_str = cJSON_GetObjectItem(root, "mode");
    if (mode_str && mode_str->valuestring) {
        if (strcmp(mode_str->valuestring, "manual") == 0) state.mode = MODE_MANUAL;
        else if (strcmp(mode_str->valuestring, "auto") == 0) state.mode = MODE_AUTO;
    }
    
    cJSON *man = cJSON_GetObjectItem(root, "manual");
    if (man) {
        cJSON *r = cJSON_GetObjectItem(man, "red");   if (r) state.manual_r = clamp_int(r->valueint, 0, 100);
        cJSON *g = cJSON_GetObjectItem(man, "green"); if (g) state.manual_g = clamp_int(g->valueint, 0, 100);
        cJSON *b = cJSON_GetObjectItem(man, "blue");  if (b) state.manual_b = clamp_int(b->valueint, 0, 100);
        cJSON *w = cJSON_GetObjectItem(man, "white"); if (w) state.manual_w = clamp_int(w->valueint, 0, 100);
    }
    
    cJSON *kfs = cJSON_GetObjectItem(root, "keyframes");
    if (kfs && cJSON_IsArray(kfs)) {
        int count = cJSON_GetArraySize(kfs);
        if (count > MAX_KEYFRAMES) count = MAX_KEYFRAMES;
        state.num_keyframes = count;
        
        for (int i=0; i<count; i++) {
            cJSON *item = cJSON_GetArrayItem(kfs, i);
            cJSON *tm = cJSON_GetObjectItem(item, "time_min"); if (tm) state.keyframes[i].time_min = clamp_int(tm->valueint, 0, 1439);
            cJSON *r = cJSON_GetObjectItem(item, "r"); if (r) state.keyframes[i].r = clamp_int(r->valueint, 0, 100);
            cJSON *g = cJSON_GetObjectItem(item, "g"); if (g) state.keyframes[i].g = clamp_int(g->valueint, 0, 100);
            cJSON *b = cJSON_GetObjectItem(item, "b"); if (b) state.keyframes[i].b = clamp_int(b->valueint, 0, 100);
            cJSON *w = cJSON_GetObjectItem(item, "w"); if (w) state.keyframes[i].w = clamp_int(w->valueint, 0, 100);
        }
    }
    
    scheduler_set_state(&state);
    cJSON_Delete(root);
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"state updated\"}", -1);
    return ESP_OK;
}

void light_api_register(httpd_handle_t server) {
    httpd_uri_t status_uri = { .uri = "/api/status", .method = HTTP_GET, .handler = api_status_handler };
    httpd_register_uri_handler(server, &status_uri);
    
    httpd_uri_t state_post_uri = { .uri = "/api/state", .method = HTTP_POST, .handler = api_state_post_handler };
    httpd_register_uri_handler(server, &state_post_uri);
}
