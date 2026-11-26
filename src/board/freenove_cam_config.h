/*
 * Build-time defaults for IMX sensors vendored into this Arduino library.
 * Arduino-esp32 3.3.12's prebuilt esp_cam_sensor has no IMX219/IMX708 Kconfig,
 * so we supply the knobs the drivers expect here.
 *
 * AUTO_DETECT macros are intentionally NOT set: registration is done by
 * freenove_cam_sensor_detect.c (overrides esp_cam_sensor_detect_get_array).
 *
 * FORMAT_INDEX_DEFAULT values are indices into each driver's format table
 * (not the public framesize list). Default = smallest mode.
 */
#pragma once

#ifndef CONFIG_CAMERA_IMX219_MIPI_IF_FORMAT_INDEX_DEFAULT
/* format table: 0=1920x1080, 1=1640x1232, 2=640x480 */
#define CONFIG_CAMERA_IMX219_MIPI_IF_FORMAT_INDEX_DEFAULT 2
#endif

#ifndef CONFIG_CAMERA_IMX708_MIPI_IF_FORMAT_INDEX_DEFAULT
/* format table: 0=1920x1080, 1=1280x720, 2=1024x768, 3=800x600, 4=640x480 */
#define CONFIG_CAMERA_IMX708_MIPI_IF_FORMAT_INDEX_DEFAULT 4
#endif
