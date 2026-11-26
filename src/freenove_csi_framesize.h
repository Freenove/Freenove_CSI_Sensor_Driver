/*
 * Shared framesize dim type + dispatch by sensor id.
 * Per-sensor index enums: sensors/{imx219,imx708,ov5647}/*_framesize.h
 */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "freenove_csi_sensor_ops.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef union {
  struct {
    uint16_t width;
    uint16_t height;
  };
  uint32_t packed;
} freenove_csi_framesize_dim_t;

#define FREENOVE_CSI_FRAMESIZE_DIM(w, h) \
  ((freenove_csi_framesize_dim_t){.width = (uint16_t)(w), .height = (uint16_t)(h)})

size_t freenove_csi_framesize_count(freenove_csi_sensor_id_t id);
bool freenove_csi_framesize_get(freenove_csi_sensor_id_t id, size_t index,
                                freenove_csi_framesize_dim_t *out);
const char *freenove_csi_framesize_name(freenove_csi_sensor_id_t id, size_t index);
size_t freenove_csi_framesize_find(freenove_csi_sensor_id_t id, uint16_t width, uint16_t height);

#ifdef __cplusplus
}
#endif
