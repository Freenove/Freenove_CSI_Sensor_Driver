/*
* SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
*
* SPDX-License-Identifier: ESPRESSIF MIT
*/

#include <string.h>
#include "esp_ipa.h"

typedef struct esp_video_ipa_index {
    const char *name;
    const char *description;
    const esp_ipa_config_t *ipa_config;
} esp_video_ipa_index_t;

static const esp_ipa_adn_bf_t s_ipa_adn_bf_OV5647_0_config[] = {
    {
        .gain = 1000,
        .bf = {
            .level = 5,
            .matrix = {
                {1, 2, 1},
                {2, 4, 2},
                {1, 2, 1}
            }
        }
    },
};

static const esp_ipa_adn_config_t s_ipa_adn_OV5647_0_config = {
    .bf_table = s_ipa_adn_bf_OV5647_0_config,
    .bf_table_size = ARRAY_SIZE(s_ipa_adn_bf_OV5647_0_config),
};

static const esp_ipa_aen_gamma_unit_t s_esp_ipa_aen_gamma_OV5647_0_table[] = {
    {
        .luma = 71.1,
        .gamma = {
            .red = {
                .x = { 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 255,  },
                .y = { 36, 60, 80, 98, 116, 132, 147, 162, 176, 190, 204, 217, 230, 243, 255, 255,  }
            },
            .green = {
                .x = { 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 255,  },
                .y = { 36, 60, 80, 98, 116, 132, 147, 162, 176, 190, 204, 217, 230, 243, 255, 255,  }
            },
            .blue = {
                .x = { 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 255,  },
                .y = { 36, 60, 80, 98, 116, 132, 147, 162, 176, 190, 204, 217, 230, 243, 255, 255,  }
            }
        }
    },
};

static const esp_ipa_aen_gamma_config_t s_ipa_aen_gamma_OV5647_0_config = {
    .model = 0,
    .luma_env = "ae.luma.avg",
    .luma_min_step = 16.0,
    .gamma_table = s_esp_ipa_aen_gamma_OV5647_0_table,
    .gamma_table_size = 1,
};

static const esp_ipa_aen_sharpen_t s_ipa_aen_sharpen_OV5647_0_config[] = {
    {
        .gain = 1000,
        .sharpen = {
            .h_thresh = 56,
            .l_thresh = 10,
            .h_coeff = 0.425,
            .m_coeff = 0.625,
            .matrix = {
                {1, 2, 1},
                {2, 2, 2},
                {1, 2, 1}
            }
        }
    },
};

static const esp_ipa_aen_con_t s_ipa_aen_con_OV5647_0_config[] = {
    {
        .gain = 1000,
        .contrast = 134
    },
};

static const esp_ipa_aen_config_t s_ipa_aen_OV5647_0_config = {
    .gamma = &s_ipa_aen_gamma_OV5647_0_config,
    .sharpen_table = s_ipa_aen_sharpen_OV5647_0_config,
    .sharpen_table_size = ARRAY_SIZE(s_ipa_aen_sharpen_OV5647_0_config),
    .con_table = s_ipa_aen_con_OV5647_0_config,
    .con_table_size = ARRAY_SIZE(s_ipa_aen_con_OV5647_0_config),
};

static const esp_ipa_ian_luma_ae_config_t s_esp_ipa_ian_luma_ae_OV5647_0_config = {                 
    .weight = {
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    },
};

static const esp_ipa_ian_luma_config_t s_esp_ipa_ian_luma_OV5647_0_config = {
    .ae = &s_esp_ipa_ian_luma_ae_OV5647_0_config,
};

static const esp_ipa_ian_config_t s_ipa_ian_OV5647_0_config = {
    .luma = &s_esp_ipa_ian_luma_OV5647_0_config,
};

static const esp_ipa_acc_sat_t s_ipa_acc_sat_OV5647_0_config[] = {
    {
        .color_temp = 0,
        .saturation = 128
    },
};

static const esp_ipa_acc_ccm_unit_t s_esp_ipa_acc_ccm_OV5647_0_table[] = {
    {
        .color_temp = 0,
        .ccm = {
            .matrix = {
                { 2.0, -0.5459, -0.4541 },
                { -0.4751, 1.7696, -0.2945 },
                { -0.2002, -0.7998, 2.0 }
            }
        }
    },
};

