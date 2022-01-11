#include <esp_http_client.h>

#include "cJSON.h"
#include "esp_tls.h"
#include "http_handlers.h"

#define MAX_HTTP_OUTPUT_BUFFER 2048

static const char TAG[] = "point_http";
// should be allocated dynamically. for now this works
static char response_content[128];

esp_err_t http_event_handler(esp_http_client_event_t* evt);

esp_err_t point_post_handler(httpd_req_t* req) {
  ESP_LOGI(TAG, "POST %s", req->uri);
  static const uint8_t MAX_CODIGO_ID_SIZE = 32;

  if (strcmp(req->uri, "/vincular.json") == 0) {
    size_t codigo_id_len = httpd_req_get_hdr_value_len(req, "X-Codigo-Id");

    if (codigo_id_len && codigo_id_len <= MAX_CODIGO_ID_SIZE) {
      char codigo_id[64];
      httpd_req_get_hdr_value_str(req, "X-Codigo-Id", codigo_id,
                                  codigo_id_len + 1);
      ESP_LOGI(TAG, "codigo_id: %s", codigo_id);

      char str_mac[13] = {0};

      esp_err_t err;
      uint8_t mac[6];
      err = esp_efuse_mac_get_default(mac);
      if (err == ESP_OK) {
        sprintf(str_mac, "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2],
                mac[3], mac[4], mac[5]);
        ESP_LOGI(TAG, "%s", str_mac);
      } else {
        ESP_LOGE(TAG, "esp_efuse_mac: %s\n", esp_err_to_name(err));
      }

      char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};

      esp_http_client_config_t config = {
          .url =
              //"http://181.45.14.233:1234/api/maquina/asignarMaquinaConCodigo",
          "http://192.168.1.200:41062/www/api/maquina/asignarMaquinaConCodigo",
          //"http://httpbin.org/post",
          .method = HTTP_METHOD_POST,
          .event_handler = http_event_handler,
          .user_data = local_response_buffer,
          .disable_auto_redirect = true,
      };

      esp_http_client_handle_t client = esp_http_client_init(&config);

      // id .json
      cJSON* json_id = cJSON_CreateObject();
      cJSON_AddStringToObject(json_id, "codigoUsuario", codigo_id);
      cJSON_AddStringToObject(json_id, "nombreMaquina", str_mac);
      const char* post_data = cJSON_Print(json_id);
      cJSON_Delete(json_id);

      ESP_LOGI(TAG, "HTTP POST data: %s", post_data);

      // POST
      esp_http_client_set_header(client, "Content-Type", "application/json");
      esp_http_client_set_post_field(client, post_data, strlen(post_data));
      err = esp_http_client_perform(client);

      if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %d",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
      } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
      }

      esp_http_client_cleanup(client);

      httpd_resp_set_status(req, "200 OK");
      httpd_resp_set_type(req, "application/json");
      httpd_resp_set_hdr(req, "Cache-Control",
                         "no-store, no-cache, must-revalidate, max-age=0");
      httpd_resp_set_hdr(req, "Pragma", "no-cache");
      // response_content may be empty if the server sent no response
      err = httpd_resp_send(req, response_content, HTTPD_RESP_USE_STRLEN);
      ESP_LOGI(TAG, "HTTPD_RESP_SEND: %s", esp_err_to_name(err));
    } else {
      httpd_resp_set_status(req, "400 Bad Request");
      httpd_resp_send(req, NULL, 0);
    }
  } else {
    httpd_resp_send_404(req);
  }

  return ESP_OK;
}

esp_err_t http_event_handler(esp_http_client_event_t* evt) {
  static char* output_buffer;
  static int output_len;
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
      ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
      /*
       *  Check for chunked encoding is added as the URL for chunked encoding
       * used in this example returns binary data. However, event handler can
       * also be used in case chunked encoding is used.
       */
      if (!esp_http_client_is_chunked_response(evt->client)) {
        // If user_data buffer is configured, copy the response into the buffer
        if (evt->user_data) {
          memcpy(evt->user_data + output_len, evt->data, evt->data_len);
        } else {
          if (output_buffer == NULL) {
            output_buffer =
                (char*)malloc(esp_http_client_get_content_length(evt->client));
            output_len = 0;
            if (output_buffer == NULL) {
              ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
              return ESP_FAIL;
            }
          }
          memcpy(output_buffer + output_len, evt->data, evt->data_len);
        }
        output_len += evt->data_len;
      }
      ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, content=%s", (char*)evt->user_data);
      strcpy(response_content, (char*)evt->user_data);

      break;
    case HTTP_EVENT_ON_FINISH:
      ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
      if (output_buffer != NULL) {
        // Response is accumulated in output_buffer. Uncomment the below line to
        // print the accumulated response ESP_LOG_BUFFER_HEX(TAG, output_buffer,
        // output_len);
        free(output_buffer);
        output_buffer = NULL;
      }
      output_len = 0;
      break;
    case HTTP_EVENT_DISCONNECTED:
      ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
      int mbedtls_err = 0;
      esp_err_t err =
          esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
      if (err != 0) {
        if (output_buffer != NULL) {
          free(output_buffer);
          output_buffer = NULL;
        }
        output_len = 0;
        ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
        ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
      }
      break;
  }
  return ESP_OK;
}
