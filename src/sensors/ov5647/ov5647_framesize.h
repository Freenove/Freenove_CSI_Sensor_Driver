/*
 * OV5647 register modes. Freenove product: only 800x640.
 */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  OV5647_FRAMESIZE_800x640 = 0,
} ov5647_framesize_index_t;

size_t ov5647_framesize_count(void);
bool ov5647_framesize_get(size_t index, uint16_t *width, uint16_t *height);
const char *ov5647_framesize_name(size_t index);

#ifdef __cplusplus
}
#endif
