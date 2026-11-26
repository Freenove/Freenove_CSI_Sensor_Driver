/*
 * Freenove override of esp_cam_sensor_detect_get_array() for Arduino-esp32 3.3.12.
 *
 * Core ships STATIC_STORE detect table WITHOUT IMX219/IMX708. This object
 * replaces get_array and prepends Freenove-vendored IMX drivers, while keeping
 * the same stock MIPI/DVP entries that the prebuilt core already links.
 */
#include "esp_cam_sensor_detect.h"
#include "esp_cam_motor_detect.h"
#include "sensors/imx219/imx219.h"
#include "sensors/imx708/imx708.h"

/* NOTE: do NOT name a macro param "port" — it rewrites designated `.port =`. */
#define FNK_DETECT_ENTRY(fn, iface, addr) \
    { .detect = (fn), .port = (iface), .sccb_addr = (addr) }

#define FNK_DECLARE_CORE(fn, iface) \
    esp_cam_sensor_device_t *__esp_cam_sensor_detect_fn_##fn##_##iface(void *config)

/* ---- Freenove IMX wrappers (drivers live in this library) ---------------- */
static esp_cam_sensor_device_t *freenove_imx219_detect(void *config)
{
    ((esp_cam_sensor_config_t *)config)->sensor_port = ESP_CAM_SENSOR_MIPI_CSI;
    return imx219_detect((esp_cam_sensor_config_t *)config);
}

static esp_cam_sensor_device_t *freenove_imx708_detect(void *config)
{
    ((esp_cam_sensor_config_t *)config)->sensor_port = ESP_CAM_SENSOR_MIPI_CSI;
    return imx708_detect((esp_cam_sensor_config_t *)config);
}

/* ---- Core-provided detect wrappers (symbols from esp32p4-libs) ----------- */
FNK_DECLARE_CORE(arducam_imx500_detect, ESP_CAM_SENSOR_MIPI_CSI);
FNK_DECLARE_CORE(bf3925_detect, ESP_CAM_SENSOR_DVP);
FNK_DECLARE_CORE(bf3a03_detect, ESP_CAM_SENSOR_DVP);
FNK_DECLARE_CORE(bf20a6_detect, ESP_CAM_SENSOR_DVP);
FNK_DECLARE_CORE(bf3045_detect, ESP_CAM_SENSOR_DVP);
FNK_DECLARE_CORE(gc0308_detect, ESP_CAM_SENSOR_DVP);
FNK_DECLARE_CORE(gc2145_detect, ESP_CAM_SENSOR_MIPI_CSI);
FNK_DECLARE_CORE(mira220_detect, ESP_CAM_SENSOR_MIPI_CSI);
FNK_DECLARE_CORE(mt9d111_detect, ESP_CAM_SENSOR_DVP);
FNK_DECLARE_CORE(os02n10_detect, ESP_CAM_SENSOR_MIPI_CSI);
FNK_DECLARE_CORE(os04c10_detect, ESP_CAM_SENSOR_MIPI_CSI);
FNK_DECLARE_CORE(ov2640_detect, ESP_CAM_SENSOR_DVP);
FNK_DECLARE_CORE(ov2710_detect, ESP_CAM_SENSOR_MIPI_CSI);
FNK_DECLARE_CORE(ov3660_detect, ESP_CAM_SENSOR_DVP);
FNK_DECLARE_CORE(ov5640_detect, ESP_CAM_SENSOR_MIPI_CSI);
FNK_DECLARE_CORE(ov5645_detect, ESP_CAM_SENSOR_MIPI_CSI);
FNK_DECLARE_CORE(ov5647_detect, ESP_CAM_SENSOR_MIPI_CSI);
FNK_DECLARE_CORE(ov9281_detect, ESP_CAM_SENSOR_MIPI_CSI);
FNK_DECLARE_CORE(sc030iot_detect, ESP_CAM_SENSOR_DVP);
FNK_DECLARE_CORE(sc035hgs_detect, ESP_CAM_SENSOR_MIPI_CSI);
FNK_DECLARE_CORE(sc101iot_detect, ESP_CAM_SENSOR_DVP);
FNK_DECLARE_CORE(sc202cs_detect, ESP_CAM_SENSOR_MIPI_CSI);
FNK_DECLARE_CORE(sc2336_detect, ESP_CAM_SENSOR_MIPI_CSI);
FNK_DECLARE_CORE(sti2250_detect, ESP_CAM_SENSOR_MIPI_CSI);
FNK_DECLARE_CORE(pivariety_detect, ESP_CAM_SENSOR_MIPI_CSI);

