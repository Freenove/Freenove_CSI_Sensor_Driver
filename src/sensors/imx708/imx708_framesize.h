/*
 * IMX708 register modes. Freenove product: IDF default 640x480 only.
 */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  IMX708_FRAMESIZE_640x480 = 0,
} imx708_framesize_index_t;

size_t imx708_framesize_count(void);
bool imx708_framesize_get(size_t index, uint16_t *width, uint16_t *height);
const char *imx708_framesize_name(size_t index);

#ifdef __cplusplus
}
#endif
