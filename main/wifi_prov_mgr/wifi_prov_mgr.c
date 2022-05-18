#include "wifi_prov_mgr.h"

#include <esp_wifi.h>
#include <freertos/event_groups.h>
#include <wifi_provisioning/manager.h>

#include "qrcode.h"

static const char *TAG = "WIFI_PROV_MGR";

#define CONFIG_EXAMPLE_RESET_PROV_MGR_ON_FAILURE 1
#define CONFIG_EXAMPLE_PROV_MGR_MAX_RETRY_CNT 5
#define QRCODE_BASE_URL "https://espressif.github.io/esp-jumpstart/qrcode.html"
#define CONFIG_EXAMPLE_PROV_SHOW_QR 1
#define PROV_QR_VERSION "v1"

extern EventGroupHandle_t wifi_event_group;
extern const int WIFI_CONNECTED_EVENT;

/* Event handler for catching system events */
void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
                   void *event_data) {
#ifdef CONFIG_EXAMPLE_RESET_PROV_MGR_ON_FAILURE
  static int retries;
#endif
  if (event_base == WIFI_PROV_EVENT) {
    switch (event_id) {
      case WIFI_PROV_START:
        ESP_LOGI(TAG, "Provisioning started");
        break;
      case WIFI_PROV_CRED_RECV: {
        wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
        ESP_LOGI(TAG,
                 "Received Wi-Fi credentials"
                 "\n\tSSID     : %s\n\tPassword : %s",
                 (const char *)wifi_sta_cfg->ssid,
                 (const char *)wifi_sta_cfg->password);
        break;
      }
      case WIFI_PROV_CRED_FAIL: {
        wifi_prov_sta_fail_reason_t *reason =
            (wifi_prov_sta_fail_reason_t *)event_data;
        ESP_LOGE(TAG,
                 "Provisioning failed!\n\tReason : %s"
                 "\n\tPlease reset to factory and retry provisioning",
                 (*reason == WIFI_PROV_STA_AUTH_ERROR)
                     ? "Wi-Fi station authentication failed"
                     : "Wi-Fi access-point not found");
#ifdef CONFIG_EXAMPLE_RESET_PROV_MGR_ON_FAILURE
        retries++;
        if (retries >= CONFIG_EXAMPLE_PROV_MGR_MAX_RETRY_CNT) {
          ESP_LOGI(TAG,
                   "Failed to connect with provisioned AP, reseting "
                   "provisioned credentials");
          wifi_prov_mgr_reset_sm_state_on_failure();
          retries = 0;
        }
#endif
        break;
      }
      case WIFI_PROV_CRED_SUCCESS:
        ESP_LOGI(TAG, "Provisioning successful");
#ifdef CONFIG_EXAMPLE_RESET_PROV_MGR_ON_FAILURE
        retries = 0;
#endif
        break;
      case WIFI_PROV_END:
        /* De-initialize manager once provisioning is finished */
        wifi_prov_mgr_deinit();
        break;
      default:
        break;
    }
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "Connected with IP Address:" IPSTR,
             IP2STR(&event->ip_info.ip));
    /* Signal main application to continue execution */
    xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_EVENT);
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    ESP_LOGI(TAG, "Disconnected. Connecting to the AP again...");
    esp_wifi_connect();
  }
}

void wifi_init_sta(void) {
  /* Start Wi-Fi in station mode */
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());
}

void get_device_service_name(char *service_name, size_t max) {
  uint8_t eth_mac[6];
  const char *ssid_prefix = "PROV_";
  esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
  snprintf(service_name, max, "%s%02X%02X%02X%02X%02X%02X", ssid_prefix,
           eth_mac[0], eth_mac[1], eth_mac[2], eth_mac[3], eth_mac[4],
           eth_mac[5]);
}

void wifi_prov_print_qr(const char *name, const char *pop,
                        const char *transport) {
  if (!name || !transport) {
    ESP_LOGW(TAG, "Cannot generate QR code payload. Data missing.");
    return;
  }
  char payload[150] = {0};
  if (pop) {
    snprintf(payload, sizeof(payload),
             "{\"ver\":\"%s\",\"name\":\"%s\""
             ",\"pop\":\"%s\",\"transport\":\"%s\"}",
             PROV_QR_VERSION, name, pop, transport);
  } else {
    snprintf(payload, sizeof(payload),
             "{\"ver\":\"%s\",\"name\":\"%s\""
             ",\"transport\":\"%s\"}",
             PROV_QR_VERSION, name, transport);
  }
#ifdef CONFIG_EXAMPLE_PROV_SHOW_QR
  ESP_LOGI(
      TAG,
      "Scan this QR code from the provisioning application for Provisioning.");
  esp_qrcode_config_t cfg = ESP_QRCODE_CONFIG_DEFAULT();
  esp_qrcode_generate(&cfg, payload);
#endif /* CONFIG_APP_WIFI_PROV_SHOW_QR */
  ESP_LOGI(TAG,
           "If QR code is not visible, copy paste the below URL in a "
           "browser.\n%s?data=%s",
           QRCODE_BASE_URL, payload);
}
