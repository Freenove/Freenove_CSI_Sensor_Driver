#include "camera_web_server.h"
#include "camera_web_page.h"

#include "esp_http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace {

constexpr uint32_t kStreamIntervalMs = 66;

const char *kStreamContentType =
  "multipart/x-mixed-replace;boundary=114731404234096";
const char *kStreamBoundary = "\r\n--114731404234096\r\n";
const char *kStreamPartHeader =
  "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

Freenove_CSI_Sensor_Driver *g_cam = nullptr;
uint8_t *g_streamBuf = nullptr;
size_t g_streamBufSize = 0;
volatile bool g_streamBusy = false;
volatile bool g_streamCancel = false;

bool ensureStreamBuf() {
  if (g_cam == nullptr) {
    return false;
  }
  const uint32_t need = g_cam->maxJpegSize();
  if (g_streamBuf != nullptr && g_streamBufSize >= need) {
    return true;
  }
  if (g_streamBuf != nullptr) {
    g_cam->freeJpegBuf(g_streamBuf);
    g_streamBuf = nullptr;
    g_streamBufSize = 0;
  }
  g_streamBuf = g_cam->allocJpegBuf(need, &g_streamBufSize);
  return g_streamBuf != nullptr;
}

int queryInt(httpd_req_t *req, const char *key, int defVal) {
  char q[96];
  if (httpd_req_get_url_query_str(req, q, sizeof(q)) != ESP_OK) {
    return defVal;
  }
  char val[24];
  if (httpd_query_key_value(q, key, val, sizeof(val)) != ESP_OK) {
    return defVal;
  }
  return atoi(val);
}

esp_err_t sendJson(httpd_req_t *req, const char *json) {
  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "Cache-Control", "no-store");
  return httpd_resp_sendstr(req, json);
}

esp_err_t handleRoot(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  httpd_resp_set_hdr(req, "Cache-Control", "no-store");
  return httpd_resp_send(req, CAMERA_WEB_PAGE_HTML, HTTPD_RESP_USE_STRLEN);
}

esp_err_t handleStatus(httpd_req_t *req) {
  if (g_cam == nullptr) {
    return sendJson(req, "{\"ok\":false,\"err\":\"no cam\"}");
  }
  bool h = false;
  bool v = false;
  (void)g_cam->getFlip(&h, &v);
  char json[384];
  snprintf(json, sizeof(json),
           "{\"ok\":true,\"sensor\":\"%s\",\"sensorId\":%d,"
           "\"width\":%lu,\"height\":%lu,"
           "\"framesize\":%u,\"framesizeName\":\"%s\",\"framesizeCount\":%u,"
           "\"hmirror\":%s,\"vflip\":%s,"
           "\"streamQuality\":%u,\"photoQuality\":%u,\"started\":%s}",
           g_cam->sensorName(), (int)g_cam->sensorId(),
           (unsigned long)g_cam->width(), (unsigned long)g_cam->height(),
           (unsigned)g_cam->frameSize(), g_cam->frameSizeName(),
           (unsigned)g_cam->frameSizeCount(),
           h ? "true" : "false", v ? "true" : "false",
           (unsigned)g_cam->streamQuality(), (unsigned)g_cam->photoQuality(),
           g_cam->isStarted() ? "true" : "false");
  return sendJson(req, json);
}

esp_err_t handleApiFlip(httpd_req_t *req) {
  if (g_cam == nullptr) {
    return sendJson(req, "{\"ok\":false,\"err\":\"no cam\"}");
  }
  const int h = queryInt(req, "h", -1);
  const int v = queryInt(req, "v", -1);
  char json[192];
  if (h < 0 || v < 0) {
    snprintf(json, sizeof(json), "{\"ok\":false,\"err\":\"missing h or v\"}");
    return sendJson(req, json);
  }
  Serial.printf("[Web] Apply setFlip(%d,%d) → softFlip\n", h, v);
  const bool ok = g_cam->setFlip(h != 0, v != 0);
  bool gh = false;
  bool gv = false;
  (void)g_cam->getFlip(&gh, &gv);
  snprintf(json, sizeof(json),
           "{\"ok\":%s,\"softFlip\":true,\"hmirror\":%s,\"vflip\":%s}",
           ok ? "true" : "false",
           gh ? "true" : "false", gv ? "true" : "false");
  return sendJson(req, json);
}

esp_err_t handleJpeg(httpd_req_t *req) {
  if (g_cam == nullptr) {
    httpd_resp_set_status(req, "503 Service Unavailable");
    return httpd_resp_sendstr(req, "no cam");
  }
  size_t bufSize = 0;
  uint8_t *buf = g_cam->allocJpegBuf(g_cam->maxJpegSize(), &bufSize);
  if (buf == nullptr) {
    httpd_resp_set_status(req, "503 Service Unavailable");
    return httpd_resp_sendstr(req, "no memory");
  }
  const int size = g_cam->encodeJpeg(true, buf, bufSize);
  esp_err_t err;
  if (size <= 0) {
    httpd_resp_set_status(req, "503 Service Unavailable");
    err = httpd_resp_sendstr(req, "capture failed");
  } else {
    httpd_resp_set_type(req, "image/jpeg");
    err = httpd_resp_send(req, (const char *)buf, size);
  }
  g_cam->freeJpegBuf(buf);
  return err;
}