static const esp_ipa_acc_ccm_config_t s_esp_ipa_acc_ccm_OV5647_0_config = {
    .model = 0,
    .luma_env = "ae.luma.avg",
    .luma_low_threshold = 28,
    .luma_low_ccm = {
        .matrix = {
            { 1.0, 0.0, 0.0 },
            { 0.0, 1.0, 0.0 },
            { 0.0, 0.0, 1.0 }
        }
    }
    ,
    .ccm_table = s_esp_ipa_acc_ccm_OV5647_0_table,
    .ccm_table_size = 1,
};

static const esp_ipa_acc_config_t s_ipa_acc_OV5647_0_config = {
    .sat_table = s_ipa_acc_sat_OV5647_0_config,
    .sat_table_size = ARRAY_SIZE(s_ipa_acc_sat_OV5647_0_config),
    .ccm = &s_esp_ipa_acc_ccm_OV5647_0_config,
};

static const esp_ipa_af_config_t s_ipa_af_OV5647_0_config = {
    .windows = {
        [0] = {
            .top_left = {
                .x = 680,
                .y = 300,
            },
            .btm_right = {
                .x = 1069,
                .y = 709,
            },
        },
        [1] = {
            .top_left = {
                .x = 2,
                .y = 2,
            },
            .btm_right = {
                .x = 5,
                .y = 5,
            },
        },
        [2] = {
            .top_left = {
                .x = 2,
                .y = 2,
            },
            .btm_right = {
                .x = 5,
                .y = 5,
            },
        }
    },
    .weight_table = {
        1, 0, 0
    },
    .edge_thresh = 32,
    .max_pos = 500,
    .min_pos = 0,
    .l1_scan_points_num = 10,
    .l2_scan_points_num = 10,
    .definition_high_threshold_ratio = 1.3,
    .definition_low_threshold_ratio = 0.7,
    .luminance_high_threshold_ratio = 1.1,
    .luminance_low_threshold_ratio = 0.9,
    .max_change_time = 2000000,
};

static const char *s_ipa_OV5647_0_names[] = {
    "esp_ipa_adn",
    "esp_ipa_aen",
    "esp_ipa_ian",
    "esp_ipa_acc",
    "esp_ipa_af",
};

static const esp_ipa_config_t s_ipa_OV5647_0_config = {
    .names = s_ipa_OV5647_0_names,
    .nums = ARRAY_SIZE(s_ipa_OV5647_0_names),
    .version = 1,
    .adn = &s_ipa_adn_OV5647_0_config,
    .aen = &s_ipa_aen_OV5647_0_config,
    .ian = &s_ipa_ian_OV5647_0_config,
    .acc = &s_ipa_acc_OV5647_0_config,
    .af = &s_ipa_af_OV5647_0_config,
};

static const esp_ipa_adn_bf_t s_ipa_adn_bf_IMX219_0_config[] = {
    {
        .gain = 1000,
        .bf = {
            .level = 3,
            .matrix = {
                {1, 2, 1},
                {2, 4, 2},
                {1, 2, 1}
            }
        }
    },
    {
        .gain = 4000,
        .bf = {
            .level = 6,
            .matrix = {
                {1, 2, 1},
                {2, 4, 2},
                {1, 2, 1}
            }
        }
    },
    {
        .gain = 8000,
        .bf = {
            .level = 9,
            .matrix = {
                {1, 2, 1},
                {2, 4, 2},
                {1, 2, 1}
            }
        }
    },
};

static const esp_ipa_adn_config_t s_ipa_adn_IMX219_0_config = {
    .bf_table = s_ipa_adn_bf_IMX219_0_config,
    .bf_table_size = ARRAY_SIZE(s_ipa_adn_bf_IMX219_0_config),
};

