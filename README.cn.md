# Freenove_CSI_Sensor_Driver

Arduino 库：`esp_cam_sensor` + `ESP_Video`，**ESP32-P4 MIPI-CSI**。  
自动识别模组 → RGB565 采集 → 硬件 JPEG。  
运行时可配：`setFlip()`（**软件 RGB565 翻转**，不改传感器寄存器）。分辨率按模组固定一档。

英文：[README.md](README.md)

## 模组

| 模组 | 传感器 | SCCB | 固定分辨率 |
|------|--------|------|------------|
| Camera Module v1 | OV5647 | 0x36 | 800×640 |
| Camera Module v2 | IMX219 | 0x10 | 640×480 |
| Camera Module 3 | IMX708 | 0x1A | 640×480 |

一次只接一个。依赖 arduino-esp32 **3.3.12**（本库补 IMX 驱动 / 探测表 / IPA）。**PSRAM = Enabled**。

## 目录

```
src/
  freenove_csi_sensor_driver.*   # Arduino API（采集 / JPEG / 软翻转）
  freenove_csi_sensor_ops.*      # 识别 / S_SENSOR_FMT（分辨率）
  freenove_csi_framesize.*       # 按 sensor id 分发档位
  board/                         # 探测表、IPA
  sensors/imx219|imx708|ov5647/  # 驱动 + *_framesize.*
examples/
  01_ConfigAPI/                  # 串口探测 + 软翻转 + JPEG
  02_WebCamera/                  # Wi-Fi MJPEG（网页模块化）
```

## 引脚（FNK0117）

| 信号 | GPIO |
|------|------|
| SCCB SDA / SCL | 7 / 8 |
| RESET / PWDN | -1 |

## 分辨率（每传感器仅一档）

| 枚举 | 尺寸 |
|------|------|
| `IMX219_FRAMESIZE_640x480` | 640×480 |
| `IMX708_FRAMESIZE_640x480` | 640×480 |
| `OV5647_FRAMESIZE_800x640` | 800×640 |

`FRAMESIZE_DEFAULT` = 上电默认（与上表一致）。一般无需再调 `setFrameSize`。

## 底层默认（库内）

| 项 | IMX219 | IMX708 | OV5647 |
|----|--------|--------|--------|
| 上电分辨率 | 640×480 | 640×480 | 800×640 |
| 翻转 | 软件 RGB565 | 软件 RGB565 | 软件 RGB565 |
| JPEG `pixel_reverse` | true | false | false |
| 白平衡 | IPA AWB | IPA AWB | V4L2 增益 1.0/1.0 |

流/照片 JPEG 质量：**40 / 80**。

## 快速开始

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

| API | 说明 |
|-----|------|
| `begin()` / `end()` / `isStarted()` | 生命周期 |
| `setFlip(h, v)` / `getFlip(*h, *v)` | 软件左右/上下翻转（不重载摄像头） |
| `frameSize()` / `frameSizeCount()` / `frameSizeName()` / `listSupportedFrameSizes` | 固定档位只读 |
| `setFrameSize(index)` / `reloadCamera()` | 可选；当前每传感器仅一档 |
| `width()` / `height()` / `sensorName()` / `sensorId()` | 状态 |
| `encodeJpeg(photo, …)` / `captureFrame()` / `allocJpegBuf` … | 采集 |

## 示例

| 示例 | 内容 |
|------|------|
| `01_ConfigAPI` | 探测 + 固定分辨率只读 + 软件 flip + 周期 JPEG |
| `02_WebCamera` | Wi-Fi MJPEG + 软件 `setFlip`（`camera_web_page.h` + `camera_web_server.*`） |
