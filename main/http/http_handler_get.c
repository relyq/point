#include "http_handlers.h"

static const char TAG[] = "point_http";

esp_err_t point_get_handler(httpd_req_t* req) {
  ESP_LOGI(TAG, "GET %s", req->uri);

  if (strcmp(req->uri, "/vincular") == 0) {
    ESP_LOGI(TAG, "serving page vincular");

    httpd_resp_set_status(req, "200 OK");
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (char*)vincular_index_html_start,
                    vincular_index_html_end - vincular_index_html_start);
  } else if (strcmp(req->uri, "/vincular_style.css") == 0) {
    ESP_LOGI(TAG, "serving page vincular style");

    httpd_resp_set_status(req, "200 OK");
    httpd_resp_set_type(req, "text/css");
    httpd_resp_set_hdr(req, "Cache-Control", "public, max-age=31536000");
    httpd_resp_send(req, (char*)vincular_style_css_start,
                    vincular_style_css_end - vincular_style_css_start);
  } else if (strcmp(req->uri, "/postearjaja.js") == 0) {
    ESP_LOGI(TAG, "serving postearjaja.js");

    httpd_resp_set_status(req, "200 OK");
    httpd_resp_set_type(req, "text/javascript");
    httpd_resp_send(req, (char*)postearjaja_js_start,
                    postearjaja_js_end - 1 - postearjaja_js_start);
  } else {
    httpd_resp_send_404(req);
  }

  return ESP_OK;
}