/*
 * SPDX-FileCopyrightText: 2026 esp_cam_sensor_imx contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * IMX219 register initialisation sequences.
 *
 * Mode windows match the Raspberry Pi / Linux imx219 driver
 * (drivers/media/i2c/imx219.c). 3280x2464 is omitted: wider than the
 * ESP32-P4 datapath. Values are plain data (address, byte); 16-bit
 * registers are written big-endian as two entries.
 */
#pragma once

#include <stdint.h>
#include "imx219_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t reg;   /*!< register address, or IMX219_REG_END / IMX219_REG_DELAY */
    uint8_t  val;   /*!< value to write, or delay in ms for IMX219_REG_DELAY   */
} imx219_reginfo_t;

/* XCLK is 24 MHz on the Raspberry Pi camera modules. EXCK_FREQ = 24 * 256. */
#define IMX219_XCLK_FREQ_HZ   24000000

/*
 * Common registers, applied before any mode. Access-enable sequence for the
 * 0x3000-0x5fff range, undocumented tuning registers, line length (HTS=3448),
 * pixel increments and D-PHY/clock setup.
 */
static const imx219_reginfo_t imx219_common_regs[] = {
    {IMX219_REG_MODE_SELECT, 0x00},          /* ensure standby */

    /* Access code to reach registers 0x3000-0x5fff */
    {0x30eb, 0x05}, {0x30eb, 0x0c},
    {0x300a, 0xff}, {0x300b, 0xff},
    {0x30eb, 0x05}, {0x30eb, 0x09},

    /* Undocumented Sony tuning registers */
    {0x455e, 0x00}, {0x471e, 0x4b}, {0x4767, 0x0f}, {0x4750, 0x14},
    {0x4540, 0x00}, {0x47b4, 0x14}, {0x4713, 0x30}, {0x478b, 0x10},
    {0x478f, 0x10}, {0x4793, 0x10}, {0x4797, 0x0e}, {0x479b, 0x0e},

    /* Line length = 3448 (0x0d78) */
    {IMX219_REG_LINE_LENGTH_H, 0x0d}, {IMX219_REG_LINE_LENGTH_L, 0x78},
    {IMX219_REG_X_ODD_INC, 0x01},
    {IMX219_REG_Y_ODD_INC, 0x01},

    /* D-PHY auto timing; EXCK_FREQ = 24MHz * 256 = 0x1800 */
    {IMX219_REG_DPHY_CTRL, 0x00},
    {IMX219_REG_EXCK_FREQ_H, 0x18}, {IMX219_REG_EXCK_FREQ_L, 0x00},

    {IMX219_REG_END, 0x00},
};

/*
 * 2-lane PLL configuration. Fixed link frequency of 456 MHz (912 Mbps/lane)
 * across all modes; pixel rate 182.4 MHz.
 */
static const imx219_reginfo_t imx219_2lane_regs[] = {
    {IMX219_REG_VTPXCK_DIV, 5},
    {IMX219_REG_VTSYCK_DIV, 1},
    {IMX219_REG_PREPLLCK_VT_DIV, 3},
    {IMX219_REG_PREPLLCK_OP_DIV, 3},
    {IMX219_REG_PLL_VT_MPY_H, 0x00}, {IMX219_REG_PLL_VT_MPY_L, 57},  /* 0x0039 */
    {IMX219_REG_OPSYCK_DIV, 1},
    {IMX219_REG_PLL_OP_MPY_H, 0x00}, {IMX219_REG_PLL_OP_MPY_L, 114}, /* 0x0072 */
    {IMX219_REG_CSI_LANE_MODE, IMX219_CSI_2_LANE_MODE},
    {IMX219_REG_END, 0x00},
};

/* RAW10 output format */
static const imx219_reginfo_t imx219_raw10_regs[] = {
    {IMX219_REG_CSI_DATA_FORMAT_H, 0x0a}, {IMX219_REG_CSI_DATA_FORMAT_L, 0x0a},
    {IMX219_REG_OPPXCK_DIV, 10},
    {IMX219_REG_END, 0x00},
};

/* RAW8 output format */
static const imx219_reginfo_t imx219_raw8_regs[] = {
    {IMX219_REG_CSI_DATA_FORMAT_H, 0x08}, {IMX219_REG_CSI_DATA_FORMAT_L, 0x08},
    {IMX219_REG_OPPXCK_DIV, 8},
    {IMX219_REG_END, 0x00},
};

/*
 * Mode: 1920x1080, unbinned centre crop, RAW10, ~30fps.
 * Linux imx219 mode (1080P cropped). VTS = 0x06e3.
 */