static const esp_ipa_agc_config_t s_ipa_agc_IMX219_0_config = {
    .exposure_frame_delay = 2,
    .exposure_adjust_delay = 0,
    .gain_frame_delay = 2,
    .min_gain_step = 0.0001,
    .max_gain = 10.667,
    .inc_gain_ratio = 1,
    .dec_gain_ratio = 1,
    .anti_flicker_mode = ESP_IPA_AGC_ANTI_FLICKER_NONE,
    .ac_freq = 0,
    .gain_only = false,
    .fixed_exposure_time = 0,
    .luma_low = 50,
    .luma_high = 150,
    .luma_target = 100,
    .luma_low_threshold = 30,
    .luma_low_regions = 10,
    .luma_high_threshold = 220,
    .luma_high_regions = 10,
    .luma_weight_table = {
        1, 1, 1, 1, 1, 1, 2, 2, 2, 1, 1, 2, 4, 2, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1, 1, 
    },
    .meter_mode = ESP_IPA_AGC_METER_LOWLIGHT_PRIOR,
    .high_light_prior_config = {
        .use_env_luma = false,
        .luma_high_threshold = 180,
        .luma_low_threshold = 120,
        .weight_offset = 5,
        .luma_offset = 5
    },
    .low_light_prior_config = {
        .use_env_luma = false,
        .luma_high_threshold = 80,
        .luma_low_threshold = 40,
        .weight_offset = 10,
        .luma_offset = 10
    },
};

static const esp_ipa_aen_gamma_unit_t s_esp_ipa_aen_gamma_IMX219_0_table[] = {
    {
        .luma = 71.1,
        .gamma = {
            .red = {
                .x = { 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 255,  },
                .y = { 36, 60, 80, 98, 116, 132, 147, 162, 176, 190, 204, 217, 230, 243, 255, 255,  }
            },
            .green = {
                .x = { 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 255,  },
                .y = { 36, 60, 80, 98, 116, 132, 147, 162, 176, 190, 204, 217, 230, 243, 255, 255,  }
            },
            .blue = {
                .x = { 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 255,  },
                .y = { 36, 60, 80, 98, 116, 132, 147, 162, 176, 190, 204, 217, 230, 243, 255, 255,  }
            }
        }
    },
};

static const esp_ipa_aen_gamma_config_t s_ipa_aen_gamma_IMX219_0_config = {
    .model = 0,
    .luma_env = "ae.luma.avg",
    .luma_min_step = 16.0,
    .gamma_table = s_esp_ipa_aen_gamma_IMX219_0_table,
    .gamma_table_size = 1,
};

static const esp_ipa_aen_sharpen_t s_ipa_aen_sharpen_IMX219_0_config[] = {
    {
        .gain = 1000,
        .sharpen = {
            .h_thresh = 56,
            .l_thresh = 10,
            .h_coeff = 0.4,
            .m_coeff = 0.6,
            .matrix = {
                {1, 2, 1},
                {2, 2, 2},
                {1, 2, 1}
            }
        }
    },
    {
        .gain = 8000,
        .sharpen = {
            .h_thresh = 80,
            .l_thresh = 24,
            .h_coeff = 0.25,
            .m_coeff = 0.35,
            .matrix = {
                {1, 2, 1},
                {2, 2, 2},
                {1, 2, 1}
            }
        }
    },
};

static const esp_ipa_aen_con_t s_ipa_aen_con_IMX219_0_config[] = {
    {
        .gain = 1000,
        .contrast = 130
    },
};

static const esp_ipa_aen_config_t s_ipa_aen_IMX219_0_config = {
    .gamma = &s_ipa_aen_gamma_IMX219_0_config,
    .sharpen_table = s_ipa_aen_sharpen_IMX219_0_config,
    .sharpen_table_size = ARRAY_SIZE(s_ipa_aen_sharpen_IMX219_0_config),
    .con_table = s_ipa_aen_con_IMX219_0_config,
    .con_table_size = ARRAY_SIZE(s_ipa_aen_con_IMX219_0_config),
};

static const esp_ipa_ian_luma_ae_config_t s_esp_ipa_ian_luma_ae_IMX219_0_config = {                 
    .weight = {
        1,1,1,1,1,1,2,2,2,1,1,2,4,2,1,1,2,2,2,1,1,1,1,1,1,
    },
};

static const esp_ipa_ian_luma_config_t s_esp_ipa_ian_luma_IMX219_0_config = {
    .ae = &s_esp_ipa_ian_luma_ae_IMX219_0_config,
};

