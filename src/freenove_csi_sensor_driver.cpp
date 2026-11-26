/*
 * freenove_csi_sensor_driver.cpp — public API + capture pipeline + ISP WB.
 *
 * Sensor mode / Bayer go through freenove_csi_sensor_ops.
 * Flip is always RGB565 software in encodeJpeg (no sensor HFLIP/VFLIP).
 */
#include "freenove_csi_sensor_driver.h"
#include "esp_cache.h"
#include "esp_heap_caps.h"
#include "esp_video_device.h"
#include "esp_video_isp_ioctl.h"
#include "freertos/task.h"
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "linux/videodev2.h"

extern "C" void freenove_force_cam_detect_override(void);
extern "C" void freenove_force_ipa_config(void);

Freenove_CSI_Sensor_Driver::Freenove_CSI_Sensor_Driver()
  : jpegEngine_(nullptr),
    frameWidth_(0),
    frameHeight_(0),
    camLock_(nullptr),
    started_(false),
    sensorId_(FREENOVE_CSI_SENSOR_UNKNOWN),
    framesizeIndex_(FRAMESIZE_DEFAULT),
    hmirror_(false),
    vflip_(false),
    rgbScratch_(nullptr),
    rgbScratchBytes_(0) {
  memset(&streamEncCfg_, 0, sizeof(streamEncCfg_));
  memset(&photoEncCfg_, 0, sizeof(photoEncCfg_));
}

Freenove_CSI_Sensor_Driver::~Freenove_CSI_Sensor_Driver() {
  end();
}

void Freenove_CSI_Sensor_Driver::pipelineStop() {
  if (capture_.isCaptureStarted()) {
    (void)capture_.stopCapture();
  }
  if (capture_.isOpened()) {
    capture_.end();
  }
}

bool Freenove_CSI_Sensor_Driver::pipelineOpenRgb565(uint16_t width, uint16_t height) {
  if (width == 0 || height == 0) {
    return false;
  }

  pipelineStop();

  {
    const int fd = ::open(ESP_VIDEO_MIPI_CSI_DEVICE_NAME, O_RDWR);
    if (fd < 0) {
      return false;
    }
    struct v4l2_format format = {};
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bool ok = (ioctl(fd, VIDIOC_G_FMT, &format) == 0);
    if (ok) {
      format.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB565;
      format.fmt.pix.width = width;
      format.fmt.pix.height = height;
      format.fmt.pix.bytesperline = 0;
      format.fmt.pix.sizeimage = 0;
      ok = (ioctl(fd, VIDIOC_S_FMT, &format) == 0);
    }
    if (ok) {
      struct v4l2_format got = {};
      got.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      ok = (ioctl(fd, VIDIOC_G_FMT, &got) == 0) &&
           (got.fmt.pix.width == width) && (got.fmt.pix.height == height);
      if (!ok) {
        Serial.printf("[CSI_Sensor] RGB565 S_FMT took %ux%u, wanted %ux%u\n",
                      got.fmt.pix.width, got.fmt.pix.height, width, height);
      }
    }
    ::close(fd);
    if (!ok) {
      Serial.println("[CSI_Sensor] RGB565 pipeline S_FMT failed");
      return false;
    }
  }

  if (!capture_.begin(ESP_VIDEO_MIPI_CSI_DEVICE_NAME, DEFAULT_BUF_COUNT)) {
    Serial.println("[CSI_Sensor] capture reopen failed");
    return false;
  }
  if (!capture_.setFormat(ESP_VIDEO_FORMAT_RGB565)) {
    Serial.println("[CSI_Sensor] RGB565 setFormat failed");
    capture_.end();
    return false;
  }
  capture_.ESPVideoSolutionClass::begin(width, height);

  if (capture_.getWidth() != width || capture_.getHeight() != height) {
    Serial.printf("[CSI_Sensor] capture size %lux%lu != %ux%u\n",
                  (unsigned long)capture_.getWidth(),
                  (unsigned long)capture_.getHeight(), width, height);
    return false;
  }
  return true;
}

