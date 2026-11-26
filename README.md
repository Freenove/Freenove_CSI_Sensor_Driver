# Freenove_CSI_Sensor_Driver

Arduino library: **esp_cam_sensor** + **ESP_Video** for **ESP32-P4 MIPI-CSI**.  
Auto-detect → RGB565 capture → hardware JPEG.  
Runtime knob: `setFlip()` (**RGB565 software flip**, no sensor register changes). One fixed resolution per sensor.

Chinese: [README.cn.md](README.cn.md)

## Modules

| Module | Sensor | SCCB | Fixed size |
|--------|--------|------|------------|
| Camera Module v1 | OV5647 | 0x36 | 800×640 |
| Camera Module v2 | IMX219 | 0x10 | 640×480 |
| Camera Module 3 | IMX708 | 0x1A | 640×480 |

One module at a time. Needs arduino-esp32 **3.3.12** (this library vendors IMX + detect/IPA overrides). **PSRAM = Enabled**.

## Layout

```
src/
  freenove_csi_sensor_driver.*   # Arduino API (capture / JPEG / soft flip)
  freenove_csi_sensor_ops.*      # identify / S_SENSOR_FMT (resolution)
  freenove_csi_framesize.*       # dispatch by sensor id
  board/                         # detect table, IPA
  sensors/imx219|imx708|ov5647/  # drivers + *_framesize.*
examples/
  01_ConfigAPI/                  # serial probe + soft flip + JPEG
  02_WebCamera/                  # Wi-Fi MJPEG (modular web)
```

## Pins (FNK0117)

| Signal | GPIO |
|--------|------|
| SCCB SDA / SCL | 7 / 8 |
| RESET / PWDN | -1 |

## Framesizes (one mode each)

| Enum | Size |
|------|------|
| `IMX219_FRAMESIZE_640x480` | 640×480 |
| `IMX708_FRAMESIZE_640x480` | 640×480 |
| `OV5647_FRAMESIZE_800x640` | 800×640 |

`FRAMESIZE_DEFAULT` = bring-up default (same as above). `setFrameSize` is usually unnecessary.

## Internal defaults

| Item | IMX219 | IMX708 | OV5647 |
|------|--------|--------|--------|
| Bring-up size | 640×480 | 640×480 | 800×640 |
| Flip | RGB565 software | RGB565 software | RGB565 software |
| JPEG `pixel_reverse` | true | false | false |
| White balance | IPA AWB | IPA AWB | V4L2 gains 1.0/1.0 |

Stream / photo JPEG quality: **40 / 80**.

## Quick start

```cpp
#include <freenove_csi_sensor_driver.h>

Freenove_CSI_Sensor_Driver cam;

void setup() {
  Serial.begin(115200);
  cam.setFlip(false, false);
  if (!cam.begin()) return;

  size_t n = 0;
  uint8_t *buf = cam.allocJpegBuf(cam.maxJpegSize(), &n);
  int len = cam.encodeJpeg(false, buf, n);
  cam.freeJpegBuf(buf);
}

void loop() {}
```

## API

| API | Notes |
|-----|-------|
| `begin()` / `end()` / `isStarted()` | Lifecycle |
| `setFlip(h, v)` / `getFlip(*h, *v)` | Software mirror/flip (no camera reload) |
| `frameSize()` / `frameSizeCount()` / `frameSizeName()` / `listSupportedFrameSizes` | Fixed mode (read-mostly) |
| `setFrameSize(index)` / `reloadCamera()` | Optional; one mode per sensor |
| `width()` / `height()` / `sensorName()` / `sensorId()` | Status |
| `encodeJpeg(photo, …)` / `captureFrame()` / `allocJpegBuf` … | Capture |

## Examples

| Example | Content |
|---------|---------|
| `01_ConfigAPI` | Probe + fixed framesize list + soft flip + periodic JPEG |
| `02_WebCamera` | Wi-Fi MJPEG + soft `setFlip` (`camera_web_page.h` + `camera_web_server.*`) |
