/*
 * SPDX-FileCopyrightText: 2026 esp_cam_sensor_imx contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdbool.h>
#include "esp_cam_sensor.h"
#include "esp_cam_sensor_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IMX219_SENSOR_NAME "IMX219"

/*
 * 7-bit SCCB/I2C address. The IMX219 is strapped to 0x10 on all Raspberry Pi
 * camera modules.
 */
#ifndef IMX219_SCCB_ADDR
#define IMX219_SCCB_ADDR   0x10
#endif

#define IMX219_PID         IMX219_CHIP_ID_VALUE
#define IMX219_CHIP_ID_VALUE 0x0219

/**
 * @brief Probe and initialise an IMX219 on the given interface.
 *
 * @param config Pointer to esp_cam_sensor_config_t.
 * @return Sensor device handle on success, NULL on failure.
 */
esp_cam_sensor_device_t *imx219_detect(esp_cam_sensor_config_t *config);

/**
 * @brief Number of MIPI RAW10 modes (indices match
 *        CAMERA_IMX219_MIPI_IF_FORMAT_INDEX_DEFAULT).
 */
size_t imx219_format_count(void);

/**
 * @brief Mode by table index: 0=1920x1080, 1=1640x1232, 2=640x480.
 * @return Pointer into the static mode table, or NULL if out of range.
 */
const esp_cam_sensor_format_t *imx219_format_by_index(size_t index);

/**
 * @brief Mode by output size.
 * @return First matching mode, or NULL if none.
 */
const esp_cam_sensor_format_t *imx219_format_by_size(uint16_t width, uint16_t height);

/**
 * @brief Horizontal mirror / vertical flip via reg 0x0172 (bit0 / bit1).
 * Updates sensor + cached Bayer. Prefer before STREAMON / after S_SENSOR_FMT
 * so ISP re-latches Bayer. ESP_ERR_INVALID_STATE if not detected.
 */
esp_err_t imx219_set_flip(bool hmirror, bool vflip);
esp_err_t imx219_get_flip(bool *hmirror, bool *vflip);

#ifdef __cplusplus
}
#endif
