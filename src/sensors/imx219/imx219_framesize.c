#include "imx219_framesize.h"
#include "imx219.h"

/* Public index → imx219_format_by_index() slot (2 = 640x480, IDF default). */
static const size_t kImx219FramesizeFmt[] = {
  2, /* 640x480 */
};

size_t imx219_framesize_count(void)
{
  return sizeof(kImx219FramesizeFmt) / sizeof(kImx219FramesizeFmt[0]);
}

bool imx219_framesize_get(size_t index, uint16_t *width, uint16_t *height)
{
  if (index >= imx219_framesize_count() || width == NULL || height == NULL) {
    return false;
  }
  const esp_cam_sensor_format_t *fmt = imx219_format_by_index(kImx219FramesizeFmt[index]);
  if (fmt == NULL) {
    return false;
  }
  *width = fmt->width;
  *height = fmt->height;
  return true;
}

const char *imx219_framesize_name(size_t index)
{
  if (index >= imx219_framesize_count()) {
    return "unknown";
  }
  const esp_cam_sensor_format_t *fmt = imx219_format_by_index(kImx219FramesizeFmt[index]);
  if (fmt == NULL || fmt->name == NULL) {
    return "unknown";
  }
  return fmt->name;
}