static const esp_ipa_ian_config_t s_ipa_ian_IMX219_0_config = {
    .luma = &s_esp_ipa_ian_luma_IMX219_0_config,
};

static const esp_ipa_awb_config_t s_ipa_awb_IMX219_0_config = {
    .model = ESP_IPA_AWB_MODEL_0,
    .min_counted = 1000,
    .min_red_gain_step = 0.5,
    .min_blue_gain_step = 0.5,
    .red_gain_scale = 1.000000f,
    .blue_gain_scale = 1.000000f,
    .range = {
        .green_max = 220,
        .green_min = 180,
        .rg_max = 1.2,
        .rg_min = 0.3,
        .bg_max = 1.2,
        .bg_min = 0.3
    },
    .green_luma_env = "ae.luma.avg",
    .green_luma_init = 200,
    .green_luma_step_ratio = 0.3,
    .enable_sub_win = false,
    .min_subwin_wp_counted = 0,
    .min_subwin_participated = 0,
    .subwin_weight = {
        { 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f },
        { 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f },
        { 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f },
        { 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f },
        { 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f },
    },
    .subwin_green_dark = 0,
    .subwin_green_mid = 100,
    .subwin_green_bright = 200,
    .zones = NULL,
    .zones_count = 0,
    .ref_points = NULL,
    .ref_points_count = 0,
    .new_w = 0.300000f,
    .prev_w = 0.700000f,
    .export_ct = false,
    .outlier_rg = 0.000000f,
    .outlier_bg = 0.000000f,
    .zone_hysteresis_ratio = 0.000000f,
    .zone_switch_count = 0,
    .type_counter_max = 20000
};

static const esp_ipa_acc_sat_t s_ipa_acc_sat_IMX219_0_config[] = {
    {
        .color_temp = 0,
        .saturation = 128
    },
};

static const esp_ipa_acc_ccm_unit_t s_esp_ipa_acc_ccm_IMX219_0_table[] = {
    {
        .color_temp = 0,
        .ccm = {
            .matrix = {
                { 1.0, 0.0, 0.0 },
                { 0.0, 1.0, 0.0 },
                { 0.0, 0.0, 1.0 }
            }
        }
    },
};

static const esp_ipa_acc_ccm_config_t s_esp_ipa_acc_ccm_IMX219_0_config = {
    .model = 0,
    .luma_env = "ae.luma.avg",
    .luma_low_threshold = 28,
    .luma_low_ccm = {
        .matrix = {
            { 1.0, 0.0, 0.0 },
            { 0.0, 1.0, 0.0 },
            { 0.0, 0.0, 1.0 }
        }
    }
    ,
    .ccm_table = s_esp_ipa_acc_ccm_IMX219_0_table,
    .ccm_table_size = 1,
};

static const esp_ipa_acc_config_t s_ipa_acc_IMX219_0_config = {
    .sat_table = s_ipa_acc_sat_IMX219_0_config,
    .sat_table_size = ARRAY_SIZE(s_ipa_acc_sat_IMX219_0_config),
    .ccm = &s_esp_ipa_acc_ccm_IMX219_0_config,
};

static const char *s_ipa_IMX219_0_names[] = {
    "esp_ipa_adn",
    "esp_ipa_agc",
    "esp_ipa_aen",
    "esp_ipa_ian",
    "esp_ipa_awb",
    "esp_ipa_acc",
};

static const esp_ipa_config_t s_ipa_IMX219_0_config = {
    .names = s_ipa_IMX219_0_names,
    .nums = ARRAY_SIZE(s_ipa_IMX219_0_names),
    .version = 1,
    .adn = &s_ipa_adn_IMX219_0_config,
    .agc = &s_ipa_agc_IMX219_0_config,
    .aen = &s_ipa_aen_IMX219_0_config,
    .ian = &s_ipa_ian_IMX219_0_config,
    .awb = &s_ipa_awb_IMX219_0_config,
    .acc = &s_ipa_acc_IMX219_0_config,
};

