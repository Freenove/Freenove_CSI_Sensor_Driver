/*
 * freenove_csi_sensor_ops.cpp — VIDIOC_S_SENSOR_FMT / flip CIDs only.
 *
 * Include Arduino.h before esp_video_ioctl.h — otherwise lwIP's INADDR_NONE
 * macro collides with Arduino IPAddress.h.
 */
#include <Arduino.h>
#include "freenove_csi_sensor_ops.h"
#include "esp_video_device.h"
#include "esp_video_ioctl.h"
#include "sensors/imx219/imx219.h"
#include "sensors/imx708/imx708.h"
#include "sensors/ov5647/ov5647_formats.h"
#include "esp_err.h"
#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "linux/videodev2.h"

namespace {

esp_cam_sensor_bayer_pattern_t bayerAfterHMirror(esp_cam_sensor_bayer_pattern_t b) {
  switch (b) {
    case ESP_CAM_SENSOR_BAYER_RGGB: return ESP_CAM_SENSOR_BAYER_GRBG;
    case ESP_CAM_SENSOR_BAYER_GRBG: return ESP_CAM_SENSOR_BAYER_RGGB;
    case ESP_CAM_SENSOR_BAYER_GBRG: return ESP_CAM_SENSOR_BAYER_BGGR;
    case ESP_CAM_SENSOR_BAYER_BGGR: return ESP_CAM_SENSOR_BAYER_GBRG;
    default: return b;
  }
}

esp_cam_sensor_bayer_pattern_t bayerAfterVFlip(esp_cam_sensor_bayer_pattern_t b) {
  switch (b) {
    case ESP_CAM_SENSOR_BAYER_RGGB: return ESP_CAM_SENSOR_BAYER_GBRG;
    case ESP_CAM_SENSOR_BAYER_GRBG: return ESP_CAM_SENSOR_BAYER_BGGR;
    case ESP_CAM_SENSOR_BAYER_GBRG: return ESP_CAM_SENSOR_BAYER_RGGB;
    case ESP_CAM_SENSOR_BAYER_BGGR: return ESP_CAM_SENSOR_BAYER_GRBG;
    default: return b;
  }
}

bool pushRemappedFormat(const esp_cam_sensor_format_t *base, bool hmirror, bool vflip,
                        const char *tag) {
  if (base == nullptr || base->isp_info == nullptr) {
    return false;
  }

  static esp_cam_sensor_format_t s_fmt;
  alignas(esp_cam_sensor_isp_info_t) static uint8_t s_isp_raw[sizeof(esp_cam_sensor_isp_info_t)];
  auto *s_isp = reinterpret_cast<esp_cam_sensor_isp_info_t *>(s_isp_raw);
  s_fmt = *base;
  memcpy(s_isp, base->isp_info, sizeof(*s_isp));

  esp_cam_sensor_bayer_pattern_t bayer = s_isp->isp_v1_info.bayer_type;
  if (hmirror) {
    bayer = bayerAfterHMirror(bayer);
  }
  if (vflip) {
    bayer = bayerAfterVFlip(bayer);
  }
  memcpy(&s_isp->isp_v1_info.bayer_type, &bayer, sizeof(bayer));
  s_fmt.isp_info = s_isp;

  Serial.printf("[CSI_Sensor] %s Bayer→%d (h=%d v=%d) %ux%u\n", tag, (int)bayer,
                hmirror ? 1 : 0, vflip ? 1 : 0, (unsigned)s_fmt.width,
                (unsigned)s_fmt.height);

  const int fd = ::open(ESP_VIDEO_MIPI_CSI_DEVICE_NAME, O_RDWR);
  if (fd < 0) {
    return false;
  }
  const bool ok = (ioctl(fd, VIDIOC_S_SENSOR_FMT, &s_fmt) == 0);
  ::close(fd);
  if (!ok) {
    Serial.printf("[CSI_Sensor] %s S_SENSOR_FMT failed\n", tag);
  }
  return ok;
}

}  // namespace

