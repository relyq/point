#include "point_blufi.h"

void wifi_event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data) {
  wifi_event_sta_connected_t *event;
  wifi_mode_t mode;

  switch (event_id) {
    case WIFI_EVENT_STA_START: {
      esp_wifi_connect();
      break;
    }
    case WIFI_EVENT_STA_CONNECTED: {
      gl_sta_connected = true;
      event = (wifi_event_sta_connected_t *)event_data;
      memcpy(gl_sta_bssid, event->bssid, 6);
      memcpy(gl_sta_ssid, event->ssid, event->ssid_len);
      gl_sta_ssid_len = event->ssid_len;
      break;
    }
    case WIFI_EVENT_STA_DISCONNECTED: {
      /* This is a workaround as ESP32 WiFi libs don't currently
         auto-reassociate. */
      gl_sta_connected = false;
      memset(gl_sta_ssid, 0, 32);
      memset(gl_sta_bssid, 0, 6);
      gl_sta_ssid_len = 0;
      esp_wifi_connect();
      xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
      break;
    }
    case WIFI_EVENT_AP_START: {
      esp_wifi_get_mode(&mode);

      /* TODO: get config or information of softap, then set to report
       * extra_info */
      if (ble_is_connected == true) {
        if (gl_sta_connected) {
          esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_SUCCESS, 0,
                                          NULL);
        } else {
          esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_FAIL, 0,
                                          NULL);
        }
      } else {
        BLUFI_INFO("BLUFI BLE is not connected yet\n");
      }
      break;
    }
    case WIFI_EVENT_SCAN_DONE: {
      uint16_t apCount = 0;
      esp_wifi_scan_get_ap_num(&apCount);
      if (apCount == 0) {
        BLUFI_INFO("Nothing AP found");
        break;
      }
      wifi_ap_record_t *ap_list =
          (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount);
      if (!ap_list) {
        BLUFI_ERROR("malloc error, ap_list is NULL");
        break;
      }
      ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, ap_list));
      esp_blufi_ap_record_t *blufi_ap_list = (esp_blufi_ap_record_t *)malloc(
          apCount * sizeof(esp_blufi_ap_record_t));
      if (!blufi_ap_list) {
        if (ap_list) {
          free(ap_list);
        }
        BLUFI_ERROR("malloc error, blufi_ap_list is NULL");
        break;
      }
      for (int i = 0; i < apCount; ++i) {
        blufi_ap_list[i].rssi = ap_list[i].rssi;
        memcpy(blufi_ap_list[i].ssid, ap_list[i].ssid, sizeof(ap_list[i].ssid));
      }

      if (ble_is_connected == true) {
        esp_blufi_send_wifi_list(apCount, blufi_ap_list);
      } else {
        BLUFI_INFO("BLUFI BLE is not connected yet\n");
      }

      esp_wifi_scan_stop();
      free(ap_list);
      free(blufi_ap_list);
      break;
    }
    default:
      break;
  }
  return;
}