static const esp_ipa_adn_bf_t s_ipa_adn_bf_IMX708_0_config[] = {
    {
        .gain = 1000,
        .bf = {
            .level = 3,
            .matrix = {
                {1, 2, 1},
                {2, 4, 2},
                {1, 2, 1}
            }
        }
    },
    {
        .gain = 4000,
        .bf = {
            .level = 6,
            .matrix = {
                {1, 2, 1},
                {2, 4, 2},
                {1, 2, 1}
            }
        }
    },
    {
        .gain = 8000,
        .bf = {
            .level = 8,
            .matrix = {
                {1, 2, 1},
                {2, 4, 2},
                {1, 2, 1}
            }
        }
    },
};

static const esp_ipa_adn_config_t s_ipa_adn_IMX708_0_config = {
    .bf_table = s_ipa_adn_bf_IMX708_0_config,
    .bf_table_size = ARRAY_SIZE(s_ipa_adn_bf_IMX708_0_config),
};

static const esp_ipa_agc_config_t s_ipa_agc_IMX708_0_config = {
    .exposure_frame_delay = 2,
    .exposure_adjust_delay = 0,
    .gain_frame_delay = 2,
    .min_gain_step = 0.0001,
    .max_gain = 16,
    .inc_gain_ratio = 1,
    .dec_gain_ratio = 1,
    .anti_flicker_mode = ESP_IPA_AGC_ANTI_FLICKER_NONE,
    .ac_freq = 0,
    .gain_only = false,
    .fixed_exposure_time = 0,
    .luma_low = 50,
    .luma_high = 150,
    .luma_target = 100,
    .luma_low_threshold = 30,
    .luma_low_regions = 10,
    .luma_high_threshold = 220,
    .luma_high_regions = 10,
    .luma_weight_table = {
        1, 1, 1, 1, 1, 1, 2, 2, 2, 1, 1, 2, 4, 2, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1, 1, 
    },
    .meter_mode = ESP_IPA_AGC_METER_LOWLIGHT_PRIOR,
    .high_light_prior_config = {
        .use_env_luma = false,
        .luma_high_threshold = 180,
        .luma_low_threshold = 120,
        .weight_offset = 5,
        .luma_offset = 5
    },
    .low_light_prior_config = {
        .use_env_luma = false,
        .luma_high_threshold = 80,
        .luma_low_threshold = 40,
        .weight_offset = 10,
        .luma_offset = 10
    },
};

static const esp_ipa_aen_gamma_unit_t s_esp_ipa_aen_gamma_IMX708_0_table[] = {
    {
        .luma = 71.1,
        .gamma = {
            .red = {
                .x = { 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 255,  },
                .y = { 36, 60, 80, 98, 116, 132, 147, 162, 176, 190, 204, 217, 230, 243, 255, 255,  }
            },
            .green = {
                .x = { 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 255,  },
                .y = { 36, 60, 80, 98, 116, 132, 147, 162, 176, 190, 204, 217, 230, 243, 255, 255,  }
            },
            .blue = {
                .x = { 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 255,  },
                .y = { 36, 60, 80, 98, 116, 132, 147, 162, 176, 190, 204, 217, 230, 243, 255, 255,  }
            }
        }
    },
};

static const esp_ipa_aen_gamma_config_t s_ipa_aen_gamma_IMX708_0_config = {
    .model = 0,
    .luma_env = "ae.luma.avg",
    .luma_min_step = 16.0,
    .gamma_table = s_esp_ipa_aen_gamma_IMX708_0_table,
    .gamma_table_size = 1,
};

static const esp_ipa_aen_sharpen_t s_ipa_aen_sharpen_IMX708_0_config[] = {
    {
        .gain = 1000,
        .sharpen = {
            .h_thresh = 56,
            .l_thresh = 10,
            .h_coeff = 0.4,
            .m_coeff = 0.6,
            .matrix = {
                {1, 2, 1},
                {2, 2, 2},
                {1, 2, 1}
            }
        }
    },
    {
        .gain = 8000,
        .sharpen = {
            .h_thresh = 80,
            .l_thresh = 24,
            .h_coeff = 0.25,
            .m_coeff = 0.35,
            .matrix = {
                {1, 2, 1},
                {2, 2, 2},
                {1, 2, 1}
            }
        }
    },
};