freenove_csi_sensor_id_t freenove_csi_identify_sensor(void) {
  const int fd = ::open(ESP_VIDEO_MIPI_CSI_DEVICE_NAME, O_RDWR);
  if (fd < 0) {
    return FREENOVE_CSI_SENSOR_UNKNOWN;
  }

  freenove_csi_sensor_id_t kind = FREENOVE_CSI_SENSOR_UNKNOWN;
  esp_cam_sensor_format_t cur = {};
  if (ioctl(fd, VIDIOC_G_SENSOR_FMT, &cur) == 0 && cur.name != nullptr) {
    for (size_t i = 0; i < imx219_format_count() && kind == FREENOVE_CSI_SENSOR_UNKNOWN; i++) {
      const esp_cam_sensor_format_t *m = imx219_format_by_index(i);
      if (m && m->name && strcmp(m->name, cur.name) == 0) {
        kind = FREENOVE_CSI_SENSOR_IMX219;
      }
    }
    for (size_t i = 0; i < imx708_format_count() && kind == FREENOVE_CSI_SENSOR_UNKNOWN; i++) {
      const esp_cam_sensor_format_t *m = imx708_format_by_index(i);
      if (m && m->name && strcmp(m->name, cur.name) == 0) {
        kind = FREENOVE_CSI_SENSOR_IMX708;
      }
    }
    for (size_t i = 0; i < ov5647_format_count() && kind == FREENOVE_CSI_SENSOR_UNKNOWN; i++) {
      const esp_cam_sensor_format_t *m = ov5647_format_by_index(i);
      if (m && m->name && strcmp(m->name, cur.name) == 0) {
        kind = FREENOVE_CSI_SENSOR_OV5647;
      }
    }
    if (kind == FREENOVE_CSI_SENSOR_UNKNOWN) {
      if (strstr(cur.name, "IMX219") || strstr(cur.name, "1640x1232")) {
        kind = FREENOVE_CSI_SENSOR_IMX219;
      } else if (strstr(cur.name, "IMX708") || strstr(cur.name, "binned_28fps")) {
        kind = FREENOVE_CSI_SENSOR_IMX708;
      } else if (strstr(cur.name, "RAW8_800") || strstr(cur.name, "binning_45fps") ||
                 strstr(cur.name, "800x1280") || strstr(cur.name, "800x640") ||
                 strstr(cur.name, "800x800") || strstr(cur.name, "1280x960") ||
                 (strstr(cur.name, "1920x1080") != nullptr &&
                  strstr(cur.name, "IMX") == nullptr)) {
        kind = FREENOVE_CSI_SENSOR_OV5647;
      }
    }
  }
  ::close(fd);
  return kind;
}

const esp_cam_sensor_format_t *freenove_csi_format_by_size(freenove_csi_sensor_id_t id,
                                                          uint16_t width, uint16_t height) {
  switch (id) {
    case FREENOVE_CSI_SENSOR_IMX219:
      return imx219_format_by_size(width, height);
    case FREENOVE_CSI_SENSOR_IMX708:
      return imx708_format_by_size(width, height);
    case FREENOVE_CSI_SENSOR_OV5647:
      return ov5647_format_by_size(width, height);
    default:
      return nullptr;
  }
}

bool freenove_csi_format_supported(freenove_csi_sensor_id_t id, uint16_t width, uint16_t height) {
  return freenove_csi_format_by_size(id, width, height) != nullptr;
}

bool freenove_csi_peek_sensor_size(uint16_t *width, uint16_t *height) {
  if (width == nullptr || height == nullptr) {
    return false;
  }
  *width = 0;
  *height = 0;
  const int fd = ::open(ESP_VIDEO_MIPI_CSI_DEVICE_NAME, O_RDWR);
  if (fd < 0) {
    return false;
  }
  esp_cam_sensor_format_t cur = {};
  const bool ok = (ioctl(fd, VIDIOC_G_SENSOR_FMT, &cur) == 0);
  ::close(fd);
  if (!ok) {
    return false;
  }
  *width = cur.width;
  *height = cur.height;
  return (*width > 0 && *height > 0);
}

