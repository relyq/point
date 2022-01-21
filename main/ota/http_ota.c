#include "http_ota.h"

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "freertos/task.h"
#include "nvs.h"

#define HASH_LEN 32

static const char *TAG = "OTA";

extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

static void print_sha256(const uint8_t *image_hash, const char *label);
static esp_err_t _http_event_handler(esp_http_client_event_t *evt);
static esp_err_t nvs_update_flag_set(void);
static esp_err_t nvs_update_flag_clear(void);

void ota_task(void *pvParameter) {
  ESP_LOGI(TAG, "Starting OTA task");

  esp_http_client_config_t config = {
      .url = "http://192.168.1.200/www/ota/mqtt_point.bin",
      .cert_pem = (char *)server_cert_pem_start,
      .event_handler = _http_event_handler,
      .keep_alive_enable = true,
  };

  // TEST ONLY
  config.skip_cert_common_name_check = true;

  // esp_wifi_set_ps(WIFI_PS_NONE);

  esp_err_t ret = esp_https_ota(&config);
  if (ret == ESP_OK) {
    ESP_LOGI(TAG, "firmware updated successfully; rebooting");

    ESP_ERROR_CHECK(nvs_update_flag_clear());

    esp_restart();
  } else {
    ESP_LOGE(TAG, "Firmware upgrade failed");
  }
  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void perform_ota_update(void) {
  // it is not possible to write over running partition, therefore
  // if current partition is not factory, reboot to factory
  if (esp_ota_get_running_partition()->subtype !=
      ESP_PARTITION_SUBTYPE_APP_FACTORY) {
    ESP_LOGI(TAG, "rebooting to factory");

    ESP_ERROR_CHECK(nvs_update_flag_set());

    esp_partition_iterator_t partition_iterator = esp_partition_find(
        ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL);

    esp_partition_t *partition_factory = esp_partition_get(partition_iterator);

    esp_ota_set_boot_partition(partition_factory);

    esp_restart();
    while (1) {
    }
  }

  // running partition should now be factory, run ota task
  // idk why espressif chose a task instead of a simple function
  // i might change this later
  xTaskCreate(&ota_task, "ota_task", 8192, NULL, 5, NULL);
}

void get_sha256_of_partitions(void) {
  uint8_t sha_256[HASH_LEN] = {0};
  esp_partition_t partition;

  // get sha256 digest for bootloader
  partition.address = ESP_BOOTLOADER_OFFSET;
  partition.size = ESP_PARTITION_TABLE_OFFSET;
  partition.type = ESP_PARTITION_TYPE_APP;
  esp_partition_get_sha256(&partition, sha_256);
  print_sha256(sha_256, "SHA-256 for bootloader: ");

  // get sha256 digest for running partition
  esp_partition_get_sha256(esp_ota_get_running_partition(), sha_256);
  print_sha256(sha_256, "SHA-256 for current firmware: ");
}

void print_running_partition(void) {
  ESP_LOGI(TAG, "running partition:\nlabel:\t%s\nsubtype:\t%d\naddress:\t%x",
           esp_ota_get_running_partition()->label,
           esp_ota_get_running_partition()->subtype,
           esp_ota_get_running_partition()->address);
}

static esp_err_t nvs_update_flag_set(void) {
  nvs_handle_t nvs_handle;

  // write update flag to nvs
  esp_err_t err = nvs_open("update", NVS_READWRITE, &nvs_handle);

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "error (%s) opening NVS handle", esp_err_to_name(err));
    return err;
  } else {
    err = nvs_set_u8(nvs_handle, "update_flag", 1);

    if (err != ESP_OK) {
      ESP_LOGE(TAG, "error (%s) writing value to nvs", esp_err_to_name(err));
      return err;
    }

    err = nvs_commit(nvs_handle);

    if (err != ESP_OK) {
      ESP_LOGE(TAG, "error (%s) committing to nvs", esp_err_to_name(err));
      return err;
    }

    nvs_close(nvs_handle);
  }

  return ESP_OK;
}

static esp_err_t nvs_update_flag_clear(void) {
  nvs_handle_t nvs_handle;

  // write update flag to nvs
  esp_err_t err = nvs_open("update", NVS_READWRITE, &nvs_handle);

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "error (%s) opening NVS handle", esp_err_to_name(err));
    return err;
  } else {
    err = nvs_set_u8(nvs_handle, "update_flag", 0);

    if (err != ESP_OK) {
      ESP_LOGE(TAG, "error (%s) writing value to nvs", esp_err_to_name(err));
      return err;
    }

    err = nvs_commit(nvs_handle);

    if (err != ESP_OK) {
      ESP_LOGE(TAG, "error (%s) committing to nvs", esp_err_to_name(err));
      return err;
    }

    nvs_close(nvs_handle);
  }

  return ESP_OK;
}

static void print_sha256(const uint8_t *image_hash, const char *label) {
  char hash_print[HASH_LEN * 2 + 1];
  hash_print[HASH_LEN * 2] = 0;
  for (int i = 0; i < HASH_LEN; ++i) {
    sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
  }
  ESP_LOGI(TAG, "%s %s", label, hash_print);
}

static esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
  switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
      ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
      break;
    case HTTP_EVENT_ON_CONNECTED:
      ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
      break;
    case HTTP_EVENT_HEADER_SENT:
      ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
      break;
    case HTTP_EVENT_ON_HEADER:
      ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key,
               evt->header_value);
      break;
    case HTTP_EVENT_ON_DATA:
      ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
      break;
    case HTTP_EVENT_ON_FINISH:
      ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
      break;
    case HTTP_EVENT_DISCONNECTED:
      ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
      break;
  }
  return ESP_OK;
}