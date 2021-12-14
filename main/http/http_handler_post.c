
#include "http_handlers.h"

static const char TAG[] = "point_http";

esp_err_t point_post_handler(httpd_req_t* req) {
  ESP_LOGI(TAG, "POST %s", req->uri);
  static const uint8_t MAX_CODIGO_ID_SIZE = 32;

  if (strcmp(req->uri, "/vincular.json") == 0) {
    size_t codigo_id_len = httpd_req_get_hdr_value_len(req, "X-Codigo-Id");

    if (codigo_id_len && codigo_id_len <= MAX_CODIGO_ID_SIZE) {
      char* codigo_id = malloc(sizeof(char) * (codigo_id_len + 1));
      httpd_req_get_hdr_value_str(req, "X-Codigo-Id", codigo_id,
                                  codigo_id_len + 1);

      // aca hago algo con codigo_id
      ESP_LOGI(TAG, "codigo_id: %s", codigo_id);

      free(codigo_id);

      httpd_resp_set_status(req, "200 OK");
      httpd_resp_set_type(req, "application/json");
      httpd_resp_set_hdr(req, "Cache-Control",
                         "no-store, no-cache, must-revalidate, max-age=0");
      httpd_resp_set_hdr(req, "Pragma", "no-cache");
      httpd_resp_send(req, NULL, 0);
    } else {
      httpd_resp_set_status(req, "400 Bad Request");
      httpd_resp_send(req, NULL, 0);
    }
  } else {
    httpd_resp_send_404(req);
  }

  return ESP_OK;
}