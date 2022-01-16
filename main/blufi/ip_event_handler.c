#include "point_blufi.h"

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

bool gl_sta_connected = false;
bool ble_is_connected = false;
uint8_t gl_sta_bssid[6];
uint8_t gl_sta_ssid[32];
int gl_sta_ssid_len;

void ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
                      void *event_data) {
  wifi_mode_t mode;

  switch (event_id) {
    case IP_EVENT_STA_GOT_IP: {
      esp_blufi_extra_info_t info;

      xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
      esp_wifi_get_mode(&mode);

      memset(&info, 0, sizeof(esp_blufi_extra_info_t));
      memcpy(info.sta_bssid, gl_sta_bssid, 6);
      info.sta_bssid_set = true;
      info.sta_ssid = gl_sta_ssid;
      info.sta_ssid_len = gl_sta_ssid_len;
      if (ble_is_connected == true) {
        esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_SUCCESS, 0,
                                        &info);
      } else {
        BLUFI_INFO("BLUFI BLE is not connected yet\n");
      }
      break;
    }
    default:
      break;
  }
  return;
}