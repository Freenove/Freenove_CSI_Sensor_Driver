#include "imx708_framesize.h"
#include "imx708.h"

/* Public index → imx708_format_by_index() slot (4 = 640x480, IDF default). */
static const size_t kImx708FramesizeFmt[] = {
  4, /* 640x480 */
};

size_t imx708_framesize_count(void)
{
  return sizeof(kImx708FramesizeFmt) / sizeof(kImx708FramesizeFmt[0]);
}

bool imx708_framesize_get(size_t index, uint16_t *width, uint16_t *height)
{
  if (index >= imx708_framesize_count() || width == NULL || height == NULL) {
    return false;
  }
  const esp_cam_sensor_format_t *fmt = imx708_format_by_index(kImx708FramesizeFmt[index]);
  if (fmt == NULL) {
    return false;
  }
  *width = fmt->width;
  *height = fmt->height;
  return true;
}

const char *imx708_framesize_name(size_t index)
{
  if (index >= imx708_framesize_count()) {
    return "unknown";
  }
  const esp_cam_sensor_format_t *fmt = imx708_format_by_index(kImx708FramesizeFmt[index]);
  if (fmt == NULL || fmt->name == NULL) {
    return "unknown";
  }
  return fmt->name;
}