bool freenove_csi_set_flip(freenove_csi_sensor_id_t id, bool hmirror, bool vflip) {
  switch (id) {
    case FREENOVE_CSI_SENSOR_IMX219:
      return imx219_set_flip(hmirror, vflip) == ESP_OK;
    case FREENOVE_CSI_SENSOR_IMX708:
      return imx708_set_flip(hmirror, vflip) == ESP_OK;
    case FREENOVE_CSI_SENSOR_OV5647:
      return freenove_csi_set_flip_cids(hmirror, vflip);
    default:
      return false;
  }
}

bool freenove_csi_get_flip(freenove_csi_sensor_id_t id, bool *hmirror, bool *vflip) {
  switch (id) {
    case FREENOVE_CSI_SENSOR_IMX219:
      return imx219_get_flip(hmirror, vflip) == ESP_OK;
    case FREENOVE_CSI_SENSOR_IMX708:
      return imx708_get_flip(hmirror, vflip) == ESP_OK;
    case FREENOVE_CSI_SENSOR_OV5647:
      /*
       * Core OV5647 rejects VIDIOC_G_EXT_CTRLS for HFLIP/VFLIP
       * (esp_video_cam: failed to get ctrl, ESP_ERR_NOT_SUPPORTED).
       * Caller should keep the values last passed to setFlip/commit.
       */
      return false;
    default:
      return false;
  }
}

bool freenove_csi_set_flip_cids(bool hmirror, bool vflip) {
  auto setCid = [](uint32_t id, int32_t value) -> bool {
    const int cfd = ::open(ESP_VIDEO_MIPI_CSI_DEVICE_NAME, O_RDWR);
    if (cfd < 0) {
      return false;
    }
    struct v4l2_ext_control control = {};
    control.id = id;
    control.value = value;
    struct v4l2_ext_controls controls = {};
    controls.count = 1;
    controls.controls = &control;
    const bool cidOk = (ioctl(cfd, VIDIOC_S_EXT_CTRLS, &controls) == 0);
    ::close(cfd);
    return cidOk;
  };
  if (!setCid(V4L2_CID_HFLIP, hmirror ? 1 : 0) && hmirror) {
    Serial.println("[CSI_Sensor] HFLIP rejected");
    return false;
  }
  if (!setCid(V4L2_CID_VFLIP, vflip ? 1 : 0) && vflip) {
    Serial.println("[CSI_Sensor] VFLIP rejected");
    return false;
  }
  return true;
}

bool freenove_csi_sensor_commit(freenove_csi_sensor_id_t id, uint16_t width, uint16_t height,
                                bool hmirror, bool vflip) {
  const esp_cam_sensor_format_t *base = freenove_csi_format_by_size(id, width, height);
  if (base == nullptr) {
    Serial.printf("[CSI_Sensor] no format %ux%u for sensor id %d\n", width, height, (int)id);
    return false;
  }

  switch (id) {
    case FREENOVE_CSI_SENSOR_IMX219:
    case FREENOVE_CSI_SENSOR_IMX708:
      /* CIDs first: soft-reset path re-asserts 0x0101 from para after tables. */
      if (!freenove_csi_set_flip_cids(hmirror, vflip)) {
        return false;
      }
      return pushRemappedFormat(base, hmirror, vflip,
                                (id == FREENOVE_CSI_SENSOR_IMX708) ? "IMX708" : "IMX219");

    case FREENOVE_CSI_SENSOR_OV5647:
      /* Orientation is software-flipped; avoid HFLIP/VFLIP CIDs (Bayer mesh). */
      if (!pushRemappedFormat(base, false, false, "OV5647")) {
        return false;
      }
      (void)hmirror;
      (void)vflip;
      return true;

    default:
      Serial.println("[CSI_Sensor] sensor_commit: unknown sensor");
      return false;
  }
}
