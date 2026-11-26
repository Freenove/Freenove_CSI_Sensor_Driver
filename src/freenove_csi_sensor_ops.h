/*
 * freenove_csi_sensor_ops — sensor-only helpers (no capture / JPEG / WB).
 *
 *   peek / identify / lookup  → read-only
 *   set_flip_cids             → HFLIP / VFLIP CIDs
 *   sensor_commit             → S_SENSOR_FMT (+ CIDs) for resolution / flip + Bayer
 *
 * Capture reopen and ISP WB stay in Freenove_CSI_Sensor_Driver.
 */
#pragma once

#include "esp_cam_sensor_types.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  FREENOVE_CSI_SENSOR_UNKNOWN = 0,
  FREENOVE_CSI_SENSOR_IMX219,
  FREENOVE_CSI_SENSOR_IMX708,
  FREENOVE_CSI_SENSOR_OV5647,
} freenove_csi_sensor_id_t;

freenove_csi_sensor_id_t freenove_csi_identify_sensor(void);

const esp_cam_sensor_format_t *freenove_csi_format_by_size(freenove_csi_sensor_id_t id,
                                                          uint16_t width, uint16_t height);

bool freenove_csi_format_supported(freenove_csi_sensor_id_t id, uint16_t width, uint16_t height);

bool freenove_csi_peek_sensor_size(uint16_t *width, uint16_t *height);

bool freenove_csi_set_flip_cids(bool hmirror, bool vflip);

/**
 * Generic flip: IMX219/IMX708 → sensor reg helpers; OV5647 → V4L2 HFLIP/VFLIP.
 * Same signature for every plugged module when called with the live sensor id.
 */
bool freenove_csi_set_flip(freenove_csi_sensor_id_t id, bool hmirror, bool vflip);
bool freenove_csi_get_flip(freenove_csi_sensor_id_t id, bool *hmirror, bool *vflip);

/**
 * Program sensor mode + flip so ISP Bayer matches.
 * Does not open V4L2 capture buffers; caller must have dropped mmap first.
 */
bool freenove_csi_sensor_commit(freenove_csi_sensor_id_t id, uint16_t width, uint16_t height,
                                bool hmirror, bool vflip);

#ifdef __cplusplus
}
#endif
