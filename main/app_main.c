/* MQTT over SSL Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "app_main.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "driver/gpio.h"
#include "esp32/rom/ets_sys.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "hal/gpio_types.h"
#include "mqtt_client.h"
#include "mqttpoint/mqttpoint.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "ota/http_ota.h"
#include "sdkconfig.h"
#include "sensors/sensors.h"
#include "wifi_prov_mgr/wifi_prov_mgr.h"
#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_ble.h"

static const char *TAG = "MQTT_POINT";

/* Signal Wi-Fi events on this event-group */
const int WIFI_CONNECTED_EVENT = BIT0;
EventGroupHandle_t wifi_event_group;

#define PROV_TRANSPORT_BLE "ble"

QueueHandle_t xMQTTDHTQueue;

void app_main(void) {
  ESP_LOGI(TAG, "[APP] Startup..");
  ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
  ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
  esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
  esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
  esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

  /* Initialize NVS partition */
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    /* NVS partition was truncated
     * and needs to be erased */
    ESP_ERROR_CHECK(nvs_flash_erase());

    /* Retry nvs_flash_init */
    ESP_ERROR_CHECK(nvs_flash_init());
  }

  get_sha256_of_partitions();

  /* Initialize TCP/IP */
  ESP_ERROR_CHECK(esp_netif_init());

  /* Initialize the event loop */
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  wifi_event_group = xEventGroupCreate();

  /* Register our event handler for Wi-Fi, IP and Provisioning related events
   */
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID,
                                             &event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                             &event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                             &event_handler, NULL));

  /* Initialize Wi-Fi including netif with default config */
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  /* Configuration for the provisioning manager */
  wifi_prov_mgr_config_t config = {
      /* What is the Provisioning Scheme that we want ?
       * wifi_prov_scheme_softap or wifi_prov_scheme_ble */
      .scheme = wifi_prov_scheme_ble,

      /* Any default scheme specific event handler that you would
       * like to choose. Since our example application requires
       * neither BT nor BLE, we can choose to release the associated
       * memory once provisioning is complete, or not needed
       * (in case when device is already provisioned). Choosing
       * appropriate scheme specific event handler allows the manager
       * to take care of this automatically. This can be set to
       * WIFI_PROV_EVENT_HANDLER_NONE when using wifi_prov_scheme_softap*/
      .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM,
  };

  /* Initialize provisioning manager with the
   * configuration parameters set above */
  ESP_ERROR_CHECK(wifi_prov_mgr_init(config));

  bool provisioned = false;
  /* Let's find out if the device is provisioned */
  ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));

  /* If device is not yet provisioned start provisioning service */
  if (!provisioned) {
    ESP_LOGI(TAG, "Starting provisioning");

    /* What is the Device Service Name that we want
     * This translates to :
     *     - Wi-Fi SSID when scheme is wifi_prov_scheme_softap
     *     - device name when scheme is wifi_prov_scheme_ble
     */
    char service_name[12];
    get_device_service_name(service_name, sizeof(service_name));

    /* What is the security level that we want (0 or 1):
     *      - WIFI_PROV_SECURITY_0 is simply plain text communication.
     *      - WIFI_PROV_SECURITY_1 is secure communication which consists of
     * secure handshake using X25519 key exchange and proof of possession
     * (pop) and AES-CTR for encryption/decryption of messages.
     */
    wifi_prov_security_t security = WIFI_PROV_SECURITY_1;

    /* Do we want a proof-of-possession (ignored if Security 0 is selected):
     *      - this should be a string with length > 0
     *      - NULL if not used
     */
    const char *pop = "abcd1234";

    /* What is the service key (could be NULL)
     * This translates to :
     *     - Wi-Fi password when scheme is wifi_prov_scheme_softap
     *     - simply ignored when scheme is wifi_prov_scheme_ble
     */
    const char *service_key = NULL;

    /* This step is only useful when scheme is wifi_prov_scheme_ble. This will
     * set a custom 128 bit UUID which will be included in the BLE
     * advertisement and will correspond to the primary GATT service that
     * provides provisioning endpoints as GATT characteristics. Each GATT
     * characteristic will be formed using the primary service UUID as base,
     * with different auto assigned 12th and 13th bytes (assume counting
     * starts from 0th byte). The client side applications must identify the
     * endpoints by reading the User Characteristic Description descriptor
     * (0x2901) for each characteristic, which contains the endpoint name of
     * the characteristic */
    uint8_t custom_service_uuid[] = {
        /* LSB <---------------------------------------
         * ---------------------------------------> MSB */
        0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf,
        0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02,
    };
    wifi_prov_scheme_ble_set_service_uuid(custom_service_uuid);

    /* An optional endpoint that applications can create if they expect to
     * get some additional custom data during provisioning workflow.
     * The endpoint name can be anything of your choice.
     * This call must be made before starting the provisioning.
     */
    wifi_prov_mgr_endpoint_create("custom-data");

    // disable auto stop to config the device
    wifi_prov_mgr_disable_auto_stop(1000);

    /* Start provisioning service */
    ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(
        security, pop, service_name, service_key));

    /* The handler for the optional endpoint created above.
     * This call must be made after starting the provisioning, and only if the
     * endpoint has already been created above.
     */
    wifi_prov_mgr_endpoint_register("custom-data", custom_prov_data_handler,
                                    NULL);

    /* Uncomment the following to wait for the provisioning to finish and then
     * release the resources of the manager. Since in this case
     * de-initialization is triggered by the default event loop handler, we
     * don't need to call the following */
    // wifi_prov_mgr_wait();
    // wifi_prov_mgr_deinit();
    /* Print QR code for provisioning */
    wifi_prov_print_qr(service_name, pop, PROV_TRANSPORT_BLE);
  } else {
    ESP_LOGI(TAG, "Already provisioned, starting Wi-Fi STA");

    /* We don't need the manager as device is already provisioned,
     * so let's release it's resources */
    wifi_prov_mgr_deinit();

    /* Start Wi-Fi station */
    wifi_init_sta();
  }

  /* Wait for Wi-Fi connection */
  xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_EVENT, false, true,
                      portMAX_DELAY);

  /* Start main application now */

  // ota
  // using a single ota partition,
  // therefore we need to reboot to factory before updating.
  // when a new update is available a flag is written to nvs so the
  // update-available state is remembered after rebooting
  print_running_partition();

  gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
  gpio_set_pull_mode(GPIO_NUM_0, GPIO_PULLUP_ONLY);

  bool update_available = 0;

  nvs_handle_t nvs_handle;

  // read update flag from nvs
  err = nvs_open("update", NVS_READWRITE, &nvs_handle);

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "error (%s) opening NVS handle", esp_err_to_name(err));
  } else {
    err = nvs_get_u8(nvs_handle, "update_flag", &update_available);

    if (err == ESP_ERR_NVS_NOT_FOUND) {
      ESP_LOGI(TAG, "update_flag not found. initiializing");
      ESP_ERROR_CHECK(nvs_set_u8(nvs_handle, "update_flag", 0));
    } else if (err != ESP_OK) {
      ESP_LOGE(TAG, "error (%s) reading value from nvs", esp_err_to_name(err));
    }

    nvs_close(nvs_handle);
  }

  if (update_available) {
    ESP_LOGI(TAG, "update available; performing ota");
    perform_ota_update();
  }

  gpio_set_direction(2, GPIO_MODE_INPUT_OUTPUT);

  xMQTTDHTQueue = xQueueCreate(3, sizeof(struct sensor_msg));

  xTaskCreate(&DHT_task, "DHT_task", 2048, NULL, 5, NULL);
  // xTaskCreate(&DHT_test0, "DHT_test0", 2048, NULL, 5, NULL);
  // xTaskCreate(&DHT_test1, "DHT_test1", 2048, NULL, 5, NULL);
  xTaskCreate(&mqtt_app_start, "mqtt_app_start", 4096, NULL, 5, NULL);
}