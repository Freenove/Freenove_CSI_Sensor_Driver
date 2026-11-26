/*
 * freenove_csi_framesize.cpp — dispatch to per-sensor framesize tables.
 */
#include "freenove_csi_framesize.h"
#include "sensors/imx219/imx219_framesize.h"
#include "sensors/imx708/imx708_framesize.h"
#include "sensors/ov5647/ov5647_framesize.h"

size_t freenove_csi_framesize_count(freenove_csi_sensor_id_t id) {
  switch (id) {
    case FREENOVE_CSI_SENSOR_IMX219:
      return imx219_framesize_count();
    case FREENOVE_CSI_SENSOR_IMX708:
      return imx708_framesize_count();
    case FREENOVE_CSI_SENSOR_OV5647:
      return ov5647_framesize_count();
    default:
      return 0;
  }
}

bool freenove_csi_framesize_get(freenove_csi_sensor_id_t id, size_t index,
                                freenove_csi_framesize_dim_t *out) {
  if (out == nullptr) {
    return false;
  }
  uint16_t w = 0;
  uint16_t h = 0;
  bool ok = false;
  switch (id) {
    case FREENOVE_CSI_SENSOR_IMX219:
      ok = imx219_framesize_get(index, &w, &h);
      break;
    case FREENOVE_CSI_SENSOR_IMX708:
      ok = imx708_framesize_get(index, &w, &h);
      break;
    case FREENOVE_CSI_SENSOR_OV5647:
      ok = ov5647_framesize_get(index, &w, &h);
      break;
    default:
      return false;
  }
  if (!ok) {
    return false;
  }
  out->width = w;
  out->height = h;
  return true;
}

const char *freenove_csi_framesize_name(freenove_csi_sensor_id_t id, size_t index) {
  switch (id) {
    case FREENOVE_CSI_SENSOR_IMX219:
      return imx219_framesize_name(index);
    case FREENOVE_CSI_SENSOR_IMX708:
      return imx708_framesize_name(index);
    case FREENOVE_CSI_SENSOR_OV5647:
      return ov5647_framesize_name(index);
    default:
      return "unknown";
  }
}

size_t freenove_csi_framesize_find(freenove_csi_sensor_id_t id, uint16_t width, uint16_t height) {
  const size_t n = freenove_csi_framesize_count(id);
  for (size_t i = 0; i < n; i++) {
    freenove_csi_framesize_dim_t fs = {};
    if (freenove_csi_framesize_get(id, i, &fs) && fs.width == width && fs.height == height) {
      return i;
    }
  }
  return SIZE_MAX;
}
