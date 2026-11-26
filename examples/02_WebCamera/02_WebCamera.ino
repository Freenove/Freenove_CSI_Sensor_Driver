/*
 * 02_WebCamera — Wi-Fi MJPEG + soft flip panel.
 *
 * Modules:
 *   camera_web_page.h     — index HTML
 *   camera_web_server.*   — httpd routes (/ /stream /jpg /status /api/flip)
 *
 * Board: FNK0117 ESP32-P4, PSRAM Enabled. SCCB SDA=7 SCL=8.
 */
#include <Arduino.h>
#include <WiFi.h>
#include <freenove_csi_sensor_driver.h>
#include "camera_web_server.h"

const char *WIFI_SSID = "FYI_2.4G";
const char *WIFI_PASSWORD = "freenove208";
const uint32_t WIFI_TIMEOUT_MS = 30000;

const size_t CAM_FRAMESIZE = Freenove_CSI_Sensor_Driver::FRAMESIZE_DEFAULT;
//const size_t CAM_FRAMESIZE = IMX219_FRAMESIZE_640x480;
//const size_t CAM_FRAMESIZE = IMX708_FRAMESIZE_640x480;
//const size_t CAM_FRAMESIZE = OV5647_FRAMESIZE_800x640;

const bool CAM_HMIRROR = false;
const bool CAM_VFLIP = false;

Freenove_CSI_Sensor_Driver cam;

static bool connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > WIFI_TIMEOUT_MS) {
      Serial.printf("Wi-Fi timeout, status %d\n", WiFi.status());
      return false;
    }
    delay(500);
    Serial.print('.');
  }
  Serial.printf("\nConnected  http://%s/\n", WiFi.localIP().toString().c_str());
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(300);

  cam.setFrameSize(CAM_FRAMESIZE);
  cam.setFlip(CAM_HMIRROR, CAM_VFLIP);
  if (!cam.begin()) {
    Serial.println("camera init failed");
    return;
  }
  Serial.printf("camera %s %lux%lu fs[%u]=%s\n",
                cam.sensorName(),
                (unsigned long)cam.width(), (unsigned long)cam.height(),
                (unsigned)cam.frameSize(), cam.frameSizeName());

  if (!connectWifi()) {
    return;
  }
  if (!camera_web_server_start(cam)) {
    Serial.println("web server start failed");
  }
}

void loop() {
  delay(1000);
}

