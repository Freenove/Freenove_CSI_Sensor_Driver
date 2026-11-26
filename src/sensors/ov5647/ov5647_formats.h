/*
 * SPDX-FileCopyrightText: 2024-2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-FileCopyrightText: 2026 Freenove
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * OV5647 hardware mode table for Freenove_CSI_Sensor_Driver.
 * Does not register a second detect() — only exposes formats for
 * VIDIOC_S_SENSOR_FMT after the core OV5647 driver has claimed the sensor.
 */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include "esp_cam_sensor.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t ov5647_format_count(void);
const esp_cam_sensor_format_t *ov5647_format_by_index(size_t index);
const esp_cam_sensor_format_t *ov5647_format_by_size(uint16_t width, uint16_t height);

#ifdef __cplusplus
}
#endif