static const imx219_reginfo_t imx219_mode_1920x1080_regs[] = {
    {IMX219_REG_X_ADD_STA_H, 0x02}, {IMX219_REG_X_ADD_STA_L, 0xa8},   /* 680  */
    {IMX219_REG_X_ADD_END_H, 0x0a}, {IMX219_REG_X_ADD_END_L, 0x27},   /* 2599 */
    {IMX219_REG_Y_ADD_STA_H, 0x02}, {IMX219_REG_Y_ADD_STA_L, 0xb4},   /* 692  */
    {IMX219_REG_Y_ADD_END_H, 0x06}, {IMX219_REG_Y_ADD_END_L, 0xeb},   /* 1771 */
    {IMX219_REG_X_OUTPUT_SIZE_H, 0x07}, {IMX219_REG_X_OUTPUT_SIZE_L, 0x80}, /* 1920 */
    {IMX219_REG_Y_OUTPUT_SIZE_H, 0x04}, {IMX219_REG_Y_OUTPUT_SIZE_L, 0x38}, /* 1080 */
    {IMX219_REG_BINNING_MODE_H, 0x00}, {IMX219_REG_BINNING_MODE_L, 0x00},   /* none */
    {IMX219_REG_VTS_H, 0x06}, {IMX219_REG_VTS_L, 0xe3},               /* 1763 */
    {IMX219_REG_END, 0x00},
};

/*
 * Mode: 1640x1232, 2x2 binned, RAW10, ~30fps. Full field of view.
 * Linux imx219 2x2 binned mode. VTS = 0x06e3.
 */
static const imx219_reginfo_t imx219_mode_1640x1232_regs[] = {
    {IMX219_REG_X_ADD_STA_H, 0x00}, {IMX219_REG_X_ADD_STA_L, 0x00},   /* 0    */
    {IMX219_REG_X_ADD_END_H, 0x0c}, {IMX219_REG_X_ADD_END_L, 0xcf},   /* 3279 */
    {IMX219_REG_Y_ADD_STA_H, 0x00}, {IMX219_REG_Y_ADD_STA_L, 0x00},   /* 0    */
    {IMX219_REG_Y_ADD_END_H, 0x09}, {IMX219_REG_Y_ADD_END_L, 0x9f},   /* 2463 */
    {IMX219_REG_X_OUTPUT_SIZE_H, 0x06}, {IMX219_REG_X_OUTPUT_SIZE_L, 0x68}, /* 1640 */
    {IMX219_REG_Y_OUTPUT_SIZE_H, 0x04}, {IMX219_REG_Y_OUTPUT_SIZE_L, 0xd0}, /* 1232 */
    {IMX219_REG_BINNING_MODE_H, 0x01}, {IMX219_REG_BINNING_MODE_L, 0x01},   /* 2x2 normal */
    {IMX219_REG_VTS_H, 0x06}, {IMX219_REG_VTS_L, 0xe3},               /* 1763 */
    {IMX219_REG_END, 0x00},
};

/*
 * Mode: 640x480, 2x2 analog-binned centre crop, RAW10, ~30fps.
 * Linux imx219 VGA mode (pre-bin window 1280x960). VTS = 0x06e3.
 */
static const imx219_reginfo_t imx219_mode_640x480_regs[] = {
    {IMX219_REG_X_ADD_STA_H, 0x03}, {IMX219_REG_X_ADD_STA_L, 0xe8},   /* 1000 */
    {IMX219_REG_X_ADD_END_H, 0x08}, {IMX219_REG_X_ADD_END_L, 0xe7},   /* 2279 */
    {IMX219_REG_Y_ADD_STA_H, 0x02}, {IMX219_REG_Y_ADD_STA_L, 0xf0},   /* 752  */
    {IMX219_REG_Y_ADD_END_H, 0x06}, {IMX219_REG_Y_ADD_END_L, 0xaf},   /* 1711 */
    {IMX219_REG_X_OUTPUT_SIZE_H, 0x02}, {IMX219_REG_X_OUTPUT_SIZE_L, 0x80}, /* 640  */
    {IMX219_REG_Y_OUTPUT_SIZE_H, 0x01}, {IMX219_REG_Y_OUTPUT_SIZE_L, 0xe0}, /* 480  */
    {IMX219_REG_BINNING_MODE_H, 0x03}, {IMX219_REG_BINNING_MODE_L, 0x03},   /* 2x2 analog */
    {IMX219_REG_VTS_H, 0x06}, {IMX219_REG_VTS_L, 0xe3},               /* 1763 */
    {IMX219_REG_END, 0x00},
};

#ifdef __cplusplus
}
#endif