bool Freenove_CSI_Sensor_Driver::pipelineStart() {
  if (!capture_.isOpened()) {
    return false;
  }
  if (capture_.isCaptureStarted()) {
    return true;
  }
  return capture_.startCapture();
}

bool Freenove_CSI_Sensor_Driver::usesSoftwareFlip() const {
  return true;
}

bool Freenove_CSI_Sensor_Driver::sensorFlipH() const {
  return false;
}

bool Freenove_CSI_Sensor_Driver::sensorFlipV() const {
  return false;
}

void Freenove_CSI_Sensor_Driver::freeRgbScratch() {
  if (rgbScratch_ != nullptr) {
    free(rgbScratch_);
    rgbScratch_ = nullptr;
    rgbScratchBytes_ = 0;
  }
}

bool Freenove_CSI_Sensor_Driver::ensureRgbScratch(uint32_t width, uint32_t height) {
  const size_t need = (size_t)width * (size_t)height * sizeof(uint16_t);
  if (need == 0) {
    return false;
  }
  if (rgbScratch_ != nullptr && rgbScratchBytes_ >= need) {
    return true;
  }
  freeRgbScratch();
  /* 64B align for cache msync + HW JPEG DMA. */
  rgbScratch_ = static_cast<uint16_t *>(
    heap_caps_aligned_alloc(64, need, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  if (rgbScratch_ == nullptr) {
    rgbScratch_ = static_cast<uint16_t *>(
      heap_caps_aligned_alloc(64, need, MALLOC_CAP_8BIT));
  }
  if (rgbScratch_ == nullptr) {
    return false;
  }
  rgbScratchBytes_ = need;
  return true;
}

void Freenove_CSI_Sensor_Driver::copyRgb565(const uint16_t *src, uint16_t *dst, uint32_t width,
                                            uint32_t height, bool hmirror, bool vflip) const {
  if (src == nullptr || dst == nullptr || width == 0 || height == 0) {
    return;
  }
  if (!hmirror && !vflip) {
    memcpy(dst, src, (size_t)width * (size_t)height * sizeof(uint16_t));
    return;
  }
  for (uint32_t y = 0; y < height; y++) {
    const uint32_t sy = vflip ? (height - 1 - y) : y;
    const uint16_t *srow = src + sy * width;
    uint16_t *drow = dst + y * width;
    if (!hmirror) {
      memcpy(drow, srow, (size_t)width * sizeof(uint16_t));
    } else {
      for (uint32_t x = 0; x < width; x++) {
        drow[x] = srow[width - 1 - x];
      }
    }
  }
}

bool Freenove_CSI_Sensor_Driver::reconfigureLive(uint16_t width, uint16_t height) {
  pipelineStop();
  if (!freenove_csi_sensor_commit(sensorId_, width, height, sensorFlipH(), sensorFlipV())) {
    return false;
  }
  if (!pipelineOpenRgb565(width, height) || !pipelineStart()) {
    return false;
  }
  frameWidth_ = capture_.getWidth();
  frameHeight_ = capture_.getHeight();
  if (!ensureRgbScratch(frameWidth_, frameHeight_)) {
    Serial.println("[CSI_Sensor] RGB scratch resize failed");
    return false;
  }
  syncJpegConfig();
  (void)applyIspWhiteBalance();
  return true;
}

bool Freenove_CSI_Sensor_Driver::reloadCamera() {
  const size_t fs = framesizeIndex_;
  const bool h = hmirror_;
  const bool v = vflip_;

  Serial.printf("[CSI_Sensor] reload: unload + begin (fs=%u h=%d v=%d)\n",
                fs == FRAMESIZE_DEFAULT ? 0u : (unsigned)fs, h ? 1 : 0, v ? 1 : 0);

  end();
  framesizeIndex_ = fs;
  hmirror_ = h;
  vflip_ = v;

  const bool ok = begin();
  if (!ok) {
    Serial.println("[CSI_Sensor] reload failed");
  } else {
    Serial.printf("[CSI_Sensor] reload ok: %s %lux%lu fs[%u]=%s flip h=%d v=%d\n",
                  sensorName(),
                  (unsigned long)frameWidth_, (unsigned long)frameHeight_,
                  framesizeIndex_ == FRAMESIZE_DEFAULT ? 0u : (unsigned)framesizeIndex_,
                  frameSizeName(), hmirror_ ? 1 : 0, vflip_ ? 1 : 0);
  }
  return ok;
}

bool Freenove_CSI_Sensor_Driver::setFrameSize(size_t index) {
  if (index == FRAMESIZE_DEFAULT) {
    framesizeIndex_ = FRAMESIZE_DEFAULT;
    if (started_) {
      return reloadCamera();
    }
    return true;
  }

  if (!started_) {
    /* Index is sensor-specific; applied after detect in begin(). */
    framesizeIndex_ = index;
    return true;
  }

  freenove_csi_framesize_dim_t dim = {};
  if (!freenove_csi_framesize_get(sensorId_, index, &dim)) {
    return false;
  }

  framesizeIndex_ = index;
  return reloadCamera();
}

size_t Freenove_CSI_Sensor_Driver::frameSize() const {
  return framesizeIndex_;
}

size_t Freenove_CSI_Sensor_Driver::frameSizeCount() const {
  if (!started_ || sensorId_ == FREENOVE_CSI_SENSOR_UNKNOWN) {
    return 0;
  }
  return freenove_csi_framesize_count(sensorId_);
}

const char *Freenove_CSI_Sensor_Driver::frameSizeName() const {
  if (framesizeIndex_ == FRAMESIZE_DEFAULT || sensorId_ == FREENOVE_CSI_SENSOR_UNKNOWN) {
    return "DEFAULT";
  }
  return freenove_csi_framesize_name(sensorId_, framesizeIndex_);
}

bool Freenove_CSI_Sensor_Driver::setFlip(bool hmirror, bool vflip) {
  hmirror_ = hmirror;
  vflip_ = vflip;
  if (started_) {
    Serial.printf("[CSI_Sensor] soft flip hmirror=%d vflip=%d (no reload)\n",
                  hmirror_ ? 1 : 0, vflip_ ? 1 : 0);
  }
  return true;
}

bool Freenove_CSI_Sensor_Driver::getFlip(bool *hmirror, bool *vflip) const {
  if (hmirror) {
    *hmirror = hmirror_;
  }
  if (vflip) {
    *vflip = vflip_;
  }
  return true;
}

bool Freenove_CSI_Sensor_Driver::begin() {
  if (started_) {
    return true;
  }

  /*
   * Arduino loop task is only 8KB. esp_video_init + IMX708 IPA/register paths
   * overflow that stack and look like a hang right after "sensor detect".
   * Run bring-up on a dedicated 32KB stack (Espressif xTaskCreate uses bytes).
   */
  static constexpr uint32_t kBeginStackBytes = 32768;
  struct BeginCtx {
    Freenove_CSI_Sensor_Driver *self;
    SemaphoreHandle_t done;
    bool ok;
  } ctx = {this, xSemaphoreCreateBinary(), false};

  if (ctx.done == nullptr) {
    Serial.println("[CSI_Sensor] begin: no semaphore, running inline");
    return beginOnTask();
  }

  const BaseType_t created =
      xTaskCreate([](void *arg) {
                    auto *c = static_cast<BeginCtx *>(arg);
                    c->ok = c->self->beginOnTask();
                    xSemaphoreGive(c->done);
                    vTaskDelete(nullptr);
                  },
                  "csi_begin", kBeginStackBytes, &ctx, 5, nullptr);
  if (created != pdPASS) {
    vSemaphoreDelete(ctx.done);
    Serial.println("[CSI_Sensor] begin: task create failed, running inline");
    return beginOnTask();
  }

  xSemaphoreTake(ctx.done, portMAX_DELAY);
  vSemaphoreDelete(ctx.done);
  return ctx.ok;
}

bool Freenove_CSI_Sensor_Driver::beginOnTask() {
  if (started_) {
    return true;
  }

  freenove_force_cam_detect_override();
  freenove_force_ipa_config();

  Serial.println("[CSI_Sensor] begin: SCCB config");
  Serial.flush();
  ESPVideoCamConfigClass camConfig;
  if (!camConfig.begin(static_cast<i2c_port_num_t>(DEFAULT_I2C_PORT),
                       DEFAULT_SCL_PIN, DEFAULT_SDA_PIN, DEFAULT_I2C_FREQ_HZ,
                       -1, -1)) {
    Serial.println("[CSI_Sensor] failed to configure SCCB (I2C)");
    return false;
  }

  Serial.println("[CSI_Sensor] begin: CSI config");
  Serial.flush();
  ESPVideoCSIConfigClass csiConfig;
  if (!csiConfig.begin(camConfig)) {
    Serial.println("[CSI_Sensor] failed to configure MIPI-CSI");
    return false;
  }

  Serial.println("[CSI_Sensor] begin: video_.begin (sensor detect)...");
  Serial.flush();
  if (!video_.begin(csiConfig)) {
    Serial.println("[CSI_Sensor] failed to init MIPI-CSI (cable or sensor?)");
    return false;
  }
  Serial.println("[CSI_Sensor] begin: video_.begin OK");
  Serial.flush();

  if (!capture_.begin(ESP_VIDEO_MIPI_CSI_DEVICE_NAME, DEFAULT_BUF_COUNT)) {
    Serial.println("[CSI_Sensor] failed to open capture device");
    video_.end();
    return false;
  }
  sensorId_ = freenove_csi_identify_sensor();
  capture_.end();

  const bool isOv5647 = (sensorId_ == FREENOVE_CSI_SENSOR_OV5647);
  freenove_csi_framesize_dim_t dim = {};
  bool haveDim = false;
  if (isOv5647) {
    /* Product: only 800x640 — select it at bring-up (soft flip handles orientation). */
    framesizeIndex_ = OV5647_FRAMESIZE_800x640;
    if (freenove_csi_framesize_get(sensorId_, framesizeIndex_, &dim)) {
      haveDim = true;
    } else {
      Serial.println("[CSI_Sensor] OV5647 800x640 missing — using sensor default");
      framesizeIndex_ = FRAMESIZE_DEFAULT;
    }
  } else if (framesizeIndex_ != FRAMESIZE_DEFAULT) {
    if (!freenove_csi_framesize_get(sensorId_, framesizeIndex_, &dim)) {
      Serial.printf("[CSI_Sensor] framesize[%u] invalid for %s — using sensor default\n",
                    (unsigned)framesizeIndex_, sensorName());
      framesizeIndex_ = FRAMESIZE_DEFAULT;
    } else {
      haveDim = true;
    }
  }

  if (haveDim) {
    if (!freenove_csi_sensor_commit(sensorId_, dim.width, dim.height, sensorFlipH(),
                                    sensorFlipV()) ||
        !pipelineOpenRgb565(dim.width, dim.height)) {
      Serial.println("[CSI_Sensor] mode at begin failed");
      video_.end();
      return false;
    }
  } else {
    if (!capture_.begin(ESP_VIDEO_MIPI_CSI_DEVICE_NAME, DEFAULT_BUF_COUNT)) {
      Serial.println("[CSI_Sensor] failed to reopen capture device");
      video_.end();
      return false;
    }
    if (!capture_.setFormat(ESP_VIDEO_FORMAT_RGB565)) {
      Serial.println("[CSI_Sensor] failed to set RGB565 (enable ISP / PSRAM)");
      capture_.end();
      video_.end();
      return false;
    }
  }

  if (!capture_.startCapture()) {
    Serial.println("[CSI_Sensor] failed to start capture");
    capture_.end();
    video_.end();
    return false;
  }

  frameWidth_ = capture_.getWidth();
  frameHeight_ = capture_.getHeight();

  /* Flip is applied in encodeJpeg (RGB565 soft flip); leave sensor orientation alone. */

  const bool wbOk = applyIspWhiteBalance();
  matchFrameSizeIndex();

  jpeg_encode_engine_cfg_t engCfg = {};
  engCfg.timeout_ms = 5000;
  if (jpeg_new_encoder_engine(&engCfg, &jpegEngine_) != ESP_OK) {
    Serial.println("[CSI_Sensor] failed to create hardware JPEG engine");
    capture_.end();
    video_.end();
    return false;
  }

  syncJpegConfig();

  if (!ensureRgbScratch(frameWidth_, frameHeight_)) {
    Serial.println("[CSI_Sensor] failed to alloc RGB scratch");
    jpeg_del_encoder_engine(jpegEngine_);
    jpegEngine_ = nullptr;
    capture_.end();
    video_.end();
    return false;
  }

  camLock_ = xSemaphoreCreateMutex();
  if (camLock_ == nullptr) {
    Serial.println("[CSI_Sensor] no memory for camera mutex");
    freeRgbScratch();
    jpeg_del_encoder_engine(jpegEngine_);
    jpegEngine_ = nullptr;
    capture_.end();
    video_.end();
    return false;
  }

  started_ = true;

  Serial.printf("[CSI_Sensor] ready: %s %lux%lu RGB565 framesize[%u]=%s, "
                "streamQ=%u photoQ=%u, SCCB SDA=%d SCL=%d, %u buffers\n",
                sensorName()[0] ? sensorName() : "sensor?",
                (unsigned long)frameWidth_, (unsigned long)frameHeight_,
                framesizeIndex_ == FRAMESIZE_DEFAULT ? 0u : (unsigned)framesizeIndex_,
                frameSizeName(),
                DEFAULT_STREAM_QUALITY, DEFAULT_PHOTO_QUALITY,
                DEFAULT_SDA_PIN, DEFAULT_SCL_PIN, (unsigned)DEFAULT_BUF_COUNT);
  if (wbOk) {
    Serial.println("[CSI_Sensor] white balance: defaults");
  } else {
    Serial.println("[CSI_Sensor] failed to set ISP white balance");
  }
  return true;
}

void Freenove_CSI_Sensor_Driver::end() {
  if (!started_ && jpegEngine_ == nullptr && camLock_ == nullptr) {
    return;
  }

  if (camLock_ != nullptr) {
    xSemaphoreTake(camLock_, portMAX_DELAY);
  }

  if (jpegEngine_ != nullptr) {
    jpeg_del_encoder_engine(jpegEngine_);
    jpegEngine_ = nullptr;
  }

  capture_.end();
  video_.end();
  freeRgbScratch();
  frameWidth_ = 0;
  frameHeight_ = 0;
  started_ = false;
  sensorId_ = FREENOVE_CSI_SENSOR_UNKNOWN;

  if (camLock_ != nullptr) {
    xSemaphoreGive(camLock_);
    vSemaphoreDelete(camLock_);
    camLock_ = nullptr;
  }
}

bool Freenove_CSI_Sensor_Driver::isStarted() const {
  return started_;
}

uint32_t Freenove_CSI_Sensor_Driver::width() const {
  return frameWidth_;
}

uint32_t Freenove_CSI_Sensor_Driver::height() const {
  return frameHeight_;
}

uint8_t Freenove_CSI_Sensor_Driver::streamQuality() const {
  return DEFAULT_STREAM_QUALITY;
}

uint8_t Freenove_CSI_Sensor_Driver::photoQuality() const {
  return DEFAULT_PHOTO_QUALITY;
}

const char *Freenove_CSI_Sensor_Driver::sensorName() const {
  switch (sensorId_) {
    case FREENOVE_CSI_SENSOR_IMX219: return "IMX219";
    case FREENOVE_CSI_SENSOR_IMX708: return "IMX708";
    case FREENOVE_CSI_SENSOR_OV5647: return "OV5647";
    default:                         return "";
  }
}

freenove_csi_sensor_id_t Freenove_CSI_Sensor_Driver::sensorId() const {
  return sensorId_;
}

void Freenove_CSI_Sensor_Driver::matchFrameSizeIndex() {
  const size_t n = freenove_csi_framesize_count(sensorId_);
  for (size_t i = 0; i < n; i++) {
    freenove_csi_framesize_dim_t d = {};
    if (!freenove_csi_framesize_get(sensorId_, i, &d)) {
      continue;
    }
    if (d.width == frameWidth_ && d.height == frameHeight_) {
      framesizeIndex_ = i;
      return;
    }
  }
  framesizeIndex_ = FRAMESIZE_DEFAULT;
}

size_t Freenove_CSI_Sensor_Driver::listSupportedFrameSizes(
    freenove_csi_framesize_dim_t *out, size_t maxCount) const {
  if (out == nullptr || maxCount == 0 || !started_ ||
      sensorId_ == FREENOVE_CSI_SENSOR_UNKNOWN) {
    return 0;
  }

  const size_t nAll = freenove_csi_framesize_count(sensorId_);
  size_t n = 0;
  for (size_t i = 0; i < nAll && n < maxCount; i++) {
    if (freenove_csi_framesize_get(sensorId_, i, &out[n])) {
      n++;
    }
  }
  return n;
}

uint32_t Freenove_CSI_Sensor_Driver::maxJpegSize() const {
  return frameWidth_ * frameHeight_ * 2;
}

uint8_t *Freenove_CSI_Sensor_Driver::allocJpegBuf(size_t hintSize, size_t *actualSize) {
  jpeg_encode_memory_alloc_cfg_t memCfg = {};
  memCfg.buffer_direction = JPEG_ENC_ALLOC_OUTPUT_BUFFER;
  return static_cast<uint8_t *>(jpeg_alloc_encoder_mem(hintSize, &memCfg, actualSize));
}

void Freenove_CSI_Sensor_Driver::freeJpegBuf(uint8_t *buf) {
  free(buf);
}

int Freenove_CSI_Sensor_Driver::encodeJpeg(bool photo, uint8_t *dst, size_t dstSize) {
  if (!started_ || dst == nullptr || dstSize == 0 || camLock_ == nullptr) {
    return -1;
  }
  if (frameWidth_ == 0 || frameHeight_ == 0) {
    return -1;
  }

  xSemaphoreTake(camLock_, portMAX_DELAY);

  int outSize = -1;
  if (!ensureRgbScratch(frameWidth_, frameHeight_)) {
    Serial.println("[CSI_Sensor] RGB scratch alloc failed");
    xSemaphoreGive(camLock_);
    return -1;
  }

  const size_t frameBytes = (size_t)frameWidth_ * (size_t)frameHeight_ * sizeof(uint16_t);
  const bool softFlip = usesSoftwareFlip() && (hmirror_ || vflip_);

  /* DQBUF → copy out → QBUF ASAP so ISP keeps 50fps without buffer starve. */
  {
    ESPVideoBufferClass buffer = capture_.captureBuffer();
    if (!buffer.valid() || buffer.data() == nullptr || buffer.size() < frameBytes) {
      xSemaphoreGive(camLock_);
      return -1;
    }

    const uintptr_t srcStart =
      reinterpret_cast<uintptr_t>(buffer.data()) & ~static_cast<uintptr_t>(0x3F);
    const uintptr_t srcEnd =
      (reinterpret_cast<uintptr_t>(buffer.data()) + buffer.size() + 0x3F) &
      ~static_cast<uintptr_t>(0x3F);
    if (esp_cache_msync(reinterpret_cast<void *>(srcStart), srcEnd - srcStart,
                        ESP_CACHE_MSYNC_FLAG_DIR_M2C | ESP_CACHE_MSYNC_FLAG_TYPE_DATA) != ESP_OK) {
      Serial.println("[CSI_Sensor] cache sync of captured frame failed");
    }

    copyRgb565(reinterpret_cast<const uint16_t *>(buffer.data()), rgbScratch_, frameWidth_,
               frameHeight_, softFlip ? hmirror_ : false, softFlip ? vflip_ : false);
  }

  const uintptr_t dstStart =
    reinterpret_cast<uintptr_t>(rgbScratch_) & ~static_cast<uintptr_t>(0x3F);
  const uintptr_t dstEnd =
    (reinterpret_cast<uintptr_t>(rgbScratch_) + frameBytes + 0x3F) &
    ~static_cast<uintptr_t>(0x3F);
  if (esp_cache_msync(reinterpret_cast<void *>(dstStart), dstEnd - dstStart,
                      ESP_CACHE_MSYNC_FLAG_DIR_C2M | ESP_CACHE_MSYNC_FLAG_TYPE_DATA) != ESP_OK) {
    Serial.println("[CSI_Sensor] cache sync of RGB scratch failed");
  }

  uint32_t encoded = 0;
  const jpeg_encode_cfg_t *encCfg = photo ? &photoEncCfg_ : &streamEncCfg_;
  const esp_err_t err =
    jpeg_encoder_process(jpegEngine_, encCfg, reinterpret_cast<uint8_t *>(rgbScratch_),
                         static_cast<uint32_t>(frameBytes), dst, static_cast<uint32_t>(dstSize),
                         &encoded);
  if (err != ESP_OK) {
    Serial.printf("[CSI_Sensor] jpeg_encoder_process failed: %d\n", static_cast<int>(err));
  } else {
    outSize = static_cast<int>(encoded);
  }

  xSemaphoreGive(camLock_);
  return outSize;
}

ESPVideoBufferClass Freenove_CSI_Sensor_Driver::captureFrame() {
  if (!started_) {
    return ESPVideoBufferClass();
  }
  return capture_.captureBuffer();
}

bool Freenove_CSI_Sensor_Driver::applyIspWhiteBalance() {
  /*
   * IMX: IPA owns AWB — keep V4L2 ISP WB off.
   * OV5647: manual gains at 1.0 / 1.0 (no IPA AWB in stock IPA for this path).
   */
  if (sensorId_ == FREENOVE_CSI_SENSOR_IMX219 || sensorId_ == FREENOVE_CSI_SENSOR_IMX708) {
    const int fdOff = open(ESP_VIDEO_ISP1_DEVICE_NAME, O_RDWR);
    if (fdOff < 0) {
      return true;
    }
    esp_video_isp_wb_t wbOff = {};
    wbOff.enable = false;
    wbOff.red_gain = 1.0f;
    wbOff.blue_gain = 1.0f;
    struct v4l2_ext_control control = {};
    control.id = V4L2_CID_USER_ESP_ISP_WB;
    control.p_u8 = reinterpret_cast<uint8_t *>(&wbOff);
    struct v4l2_ext_controls controls = {};
    controls.ctrl_class = V4L2_CTRL_CLASS_USER;
    controls.count = 1;
    controls.controls = &control;
    (void)ioctl(fdOff, VIDIOC_S_EXT_CTRLS, &controls);
    close(fdOff);
    return true;
  }

  const int fd = open(ESP_VIDEO_ISP1_DEVICE_NAME, O_RDWR);
  if (fd < 0) {
    return false;
  }

  esp_video_isp_wb_t wb = {};
  wb.enable = true;
  wb.red_gain = 1.0f;
  wb.blue_gain = 1.0f;

  struct v4l2_ext_control control = {};
  control.id = V4L2_CID_USER_ESP_ISP_WB;
  control.p_u8 = reinterpret_cast<uint8_t *>(&wb);

  struct v4l2_ext_controls controls = {};
  controls.ctrl_class = V4L2_CTRL_CLASS_USER;
  controls.count = 1;
  controls.controls = &control;

  const bool ok = (ioctl(fd, VIDIOC_S_EXT_CTRLS, &controls) == 0);
  close(fd);
  return ok;
}

void Freenove_CSI_Sensor_Driver::syncJpegConfig() {
  streamEncCfg_.width = frameWidth_;
  streamEncCfg_.height = frameHeight_;
  streamEncCfg_.src_type = JPEG_ENCODE_IN_FORMAT_RGB565;
  streamEncCfg_.sub_sample = JPEG_DOWN_SAMPLING_YUV422;
  streamEncCfg_.image_quality = DEFAULT_STREAM_QUALITY;
  /* Per-sensor RGB565 byte order for the HW JPEG encoder. */
  switch (sensorId_) {
    case FREENOVE_CSI_SENSOR_IMX219:
      streamEncCfg_.pixel_reverse = true;
      break;
    case FREENOVE_CSI_SENSOR_IMX708:
    case FREENOVE_CSI_SENSOR_OV5647:
    default:
      streamEncCfg_.pixel_reverse = false;
      break;
  }

  photoEncCfg_ = streamEncCfg_;
  photoEncCfg_.image_quality = DEFAULT_PHOTO_QUALITY;
}
