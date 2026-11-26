/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Freenove product: OV5647 exposes 800x640 RAW8 only.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "ov5647_regs.h"
#include "ov5647_types.h"

#ifndef BIT
#define BIT(nr) (1UL << (nr))
#endif

#define OV5647_IDI_CLOCK_RATE_800x640_50FPS        (100000000ULL)
#define OV5647_MIPI_CSI_LINE_RATE_800x640_50FPS    (OV5647_IDI_CLOCK_RATE_800x640_50FPS * 4)
#define OV5647_8BIT_MODE                           (0x18)
#define OV5647_10BIT_MODE                          (0x1A)
#define OV5647_MIPI_CTRL00_CLOCK_LANE_GATE         BIT(5)
#define OV5647_MIPI_CTRL00_BUS_IDLE                BIT(2)
#define OV5647_MIPI_CTRL00_CLOCK_LANE_DISABLE      BIT(0)

#include "ov5647_mipi_2lane_24Minput_800x640_raw8_50fps.h"

#ifdef __cplusplus
}
#endif
