/*
 * SPDX-FileCopyrightText: 2024-2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-FileCopyrightText: 2026 Freenove
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Freenove product build: OV5647 exposes only 800x640 RAW8.
 */
#include <stddef.h>
#include "esp_cam_sensor.h"
#include "modes/ov5647_settings.h"
#include "ov5647_formats.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

enum {
    OV5647_FMT_800x640_RAW8_50FPS = 0,
    OV5647_FMT_MAX,
};

static const esp_cam_sensor_isp_info_t ov5647_isp_info[] = {
    [OV5647_FMT_800x640_RAW8_50FPS] = {
        .isp_v1_info = {
            .version = SENSOR_ISP_INFO_VERSION_DEFAULT,
            .pclk = 81666700,
            .vts = 1896,
            .hts = 984,
            .bayer_type = ESP_CAM_SENSOR_BAYER_GBRG,
        }
    },
};

static const esp_cam_sensor_format_t ov5647_format_info[] = {
    [OV5647_FMT_800x640_RAW8_50FPS] = {
        .name = "MIPI_2lane_24Minput_RAW8_800x640_50fps",
        .format = ESP_CAM_SENSOR_PIXFORMAT_RAW8,
        .port = ESP_CAM_SENSOR_MIPI_CSI,
        .xclk = 24000000,
        .width = 800,
        .height = 640,
        .regs = ov5647_mipi_2lane_24Minput_800x640_raw8_50fps,
        .regs_size = ARRAY_SIZE(ov5647_mipi_2lane_24Minput_800x640_raw8_50fps),
        .fps = 50,
        .isp_info = &ov5647_isp_info[OV5647_FMT_800x640_RAW8_50FPS],
        .mipi_info = {
            .mipi_clk = OV5647_MIPI_CSI_LINE_RATE_800x640_50FPS,
            .lane_num = 2,
            .line_sync_en = false,
        },
        .reserved = NULL,
    },
};

_Static_assert(OV5647_FMT_MAX == ARRAY_SIZE(ov5647_format_info),
               "OV5647 format enum and table disagree");

size_t ov5647_format_count(void)
{
    return ARRAY_SIZE(ov5647_format_info);
}

const esp_cam_sensor_format_t *ov5647_format_by_index(size_t index)
{
    if (index >= ARRAY_SIZE(ov5647_format_info)) {
        return NULL;
    }
    return &ov5647_format_info[index];
}

const esp_cam_sensor_format_t *ov5647_format_by_size(uint16_t width, uint16_t height)
{
    for (size_t i = 0; i < ARRAY_SIZE(ov5647_format_info); i++) {
        if (ov5647_format_info[i].width == width &&
            ov5647_format_info[i].height == height) {
            return &ov5647_format_info[i];
        }
    }
    return NULL;
}