static const esp_ipa_aen_con_t s_ipa_aen_con_IMX708_0_config[] = {
    {
        .gain = 1000,
        .contrast = 130
    },
};

static const esp_ipa_aen_config_t s_ipa_aen_IMX708_0_config = {
    .gamma = &s_ipa_aen_gamma_IMX708_0_config,
    .sharpen_table = s_ipa_aen_sharpen_IMX708_0_config,
    .sharpen_table_size = ARRAY_SIZE(s_ipa_aen_sharpen_IMX708_0_config),
    .con_table = s_ipa_aen_con_IMX708_0_config,
    .con_table_size = ARRAY_SIZE(s_ipa_aen_con_IMX708_0_config),
};

static const esp_ipa_ian_luma_ae_config_t s_esp_ipa_ian_luma_ae_IMX708_0_config = {                 
    .weight = {
        1,1,1,1,1,1,2,2,2,1,1,2,4,2,1,1,2,2,2,1,1,1,1,1,1,
    },
};

static const esp_ipa_ian_luma_config_t s_esp_ipa_ian_luma_IMX708_0_config = {
    .ae = &s_esp_ipa_ian_luma_ae_IMX708_0_config,
};

static const esp_ipa_ian_config_t s_ipa_ian_IMX708_0_config = {
    .luma = &s_esp_ipa_ian_luma_IMX708_0_config,
};

static const esp_ipa_awb_config_t s_ipa_awb_IMX708_0_config = {
    .model = ESP_IPA_AWB_MODEL_0,
    .min_counted = 1000,
    .min_red_gain_step = 0.5,
    .min_blue_gain_step = 0.5,
    .red_gain_scale = 1.000000f,
    .blue_gain_scale = 1.000000f,
    .range = {
        .green_max = 220,
        .green_min = 180,
        .rg_max = 0.8899,
        .rg_min = 0.504,
        .bg_max = 0.7822,
        .bg_min = 0.4838
    },
    .green_luma_env = "ae.luma.avg",
    .green_luma_init = 200,
    .green_luma_step_ratio = 0.3,
    .enable_sub_win = false,
    .min_subwin_wp_counted = 0,
    .min_subwin_participated = 0,
    .subwin_weight = {
        { 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f },
        { 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f },
        { 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f },
        { 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f },
        { 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f },
    },
    .subwin_green_dark = 0,
    .subwin_green_mid = 100,
    .subwin_green_bright = 200,
    .zones = NULL,
    .zones_count = 0,
    .ref_points = NULL,
    .ref_points_count = 0,
    .new_w = 0.300000f,
    .prev_w = 0.700000f,
    .export_ct = false,
    .outlier_rg = 0.000000f,
    .outlier_bg = 0.000000f,
    .zone_hysteresis_ratio = 0.000000f,
    .zone_switch_count = 0,
    .type_counter_max = 20000
};

static const esp_ipa_af_config_t s_ipa_af_IMX708_0_config = {
    .windows = {
        [0] = {
            .top_left = {
                .x = 240,
                .y = 340,
            },
            .btm_right = {
                .x = 719,
                .y = 739,
            },
        },
        [1] = {
            .top_left = {
                .x = 720,
                .y = 340,
            },
            .btm_right = {
                .x = 1199,
                .y = 739,
            },
        },
        [2] = {
            .top_left = {
                .x = 1200,
                .y = 340,
            },
            .btm_right = {
                .x = 1679,
                .y = 739,
            },
        }
    },
    .weight_table = {
        1, 2, 1
    },
    .edge_thresh = 10,
    .max_pos = 960,
    .min_pos = 448,
    .l1_scan_points_num = 10,
    .l2_scan_points_num = 10,
    .definition_high_threshold_ratio = 1.5,
    .definition_low_threshold_ratio = 0.5,
    .luminance_high_threshold_ratio = 1.5,
    .luminance_low_threshold_ratio = 0.5,
    .max_change_time = 10000000,
};

static const esp_ipa_acc_sat_t s_ipa_acc_sat_IMX708_0_config[] = {
    {
        .color_temp = 0,
        .saturation = 128
    },
};

