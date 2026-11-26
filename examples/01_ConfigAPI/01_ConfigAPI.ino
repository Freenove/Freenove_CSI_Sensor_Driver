/*
 * 01_ConfigAPI — detect, soft flip, JPEG encode.
 *
 * Per-sensor resolution is fixed (IDF / product default):
 *   IMX219 / IMX708 → 640×480
 *   OV5647         → 800×640
 * setFlip is RGB565 software flip (no sensor reload).
 *
 * Board: FNK0117 ESP32-P4 (MIPI-CSI). Tools → PSRAM = Enabled.
 */
#include <freenove_csi_sensor_driver.h>

Freenove_CSI_Sensor_Driver cam;

const size_t CAM_FRAMESIZE = Freenove_CSI_Sensor_Driver::FRAMESIZE_DEFAULT;
//const size_t CAM_FRAMESIZE = IMX219_FRAMESIZE_640x480;
//const size_t CAM_FRAMESIZE = IMX708_FRAMESIZE_640x480;
//const size_t CAM_FRAMESIZE = OV5647_FRAMESIZE_800x640;

const bool CAM_HMIRROR = false;
const bool CAM_VFLIP = false;

void printBanner(const char *title) {
  Serial.println();
  Serial.println("------------------------------------------------------------");
  Serial.println(title);
  Serial.println("------------------------------------------------------------");
}

void setup() {
  Serial.begin(115200);
  delay(500);

  printBanner("1) begin()");
  Serial.printf("  SCCB SDA=GPIO%d  SCL=GPIO%d\n",
                Freenove_CSI_Sensor_Driver::DEFAULT_SDA_PIN,
                Freenove_CSI_Sensor_Driver::DEFAULT_SCL_PIN);

  cam.setFrameSize(CAM_FRAMESIZE);
  cam.setFlip(CAM_HMIRROR, CAM_VFLIP);
  if (!cam.begin()) {
    Serial.println("Camera init failed.");
    return;
  }

  bool hmirror = false;
  bool vflip = false;
  (void)cam.getFlip(&hmirror, &vflip);
  Serial.printf("  %s %lux%lu  %s  flip=%d/%d\n",
                cam.sensorName(),
                (unsigned long)cam.width(), (unsigned long)cam.height(),
                cam.frameSizeName(),
                hmirror ? 1 : 0, vflip ? 1 : 0);

  printBanner("2) fixed framesize (read-only)");
  freenove_csi_framesize_dim_t dims[4];
  const size_t n = cam.listSupportedFrameSizes(dims, sizeof(dims) / sizeof(dims[0]));
  Serial.printf("  count=%u (one mode per sensor)\n", (unsigned)n);
  for (size_t i = 0; i < n; i++) {
    Serial.printf("  [%u] %ux%u  %s\n",
                  (unsigned)i, dims[i].width, dims[i].height,
                  freenove_csi_framesize_name(cam.sensorId(), i));
  }

  printBanner("3) setFlip / getFlip (soft, no reload)");
  if (cam.setFlip(!CAM_HMIRROR, !CAM_VFLIP)) {
    (void)cam.getFlip(&hmirror, &vflip);
    Serial.printf("  soft flip hmirror=%d vflip=%d\n", hmirror ? 1 : 0, vflip ? 1 : 0);
  }
  (void)cam.setFlip(CAM_HMIRROR, CAM_VFLIP);
  (void)cam.getFlip(&hmirror, &vflip);
  Serial.printf("  restored flip=%d/%d\n", hmirror ? 1 : 0, vflip ? 1 : 0);

  printBanner("Ready — JPEG every 3 s");
}

void loop() {
  if (!cam.isStarted()) {
    delay(1000);
    return;
  }

  size_t bufSize = 0;
  uint8_t *jpeg = cam.allocJpegBuf(cam.maxJpegSize(), &bufSize);
  if (jpeg == nullptr) {
    Serial.println("allocJpegBuf failed");
    delay(3000);
    return;
  }

  const int n = cam.encodeJpeg(false, jpeg, bufSize);
  if (n > 0) {
    bool hmirror = false;
    bool vflip = false;
    (void)cam.getFlip(&hmirror, &vflip);
    Serial.printf("%lux%lu  JPEG %d  %s  flip=%d/%d\n",
                  (unsigned long)cam.width(), (unsigned long)cam.height(),
                  n, cam.frameSizeName(),
                  hmirror ? 1 : 0, vflip ? 1 : 0);
  } else {
    Serial.println("encodeJpeg failed");
  }

  cam.freeJpegBuf(jpeg);
  delay(3000);
}

