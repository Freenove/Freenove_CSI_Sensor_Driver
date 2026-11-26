/*
 * HTTP MJPEG + soft-flip API for 02_WebCamera.
 */
#pragma once

#include <freenove_csi_sensor_driver.h>

/** Start httpd on port 80. Routes: /, /stream, /jpg, /status, /api/flip. */
bool camera_web_server_start(Freenove_CSI_Sensor_Driver &cam);