static const esp_ipa_acc_ccm_unit_t s_esp_ipa_acc_ccm_IMX708_0_table[] = {
    {
        .color_temp = 0,
        .ccm = {
            .matrix = {
                { 1.6, -0.4, -0.2 },
                { -0.3, 1.55, -0.25 },
                { -0.15, -0.55, 1.7 }
            }
        }
    },
};

static const esp_ipa_acc_ccm_config_t s_esp_ipa_acc_ccm_IMX708_0_config = {
    .model = 0,
    .luma_env = "ae.luma.avg",
    .luma_low_threshold = 28,
    .luma_low_ccm = {
        .matrix = {
            { 1.0, 0.0, 0.0 },
            { 0.0, 1.0, 0.0 },
            { 0.0, 0.0, 1.0 }
        }
    }
    ,
    .ccm_table = s_esp_ipa_acc_ccm_IMX708_0_table,
    .ccm_table_size = 1,
};

static const esp_ipa_acc_config_t s_ipa_acc_IMX708_0_config = {
    .sat_table = s_ipa_acc_sat_IMX708_0_config,
    .sat_table_size = ARRAY_SIZE(s_ipa_acc_sat_IMX708_0_config),
    .ccm = &s_esp_ipa_acc_ccm_IMX708_0_config,
};

/*
 * IPA tables aligned with 13_WEB_CAMERA imx708_default.json / generated
 * esp_video_ipa_config.c. AF unit omitted here: that example also links
 * DW9807 VCM; this Arduino library does not.
 */
static const char *s_ipa_IMX708_0_names[] = {
    "esp_ipa_adn",
    "esp_ipa_agc",
    "esp_ipa_aen",
    "esp_ipa_ian",
    "esp_ipa_awb",
    "esp_ipa_acc",
};

static const esp_ipa_config_t s_ipa_IMX708_0_config = {
    .names = s_ipa_IMX708_0_names,
    .nums = ARRAY_SIZE(s_ipa_IMX708_0_names),
    .version = 1,
    .adn = &s_ipa_adn_IMX708_0_config,
    .agc = &s_ipa_agc_IMX708_0_config,
    .aen = &s_ipa_aen_IMX708_0_config,
    .ian = &s_ipa_ian_IMX708_0_config,
    .awb = &s_ipa_awb_IMX708_0_config,
    .af = NULL,
    .acc = &s_ipa_acc_IMX708_0_config,
};

static const esp_video_ipa_index_t s_video_ipa_configs[] = {
    {
        .name = "OV5647",
        .ipa_config = &s_ipa_OV5647_0_config
    },
    {
        .name = "IMX219",
        .ipa_config = &s_ipa_IMX219_0_config
    },
    {
        .name = "IMX708",
        .ipa_config = &s_ipa_IMX708_0_config
    },
};

const esp_ipa_config_t *esp_ipa_pipeline_get_config(const char *name)
{
    for (int i = 0; i < ARRAY_SIZE(s_video_ipa_configs); i++) {
        if (!strcmp(name, s_video_ipa_configs[i].name)) {
            return s_video_ipa_configs[i].ipa_config;
        }
    }
    return NULL;
}
const esp_ipa_config_t *esp_ipa_pipeline_enum_configs(const char *sensor_name, int index)
{
    int n = 0;
    if (!sensor_name || index < 0) {
        return NULL;
    }
    for (int i = 0; i < ARRAY_SIZE(s_video_ipa_configs); i++) {
        if (!strcmp(sensor_name, s_video_ipa_configs[i].name)) {
            if (n == index) {
                return s_video_ipa_configs[i].ipa_config;
            }
            n++;
        }
    }
    return NULL;
}

/* Json file: E:/GitHub/FNK0117_ESP32_P4_Board/IDF_P4_Code/13_WEB_CAMERA/components/esp_cam_sensor/sensors/ov5647/cfg/ov5647_default.json E:/GitHub/FNK0117_ESP32_P4_Board/IDF_P4_Code/13_WEB_CAMERA/components/esp_cam_sensor/sensors/imx219/cfg/imx219_default.json E:/GitHub/FNK0117_ESP32_P4_Board/IDF_P4_Code/13_WEB_CAMERA/components/esp_cam_sensor/sensors/imx708/cfg/imx708_default.json */


void freenove_force_ipa_config(void) {}

