/*
 * Freenove_CSI_Sensor_Driver - Arduino wrapper around esp_cam_sensor + ESP_Video (MIPI-CSI).
 *
 * Auto-detects OV5647 / IMX219 / IMX708.
 * Framesize (one mode each): IMX219/708 → 640x480; OV5647 → 800x640.
 */
#pragma once

#include <Arduino.h>
#include <ESP_Video.h>
#include "driver/jpeg_encode.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freenove_csi_framesize.h"
#include "sensors/imx219/imx219_framesize.h"
#include "sensors/imx708/imx708_framesize.h"
#include "sensors/ov5647/ov5647_framesize.h"
#include "freenove_csi_sensor_ops.h"

class Freenove_CSI_Sensor_Driver {
public:
  static constexpr int8_t DEFAULT_SDA_PIN = 7;
  static constexpr int8_t DEFAULT_SCL_PIN = 8;
  static constexpr int8_t DEFAULT_I2C_PORT = 0;
  static constexpr uint32_t DEFAULT_I2C_FREQ_HZ = 100000;
  static constexpr size_t DEFAULT_BUF_COUNT = 6;
  static constexpr uint8_t DEFAULT_STREAM_QUALITY = 40;
  static constexpr uint8_t DEFAULT_PHOTO_QUALITY = 80;

  /** Use sensor bring-up mode (smallest). */
  static constexpr size_t FRAMESIZE_DEFAULT = SIZE_MAX;

  Freenove_CSI_Sensor_Driver();
  ~Freenove_CSI_Sensor_Driver();

  Freenove_CSI_Sensor_Driver(const Freenove_CSI_Sensor_Driver &) = delete;
  Freenove_CSI_Sensor_Driver &operator=(const Freenove_CSI_Sensor_Driver &) = delete;

  /**
   * @brief Select mode by per-sensor index (e.g. IMX219_FRAMESIZE_640x480).
   * FRAMESIZE_DEFAULT = bring-up default. Invalid index for live sensor → false.
   */
  bool setFrameSize(size_t index);
  size_t frameSize() const;
  size_t frameSizeCount() const;
  const char *frameSizeName() const;

  bool setFlip(bool hmirror, bool vflip);
  bool getFlip(bool *hmirror, bool *vflip) const;

  /**
   * Unload video/ISP completely and begin() again with pending framesize/flip.
   * Used by live setFrameSize / setFlip so hot-reconfigure cannot leave Bayer broken.
   */
  bool reloadCamera();

  bool begin();
  void end();
  bool isStarted() const;

  uint32_t width() const;
  uint32_t height() const;
  uint8_t streamQuality() const;
  uint8_t photoQuality() const;
  const char *sensorName() const;
  freenove_csi_sensor_id_t sensorId() const;

  size_t listSupportedFrameSizes(freenove_csi_framesize_dim_t *out, size_t maxCount) const;

  uint32_t maxJpegSize() const;
  uint8_t *allocJpegBuf(size_t hintSize, size_t *actualSize);
  void freeJpegBuf(uint8_t *buf);
  int encodeJpeg(bool photo, uint8_t *dst, size_t dstSize);
  ESPVideoBufferClass captureFrame();

private:
  bool beginOnTask();
  void pipelineStop();
  bool pipelineOpenRgb565(uint16_t width, uint16_t height);
  bool pipelineStart();
  bool reconfigureLive(uint16_t width, uint16_t height);
  bool applyIspWhiteBalance();
  void syncJpegConfig();
  void matchFrameSizeIndex();
  /** All sensors: orientation via RGB565 soft flip (no sensor HFLIP/VFLIP). */
  bool usesSoftwareFlip() const;
  bool sensorFlipH() const;
  bool sensorFlipV() const;
  bool ensureRgbScratch(uint32_t width, uint32_t height);
  void freeRgbScratch();
  /** Copy RGB565 with optional mirror/flip into dst (does not touch src after). */
  void copyRgb565(const uint16_t *src, uint16_t *dst, uint32_t width, uint32_t height,
                  bool hmirror, bool vflip) const;

  ESPVideoClass video_;
  ESPVideoCaptureDevClass capture_;
  jpeg_encoder_handle_t jpegEngine_;
  jpeg_encode_cfg_t streamEncCfg_;
  jpeg_encode_cfg_t photoEncCfg_;
  uint32_t frameWidth_;
  uint32_t frameHeight_;
  SemaphoreHandle_t camLock_;
  bool started_;
  freenove_csi_sensor_id_t sensorId_;
  size_t framesizeIndex_;
  bool hmirror_;
  bool vflip_;
  /** PSRAM scratch: release V4L2 buf before JPEG (avoids ISP buffer starve / hitch). */
  uint16_t *rgbScratch_;
  size_t rgbScratchBytes_;
};
