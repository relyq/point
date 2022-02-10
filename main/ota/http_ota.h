#ifndef __HTTP_OTA_H__
#define __HTTP_OTA_H__
#include <stdbool.h>

#include "esp_err.h"

void simple_ota_example_task(void* pvParameter);
void perform_ota_update(const char* url);
esp_err_t nvs_ota_url_get(char* url);
esp_err_t nvs_update_flag_get(bool* update_flag);
void get_sha256_of_partitions(void);
// only for debugging purposes
void print_running_partition(void);
#endif  // __HTTP_OTA_H__