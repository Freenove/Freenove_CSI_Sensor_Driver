#include "ov5647_framesize.h"
#include "ov5647_formats.h"

/* Public order maps to ov5647_format_by_index() slots. */
static const size_t kOv5647FramesizeFmt[] = {
  0, /* 800x640 — only supported mode */
};

size_t ov5647_framesize_count(void)
{
  return sizeof(kOv5647FramesizeFmt) / sizeof(kOv5647FramesizeFmt[0]);
}

bool ov5647_framesize_get(size_t index, uint16_t *width, uint16_t *height)
{
  if (index >= ov5647_framesize_count() || width == NULL || height == NULL) {
    return false;
  }
  const esp_cam_sensor_format_t *fmt = ov5647_format_by_index(kOv5647FramesizeFmt[index]);
  if (fmt == NULL) {
    return false;
  }
  *width = fmt->width;
  *height = fmt->height;
  return true;
}

const char *ov5647_framesize_name(size_t index)
{
  if (index >= ov5647_framesize_count()) {
    return "unknown";
  }
  const esp_cam_sensor_format_t *fmt = ov5647_format_by_index(kOv5647FramesizeFmt[index]);
  if (fmt == NULL || fmt->name == NULL) {
    return "unknown";
  }
  return fmt->name;
}