static const esp_cam_sensor_detect_fn_t s_freenove_detect_table[] = {
    /* RPi modules first; IMX708 before IMX219 (same bring-up order as 13_WEB_CAMERA usage). */
    FNK_DETECT_ENTRY(freenove_imx708_detect, ESP_CAM_SENSOR_MIPI_CSI, IMX708_SCCB_ADDR),
    FNK_DETECT_ENTRY(freenove_imx219_detect, ESP_CAM_SENSOR_MIPI_CSI, IMX219_SCCB_ADDR),

    /* Stock MIPI (Arduino esp32p4-libs 3.3.12) */
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_arducam_imx500_detect_ESP_CAM_SENSOR_MIPI_CSI, ESP_CAM_SENSOR_MIPI_CSI, 0x0c),
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_gc2145_detect_ESP_CAM_SENSOR_MIPI_CSI, ESP_CAM_SENSOR_MIPI_CSI, 0x3C),
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_mira220_detect_ESP_CAM_SENSOR_MIPI_CSI, ESP_CAM_SENSOR_MIPI_CSI, 0x54),
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_os02n10_detect_ESP_CAM_SENSOR_MIPI_CSI, ESP_CAM_SENSOR_MIPI_CSI, 0x3c),
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_os04c10_detect_ESP_CAM_SENSOR_MIPI_CSI, ESP_CAM_SENSOR_MIPI_CSI, 0x36),
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_ov2710_detect_ESP_CAM_SENSOR_MIPI_CSI, ESP_CAM_SENSOR_MIPI_CSI, 0x36),
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_ov5640_detect_ESP_CAM_SENSOR_MIPI_CSI, ESP_CAM_SENSOR_MIPI_CSI, 0x3C),
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_ov5645_detect_ESP_CAM_SENSOR_MIPI_CSI, ESP_CAM_SENSOR_MIPI_CSI, 0x3C),
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_ov5647_detect_ESP_CAM_SENSOR_MIPI_CSI, ESP_CAM_SENSOR_MIPI_CSI, 0x36),
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_ov9281_detect_ESP_CAM_SENSOR_MIPI_CSI, ESP_CAM_SENSOR_MIPI_CSI, 0xe0),
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_sc035hgs_detect_ESP_CAM_SENSOR_MIPI_CSI, ESP_CAM_SENSOR_MIPI_CSI, 0x30),
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_sc202cs_detect_ESP_CAM_SENSOR_MIPI_CSI, ESP_CAM_SENSOR_MIPI_CSI, 0x36),
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_sc2336_detect_ESP_CAM_SENSOR_MIPI_CSI, ESP_CAM_SENSOR_MIPI_CSI, 0x30),
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_sti2250_detect_ESP_CAM_SENSOR_MIPI_CSI, ESP_CAM_SENSOR_MIPI_CSI, 0x37),
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_pivariety_detect_ESP_CAM_SENSOR_MIPI_CSI, ESP_CAM_SENSOR_MIPI_CSI, 0x0c),

    /* Stock DVP */
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_bf3925_detect_ESP_CAM_SENSOR_DVP, ESP_CAM_SENSOR_DVP, 0x6E),
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_bf3a03_detect_ESP_CAM_SENSOR_DVP, ESP_CAM_SENSOR_DVP, 0x6E),
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_bf20a6_detect_ESP_CAM_SENSOR_DVP, ESP_CAM_SENSOR_DVP, 0x6E),
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_bf3045_detect_ESP_CAM_SENSOR_DVP, ESP_CAM_SENSOR_DVP, 0x6E),
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_gc0308_detect_ESP_CAM_SENSOR_DVP, ESP_CAM_SENSOR_DVP, 0x21),
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_mt9d111_detect_ESP_CAM_SENSOR_DVP, ESP_CAM_SENSOR_DVP, 0x48),
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_ov2640_detect_ESP_CAM_SENSOR_DVP, ESP_CAM_SENSOR_DVP, 0x30),
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_ov3660_detect_ESP_CAM_SENSOR_DVP, ESP_CAM_SENSOR_DVP, 0x3c),
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_sc030iot_detect_ESP_CAM_SENSOR_DVP, ESP_CAM_SENSOR_DVP, 0x68),
    FNK_DETECT_ENTRY(__esp_cam_sensor_detect_fn_sc101iot_detect_ESP_CAM_SENSOR_DVP, ESP_CAM_SENSOR_DVP, 0x68),
};

void esp_cam_sensor_detect_get_array(esp_cam_sensor_detect_fn_t **array_start_ptr,
                                    esp_cam_sensor_detect_fn_t **array_end_ptr)
{
    const int n = (int)(sizeof(s_freenove_detect_table) / sizeof(s_freenove_detect_table[0]));
    *array_start_ptr = (esp_cam_sensor_detect_fn_t *)s_freenove_detect_table;
    *array_end_ptr = (esp_cam_sensor_detect_fn_t *)(s_freenove_detect_table + n);
}

/*
 * No VCM in this library (DW9807 out of scope). Empty motor table avoids a
 * post-detect hang while esp_video probes I2C 0x0c for Cam Module 3 AF.
 */
void esp_cam_motor_detect_get_array(esp_cam_motor_detect_fn_t **array_start_ptr,
                                    esp_cam_motor_detect_fn_t **array_end_ptr)
{
    static esp_cam_motor_detect_fn_t empty;
    *array_start_ptr = &empty;
    *array_end_ptr = &empty;
}

void freenove_force_cam_detect_override(void) {}