void streamTask(void *arg) {
  httpd_req_t *req = (httpd_req_t *)arg;
  uint32_t nextFrameAt = millis();
  while (!g_streamCancel) {
    const int32_t waitMs = (int32_t)(nextFrameAt - millis());
    if (waitMs > 0) {
      delay(waitMs);
    }
    if (g_streamCancel) {
      break;
    }
    if (g_cam == nullptr || g_streamBuf == nullptr || g_streamBufSize == 0) {
      nextFrameAt = millis() + kStreamIntervalMs;
      continue;
    }
    const int size = g_cam->encodeJpeg(false, g_streamBuf, g_streamBufSize);
    if (size <= 0) {
      nextFrameAt = millis() + kStreamIntervalMs;
      continue;
    }
    if (httpd_resp_send_chunk(req, kStreamBoundary, strlen(kStreamBoundary)) != ESP_OK) {
      break;
    }
    char partHeader[96];
    const int headerLen =
      snprintf(partHeader, sizeof(partHeader), kStreamPartHeader, (unsigned)size);
    if (headerLen > 0 &&
        httpd_resp_send_chunk(req, partHeader, headerLen) != ESP_OK) {
      break;
    }
    if (httpd_resp_send_chunk(req, (const char *)g_streamBuf, size) != ESP_OK) {
      break;
    }
    nextFrameAt += kStreamIntervalMs;
    if ((int32_t)(millis() - nextFrameAt) > 0) {
      nextFrameAt = millis();
    }
  }
  Serial.println("stream client disconnected");
  g_streamBusy = false;
  g_streamCancel = false;
  httpd_req_async_handler_complete(req);
  vTaskDelete(nullptr);
}

esp_err_t handleStream(httpd_req_t *req) {
  if (g_streamBusy) {
    g_streamCancel = true;
    for (int i = 0; i < 40 && g_streamBusy; i++) {
      delay(50);
    }
  }
  if (g_streamBusy) {
    httpd_resp_set_status(req, "503 Service Unavailable");
    return httpd_resp_sendstr(req, "another client is already watching\n");
  }
  g_streamCancel = false;
  if (!ensureStreamBuf()) {
    httpd_resp_set_status(req, "503 Service Unavailable");
    return httpd_resp_sendstr(req, "no stream buffer\n");
  }
  httpd_req_t *asyncReq = nullptr;
  esp_err_t err = httpd_req_async_handler_begin(req, &asyncReq);
  if (err != ESP_OK) {
    return err;
  }
  err = httpd_resp_set_type(asyncReq, kStreamContentType);
  if (err == ESP_OK) {
    err = httpd_resp_set_hdr(asyncReq, "Access-Control-Allow-Origin", "*");
  }
  if (err == ESP_OK) {
    g_streamBusy = true;
    if (xTaskCreate(streamTask, "mjpeg_stream", 6144, asyncReq, 5, nullptr) == pdPASS) {
      return ESP_OK;
    }
    g_streamBusy = false;
    err = ESP_ERR_NO_MEM;
  }
  httpd_req_async_handler_complete(asyncReq);
  return err;
}

}  // namespace

bool camera_web_server_start(Freenove_CSI_Sensor_Driver &cam) {
  g_cam = &cam;
  if (!ensureStreamBuf()) {
    Serial.println("failed to allocate JPEG stream buffer");
    return false;
  }

  httpd_handle_t server = nullptr;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  config.ctrl_port = 32768;
  config.stack_size = 8192;
  config.max_uri_handlers = 12;
  config.recv_wait_timeout = 60;
  config.send_wait_timeout = 60;
  if (httpd_start(&server, &config) != ESP_OK) {
    Serial.println("failed to start HTTP server");
    return false;
  }

  const httpd_uri_t uris[] = {
    {.uri = "/", .method = HTTP_GET, .handler = handleRoot, .user_ctx = nullptr},
    {.uri = "/stream", .method = HTTP_GET, .handler = handleStream, .user_ctx = nullptr},
    {.uri = "/jpg", .method = HTTP_GET, .handler = handleJpeg, .user_ctx = nullptr},
    {.uri = "/status", .method = HTTP_GET, .handler = handleStatus, .user_ctx = nullptr},
    {.uri = "/api/flip", .method = HTTP_GET, .handler = handleApiFlip, .user_ctx = nullptr},
  };
  for (size_t i = 0; i < sizeof(uris) / sizeof(uris[0]); i++) {
    httpd_register_uri_handler(server, &uris[i]);
  }

  Serial.println("web camera server on port 80");
  return true;
}
