/***************************************************************************
 * Copyright (c) 2010-2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ***************************************************************************/

#include <stddef.h>
#include <string.h>
#include <assert.h>

#include "aec.h"
#include "awb.h"
#include "af.h"
#include "asd.h"

#include "eztune.h"
#include "eztune_logs.h"
#include "eztune_items_diag.h"
#include "eztune_diagnostics.h"
#include "eztune_vfe_diagnostics.h"

//////////////////////////////////////////////////////////
// local (static) function delarations & forward declares


//////////////////////////////////////////////////////////
// function implemenataions

void eztune_set_item_data(
    eztune_item_t *item_ptr,
    char *name,
    eztune_item_format_t format,
    eztune_item_type_t type,
    eztune_item_reg_flag_t reg_flag,
    uint32_t offset,
    uint16_t entry_count,
    uint32_t step_size)
{
    strlcpy(item_ptr->name, name, EZTUNE_FORMAT_MAX);

    item_ptr->format = format;
    item_ptr->type = type;
    item_ptr->reg_flag = reg_flag;
    item_ptr->offset = offset;
    item_ptr->entry_count = entry_count;
    item_ptr->step_size = step_size;
}

void eztune_diag_set_item_value(
    eztune_t *ezctrl,
    eztune_item_t *item,
    eztune_set_val_t *item_data)
{
    int32_t rc = 0;
    static uint8_t aec_force_lock = 0, aec_enable = 1;

    EZ_ASSERT(item->id >= EZT_PARMS_DIAG_HEADER_MARKER && item->id < EZT_PARMS_DIAG_MAX,
        "Invalid diag field range");

    if (item->id == EZT_PARMS_ROLLOFF_ENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_ROLLOFF, SET_ENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_ROLLOFF_CONTROLENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_ROLLOFF, SET_CONTROLENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_5X5ASF_ENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_ASF5X5, SET_ENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_5X5ASF_CONTROLENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_ASF5X5, SET_CONTROLENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_7X7ASF_ENABLE) {
        ezctrl->tuning_set_pp(PP_MODULE_ASF, SET_ENABLE,
            atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_7X7ASF_CONTROLENABLE) {
        ezctrl->tuning_set_pp(PP_MODULE_ASF, SET_CONTROLENABLE,
                      atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_9X9ASF_ENABLE) {
        ezctrl->tuning_set_pp(PP_MODULE_ASF, SET_ENABLE,
                      atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_9X9ASF_CONTROLENABLE) {
        ezctrl->tuning_set_pp(PP_MODULE_ASF, SET_CONTROLENABLE,
                      atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_CHROMASUPP_ENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_CHROMASUPPRESSION, SET_ENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_CHROMASUPP_CONTROLENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_CHROMASUPPRESSION,
                       SET_CONTROLENABLE, atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_MCE_ENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_MCE, SET_ENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_MCE_CONTROLENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_MCE, SET_CONTROLENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_WAVELET_ENABLE) {
        ezctrl->tuning_set_pp(PP_MODULE_WNR, SET_ENABLE,
                      atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_WAVELET_CONTROLENABLE) {
        ezctrl->tuning_set_pp(PP_MODULE_WNR, SET_CONTROLENABLE,
                      atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_ABF_ENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_ABF, SET_ENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_ABF_CONTROLENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_ABF, SET_CONTROLENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_BPC_ENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_BPC, SET_ENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_BPC_CONTROLENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_BPC, SET_CONTROLENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_BCC_ENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_BCC, SET_ENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_BCC_CONTROLENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_BCC, SET_CONTROLENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_DEMOSAIC_ENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_DEMOSAIC, SET_ENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_DEMOSAIC_CONTROLENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_DEMOSAIC, SET_CONTROLENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_CLFILTER_ENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_CLFILTER, SET_ENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_CLFILTER_CONTROLENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_CLFILTER, SET_CONTROLENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_LINEAR_ENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_LINEARIZATION, SET_ENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_LINEAR_CONTROLENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_LINEARIZATION, SET_CONTROLENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_SCE_DENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_SCE, SET_ENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_SCE_CONTROLENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_SCE, SET_CONTROLENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_DEMUXCHGAIN_ENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_DEMUX, SET_ENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_DEMUXCHGAIN_CONTROLENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_DEMUX, SET_CONTROLENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_GIC_DIAG_ENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_GIC, SET_ENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_GIC_DIAG_CONTROLENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_GIC, SET_CONTROLENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_GTM_DIAG_ENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_GTM, SET_ENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_GTM_DIAG_CONTROLENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_GTM, SET_CONTROLENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_PEDESTAL_DIAG_ENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_PEDESTAL, SET_ENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_PEDESTAL_DIAG_CONTROLENABLE) {
        ezctrl->tuning_set_vfe(VFE_MODULE_PEDESTAL, SET_CONTROLENABLE,
                       atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_AWB_MODE) {
        ezctrl->tuning_set_3a(EZ_AWB_MODE, atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_AWB_ENABLE) {
        EZLOGV("enable : EZT_PARMS_AWB_ENABLE value = %s", item_data->value_string);
        ezctrl->tuning_set_3a(EZ_AWB_ENABLE, atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_AWB_LOCK) {
        EZLOGV("enable : EZT_PARMS_AWB_LOCK value = %s", item_data->value_string);
        ezctrl->tuning_set_3a(EZ_AWB_LOCK, atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_AWB_BAYER_MIXLEDTABLE_INDEXOVERRIDE) {
        EZLOGV("index : EZT_PARMS_AWB_BAYER_MIXLEDTABLE_INDEXOVERRIDE value = %s", item_data->value_string);
        ezctrl->tuning_set_3a(EZ_AWB_FORCE_DUAL_LED_IDX, atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_AEC_ENABLE) {
        EZLOGV("enable : EZT_PARMS_AEC_ENABLE value = %s", item_data->value_string);
        aec_enable = atoi(item_data->value_string);
        ezctrl->tuning_set_3a(EZ_AEC_ENABLE, aec_enable);
    } else if (item->id == EZT_PARMS_AEC_TESTAEC_ENABLE) {
        ezctrl->tuning_set_3a(EZ_AEC_TESTENABLE,
                atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_AEC_LOCK) {
        EZLOGV("enable : EZT_PARMS_AEC_LOCK value = %s", item_data->value_string);
        aec_force_lock = atoi(item_data->value_string);
        ezctrl->tuning_set_3a(EZ_AEC_LOCK, aec_force_lock);
    } else if (item->id == EZT_PARMS_AEC_SNAPSHOT_FORCEEXP) {
        if (!aec_enable)
            ezctrl->tuning_set_3a(EZ_AEC_FORCESNAPEXPOSURE,
                    (atof(item_data->value_string) * Q10));
    } else if (item->id == EZT_PARMS_AEC_PREVIEW_FORCEEXP) {
        if (!aec_enable)
            ezctrl->tuning_set_3a(EZ_AEC_FORCEPREVEXPOSURE,
                    (atof(item_data->value_string) * Q10));
    } else if (item->id == EZT_PARMS_AEC_FORCE_SNAPGAIN) {
        if (!aec_enable)
            ezctrl->tuning_set_3a(EZ_AEC_FORCESNAPGAIN,
                    (atof(item_data->value_string) * Q10));
    } else if (item->id == EZT_PARMS_AEC_FORCE_SNAPLINECOUNT) {
        if (!aec_enable)
            ezctrl->tuning_set_3a(EZ_AEC_FORCESNAPLINECOUNT,
                    atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_AEC_FORCE_PREVGAIN) {
        if (!aec_enable)
            ezctrl->tuning_set_3a(EZ_AEC_FORCEPREVGAIN,
                    (atof(item_data->value_string) * Q10));
    } else if (item->id == EZT_PARMS_AEC_FORCE_PREVLINECOUNT) {
        if (!aec_enable)
            ezctrl->tuning_set_3a(EZ_AEC_FORCEPREVLINECOUNT,
                    atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_AF_ENABLE) {
        ezctrl->tuning_set_3a(EZ_AF_ENABLE, atoi(item_data->value_string));
    } else if (item->id == EZT_PARMS_MISC_FLASHMODE) {
        ezctrl->tuning_post_bus_msg(EZTUNE_BUSMSG_FLASHMODE,
            atoi(item_data->value_string));

    } else if (item->id == EZT_PARMS_MISC_TUNINGDATATIMESTAMP) {
        rc = property_set(EZTUNE_PROP_TUNE_DATE, item_data->value_string);
    } else if (item->id == EZT_PARMS_MISC_TUNINGDATANAME) {
        rc = property_set(EZTUNE_PROP_TUNE_NAME, item_data->value_string);
    } else if (item->id == EZT_PARMS_MISC_TUNINGDATAUPDATEDBY) {
        rc = property_set(EZTUNE_PROP_TUNE_AUTHOR, item_data->value_string);
    } else if (item->id == EZT_PARMS_MISC_PARAMETERRETENTIONENABLE) {
        rc = property_set(EZTUNE_PROP_STICKY_SETTINGS, item_data->value_string);

    } else if (item->id == EZT_PARMS_AFTUNE_TUNING_REGIONSIZE) {
        *(uint16_t *)(&ezctrl->af_driver_ptr->actuator_tuned_params.region_size) =
            atoi(item_data->value_string);
    } else if (item->id == EZT_PARMS_AFTUNE_TUNING_REGIONPARAMS_MACROSTEPBOUND) {
        *(uint16_t *)(&ezctrl->af_driver_ptr->actuator_tuned_params.
                region_params[item_data->table_index].step_bound[0]) =
            atoi(item_data->value_string);
    } else if (item->id == EZT_PARMS_AFTUNE_TUNING_REGIONPARAMS_INFSTEPBOUND) {
        *(uint16_t *)(&ezctrl->af_driver_ptr->actuator_tuned_params.
                region_params[item_data->table_index].step_bound[1]) =
            atoi(item_data->value_string);
    } else if (item->id == EZT_PARMS_AFTUNE_TUNING_INITIALCODE) {
        *(uint16_t *)(&ezctrl->af_driver_ptr->actuator_tuned_params.initial_code) =
            atoi(item_data->value_string);
    } else if (item->id == EZT_PARMS_AFTUNE_TUNING_TEST_DEFFOCUS_ENABLE) {
        uint8_t value = atoi(item_data->value_string);
        if (value == 1)
            ezctrl->tuning_set_focus(ezctrl, EZ_AF_DEFFOCUSTEST_ENABLE, value);
    } else if (item->id == EZT_PARMS_AFTUNE_TUNING_TEST_MOVEFOCUS_ENABLE) {
        uint8_t value = atoi(item_data->value_string);
        if (value == 1)
            ezctrl->tuning_set_focus(ezctrl, EZ_AF_MOVFOCUSTEST_ENABLE, value);
    } else if (item->id == EZT_PARMS_AFTUNE_TUNING_TEST_MOVEFOCUS_DIR) {
        *(uint8_t *)(&ezctrl->af_tuning_ptr->movfocdirection) =
            atoi(item_data->value_string);
    } else if (item->id == EZT_PARMS_AFTUNE_TUNING_TEST_MOVEFOCUS_STEPS) {
        *(uint8_t *)(&ezctrl->af_tuning_ptr->movfocsteps) =
            atoi(item_data->value_string);
    } else {
        EZLOGE("Error: eztune_set_item_value diag not supported (%d %s)",
            item->id, item->name);
    }
}

int eztune_diag_get_item_value(
    eztune_t *ezctrl,
    eztune_item_t *item,
    int32_t table_index,
    char *output_buf)
{
    int rc = 0;
    uint16_t stringsize;

    float    *f_value;
    int32_t  *i32_value;
    uint32_t *u32_value;
    int16_t  *i16_value;
    uint16_t *u16_value;
    char     *ch_value;
    uint8_t curr_entry;
    uint8_t is_chromatix_lite_ae_stats_valid = 0;
    uint8_t is_chromatix_lite_awb_stats_valid = 0;
    uint8_t is_chromatix_lite_af_stats_valid = 0;
    uint8_t is_chromatix_lite_asd_stats_valid = 0;
    uint8_t is_chromatix_lite_pp_stats_valid = 0;
    uint8_t is_chromatix_lite_vfe_stats_valid = 0;
    uint8_t is_chromatix_lite_flash_mode_valid = 0;

    metadata_buffer_t* metadata = ezctrl->metadata;

    static int                       init_done = FALSE;
    static aec_ez_tune_t             ae_stat;
    static awb_output_eztune_data_t  awb_stat;
    static af_output_eztune_data_t   af_stat;
    static asd_ez_tune_t             asd_stat;
    static ez_pp_params_t            pp_stat;
    static vfe_diagnostics_t         vfe_stat;
    static cam_flash_mode_t          flash_stat;
    aec_ez_tune_t                   *ae_params = &ae_stat;
    awb_output_eztune_data_t        *awb_params = &awb_stat;
    af_output_eztune_data_t         *af_params = &af_stat;
    asd_ez_tune_t                   *asd_params = &asd_stat;
    ez_pp_params_t                  *pp_diagnostics = &pp_stat;
    vfe_diagnostics_t               *vfe_diagnostics = &vfe_stat;
    cam_flash_mode_t                *flash_params = &flash_stat;

    EZLOGV("name %s id %d index %d", item->name, item->id, table_index);

    if(!init_done) {
        memset(&ae_stat, 0xff, sizeof(aec_ez_tune_t));
        memset(&awb_stat, 0xff, sizeof(awb_output_eztune_data_t));
        memset(&af_stat, 0xff, sizeof(af_output_eztune_data_t));
        memset(&asd_stat, 0xff, sizeof(asd_ez_tune_t));
        memset(&pp_stat, 0xff, sizeof(ez_pp_params_t));
        memset(&vfe_stat, 0xff, sizeof(vfe_diagnostics_t));
        memset(&flash_stat, 0x0, sizeof(cam_flash_mode_t));
        init_done = TRUE;
    }

    IF_META_AVAILABLE(cam_chromatix_lite_asd_stats_t, p_asd_stats,
            CAM_INTF_META_CHROMATIX_LITE_ASD, metadata) {
        memcpy((void *)&asd_stat, (void*)p_asd_stats, sizeof(asd_ez_tune_t));
        is_chromatix_lite_asd_stats_valid = 1;
    }
    IF_META_AVAILABLE(cam_chromatix_lite_ae_stats_t, p_ae_stats,
            CAM_INTF_META_CHROMATIX_LITE_AE, metadata) {
        memcpy((void *)&ae_stat, (void*)p_ae_stats, sizeof(aec_ez_tune_t));
        is_chromatix_lite_ae_stats_valid = 1;
    }
    IF_META_AVAILABLE(cam_chromatix_lite_awb_stats_t, p_awb_stats,
            CAM_INTF_META_CHROMATIX_LITE_AWB, metadata) {
        memcpy((void *)&awb_stat, (void*)p_awb_stats,
                sizeof(awb_output_eztune_data_t));
        is_chromatix_lite_awb_stats_valid = 1;
    }
    IF_META_AVAILABLE(cam_chromatix_lite_af_stats_t, p_af_stats,
            CAM_INTF_META_CHROMATIX_LITE_AF, metadata) {
        memcpy((void *)&af_stat, (void *)p_af_stats,
                sizeof(af_output_eztune_data_t));
        is_chromatix_lite_af_stats_valid = 1;
    }
    IF_META_AVAILABLE(uint32_t, p_flash_mode,
        CAM_INTF_META_LED_MODE_OVERRIDE, metadata) {
        flash_stat = (cam_flash_mode_t) *p_flash_mode;
        is_chromatix_lite_flash_mode_valid = 1;
    }
    IF_META_AVAILABLE(cam_chromatix_lite_pp_t, p_pp,
            CAM_INTF_META_CHROMATIX_LITE_PP, metadata) {
        memcpy((void *)&pp_stat, (void *)p_pp,
                sizeof(ez_pp_params_t));
        is_chromatix_lite_pp_stats_valid = 1;
    }
    IF_META_AVAILABLE(cam_chromatix_lite_isp_t, p_isp,
            CAM_INTF_META_CHROMATIX_LITE_ISP, metadata) {
        memcpy((void *)&vfe_stat, (void *)p_isp,
                sizeof(vfe_diagnostics_t));
        is_chromatix_lite_vfe_stats_valid = 1;
    }

    EZLOGV("ae_valid %d, awb_valid %d, af_valid %d, asd_valid %d vfe_valid %d, pp_valid %d",
        is_chromatix_lite_ae_stats_valid, is_chromatix_lite_awb_stats_valid,
        is_chromatix_lite_af_stats_valid, is_chromatix_lite_asd_stats_valid,
        is_chromatix_lite_vfe_stats_valid, is_chromatix_lite_pp_stats_valid);

    switch (item->id) {
    case EZT_PARMS_3A: {
        switch (awb_params-> decision) {
            case 0: ch_value = "SUNLIGHT-D65"; break;
            case 1: ch_value = "CLOUDY-D75"; break;
            case 2: ch_value = "INCANDESCENT-A"; break;
            case 3: ch_value = "FLUORESCENT-TL84"; break;
            case 4: ch_value = "FLUORESCENT-CW"; break;
            case 5: ch_value = "HORIZON-H"; break;
            case 6: ch_value = "SUNLIGHT-D50"; break;
            case 7: ch_value = "FLUORESCENT-CUS"; break;
            case 8: ch_value = "NOON"; break;
            case 9: ch_value = "DAYLIGHT-CUS"; break;
            case 10: ch_value = "A-CUS"; break;
            case 11: ch_value = "U30"; break;
            case 12: ch_value = "DAY-LINE1"; break;
            case 13: ch_value = "DAY-LINE2"; break;
            case 14: ch_value = "FINE"; break;
            case 15: ch_value = "A-LINE1"; break;
            case 16: ch_value = "A-LINE2"; break;
            case 17: ch_value = "HYBRID"; break;
            default: ch_value = "BAD"; break;
        }
        stringsize = snprintf(output_buf, 0, "%s%d : %s%d : %s%s",
            "AF=", af_params->peak_location_index,
            "AEC=", ae_params->exposure_index,
            "AWB=", ch_value);
        rc = snprintf(output_buf, stringsize + 1, "%s%d : %s%d : %s%s",
            "AF=", af_params->peak_location_index,
            "AEC=", ae_params->exposure_index,
            "AWB=", ch_value);
        break;
    }

        /* Color Corr ----------------------------------------------------------------
         */
    case EZT_PARMS_COLORCORR_CURRENT_RTOR:
        EZLOGV("EZT_PARMS_COLORCORR_CURRENT_RTOR %d",
                vfe_diagnostics->prev_colorcorr.coef_rtor);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_colorcorr.coef_rtor);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_GTOR:
        EZLOGV("EZT_PARMS_COLORCORR_CURRENT_GTOR %d",
                vfe_diagnostics->prev_colorcorr.coef_gtor);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_colorcorr.coef_gtor);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_BTOR:
        EZLOGV("EZT_PARMS_COLORCORR_CURRENT_BTOR %d",
                vfe_diagnostics->prev_colorcorr.coef_btor);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_colorcorr.coef_btor);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_RTOG:
        EZLOGV("EZT_PARMS_COLORCORR_CURRENT_RTOG %d",
                vfe_diagnostics->prev_colorcorr.coef_rtog);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_colorcorr.coef_rtog);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_GTOG:
        EZLOGV("EZT_PARMS_COLORCORR_CURRENT_GTOG %d",
                vfe_diagnostics->prev_colorcorr.coef_gtog);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_colorcorr.coef_gtog);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_BTOG:
        EZLOGV("EZT_PARMS_COLORCORR_CURRENT_BTOG %d",
                vfe_diagnostics->prev_colorcorr.coef_btog);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_colorcorr.coef_btog);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_RTOB:
        EZLOGV("EZT_PARMS_COLORCORR_CURRENT_RTOB %d",
                vfe_diagnostics->prev_colorcorr.coef_rtob);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_colorcorr.coef_rtob);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_GTOB:
        EZLOGV("EZT_PARMS_COLORCORR_CURRENT_BTOG %d",
                vfe_diagnostics->prev_colorcorr.coef_gtob);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_colorcorr.coef_gtob);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_BTOB:
        EZLOGV("EZT_PARMS_COLORCORR_CURRENT_BTOB %d",
                vfe_diagnostics->prev_colorcorr.coef_btob);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_colorcorr.coef_btob);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_ROFFSET:
        EZLOGV("EZT_PARMS_COLORCORR_CURRENT_ROFFSET %d",
                vfe_diagnostics->prev_colorcorr.roffset);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_colorcorr.roffset);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_GOFFSET:
        EZLOGV("EZT_PARMS_COLORCORR_CURRENT_GOFFSET %d",
                vfe_diagnostics->prev_colorcorr.goffset);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_colorcorr.goffset);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_BOFFSET:
        EZLOGV("EZT_PARMS_COLORCORR_CURRENT_BOFFSET %d",
                vfe_diagnostics->prev_colorcorr.boffset);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_colorcorr.boffset);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_QFACTOR:
        EZLOGV("EZT_PARMS_COLORCORR_CURRENT_QFACTOR %d",
                vfe_diagnostics->prev_colorcorr.coef_qfactor);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_colorcorr.coef_qfactor);
        break;

        /* Color Conv ----------------------------------------------------------------
         */
    case EZT_PARMS_COLORCONV_CURRENT_CBSCALINGNEGATIVE_AM:
        EZLOGV("EZT_PARMS_COLORCONV_CURRENT_CBSCALINGNEGATIVE_AM %d",
                vfe_diagnostics->colorconv.param_am);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->colorconv.param_am);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_CBSCALINGPOSITIVE_AP:
        EZLOGV("EZT_PARMS_COLORCONV_CURRENT_CBSCALINGPOSITIVE_AP %d",
                vfe_diagnostics->colorconv.param_ap);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->colorconv.param_ap);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_DIFFRGTOCBSHEARINGNEGATIVE_BM:
        EZLOGV("EZT_PARMS_COLORCONV_CURRENT_DIFFRGTOCBSHEARINGNEGATIVE_BM %d",
                vfe_diagnostics->colorconv.param_bm);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->colorconv.param_bm);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_DIFFRGTOCBSHEARINGPOSITIVE_BP:
        EZLOGV("EZT_PARMS_COLORCONV_CURRENT_DIFFRGTOCBSHEARINGPOSITIVE_BP %d",
                vfe_diagnostics->colorconv.param_bp);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->colorconv.param_bp);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_CRSCALINGNEGATIVE_CM:
        EZLOGV("EZT_PARMS_COLORCONV_CURRENT_CRSCALINGNEGATIVE_CM %d",
                vfe_diagnostics->colorconv.param_cm);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->colorconv.param_cm);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_CRSCALINGPOSITIVE_CP:
        EZLOGV("EZT_PARMS_COLORCONV_CURRENT_CRSCALINGPOSITIVE_CP %d",
                vfe_diagnostics->colorconv.param_cp);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->colorconv.param_cp);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_DIFFBGTOCRSHEARINGNEGATIVE_DM:
        EZLOGV("EZT_PARMS_COLORCONV_CURRENT_DIFFBGTOCRSHEARINGNEGATIVE_DM %d",
                vfe_diagnostics->colorconv.param_dm);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->colorconv.param_dm);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_DIFFBGTOCRSHEARINGPOSITIVE_DP:
        EZLOGV("EZT_PARMS_COLORCONV_CURRENT_DIFFBGTOCRSHEARINGPOSITIVE_DP %d",
                vfe_diagnostics->colorconv.param_dp);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->colorconv.param_dp);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_CBOFFSET_K_CB:
        EZLOGV("EZT_PARMS_COLORCONV_CURRENT_CBOFFSET_K_CB %d",
                vfe_diagnostics->colorconv.param_kcb);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->colorconv.param_kcb);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_CROFFSET_K_CR:
        EZLOGV("EZT_PARMS_COLORCONV_CURRENT_CROFFSET_K_CR %d",
                vfe_diagnostics->colorconv.param_kcr);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->colorconv.param_kcr);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_RTOY:
        EZLOGV("EZT_PARMS_COLORCONV_CURRENT_RTOY %d",
                vfe_diagnostics->colorconv.param_rtoy);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->colorconv.param_rtoy);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_GTOY:
        EZLOGV("EZT_PARMS_COLORCONV_CURRENT_GTOY %d",
                vfe_diagnostics->colorconv.param_gtoy);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->colorconv.param_gtoy);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_BTOY:
        EZLOGV("EZT_PARMS_COLORCONV_CURRENT_BTOY %d",
                vfe_diagnostics->colorconv.param_btoy);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->colorconv.param_btoy);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_YOFFSET:
        EZLOGV("EZT_PARMS_COLORCONV_CURRENT_YOFFSET %d",
                vfe_diagnostics->colorconv.param_yoffset);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->colorconv.param_yoffset);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_BOFFSET:
        EZLOGV("EZT_PARMS_COLORCONV_CURRENT_BOFFSET %d",
                vfe_diagnostics->colorconv.param_boffset);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->colorconv.param_yoffset);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_ROFFSET:
        EZLOGV("EZT_PARMS_COLORCONV_CURRENT_ROFFSET %d",
                vfe_diagnostics->colorconv.param_roffset);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->colorconv.param_yoffset);
        break;

        /* Black Level ---------------------------------------------------------------
         */
    case EZT_PARMS_BLACKLEVEL_SNAPSHOT_BLACKEVENROWEVENCOL:
        EZLOGV("EZT_PARMS_BLACKLEVEL_SNAPSHOT_BLACKEVENROWEVENCOL %d",
                vfe_diagnostics->snap_blacklevel.evenRevenC);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_blacklevel.evenRevenC);
        break;
    case EZT_PARMS_BLACKLEVEL_SNAPSHOT_BLACKEVENROWODDCOL:
        EZLOGV("EZT_PARMS_BLACKLEVEL_SNAPSHOT_BLACKEVENROWODDCOL %d",
                vfe_diagnostics->snap_blacklevel.evenRoddC);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_blacklevel.evenRoddC);
        break;
    case EZT_PARMS_BLACKLEVEL_SNAPSHOT_BLACKODDROWEVENCOL:
        EZLOGV("EZT_PARMS_BLACKLEVEL_SNAPSHOT_BLACKODDROWEVENCOL %d",
                vfe_diagnostics->snap_blacklevel.oddRevenC);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_blacklevel.oddRevenC);
        break;
    case EZT_PARMS_BLACKLEVEL_SNAPSHOT_BLACKODDROWODDCOL:
        EZLOGV("EZT_PARMS_BLACKLEVEL_SNAPSHOT_BLACKODDROWODDCOL %d",
                vfe_diagnostics->snap_blacklevel.oddRoddC);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_blacklevel.oddRoddC);
        break;
    case EZT_PARMS_BLACKLEVEL_PREVIEW_BLACKEVENROWEVENCOL:
        EZLOGV("EZT_PARMS_BLACKLEVEL_PREVIEW_BLACKEVENROWEVENCOL %d",
                vfe_diagnostics->prev_blacklevel.evenRevenC);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_blacklevel.evenRevenC);
        break;
    case EZT_PARMS_BLACKLEVEL_PREVIEW_BLACKEVENROWODDCOL:
        EZLOGV("EZT_PARMS_BLACKLEVEL_PREVIEW_BLACKEVENROWODDCOL %d",
                vfe_diagnostics->prev_blacklevel.evenRoddC);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_blacklevel.evenRoddC);
        break;
    case EZT_PARMS_BLACKLEVEL_PREVIEW_BLACKODDROWEVENCOL:
        EZLOGV("EZT_PARMS_BLACKLEVEL_PREVIEW_BLACKODDROWEVENCOL %d",
                vfe_diagnostics->prev_blacklevel.oddRevenC);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_blacklevel.oddRevenC);
        break;
    case EZT_PARMS_BLACKLEVEL_PREVIEW_BLACKODDROWODDCOL:
        EZLOGV("EZT_PARMS_BLACKLEVEL_PREVIEW_BLACKODDROWODDCOL %d",
                vfe_diagnostics->prev_blacklevel.oddRoddC);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_blacklevel.oddRoddC);
        break;

        /* Rolloff -------------------------------------------------------------------
         */
    case EZT_PARMS_ROLLOFF_ENABLE:
        EZLOGV("EZT_PARMS_ROLLOFF_ENABLE %d",
                vfe_diagnostics->control_rolloff.enable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_rolloff.enable);
        break;
    case EZT_PARMS_ROLLOFF_CONTROLENABLE:
        EZLOGV("EZT_PARMS_ROLLOFF_CONTROLENABLE %d",
                vfe_diagnostics->control_rolloff.cntrlenable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_rolloff.cntrlenable);
        break;
    case EZT_PARMS_ROLLOFF_PREVIEW_COEFFTABLE_R:
        f_value = (float *)&(vfe_diagnostics->prev_rolloff.coefftable_R);
        EZLOGV("EZT_PARMS_ROLLOFF_PREVIEW_COEFFTABLE_R %f",
                f_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                f_value[table_index]);
        break;
    case EZT_PARMS_ROLLOFF_PREVIEW_COEFFTABLE_GR:
        f_value = (float *)&(vfe_diagnostics->prev_rolloff.coefftable_Gr);
        EZLOGV("EZT_PARMS_ROLLOFF_PREVIEW_COEFFTABLE_GR %f",
                f_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                f_value[table_index]);
        break;
    case EZT_PARMS_ROLLOFF_PREVIEW_COEFFTABLE_GB:
        f_value = (float *)&(vfe_diagnostics->prev_rolloff.coefftable_Gb);
        EZLOGV("EZT_PARMS_ROLLOFF_PREVIEW_COEFFTABLE_GB %f",
                f_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                f_value[table_index]);
        break;
    case EZT_PARMS_ROLLOFF_PREVIEW_COEFFTABLE_B:
        f_value = (float *)&(vfe_diagnostics->prev_rolloff.coefftable_B);
        EZLOGV("EZT_PARMS_ROLLOFF_PREVIEW_COEFFTABLE_B %f",
                f_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                f_value[table_index]);
        break;
    case EZT_PARMS_ROLLOFF_SNAPSHOT_COEFFTABLE_R:
        f_value = (float *)&(vfe_diagnostics->snap_rolloff.coefftable_R);
        EZLOGV("EZT_PARMS_ROLLOFF_SNAPSHOT_COEFFTABLE_R %f",
                f_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                f_value[table_index]);
        break;
    case EZT_PARMS_ROLLOFF_SNAPSHOT_COEFFTABLE_GR:
        f_value = (float *)&(vfe_diagnostics->snap_rolloff.coefftable_Gr);
        EZLOGV("EZT_PARMS_ROLLOFF_SNAPSHOT_COEFFTABLE_GR %f",
                f_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                f_value[table_index]);
        break;
    case EZT_PARMS_ROLLOFF_SNAPSHOT_COEFFTABLE_GB:
        f_value = (float *)&(vfe_diagnostics->snap_rolloff.coefftable_Gb);
        EZLOGV("EZT_PARMS_ROLLOFF_SNAPSHOT_COEFFTABLE_GB %f",
                f_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                f_value[table_index]);
        break;
    case EZT_PARMS_ROLLOFF_SNAPSHOT_COEFFTABLE_B:
        f_value = (float *)&(vfe_diagnostics->snap_rolloff.coefftable_B);
        EZLOGV("EZT_PARMS_ROLLOFF_SNAPSHOT_COEFFTABLE_B %f",
                f_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                f_value[table_index]);
        break;
        /* 5X5 ASF -------------------------------------------------------------------
         */
    case EZT_PARMS_5X5ASF_ENABLE:
        EZLOGV("EZT_PARMS_5X5ASF_ENABLE %d",
                vfe_diagnostics->control_asf5x5.enable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_asf5x5.enable);
        break;
    case EZT_PARMS_5X5ASF_CONTROLENABLE:
        EZLOGV("EZT_PARMS_5X5ASF_CONTROLENABLE %d",
                vfe_diagnostics->control_asf5x5.cntrlenable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_asf5x5.cntrlenable);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SMOOTHFILTERENABLED:
        EZLOGV("EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SMOOTHFILTERENABLED %d",
                vfe_diagnostics->prev_asf5x5.smoothfilterEnabled);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_asf5x5.smoothfilterEnabled);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SHARPMODE:
        EZLOGV("EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SHARPMODE %d",
                vfe_diagnostics->prev_asf5x5.sharpMode);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_asf5x5.sharpMode);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_LPFMODE:
        EZLOGV("EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_LPFMODE %d",
                vfe_diagnostics->prev_asf5x5.lpfMode);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_asf5x5.lpfMode);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SMOOTHCOEFCENTER:
        EZLOGV("EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SMOOTHCOEFCENTER %d",
                vfe_diagnostics->prev_asf5x5.smoothcoefCenter);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_asf5x5.smoothcoefCenter);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SMOOTHCOEFSURR:
        EZLOGV("EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SMOOTHCOEFSURR %d",
                vfe_diagnostics->prev_asf5x5.smoothcoefSurr);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_asf5x5.smoothcoefSurr);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_PIPEFLUSHCOUNT:
        EZLOGV("EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_PIPEFLUSHCOUNT %d",
                vfe_diagnostics->prev_asf5x5.pipeflushCount);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_asf5x5.pipeflushCount);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_PIPEFLUSHOVD:
        EZLOGV("EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_PIPEFLUSHOVD %d",
                vfe_diagnostics->prev_asf5x5.pipeflushOvd);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_asf5x5.pipeflushOvd);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FLUSHHALTOVD:
        EZLOGV("EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FLUSHHALTOVD %d",
                vfe_diagnostics->prev_asf5x5.flushhaltOvd);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_asf5x5.flushhaltOvd);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_CROPENABLE:
        EZLOGV("EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_CROPENABLE %d",
                vfe_diagnostics->prev_asf5x5.cropEnable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_asf5x5.cropEnable);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_NORMALIZEFACTOR:
        EZLOGV("EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_NORMALIZEFACTOR %d",
                vfe_diagnostics->prev_asf5x5.normalizeFactor);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_asf5x5.normalizeFactor);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SETTINGLOWTHRESH:
        EZLOGV("EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SETTINGLOWTHRESH %d",
                vfe_diagnostics->prev_asf5x5.sharpthreshE1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_asf5x5.sharpthreshE1);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SETTINGUPTHRESH:
        EZLOGV("EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SETTINGUPTHRESH %d",
                vfe_diagnostics->prev_asf5x5.sharpthreshE2);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_asf5x5.sharpthreshE2);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SETTINGNEGTHRESH:
        EZLOGV("EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SETTINGNEGTHRESH %d",
                vfe_diagnostics->prev_asf5x5.sharpthreshE3);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_asf5x5.sharpthreshE3);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SETTINGUPTHRESHF2:
        EZLOGV("EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SETTINGUPTHRESHF2 %d",
                vfe_diagnostics->prev_asf5x5.sharpthreshE4);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_asf5x5.sharpthreshE4);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SETTINGNEGTHRESHF2:
        EZLOGV("EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SETTINGNEGTHRESHF2 %d",
                vfe_diagnostics->prev_asf5x5.sharpthreshE5);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_asf5x5.sharpthreshE5);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SETTINGSHARPAMTF1:
        EZLOGV("EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SETTINGSHARPAMTF1 %d",
                vfe_diagnostics->prev_asf5x5.sharpK1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_asf5x5.sharpK1);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SETTINGSHARPAMTF2:
        EZLOGV("EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SETTINGSHARPAMTF2 %d",
                vfe_diagnostics->prev_asf5x5.sharpK2);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_asf5x5.sharpK2);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER1A11:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER1A12:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER1A13:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER1A21:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER1A22:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER1A23:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER1A31:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER1A32:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER1A33:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER2A11:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER2A12:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER2A13:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER2A21:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER2A22:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER2A23:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER2A31:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER2A32:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER2A33:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER3A11:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER3A12:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER3A13:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER3A21:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER3A22:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER3A23:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER3A31:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER3A32:
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER3A33:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_asf5x5.f1coef0);
        EZLOGV("EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT %d",
                i32_value[item->id - EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER1A11]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[item->id - EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER1A11]);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SMOOTHFILTERENABLED:
        EZLOGV("EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SMOOTHFILTERENABLED %d",
                vfe_diagnostics->snap_asf5x5.smoothfilterEnabled);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_asf5x5.smoothfilterEnabled);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SHARPMODE:
        EZLOGV("EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SHARPMODE %d",
                vfe_diagnostics->snap_asf5x5.sharpMode);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_asf5x5.sharpMode);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_LPFMODE:
        EZLOGV("EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_LPFMODE %d",
                vfe_diagnostics->snap_asf5x5.lpfMode);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_asf5x5.lpfMode);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SMOOTHCOEFCENTER:
        EZLOGV("EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SMOOTHCOEFCENTER %d",
                vfe_diagnostics->snap_asf5x5.smoothcoefCenter);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_asf5x5.smoothcoefCenter);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SMOOTHCOEFSURR:
        EZLOGV("EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SMOOTHCOEFSURR %d",
                vfe_diagnostics->snap_asf5x5.smoothcoefSurr);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_asf5x5.smoothcoefSurr);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_PIPEFLUSHCOUNT:
        EZLOGV("EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_PIPEFLUSHCOUNT %d",
                vfe_diagnostics->snap_asf5x5.pipeflushCount);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_asf5x5.pipeflushCount);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_PIPEFLUSHOVD:
        EZLOGV("EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_PIPEFLUSHOVD %d",
                vfe_diagnostics->snap_asf5x5.pipeflushOvd);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_asf5x5.pipeflushOvd);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FLUSHHALTOVD:
        EZLOGV("EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FLUSHHALTOVD %d",
                vfe_diagnostics->snap_asf5x5.flushhaltOvd);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_asf5x5.flushhaltOvd);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_CROPENABLE:
        EZLOGV("EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_CROPENABLE %d",
                vfe_diagnostics->snap_asf5x5.cropEnable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_asf5x5.cropEnable);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_NORMALIZEFACTOR:
        EZLOGV("EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_NORMALIZEFACTOR %d",
                vfe_diagnostics->snap_asf5x5.normalizeFactor);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_asf5x5.normalizeFactor);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SETTINGLOWTHRESH:
        EZLOGV("EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SETTINGLOWTHRESH %d",
                vfe_diagnostics->snap_asf5x5.sharpthreshE1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_asf5x5.sharpthreshE1);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SETTINGUPTHRESH:
        EZLOGV("EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SETTINGUPTHRESH %d",
                vfe_diagnostics->snap_asf5x5.sharpthreshE2);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_asf5x5.sharpthreshE2);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SETTINGNEGTHRESH:
        EZLOGV("EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SETTINGNEGTHRESH %d",
                vfe_diagnostics->snap_asf5x5.sharpthreshE3);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_asf5x5.sharpthreshE3);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SETTINGUPTHRESHF2:
        EZLOGV("EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SETTINGUPTHRESHF2 %d",
                vfe_diagnostics->snap_asf5x5.sharpthreshE4);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_asf5x5.sharpthreshE4);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SETTINGNEGTHRESHF2:
        EZLOGV("EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SETTINGNEGTHRESHF2 %d",
                vfe_diagnostics->snap_asf5x5.sharpthreshE5);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_asf5x5.sharpthreshE5);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SETTINGSHARPAMTF1:
        EZLOGV("EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SETTINGSHARPAMTF1 %d",
                vfe_diagnostics->snap_asf5x5.sharpK1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_asf5x5.sharpK1);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SETTINGSHARPAMTF2:
        EZLOGV("EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SETTINGSHARPAMTF2 %d",
                vfe_diagnostics->snap_asf5x5.sharpK2);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_asf5x5.sharpK2);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER1A11:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER1A12:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER1A13:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER1A21:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER1A22:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER1A23:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER1A31:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER1A32:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER1A33:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER2A11:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER2A12:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER2A13:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER2A21:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER2A22:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER2A23:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER2A31:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER2A32:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER2A33:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER3A11:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER3A12:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER3A13:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER3A21:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER3A22:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER3A23:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER3A31:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER3A32:
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER3A33:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_asf5x5.f1coef0);
        EZLOGV("EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT %d",
                i32_value[item->id - EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER1A11]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[item->id - EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER1A11]);
        break;

        /* 7X7 ASF -------------------------------------------------------------------
         */
    case EZT_PARMS_7X7ASF_ENABLE:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->control_asf7x7.enable);
        break;
    case EZT_PARMS_7X7ASF_CONTROLENABLE:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->control_asf7x7.cntrlenable);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_SMOOTHPCT:
        EZLOGV("EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_SMOOTHPCT %d",
                pp_diagnostics->prev_asf7x7.smoothpercent);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf7x7.smoothpercent);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_ENABLESPECIALEFFECTS:
        EZLOGV("EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_ENABLESPECIALEFFECTS %d",
                pp_diagnostics->prev_asf7x7.sp_eff_en);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf7x7.sp_eff_en);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_NEGABSY1:
        EZLOGV("EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_NEGABSY1 %d",
                pp_diagnostics->prev_asf7x7.neg_abs_y1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf7x7.neg_abs_y1);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_NZ:
        EZLOGV("EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_NZ %d",
                pp_diagnostics->prev_asf7x7.nz_flag);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf7x7.nz_flag);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_F1:
        i32_value = (int32_t *)&(pp_diagnostics->prev_asf7x7.sobel_h_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_F2:
        i32_value = (int32_t *)&(pp_diagnostics->prev_asf7x7.sobel_v_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_F3:
        i32_value = (int32_t *)&(pp_diagnostics->prev_asf7x7.hpf_h_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_F4:
        i32_value = (int32_t *)&(pp_diagnostics->prev_asf7x7.hpf_v_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_F5:
        i32_value = (int32_t *)&(pp_diagnostics->prev_asf7x7.lpf_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_LUT1:
        i32_value = (int32_t *)&(pp_diagnostics->prev_asf7x7.lut1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_LUT2:
        i32_value = (int32_t *)&(pp_diagnostics->prev_asf7x7.lut2);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_LUT3:
        i32_value = (int32_t *)&(pp_diagnostics->prev_asf7x7.lut3);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_ENABLEDYNCLAMP:
        EZLOGV("EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_ENABLEDYNCLAMP %d",
                pp_diagnostics->prev_asf7x7.dyna_clamp_en);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf7x7.dyna_clamp_en);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_SMAX:
        EZLOGV("EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_SMAX %d",
                pp_diagnostics->prev_asf7x7.clamp_scale_max);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf7x7.clamp_scale_max);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_OMAX:
        EZLOGV("EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_OMAX %d",
                pp_diagnostics->prev_asf7x7.clamp_offset_max);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf7x7.clamp_offset_max);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_SMIN:
        EZLOGV("EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_SMIN %d",
                pp_diagnostics->prev_asf7x7.clamp_scale_min);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf7x7.clamp_scale_min);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_OMIN:
        EZLOGV("EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_OMIN %d",
                pp_diagnostics->prev_asf7x7.clamp_offset_min);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf7x7.clamp_offset_min);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_REGHH:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf7x7.clamp_hh);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_REGHL:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf7x7.clamp_hl);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_REGVH:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf7x7.clamp_vh);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_REGVL:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf7x7.clamp_vl);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_SMOOTHPCT:
        EZLOGV("here EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_SMOOTHPCT %d",
                pp_diagnostics->snap_asf7x7.smoothpercent);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf7x7.smoothpercent);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_ENABLESPECIALEFFECTS:
        EZLOGV("EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_ENABLESPECIALEFFECTS %d",
                pp_diagnostics->snap_asf7x7.sp_eff_en);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf7x7.sp_eff_en);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_NEGABSY1:
        EZLOGV("EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_NEGABSY1 %d",
                pp_diagnostics->snap_asf7x7.neg_abs_y1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf7x7.neg_abs_y1);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_NZ:
        EZLOGV("EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_NZ %d",
                pp_diagnostics->snap_asf7x7.nz_flag);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf7x7.nz_flag);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_F1:
        i32_value = (int32_t *)&(pp_diagnostics->snap_asf7x7.sobel_h_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_F2:
        i32_value = (int32_t *)&(pp_diagnostics->snap_asf7x7.sobel_v_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_F3:
        i32_value = (int32_t *)&(pp_diagnostics->snap_asf7x7.hpf_h_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_F4:
        i32_value = (int32_t *)&(pp_diagnostics->snap_asf7x7.hpf_v_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_F5:
        i32_value = (int32_t *)&(pp_diagnostics->snap_asf7x7.lpf_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_LUT1:
        i32_value = (int32_t *)&(pp_diagnostics->snap_asf7x7.lut1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_LUT2:
        i32_value = (int32_t *)&(pp_diagnostics->snap_asf7x7.lut2);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_LUT3:
        i32_value = (int32_t *)&(pp_diagnostics->snap_asf7x7.lut3);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_ENABLEDYNCLAMP:
        EZLOGV("EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_ENABLEDYNCLAMP %d",
                pp_diagnostics->snap_asf7x7.dyna_clamp_en);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf7x7.dyna_clamp_en);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_SMAX:
        EZLOGV("EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_SMAX %d",
                pp_diagnostics->snap_asf7x7.clamp_scale_max);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf7x7.clamp_scale_max);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_OMAX:
        EZLOGV("EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_OMAX %d",
                pp_diagnostics->snap_asf7x7.clamp_offset_max);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf7x7.clamp_offset_max);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_SMIN:
        EZLOGV("EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_SMIN %d",
                pp_diagnostics->snap_asf7x7.clamp_scale_min);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf7x7.clamp_scale_min);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_OMIN:
        EZLOGV("EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_OMIN %d",
                pp_diagnostics->snap_asf7x7.clamp_offset_min);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf7x7.clamp_offset_min);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_REGHH:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf7x7.clamp_hh);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_REGHL:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf7x7.clamp_hl);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_REGVH:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf7x7.clamp_vh);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_REGVL:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf7x7.clamp_vl);
        break;
    case EZT_PARMS_9X9ASF_ENABLE:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->control_asf9x9.enable);
        break;
    case EZT_PARMS_9X9ASF_CONTROLENABLE:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->control_asf9x9.cntrlenable);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_TIGGER_GAINSTART:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf9x9.trigger_gainStart);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_TIGGER_GAINEND:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf9x9.trigger_gainEnd);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_TRIGGER_LUTINDEXSTART:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf9x9.trigger_lutIndexStart);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_TRIGGER_LUTINDEXEND:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf9x9.trigger_lutIndexEnd);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_SMOOTHPCT:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf9x9.sp);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_HORIZNZ:
        i32_value = (int32_t *)&(pp_diagnostics->prev_asf9x9.horizontal_nz);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_VERTICALNZ:
        i32_value = (int32_t *)&(pp_diagnostics->prev_asf9x9.vertical_nz);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_SOBELHCOEFF:
        i16_value = (int16_t *)&(pp_diagnostics->prev_asf9x9.sobel_H_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i16_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_SOBELSEDIAGONALCOEFF:
        i16_value = (int16_t *)&(pp_diagnostics->prev_asf9x9.sobel_se_diagonal_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i16_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_HIGHPASSHCOEFF:
        i16_value = (int16_t *)&(pp_diagnostics->prev_asf9x9.hpf_h_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i16_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_HIGHPASSSEDIAGONALCOEFF:
        i16_value = (int16_t *)&(pp_diagnostics->prev_asf9x9.hpf_se_diagonal_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i16_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_HIGHPASSSYMMETRICCOEFF:
        i16_value = (int16_t *)&(pp_diagnostics->prev_asf9x9.hpf_symmetric_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i16_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_LOWPASSCOEFF:
        i16_value = (int16_t *)&(pp_diagnostics->prev_asf9x9.lpf_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i16_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_ACTIVITYLOWPASSCOEFF:
        i16_value = (int16_t *)&(pp_diagnostics->prev_asf9x9.activity_lpf_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i16_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_ACTIVITYBANDPASSCOEFF:
        i16_value = (int16_t *)&(pp_diagnostics->prev_asf9x9.activity_band_pass_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i16_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_ACTIVITYNORMALIZATIONLUT:
        f_value = (float *)&(pp_diagnostics->prev_asf9x9.activity_normalization_lut);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                f_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_WEIGHTMODULATIONLUT:
        f_value = (float *)&(pp_diagnostics->prev_asf9x9.weight_modulation_lut);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                f_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_SOFTTHREASHLUT:
        u32_value = (uint32_t *)&(pp_diagnostics->prev_asf9x9.soft_threshold_lut);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                u32_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_GAINLUT:
        f_value = (float *)&(pp_diagnostics->prev_asf9x9.gain_lut);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                f_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_GAINWEIGHTLUT:
        f_value = (float *)&(pp_diagnostics->prev_asf9x9.gain_weight_lut);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                f_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_GAINCAP:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                pp_diagnostics->prev_asf9x9.gain_cap);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_GAMMACORRECTEDLUMATARGET:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf9x9.gamma_corrected_luma_target);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_ENABLEDYNCLAMP:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf9x9.en_dyna_clamp);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_SMAX:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                pp_diagnostics->prev_asf9x9.smax);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_OMAX:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf9x9.omax);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_SMIN:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                pp_diagnostics->prev_asf9x9.smin);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_OMIN:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf9x9.omin);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_CLAMPUL:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf9x9.clamp_UL);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_CLAMPLL:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf9x9.clamp_LL);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_PERPENDICULARSCALEFACTOR:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                pp_diagnostics->prev_asf9x9.perpendicular_scale_factor);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_MAXVALUETHRESH:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf9x9.max_value_threshold);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_NORMSCALE:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                pp_diagnostics->prev_asf9x9.norm_scale);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_ACTIVITYCLAMPTHRESH:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf9x9.activity_clamp_threshold);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_L2NORMENABLE:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->prev_asf9x9.L2_norm_en);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_MEDIANBLENDUPPEROFFSET:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                pp_diagnostics->prev_asf9x9.median_blend_upper_offset);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_MEDIANBLENDLOWEROFFSET:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                pp_diagnostics->prev_asf9x9.median_blend_lower_offset);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_TIGGER_GAINSTART:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf9x9.trigger_gainStart);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_TIGGER_GAINEND:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf9x9.trigger_gainEnd);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_TRIGGER_LUTINDEXSTART:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf9x9.trigger_lutIndexStart);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_TRIGGER_LUTINDEXEND:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf9x9.trigger_lutIndexEnd);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_SMOOTHPCT:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf9x9.sp);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_HORIZNZ:
        i32_value = (int32_t *)&(pp_diagnostics->snap_asf9x9.horizontal_nz);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_VERTICALNZ:
        i32_value = (int32_t *)&(pp_diagnostics->snap_asf9x9.vertical_nz);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_SOBELHCOEFF:
        i16_value = (int16_t *)&(pp_diagnostics->snap_asf9x9.sobel_H_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i16_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_SOBELSEDIAGONALCOEFF:
        i16_value = (int16_t *)&(pp_diagnostics->snap_asf9x9.sobel_se_diagonal_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i16_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_HIGHPASSHCOEFF:
        i16_value = (int16_t *)&(pp_diagnostics->snap_asf9x9.hpf_h_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i16_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_HIGHPASSSEDIAGONALCOEFF:
        i16_value = (int16_t *)&(pp_diagnostics->snap_asf9x9.hpf_se_diagonal_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i16_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_HIGHPASSSYMMETRICCOEFF:
        i16_value = (int16_t *)&(pp_diagnostics->snap_asf9x9.hpf_symmetric_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i16_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_LOWPASSCOEFF:
        i16_value = (int16_t *)&(pp_diagnostics->snap_asf9x9.lpf_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i16_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_ACTIVITYLOWPASSCOEFF:
        i16_value = (int16_t *)&(pp_diagnostics->snap_asf9x9.activity_lpf_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i16_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_ACTIVITYBANDPASSCOEFF:
        i16_value = (int16_t *)&(pp_diagnostics->snap_asf9x9.activity_band_pass_coeff);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i16_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_ACTIVITYNORMALIZATIONLUT:
        f_value = (float *)&(pp_diagnostics->snap_asf9x9.activity_normalization_lut);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                f_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_WEIGHTMODULATIONLUT:
        f_value = (float *)&(pp_diagnostics->snap_asf9x9.weight_modulation_lut);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                f_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_SOFTTHREASHLUT:
        u32_value = (uint32_t *)&(pp_diagnostics->snap_asf9x9.soft_threshold_lut);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                u32_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_GAINLUT:
        f_value = (float *)&(pp_diagnostics->snap_asf9x9.gain_lut);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                f_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_GAINWEIGHTLUT:
        f_value = (float *)&(pp_diagnostics->snap_asf9x9.gain_weight_lut);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                f_value[table_index]);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_GAINCAP:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                pp_diagnostics->snap_asf9x9.gain_cap);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_GAMMACORRECTEDLUMATARGET:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf9x9.gamma_corrected_luma_target);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_ENABLEDYNCLAMP:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf9x9.en_dyna_clamp);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_SMAX:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                pp_diagnostics->snap_asf9x9.smax);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_OMAX:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf9x9.omax);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_SMIN:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                pp_diagnostics->snap_asf9x9.smin);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_OMIN:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf9x9.omin);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_CLAMPUL:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf9x9.clamp_UL);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_CLAMPLL:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf9x9.clamp_LL);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_PERPENDICULARSCALEFACTOR:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                pp_diagnostics->snap_asf9x9.perpendicular_scale_factor);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_MAXVALUETHRESH:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf9x9.max_value_threshold);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_NORMSCALE:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                pp_diagnostics->snap_asf9x9.norm_scale);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_ACTIVITYCLAMPTHRESH:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf9x9.activity_clamp_threshold);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_L2NORMENABLE:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->snap_asf9x9.L2_norm_en);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_MEDIANBLENDUPPEROFFSET:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                pp_diagnostics->snap_asf9x9.median_blend_upper_offset);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_MEDIANBLENDLOWEROFFSET:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                pp_diagnostics->snap_asf9x9.median_blend_lower_offset);
        break;

        /* Luma Adapt ----------------------------------------------------------------
         */
    case EZT_PARMS_LUMAADAPT_PREVIEW_LUTYRATIO:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_lumaadaptation.lut_yratio);
        EZLOGV("EZT_PARMS_LUMAADAPT_PREVIEW_LUTYRATIO %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_LUMAADAPT_SNAPSHOT_LUTYRATIO:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_lumaadaptation.lut_yratio);
        EZLOGV("EZT_PARMS_LUMAADAPT_SNAPSHOT_LUTYRATIO %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;

        /* Chroma Supp ---------------------------------------------------------------
         */
    case EZT_PARMS_CHROMASUPP_ENABLE:
        EZLOGV("EZT_PARMS_CHROMASUPP_ENABLE %d",
                vfe_diagnostics->control_chromasupp.enable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_chromasupp.enable);
        break;
    case EZT_PARMS_CHROMASUPP_CONTROLENABLE:
        EZLOGV("EZT_PARMS_CHROMASUPP_CONTROLENABLE %d",
                vfe_diagnostics->control_chromasupp.cntrlenable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_chromasupp.cntrlenable);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_YSUP1:
        EZLOGV("EZT_PARMS_CHROMASUPP_PREVIEW_YSUP1 %d",
                vfe_diagnostics->prev_chromasupp.ysup1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_chromasupp.ysup1);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_YSUP2:
        EZLOGV("EZT_PARMS_CHROMASUPP_PREVIEW_YSUP2 %d",
                vfe_diagnostics->prev_chromasupp.ysup2);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_chromasupp.ysup2);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_YSUP3:
        EZLOGV("EZT_PARMS_CHROMASUPP_PREVIEW_YSUP3 %d",
                vfe_diagnostics->prev_chromasupp.ysup3);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_chromasupp.ysup3);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_YSUP4:
        EZLOGV("EZT_PARMS_CHROMASUPP_PREVIEW_YSUP4 %d",
                vfe_diagnostics->prev_chromasupp.ysup4);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_chromasupp.ysup4);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_YSUPS1:
        EZLOGV("EZT_PARMS_CHROMASUPP_PREVIEW_YSUPS1 %d",
                vfe_diagnostics->prev_chromasupp.ysupS1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_chromasupp.ysupS1);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_YSUPS3:
        EZLOGV("EZT_PARMS_CHROMASUPP_PREVIEW_YSUPS3 %d",
                vfe_diagnostics->prev_chromasupp.ysupS3);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_chromasupp.ysupS3);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_YSUPM1:
        EZLOGV("EZT_PARMS_CHROMASUPP_PREVIEW_YSUPM1 %d",
                vfe_diagnostics->prev_chromasupp.ysupM1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_chromasupp.ysupM1);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_YSUPM3:
        EZLOGV("EZT_PARMS_CHROMASUPP_PREVIEW_YSUPM3 %d",
                vfe_diagnostics->prev_chromasupp.ysupM3);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_chromasupp.ysupM3);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_CSUP1:
        EZLOGV("EZT_PARMS_CHROMASUPP_PREVIEW_CSUP1 %d",
                vfe_diagnostics->prev_chromasupp.csup1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_chromasupp.csup1);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_CSUP2:
        EZLOGV("EZT_PARMS_CHROMASUPP_PREVIEW_CSUP2 %d",
                vfe_diagnostics->prev_chromasupp.csup2);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_chromasupp.csup2);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_CSUPM1:
        EZLOGV("EZT_PARMS_CHROMASUPP_PREVIEW_CSUPM1 %d",
                vfe_diagnostics->prev_chromasupp.csupM1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_chromasupp.csupM1);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_CSUPS1:
        EZLOGV("EZT_PARMS_CHROMASUPP_PREVIEW_CSUPS1 %d",
                vfe_diagnostics->prev_chromasupp.csupS1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_chromasupp.csupS1);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUP1:
        EZLOGV("EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUP1 %d",
                vfe_diagnostics->snap_chromasupp.ysup1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_chromasupp.ysup1);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUP2:
        EZLOGV("EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUP2 %d",
                vfe_diagnostics->snap_chromasupp.ysup2);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_chromasupp.ysup2);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUP3:
        EZLOGV("EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUP3 %d",
                vfe_diagnostics->snap_chromasupp.ysup3);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_chromasupp.ysup3);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUP4:
        EZLOGV("EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUP4 %d",
                vfe_diagnostics->snap_chromasupp.ysup4);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_chromasupp.ysup4);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUPS1:
        EZLOGV("EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUPS1 %d",
                vfe_diagnostics->snap_chromasupp.ysupS1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_chromasupp.ysupS1);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUPS3:
        EZLOGV("EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUPS3 %d",
                vfe_diagnostics->snap_chromasupp.ysupS3);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_chromasupp.ysupS3);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUPM1:
        EZLOGV("EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUPM1 %d",
                vfe_diagnostics->snap_chromasupp.ysupM1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_chromasupp.ysupM1);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUPM3:
        EZLOGV("EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUPM3 %d",
                vfe_diagnostics->snap_chromasupp.ysupM3);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_chromasupp.ysupM3);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_CSUP1:
        EZLOGV("EZT_PARMS_CHROMASUPP_SNAPSHOT_CSUP1 %d",
                vfe_diagnostics->snap_chromasupp.csup1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_chromasupp.csup1);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_CSUP2:
        EZLOGV("EZT_PARMS_CHROMASUPP_SNAPSHOT_CSUP2 %d",
                vfe_diagnostics->snap_chromasupp.csup2);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_chromasupp.csup2);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_CSUPM1:
        EZLOGV("EZT_PARMS_CHROMASUPP_SNAPSHOT_CSUPM1 %d",
                vfe_diagnostics->snap_chromasupp.csupM1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_chromasupp.csupM1);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_CSUPS1:
        EZLOGV("EZT_PARMS_CHROMASUPP_SNAPSHOT_CSUPS1 %d",
                vfe_diagnostics->snap_chromasupp.csupS1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_chromasupp.csupS1);
        break;
        /* MCE -----------------------------------------------------------------------
         */
    case EZT_PARMS_MCE_ENABLE:
        EZLOGV("EZT_PARMS_MCE_ENABLE %d",
                vfe_diagnostics->control_memcolorenhan.enable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_memcolorenhan.enable);
        break;
    case EZT_PARMS_MCE_CONTROLENABLE:
        EZLOGV("EZT_PARMS_MCE_CONTROLENABLE %d",
                vfe_diagnostics->control_memcolorenhan.cntrlenable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_memcolorenhan.cntrlenable);
        break;
    case EZT_PARMS_MCE_PREVIEW_QK:
        EZLOGV("EZT_PARMS_MCE_PREVIEW_QK %d",
                vfe_diagnostics->prev_memcolorenhan.qk);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_memcolorenhan.qk);
        break;
    case EZT_PARMS_MCE_GREEN_PREVIEW_Y1:
    case EZT_PARMS_MCE_GREEN_PREVIEW_Y2:
    case EZT_PARMS_MCE_GREEN_PREVIEW_Y3:
    case EZT_PARMS_MCE_GREEN_PREVIEW_Y4:
    case EZT_PARMS_MCE_GREEN_PREVIEW_YM1:
    case EZT_PARMS_MCE_GREEN_PREVIEW_YM3:
    case EZT_PARMS_MCE_GREEN_PREVIEW_YS1:
    case EZT_PARMS_MCE_GREEN_PREVIEW_YS3:
    case EZT_PARMS_MCE_GREEN_PREVIEW_TRANSWIDTH:
    case EZT_PARMS_MCE_GREEN_PREVIEW_TRANSTRUNCATE:
    case EZT_PARMS_MCE_GREEN_PREVIEW_CBZONE:
    case EZT_PARMS_MCE_GREEN_PREVIEW_CRZONE:
    case EZT_PARMS_MCE_GREEN_PREVIEW_TRANSSLOPE:
    case EZT_PARMS_MCE_GREEN_PREVIEW_K:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_memcolorenhan.green);
        EZLOGV("EZT_PARMS_MCE_GREEN_PREVIEW %d",
                i32_value[item->id - EZT_PARMS_MCE_GREEN_PREVIEW_Y1]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[item->id - EZT_PARMS_MCE_GREEN_PREVIEW_Y1]);
        break;
    case EZT_PARMS_MCE_RED_PREVIEW_Y1:
    case EZT_PARMS_MCE_RED_PREVIEW_Y2:
    case EZT_PARMS_MCE_RED_PREVIEW_Y3:
    case EZT_PARMS_MCE_RED_PREVIEW_Y4:
    case EZT_PARMS_MCE_RED_PREVIEW_YM1:
    case EZT_PARMS_MCE_RED_PREVIEW_YM3:
    case EZT_PARMS_MCE_RED_PREVIEW_YS1:
    case EZT_PARMS_MCE_RED_PREVIEW_YS3:
    case EZT_PARMS_MCE_RED_PREVIEW_TRANSWIDTH:
    case EZT_PARMS_MCE_RED_PREVIEW_TRANSTRUNCATE:
    case EZT_PARMS_MCE_RED_PREVIEW_CBZONE:
    case EZT_PARMS_MCE_RED_PREVIEW_CRZONE:
    case EZT_PARMS_MCE_RED_PREVIEW_TRANSSLOPE:
    case EZT_PARMS_MCE_RED_PREVIEW_K:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_memcolorenhan.red);
        EZLOGV("EZT_PARMS_MCE_RED_PREVIEW %d",
                i32_value[item->id - EZT_PARMS_MCE_RED_PREVIEW_Y1]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[item->id - EZT_PARMS_MCE_RED_PREVIEW_Y1]);
        break;
    case EZT_PARMS_MCE_BLUE_PREVIEW_Y1:
    case EZT_PARMS_MCE_BLUE_PREVIEW_Y2:
    case EZT_PARMS_MCE_BLUE_PREVIEW_Y3:
    case EZT_PARMS_MCE_BLUE_PREVIEW_Y4:
    case EZT_PARMS_MCE_BLUE_PREVIEW_YM1:
    case EZT_PARMS_MCE_BLUE_PREVIEW_YM3:
    case EZT_PARMS_MCE_BLUE_PREVIEW_YS1:
    case EZT_PARMS_MCE_BLUE_PREVIEW_YS3:
    case EZT_PARMS_MCE_BLUE_PREVIEW_TRANSWIDTH:
    case EZT_PARMS_MCE_BLUE_PREVIEW_TRANSTRUNCATE:
    case EZT_PARMS_MCE_BLUE_PREVIEW_CBZONE:
    case EZT_PARMS_MCE_BLUE_PREVIEW_CRZONE:
    case EZT_PARMS_MCE_BLUE_PREVIEW_TRANSSLOPE:
    case EZT_PARMS_MCE_BLUE_PREVIEW_K:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_memcolorenhan.blue);
        EZLOGV("EZT_PARMS_MCE_BLUE_PREVIEW %d",
                i32_value[item->id - EZT_PARMS_MCE_BLUE_PREVIEW_Y1]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[item->id - EZT_PARMS_MCE_BLUE_PREVIEW_Y1]);
        break;

    case EZT_PARMS_MCE_SNAPSHOT_QK:
        EZLOGV("EZT_PARMS_MCE_SNAPSHOT_QK %d",
                vfe_diagnostics->snap_memcolorenhan.qk);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_memcolorenhan.qk);
        break;
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_Y1:
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_Y2:
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_Y3:
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_Y4:
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_YM1:
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_YM3:
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_YS1:
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_YS3:
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_TRANSWIDTH:
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_TRANSTRUNCATE:
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_CBZONE:
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_CRZONE:
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_TRANSSLOPE:
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_K:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_memcolorenhan.green);
        EZLOGV("EZT_PARMS_MCE_GREEN_SNAPSHOT %d",
                i32_value[item->id - EZT_PARMS_MCE_GREEN_SNAPSHOT_Y1]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[item->id - EZT_PARMS_MCE_GREEN_SNAPSHOT_Y1]);
        break;
    case EZT_PARMS_MCE_RED_SNAPSHOT_Y1:
    case EZT_PARMS_MCE_RED_SNAPSHOT_Y2:
    case EZT_PARMS_MCE_RED_SNAPSHOT_Y3:
    case EZT_PARMS_MCE_RED_SNAPSHOT_Y4:
    case EZT_PARMS_MCE_RED_SNAPSHOT_YM1:
    case EZT_PARMS_MCE_RED_SNAPSHOT_YM3:
    case EZT_PARMS_MCE_RED_SNAPSHOT_YS1:
    case EZT_PARMS_MCE_RED_SNAPSHOT_YS3:
    case EZT_PARMS_MCE_RED_SNAPSHOT_TRANSWIDTH:
    case EZT_PARMS_MCE_RED_SNAPSHOT_TRANSTRUNCATE:
    case EZT_PARMS_MCE_RED_SNAPSHOT_CBZONE:
    case EZT_PARMS_MCE_RED_SNAPSHOT_CRZONE:
    case EZT_PARMS_MCE_RED_SNAPSHOT_TRANSSLOPE:
    case EZT_PARMS_MCE_RED_SNAPSHOT_K:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_memcolorenhan.red);
        EZLOGV("EZT_PARMS_MCE_RED_SNAPSHOT %d",
                i32_value[item->id - EZT_PARMS_MCE_RED_SNAPSHOT_Y1]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[item->id - EZT_PARMS_MCE_RED_SNAPSHOT_Y1]);
        break;
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_Y1:
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_Y2:
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_Y3:
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_Y4:
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_YM1:
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_YM3:
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_YS1:
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_YS3:
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_TRANSWIDTH:
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_TRANSTRUNCATE:
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_CBZONE:
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_CRZONE:
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_TRANSSLOPE:
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_K:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_memcolorenhan.blue);
        EZLOGV("EZT_PARMS_MCE_BLUE_SNAPSHOT %d",
                i32_value[item->id - EZT_PARMS_MCE_BLUE_SNAPSHOT_Y1]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[item->id - EZT_PARMS_MCE_BLUE_SNAPSHOT_Y1]);
        break;
        /* Wavelet -------------------------------------------------------------------
         */
    case EZT_PARMS_WAVELET_ENABLE                                                                                                                                                                                                                                                                                                                           :
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->control_wnr.enable);
        break;
    case EZT_PARMS_WAVELET_CONTROLENABLE:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                pp_diagnostics->control_wnr.cntrlenable);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_BILATERALSCALECORE0:
        i32_value = (int32_t *)&(pp_diagnostics->prev_wnr.bilateral_scalecore0);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_BILATERALSCALECORE1:
        i32_value = (int32_t *)&(pp_diagnostics->prev_wnr.bilateral_scalecore1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_BILATERALSCALECORE2:
        i32_value = (int32_t *)&(pp_diagnostics->prev_wnr.bilateral_scalecore2);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_NOISETHRESHOLDCORE0:
        i32_value = (int32_t *)&(pp_diagnostics->prev_wnr.noise_thresholdcore0);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_NOISETHRESHOLDCORE1:
        i32_value = (int32_t *)&(pp_diagnostics->prev_wnr.noise_thresholdcore1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_NOISETHRESHOLDCORE2:
        i32_value = (int32_t *)&(pp_diagnostics->prev_wnr.noise_thresholdcore2);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_WEIGHTCORE0:
        i32_value = (int32_t *)&(pp_diagnostics->prev_wnr.weightcore0);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_WEIGHTCORE1:
        i32_value = (int32_t *)&(pp_diagnostics->prev_wnr.weightcore1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_WEIGHTCORE2:
        i32_value = (int32_t *)&(pp_diagnostics->prev_wnr.weightcore2);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_YNOISETHRESH:
        i32_value = (int32_t *)&(pp_diagnostics->prev_wnr.y_noise_thresh);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_YWEIGHT1:
        i32_value = (int32_t *)&(pp_diagnostics->prev_wnr.y_weight1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_YBILATSCALE:
        i32_value = (int32_t *)&(pp_diagnostics->prev_wnr.y_bilat_scale);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_CBNOISETHRESH:
        i32_value = (int32_t *)&(pp_diagnostics->prev_wnr.cb_noise_thresh);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_CBWEIGHT1:
        i32_value = (int32_t *)&(pp_diagnostics->prev_wnr.cb_weight1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_CBBILATSCALE:
        i32_value = (int32_t *)&(pp_diagnostics->prev_wnr.cb_bilat_scale);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_CRNOISETHRESH:
        i32_value = (int32_t *)&(pp_diagnostics->prev_wnr.cr_noise_thresh);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_CRWEIGHT1:
        i32_value = (int32_t *)&(pp_diagnostics->prev_wnr.cr_weight1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_CRBILATSCALE:
        i32_value = (int32_t *)&(pp_diagnostics->prev_wnr.cr_bilat_scale);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_BILATERALSCALECORE0:
        i32_value = (int32_t *)&(pp_diagnostics->snap_wnr.bilateral_scalecore0);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_BILATERALSCALECORE1:
        i32_value = (int32_t *)&(pp_diagnostics->snap_wnr.bilateral_scalecore1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_BILATERALSCALECORE2:
        i32_value = (int32_t *)&(pp_diagnostics->snap_wnr.bilateral_scalecore2);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_NOISETHRESHOLDCORE0:
        i32_value = (int32_t *)&(pp_diagnostics->snap_wnr.noise_thresholdcore0);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_NOISETHRESHOLDCORE1:
        i32_value = (int32_t *)&(pp_diagnostics->snap_wnr.noise_thresholdcore1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_NOISETHRESHOLDCORE2:
        i32_value = (int32_t *)&(pp_diagnostics->snap_wnr.noise_thresholdcore2);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_WEIGHTCORE0:
        i32_value = (int32_t *)&(pp_diagnostics->snap_wnr.weightcore0);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_WEIGHTCORE1:
        i32_value = (int32_t *)&(pp_diagnostics->snap_wnr.weightcore1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_WEIGHTCORE2:
        i32_value = (int32_t *)&(pp_diagnostics->snap_wnr.weightcore2);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;

    case EZT_PARMS_WAVELET_SNAPSHOT_YNOISETHRESH:
        i32_value = (int32_t *)&(pp_diagnostics->snap_wnr.y_noise_thresh);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_YWEIGHT1:
        i32_value = (int32_t *)&(pp_diagnostics->snap_wnr.y_weight1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_YBILATSCALE:
        i32_value = (int32_t *)&(pp_diagnostics->snap_wnr.y_bilat_scale);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_CBNOISETHRESH:
        i32_value = (int32_t *)&(pp_diagnostics->snap_wnr.cb_noise_thresh);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_CBWEIGHT1:
        i32_value = (int32_t *)&(pp_diagnostics->snap_wnr.cb_weight1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_CBBILATSCALE:
        i32_value = (int32_t *)&(pp_diagnostics->snap_wnr.cb_bilat_scale);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_CRNOISETHRESH:
        i32_value = (int32_t *)&(pp_diagnostics->snap_wnr.cr_noise_thresh);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_CRWEIGHT1:
        i32_value = (int32_t *)&(pp_diagnostics->snap_wnr.cr_weight1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_CRBILATSCALE:
        i32_value = (int32_t *)&(pp_diagnostics->snap_wnr.cr_bilat_scale);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;

    /* ABF -----------------------------------------------------------------------
     */
    case EZT_PARMS_ABF_ENABLE:
        EZLOGV("EZT_PARMS_ABF_ENABLE %d",
                vfe_diagnostics->control_abfilter.enable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_abfilter.enable);
        break;
    case EZT_PARMS_ABF_CONTROLENABLE:
        EZLOGV("EZT_PARMS_ABF_CONTROLENABLE %d",
                vfe_diagnostics->control_abfilter.cntrlenable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_abfilter.cntrlenable);
        break;

        /* ABF2 ----------------------------------------------------------------------
         */
    case EZT_PARMS_ABF2_PREVIEW_GREEN_THRESH:
        u16_value = (uint16_t *)&(vfe_diagnostics->prev_abfilter.green.threshold);
        EZLOGV("EZT_PARMS_ABF2_PREVIEW_GREEN_THRESH %d", u16_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                u16_value[table_index]);
        break;
    case EZT_PARMS_ABF2_PREVIEW_GREEN_POSLUT:
        u16_value = (uint16_t *)&(vfe_diagnostics->prev_abfilter.green.pos);
        EZLOGV("EZT_PARMS_ABF2_PREVIEW_GREEN_POSLUT %d", u16_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                u16_value[table_index]);
        break;
    case EZT_PARMS_ABF2_PREVIEW_GREEN_NEGLUT:
        i16_value = (int16_t *)&(vfe_diagnostics->prev_abfilter.green.neg);
        EZLOGV("EZT_PARMS_ABF2_PREVIEW_GREEN_POSLUT %d", i16_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i16_value[table_index]);
        break;
    case EZT_PARMS_ABF2_PREVIEW_BLUE_THRESH:
        u16_value = (uint16_t *)&(vfe_diagnostics->prev_abfilter.blue.threshold);
        EZLOGV("EZT_PARMS_ABF2_PREVIEW_BLUE_THRESH %d", u16_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                u16_value[table_index]);
        break;
    case EZT_PARMS_ABF2_PREVIEW_BLUE_POSLUT:
        u16_value = (uint16_t *)&(vfe_diagnostics->prev_abfilter.blue.pos);
        EZLOGV("EZT_PARMS_ABF2_PREVIEW_BLUE_POSLUT %d", u16_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                u16_value[table_index]);
        break;
    case EZT_PARMS_ABF2_PREVIEW_BLUE_NEGLUT:
        i16_value = (int16_t *)&(vfe_diagnostics->prev_abfilter.blue.neg);
        EZLOGV("EZT_PARMS_ABF2_PREVIEW_BLUE_NEGLUT %d", i16_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i16_value[table_index]);
        break;
    case EZT_PARMS_ABF2_PREVIEW_RED_THRESH:
        u16_value = (uint16_t *)&(vfe_diagnostics->prev_abfilter.red.threshold);
        EZLOGV("EZT_PARMS_ABF2_PREVIEW_RED_THRESH %d", u16_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                u16_value[table_index]);
        break;
    case EZT_PARMS_ABF2_PREVIEW_RED_POSLUT:
        u16_value = (uint16_t *)&(vfe_diagnostics->prev_abfilter.red.pos);
        EZLOGV("EZT_PARMS_ABF2_PREVIEW_RED_POSLUT %d", u16_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                u16_value[table_index]);
        break;
    case EZT_PARMS_ABF2_PREVIEW_RED_NEGLUT:
        i16_value = (int16_t *)&(vfe_diagnostics->prev_abfilter.red.neg);
        EZLOGV("EZT_PARMS_ABF2_PREVIEW_RED_NEGLUT %d", i16_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i16_value[table_index]);
        break;
    case EZT_PARMS_ABF3_PREVIEW_ENABLE:
        EZLOGV("EZT_PARMS_ABF3_PREVIEW_ENABLE %d",
                vfe_diagnostics->prev_abfilter.enable_v3);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_abfilter.enable_v3);
        break;
    case EZT_PARMS_ABF3_PREVIEW_NOISESTD2LUTLEVEL0:
        u32_value = (uint32_t *)&(vfe_diagnostics->prev_abfilter.noise_std2_lut_level0);
        EZLOGV("EZT_PARMS_ABF3_PREVIEW_NOISESTD2LUTLEVEL0 %d", u32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                u32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_PREVIEW_NOISESTD2LUTLEVEL1:
        u32_value = (uint32_t *)&(vfe_diagnostics->prev_abfilter.noise_std2_lut_level1);
        EZLOGV("EZT_PARMS_ABF3_PREVIEW_NOISESTD2LUTLEVEL1 %d", u32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                u32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_PREVIEW_EDGESOFTNESS:
        EZLOGV("EZT_PARMS_ABF3_PREVIEW_EDGESOFTNESS %f",
                vfe_diagnostics->prev_abfilter.edge_softness);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                vfe_diagnostics->prev_abfilter.edge_softness);
        break;
    case EZT_PARMS_ABF3_PREVIEW_SOFTTHRESHNOISESCALE:
        f_value = (float *)&(vfe_diagnostics->prev_abfilter.softthld_noise_scale);
        EZLOGV("EZT_PARMS_ABF3_PREVIEW_SOFTTHRESHNOISESCALE %f", f_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                f_value[table_index]);
        break;
    case EZT_PARMS_ABF3_PREVIEW_FILTERSTRENGTH:
        f_value = (float *)&(vfe_diagnostics->prev_abfilter.filter_strength);
        EZLOGV("EZT_PARMS_ABF3_PREVIEW_FILTERSTRENGTH %f", f_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                f_value[table_index]);
        break;
    case EZT_PARMS_ABF3_PREVIEW_LUTBANKSEL:
        EZLOGV("EZT_PARMS_ABF3_PREVIEW_LUTBANKSEL %d",
                vfe_diagnostics->prev_abfilter.lut_bank_sel);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_abfilter.lut_bank_sel);
        break;
    case EZT_PARMS_ABF3_PREVIEW_FILTEROPTIONS:
        EZLOGV("EZT_PARMS_ABF3_PREVIEW_FILTEROPTIONS %d",
                vfe_diagnostics->prev_abfilter.filter_options);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_abfilter.filter_options);
        break;
    case EZT_PARMS_ABF3_PREVIEW_SIGNAL2LUTLEVEL0:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_abfilter.signal2_lut_level0);
        EZLOGV("EZT_PARMS_ABF3_PREVIEW_SIGNAL2LUTLEVEL0 %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_PREVIEW_SIGNAL2LUTLEVEL1:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_abfilter.signal2_lut_level1);
        EZLOGV("EZT_PARMS_ABF3_PREVIEW_SIGNAL2LUTLEVEL1 %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_PREVIEW_WTABLEADJ:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_abfilter.w_table_adj);
        EZLOGV("EZT_PARMS_ABF3_PREVIEW_WTABLEADJ %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_PREVIEW_NOISESCALE0:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_abfilter.noise_scale0);
        EZLOGV("EZT_PARMS_ABF3_PREVIEW_NOISESCALE0 %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_PREVIEW_NOISESCALE1:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_abfilter.noise_scale1);
        EZLOGV("EZT_PARMS_ABF3_PREVIEW_NOISESCALE1 %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_PREVIEW_NOISESCALE2:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_abfilter.noise_scale2);
        EZLOGV("EZT_PARMS_ABF3_PREVIEW_NOISESCALE2 %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_PREVIEW_NOISEOFFSET:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_abfilter.noise_offset);
        EZLOGV("EZT_PARMS_ABF3_PREVIEW_NOISEOFFSET %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_PREVIEW_SOFTTHRESHNOISESHIFT:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_abfilter.softthld_noise_shift);
        EZLOGV("EZT_PARMS_ABF3_PREVIEW_SOFTTHRESHNOISESHIFT %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_PREVIEW_DISTANCEKER0:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_abfilter.distance_ker0);
        EZLOGV("EZT_PARMS_ABF3_PREVIEW_DISTANCEKER0 %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_PREVIEW_DISTANCEKER1:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_abfilter.distance_ker1);
        EZLOGV("EZT_PARMS_ABF3_PREVIEW_DISTANCEKER1 %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_PREVIEW_MINMAXSEL0:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_abfilter.min_max_sel0);
        EZLOGV("EZT_PARMS_ABF3_PREVIEW_MINMAXSEL0 %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_PREVIEW_MINMAXSEL1:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_abfilter.min_max_sel1);
        EZLOGV("EZT_PARMS_ABF3_PREVIEW_MINMAXSEL1 %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_PREVIEW_CURVEOFFSET:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_abfilter.curve_offset);
        EZLOGV("EZT_PARMS_ABF3_PREVIEW_CURVEOFFSET %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF2_SNAPSHOT_GREEN_THRESH:
        u16_value = (uint16_t *)&(vfe_diagnostics->snap_abfilter.green.threshold);
        EZLOGV("EZT_PARMS_ABF2_SNAPSHOT_GREEN_THRESH %d", u16_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                u16_value[table_index]);
        break;
    case EZT_PARMS_ABF2_SNAPSHOT_GREEN_POSLUT:
        u16_value = (uint16_t *)&(vfe_diagnostics->snap_abfilter.green.pos);
        EZLOGV("EZT_PARMS_ABF2_SNAPSHOT_GREEN_POSLUT %d", u16_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                u16_value[table_index]);
        break;
    case EZT_PARMS_ABF2_SNAPSHOT_GREEN_NEGLUT:
        i16_value = (int16_t *)&(vfe_diagnostics->snap_abfilter.green.neg);
        EZLOGV("EZT_PARMS_ABF2_SNAPSHOT_GREEN_NEGLUT %d", i16_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i16_value[table_index]);
        break;
    case EZT_PARMS_ABF2_SNAPSHOT_BLUE_THRESH:
        u16_value = (uint16_t *)&(vfe_diagnostics->snap_abfilter.blue.threshold);
        EZLOGV("EZT_PARMS_ABF2_SNAPSHOT_BLUE_THRESH %d", u16_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                u16_value[table_index]);
        break;
    case EZT_PARMS_ABF2_SNAPSHOT_BLUE_POSLUT:
        u16_value = (uint16_t *)&(vfe_diagnostics->snap_abfilter.blue.pos);
        EZLOGV("EZT_PARMS_ABF2_SNAPSHOT_BLUE_POSLUT %d", u16_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                u16_value[table_index]);
        break;
    case EZT_PARMS_ABF2_SNAPSHOT_BLUE_NEGLUT:
        i16_value = (int16_t *)&(vfe_diagnostics->snap_abfilter.blue.neg);
        EZLOGV("EZT_PARMS_ABF2_SNAPSHOT_BLUE_NEGLUT %d", i16_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i16_value[table_index]);
        break;
    case EZT_PARMS_ABF2_SNAPSHOT_RED_THRESH:
        u16_value = (uint16_t *)&(vfe_diagnostics->snap_abfilter.red.threshold);
        EZLOGV("EZT_PARMS_ABF2_SNAPSHOT_RED_THRESH %d", u16_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                u16_value[table_index]);
        break;
    case EZT_PARMS_ABF2_SNAPSHOT_RED_POSLUT:
        u16_value = (uint16_t *)&(vfe_diagnostics->snap_abfilter.red.pos);
        EZLOGV("EZT_PARMS_ABF2_SNAPSHOT_RED_POSLUT %d", u16_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                u16_value[table_index]);
        break;
    case EZT_PARMS_ABF2_SNAPSHOT_RED_NEGLUT:
        i16_value = (int16_t *)&(vfe_diagnostics->snap_abfilter.red.neg);
        EZLOGV("EZT_PARMS_ABF2_SNAPSHOT_RED_NEGLUT %d", i16_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i16_value[table_index]);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_ENABLE:
        EZLOGV("EZT_PARMS_ABF3_SNAPSHOT_ENABLE %d",
                vfe_diagnostics->snap_abfilter.enable_v3);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_abfilter.enable_v3);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_NOISESTD2LUTLEVEL0:
        u32_value = (uint32_t *)&(vfe_diagnostics->snap_abfilter.noise_std2_lut_level0);
        EZLOGV("EZT_PARMS_ABF3_SNAPSHOT_NOISESTD2LUTLEVEL0 %d", u32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                u32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_NOISESTD2LUTLEVEL1:
        u32_value = (uint32_t *)&(vfe_diagnostics->snap_abfilter.noise_std2_lut_level1);
        EZLOGV("EZT_PARMS_ABF3_SNAPSHOT_NOISESTD2LUTLEVEL1 %d", u32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                u32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_EDGESOFTNESS:
        EZLOGV("EZT_PARMS_ABF3_SNAPSHOT_EDGESOFTNESS %f",
                vfe_diagnostics->snap_abfilter.edge_softness);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                vfe_diagnostics->snap_abfilter.edge_softness);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_SOFTTHRESHNOISESCALE:
        f_value = (float *)&(vfe_diagnostics->snap_abfilter.softthld_noise_scale);
        EZLOGV("EZT_PARMS_ABF3_SNAPSHOT_SOFTTHRESHNOISESCALE %f", f_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                f_value[table_index]);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_FILTERSTRENGTH:
        f_value = (float *)&(vfe_diagnostics->snap_abfilter.filter_strength);
        EZLOGV("EZT_PARMS_ABF3_SNAPSHOT_FILTERSTRENGTH %f", f_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                f_value[table_index]);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_LUTBANKSEL:
        EZLOGV("EZT_PARMS_ABF3_SNAPSHOT_LUTBANKSEL %d",
                vfe_diagnostics->snap_abfilter.lut_bank_sel);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_abfilter.lut_bank_sel);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_FILTEROPTIONS:
        EZLOGV("EZT_PARMS_ABF3_SNAPSHOT_FILTEROPTIONS %d",
                vfe_diagnostics->snap_abfilter.filter_options);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_abfilter.filter_options);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_SIGNAL2LUTLEVEL0:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_abfilter.signal2_lut_level0);
        EZLOGV("EZT_PARMS_ABF3_SNAPSHOT_SIGNAL2LUTLEVEL0 %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_SIGNAL2LUTLEVEL1:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_abfilter.signal2_lut_level1);
        EZLOGV("EZT_PARMS_ABF3_SNAPSHOT_SIGNAL2LUTLEVEL1 %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_WTABLEADJ:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_abfilter.w_table_adj);
        EZLOGV("EZT_PARMS_ABF3_SNAPSHOT_WTABLEADJ %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_NOISESCALE0:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_abfilter.noise_scale0);
        EZLOGV("EZT_PARMS_ABF3_SNAPSHOT_NOISESCALE0 %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_NOISESCALE1:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_abfilter.noise_scale1);
        EZLOGV("EZT_PARMS_ABF3_SNAPSHOT_NOISESCALE1 %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_NOISESCALE2:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_abfilter.noise_scale2);
        EZLOGV("EZT_PARMS_ABF3_SNAPSHOT_NOISESCALE2 %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_NOISEOFFSET:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_abfilter.noise_offset);
        EZLOGV("EZT_PARMS_ABF3_SNAPSHOT_NOISEOFFSET %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_SOFTTHRESHNOISESHIFT:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_abfilter.softthld_noise_shift);
        EZLOGV("EZT_PARMS_ABF3_SNAPSHOT_SOFTTHRESHNOISESHIFT %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_DISTANCEKER0:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_abfilter.distance_ker0);
        EZLOGV("EZT_PARMS_ABF3_SNAPSHOT_DISTANCEKER0 %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_DISTANCEKER1:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_abfilter.distance_ker1);
        EZLOGV("EZT_PARMS_ABF3_SNAPSHOT_DISTANCEKER1 %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_MINMAXSEL0:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_abfilter.min_max_sel0);
        EZLOGV("EZT_PARMS_ABF3_SNAPSHOT_MINMAXSEL0 %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_MINMAXSEL1:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_abfilter.min_max_sel1);
        EZLOGV("EZT_PARMS_ABF3_SNAPSHOT_MINMAXSEL1 %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_CURVEOFFSET:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_abfilter.curve_offset);
        EZLOGV("EZT_PARMS_ABF3_SNAPSHOT_CURVEOFFSET %d", i32_value[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;

        /* BPC -----------------------------------------------------------------------
         */
    case EZT_PARMS_BPC_ENABLE:
        EZLOGV("EZT_PARMS_BPC_ENABLE %d", vfe_diagnostics->control_bpc.enable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_bpc.enable);
        break;
    case EZT_PARMS_BPC_CONTROLENABLE:
        EZLOGV("EZT_PARMS_BPC_CONTROLENABLE %d",
                vfe_diagnostics->control_bpc.cntrlenable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_bpc.cntrlenable);
        break;
    case EZT_PARMS_BPC_PREVIEW_CURRENT_FMINTHRESHOLD:
        EZLOGV("EZT_PARMS_BPC_PREVIEW_CURRENT_FMINTHRESHOLD %d",
                vfe_diagnostics->prev_bpc.fminThreshold);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_bpc.fminThreshold);
        break;
    case EZT_PARMS_BPC_PREVIEW_CURRENT_FMAXTHRESHOLD:
        EZLOGV("EZT_PARMS_BPC_PREVIEW_CURRENT_FMAXTHRESHOLD %d",
                vfe_diagnostics->prev_bpc.fmaxThreshold);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_bpc.fmaxThreshold);
        break;
    case EZT_PARMS_BPC_PREVIEW_CURRENT_ROFFSETLO:
        EZLOGV("EZT_PARMS_BPC_PREVIEW_CURRENT_ROFFSETLO %d",
                vfe_diagnostics->prev_bpc.rOffsetLo);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_bpc.rOffsetLo);
        break;
    case EZT_PARMS_BPC_PREVIEW_CURRENT_ROFFSETHI:
        EZLOGV("EZT_PARMS_BPC_PREVIEW_CURRENT_ROFFSETHI %d",
                vfe_diagnostics->prev_bpc.rOffsetHi);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_bpc.rOffsetHi);
        break;
    case EZT_PARMS_BPC_PREVIEW_CURRENT_GROFFSETLO:
        EZLOGV("EZT_PARMS_BPC_PREVIEW_CURRENT_GROFFSETLO %d",
                vfe_diagnostics->prev_bpc.grOffsetLo);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_bpc.grOffsetLo);
        break;
    case EZT_PARMS_BPC_PREVIEW_CURRENT_GROFFSETHI:
        EZLOGV("EZT_PARMS_BPC_PREVIEW_CURRENT_GROFFSETHI %d",
                vfe_diagnostics->prev_bpc.grOffsetHi);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_bpc.grOffsetHi);
        break;
    case EZT_PARMS_BPC_PREVIEW_CURRENT_GBOFFSETLO:
        EZLOGV("EZT_PARMS_BPC_PREVIEW_CURRENT_GBOFFSETLO %d",
                vfe_diagnostics->prev_bpc.gbOffsetLo);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_bpc.gbOffsetLo);
        break;
    case EZT_PARMS_BPC_PREVIEW_CURRENT_GBOFFSETHI:
        EZLOGV("EZT_PARMS_BPC_PREVIEW_CURRENT_GBOFFSETHI %d",
                vfe_diagnostics->prev_bpc.gbOffsetHi);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_bpc.gbOffsetHi);
        break;
    case EZT_PARMS_BPC_PREVIEW_CURRENT_BOFFSETHI:
        EZLOGV("EZT_PARMS_BPC_PREVIEW_CURRENT_BOFFSETHI %d",
                vfe_diagnostics->prev_bpc.bOffsetHi);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_bpc.bOffsetHi);
        break;
    case EZT_PARMS_BPC_PREVIEW_CURRENT_BOFFSETLO:
        EZLOGV("EZT_PARMS_BPC_PREVIEW_CURRENT_BOFFSETLO %d",
                vfe_diagnostics->prev_bpc.bOffsetLo);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_bpc.bOffsetLo);
        break;
    case EZT_PARMS_BPC_SNAPSHOT_CURRENT_FMINTHRESHOLD:
        EZLOGV("EZT_PARMS_BPC_SNAPSHOT_CURRENT_FMINTHRESHOLD %d",
                vfe_diagnostics->snap_bpc.fminThreshold);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_bpc.fminThreshold);
        break;
    case EZT_PARMS_BPC_SNAPSHOT_CURRENT_FMAXTHRESHOLD:
        EZLOGV("EZT_PARMS_BPC_SNAPSHOT_CURRENT_FMAXTHRESHOLD %d",
                vfe_diagnostics->snap_bpc.fmaxThreshold);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_bpc.fmaxThreshold);
        break;
    case EZT_PARMS_BPC_SNAPSHOT_CURRENT_ROFFSETLO:
        EZLOGV("EZT_PARMS_BPC_SNAPSHOT_CURRENT_ROFFSETLO %d",
                vfe_diagnostics->snap_bpc.rOffsetLo);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_bpc.rOffsetLo);
        break;
    case EZT_PARMS_BPC_SNAPSHOT_CURRENT_ROFFSETHI:
        EZLOGV("EZT_PARMS_BPC_SNAPSHOT_CURRENT_ROFFSETHI %d",
                vfe_diagnostics->snap_bpc.rOffsetHi);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_bpc.rOffsetHi);
        break;
    case EZT_PARMS_BPC_SNAPSHOT_CURRENT_GROFFSETLO:
        EZLOGV("EZT_PARMS_BPC_SNAPSHOT_CURRENT_GROFFSETLO %d",
                vfe_diagnostics->snap_bpc.grOffsetLo);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_bpc.grOffsetLo);
        break;
    case EZT_PARMS_BPC_SNAPSHOT_CURRENT_GROFFSETHI:
        EZLOGV("EZT_PARMS_BPC_SNAPSHOT_CURRENT_GROFFSETHI %d",
                vfe_diagnostics->snap_bpc.grOffsetHi);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_bpc.grOffsetHi);
        break;
    case EZT_PARMS_BPC_SNAPSHOT_CURRENT_GBOFFSETLO:
        EZLOGV("EZT_PARMS_BPC_SNAPSHOT_CURRENT_GBOFFSETLO %d",
                vfe_diagnostics->snap_bpc.gbOffsetLo);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_bpc.gbOffsetLo);
        break;
    case EZT_PARMS_BPC_SNAPSHOT_CURRENT_GBOFFSETHI:
        EZLOGV("EZT_PARMS_BPC_SNAPSHOT_CURRENT_GBOFFSETHI %d",
                vfe_diagnostics->snap_bpc.gbOffsetHi);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_bpc.gbOffsetHi);
        break;
    case EZT_PARMS_BPC_SNAPSHOT_CURRENT_BOFFSETHI:
        EZLOGV("EZT_PARMS_BPC_SNAPSHOT_CURRENT_BOFFSETHI %d",
                vfe_diagnostics->snap_bpc.bOffsetHi);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_bpc.bOffsetHi);
        break;
    case EZT_PARMS_BPC_SNAPSHOT_CURRENT_BOFFSETLO:
        EZLOGV("EZT_PARMS_BPC_SNAPSHOT_CURRENT_BOFFSETLO %d",
                vfe_diagnostics->snap_bpc.bOffsetLo);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_bpc.bOffsetLo);
        break;

        /* BCC -----------------------------------------------------------------------
         */
    case EZT_PARMS_BCC_ENABLE:
        EZLOGV("EZT_PARMS_BCC_ENABLE %d", vfe_diagnostics->control_bcc.enable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_bcc.enable);
        break;
    case EZT_PARMS_BCC_CONTROLENABLE:
        EZLOGV("EZT_PARMS_BCC_CONTROLENABLE %d",
                vfe_diagnostics->control_bcc.cntrlenable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_bcc.cntrlenable);
        break;
    case EZT_PARMS_BCC_PREVIEW_CURRENT_FMINTHRESHOLD:
        EZLOGV("EZT_PARMS_BCC_PREVIEW_CURRENT_FMINTHRESHOLD %d",
                vfe_diagnostics->prev_bcc.fminThreshold);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_bcc.fminThreshold);
        break;
    case EZT_PARMS_BCC_PREVIEW_CURRENT_FMAXTHRESHOLD:
        EZLOGV("EZT_PARMS_BCC_PREVIEW_CURRENT_FMAXTHRESHOLD %d",
                vfe_diagnostics->prev_bcc.fmaxThreshold);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_bcc.fmaxThreshold);
        break;
    case EZT_PARMS_BCC_PREVIEW_CURRENT_ROFFSETLO:
        EZLOGV("EZT_PARMS_BCC_PREVIEW_CURRENT_ROFFSETLO %d",
                vfe_diagnostics->prev_bcc.rOffsetLo);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_bcc.rOffsetLo);
        break;
    case EZT_PARMS_BCC_PREVIEW_CURRENT_ROFFSETHI:
        EZLOGV("EZT_PARMS_BCC_PREVIEW_CURRENT_ROFFSETHI %d",
                vfe_diagnostics->prev_bcc.rOffsetHi);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_bcc.rOffsetHi);
        break;
    case EZT_PARMS_BCC_PREVIEW_CURRENT_GROFFSETLO:
        EZLOGV("EZT_PARMS_BCC_PREVIEW_CURRENT_GROFFSETLO %d",
                vfe_diagnostics->prev_bcc.grOffsetLo);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_bcc.grOffsetLo);
        break;
    case EZT_PARMS_BCC_PREVIEW_CURRENT_GROFFSETHI:
        EZLOGV("EZT_PARMS_BCC_PREVIEW_CURRENT_GROFFSETHI %d",
                vfe_diagnostics->prev_bcc.grOffsetHi);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_bcc.grOffsetHi);
        break;
    case EZT_PARMS_BCC_PREVIEW_CURRENT_GBOFFSETLO:
        EZLOGV("EZT_PARMS_BCC_PREVIEW_CURRENT_GBOFFSETLO %d",
                vfe_diagnostics->prev_bcc.gbOffsetLo);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_bcc.gbOffsetLo);
        break;
    case EZT_PARMS_BCC_PREVIEW_CURRENT_GBOFFSETHI:
        EZLOGV("EZT_PARMS_BCC_PREVIEW_CURRENT_GBOFFSETHI %d",
                vfe_diagnostics->prev_bcc.gbOffsetHi);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_bcc.gbOffsetHi);
        break;
    case EZT_PARMS_BCC_PREVIEW_CURRENT_BOFFSETHI:
        EZLOGV("EZT_PARMS_BCC_PREVIEW_CURRENT_BOFFSETHI %d",
                vfe_diagnostics->prev_bcc.bOffsetHi);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_bcc.bOffsetHi);
        break;
    case EZT_PARMS_BCC_PREVIEW_CURRENT_BOFFSETLO:
        EZLOGV("EZT_PARMS_BCC_PREVIEW_CURRENT_BOFFSETLO %d",
                vfe_diagnostics->prev_bcc.bOffsetLo);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_bcc.bOffsetLo);
        break;
    case EZT_PARMS_BCC_SNAPSHOT_CURRENT_FMINTHRESHOLD:
        EZLOGV("EZT_PARMS_BCC_SNAPSHOT_CURRENT_FMINTHRESHOLD %d",
                vfe_diagnostics->snap_bcc.fminThreshold);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_bcc.fminThreshold);
        break;
    case EZT_PARMS_BCC_SNAPSHOT_CURRENT_FMAXTHRESHOLD:
        EZLOGV("EZT_PARMS_BCC_SNAPSHOT_CURRENT_FMAXTHRESHOLD %d",
                vfe_diagnostics->snap_bcc.fmaxThreshold);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_bcc.fmaxThreshold);
        break;
    case EZT_PARMS_BCC_SNAPSHOT_CURRENT_ROFFSETLO:
        EZLOGV("EZT_PARMS_BCC_SNAPSHOT_CURRENT_ROFFSETLO %d",
                vfe_diagnostics->snap_bcc.rOffsetLo);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_bcc.rOffsetLo);
        break;
    case EZT_PARMS_BCC_SNAPSHOT_CURRENT_ROFFSETHI:
        EZLOGV("EZT_PARMS_BCC_SNAPSHOT_CURRENT_ROFFSETHI %d",
                vfe_diagnostics->snap_bcc.rOffsetHi);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_bcc.rOffsetHi);
        break;
    case EZT_PARMS_BCC_SNAPSHOT_CURRENT_GROFFSETLO:
        EZLOGV("EZT_PARMS_BCC_SNAPSHOT_CURRENT_GROFFSETLO %d",
                vfe_diagnostics->snap_bcc.grOffsetLo);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_bcc.grOffsetLo);
        break;
    case EZT_PARMS_BCC_SNAPSHOT_CURRENT_GROFFSETHI:
        EZLOGV("EZT_PARMS_BCC_SNAPSHOT_CURRENT_GROFFSETHI %d",
                vfe_diagnostics->snap_bcc.grOffsetHi);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_bcc.grOffsetHi);
        break;
    case EZT_PARMS_BCC_SNAPSHOT_CURRENT_GBOFFSETLO:
        EZLOGV("EZT_PARMS_BCC_SNAPSHOT_CURRENT_GBOFFSETLO %d",
                vfe_diagnostics->snap_bcc.gbOffsetLo);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_bcc.gbOffsetLo);
        break;
    case EZT_PARMS_BCC_SNAPSHOT_CURRENT_GBOFFSETHI:
        EZLOGV("EZT_PARMS_BCC_SNAPSHOT_CURRENT_GBOFFSETHI %d",
                vfe_diagnostics->snap_bcc.gbOffsetHi);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_bcc.gbOffsetHi);
        break;
    case EZT_PARMS_BCC_SNAPSHOT_CURRENT_BOFFSETHI:
        EZLOGV("EZT_PARMS_BCC_SNAPSHOT_CURRENT_BOFFSETHI %d",
                vfe_diagnostics->snap_bcc.bOffsetHi);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_bcc.bOffsetHi);
        break;
    case EZT_PARMS_BCC_SNAPSHOT_CURRENT_BOFFSETLO:
        EZLOGV("EZT_PARMS_BCC_SNAPSHOT_CURRENT_BOFFSETLO %d",
                vfe_diagnostics->snap_bcc.bOffsetLo);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_bcc.bOffsetLo);
        break;

        /* Demosaic ------------------------------------------------------------------
         */
    case EZT_PARMS_DEMOSAIC_ENABLE:
        EZLOGV("EZT_PARMS_DEMOSAIC_ENABLE %d",
                vfe_diagnostics->control_demosaic.enable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_demosaic.enable);
        break;
    case EZT_PARMS_DEMOSAIC_CONTROLENABLE:
        EZLOGV("EZT_PARMS_DEMOSAIC_CONTROLENABLE %d",
                vfe_diagnostics->control_demosaic.cntrlenable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_demosaic.cntrlenable);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_WK:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_WK %d",
                vfe_diagnostics->prev_demosaic.lut[table_index].wk);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.lut[table_index].wk);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_BK:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_BK %d",
                vfe_diagnostics->prev_demosaic.lut[table_index].bk);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.lut[table_index].bk);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_LK:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_LK %d",
                vfe_diagnostics->prev_demosaic.lut[table_index].lk);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.lut[table_index].lk);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_TK:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_TK %d",
                vfe_diagnostics->prev_demosaic.lut[table_index].tk);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.lut[table_index].tk);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_AG:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_AG %d",
                vfe_diagnostics->prev_demosaic.aG);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.aG);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_BL:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_BL %d",
                vfe_diagnostics->prev_demosaic.bL);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.bL);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_DISABLEDIRECTIONALG:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_DISABLEDIRECTIONALG %d",
                vfe_diagnostics->prev_demosaic.disable_dirG);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.disable_dirG);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_ENABLEDYNCLAMPG:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_ENABLEDYNCLAMPG %d",
                vfe_diagnostics->prev_demosaic.enable_dynG);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.enable_dynG);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_DISABLEDIRECTIONALRB:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_DISABLEDIRECTIONALRB %d",
                vfe_diagnostics->prev_demosaic.disable_dirRB);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.disable_dirRB);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_ENABLEDYNCLAMPRB:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_ENABLEDYNCLAMPRB %d",
                vfe_diagnostics->prev_demosaic.enable_dynRB);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.enable_dynRB);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_NOISELEVELG:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_NOISELEVELG %d",
                vfe_diagnostics->prev_demosaic.noise_levelG);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.noise_levelG);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_NOISELEVELRB:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_NOISELEVELRB %d",
                vfe_diagnostics->prev_demosaic.noise_levelRB);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.noise_levelRB);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_BU:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_BU %d",
                vfe_diagnostics->prev_demosaic.bU);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.bU);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_DBLU:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_DBLU %d",
                vfe_diagnostics->prev_demosaic.dbLU);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.dbLU);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_KRG:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_KRG %d",
                vfe_diagnostics->prev_demosaic.krg);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.krg);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_KBG:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_KBG %d",
                vfe_diagnostics->prev_demosaic.kbg);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.kbg);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_KGR:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_KGR %d",
                vfe_diagnostics->prev_demosaic.kgr);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.kgr);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_KGB:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_KGB %d",
                vfe_diagnostics->prev_demosaic.kgb);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.kgb);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_ENABLEV4:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_ENABLEV4 %d",
                vfe_diagnostics->prev_demosaic.enable_v4);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.enable_v4);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_ENABLEDYNCLAMPRBXCL:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_ENABLEDYNCLAMPRBXCL %d",
                vfe_diagnostics->prev_demosaic.enable_dynClamp_RBXcl);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.enable_dynClamp_RBXcl);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_ENABLEDYNCLAMPGXCL:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_ENABLEDYNCLAMPGXCL %d",
                vfe_diagnostics->prev_demosaic.enable_dynClamp_GXcl);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.enable_dynClamp_GXcl);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_LAMBDAG:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_LAMBDAG %d",
                vfe_diagnostics->prev_demosaic.lambdaG);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.lambdaG);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_LAMBDARB:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_LAMBDARB %d",
                vfe_diagnostics->prev_demosaic.lambdaRB);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.lambdaRB);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_WGR1:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_WGR1 %d",
                vfe_diagnostics->prev_demosaic.wgr1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.wgr1);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_WGR2:
        EZLOGV("EZT_PARMS_DEMOSAIC_PREVIEW_WGR2 %d",
                vfe_diagnostics->prev_demosaic.wgr2);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demosaic.wgr2);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_WK:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_WK %d",
                vfe_diagnostics->snap_demosaic.lut[table_index].wk);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.lut[table_index].wk);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_BK:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_BK %d",
                vfe_diagnostics->snap_demosaic.lut[table_index].bk);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.lut[table_index].bk);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_LK:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_LK %d",
                vfe_diagnostics->snap_demosaic.lut[table_index].lk);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.lut[table_index].lk);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_TK:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_TK %d",
                vfe_diagnostics->snap_demosaic.lut[table_index].tk);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.lut[table_index].tk);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_AG:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_AG %d",
                vfe_diagnostics->snap_demosaic.aG);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.aG);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_BL:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_BL %d",
                vfe_diagnostics->snap_demosaic.bL);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.bL);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_DISABLEDIRECTIONALG:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_DISABLEDIRECTIONALG %d",
                vfe_diagnostics->snap_demosaic.disable_dirG);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.disable_dirG);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_ENABLEDYNCLAMPG:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_ENABLEDYNCLAMPG %d",
                vfe_diagnostics->snap_demosaic.enable_dynG);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.enable_dynG);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_DISABLEDIRECTIONALRB:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_DISABLEDIRECTIONALRB %d",
                vfe_diagnostics->snap_demosaic.disable_dirRB);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.disable_dirRB);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_ENABLEDYNCLAMPRB:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_ENABLEDYNCLAMPRB %d",
                vfe_diagnostics->snap_demosaic.enable_dynRB);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.enable_dynRB);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_NOISELEVELG:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_NOISELEVELG %d",
                vfe_diagnostics->snap_demosaic.noise_levelG);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.noise_levelG);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_NOISELEVELRB:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_NOISELEVELRB %d",
                vfe_diagnostics->snap_demosaic.noise_levelRB);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.noise_levelRB);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_BU:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_BU %d",
                vfe_diagnostics->snap_demosaic.bU);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.bU);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_DBLU:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_DBLU %d",
                vfe_diagnostics->snap_demosaic.dbLU);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.dbLU);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_KRG:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_KRG %d",
                vfe_diagnostics->snap_demosaic.krg);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.krg);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_KBG:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_KBG %d",
                vfe_diagnostics->snap_demosaic.kbg);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.kbg);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_KGR:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_KGR %d",
                vfe_diagnostics->snap_demosaic.kgr);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.kgr);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_KGB:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_KGB %d",
                vfe_diagnostics->snap_demosaic.kgb);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.kgb);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_ENABLEV4:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_ENABLEV4 %d",
                vfe_diagnostics->snap_demosaic.enable_v4);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.enable_v4);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_ENABLEDYNCLAMPRBXCL:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_ENABLEDYNCLAMPRBXCL %d",
                vfe_diagnostics->snap_demosaic.enable_dynClamp_RBXcl);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.enable_dynClamp_RBXcl);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_ENABLEDYNCLAMPGXCL:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_ENABLEDYNCLAMPGXCL %d",
                vfe_diagnostics->snap_demosaic.enable_dynClamp_GXcl);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.enable_dynClamp_GXcl);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_LAMBDAG:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_LAMBDAG %d",
                vfe_diagnostics->snap_demosaic.lambdaG);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.lambdaG);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_LAMBDARB:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_LAMBDARB %d",
                vfe_diagnostics->snap_demosaic.lambdaRB);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.lambdaRB);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_WGR1:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_WGR1 %d",
                vfe_diagnostics->snap_demosaic.wgr1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.wgr1);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_WGR2:
        EZLOGV("EZT_PARMS_DEMOSAIC_SNAPSHOT_WGR2 %d",
                vfe_diagnostics->snap_demosaic.wgr2);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demosaic.wgr2);
        break;

        /* CLF -----------------------------------------------------------------------
         */
    case EZT_PARMS_CLFILTER_ENABLE:
        EZLOGV("EZT_PARMS_CLFILTER_ENABLE %d",
                vfe_diagnostics->control_clfilter.enable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_clfilter.enable);
        break;
    case EZT_PARMS_CLFILTER_CONTROLENABLE:
        EZLOGV("EZT_PARMS_CLFILTER_CONTROLENABLE %d",
                vfe_diagnostics->control_clfilter.cntrlenable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_clfilter.cntrlenable);
        break;
    case EZT_PARMS_CHROMAFILTER_PREVIEW_H:
        EZLOGV("EZT_PARMS_CHROMAFILTER_PREVIEW_H %f",
                vfe_diagnostics->prev_chromalumafilter.chromafilter.hcoeff[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                vfe_diagnostics->prev_chromalumafilter.chromafilter.hcoeff[table_index]);
        break;
    case EZT_PARMS_CHROMAFILTER_PREVIEW_V:
        EZLOGV("EZT_PARMS_CHROMAFILTER_PREVIEW_V %f",
                vfe_diagnostics->prev_chromalumafilter.chromafilter.vcoeff[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                vfe_diagnostics->prev_chromalumafilter.chromafilter.vcoeff[table_index]);
        break;
    case EZT_PARMS_CHROMAFILTER_SNAPSHOT_H:
        EZLOGV("EZT_PARMS_CHROMAFILTER_SNAPSHOT_H %f",
                vfe_diagnostics->snap_chromalumafilter.chromafilter.hcoeff[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                vfe_diagnostics->snap_chromalumafilter.chromafilter.hcoeff[table_index]);
        break;
    case EZT_PARMS_CHROMAFILTER_SNAPSHOT_V:
        EZLOGV("EZT_PARMS_CHROMAFILTER_SNAPSHOT_V %f",
                vfe_diagnostics->snap_chromalumafilter.chromafilter.vcoeff[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                vfe_diagnostics->snap_chromalumafilter.chromafilter.vcoeff[table_index]);
        break;
    case EZT_PARMS_LUMAFILTER_PREVIEW_THRESHOLDRED:
        EZLOGV("EZT_PARMS_LUMAFILTER_PREVIEW_THRESHOLDRED %d",
                vfe_diagnostics->prev_chromalumafilter.lumafilter
                .threshold_red[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_chromalumafilter.lumafilter
                .threshold_red[table_index]);
        break;
    case EZT_PARMS_LUMAFILTER_PREVIEW_SCALEFACTORRED:
        EZLOGV("EZT_PARMS_LUMAFILTER_PREVIEW_SCALEFACTORRED %f",
                vfe_diagnostics->prev_chromalumafilter.lumafilter
                .scalefactor_red[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                vfe_diagnostics->prev_chromalumafilter.lumafilter
                .scalefactor_red[table_index]);
        break;
    case EZT_PARMS_LUMAFILTER_PREVIEW_THRESHOLDGREEN:
        EZLOGV("EZT_PARMS_LUMAFILTER_PREVIEW_THRESHOLDGREEN %d",
                vfe_diagnostics->prev_chromalumafilter.lumafilter
                .threshold_green[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_chromalumafilter.lumafilter
                .threshold_green[table_index]);
        break;
    case EZT_PARMS_LUMAFILTER_PREVIEW_SCALEFACTORGREEN:
        EZLOGV("EZT_PARMS_LUMAFILTER_PREVIEW_SCALEFACTORGREEN %f",
                vfe_diagnostics->prev_chromalumafilter.lumafilter
                .scalefactor_green[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                vfe_diagnostics->prev_chromalumafilter.lumafilter
                .scalefactor_green[table_index]);
        break;
    case EZT_PARMS_LUMAFILTER_PREVIEW_THRESHOLDBLUE:
        EZLOGV("EZT_PARMS_LUMAFILTER_PREVIEW_THRESHOLDBLUE %d",
                vfe_diagnostics->prev_chromalumafilter.lumafilter
                .threshold_blue[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_chromalumafilter.lumafilter
                .threshold_blue[table_index]);
        break;
    case EZT_PARMS_LUMAFILTER_PREVIEW_SCALEFACTORBLUE:
        EZLOGV("EZT_PARMS_LUMAFILTER_PREVIEW_SCALEFACTORBLUE %f",
                vfe_diagnostics->prev_chromalumafilter.lumafilter
                .scalefactor_blue[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                vfe_diagnostics->prev_chromalumafilter.lumafilter
                .scalefactor_blue[table_index]);
        break;
    case EZT_PARMS_LUMAFILTER_PREVIEW_A:
        EZLOGV("EZT_PARMS_LUMAFILTER_PREVIEW_A %f",
                vfe_diagnostics->prev_chromalumafilter.lumafilter.a[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                vfe_diagnostics->prev_chromalumafilter.lumafilter.a[table_index]);
        break;
    case EZT_PARMS_LUMAFILTER_PREVIEW_TABLEPOS:
        EZLOGV("EZT_PARMS_LUMAFILTER_PREVIEW_TABLEPOS %f",
                vfe_diagnostics->prev_chromalumafilter.lumafilter.tablepos[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                vfe_diagnostics->prev_chromalumafilter.lumafilter.tablepos[table_index]);
        break;
    case EZT_PARMS_LUMAFILTER_PREVIEW_TABLENEG:
        EZLOGV("EZT_PARMS_LUMAFILTER_PREVIEW_TABLENEG %f",
                vfe_diagnostics->prev_chromalumafilter.lumafilter.tableneg[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                vfe_diagnostics->prev_chromalumafilter.lumafilter.tableneg[table_index]);
        break;
    case EZT_PARMS_LUMAFILTER_SNAPSHOT_THRESHOLDRED:
        EZLOGV("EZT_PARMS_LUMAFILTER_SNAPSHOT_THRESHOLDRED %d",
                vfe_diagnostics->snap_chromalumafilter.lumafilter
                .threshold_red[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_chromalumafilter.lumafilter
                .threshold_red[table_index]);
        break;
    case EZT_PARMS_LUMAFILTER_SNAPSHOT_SCALEFACTORRED:
        EZLOGV("EZT_PARMS_LUMAFILTER_SNAPSHOT_SCALEFACTORRED %f",
                vfe_diagnostics->snap_chromalumafilter.lumafilter
                .scalefactor_red[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                vfe_diagnostics->snap_chromalumafilter.lumafilter
                .scalefactor_red[table_index]);
        break;
    case EZT_PARMS_LUMAFILTER_SNAPSHOT_THRESHOLDGREEN:
        EZLOGV("EZT_PARMS_LUMAFILTER_SNAPSHOT_THRESHOLDGREEN %d",
                vfe_diagnostics->snap_chromalumafilter
                .lumafilter.threshold_green[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_chromalumafilter
                .lumafilter.threshold_green[table_index]);
        break;
    case EZT_PARMS_LUMAFILTER_SNAPSHOT_SCALEFACTORGREEN:
        EZLOGV("EZT_PARMS_LUMAFILTER_SNAPSHOT_SCALEFACTORGREEN %f",
                vfe_diagnostics->snap_chromalumafilter
                .lumafilter.scalefactor_green[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                vfe_diagnostics->snap_chromalumafilter
                .lumafilter.scalefactor_green[table_index]);
        break;
    case EZT_PARMS_LUMAFILTER_SNAPSHOT_THRESHOLDBLUE:
        EZLOGV("EZT_PARMS_LUMAFILTER_SNAPSHOT_THRESHOLDBLUE %d",
                vfe_diagnostics->snap_chromalumafilter
                .lumafilter.threshold_blue[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_chromalumafilter
                .lumafilter.threshold_blue[table_index]);
        break;
    case EZT_PARMS_LUMAFILTER_SNAPSHOT_SCALEFACTORBLUE:
        EZLOGV("EZT_PARMS_LUMAFILTER_SNAPSHOT_SCALEFACTORBLUE %f",
                vfe_diagnostics->snap_chromalumafilter
                .lumafilter.scalefactor_blue[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                vfe_diagnostics->snap_chromalumafilter
                .lumafilter.scalefactor_blue[table_index]);
        break;
    case EZT_PARMS_LUMAFILTER_SNAPSHOT_A:
        EZLOGV("EZT_PARMS_LUMAFILTER_SNAPSHOT_A %f",
                vfe_diagnostics->snap_chromalumafilter.lumafilter.a[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                vfe_diagnostics->snap_chromalumafilter.lumafilter.a[table_index]);
        break;
    case EZT_PARMS_LUMAFILTER_SNAPSHOT_TABLEPOS:
        EZLOGV("EZT_PARMS_LUMAFILTER_SNAPSHOT_TABLEPOS %f",
                vfe_diagnostics->snap_chromalumafilter.lumafilter.tablepos[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                vfe_diagnostics->snap_chromalumafilter.lumafilter.tablepos[table_index]);
        break;
    case EZT_PARMS_LUMAFILTER_SNAPSHOT_TABLENEG:
        EZLOGV("EZT_PARMS_LUMAFILTER_SNAPSHOT_TABLENEG %f",
                vfe_diagnostics->snap_chromalumafilter.lumafilter.tableneg[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                vfe_diagnostics->snap_chromalumafilter.lumafilter.tableneg[table_index]);
        break;

    /* Chromatix Abberation Correction---------------------------------------------
     */
    case EZT_PARMS_CAC_NZHV:
        EZLOGV("EZT_PARMS_CAC_NZHV %d",
                vfe_diagnostics->cac.nzhv);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.nzhv);
        break;
    case EZT_PARMS_CAC_THRESHEDGE:
        EZLOGV("EZT_PARMS_CAC_THRESHEDGE %d",
                vfe_diagnostics->cac.threshold_edge);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.threshold_edge);
        break;
    case EZT_PARMS_CAC_THRESHSATURATION:
        EZLOGV("EZT_PARMS_CAC_THRESHSATURATION %d",
                vfe_diagnostics->cac.threshold_saturation);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.threshold_saturation);
        break;
    case EZT_PARMS_CAC_POSSCAPPIXELS_RGHIGHTHRESH:
        EZLOGV("EZT_PARMS_CAC_POSSCAPPIXELS_RGHIGHTHRESH %d",
                vfe_diagnostics->cac.poss_cap_pixels_rg_high_thresh);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.poss_cap_pixels_rg_high_thresh);
        break;
    case EZT_PARMS_CAC_POSSCAPPIXELS_RGLOWTHRESH:
        EZLOGV("EZT_PARMS_CAC_POSSCAPPIXELS_RGLOWTHRESH %d",
                vfe_diagnostics->cac.poss_cap_pixels_rg_low_thresh);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.poss_cap_pixels_rg_low_thresh);
        break;
    case EZT_PARMS_CAC_POSSCAPPIXELS_BGHIGHTHRESH:
        EZLOGV("EZT_PARMS_CAC_POSSCAPPIXELS_BGHIGHTHRESH %d",
                vfe_diagnostics->cac.poss_cap_pixels_bg_high_thresh);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.poss_cap_pixels_bg_high_thresh);
        break;
    case EZT_PARMS_CAC_POSSCAPPIXELS_BGLOWTHRESH:
        EZLOGV("EZT_PARMS_CAC_POSSCAPPIXELS_BGLOWTHRESH %d",
                vfe_diagnostics->cac.poss_cap_pixels_bg_low_thresh);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.poss_cap_pixels_bg_low_thresh);
        break;
    case EZT_PARMS_CAC_RGDIFFHIGHTHRESH:
        EZLOGV("EZT_PARMS_CAC_RGDIFFHIGHTHRESH %d",
                vfe_diagnostics->cac.rg_diff_high_thresh);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.rg_diff_high_thresh);
        break;
    case EZT_PARMS_CAC_RGDIFFLOWTHRESH:
        EZLOGV("EZT_PARMS_CAC_RGDIFFLOWTHRESH %d",
                vfe_diagnostics->cac.rg_diff_low_thresh);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.rg_diff_low_thresh);
        break;
    case EZT_PARMS_CAC_BGDIFFHIGHTHRESH:
        EZLOGV("EZT_PARMS_CAC_BGDIFFHIGHTHRESH %d",
                vfe_diagnostics->cac.bg_diff_high_thresh);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.bg_diff_high_thresh);
        break;
    case EZT_PARMS_CAC_BGDIFFLOWTHRESH:
        EZLOGV("EZT_PARMS_CAC_BGDIFFLOWTHRESH %d",
                vfe_diagnostics->cac.bg_diff_low_thresh);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.bg_diff_low_thresh);
        break;
    case EZT_PARMS_CAC_EDGEHV:
        EZLOGV("EZT_PARMS_CAC_EDGEHV %d",
                vfe_diagnostics->cac.edge_hv);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.edge_hv);
        break;
    case EZT_PARMS_CAC_EDGEDIAG:
        EZLOGV("EZT_PARMS_CAC_EDGEDIAG %d",
                vfe_diagnostics->cac.edge_diag);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.edge_diag);
        break;
    case EZT_PARMS_CAC_NZDIAG:
        EZLOGV("EZT_PARMS_CAC_NZDIAG %d",
                vfe_diagnostics->cac.nz_diag);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.nz_diag);
        break;
    case EZT_PARMS_CAC_FORCEOFF:
        EZLOGV("EZT_PARMS_CAC_FORCEOFF %d",
                vfe_diagnostics->cac.force_off);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.force_off);
        break;
    case EZT_PARMS_CAC_INVSLOPERG:
        EZLOGV("EZT_PARMS_CAC_INVSLOPERG %d",
                vfe_diagnostics->cac.inv_slope_rg);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.inv_slope_rg);
        break;
    case EZT_PARMS_CAC_QISLOPERG:
        EZLOGV("EZT_PARMS_CAC_QISLOPERG %d",
                vfe_diagnostics->cac.qi_slope_rg);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.qi_slope_rg);
        break;
    case EZT_PARMS_CAC_INVSLOPEBG:
        EZLOGV("EZT_PARMS_CAC_INVSLOPEBG %d",
                vfe_diagnostics->cac.inv_slope_bg);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.inv_slope_bg);
        break;
    case EZT_PARMS_CAC_QISLOPEBG:
        EZLOGV("EZT_PARMS_CAC_QISLOPEBG %d",
                vfe_diagnostics->cac.qi_slope_bg);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.qi_slope_bg);
        break;
    case EZT_PARMS_CAC_GAING:
        EZLOGV("EZT_PARMS_CAC_GAING %d",
                vfe_diagnostics->cac.gain_g);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.gain_g);
        break;
    case EZT_PARMS_CAC_OFFSETG:
        EZLOGV("EZT_PARMS_CAC_OFFSETG %d",
                vfe_diagnostics->cac.offset_g);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.offset_g);
        break;
    case EZT_PARMS_CAC_GAINB:
        EZLOGV("EZT_PARMS_CAC_GAINB %d",
                vfe_diagnostics->cac.gain_b);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.gain_b);
        break;
    case EZT_PARMS_CAC_OFFSETB:
        EZLOGV("EZT_PARMS_CAC_OFFSETB %d",
                vfe_diagnostics->cac.offset_b);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.offset_b);
        break;
    case EZT_PARMS_CAC_GAINR:
        EZLOGV("EZT_PARMS_CAC_GAINR %d",
                vfe_diagnostics->cac.gain_r);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.gain_r);
        break;
    case EZT_PARMS_CAC_OFFSETR:
        EZLOGV("EZT_PARMS_CAC_OFFSETR %d",
                vfe_diagnostics->cac.offset_r);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->cac.offset_r);
        break;


    /* Linearization -------------------------------------------------------------
     */
    case EZT_PARMS_LINEAR_ENABLE:
        EZLOGV("EZT_PARMS_LINEAR_ENABLE %d",
                vfe_diagnostics->control_linear.enable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_linear.enable);
        break;
    case EZT_PARMS_LINEAR_CONTROLENABLE:
        EZLOGV("EZT_PARMS_LINEAR_CONTROLENABLE %d",
                vfe_diagnostics->control_linear.cntrlenable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_linear.cntrlenable);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_RLUTPL:
        EZLOGV("EZT_PARMS_LINEAR_PREVIEW_RLUTPL %d",
                vfe_diagnostics->prev_linear.rlut_pl[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_linear.rlut_pl[table_index]);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_RLUTBASE:
        EZLOGV("EZT_PARMS_LINEAR_PREVIEW_RLUTBASE %d",
                vfe_diagnostics->prev_linear.rlut_base[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_linear.rlut_base[table_index]);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_RLUTDELTA:
        EZLOGV("EZT_PARMS_LUMAFILTER_PREVIEW_THRESHOLDBLUE %d",
                vfe_diagnostics->prev_linear.rlut_delta[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_linear.rlut_delta[table_index]);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_GRLUTPL:
        EZLOGV("EZT_PARMS_LINEAR_PREVIEW_GRLUTPL %d",
                vfe_diagnostics->prev_linear.grlut_pl[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_linear.grlut_pl[table_index]);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_GRLUTBASE:
        EZLOGV("EZT_PARMS_LINEAR_PREVIEW_GRLUTBASE %d",
                vfe_diagnostics->prev_linear.grlut_base[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_linear.grlut_base[table_index]);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_GRLUTDELTA:
        EZLOGV("EZT_PARMS_LINEAR_PREVIEW_GRLUTDELTA %d",
                vfe_diagnostics->prev_linear.grlut_delta[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_linear.grlut_delta[table_index]);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_GBLUTPL:
        EZLOGV("EZT_PARMS_LINEAR_PREVIEW_GBLUTPL %d",
                vfe_diagnostics->prev_linear.gblut_pl[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_linear.gblut_pl[table_index]);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_GBLUTBASE:
        EZLOGV("EZT_PARMS_LINEAR_PREVIEW_GBLUTBASE %d",
                vfe_diagnostics->prev_linear.gblut_base[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_linear.gblut_base[table_index]);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_GBLUTDELTA:
        EZLOGV("EZT_PARMS_LINEAR_PREVIEW_GBLUTDELTA %d",
                vfe_diagnostics->prev_linear.gblut_delta[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_linear.gblut_delta[table_index]);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_BLUTPL:
        EZLOGV("EZT_PARMS_LINEAR_PREVIEW_BLUTPL %d",
                vfe_diagnostics->prev_linear.blut_pl[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_linear.blut_pl[table_index]);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_BLUTBASE:
        EZLOGV("EZT_PARMS_LINEAR_PREVIEW_BLUTBASE %d",
                vfe_diagnostics->prev_linear.blut_base[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_linear.blut_base[table_index]);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_BLUTDELTA:
        EZLOGV("EZT_PARMS_LINEAR_PREVIEW_BLUTDELTA %d",
                vfe_diagnostics->prev_linear.blut_delta[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_linear.blut_delta[table_index]);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_RLUTPL:
        EZLOGV("EZT_PARMS_LINEAR_SNAPSHOT_RLUTPL %d",
                vfe_diagnostics->snap_linear.rlut_pl[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_linear.rlut_pl[table_index]);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_RLUTBASE:
        EZLOGV("EZT_PARMS_LINEAR_SNAPSHOT_RLUTBASE %d",
                vfe_diagnostics->snap_linear.rlut_base[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_linear.rlut_base[table_index]);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_RLUTDELTA:
        EZLOGV("EZT_PARMS_LINEAR_SNAPSHOT_RLUTDELTA %d",
                vfe_diagnostics->snap_linear.rlut_delta[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_linear.rlut_delta[table_index]);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_GRLUTPL:
        EZLOGV("EZT_PARMS_LINEAR_SNAPSHOT_GRLUTPL %d",
                vfe_diagnostics->snap_linear.grlut_pl[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_linear.grlut_pl[table_index]);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_GRLUTBASE:
        EZLOGV("EZT_PARMS_LINEAR_SNAPSHOT_GRLUTBASE %d",
                vfe_diagnostics->snap_linear.grlut_base[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_linear.grlut_base[table_index]);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_GRLUTDELTA:
        EZLOGV("EZT_PARMS_LINEAR_SNAPSHOT_GRLUTDELTA %d",
                vfe_diagnostics->snap_linear.grlut_delta[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_linear.grlut_delta[table_index]);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_GBLUTPL:
        EZLOGV("EZT_PARMS_LINEAR_SNAPSHOT_GBLUTPL %d",
                vfe_diagnostics->snap_linear.gblut_pl[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_linear.gblut_pl[table_index]);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_GBLUTBASE:
        EZLOGV("EZT_PARMS_LINEAR_SNAPSHOT_GBLUTBASE %d",
                vfe_diagnostics->snap_linear.gblut_base[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_linear.gblut_base[table_index]);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_GBLUTDELTA:
        EZLOGV("EZT_PARMS_LINEAR_SNAPSHOT_GBLUTDELTA %d",
                vfe_diagnostics->snap_linear.gblut_delta[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_linear.gblut_delta[table_index]);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_BLUTPL:
        EZLOGV("EZT_PARMS_LINEAR_SNAPSHOT_BLUTPL %d",
                vfe_diagnostics->snap_linear.blut_pl[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_linear.blut_pl[table_index]);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_BLUTBASE:
        EZLOGV("EZT_PARMS_LINEAR_SNAPSHOT_BLUTBASE %d",
                vfe_diagnostics->snap_linear.blut_base[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_linear.blut_base[table_index]);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_BLUTDELTA:
        EZLOGV("EZT_PARMS_LINEAR_SNAPSHOT_BLUTDELTA %d",
                vfe_diagnostics->snap_linear.blut_delta[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_linear.blut_delta[table_index]);
        break;

        /* SCE -----------------------------------------------------------------------
         */
    case EZT_PARMS_SCE_DENABLE:
        EZLOGV("EZT_PARMS_SCE_DENABLE %d",
                vfe_diagnostics->control_skincolorenhan.enable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_skincolorenhan.enable);
        break;
    case EZT_PARMS_SCE_CONTROLENABLE:
        EZLOGV("EZT_PARMS_SCE_CONTROLENABLE %d",
                vfe_diagnostics->control_skincolorenhan.cntrlenable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_skincolorenhan.cntrlenable);
        break;

    case EZT_PARMS_SCE_PREVIEW_TRIANG0_VER0CR:
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_VER1CR:
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_VER2CR:
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_VER0CR:
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_VER1CR:
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_VER2CR:
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_VER0CR:
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_VER1CR:
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_VER2CR:
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_VER0CR:
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_VER1CR:
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_VER2CR:
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_VER0CR:
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_VER1CR:
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_VER2CR:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_skincolorenhan.crcoord);
        EZLOGV("EZT_PARMS_SCE_PREVIEW_CR_COORD %d",
                i32_value[item->id - EZT_PARMS_SCE_PREVIEW_TRIANG0_VER0CR]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[item->id - EZT_PARMS_SCE_PREVIEW_TRIANG0_VER0CR]);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_VER0CB:
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_VER1CB:
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_VER2CB:
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_VER0CB:
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_VER1CB:
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_VER2CB:
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_VER0CB:
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_VER1CB:
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_VER2CB:
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_VER0CB:
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_VER1CB:
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_VER2CB:
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_VER0CB:
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_VER1CB:
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_VER2CB:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_skincolorenhan.cbcoord);
        EZLOGV("EZT_PARMS_SCE_PREVIEW_CB_COORD %d",
                i32_value[item->id - EZT_PARMS_SCE_PREVIEW_TRIANG0_VER0CB]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[item->id - EZT_PARMS_SCE_PREVIEW_TRIANG0_VER0CB]);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_COEFFA:
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_COEFFB:
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_COEFFA:
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_COEFFB:
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_COEFFA:
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_COEFFB:
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_COEFFA:
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_COEFFB:
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_COEFFA:
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_COEFFB:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_skincolorenhan.crcoeff);
        EZLOGV("EZT_PARMS_SCE_PREVIEW_CR_COEFF %d",
                i32_value[item->id - EZT_PARMS_SCE_PREVIEW_TRIANG0_COEFFA]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[item->id - EZT_PARMS_SCE_PREVIEW_TRIANG0_COEFFA]);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_COEFFD:
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_COEFFE:
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_COEFFD:
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_COEFFE:
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_COEFFD:
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_COEFFE:
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_COEFFD:
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_COEFFE:
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_COEFFD:
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_COEFFE:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_skincolorenhan.cbcoeff);
        EZLOGV("EZT_PARMS_SCE_PREVIEW_CB_COEFF %d",
                i32_value[item->id - EZT_PARMS_SCE_PREVIEW_TRIANG0_COEFFD]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[item->id - EZT_PARMS_SCE_PREVIEW_TRIANG0_COEFFD]);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_COEFFC:
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_COEFFC:
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_COEFFC:
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_COEFFC:
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_COEFFC:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_skincolorenhan.croffset);
        EZLOGV("EZT_PARMS_SCE_PREVIEW_CR_OFFSET %d",
                i32_value[item->id - EZT_PARMS_SCE_PREVIEW_TRIANG0_COEFFC]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[item->id - EZT_PARMS_SCE_PREVIEW_TRIANG0_COEFFC]);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_COEFFF:
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_COEFFF:
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_COEFFF:
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_COEFFF:
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_COEFFF:
        i32_value = (int32_t *)&(vfe_diagnostics->prev_skincolorenhan.cboffset);
        EZLOGV("EZT_PARMS_SCE_PREVIEW_CB_OFFSET %d",
                i32_value[item->id - EZT_PARMS_SCE_PREVIEW_TRIANG0_COEFFF]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[item->id - EZT_PARMS_SCE_PREVIEW_TRIANG0_COEFFF]);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_MATRIXSHIFT:
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_MATRIXSHIFT:
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_MATRIXSHIFT:
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_MATRIXSHIFT:
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_MATRIXSHIFT:
        u32_value = (uint32_t *)&(vfe_diagnostics->prev_skincolorenhan.croffset.shift0);
        EZLOGV("EZT_PARMS_SCE_PREVIEW_CR_SHIFT %d",
                u32_value[item->id - EZT_PARMS_SCE_PREVIEW_TRIANG0_MATRIXSHIFT]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                u32_value[item->id - EZT_PARMS_SCE_PREVIEW_TRIANG0_MATRIXSHIFT]);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_OFFSETSHIFT:
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_OFFSETSHIFT:
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_OFFSETSHIFT:
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_OFFSETSHIFT:
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_OFFSETSHIFT:
        u32_value = (uint32_t *)&(vfe_diagnostics->prev_skincolorenhan.cboffset.shift0);
        EZLOGV("EZT_PARMS_SCE_PREVIEW_CB_SHIFT %d",
                u32_value[item->id - EZT_PARMS_SCE_PREVIEW_TRIANG0_OFFSETSHIFT]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                u32_value[item->id - EZT_PARMS_SCE_PREVIEW_TRIANG0_OFFSETSHIFT]);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_VER0CR:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_VER1CR:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_VER2CR:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_VER0CR:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_VER1CR:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_VER2CR:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_VER0CR:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_VER1CR:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_VER2CR:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_VER0CR:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_VER1CR:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_VER2CR:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_VER0CR:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_VER1CR:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_VER2CR:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_skincolorenhan.crcoord);
        EZLOGV("EZT_PARMS_SCE_SNAPSHOT_CR_COORD %d",
                i32_value[item->id - EZT_PARMS_SCE_SNAPSHOT_TRIANG0_VER0CR]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[item->id - EZT_PARMS_SCE_SNAPSHOT_TRIANG0_VER0CR]);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_VER0CB:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_VER1CB:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_VER2CB:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_VER0CB:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_VER1CB:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_VER2CB:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_VER0CB:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_VER1CB:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_VER2CB:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_VER0CB:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_VER1CB:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_VER2CB:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_VER0CB:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_VER1CB:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_VER2CB:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_skincolorenhan.cbcoord);
        EZLOGV("EZT_PARMS_SCE_SNAPSHOT_CB_COORD %d",
                i32_value[item->id - EZT_PARMS_SCE_SNAPSHOT_TRIANG0_VER0CB]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[item->id - EZT_PARMS_SCE_SNAPSHOT_TRIANG0_VER0CB]);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_COEFFA:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_COEFFB:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_COEFFA:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_COEFFB:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_COEFFA:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_COEFFB:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_COEFFA:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_COEFFB:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_COEFFA:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_COEFFB:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_skincolorenhan.crcoeff);
        EZLOGV("EZT_PARMS_SCE_SNAPSHOT_CR_COEFF %d",
                i32_value[item->id - EZT_PARMS_SCE_SNAPSHOT_TRIANG0_COEFFA]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[item->id - EZT_PARMS_SCE_SNAPSHOT_TRIANG0_COEFFA]);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_COEFFD:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_COEFFE:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_COEFFD:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_COEFFE:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_COEFFD:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_COEFFE:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_COEFFD:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_COEFFE:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_COEFFD:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_COEFFE:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_skincolorenhan.cbcoeff);
        EZLOGV("EZT_PARMS_SCE_SNAPSHOT_CB_COEFF %d",
                i32_value[item->id - EZT_PARMS_SCE_SNAPSHOT_TRIANG0_COEFFD]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[item->id - EZT_PARMS_SCE_SNAPSHOT_TRIANG0_COEFFD]);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_COEFFC:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_COEFFC:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_COEFFC:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_COEFFC:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_COEFFC:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_skincolorenhan.croffset);
        EZLOGV("EZT_PARMS_SCE_SNAPSHOT_CR_OFFSET %d",
                i32_value[item->id - EZT_PARMS_SCE_SNAPSHOT_TRIANG0_COEFFC]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[item->id - EZT_PARMS_SCE_SNAPSHOT_TRIANG0_COEFFC]);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_COEFFF:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_COEFFF:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_COEFFF:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_COEFFF:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_COEFFF:
        i32_value = (int32_t *)&(vfe_diagnostics->snap_skincolorenhan.cboffset);
        EZLOGV("EZT_PARMS_SCE_SNAPSHOT_CB_OFFSET %d",
                i32_value[item->id - EZT_PARMS_SCE_SNAPSHOT_TRIANG0_COEFFF]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[item->id - EZT_PARMS_SCE_SNAPSHOT_TRIANG0_COEFFF]);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_MATRIXSHIFT:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_MATRIXSHIFT:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_MATRIXSHIFT:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_MATRIXSHIFT:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_MATRIXSHIFT:
        u32_value = (uint32_t *)&(vfe_diagnostics->snap_skincolorenhan.croffset.shift0);
        EZLOGV("EZT_PARMS_SCE_SNAPSHOT_CR_SHIFT %d",
                u32_value[item->id - EZT_PARMS_SCE_SNAPSHOT_TRIANG0_MATRIXSHIFT]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                u32_value[item->id - EZT_PARMS_SCE_SNAPSHOT_TRIANG0_MATRIXSHIFT]);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_OFFSETSHIFT:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_OFFSETSHIFT:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_OFFSETSHIFT:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_OFFSETSHIFT:
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_OFFSETSHIFT:
        u32_value = (uint32_t *)&(vfe_diagnostics->snap_skincolorenhan.cboffset.shift0);
        EZLOGV("EZT_PARMS_SCE_SNAPSHOT_CB_SHIFT %d",
                u32_value[item->id - EZT_PARMS_SCE_SNAPSHOT_TRIANG0_OFFSETSHIFT]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                u32_value[item->id - EZT_PARMS_SCE_SNAPSHOT_TRIANG0_OFFSETSHIFT]);
        break;

        /* Demux ---------------------------------------------------------------------
         */
    case EZT_PARMS_DEMUXCHGAIN_ENABLE:
        EZLOGV("EZT_PARMS_DEMUXCHGAIN_ENABLE %d",
                vfe_diagnostics->control_demux.enable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_demux.enable);
        break;
    case EZT_PARMS_DEMUXCHGAIN_CONTROLENABLE:
        EZLOGV("EZT_PARMS_DEMUXCHGAIN_CONTROLENABLE %d",
                vfe_diagnostics->control_demux.cntrlenable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_demux.cntrlenable);
        break;
    case EZT_PARMS_DEMUXCHGAIN_PREVIEW_GREENEVENROW:
        EZLOGV("EZT_PARMS_DEMUXCHGAIN_PREVIEW_GREENEVENROW %d",
                vfe_diagnostics->prev_demuxchannelgain.greenEvenRow);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demuxchannelgain.greenEvenRow);
        break;
    case EZT_PARMS_DEMUXCHGAIN_PREVIEW_GREENODDROW:
        EZLOGV("EZT_PARMS_DEMUXCHGAIN_PREVIEW_GREENODDROW %d",
                vfe_diagnostics->prev_demuxchannelgain.greenOddRow);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demuxchannelgain.greenOddRow);
        break;
    case EZT_PARMS_DEMUXCHGAIN_PREVIEW_BLUE:
        EZLOGV("EZT_PARMS_DEMUXCHGAIN_PREVIEW_BLUE %d",
                vfe_diagnostics->prev_demuxchannelgain.blue);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demuxchannelgain.blue);
        break;
    case EZT_PARMS_DEMUXCHGAIN_PREVIEW_RED:
        EZLOGV("EZT_PARMS_DEMUXCHGAIN_PREVIEW_RED %d",
                vfe_diagnostics->prev_demuxchannelgain.red);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_demuxchannelgain.red);
        break;
    case EZT_PARMS_DEMUXCHGAIN_SNAPSHOT_GREENEVENROW:
        EZLOGV("EZT_PARMS_DEMUXCHGAIN_SNAPSHOT_GREENEVENROW %d",
                vfe_diagnostics->snap_demuxchannelgain.greenEvenRow);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demuxchannelgain.greenEvenRow);
        break;
    case EZT_PARMS_DEMUXCHGAIN_SNAPSHOT_GREENODDROW:
        EZLOGV("EZT_PARMS_DEMUXCHGAIN_SNAPSHOT_GREENODDROW %d",
                vfe_diagnostics->snap_demuxchannelgain.greenOddRow);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demuxchannelgain.greenOddRow);
        break;
    case EZT_PARMS_DEMUXCHGAIN_SNAPSHOT_BLUE:
        EZLOGV("EZT_PARMS_DEMUXCHGAIN_SNAPSHOT_BLUE %d",
                vfe_diagnostics->snap_demuxchannelgain.blue);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demuxchannelgain.blue);
        break;
    case EZT_PARMS_DEMUXCHGAIN_SNAPSHOT_RED:
        EZLOGV("EZT_PARMS_DEMUXCHGAIN_SNAPSHOT_RED %d",
                vfe_diagnostics->snap_demuxchannelgain.red);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->snap_demuxchannelgain.red);
        break;

    /* GIC */
    case EZT_PARMS_GIC_DIAG_ENABLE:
        EZLOGV("EZT_PARMS_GIC_DIAG_ENABLE %d",
                vfe_diagnostics->control_gicdiag.enable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_gicdiag.enable);
        break;
    case EZT_PARMS_GIC_DIAG_CONTROLENABLE:
        EZLOGV("EZT_PARMS_GIC_DIAG_CONTROLENABLE %d",
                vfe_diagnostics->control_gicdiag.cntrlenable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_gicdiag.cntrlenable);
        break;
    case EZT_PARMS_GIC_SOFTTHRESHNOISESCALE:
        EZLOGV("EZT_PARMS_GIC_SOFTTHRESHNOISESCALE %d",
                vfe_diagnostics->prev_gicdiag.SoftThreshNoiseScale);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_gicdiag.SoftThreshNoiseScale);
        break;
    case EZT_PARMS_GIC_SOFTTHRESHNOISESHIFT:
        EZLOGV("EZT_PARMS_GIC_SOFTTHRESHNOISESHIFT %d",
                vfe_diagnostics->prev_gicdiag.SoftThreshNoiseShift);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_gicdiag.SoftThreshNoiseShift);
        break;
    case EZT_PARMS_GIC_FILTERSTRENGTH:
        EZLOGV("EZT_PARMS_GIC_FILTERSTRENGTH %d",
                vfe_diagnostics->prev_gicdiag.FilterStrength);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_gicdiag.FilterStrength);
        break;
    case EZT_PARMS_GIC_NOISESCALE0:
        EZLOGV("EZT_PARMS_GIC_NOISESCALE0 %d",
                vfe_diagnostics->prev_gicdiag.FilterStrength);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_gicdiag.NoiseScale0);
        break;
    case EZT_PARMS_GIC_NOISESCALE1:
        EZLOGV("EZT_PARMS_GIC_NOISESCALE1 %d",
                vfe_diagnostics->prev_gicdiag.NoiseScale1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_gicdiag.NoiseScale1);
        break;
    case EZT_PARMS_GIC_NOISEOFFSET:
        EZLOGV("EZT_PARMS_GIC_NOISEOFFSET %d",
                vfe_diagnostics->prev_gicdiag.NoiseOffset);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_gicdiag.NoiseOffset);
        break;
    case EZT_PARMS_GIC_NOISESTD2LUTLEVEL0:
        EZLOGV("EZT_PARMS_GIC_NOISESTD2LUTLEVEL0 %d",
                vfe_diagnostics->prev_gicdiag.NoiseStd2LUTLevel0[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_gicdiag.NoiseStd2LUTLevel0[table_index]);
        break;
    case EZT_PARMS_GIC_SIGNAL2LUTLEVEL0:
        EZLOGV("EZT_PARMS_GIC_SIGNAL2LUTLEVEL0 %d",
                vfe_diagnostics->prev_gicdiag.Signal2LUTLevel0[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_gicdiag.Signal2LUTLevel0[table_index]);
        break;

    /* GTM */
    case EZT_PARMS_GTM_DIAG_ENABLE:
        EZLOGV("EZT_PARMS_GTM_DIAG_ENABLE %d",
                vfe_diagnostics->control_gtmdiag.enable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_gtmdiag.enable);
        break;
    case EZT_PARMS_GTM_DIAG_CONTROLENABLE:
        EZLOGV("EZT_PARMS_GTM_DIAG_CONTROLENABLE %d",
                vfe_diagnostics->control_gtmdiag.cntrlenable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_gtmdiag.cntrlenable);
        break;
    case EZT_PARMS_GTM_AMIDDLETONE:
        EZLOGV("EZT_PARMS_GTM_AMIDDLETONE %f",
                vfe_diagnostics->prev_gtmdiag.AMiddleTone);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                vfe_diagnostics->prev_gtmdiag.AMiddleTone);
        break;
    case EZT_PARMS_GTM_MAXVALTHRESH:
        EZLOGV("EZT_PARMS_GTM_MAXVALTHRESH %d",
                vfe_diagnostics->prev_gtmdiag.MaxValThresh);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_gtmdiag.MaxValThresh);
        break;
    case EZT_PARMS_GTM_KEYMINTHRESH:
        EZLOGV("EZT_PARMS_GTM_KEYMINTHRESH %d",
                vfe_diagnostics->prev_gtmdiag.KeyMinThresh);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_gtmdiag.KeyMinThresh);
        break;
    case EZT_PARMS_GTM_KEYMAXTHRESH:
        EZLOGV("EZT_PARMS_GTM_KEYMAXTHRESH %d",
                vfe_diagnostics->prev_gtmdiag.KeyMaxThresh);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_gtmdiag.KeyMaxThresh);
        break;
    case EZT_PARMS_GTM_KEYHISTBINWEIGHT:
        EZLOGV("EZT_PARMS_GTM_KEYHISTBINWEIGHT %f",
                vfe_diagnostics->prev_gtmdiag.KeyHistBinWeight);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                vfe_diagnostics->prev_gtmdiag.KeyHistBinWeight);
        break;
    case EZT_PARMS_GTM_YOUTMAXVAL:
        EZLOGV("EZT_PARMS_GTM_YOUTMAXVAL %d",
                vfe_diagnostics->prev_gtmdiag.YoutMaxVal);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_gtmdiag.YoutMaxVal);
        break;
    case EZT_PARMS_GTM_TEMPORALW:
        EZLOGV("EZT_PARMS_GTM_TEMPORALW %f",
                vfe_diagnostics->prev_gtmdiag.TemporalW);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                vfe_diagnostics->prev_gtmdiag.TemporalW);
        break;
    case EZT_PARMS_GTM_MIDDLETONEW:
        EZLOGV("EZT_PARMS_GTM_MIDDLETONEW %f",
                vfe_diagnostics->prev_gtmdiag.MiddleToneW);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                vfe_diagnostics->prev_gtmdiag.MiddleToneW);
        break;
    case EZT_PARMS_GTM_LUTBANKSEL:
        EZLOGV("EZT_PARMS_GTM_LUTBANKSEL %d",
                vfe_diagnostics->prev_gtmdiag.LUTBankSel);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_gtmdiag.LUTBankSel);
        break;
    case EZT_PARMS_GTM_XARR:
        EZLOGV("EZT_PARMS_GTM_XARR %d",
                vfe_diagnostics->prev_gtmdiag.Xarr[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_gtmdiag.Xarr[table_index]);
        break;
    case EZT_PARMS_GTM_YRATIOBASE:
        EZLOGV("EZT_PARMS_GTM_YRATIOBASE %f",
                vfe_diagnostics->prev_gtmdiag.YRatioBase[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                vfe_diagnostics->prev_gtmdiag.YRatioBase[table_index]);
        break;
    case EZT_PARMS_GTM_YRATIOSLOPE:
        EZLOGV("EZT_PARMS_GTM_YRATIOSLOPE %d",
                vfe_diagnostics->prev_gtmdiag.YRatioSlope[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_gtmdiag.YRatioSlope[table_index]);
        break;

    /* Pedestal */
    case EZT_PARMS_PEDESTAL_DIAG_ENABLE:
        EZLOGV("EZT_PARMS_PEDESTAL_DIAG_ENABLE %d",
                vfe_diagnostics->control_pedestaldiag.enable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_pedestaldiag.enable);
        break;
    case EZT_PARMS_PEDESTAL_DIAG_CONTROLENABLE:
        EZLOGV("EZT_PARMS_PEDESTAL_DIAG_CONTROLENABLE %d",
                vfe_diagnostics->control_pedestaldiag.cntrlenable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->control_pedestaldiag.cntrlenable);
        break;
    case EZT_PARMS_PEDESTAL_HDRENABLE:
        EZLOGV("EZT_PARMS_PEDESTAL_HDRENABLE %d",
                vfe_diagnostics->prev_pedestaldiag.HDREnable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.HDREnable);
        break;
    case EZT_PARMS_PEDESTAL_SCALEBYPASS:
        EZLOGV("EZT_PARMS_PEDESTAL_SCALEBYPASS %d",
                vfe_diagnostics->prev_pedestaldiag.ScaleBypass);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.ScaleBypass);
        break;
    case EZT_PARMS_PEDESTAL_INTPFACTOR:
        EZLOGV("EZT_PARMS_PEDESTAL_INTPFACTOR %d",
                vfe_diagnostics->prev_pedestaldiag.IntpFactor);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.IntpFactor);
        break;
    case EZT_PARMS_PEDESTAL_BWIDTH:
        EZLOGV("EZT_PARMS_PEDESTAL_BWIDTH %d",
                vfe_diagnostics->prev_pedestaldiag.BWidth);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.BWidth);
        break;
    case EZT_PARMS_PEDESTAL_BHEIGHT:
        EZLOGV("EZT_PARMS_PEDESTAL_BHEIGHT %d",
                vfe_diagnostics->prev_pedestaldiag.BHeight);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.BHeight);
        break;
    case EZT_PARMS_PEDESTAL_XDELTA:
        EZLOGV("EZT_PARMS_PEDESTAL_XDELTA %d",
                vfe_diagnostics->prev_pedestaldiag.XDelta);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.XDelta);
        break;
    case EZT_PARMS_PEDESTAL_YDELTA:
        EZLOGV("EZT_PARMS_PEDESTAL_YDELTA %d",
                vfe_diagnostics->prev_pedestaldiag.YDelta);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.YDelta);
        break;
    case EZT_PARMS_PEDESTAL_LEFTIMAGEWD:
        EZLOGV("EZT_PARMS_PEDESTAL_LEFTIMAGEWD %d",
                vfe_diagnostics->prev_pedestaldiag.LeftImageWD);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.LeftImageWD);
        break;
    case EZT_PARMS_PEDESTAL_ENABLE3D:
        EZLOGV("EZT_PARMS_PEDESTAL_ENABLE3D %d",
                vfe_diagnostics->prev_pedestaldiag.Enable3D);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.Enable3D);
        break;
    case EZT_PARMS_PEDESTAL_MESHGRIDBWIDTH:
        EZLOGV("EZT_PARMS_PEDESTAL_MESHGRIDBWIDTH %d",
                vfe_diagnostics->prev_pedestaldiag.MeshGridBWidth);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.MeshGridBWidth);
        break;
    case EZT_PARMS_PEDESTAL_MESHGRIDBHEIGHT:
        EZLOGV("EZT_PARMS_PEDESTAL_MESHGRIDBHEIGHT %d",
                vfe_diagnostics->prev_pedestaldiag.MeshGridBHeight);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.MeshGridBHeight);
        break;
    case EZT_PARMS_PEDESTAL_LXSTART:
        EZLOGV("EZT_PARMS_PEDESTAL_LXSTART %d",
                vfe_diagnostics->prev_pedestaldiag.LXStart);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.LXStart);
        break;
    case EZT_PARMS_PEDESTAL_LYSTART:
        EZLOGV("EZT_PARMS_PEDESTAL_LYSTART %d",
                vfe_diagnostics->prev_pedestaldiag.LYStart);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.LYStart);
        break;
    case EZT_PARMS_PEDESTAL_BXSTART:
        EZLOGV("EZT_PARMS_PEDESTAL_BXSTART %d",
                vfe_diagnostics->prev_pedestaldiag.BXStart);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.BXStart);
        break;
    case EZT_PARMS_PEDESTAL_BYSTART:
        EZLOGV("EZT_PARMS_PEDESTAL_BYSTART %d",
                vfe_diagnostics->prev_pedestaldiag.BYStart);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.BYStart);
        break;
    case EZT_PARMS_PEDESTAL_BXD1:
        EZLOGV("EZT_PARMS_PEDESTAL_BXD1 %d",
                vfe_diagnostics->prev_pedestaldiag.BXD1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.BXD1);
        break;
    case EZT_PARMS_PEDESTAL_BYE1:
        EZLOGV("EZT_PARMS_PEDESTAL_BYE1 %d",
                vfe_diagnostics->prev_pedestaldiag.BYE1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.BYE1);
        break;
    case EZT_PARMS_PEDESTAL_BYINITE1:
        EZLOGV("EZT_PARMS_PEDESTAL_BYINITE1 %d",
                vfe_diagnostics->prev_pedestaldiag.BYInitE1);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.BYInitE1);
        break;
    case EZT_PARMS_PEDESTAL_MESHTABLET1_R:
        EZLOGV("EZT_PARMS_PEDESTAL_MESHTABLET1_R %d",
                vfe_diagnostics->prev_pedestaldiag.MeshTableT1_R[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.MeshTableT1_R[table_index]);
        break;
    case EZT_PARMS_PEDESTAL_MESHTABLET1_GR:
        EZLOGV("EZT_PARMS_PEDESTAL_MESHTABLET1_GR %d",
                vfe_diagnostics->prev_pedestaldiag.MeshTableT1_Gr[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.MeshTableT1_Gr[table_index]);
        break;
    case EZT_PARMS_PEDESTAL_MESHTABLET1_GB:
        EZLOGV("EZT_PARMS_PEDESTAL_MESHTABLET1_GB %d",
                vfe_diagnostics->prev_pedestaldiag.MeshTableT1_Gb[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.MeshTableT1_Gb[table_index]);
        break;
    case EZT_PARMS_PEDESTAL_MESHTABLET1_B:
        EZLOGV("EZT_PARMS_PEDESTAL_MESHTABLET1_B %d",
                vfe_diagnostics->prev_pedestaldiag.MeshTableT1_B[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.MeshTableT1_B[table_index]);
        break;
    case EZT_PARMS_PEDESTAL_MESHTABLET2_R:
        EZLOGV("EZT_PARMS_PEDESTAL_MESHTABLET2_R %d",
                vfe_diagnostics->prev_pedestaldiag.MeshTableT2_R[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.MeshTableT2_R[table_index]);
        break;
    case EZT_PARMS_PEDESTAL_MESHTABLET2_GR:
        EZLOGV("EZT_PARMS_PEDESTAL_MESHTABLET2_GR %d",
                vfe_diagnostics->prev_pedestaldiag.MeshTableT2_Gr[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.MeshTableT2_Gr[table_index]);
        break;
    case EZT_PARMS_PEDESTAL_MESHTABLET2_GB:
        EZLOGV("EZT_PARMS_PEDESTAL_MESHTABLET2_GB %d",
                vfe_diagnostics->prev_pedestaldiag.MeshTableT2_Gb[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.MeshTableT2_Gb[table_index]);
        break;
    case EZT_PARMS_PEDESTAL_MESHTABLET2_B:
        EZLOGV("EZT_PARMS_PEDESTAL_MESHTABLET2_B %d",
                vfe_diagnostics->prev_pedestaldiag.MeshTableT2_B[table_index]);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                vfe_diagnostics->prev_pedestaldiag.MeshTableT2_B[table_index]);
        break;

    // AWB /////////////////////////////////////////////////////////////////////////
    /* AWB ---------------------------------------------------------------------
     */
    case EZT_PARMS_AWB_MODE:
        EZLOGV("EZT_PARMS_AWB_MODE %d", awb_params->awb_mode);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->awb_mode);
        break;
    case EZT_PARMS_AWB_ENABLE:
        EZLOGV("EZT_PARMS_AWB_ENABLE %d", awb_params->awb_enable);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->awb_enable);
        break;
    case EZT_PARMS_AWB_LOCK:
        EZLOGV("EZT_PARMS_AWB_LOCK %d", awb_params->lock);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->lock);
        break;
    case EZT_PARMS_AWB_PREVIEW_RGAIN:
        EZLOGV("EZT_PARMS_AWB_PREVIEW_RGAIN %f", awb_params->preview_r_gain);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->preview_r_gain);
        break;
    case EZT_PARMS_AWB_PREVIEW_GGAIN:
        EZLOGV("EZT_PARMS_AWB_PREVIEW_GGAIN %f", awb_params->preview_g_gain);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->preview_g_gain);
        break;
    case EZT_PARMS_AWB_PREVIEW_BGAIN:
        EZLOGV("EZT_PARMS_AWB_PREVIEW_BGAIN %f", awb_params->preview_b_gain);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->preview_b_gain);
        break;
    case EZT_PARMS_AWB_SNAPSHOT_RGAIN:
        EZLOGV("EZT_PARMS_AWB_SNAPSHOT_RGAIN %f", awb_params->snapshot_r_gain);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->snapshot_r_gain);
        break;
    case EZT_PARMS_AWB_SNAPSHOT_GGAIN:
        EZLOGV("EZT_PARMS_AWB_SNAPSHOT_GGAIN %f", awb_params->snapshot_g_gain);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->snapshot_g_gain);
        break;
    case EZT_PARMS_AWB_SNAPSHOT_BGAIN:
        EZLOGV("EZT_PARMS_AWB_SNAPSHOT_BGAIN %f", awb_params->snapshot_b_gain);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->snapshot_b_gain);
        break;
    case EZT_PARMS_AWB_COLOR_TEMP:
        EZLOGV("EZT_PARMS_AWB_COLOR_TEMP %d", awb_params->color_temp);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->color_temp);
        break;
    case EZT_PARMS_AWB_DECISION:
        EZLOGV("EZT_PARMS_AWB_DECISION %d", awb_params->decision);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->decision);
        break;
    case EZT_PARMS_AWB_SAMPLEDECISION:
        i32_value = (int32_t *)&(awb_params->samp_decision);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                i32_value[table_index]);
        break;
    case EZT_PARMS_AWB_DECISIONSTRING:
        switch (awb_params->decision) {
            case 0: ch_value = "SUNLIGHT-D65"; break;
            case 1: ch_value = "CLOUDY-D75"; break;
            case 2: ch_value = "INCANDESCENT-A"; break;
            case 3: ch_value = "FLUORESCENT-TL84"; break;
            case 4: ch_value = "FLUORESCENT-CW"; break;
            case 5: ch_value = "HORIZON-H"; break;
            case 6: ch_value = "SUNLIGHT-D50"; break;
            case 7: ch_value = "FLUORESCENT-CUS"; break;
            case 8: ch_value = "NOON"; break;
            case 9: ch_value = "DAYLIGHT-CUS"; break;
            case 10: ch_value = "A-CUS"; break;
            case 11: ch_value = "U30"; break;
            case 12: ch_value = "DAY-LINE1"; break;
            case 13: ch_value = "DAY-LINE2"; break;
            case 14: ch_value = "FINE"; break;
            case 15: ch_value = "A-LINE1"; break;
            case 16: ch_value = "A-LINE2"; break;
            case 17: ch_value = "HYBRID"; break;
            default: ch_value = "BAD"; break;
        }
        EZLOGV("EZT_PARMS_AWB_DECISIONSTRING %s", ch_value);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%s", ch_value);
        break;
    case EZT_PARMS_AWB_PREVEXPINDEX:
        EZLOGV("EZT_PARMS_AWB_PREVEXPINDEX %d",
                awb_params->prev_exp_index);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->prev_exp_index);
        break;
    case EZT_PARMS_AWB_OUTLIER_DIST2_DAYLEFT:
        EZLOGV("EZT_PARMS_AWB_OUTLIER_DIST2_DAYLEFT %d",
                awb_params->outlier_dist2_dayleft);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->outlier_dist2_dayleft);
        break;
    case EZT_PARMS_AWB_OUTLIER_DIST2_DAYTOP:
        EZLOGV("EZT_PARMS_AWB_OUTLIER_DIST2_DAYTOP %d",
                awb_params->outlier_dist2_daytop);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->outlier_dist2_daytop);
        break;
    case EZT_PARMS_AWB_VALIDSAMPLECOUNT:
        EZLOGV("EZT_PARMS_AWB_VALIDSAMPLECOUNT %d",
                awb_params->valid_sample_cnt);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->valid_sample_cnt);
        break;
    case EZT_PARMS_AWB_NOUTLIER:
        EZLOGV("EZT_PARMS_AWB_NOUTLIER %d", awb_params->n_outlier);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->n_outlier);
        break;
    case EZT_PARMS_AWB_DAY_RGRATIO:
        EZLOGV("EZT_PARMS_AWB_DAY_RGRATIO %f",
                awb_params->day_rg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->day_rg_ratio);
        break;
    case EZT_PARMS_AWB_DAY_BGRATIO:
        EZLOGV("EZT_PARMS_AWB_DAY_BGRATIO %f",
                awb_params->day_bg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->day_bg_ratio);
        break;
    case EZT_PARMS_AWB_DAY_CLUSTER:
        EZLOGV("EZT_PARMS_AWB_DAY_CLUSTER %d", awb_params->day_cluster);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->day_cluster);
        break;
    case EZT_PARMS_AWB_DAY_CLUSTER_WEIGHTDISTANCE:
        EZLOGV("EZT_PARMS_AWB_DAY_CLUSTER_WEIGHTDISTANCE %d",
                awb_params->day_cluster_weight_distance);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->day_cluster_weight_distance);
        break;
    case EZT_PARMS_AWB_DAY_CLUSTER_WEIGHTILLUMINANT:
        EZLOGV("EZT_PARMS_AWB_DAY_CLUSTER_WEIGHTILLUMINANT %d",
                awb_params->day_cluster_weight_illuminant);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->day_cluster_weight_illuminant);
        break;
    case EZT_PARMS_AWB_DAY_CLUSTER_WEIGHTDISILL:
        EZLOGV("EZT_PARMS_AWB_DAY_CLUSTER_WEIGHTDISILL %d",
                awb_params->day_cluster_weight_dis_ill);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->day_cluster_weight_dis_ill);
        break;
    case EZT_PARMS_AWB_F_RGRATIO:
        EZLOGV("EZT_PARMS_AWB_F_RGRATIO %f", awb_params->f_rg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->f_rg_ratio);
        break;
    case EZT_PARMS_AWB_F_BGRATIO:
        EZLOGV("EZT_PARMS_AWB_F_BGRATIO %f", awb_params->f_bg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->f_bg_ratio);
        break;
    case EZT_PARMS_AWB_F_CLUSTER:
        EZLOGV("EZT_PARMS_AWB_F_CLUSTER %d", awb_params->f_cluster);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->f_cluster);
        break;
    case EZT_PARMS_AWB_F_CLUSTER_WEIGHTDISTANCE:
        EZLOGV("EZT_PARMS_AWB_F_CLUSTER_WEIGHTDISTANCE %d",
                awb_params->f_cluster_weight_distance);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->f_cluster_weight_distance);
        break;
    case EZT_PARMS_AWB_F_CLUSTER_WEIGHTILLUMINANT:
        EZLOGV("EZT_PARMS_AWB_F_CLUSTER_WEIGHTILLUMINANT %d",
                awb_params->f_cluster_weight_illuminant);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->f_cluster_weight_illuminant);
        break;
    case EZT_PARMS_AWB_F_CLUSTER_WEIGHTDISILL:
        EZLOGV("EZT_PARMS_AWB_F_CLUSTER_WEIGHTDISILL %d",
                awb_params->f_cluster_weight_dis_ill);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->f_cluster_weight_dis_ill);
        break;
    case EZT_PARMS_AWB_A_RGRATIO:
        EZLOGV("EZT_PARMS_AWB_A_RGRATIO %f", awb_params->a_rg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->a_rg_ratio);
        break;
    case EZT_PARMS_AWB_A_BGRATIO:
        EZLOGV("EZT_PARMS_AWB_A_BGRATIO %f", awb_params->a_bg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->a_bg_ratio);
        break;
    case EZT_PARMS_AWB_A_CLUSTER:
        EZLOGV("EZT_PARMS_AWB_A_CLUSTER %d", awb_params->a_cluster);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->a_cluster);
        break;
    case EZT_PARMS_AWB_A_CLUSTER_WEIGHTDISTANCE:
        EZLOGV("EZT_PARMS_AWB_A_CLUSTER_WEIGHTDISTANCE %d",
                awb_params->a_cluster_weight_distance);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->a_cluster_weight_distance);
        break;
    case EZT_PARMS_AWB_A_CLUSTER_WEIGHTILLUMINANT:
        EZLOGV("EZT_PARMS_AWB_A_CLUSTER_WEIGHTILLUMINANT %d",
                awb_params->a_cluster_weight_illuminant);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->a_cluster_weight_illuminant);
        break;
    case EZT_PARMS_AWB_A_CLUSTER_WEIGHTDISILL:
        EZLOGV("EZT_PARMS_AWB_A_CLUSTER_WEIGHTDISILL %d",
                awb_params->a_cluster_weight_dis_ill);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->a_cluster_weight_dis_ill);
        break;
    case EZT_PARMS_AWB_H_RGRATIO:
        EZLOGV("EZT_PARMS_AWB_H_RGRATIO %f", awb_params->h_rg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->h_rg_ratio);
        break;
    case EZT_PARMS_AWB_H_BGRATIO:
        EZLOGV("EZT_PARMS_AWB_H_BGRATIO %f", awb_params->h_bg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->h_bg_ratio);
        break;
    case EZT_PARMS_AWB_H_CLUSTER:
        EZLOGV("EZT_PARMS_AWB_H_CLUSTER %d", awb_params->h_cluster);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->h_cluster);
        break;
    case EZT_PARMS_AWB_H_CLUSTER_WEIGHTDISTANCE:
        EZLOGV("EZT_PARMS_AWB_H_CLUSTER_WEIGHTDISTANCE %d",
                awb_params->h_cluster_weight_distance);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->h_cluster_weight_distance);
        break;
    case EZT_PARMS_AWB_H_CLUSTER_WEIGHTILLUMINANT:
        EZLOGV("EZT_PARMS_AWB_H_CLUSTER_WEIGHTILLUMINANT %d",
                awb_params->h_cluster_weight_illuminant);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->h_cluster_weight_illuminant);
        break;
    case EZT_PARMS_AWB_H_CLUSTER_WEIGHTDISILL:
        EZLOGV("EZT_PARMS_AWB_H_CLUSTER_WEIGHTDISILL %d",
                awb_params->h_cluster_weight_dis_ill);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->h_cluster_weight_dis_ill);
        break;
    case EZT_PARMS_AWB_SGW_COUNT:
        EZLOGV("EZT_PARMS_AWB_SGW_COUNT %d", awb_params->sgw_cnt);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->sgw_cnt);
        break;
    case EZT_PARMS_AWB_SGW_RG_RATIO:
        EZLOGV("EZT_PARMS_AWB_SGW_RG_RATIO %f",
                awb_params->sgw_rg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->sgw_rg_ratio);
        break;
    case EZT_PARMS_AWB_SGW_BG_RATIO:
        EZLOGV("EZT_PARMS_AWB_SGW_BG_RATIO %f",
                awb_params->sgw_bg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->sgw_bg_ratio);
        break;
    case EZT_PARMS_AWB_GREENLINE_MX:
        EZLOGV("EZT_PARMS_AWB_GREENLINE_MX %d",
                awb_params->green_line_mx);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->green_line_mx);
        break;
    case EZT_PARMS_AWB_GREENLINE_BX:
        EZLOGV("EZT_PARMS_AWB_GREENLINE_BX %d",
                awb_params->green_line_bx);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->green_line_bx);
        break;
    case EZT_PARMS_AWB_GREENZONE_TOP:
        EZLOGV("EZT_PARMS_AWB_GREENZONE_TOP %d",
                awb_params->green_zone_top);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->green_zone_top);
        break;
    case EZT_PARMS_AWB_GREENZONE_BOTTOM:
        EZLOGV("EZT_PARMS_AWB_GREENZONE_BOTTOM %d",
                awb_params->green_zone_bottom);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->green_zone_bottom);
        break;
    case EZT_PARMS_AWB_GREENZONE_LEFT:
        EZLOGV("EZT_PARMS_AWB_GREENZONE_LEFT %d",
                awb_params->green_zone_left);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->green_zone_left);
        break;
    case EZT_PARMS_AWB_GREENZONE_RIGHT:
        EZLOGV("EZT_PARMS_AWB_GREENZONE_RIGHT %d",
                awb_params->green_zone_right);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->green_zone_right);
        break;
    case EZT_PARMS_AWB_OUTDOOR_GREEN_RGRATIO:
        EZLOGV("EZT_PARMS_AWB_OUTDOOR_GREEN_RGRATIO %f",
                awb_params->outdoor_green_rg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->outdoor_green_rg_ratio);
        break;
    case EZT_PARMS_AWB_OUTDOOR_GREEN_BGRATIO:
        EZLOGV("EZT_PARMS_AWB_OUTDOOR_GREEN_BGRATIO %f",
                awb_params->outdoor_green_bg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->outdoor_green_bg_ratio);
        break;
    case EZT_PARMS_AWB_OUTDOOR_GREEN_GREY_RGRATIO:
        EZLOGV("EZT_PARMS_AWB_OUTDOOR_GREEN_GREY_RGRATIO %f",
                awb_params->outdoor_green_grey_rg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->outdoor_green_grey_rg_ratio);
        break;
    case EZT_PARMS_AWB_OUTDOOR_GREEN_GREY_BGRATIO:
        EZLOGV("EZT_PARMS_AWB_OUTDOOR_GREEN_GREY_BGRATIO %f",
                awb_params->outdoor_green_grey_bg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->outdoor_green_grey_bg_ratio);
        break;
    case EZT_PARMS_AWB_OUTDOOR_GREEN_COUNT:
        EZLOGV("EZT_PARMS_AWB_OUTDOOR_GREEN_COUNT %d",
                awb_params->outdoor_green_cnt);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->outdoor_green_cnt);
        break;
    case EZT_PARMS_AWB_GREEN_PERCENT:
        EZLOGV("EZT_PARMS_AWB_GREEN_PERCENT %d",
                awb_params->green_percent);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->green_percent);
        break;
    case EZT_PARMS_AWB_SLOPE_FACTOR_M:
        EZLOGV("EZT_PARMS_AWB_SLOPE_FACTOR_M %f",
                awb_params->slope_factor_m);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->slope_factor_m);
        break;
    case EZT_PARMS_AWB_EXTREME_BMAG:
        EZLOGV("EZT_PARMS_AWB_EXTREME_BMAG %d",
                awb_params->extreme_b_mag);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->extreme_b_mag);
        break;
    case EZT_PARMS_AWB_NONEXTREME_BMAG:
        EZLOGV("EZT_PARMS_AWB_NONEXTREME_BMAG %d",
                awb_params->nonextreme_b_mag);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->nonextreme_b_mag);
        break;
    case EZT_PARMS_AWB_AVE_RGRATIOX:
        EZLOGV("EZT_PARMS_AWB_AVE_RGRATIOX %d",
                awb_params->ave_rg_ratio_x);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->ave_rg_ratio_x);
        break;
    case EZT_PARMS_AWB_AVE_BGRATIOX:
        EZLOGV("EZT_PARMS_AWB_AVE_BGRATIOX %d",
                awb_params->ave_bg_ratio_x);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->ave_bg_ratio_x);
        break;
    case EZT_PARMS_AWB_WEIGHTEDSAMPLE_RGGRID:
        EZLOGV("EZT_PARMS_AWB_WEIGHTEDSAMPLE_RGGRID %d",
                awb_params->weighted_sample_rg_grid);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->weighted_sample_rg_grid);
        break;
    case EZT_PARMS_AWB_WEIGHTEDSAMPLE_BGGRID:
        EZLOGV("EZT_PARMS_AWB_WEIGHTEDSAMPLE_BGGRID %d",
                awb_params->weighted_sample_bg_grid);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->weighted_sample_bg_grid);
        break;
    case EZT_PARMS_AWB_WEIGHTEDSAMPLE_DAY_RGRATIO:
        EZLOGV("EZT_PARMS_AWB_WEIGHTEDSAMPLE_DAY_RGRATIO %f",
                awb_params->weighted_sample_day_rg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->weighted_sample_day_rg_ratio);
        break;
    case EZT_PARMS_AWB_WEIGHTEDSAMPLE_DAY_BGRATIO:
        EZLOGV("EZT_PARMS_AWB_WEIGHTEDSAMPLE_DAY_BGRATIO %f",
                awb_params->weighted_sample_day_bg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->weighted_sample_day_bg_ratio);
        break;
    case EZT_PARMS_AWB_WEIGHTEDSAMPLE_DAY_SHADE_RGRATIO:
        EZLOGV("EZT_PARMS_AWB_WEIGHTEDSAMPLE_DAY_SHADE_RGRATIO %f",
                awb_params->weighted_sample_day_shade_rg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->weighted_sample_day_shade_rg_ratio);
        break;
    case EZT_PARMS_AWB_WEIGHTEDSAMPLE_DAY_SHADE_BGRATIO:
        EZLOGV("EZT_PARMS_AWB_WEIGHTEDSAMPLE_DAY_SHADE_BGRATIO %f",
                awb_params->weighted_sample_day_shade_bg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->weighted_sample_day_shade_bg_ratio);
        break;
    case EZT_PARMS_AWB_WEIGHTEDSAMPLE_DAY_D50_RGRATIO:
        EZLOGV("EZT_PARMS_AWB_WEIGHTEDSAMPLE_DAY_D50_RGRATIO %f",
                awb_params->weighted_sample_day_d50_rg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->weighted_sample_day_d50_rg_ratio);
        break;
    case EZT_PARMS_AWB_WEIGHTEDSAMPLE_DAY_D50_BGRATIO:
        EZLOGV("EZT_PARMS_AWB_WEIGHTEDSAMPLE_DAY_D50_BGRATIO %f",
                awb_params->weighted_sample_day_d50_bg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->weighted_sample_day_d50_bg_ratio);
        break;
    case EZT_PARMS_AWB_WEIGHTEDSAMPLE_FAH_RGRATIO:
        EZLOGV("EZT_PARMS_AWB_WEIGHTEDSAMPLE_FAH_RGRATIO %f",
                awb_params->weighted_sample_fah_rg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->weighted_sample_fah_rg_ratio);
        break;
    case EZT_PARMS_AWB_WEIGHTEDSAMPLE_FAH_BGRATIO:
        EZLOGV("EZT_PARMS_AWB_WEIGHTEDSAMPLE_FAH_BGRATIO %f",
                awb_params->weighted_sample_fah_bg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->weighted_sample_fah_bg_ratio);
        break;
    case EZT_PARMS_AWB_WHITE_RGRATIO:
        EZLOGV("EZT_PARMS_AWB_WHITE_RGRATIO %f",
                awb_params->white_rg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->white_rg_ratio);
        break;
    case EZT_PARMS_AWB_WHITE_BGRATIO:
        EZLOGV("EZT_PARMS_AWB_WHITE_BGRATIO %f",
                awb_params->white_bg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->white_bg_ratio);
        break;
    case EZT_PARMS_AWB_WHITE_STATYTHRESHOLDLOW:
        EZLOGV("EZT_PARMS_AWB_WHITE_STATYTHRESHOLDLOW %d",
                awb_params->white_stat_y_threshold_low);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->white_stat_y_threshold_low);
        break;
    case EZT_PARMS_AWB_UNSAT_YMINTHRESHOLD:
        EZLOGV("EZT_PARMS_AWB_UNSAT_YMINTHRESHOLD %d",
                awb_params->unsat_y_min_threshold);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->unsat_y_min_threshold);
        break;
    case EZT_PARMS_AWB_UNSAT_Y_MAX:
        EZLOGV("EZT_PARMS_AWB_UNSAT_Y_MAX %d", awb_params->unsat_y_max);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->unsat_y_max);
        break;
    case EZT_PARMS_AWB_UNSAT_Y_MID:
        EZLOGV("EZT_PARMS_AWB_UNSAT_Y_MID %d", awb_params->unsat_y_mid);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->unsat_y_mid);
        break;
    case EZT_PARMS_AWB_UNSAT_Y_DAY_MAX:
        EZLOGV("EZT_PARMS_AWB_UNSAT_Y_DAY_MAX %d",
                awb_params->unsat_y_day_max);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->unsat_y_day_max);
        break;
    case EZT_PARMS_AWB_UNSAT_Y_F_MAX:
        EZLOGV("EZT_PARMS_AWB_UNSAT_Y_F_MAX %d",
                awb_params->unsat_y_f_max);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->unsat_y_f_max);
        break;
    case EZT_PARMS_AWB_UNSAT_Y_A_MAX:
        EZLOGV("EZT_PARMS_AWB_UNSAT_Y_A_MAX %d",
                awb_params->unsat_y_a_max);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->unsat_y_a_max);
        break;
    case EZT_PARMS_AWB_UNSAT_Y_H_MAX:
        EZLOGV("EZT_PARMS_AWB_UNSAT_Y_H_MAX %d",
                awb_params->unsat_y_h_max);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->unsat_y_h_max);
        break;
    case EZT_PARMS_AWB_SAT_DAY_RGRATIO:
        EZLOGV("EZT_PARMS_AWB_SAT_DAY_RGRATIO %f",
                awb_params->sat_day_rg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->sat_day_rg_ratio);
        break;
    case EZT_PARMS_AWB_SAT_DAY_BGRATIO:
        EZLOGV("EZT_PARMS_AWB_SAT_DAY_BGRATIO %f",
                awb_params->sat_day_bg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->sat_day_bg_ratio);
        break;
    case EZT_PARMS_AWB_SAT_DAY_CLUSTER:
        EZLOGV("EZT_PARMS_AWB_SAT_DAY_CLUSTER %d",
                awb_params->sat_day_cluster);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->sat_day_cluster);
        break;
    case EZT_PARMS_AWB_SAT_F_RGRATIO:
        EZLOGV("EZT_PARMS_AWB_SAT_F_RGRATIO %f",
                awb_params->sat_f_rg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->sat_f_rg_ratio);
        break;
    case EZT_PARMS_AWB_SAT_F_BGRATIO:
        EZLOGV("EZT_PARMS_AWB_SAT_F_BGRATIO %f",
                awb_params->sat_f_bg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->sat_f_bg_ratio);
        break;
    case EZT_PARMS_AWB_SAT_F_CLUSTER:
        EZLOGV("EZT_PARMS_AWB_SAT_F_CLUSTER %d",
                awb_params->sat_f_cluster);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->sat_f_cluster);
        break;
    case EZT_PARMS_AWB_SAT_A_RGRATIO:
        EZLOGV("EZT_PARMS_AWB_SAT_A_RGRATIO %f",
                awb_params->sat_a_rg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->sat_a_rg_ratio);
        break;
    case EZT_PARMS_AWB_SAT_A_BGRATIO:
        EZLOGV("EZT_PARMS_AWB_SAT_A_BGRATIO %f",
                awb_params->sat_a_bg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->sat_a_bg_ratio);
        break;
    case EZT_PARMS_AWB_SAT_A_CLUSTER:
        EZLOGV("EZT_PARMS_AWB_SAT_A_CLUSTER %d",
                awb_params->sat_a_cluster);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->sat_a_cluster);
        break;
    case EZT_PARMS_AWB_SAT_H_RGRATIO:
        EZLOGV("EZT_PARMS_AWB_SAT_H_RGRATIO %f",
                awb_params->sat_h_rg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->sat_h_rg_ratio);
        break;
    case EZT_PARMS_AWB_SAT_H_BGRATIO:
        EZLOGV("EZT_PARMS_AWB_SAT_H_BGRATIO %f",
                awb_params->sat_h_bg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->sat_h_bg_ratio);
        break;
    case EZT_PARMS_AWB_SAT_H_CLUSTER:
        EZLOGV("EZT_PARMS_AWB_SAT_H_CLUSTER %d",
                awb_params->sat_h_cluster);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->sat_h_cluster);
        break;
    case EZT_PARMS_AWB_MAX_COMPACT_CLUSTER:
        EZLOGV("EZT_PARMS_AWB_MAX_COMPACT_CLUSTER %f",
                awb_params->max_compact_cluster);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->max_compact_cluster);
        break;
    case EZT_PARMS_AWB_COUNT_EXTREMEB_MCC:
        EZLOGV("EZT_PARMS_AWB_COUNT_EXTREMEB_MCC %d",
                awb_params->count_extreme_b_mcc);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->count_extreme_b_mcc);
        break;
    case EZT_PARMS_AWB_GREENZONE_RIGHT2:
        EZLOGV("EZT_PARMS_AWB_GREENZONE_RIGHT2 %d",
                awb_params->green_zone_right2);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->green_zone_right2);
        break;
    case EZT_PARMS_AWB_GREENLINE_BX2:
        EZLOGV("EZT_PARMS_AWB_GREENLINE_BX2 %d",
                awb_params->green_line_bx2);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->green_line_bx2);
        break;
    case EZT_PARMS_AWB_GREENZONE_BOTTOM2:
        EZLOGV("EZT_PARMS_AWB_GREENZONE_BOTTOM2 %d",
                awb_params->green_zone_bottom2);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->green_zone_bottom2);
        break;
    case EZT_PARMS_AWB_OUTPUT_ISCONFIDENT:
        EZLOGV("EZT_PARMS_AWB_OUTPUT_ISCONFIDENT %d",
                awb_params->output_is_confident);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->output_is_confident);
        break;
    case EZT_PARMS_AWB_OUTPUT_SAMPLEDECISION:
        EZLOGV("EZT_PARMS_AWB_OUTPUT_SAMPLEDECISION %d",
                awb_params->output_sample_decision);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->output_sample_decision);
        break;
    case EZT_PARMS_AWB_OUTPUT_WBGAIN_R:
        EZLOGV("EZT_PARMS_AWB_OUTPUT_WBGAIN_R %f",
                awb_params->output_wb_gain_r);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->output_wb_gain_r);
        break;
    case EZT_PARMS_AWB_OUTPUT_WBGAIN_G:
        EZLOGV("EZT_PARMS_AWB_OUTPUT_WBGAIN_G %f",
                awb_params->output_wb_gain_g);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->output_wb_gain_g);
        break;
    case EZT_PARMS_AWB_OUTPUT_WBGAIN_B:
        EZLOGV("EZT_PARMS_AWB_OUTPUT_WBGAIN_B %f",
                awb_params->output_wb_gain_b);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->output_wb_gain_b);
        break;
    case EZT_PARMS_AWB_REGULAR_AVE_RGRATIO:
        EZLOGV("EZT_PARMS_AWB_REGULAR_AVE_RGRATIO %f",
                awb_params->regular_ave_rg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->regular_ave_rg_ratio);
        break;
    case EZT_PARMS_AWB_REGULAR_AVE_BGRATIO:
        EZLOGV("EZT_PARMS_AWB_REGULAR_AVE_BGRATIO %f",
                awb_params->regular_ave_bg_ratio);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->regular_ave_bg_ratio);
        break;
    case EZT_PARMS_AWB_CCT_AWBBAYER:
        EZLOGV("EZT_PARMS_AWB_CCT_AWBBAYER %f",
                awb_params->cct_awb_bayer);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                awb_params->cct_awb_bayer);
        break;
    case EZT_PARMS_AWB_COUNT_EXTREMEB:
        EZLOGV("EZT_PARMS_AWB_COUNT_EXTREMEB %d",
                awb_params->count_extreme_b);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->count_extreme_b);
        break;
    case EZT_PARMS_AWB_BAYER_MIXLEDTABLE_INDEXOVERRIDE:
        EZLOGV("EZT_PARMS_AWB_BAYER_MIXLEDTABLE_INDEXOVERRIDE %d",
                awb_params->mix_led_table_index_override);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                awb_params->mix_led_table_index_override);
        break;

            /* AEC -----------------------------------------------------------------------
             */
    case EZT_PARMS_AEC_ENABLE:
            EZLOGV("EZT_PARMS_AEC_ENABLE %d", ae_params->enable);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->enable);
            break;
    case EZT_PARMS_AEC_LUMA:
            EZLOGV("EZT_PARMS_AEC_LUMA %d", ae_params->luma);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->luma);
            break;
    case EZT_PARMS_AEC_LOCK:
            EZLOGV("EZT_PARMS_AEC_LOCK %d", ae_params->lock);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->lock);
            break;
    case EZT_PARMS_AEC_EXPINDEX:
            EZLOGV("EZT_PARMS_AEC_EXPINDEX %d", ae_params->exposure_index);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->exposure_index);
            break;
    case EZT_PARMS_AEC_LUXINDEX:
            EZLOGV("EZT_PARMS_AEC_LUXINDEX %d", ae_params->lux_index);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->lux_index);
            break;
    case EZT_PARMS_AEC_TOUCH_ROILUMA:
            EZLOGV("EZT_PARMS_AEC_TOUCH_ROILUMA %d", ae_params->touch_roi_luma);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->touch_roi_luma);
            break;
    case EZT_PARMS_AEC_TESTAEC_ENABLE:
            EZLOGV("EZT_PARMS_AEC_TESTAEC_ENABLE %d", ae_params->test_enable);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->test_enable);
            break;
    case EZT_PARMS_AEC_FORCE_SNAPLINECOUNT:
            EZLOGV("EZT_PARMS_AEC_FORCE_SNAPLINECOUNT %d",
                    ae_params->force_snap_linecount.force_snap_linecount_value);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->force_snap_linecount.force_snap_linecount_value);
            break;
    case EZT_PARMS_AEC_FORCE_PREVLINECOUNT:
            EZLOGV("EZT_PARMS_AEC_FORCE_PREVLINECOUNT %d",
                    ae_params->force_linecount.force_linecount_value);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->force_linecount.force_linecount_value);
            break;
    case EZT_PARMS_AEC_PREVIEW_FORCEEXP:
            EZLOGV("EZT_PARMS_AEC_PREVIEW_FORCEEXP %f",
                    ae_params->force_exp.force_exp_value);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                    ae_params->force_exp.force_exp_value);
            break;
    case EZT_PARMS_AEC_SNAPSHOT_FORCEEXP:
            EZLOGV("EZT_PARMS_AEC_PREVIEW_FORCEEXP %f",
                    ae_params->force_snapshot_exp.force_snap_exp_value);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                    ae_params->force_snapshot_exp.force_snap_exp_value);
            break;
    case EZT_PARMS_AEC_PREVIEW_LINECOUNT:
            EZLOGV("EZT_PARMS_AEC_PREVIEW_LINECOUNT %d", ae_params->preview_linecount);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->preview_linecount);
            break;
    case EZT_PARMS_AEC_SNAPSHOT_LINECOUNT:
            EZLOGV("EZT_PARMS_AEC_SNAPSHOT_LINECOUNT %d", ae_params->snap_linecount);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->snap_linecount);
            break;
    case EZT_PARMS_AEC_PREVIEW_REALGAIN:
            EZLOGV("EZT_PARMS_AEC_PREVIEW_REALGAIN %f", ae_params->preview_realgain);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                    ae_params->preview_realgain);
            break;
    case EZT_PARMS_AEC_PREVIEW_EXPOSURETIME:
            EZLOGV("EZT_PARMS_AEC_PREVIEW_EXPOSURETIME %f",
                    ae_params->preview_exp_time);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                    ae_params->preview_exp_time);
            break;
    case EZT_PARMS_AEC_SNAPSHOT_REALGAIN:
            EZLOGV("EZT_PARMS_AEC_SNAPSHOT_REALGAIN %f", ae_params->snap_realgain);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                    ae_params->snap_realgain);
            break;
    case EZT_PARMS_AEC_SNAPSHOT_EXPOSURETIME:
            EZLOGV("EZT_PARMS_AEC_SNAPSHOT_EXPOSURETIME %f", ae_params->snap_exp_time);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                    ae_params->snap_exp_time);
            break;
    case EZT_PARMS_AEC_FORCE_SNAPGAIN:
            EZLOGV("EZT_PARMS_AEC_FORCE_SNAPGAIN %f",
                    ae_params->force_snap_gain.force_snap_gain_value);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                    ae_params->force_snap_gain.force_snap_gain_value);
            break;
    case EZT_PARMS_AEC_FORCE_PREVGAIN:
            EZLOGV("EZT_PARMS_AEC_FORCE_PREVGAIN %f",
                    ae_params->force_gain.force_gain_value);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                    ae_params->force_gain.force_gain_value);
            break;
    case EZT_PARMS_AEC_FORCEDEXPOSURE_GAIN:
            EZLOGV("EZT_PARMS_AEC_FORCEDEXPOSURE_GAIN %f",
                    ae_params->force_gain.force_gain_value);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                    ae_params->force_gain.force_gain_value);
            break;
    case EZT_PARMS_AEC_FORCEDEXPOSURE_LINECOUNT:
            EZLOGV("EZT_PARMS_AEC_FORCEDEXPOSURE_LINECOUNT %d",
                    ae_params->force_linecount.force_linecount_value);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->force_linecount.force_linecount_value);
            break;
    case EZT_PARMS_AEC_HYBRIDLUMA:
            EZLOGV("EZT_PARMS_AEC_HYBRIDLUMA %d",
                    ae_params->hybrid_luma);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->hybrid_luma);
            break;
    case EZT_PARMS_AEC_AVERAGELUMA:
            EZLOGV("EZT_PARMS_AEC_AVERAGELUMA %d",
                    ae_params->average_luma);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->average_luma);
            break;
    case EZT_PARMS_AEC_BRIGHTREGIONS:
            EZLOGV("EZT_PARMS_AEC_BRIGHTREGIONS %d",
                    ae_params->bright_regions);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->bright_regions);
            break;
    case EZT_PARMS_AEC_REDSEVERITY:
            EZLOGV("EZT_PARMS_AEC_REDSEVERITY %f",
                    ae_params->red_severity);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                    ae_params->red_severity);
            break;
    case EZT_PARMS_AEC_GREENSEVERITY:
            EZLOGV("EZT_PARMS_AEC_GREENSEVERITY %f",
                    ae_params->green_severity);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                    ae_params->green_severity);
            break;
    case EZT_PARMS_AEC_BLUESEVERITY:
            EZLOGV("EZT_PARMS_AEC_BLUESEVERITY %f",
                    ae_params->blue_severity);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                    ae_params->blue_severity);
            break;
    case EZT_PARMS_AEC_BIMODALSEVERITY:
            EZLOGV("EZT_PARMS_AEC_BIMODALSEVERITY %f",
                    ae_params->bimodal_severity);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                    ae_params->bimodal_severity);
            break;
    case EZT_PARMS_AEC_COMPENSATEDTARGET:
            EZLOGV("EZT_PARMS_AEC_COMPENSATEDTARGET %d",
                    ae_params->compensated_target);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->compensated_target);
            break;
    case EZT_PARMS_AEC_DEFAULTTARGET:
            EZLOGV("EZT_PARMS_AEC_DEFAULTTARGET %d",
                    ae_params->default_target);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->default_target);
            break;
    case EZT_PARMS_AEC_CURRENTBRIGHTLEVEL:
            EZLOGV("EZT_PARMS_AEC_CURRENTBRIGHTLEVEL %d",
                    ae_params->current_bright_level);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->current_bright_level);
            break;
    case EZT_PARMS_AEC_CURRENTBRIGHTWEIGHT:
            EZLOGV("EZT_PARMS_AEC_CURRENTBRIGHTWEIGHT %f",
                    ae_params->current_bright_weight);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                    ae_params->current_bright_weight);
            break;
    case EZT_PARMS_AEC_CURRENTEXPOSURETIME:
            EZLOGV("EZT_PARMS_AEC_CURRENTEXPOSURETIME %f",
                    ae_params->preview_exp_time);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                    ae_params->preview_exp_time);
            break;
    case EZT_PARMS_AEC_ENTROPYLUMAOFFSET:
            EZLOGV("EZT_PARMS_AEC_ENTROPYLUMAOFFSET %d",
                    ae_params->entropy_luma_offset);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->entropy_luma_offset);
            break;
    case EZT_PARMS_AEC_FLASHOFFEXPINDEX:
            EZLOGV("EZT_PARMS_AEC_FLASHOFFEXPINDEX %d",
                    ae_params->flash_off_exp_index);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->flash_off_exp_index);
            break;
    case EZT_PARMS_AEC_FLASHLOEXPINDEX:
            EZLOGV("EZT_PARMS_AEC_FLASHLOEXPINDEX %d",
                    ae_params->flash_lo_exp_index);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->flash_lo_exp_index);
            break;
    case EZT_PARMS_AEC_FLASHOFFLUMA:
            EZLOGV("EZT_PARMS_AEC_FLASHOFFLUMA %d",
                    ae_params->flash_off_luma);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->flash_off_luma);
            break;
    case EZT_PARMS_AEC_FLASHLOLUMA:
            EZLOGV("EZT_PARMS_AEC_FLASHLOLUMA %d",
                    ae_params->flash_lo_luma);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->flash_lo_luma);
            break;
    case EZT_PARMS_AEC_FLASHSENSITIVITYOFF:
            EZLOGV("EZT_PARMS_AEC_FLASHSENSITIVITYOFF %f",
                    ae_params->flash_sensitivity_off);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                    ae_params->flash_sensitivity_off);
            break;
    case EZT_PARMS_AEC_FLASHSENSITIVITYLO:
            EZLOGV("EZT_PARMS_AEC_FLASHSENSITIVITYLO %f",
                    ae_params->flash_sensitivity_lo);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                    ae_params->flash_sensitivity_lo);
            break;
    case EZT_PARMS_AEC_FLASHSENSITIVIEYHI:
            EZLOGV("EZT_PARMS_AEC_FLASHSENSITIVIEYHI %f",
                    ae_params->flash_sensitivity_high);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                    ae_params->flash_sensitivity_high);
            break;
    case EZT_PARMS_AEC_FLATSEVERITY:
            EZLOGV("EZT_PARMS_AEC_FLATSEVERITY %f",
                    ae_params->flat_severity);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                    ae_params->flat_severity);
            break;
    case EZT_PARMS_AEC_FLATNEARSENSITIVITY:
            EZLOGV("EZT_PARMS_AEC_FLATNEARSENSITIVITY %f",
                    ae_params->flat_near_severity);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                    ae_params->flat_near_severity);
            break;
    case EZT_PARMS_AEC_FLATBRIGHTDETECTED:
            EZLOGV("EZT_PARMS_AEC_FLATBRIGHTDETECTED %d",
                    ae_params->flat_bright_detected);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->flat_bright_detected);
            break;
    case EZT_PARMS_AEC_FLATDARKDETECTED:
            EZLOGV("EZT_PARMS_AEC_FLATDARKDETECTED %d",
                    ae_params->flat_dark_detected);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->flat_dark_detected);
            break;
    case EZT_PARMS_AEC_FLATLUMARATIO:
            EZLOGV("EZT_PARMS_AEC_FLATLUMARATIO %d",
                    ae_params->flat_luma_ratio);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->flat_luma_ratio);
            break;
    case EZT_PARMS_AEC_SSDLUMATARGETOFFSET:
            EZLOGV("EZT_PARMS_AEC_SSDLUMATARGETOFFSET %d",
                    ae_params->ssd_luma_target_offset);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->ssd_luma_target_offset);
            break;
    case EZT_PARMS_AEC_BSDLUMATARGETOFFSET:
            EZLOGV("EZT_PARMS_AEC_BSDLUMATARGETOFFSET %d",
                    ae_params->bsd_luma_target_offset);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->bsd_luma_target_offset);
            break;
    case EZT_PARMS_AEC_FORCEDEXPOSURE_ENABLE:
            EZLOGV("EZT_PARMS_AEC_FORCEDEXPOSURE_ENABLE %d",
                    ae_params->force_exp.forced);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    ae_params->force_exp.forced);
            break;

            /* AF ------------------------------------------------------------------------
             */
    case EZT_PARMS_AF_ENABLE:
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->enable);
            break;
    case EZT_PARMS_AF_PEAKLOCATIONINDEX:
            EZLOGV("EZT_PARMS_AF_ENABLE %d", af_params->peak_location_index);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->peak_location_index);
            break;
    case EZT_PARMS_AF_ROI_LEFT:
            EZLOGV("EZT_PARMS_AF_ROI_LEFT %d", af_params->roi_left);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->roi_left);
            break;
    case EZT_PARMS_AF_ROI_TOP:
            EZLOGV("EZT_PARMS_AF_ROI_TOP %d", af_params->roi_top);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->roi_top);
            break;
    case EZT_PARMS_AF_ROI_WIDTH:
            EZLOGV("EZT_PARMS_AF_ROI_WIDTH %d", af_params->roi_width);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->roi_width);
            break;
    case EZT_PARMS_AF_ROI_HEIGHT:
            EZLOGV("EZT_PARMS_AF_ROI_HEIGHT %d", af_params->roi_height);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->roi_height);
            break;
    case EZT_PARMS_AF_GRIDINFO_HNUM:
            EZLOGV("EZT_PARMS_AF_GRIDINFO_HNUM %d", af_params->grid_info_h_num);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->grid_info_h_num);
            break;
    case EZT_PARMS_AF_GRIDINFO_VNUM:
            EZLOGV("EZT_PARMS_AF_GRIDINFO_VNUM %d", af_params->grid_info_v_num);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->grid_info_v_num);
            break;
    case EZT_PARMS_AF_R_FVMIN:
            EZLOGV("EZT_PARMS_AF_R_FVMIN %d", af_params->r_fv_min);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->r_fv_min);
            break;
    case EZT_PARMS_AF_GR_FVMIN:
            EZLOGV("EZT_PARMS_AF_GR_FVMIN %d", af_params->gr_fv_min);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->gr_fv_min);
            break;
    case EZT_PARMS_AF_GB_FVMIN:
            EZLOGV("EZT_PARMS_AF_GB_FVMIN %d", af_params->gb_fv_min);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->gb_fv_min);
            break;
    case EZT_PARMS_AF_B_FVMIN:
            EZLOGV("EZT_PARMS_AF_B_FVMIN %d", af_params->b_fv_min);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->b_fv_min);
            break;
    case EZT_PARMS_AF_HPF:
            i32_value = (int32_t *)&(af_params->hpf);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    i32_value[table_index]);
            break;
    case EZT_PARMS_AF_MODE:
            EZLOGV("EZT_PARMS_AF_MODE %d", af_params->mode);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->mode);
            break;
    case EZT_PARMS_AF_STATUS:
            EZLOGV("EZT_PARMS_AF_STATUS %d", af_params->status);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->status);
            break;
    case EZT_PARMS_AF_FAREND:
            EZLOGV("EZT_PARMS_AF_FAREND %d", af_params->far_end);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->far_end);
            break;
    case EZT_PARMS_AF_NEAREND:
            EZLOGV("EZT_PARMS_AF_NEAREND %d", af_params->near_end);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->near_end);
            break;
    case EZT_PARMS_AF_HYPPOS:
            EZLOGV("EZT_PARMS_AF_HYPPOS %d", af_params->hyp_pos);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->hyp_pos);
            break;
    case EZT_PARMS_AF_STATE:
            EZLOGV("EZT_PARMS_AF_STATE %d", af_params->state);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->state);
            break;
    case EZT_PARMS_AF_STATS_INDEX:
            EZLOGV("EZT_PARMS_AF_STATS_INDEX %d", af_params->stats_index);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->stats_index);
            break;
    case EZT_PARMS_AF_STATS_POS:
            EZLOGV("EZT_PARMS_AF_STATS_POS %d", af_params->stats_pos);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->stats_pos);
            break;
    case EZT_PARMS_AF_STATS_FV:
            i32_value = (int32_t *)&(af_params->stats_fv);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    i32_value[table_index]);
            break;
    case EZT_PARMS_AF_STATS_MAXFV:
            EZLOGV("EZT_PARMS_AF_STATS_MAXFV %d", af_params->stats_max_fv);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->stats_max_fv);
            break;
    case EZT_PARMS_AF_STATS_MINFV:
            EZLOGV("EZT_PARMS_AF_STATS_MINFV %d", af_params->stats_min_fv);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->stats_min_fv);
            break;
    case EZT_PARMS_AF_FRAMEDELAY:
            EZLOGV("EZT_PARMS_AF_FRAMEDELAY %d", af_params->frame_delay);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->frame_delay);
            break;
    case EZT_PARMS_AF_ENABLEMULTIWINDOW:
            EZLOGV("EZT_PARMS_AF_ENABLEMULTIWINDOW %d", af_params->enable_multiwindow);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->enable_multiwindow);
            break;
    case EZT_PARMS_AF_MWIN:
            i32_value = (int32_t *)&(af_params->Mwin);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    i32_value[table_index]);
            break;
    case EZT_PARMS_AF_NUMDOWNHILL:
            EZLOGV("EZT_PARMS_AF_NUMDOWNHILL %d", af_params->num_downhill);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->num_downhill);
            break;
    case EZT_PARMS_AF_CAF_STATE:
            EZLOGV("EZT_PARMS_AF_CAF_STATE %d", af_params->caf_state);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->caf_state);
            break;
    case EZT_PARMS_AF_CURRLUMA:
            EZLOGV("EZT_PARMS_AF_CURRLUMA %d", af_params->cur_luma);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->cur_luma);
            break;
    case EZT_PARMS_AF_EXPINDEX:
            EZLOGV("EZT_PARMS_AF_EXPINDEX %d", af_params->exp_index);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->exp_index);
            break;
    case EZT_PARMS_AF_LUMASETTLEDCOUNT:
            EZLOGV("EZT_PARMS_AF_LUMASETTLEDCOUNT %d", af_params->luma_settled_cnt);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->luma_settled_cnt);
            break;
    case EZT_PARMS_AF_AVEFV:
            EZLOGV("EZT_PARMS_AF_AVEFV %d", af_params->ave_fv);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->ave_fv);
            break;
    case EZT_PARMS_AF_CAF_PANNING_UNSTABLECOUNT:
            EZLOGV("EZT_PARMS_AF_CAF_PANNING_UNSTABLECOUNT %d",
                    af_params->caf_panning_unstable_cnt);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->caf_panning_unstable_cnt);
            break;
    case EZT_PARMS_AF_CAF_PANNING_STABLECOUNT:
            EZLOGV("EZT_PARMS_AF_CAF_PANNING_STABLECOUNT %d",
                    af_params->caf_panning_stable_cnt);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->caf_panning_stable_cnt);
            break;
    case EZT_PARMS_AF_CAF_PANNING_STABLE:
            EZLOGV("EZT_PARMS_AF_CAF_PANNING_STABLE %d", af_params->caf_panning_stable);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->caf_panning_stable);
            break;
    case EZT_PARMS_AF_CAF_SAD_CHANGE:
            EZLOGV("EZT_PARMS_AF_CAF_SAD_CHANGE %d", af_params->caf_sad_change);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->caf_sad_change);
            break;
    case EZT_PARMS_AF_CAF_EXP_CHANGE:
            EZLOGV("EZT_PARMS_AF_CAF_EXP_CHANGE %d", af_params->caf_exposure_change);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->caf_exposure_change);
            break;
    case EZT_PARMS_AF_CAF_LUMACHNG_DURINGSRCH:
            EZLOGV("EZT_PARMS_AF_CAF_LUMACHNG_DURINGSRCH %d",
                    af_params->caf_luma_chg_during_srch);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->caf_luma_chg_during_srch);
            break;
    case EZT_PARMS_AF_CAF_TRIG_REFOCUS:
            EZLOGV("EZT_PARMS_AF_CAF_TRIG_REFOCUS %d", af_params->caf_trig_refocus);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->caf_trig_refocus);
            break;
    case EZT_PARMS_AF_CAF_GYRO_ASSISTED_PANNING:
            EZLOGV("EZT_PARMS_AF_CAF_GYRO_ASSISTED_PANNING %d",
                    af_params->caf_gyro_assisted_panning);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    af_params->caf_gyro_assisted_panning);
            break;

            /* AFD -----------------------------------------------------------------------
             */
#if 0 // ???
    case EZT_PARMS_AFD_PREVIEW_FLICKERDECTIONAPPLIED:
    case EZT_PARMS_AFD_FLICKERFREQ:
    case EZT_PARMS_AFD_STATUS:
    case EZT_PARMS_AFD_STDWIDTH:
    case EZT_PARMS_AFD_MULTIPLEPEAKALGO:
    case EZT_PARMS_AFD_ACTUALPEAKS:
            i32_value = (int32_t *)&(pp_diagnostics->afd_params.flicker_detect);
            EZLOGV("EZT_PARMS_AFD %d",
                    i32_value[item->id - EZT_PARMS_AFD_PREVIEW_FLICKERDECTIONAPPLIED]);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    i32_value[item->id - EZT_PARMS_AFD_PREVIEW_FLICKERDECTIONAPPLIED]);
            break;
#endif

    /* ASD -----------------------------------------------------------------------
     */
    case EZT_PARMS_ASD_BLS_DETECTED:
        EZLOGV("EZT_PARMS_ASD_BLS_DETECTED %d",
                asd_params->bls_awb_blacklit_detected);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                asd_params->bls_awb_blacklit_detected);
        break;
    case EZT_PARMS_ASD_BLS_HISTBLKLITDETECTED:
        EZLOGV("EZT_PARMS_ASD_BLS_HISTBLKLITDETECTED %d",
                asd_params->bls_histogram_blacklit_detected);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                asd_params->bls_histogram_blacklit_detected);
        break;
    case EZT_PARMS_ASD_BLS_SEVERITY:
        EZLOGV("EZT_PARMS_ASD_BLS_SEVERITY %d",
                asd_params->bls_severity);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                asd_params->bls_severity);
        break;
    case EZT_PARMS_ASD_BLS_LUMACOMPENSATIONOFFSET:
        EZLOGV("EZT_PARMS_ASD_BLS_LUMACOMPENSATIONOFFSET %d",
                asd_params->bls_luma_compensation_offset);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                asd_params->bls_luma_compensation_offset);
        break;
    case EZT_PARMS_ASD_SNS_LUMATHRESHOLD:
        EZLOGV("EZT_PARMS_ASD_SNS_LUMATHRESHOLD %d",
                asd_params->ss_luma_threadhold);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                asd_params->ss_luma_threadhold);
        break;
    case EZT_PARMS_ASD_SNS_AWBYMAXINGRAY:
        EZLOGV("EZT_PARMS_ASD_SNS_AWBYMAXINGRAY %d",
                asd_params->ss_awb_ymax_gray);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                asd_params->ss_awb_ymax_gray);
        break;
    case EZT_PARMS_ASD_SNS_MINSAMPLETHRESHOLD:
        EZLOGV("EZT_PARMS_ASD_SNS_MINSAMPLETHRESHOLD %d",
                asd_params->ss_min_sample_threshold);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                asd_params->ss_min_sample_threshold);
        break;
    case EZT_PARMS_ASD_SNS_EXTREAMSAMPLETHRESHOLD:
        EZLOGV("EZT_PARMS_ASD_SNS_EXTREAMSAMPLETHRESHOLD %d",
                asd_params->ss_extreme_sample_threshold);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                asd_params->ss_extreme_sample_threshold);
        break;
    case EZT_PARMS_ASD_SNS_SEVERITY:
        // not currently implemented
        EZLOGV("EZT_PARMS_ASD_SNS_SEVERITY NOT IMPLEMENTED");
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", -1);
        break;
    case EZT_PARMS_ASD_SNS_INITIAL_LUMAOFFSET:
        EZLOGV("EZT_PARMS_ASD_SNS_INITIAL_LUMAOFFSET %f",
                asd_params->ss_initial_luma_offset);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                asd_params->ss_initial_luma_offset);
        break;
    case EZT_PARMS_ASD_SNS_SCALED_LUMAOFFSET:
        EZLOGV("EZT_PARMS_ASD_SNS_SCALED_LUMAOFFSET %f",
                asd_params->ss_scaled_luma_offset);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f",
                asd_params->ss_scaled_luma_offset);
        break;
    case EZT_PARMS_ASD_SNS_FILTERED_LUMAOFFSET:
        EZLOGV("EZT_PARMS_ASD_SNS_FILTERED_LUMAOFFSET %d",
                asd_params->ss_filtered_luma_offset);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                asd_params->ss_filtered_luma_offset);
        break;

    /* Antibanding ---------------------------------------------------------------
     */
#if 0
    case EZT_PARMS_ANTIBANDING_PIXELCLK:
            mctl_eztune_get_misc_sensor(EZ_MISC_SENSOR_PIXELCLKFREQ);
            EZLOGV("EZT_PARMS_ANTIBANDING_PIXELCLK %d",
                    sensor_diagnostics->pixelclock_freq);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    sensor_diagnostics->pixelclock_freq);
            break;
    case EZT_PARMS_ANTIBANDING_PIXELCLKPERLINE:
            mctl_eztune_get_misc_sensor(EZ_MISC_SENSOR_PIXELSPERLINE);
            EZLOGV("EZT_PARMS_ANTIBANDING_PIXELCLKPERLINE %d",
                    sensor_diagnostics->pixelsperLine);
            rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                    sensor_diagnostics->pixelsperLine);
            break;
#endif

    /* Misc ----------------------------------------------------------------------
     */
    case EZT_PARMS_MISC_FLASHMODE:
        EZLOGV("EZT_PARMS_MISC_FLASHMODE %d", *flash_params);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                *flash_params);
        break;
    case EZT_PARMS_MISC_CHIPSETID:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%s", "TBD"); /*FIX ME*/
        break;
    case EZT_PARMS_MISC_TUNINGDATATIMESTAMP:
        rc = property_get(EZTUNE_PROP_TUNE_DATE, output_buf, "00:00:00");
        break;
    case EZT_PARMS_MISC_TUNINGDATANAME:
        rc = property_get(EZTUNE_PROP_TUNE_NAME , output_buf, "No Name");
        break;
    case EZT_PARMS_MISC_TUNINGDATAUPDATEDBY:
        rc = property_get(EZTUNE_PROP_TUNE_AUTHOR, output_buf, "No Author");
        break;
    case EZT_PARMS_MISC_PARAMETERRETENTIONENABLE:
        rc = property_get(EZTUNE_PROP_STICKY_SETTINGS, output_buf, "0");
        break;

    /* POSTPROC_VIDEOHDR -----------------------------------------------
     */
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_RECONENABLE:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_reconenable);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACENABLE:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_macenable);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_RECONLINEARMODE:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_reconlinearmode);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACLINEARMODE:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_maclinearmode);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_RECONEDGELPFTAP0:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_reconedgelpftap0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACMOTIONDILATION:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_macmotiondilation);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACSMOOTHENABLE:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_macsmoothenable);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_RECONFLATREGIONTHRESH:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_reconflatregionthresh);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_RECONMINFACTOR:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_reconminfactor);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_RECONHEDGETHRESH1:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_reconhedgethresh1);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_RECONHEDGELOGTHRESHDIFF:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_reconhedgelogthreshdiff);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_RECONMOTIONTHRESH1:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_reconmotionthresh1);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_RECONMOTIONLOGTHRESHDIFF:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_reconmotionlogthreshdiff);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_RECONDARKTHRESH1:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_recondarkthresh1);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_RECONDARKLOGTHRESHDIFF:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_recondarklogthreshdiff);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACMOTION0THRESH1:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_macmotion0thresh1);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACMOTION0THRESH2:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_macmotion0thresh2);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MOTION0DT0:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_motion0dt0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACMOTIONSTRENGTH:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_macmotionstrength);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACLOWLIGHTSTRENGTH:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_maclowlightstrength);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACLOWLIGHTTHRESH1:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_maclowlightthresh1);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACLOWLIGHTLOGTHRESHDIFF:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_maclowlightlogthreshdiff);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACBRIGHTTHRESH1:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_macbrightthresh1);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACBRIGHTLOGTHRESHDIFF:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_macbrightlogthreshdiff);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACSMOOTHTAP0:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_macsmoothtap0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACSMOOTHTHRESH1:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_macsmooththresh1);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACSMOOTHLOGTHRESHDIFF:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.current_macsmoothlogthreshdiff);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_PIXELPATTERN:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.pixelpattern);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_3DENABLE:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.enable_3d);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_LEFTPANELWIDTH:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.leftpanelwidth);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_FIRSTFIELD:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.firstfield);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_MSBALIGN:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.msbalign);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_EXPRATIO:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.expratio);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_EXPRATIORECIP:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.expratiorecip);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_RGWBGAINRATIO:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.rgwbgainratio);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_BGWBGAINRATIO:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.bgwbgainratio);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_GRWBGAINRATIO:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.grwbgainratio);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_GBWBGAINRATIO:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.gbwbgainratio);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_MACSQRTANALOGGAIN:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", pp_diagnostics->video_hdr.macsqrtanaloggain);
        break;

    /* autofocus tuning parameters */
    case EZT_PARMS_AFTUNE_TUNING_REGIONSIZE:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                *(uint16_t *)(&ezctrl->af_driver_ptr->actuator_tuned_params.region_size));
        break;
    case EZT_PARMS_AFTUNE_TUNING_REGIONPARAMS_MACROSTEPBOUND:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                *(uint16_t *)(&ezctrl->af_driver_ptr->actuator_tuned_params.
                    region_params[table_index].step_bound[0]));
        break;
    case EZT_PARMS_AFTUNE_TUNING_REGIONPARAMS_INFSTEPBOUND:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                *(uint16_t *)(&ezctrl->af_driver_ptr->actuator_tuned_params.
                    region_params[table_index].step_bound[1]));
        break;
    case EZT_PARMS_AFTUNE_TUNING_INITIALCODE:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
                *(uint16_t *)(&ezctrl->af_driver_ptr->actuator_tuned_params.initial_code));
        break;
    case EZT_PARMS_AFTUNE_TUNING_TEST_DEFFOCUS_ENABLE:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", 0);
        break;
    case EZT_PARMS_AFTUNE_TUNING_TEST_MOVEFOCUS_ENABLE:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", 0);
        break;
    case EZT_PARMS_AFTUNE_TUNING_TEST_MOVEFOCUS_DIR:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
            ezctrl->af_tuning_ptr->movfocdirection);
        break;
    case EZT_PARMS_AFTUNE_TUNING_TEST_MOVEFOCUS_STEPS:
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d",
            ezctrl->af_tuning_ptr->movfocsteps);
        break;

    default:
        EZLOGV("eztune_get_diagnostic_value: invalid id %d!!!", item->id);
        break;
    }

    return rc;
}


eztune_item_t eztune_diag_get_item(int i)
{
    static eztune_item_t eztune_item;
    EZLOGV("eztune_get_diag_item at i=%d",i);
    eztune_item.id = i;
    switch (i) {
    case EZT_PARMS_3A:
        eztune_set_item_data(&eztune_item,
                "3A",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_3A_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_RTOR:
        eztune_set_item_data(&eztune_item,
                "ColorCorr-Current-RtoR",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_GTOR:
        eztune_set_item_data(&eztune_item,
                "ColorCorr-Current-GtoR",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_BTOR:
        eztune_set_item_data(&eztune_item,
                "ColorCorr-Current-BtoR",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_RTOG:
        eztune_set_item_data(&eztune_item,
                "ColorCorr-Current-RtoG",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_GTOG:
        eztune_set_item_data(&eztune_item,
                "ColorCorr-Current-GtoG",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_BTOG:
        eztune_set_item_data(&eztune_item,
                "ColorCorr-Current-BtoG",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_RTOB:
        eztune_set_item_data(&eztune_item,
                "ColorCorr-Current-RtoB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_GTOB:
        eztune_set_item_data(&eztune_item,
                "ColorCorr-Current-GtoB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_BTOB:
        eztune_set_item_data(&eztune_item,
                "ColorCorr-Current-BtoB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_ROFFSET:
        eztune_set_item_data(&eztune_item,
                "ColorCorr-Current-ROffset",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_GOFFSET:
        eztune_set_item_data(&eztune_item,
                "ColorCorr-Current-GOffset",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_BOFFSET:
        eztune_set_item_data(&eztune_item,
                "ColorCorr-Current-BOffset",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCORR_CURRENT_QFACTOR:
        eztune_set_item_data(&eztune_item,
                "ColorCorr-Current-Qfactor",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_CBSCALINGNEGATIVE_AM:
        eztune_set_item_data(&eztune_item,
                "ColorConv-Current-CbScalingNegative_am",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_CBSCALINGPOSITIVE_AP:
        eztune_set_item_data(&eztune_item,
                "ColorConv-Current-CbScalingPositive_ap",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_DIFFRGTOCBSHEARINGNEGATIVE_BM:
        eztune_set_item_data(&eztune_item,
                "ColorConv-Current-DiffRGtoCbShearingNegative_bm",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_DIFFRGTOCBSHEARINGPOSITIVE_BP:
        eztune_set_item_data(&eztune_item,
                "ColorConv-Current-DiffRGtoCbShearingPositive_bp",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_CRSCALINGNEGATIVE_CM:
        eztune_set_item_data(&eztune_item,
                "ColorConv-Current-CrScalingNegative_cm",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_CRSCALINGPOSITIVE_CP:
        eztune_set_item_data(&eztune_item,
                "ColorConv-Current-CrScalingPositive_cp",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_DIFFBGTOCRSHEARINGNEGATIVE_DM:
        eztune_set_item_data(&eztune_item,
                "ColorConv-Current-DiffBGtoCrShearingNegative_dm",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_DIFFBGTOCRSHEARINGPOSITIVE_DP:
        eztune_set_item_data(&eztune_item,
                "ColorConv-Current-DiffBGtoCrShearingPositive_dp",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_CBOFFSET_K_CB:
        eztune_set_item_data(&eztune_item,
                "ColorConv-Current-CBOffset_k_cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_CROFFSET_K_CR:
        eztune_set_item_data(&eztune_item,
                "ColorConv-Current-CROffset_k_cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_RTOY:
        eztune_set_item_data(&eztune_item,
                "ColorConv-Current-RtoY",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_GTOY:
        eztune_set_item_data(&eztune_item,
                "ColorConv-Current-GtoY",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_BTOY:
        eztune_set_item_data(&eztune_item,
                "ColorConv-Current-BtoY",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_YOFFSET:
        eztune_set_item_data(&eztune_item,
                "ColorConv-Current-Yoffset",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_BOFFSET:
        eztune_set_item_data(&eztune_item,
                "ColorConv-Current-Boffset",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_COLORCONV_CURRENT_ROFFSET:
        eztune_set_item_data(&eztune_item,
                "ColorConv-Current-Roffset",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BLACKLEVEL_SNAPSHOT_BLACKEVENROWEVENCOL:
        eztune_set_item_data(&eztune_item,
                "BlackLevel-Snapshot-EvenRowEvenCol",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BLACKLEVEL_SNAPSHOT_BLACKEVENROWODDCOL:
        eztune_set_item_data(&eztune_item,
                "BlackLevel-Snapshot-EvenRowOddCol",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BLACKLEVEL_SNAPSHOT_BLACKODDROWEVENCOL:
        eztune_set_item_data(&eztune_item,
                "BlackLevel-Snapshot-OddRowEvenCol",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BLACKLEVEL_SNAPSHOT_BLACKODDROWODDCOL:
        eztune_set_item_data(&eztune_item,
                "BlackLevel-Snapshot-OddRowOddCol",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BLACKLEVEL_PREVIEW_BLACKEVENROWEVENCOL:
        eztune_set_item_data(&eztune_item,
                "BlackLevel-Preview-EvenRowEvenCol",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BLACKLEVEL_PREVIEW_BLACKEVENROWODDCOL:
        eztune_set_item_data(&eztune_item,
                "BlackLevel-Preview-EvenRowOddCol",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BLACKLEVEL_PREVIEW_BLACKODDROWEVENCOL:
        eztune_set_item_data(&eztune_item,
                "BlackLevel-Preview-OddRowEvenCol",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BLACKLEVEL_PREVIEW_BLACKODDROWODDCOL:
        eztune_set_item_data(&eztune_item,
                "BlackLevel-Preview-OddRowOddCol",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ROLLOFF_ENABLE:
        eztune_set_item_data(&eztune_item,
                "Rolloff-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ROLLOFF_CONTROLENABLE:
        eztune_set_item_data(&eztune_item,
                "Rolloff-ControlEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ROLLOFF_PREVIEW_COEFFTABLE_R:
        eztune_set_item_data(&eztune_item,
                "Rolloff-Preview-Coefftable-R",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, ROLLOFF_TABLE_SIZE, 0);
        break;
    case EZT_PARMS_ROLLOFF_PREVIEW_COEFFTABLE_GR:
        eztune_set_item_data(&eztune_item,
                "Rolloff-Preview-Coefftable-Gr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, ROLLOFF_TABLE_SIZE, 0);
        break;
    case EZT_PARMS_ROLLOFF_PREVIEW_COEFFTABLE_GB:
        eztune_set_item_data(&eztune_item,
                "Rolloff-Preview-Coefftable-Gb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, ROLLOFF_TABLE_SIZE, 0);
        break;
    case EZT_PARMS_ROLLOFF_PREVIEW_COEFFTABLE_B:
        eztune_set_item_data(&eztune_item,
                "Rolloff-Preview-Coefftable-B",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, ROLLOFF_TABLE_SIZE, 0);
        break;
    case EZT_PARMS_ROLLOFF_SNAPSHOT_COEFFTABLE_R:
        eztune_set_item_data(&eztune_item,
                "Rolloff-Snapshot-Coefftable-R",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, ROLLOFF_TABLE_SIZE, 0);
        break;
    case EZT_PARMS_ROLLOFF_SNAPSHOT_COEFFTABLE_GR:
        eztune_set_item_data(&eztune_item,
                "Rolloff-Snapshot-Coefftable-Gr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, ROLLOFF_TABLE_SIZE, 0);
        break;
    case EZT_PARMS_ROLLOFF_SNAPSHOT_COEFFTABLE_GB:
        eztune_set_item_data(&eztune_item,
                "Rolloff-Snapshot-Coefftable-Gb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, ROLLOFF_TABLE_SIZE, 0);
        break;
    case EZT_PARMS_ROLLOFF_SNAPSHOT_COEFFTABLE_B:
        eztune_set_item_data(&eztune_item,
                "Rolloff-Snapshot-Coefftable-B",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, ROLLOFF_TABLE_SIZE, 0);
        break;
    case EZT_PARMS_5X5ASF_ENABLE:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_CONTROLENABLE:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-ControlEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SMOOTHFILTERENABLED:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-SmoothFilterEnabled",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SHARPMODE:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-SharpMode",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_LPFMODE:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-LpfMode",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SMOOTHCOEFCENTER:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-SmoothCoefCenter",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SMOOTHCOEFSURR:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-SmoothCoefSurr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_PIPEFLUSHCOUNT:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-PipeFlushCount",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_PIPEFLUSHOVD:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-PipeFlushOvd",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FLUSHHALTOVD:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-FlushHaltOvd",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_CROPENABLE:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-CropEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_NORMALIZEFACTOR:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-NormalizeFactor",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SETTINGLOWTHRESH:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-SettingLowThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SETTINGUPTHRESH:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-SettingUpThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SETTINGNEGTHRESH:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-SettingNegThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SETTINGUPTHRESHF2:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-SettingUpThreshF2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SETTINGNEGTHRESHF2:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-SettingNegThreshF2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SETTINGSHARPAMTF1:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-SettingSharpAmtF1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_SETTINGSHARPAMTF2:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-SettingSharpAmtF2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER1A11:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter1A11",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER1A12:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter1A12",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER1A13:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter1A13",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER1A21:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter1A21",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER1A22:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter1A22",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER1A23:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter1A23",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER1A31:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter1A31",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER1A32:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter1A32",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER1A33:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter1A33",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER2A11:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter2A11",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER2A12:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter2A12",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER2A13:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter2A13",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER2A21:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter2A21",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER2A22:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter2A22",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER2A23:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter2A23",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER2A31:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter2A31",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER2A32:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter2A32",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER2A33:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter2A33",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER3A11:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter3A11",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER3A12:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter3A12",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER3A13:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter3A13",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER3A21:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter3A21",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER3A22:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter3A22",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER3A23:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter3A23",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER3A31:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter3A31",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER3A32:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter3A32",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_PREVIEWFILTERCURRENT_FILTER3A33:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Preview-Filter3A33",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SMOOTHFILTERENABLED:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-SmoothFilterEnabled",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SHARPMODE:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-SharpMode",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_LPFMODE:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-LpfMode",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SMOOTHCOEFCENTER:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-SmoothCoefCenter",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SMOOTHCOEFSURR:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-SmoothCoefSurr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_PIPEFLUSHCOUNT:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-PipeFlushCount",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_PIPEFLUSHOVD:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-PipeFlushOvd",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FLUSHHALTOVD:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-FlushHaltOvd",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_CROPENABLE:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-CropEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_NORMALIZEFACTOR:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-NormalizeFactor",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SETTINGLOWTHRESH:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-SettingLowThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SETTINGUPTHRESH:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-SettingUpThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SETTINGNEGTHRESH:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-SettingNegThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SETTINGUPTHRESHF2:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-SettingUpThreshF2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SETTINGNEGTHRESHF2:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-SettingNegThreshF2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SETTINGSHARPAMTF1:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-SettingSharpAmtF1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_SETTINGSHARPAMTF2:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-SettingSharpAmtF2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER1A11:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter1A11",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER1A12:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter1A12",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER1A13:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter1A13",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER1A21:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter1A21",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER1A22:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter1A22",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER1A23:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter1A23",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER1A31:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter1A31",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER1A32:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter1A32",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER1A33:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter1A33",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER2A11:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter2A11",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER2A12:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter2A12",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER2A13:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter2A13",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER2A21:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter2A21",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER2A22:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter2A22",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER2A23:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter2A23",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER2A31:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter2A31",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER2A32:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter2A32",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER2A33:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter2A33",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER3A11:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter3A11",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER3A12:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter3A12",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER3A13:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter3A13",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER3A21:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter3A21",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER3A22:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter3A22",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER3A23:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter3A23",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER3A31:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter3A31",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER3A32:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter3A32",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_5X5ASF_SNAPSHOTFILTER_CURRENT_FILTER3A33:
        eztune_set_item_data(&eztune_item,
                "5X5ASF-Snapshot-Filter3A33",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_ENABLE:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_CONTROLENABLE:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-ControlEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_SMOOTHPCT:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Preview-FilterCurrent-SmoothPct",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_ENABLESPECIALEFFECTS:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Preview-FilterCurrent-EnableSpecialEffects",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_NEGABSY1:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Preview-FilterCurrent-NegAbsY1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_NZ:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Preview-FilterCurrent-NZ",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_F1:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Preview-FilterCurrent-F1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 16, 0);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_F2:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Preview-FilterCurrent-F2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 16, 0);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_F3:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Preview-FilterCurrent-F3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 16, 0);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_F4:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Preview-FilterCurrent-F4",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 16, 0);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_F5:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Preview-FilterCurrent-F5",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 16, 0);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_LUT1:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Preview-FilterCurrent-LUT1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 24, 0);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_LUT2:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Preview-FilterCurrent-LUT2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 24, 0);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_LUT3:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Preview-FilterCurrent-LUT3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 12, 0);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_ENABLEDYNCLAMP:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Preview-FilterCurrent-EnableDynClamp",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_SMAX:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Preview-FilterCurrent-SMax",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_OMAX:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Preview-FilterCurrent-OMax",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_SMIN:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Preview-FilterCurrent-SMin",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_OMIN:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Preview-FilterCurrent-OMin",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_REGHH:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Preview-FilterCurrent-RegHH",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_REGHL:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Preview-FilterCurrent-RegHL",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_REGVH:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Preview-FilterCurrent-RegVH",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_PREVIEWFILTERCURR_REGVL:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Preview-FilterCurrent-RegVL",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_SMOOTHPCT:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Snapshot-FilterCurrent-SmoothPct",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_ENABLESPECIALEFFECTS:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Snapshot-FilterCurrent-EnableSpecialEffects",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_NEGABSY1:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Snapshot-FilterCurrent-NegAbsY1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_NZ:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Snapshot-FilterCurrent-NZ",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_F1:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Snapshot-FilterCurrent-F1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 16, 0);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_F2:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Snapshot-FilterCurrent-F2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 16, 0);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_F3:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Snapshot-FilterCurrent-F3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 16, 0);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_F4:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Snapshot-FilterCurrent-F4",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 16, 0);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_F5:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Snapshot-FilterCurrent-F5",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 16, 0);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_LUT1:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Snapshot-FilterCurrent-LUT1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 24, 0);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_LUT2:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Snapshot-FilterCurrent-LUT2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 24, 0);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_LUT3:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Snapshot-FilterCurrent-LUT3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 12, 0);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_ENABLEDYNCLAMP:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Snapshot-FilterCurrent-EnableDynClamp",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_SMAX:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Snapshot-FilterCurrent-SMax",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_OMAX:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Snapshot-FilterCurrent-OMax",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_SMIN:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Snapshot-FilterCurrent-SMin",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_OMIN:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Snapshot-FilterCurrent-OMin",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_REGHH:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Snapshot-FilterCurrent-RegHH",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_REGHL:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Snapshot-FilterCurrent-RegHL",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_REGVH:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Snapshot-FilterCurrent-RegVH",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_7X7ASF_SNAPSHOTFILTERCURR_REGVL:
        eztune_set_item_data(&eztune_item,
                "7X7ASF-Snapshot-FilterCurrent-RegVL",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;

    case EZT_PARMS_9X9ASF_ENABLE:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Current-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_CONTROLENABLE:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-ControlEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_TIGGER_GAINSTART:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-Trigger-GainStart",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_TIGGER_GAINEND:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-Trigger-GainEnd",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_TRIGGER_LUTINDEXSTART:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-Trigger-LuxIndexStart",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_TRIGGER_LUTINDEXEND:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-Trigger-LuxIndexEnd",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_SMOOTHPCT:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-SmoothPct",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_HORIZNZ:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-HorizNZ",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_VERTICALNZ:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-VerticalNZ",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_SOBELHCOEFF:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-SobelHCoeff",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 25, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_SOBELSEDIAGONALCOEFF:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-SobelSEDiagonalCoeff",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 25, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_HIGHPASSHCOEFF:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-HighPassHCoeff",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 25, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_HIGHPASSSEDIAGONALCOEFF:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-HighPassSEDiagonalCoeff",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 25, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_HIGHPASSSYMMETRICCOEFF:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-HighPassSymmetricCoeff",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 15, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_LOWPASSCOEFF:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-LowPassCoeff",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 15, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_ACTIVITYLOWPASSCOEFF:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-ActivityLowPassCoeff",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 6, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_ACTIVITYBANDPASSCOEFF:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-ActivityBandPassCoeff",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 6, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_ACTIVITYNORMALIZATIONLUT:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-ActivityNormalizationLUT",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 256, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_WEIGHTMODULATIONLUT:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-WeightModulationLUT",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 256, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_SOFTTHREASHLUT:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-SoftThreshLUT",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 256, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_GAINLUT:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-GainLUT",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 256, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_GAINWEIGHTLUT:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-GainWeightLUT",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 256, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_GAINCAP:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-GainCap",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_GAMMACORRECTEDLUMATARGET:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-GammaCorrectedLumaTarget",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_ENABLEDYNCLAMP:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-EnableDynClamp",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_SMAX:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-SMax",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_OMAX:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-OMax",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_SMIN:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-SMin",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_OMIN:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-OMin",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_CLAMPUL:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-ClampUL",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_CLAMPLL:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-ClampLL",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_PERPENDICULARSCALEFACTOR:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-PerpendicularScaleFactor",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_MAXVALUETHRESH:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-MaxValueThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_NORMSCALE:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-NormScale",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_ACTIVITYCLAMPTHRESH:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-ActivityClampThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_L2NORMENABLE:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-L2NormEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_MEDIANBLENDUPPEROFFSET:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-MedianBlendUpperOffset",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_PREVIEWFILTERCURR_MEDIANBLENDLOWEROFFSET:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Preview-FilterCurrent-MedianBlendLowerOffset",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;

    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_TIGGER_GAINSTART:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-Trigger-GainStart",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_TIGGER_GAINEND:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-Trigger-GainEnd",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_TRIGGER_LUTINDEXSTART:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-Trigger-LuxIndexStart",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_TRIGGER_LUTINDEXEND:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-Trigger-LuxIndexEnd",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_SMOOTHPCT:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-SmoothPct",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_HORIZNZ:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-HorizNZ",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_VERTICALNZ:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-VerticalNZ",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_SOBELHCOEFF:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-SobelHCoeff",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 25, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_SOBELSEDIAGONALCOEFF:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-SobelSEDiagonalCoeff",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 25, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_HIGHPASSHCOEFF:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-HighPassHCoeff",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 25, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_HIGHPASSSEDIAGONALCOEFF:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-HighPassSEDiagonalCoeff",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 25, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_HIGHPASSSYMMETRICCOEFF:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-HighPassSymmetricCoeff",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 15, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_LOWPASSCOEFF:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-LowPassCoeff",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 15, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_ACTIVITYLOWPASSCOEFF:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-ActivityLowPassCoeff",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 6, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_ACTIVITYBANDPASSCOEFF:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-ActivityBandPassCoeff",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 6, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_ACTIVITYNORMALIZATIONLUT:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-ActivityNormalizationLUT",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 256, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_WEIGHTMODULATIONLUT:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-WeightModulationLUT",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 256, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_SOFTTHREASHLUT:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-SoftThreshLUT",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 256, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_GAINLUT:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-GainLUT",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 256, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_GAINWEIGHTLUT:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-GainWeightLUT",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 256, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_GAINCAP:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-GainCap",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_GAMMACORRECTEDLUMATARGET:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-GammaCorrectedLumaTarget",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_ENABLEDYNCLAMP:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-EnableDynClamp",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_SMAX:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-SMax",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_OMAX:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-OMax",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_SMIN:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-SMin",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_OMIN:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-OMin",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_CLAMPUL:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-ClampUL",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_CLAMPLL:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-ClampLL",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_PERPENDICULARSCALEFACTOR:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-PerpendicularScaleFactor",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_MAXVALUETHRESH:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-MaxValueThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_NORMSCALE:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-NormScale",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_ACTIVITYCLAMPTHRESH:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-ActivityClampThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_L2NORMENABLE:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-L2NormEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_MEDIANBLENDUPPEROFFSET:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-MedianBlendUpperOffset",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_9X9ASF_SNAPSHOTFILTERCURR_MEDIANBLENDLOWEROFFSET:
        eztune_set_item_data(&eztune_item,
                "9X9ASF-Snapshot-FilterCurrent-MedianBlendLowerOffset",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_LUMAADAPT_PREVIEW_LUTYRATIO:
        eztune_set_item_data(&eztune_item,
                "LumaAdapt-Preview-lutYratio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, LA_LUT_SIZE, 0);
        break;
    case EZT_PARMS_LUMAADAPT_SNAPSHOT_LUTYRATIO:
        eztune_set_item_data(&eztune_item,
                "LumaAdapt-Snapshot-lutYratio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, LA_LUT_SIZE, 0);
        break;
    case EZT_PARMS_CHROMASUPP_ENABLE:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_CONTROLENABLE:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-ControlEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_YSUP1:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Preview-Ysup1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_YSUP2:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Preview-Ysup2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_YSUP3:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Preview-Ysup3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_YSUP4:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Preview-Ysup4",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_YSUPS1:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Preview-YsupS1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_YSUPS3:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Preview-YsupS3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_YSUPM1:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Preview-YsupM1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_YSUPM3:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Preview-YsupM3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_CSUP1:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Preview-Csup1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_CSUP2:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Preview-Csup2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_CSUPM1:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Preview-CsupM1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_PREVIEW_CSUPS1:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Preview-CsupS1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUP1:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Snapshot-Ysup1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUP2:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Snapshot-Ysup2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUP3:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Snapshot-Ysup3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUP4:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Snapshot-Ysup4",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUPS1:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Snapshot-YsupS1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUPS3:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Snapshot-YsupS3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUPM1:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Snapshot-YsupM1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_YSUPM3:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Snapshot-YsupM3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_CSUP1:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Snapshot-Csup1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_CSUP2:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Snapshot-Csup2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_CSUPM1:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Snapshot-CsupM1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMASUPP_SNAPSHOT_CSUPS1:
        eztune_set_item_data(&eztune_item,
                "ChromaSupp-Snapshot-CsupS1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_ENABLE:
        eztune_set_item_data(&eztune_item,
                "MCE-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_CONTROLENABLE:
        eztune_set_item_data(&eztune_item,
                "MCE-ControlEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_PREVIEW_QK:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Qk",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_PREVIEW_Y1:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Green-Y1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_PREVIEW_Y2:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Green-Y2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_PREVIEW_Y3:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Green-Y3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_PREVIEW_Y4:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Green-Y4",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_PREVIEW_YM1:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Green-YM1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_PREVIEW_YM3:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Green-YM3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_PREVIEW_YS1:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Green-YS1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_PREVIEW_YS3:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Green-YS3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_PREVIEW_TRANSWIDTH:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Green-transWidth",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_PREVIEW_TRANSTRUNCATE:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Green-transTruncate",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_PREVIEW_CBZONE:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Green-CbZone",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_PREVIEW_CRZONE:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Green-CrZone",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_PREVIEW_TRANSSLOPE:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Green-transSlope",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_PREVIEW_K:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Green-k",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_PREVIEW_Y1:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Red-Y1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_PREVIEW_Y2:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Red-Y2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_PREVIEW_Y3:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Red-Y3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_PREVIEW_Y4:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Red-Y4",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_PREVIEW_YM1:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Red-YM1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_PREVIEW_YM3:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Red-YM3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_PREVIEW_YS1:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Red-YS1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_PREVIEW_YS3:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Red-YS3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_PREVIEW_TRANSWIDTH:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Red-transWidth",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_PREVIEW_TRANSTRUNCATE:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Red-transTruncate",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_PREVIEW_CBZONE:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Red-CbZone",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_PREVIEW_CRZONE:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Red-CrZone",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_PREVIEW_TRANSSLOPE:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Red-transSlope",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_PREVIEW_K:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Red-k",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_PREVIEW_Y1:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Blue-Y1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_PREVIEW_Y2:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Blue-Y2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_PREVIEW_Y3:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Blue-Y3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_PREVIEW_Y4:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Blue-Y4",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_PREVIEW_YM1:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Blue-YM1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_PREVIEW_YM3:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Blue-YM3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_PREVIEW_YS1:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Blue-YS1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_PREVIEW_YS3:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Blue-YS3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_PREVIEW_TRANSWIDTH:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Blue-transWidth",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_PREVIEW_TRANSTRUNCATE:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Blue-transTruncate",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_PREVIEW_CBZONE:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Blue-CbZone",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_PREVIEW_CRZONE:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Blue-CrZone",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_PREVIEW_TRANSSLOPE:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Blue-transSlope",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_PREVIEW_K:
        eztune_set_item_data(&eztune_item,
                "MCE-Preview-Blue-k",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_SNAPSHOT_QK:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Qk",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_Y1:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Green-Y1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_Y2:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Green-Y2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_Y3:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Green-Y3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_Y4:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Green-Y4",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_YM1:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Green-YM1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_YM3:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Green-YM3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_YS1:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Green-YS1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_YS3:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Green-YS3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_TRANSWIDTH:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Green-transWidth",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_TRANSTRUNCATE:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Green-transTruncate",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_CBZONE:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Green-CbZone",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_CRZONE:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Green-CrZone",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_TRANSSLOPE:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Green-transSlope",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_GREEN_SNAPSHOT_K:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Green-k",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_SNAPSHOT_Y1:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Red-Y1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_SNAPSHOT_Y2:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Red-Y2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_SNAPSHOT_Y3:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Red-Y3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_SNAPSHOT_Y4:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Red-Y4",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_SNAPSHOT_YM1:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Red-YM1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_SNAPSHOT_YM3:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Red-YM3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_SNAPSHOT_YS1:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Red-YS1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_SNAPSHOT_YS3:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Red-YS3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_SNAPSHOT_TRANSWIDTH:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Red-transWidth",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_SNAPSHOT_TRANSTRUNCATE:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Red-transTruncate",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_SNAPSHOT_CBZONE:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Red-CbZone",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_SNAPSHOT_CRZONE:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Red-CrZone",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_SNAPSHOT_TRANSSLOPE:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Red-transSlope",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_RED_SNAPSHOT_K:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Red-k",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_Y1:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Blue-Y1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_Y2:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Blue-Y2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_Y3:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Blue-Y3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_Y4:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Blue-Y4",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_YM1:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Blue-YM1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_YM3:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Blue-YM3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_YS1:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Blue-YS1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_YS3:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Blue-YS3",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_TRANSWIDTH:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Blue-transWidth",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_TRANSTRUNCATE:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Blue-transTruncate",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_CBZONE:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Blue-CbZone",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_CRZONE:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Blue-CrZone",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_TRANSSLOPE:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Blue-transSlope",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MCE_BLUE_SNAPSHOT_K:
        eztune_set_item_data(&eztune_item,
                "MCE-Snapshot-Blue-k",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_WAVELET_ENABLE:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_WAVELET_CONTROLENABLE:
        eztune_set_item_data(&eztune_item,
                "Wavelet-ControlEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_BILATERALSCALECORE0:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Preview-BilateralScaleCore0",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, BILATERAL_LAYERS, 0);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_BILATERALSCALECORE1:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Preview-BilateralScaleCore1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, BILATERAL_LAYERS, 0);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_BILATERALSCALECORE2:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Preview-BilateralScaleCore2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, BILATERAL_LAYERS, 0);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_NOISETHRESHOLDCORE0:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Preview-NoiseThresholdCore0",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_NOISETHRESHOLDCORE1:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Preview-NoiseThresholdCore1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_NOISETHRESHOLDCORE2:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Preview-NoiseThresholdCore2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_WEIGHTCORE0:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Preview-WeightCore0",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_WEIGHTCORE1:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Preview-WeightCore1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_WEIGHTCORE2:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Preview-WeightCore2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_YNOISETHRESH:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Preview-YNoiseThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_YWEIGHT1:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Preview-YWeight1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_YBILATSCALE:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Preview-YBilatScale",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_CBNOISETHRESH:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Preview-CbNoiseThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_CBWEIGHT1:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Preview-CbWeight1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_CBBILATSCALE:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Preview-CbBilatScale",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_CRNOISETHRESH:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Preview-CrNoiseThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_CRWEIGHT1:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Preview-CrWeight1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_PREVIEW_CRBILATSCALE:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Preview-CrBilatScale",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_BILATERALSCALECORE0:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Snapshot-BilateralScaleCore0",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, BILATERAL_LAYERS, 0);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_BILATERALSCALECORE1:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Snapshot-BilateralScaleCore1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, BILATERAL_LAYERS, 0);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_BILATERALSCALECORE2:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Snapshot-BilateralScaleCore2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, BILATERAL_LAYERS, 0);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_NOISETHRESHOLDCORE0:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Snapshot-NoiseThresholdCore0",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_NOISETHRESHOLDCORE1:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Snapshot-NoiseThresholdCore1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_NOISETHRESHOLDCORE2:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Snapshot-NoiseThresholdCore2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_WEIGHTCORE0:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Snapshot-WeightCore0",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_WEIGHTCORE1:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Snapshot-WeightCore1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_WEIGHTCORE2:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Snapshot-WeightCore2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_YNOISETHRESH:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Snapshot-YNoiseThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_YWEIGHT1:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Snapshot-YWeight1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_YBILATSCALE:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Snapshot-YBilatScale",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_CBNOISETHRESH:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Snapshot-CbNoiseThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_CBWEIGHT1:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Snapshot-CbWeight1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_CBBILATSCALE:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Snapshot-CbBilatScale",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_CRNOISETHRESH:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Snapshot-CrNoiseThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_CRWEIGHT1:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Snapshot-CrWeight1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_WAVELET_SNAPSHOT_CRBILATSCALE:
        eztune_set_item_data(&eztune_item,
                "Wavelet-Snapshot-CrBilatScale",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, DENOISE_NUM_PROFILES, 0);
        break;
    case EZT_PARMS_ABF_ENABLE:
        eztune_set_item_data(&eztune_item,
                "ABF-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ABF_CONTROLENABLE:
        eztune_set_item_data(&eztune_item,
                "ABF-ControlEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ABF2_PREVIEW_GREEN_THRESH:
        eztune_set_item_data(&eztune_item,
                "ABF2-Preview-Green-Threshold",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 3, 0);
        break;
    case EZT_PARMS_ABF2_PREVIEW_GREEN_POSLUT:
        eztune_set_item_data(&eztune_item,
                "ABF2-Preview-Green-PosLut",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 16, 0);
        break;
    case EZT_PARMS_ABF2_PREVIEW_GREEN_NEGLUT:
        eztune_set_item_data(&eztune_item,
                "ABF2-Preview-Green-NegLut",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_ABF2_PREVIEW_BLUE_THRESH:
        eztune_set_item_data(&eztune_item,
                "ABF2-Preview-Blue-Threshold",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 3, 0);
        break;
    case EZT_PARMS_ABF2_PREVIEW_BLUE_POSLUT:
        eztune_set_item_data(&eztune_item,
                "ABF2-Preview-Blue-PosLut",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 16, 0);
        break;
    case EZT_PARMS_ABF2_PREVIEW_BLUE_NEGLUT:
        eztune_set_item_data(&eztune_item,
                "ABF2-Preview-Blue-NegLut",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_ABF2_PREVIEW_RED_THRESH:
        eztune_set_item_data(&eztune_item,
                "ABF2-Preview-Red-Threshold",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 3, 0);
        break;
    case EZT_PARMS_ABF2_PREVIEW_RED_POSLUT:
        eztune_set_item_data(&eztune_item,
                "ABF2-Preview-Red-PosLut",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 16, 0);
        break;
    case EZT_PARMS_ABF2_PREVIEW_RED_NEGLUT:
        eztune_set_item_data(&eztune_item,
                "ABF2-Preview-Red-NegLut",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_ABF3_PREVIEW_ENABLE:
        eztune_set_item_data(&eztune_item,
                "ABF3-Preview-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ABF3_PREVIEW_NOISESTD2LUTLEVEL0:
        eztune_set_item_data(&eztune_item,
                "ABF3-Preview-NoiseStd2LUTLevel0",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 128, 0);
        break;
    case EZT_PARMS_ABF3_PREVIEW_NOISESTD2LUTLEVEL1:
        eztune_set_item_data(&eztune_item,
                "ABF3-Preview-NoiseStd2LUTLevel1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 128, 0);
        break;
    case EZT_PARMS_ABF3_PREVIEW_EDGESOFTNESS:
        eztune_set_item_data(&eztune_item,
                "ABF3-Preview-EdgeSoftness",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ABF3_PREVIEW_SOFTTHRESHNOISESCALE:
        eztune_set_item_data(&eztune_item,
                "ABF3-Preview-SoftThreshNoiseScale",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_ABF3_PREVIEW_FILTERSTRENGTH:
        eztune_set_item_data(&eztune_item,
                "ABF3-Preview-FilterStrength",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_ABF3_PREVIEW_LUTBANKSEL:
        eztune_set_item_data(&eztune_item,
                "ABF3-Preview-LutBankSelt",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ABF3_PREVIEW_FILTEROPTIONS:
        eztune_set_item_data(&eztune_item,
                "ABF3-Preview-FilterOptions",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ABF3_PREVIEW_SIGNAL2LUTLEVEL0:
        eztune_set_item_data(&eztune_item,
                "ABF3-Preview-Signal2LUTLevel0",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 32, 0);
        break;
    case EZT_PARMS_ABF3_PREVIEW_SIGNAL2LUTLEVEL1:
        eztune_set_item_data(&eztune_item,
                "ABF3-Preview-Signal2LUTLevel1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 32, 0);
        break;
    case EZT_PARMS_ABF3_PREVIEW_WTABLEADJ:
        eztune_set_item_data(&eztune_item,
                "ABF3-Preview-WTableAdj",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_ABF3_PREVIEW_NOISESCALE0:
        eztune_set_item_data(&eztune_item,
                "ABF3-Preview-NoiseScale0",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_ABF3_PREVIEW_NOISESCALE1:
        eztune_set_item_data(&eztune_item,
                "ABF3-Preview-NoiseScale1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_ABF3_PREVIEW_NOISESCALE2:
        eztune_set_item_data(&eztune_item,
                "ABF3-Preview-NoiseScale2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_ABF3_PREVIEW_NOISEOFFSET:
        eztune_set_item_data(&eztune_item,
                "ABF3-Preview-NoiseOffset",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_ABF3_PREVIEW_SOFTTHRESHNOISESHIFT:
        eztune_set_item_data(&eztune_item,
                "ABF3-Preview-SoftThreshNoiseShift",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_ABF3_PREVIEW_DISTANCEKER0:
        eztune_set_item_data(&eztune_item,
                "ABF3-Preview-DistanceKer0",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 5, 0);
        break;
    case EZT_PARMS_ABF3_PREVIEW_DISTANCEKER1:
        eztune_set_item_data(&eztune_item,
                "ABF3-Preview-DistanceKer1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 5, 0);
        break;
    case EZT_PARMS_ABF3_PREVIEW_MINMAXSEL0:
        eztune_set_item_data(&eztune_item,
                "ABF3-Preview-MinMaxSel0",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 2, 0);
        break;
    case EZT_PARMS_ABF3_PREVIEW_MINMAXSEL1:
        eztune_set_item_data(&eztune_item,
                "ABF3-Preview-MinMaxSel1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 2, 0);
        break;
    case EZT_PARMS_ABF3_PREVIEW_CURVEOFFSET:
        eztune_set_item_data(&eztune_item,
                "ABF3-Preview-CurveOffset",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_ABF2_SNAPSHOT_GREEN_THRESH:
        eztune_set_item_data(&eztune_item,
                "ABF2-Snapshot-Green-Threshold",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 3, 0);
        break;
    case EZT_PARMS_ABF2_SNAPSHOT_GREEN_POSLUT:
        eztune_set_item_data(&eztune_item,
                "ABF2-Snapshot-Green-PosLut",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 16, 0);
        break;
    case EZT_PARMS_ABF2_SNAPSHOT_GREEN_NEGLUT:
        eztune_set_item_data(&eztune_item,
                "ABF2-Snapshot-Green-NegLut",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_ABF2_SNAPSHOT_BLUE_THRESH:
        eztune_set_item_data(&eztune_item,
                "ABF2-Snapshot-Blue-Threshold",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 3, 0);
        break;
    case EZT_PARMS_ABF2_SNAPSHOT_BLUE_POSLUT:
        eztune_set_item_data(&eztune_item,
                "ABF2-Snapshot-Blue-PosLut",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 16, 0);
        break;
    case EZT_PARMS_ABF2_SNAPSHOT_BLUE_NEGLUT:
        eztune_set_item_data(&eztune_item,
                "ABF2-Snapshot-Blue-NegLut",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_ABF2_SNAPSHOT_RED_THRESH:
        eztune_set_item_data(&eztune_item,
                "ABF2-Snapshot-Red-Threshold",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 3, 0);
        break;
    case EZT_PARMS_ABF2_SNAPSHOT_RED_POSLUT:
        eztune_set_item_data(&eztune_item,
                "ABF2-Snapshot-Red-PosLut",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 16, 0);
        break;
    case EZT_PARMS_ABF2_SNAPSHOT_RED_NEGLUT:
        eztune_set_item_data(&eztune_item,
                "ABF2-Snapshot-Red-NegLut",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_ENABLE:
        eztune_set_item_data(&eztune_item,
                "ABF3-Snapshot-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_NOISESTD2LUTLEVEL0:
        eztune_set_item_data(&eztune_item,
                "ABF3-Snapshot-NoiseStd2LUTLevel0",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 128, 0);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_NOISESTD2LUTLEVEL1:
        eztune_set_item_data(&eztune_item,
                "ABF3-Snapshot-NoiseStd2LUTLevel1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 128, 0);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_EDGESOFTNESS:
        eztune_set_item_data(&eztune_item,
                "ABF3-Snapshot-EdgeSoftness",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_SOFTTHRESHNOISESCALE:
        eztune_set_item_data(&eztune_item,
                "ABF3-Snapshot-SoftThreshNoiseScale",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_FILTERSTRENGTH:
        eztune_set_item_data(&eztune_item,
                "ABF3-Snapshot-FilterStrength",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_LUTBANKSEL:
        eztune_set_item_data(&eztune_item,
                "ABF3-Snapshot-LutBankSelt",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_FILTEROPTIONS:
        eztune_set_item_data(&eztune_item,
                "ABF3-Snapshot-FilterOptions",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_SIGNAL2LUTLEVEL0:
        eztune_set_item_data(&eztune_item,
                "ABF3-Snapshot-Signal2LUTLevel0",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 32, 0);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_SIGNAL2LUTLEVEL1:
        eztune_set_item_data(&eztune_item,
                "ABF3-Snapshot-Signal2LUTLevel1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 32, 0);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_WTABLEADJ:
        eztune_set_item_data(&eztune_item,
                "ABF3-Snapshot-WTableAdj",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_NOISESCALE0:
        eztune_set_item_data(&eztune_item,
                "ABF3-Snapshot-NoiseScale0",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_NOISESCALE1:
        eztune_set_item_data(&eztune_item,
                "ABF3-Snapshot-NoiseScale1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_NOISESCALE2:
        eztune_set_item_data(&eztune_item,
                "ABF3-Snapshot-NoiseScale2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_NOISEOFFSET:
        eztune_set_item_data(&eztune_item,
                "ABF3-Snapshot-NoiseOffset",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_SOFTTHRESHNOISESHIFT:
        eztune_set_item_data(&eztune_item,
                "ABF3-Snapshot-SoftThreshNoiseShift",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_DISTANCEKER0:
        eztune_set_item_data(&eztune_item,
                "ABF3-Snapshot-DistanceKer0",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 5, 0);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_DISTANCEKER1:
        eztune_set_item_data(&eztune_item,
                "ABF3-Snapshot-DistanceKer1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 5, 0);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_MINMAXSEL0:
        eztune_set_item_data(&eztune_item,
                "ABF3-Snapshot-MinMaxSel0",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 2, 0);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_MINMAXSEL1:
        eztune_set_item_data(&eztune_item,
                "ABF3-Snapshot-MinMaxSel1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 2, 0);
        break;
    case EZT_PARMS_ABF3_SNAPSHOT_CURVEOFFSET:
        eztune_set_item_data(&eztune_item,
                "ABF3-Snapshot-CurveOffset",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_BPC_ENABLE:
        eztune_set_item_data(&eztune_item,
                "BPC-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BPC_CONTROLENABLE:
        eztune_set_item_data(&eztune_item,
                "BPC-ControlEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BPC_PREVIEW_CURRENT_FMINTHRESHOLD:
        eztune_set_item_data(&eztune_item,
                "BPC-Preview-fminThreshold",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BPC_PREVIEW_CURRENT_FMAXTHRESHOLD:
        eztune_set_item_data(&eztune_item,
                "BPC-Preview-fmaxThreshold",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BPC_PREVIEW_CURRENT_ROFFSETLO:
        eztune_set_item_data(&eztune_item,
                "BPC-Preview-rOffsetLo",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BPC_PREVIEW_CURRENT_ROFFSETHI:
        eztune_set_item_data(&eztune_item,
                "BPC-Preview-rOffsetHi",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BPC_PREVIEW_CURRENT_GROFFSETLO:
        eztune_set_item_data(&eztune_item,
                "BPC-Preview-grOffsetLo",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BPC_PREVIEW_CURRENT_GROFFSETHI:
        eztune_set_item_data(&eztune_item,
                "BPC-Preview-grOffsetHi",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BPC_PREVIEW_CURRENT_GBOFFSETLO:
        eztune_set_item_data(&eztune_item,
                "BPC-Preview-gbOffsetLo",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BPC_PREVIEW_CURRENT_GBOFFSETHI:
        eztune_set_item_data(&eztune_item,
                "BPC-Preview-gbOffsetHi",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BPC_PREVIEW_CURRENT_BOFFSETHI:
        eztune_set_item_data(&eztune_item,
                "BPC-Preview-bOffsetHi",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BPC_PREVIEW_CURRENT_BOFFSETLO:
        eztune_set_item_data(&eztune_item,
                "BPC-Preview-bOffsetLo",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BPC_SNAPSHOT_CURRENT_FMINTHRESHOLD:
        eztune_set_item_data(&eztune_item,
                "BPC-Snapshot-fminThreshold",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BPC_SNAPSHOT_CURRENT_FMAXTHRESHOLD:
        eztune_set_item_data(&eztune_item,
                "BPC-Snapshot-fmaxThreshold",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BPC_SNAPSHOT_CURRENT_ROFFSETLO:
        eztune_set_item_data(&eztune_item,
                "BPC-Snapshot-rOffsetLo",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BPC_SNAPSHOT_CURRENT_ROFFSETHI:
        eztune_set_item_data(&eztune_item,
                "BPC-Snapshot-rOffsetHi",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BPC_SNAPSHOT_CURRENT_GROFFSETLO:
        eztune_set_item_data(&eztune_item,
                "BPC-Snapshot-grOffsetLo",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BPC_SNAPSHOT_CURRENT_GROFFSETHI:
        eztune_set_item_data(&eztune_item,
                "BPC-Snapshot-grOffsetHi",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BPC_SNAPSHOT_CURRENT_GBOFFSETLO:
        eztune_set_item_data(&eztune_item,
                "BPC-Snapshot-gbOffsetLo",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BPC_SNAPSHOT_CURRENT_GBOFFSETHI:
        eztune_set_item_data(&eztune_item,
                "BPC-Snapshot-gbOffsetHi",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BPC_SNAPSHOT_CURRENT_BOFFSETHI:
        eztune_set_item_data(&eztune_item,
                "BPC-Snapshot-bOffsetHi",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BPC_SNAPSHOT_CURRENT_BOFFSETLO:
        eztune_set_item_data(&eztune_item,
                "BPC-Snapshot-bOffsetLo",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BCC_ENABLE:
        eztune_set_item_data(&eztune_item,
                "BCC-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BCC_CONTROLENABLE:
        eztune_set_item_data(&eztune_item,
                "BCC-ControlEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BCC_PREVIEW_CURRENT_FMINTHRESHOLD:
        eztune_set_item_data(&eztune_item,
                "BCC-Preview-fminThreshold",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BCC_PREVIEW_CURRENT_FMAXTHRESHOLD:
        eztune_set_item_data(&eztune_item,
                "BCC-Preview-fmaxThreshold",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BCC_PREVIEW_CURRENT_ROFFSETLO:
        eztune_set_item_data(&eztune_item,
                "BCC-Preview-rOffsetLo",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BCC_PREVIEW_CURRENT_ROFFSETHI:
        eztune_set_item_data(&eztune_item,
                "BCC-Preview-rOffsetHi",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BCC_PREVIEW_CURRENT_GROFFSETLO:
        eztune_set_item_data(&eztune_item,
                "BCC-Preview-grOffsetLo",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BCC_PREVIEW_CURRENT_GROFFSETHI:
        eztune_set_item_data(&eztune_item,
                "BCC-Preview-grOffsetHi",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BCC_PREVIEW_CURRENT_GBOFFSETLO:
        eztune_set_item_data(&eztune_item,
                "BCC-Preview-gbOffsetLo",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BCC_PREVIEW_CURRENT_GBOFFSETHI:
        eztune_set_item_data(&eztune_item,
                "BCC-Preview-gbOffsetHi",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BCC_PREVIEW_CURRENT_BOFFSETHI:
        eztune_set_item_data(&eztune_item,
                "BCC-Preview-bOffsetHi",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BCC_PREVIEW_CURRENT_BOFFSETLO:
        eztune_set_item_data(&eztune_item,
                "BCC-Preview-bOffsetLo",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BCC_SNAPSHOT_CURRENT_FMINTHRESHOLD:
        eztune_set_item_data(&eztune_item,
                "BCC-Snapshot-fminThreshold",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BCC_SNAPSHOT_CURRENT_FMAXTHRESHOLD:
        eztune_set_item_data(&eztune_item,
                "BCC-Snapshot-fmaxThreshold",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BCC_SNAPSHOT_CURRENT_ROFFSETLO:
        eztune_set_item_data(&eztune_item,
                "BCC-Snapshot-rOffsetLo",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BCC_SNAPSHOT_CURRENT_ROFFSETHI:
        eztune_set_item_data(&eztune_item,
                "BCC-Snapshot-rOffsetHi",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BCC_SNAPSHOT_CURRENT_GROFFSETLO:
        eztune_set_item_data(&eztune_item,
                "BCC-Snapshot-grOffsetLo",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BCC_SNAPSHOT_CURRENT_GROFFSETHI:
        eztune_set_item_data(&eztune_item,
                "BCC-Snapshot-grOffsetHi",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BCC_SNAPSHOT_CURRENT_GBOFFSETLO:
        eztune_set_item_data(&eztune_item,
                "BCC-Snapshot-gbOffsetLo",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BCC_SNAPSHOT_CURRENT_GBOFFSETHI:
        eztune_set_item_data(&eztune_item,
                "BCC-Snapshot-gbOffsetHi",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BCC_SNAPSHOT_CURRENT_BOFFSETHI:
        eztune_set_item_data(&eztune_item,
                "BCC-Snapshot-bOffsetHi",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_BCC_SNAPSHOT_CURRENT_BOFFSETLO:
        eztune_set_item_data(&eztune_item,
                "BCC-Snapshot-bOffsetLo",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_ENABLE:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_CONTROLENABLE:
        eztune_set_item_data(&eztune_item,
                "Demosaic-ControlEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_WK:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-Lut-Wk",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 18, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_BK:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-Lut-Bk",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 18, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_LK:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-Lut-Lk",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 18, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_TK:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-Lut-Tk",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 18, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_AG:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-Ag",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_BL:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-Bl",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_DISABLEDIRECTIONALG:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-DisableDirectionalG",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_ENABLEDYNCLAMPG:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-EnableDynClampG",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_DISABLEDIRECTIONALRB:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-DisableDirectionalRB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_ENABLEDYNCLAMPRB:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-EnableDynClampRB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_NOISELEVELG:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-NoiseLevelG",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_NOISELEVELRB:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-NoiseLevelRB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_BU:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-bU",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_DBLU:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-dbLU",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_KRG:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-KRG",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_KBG:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-KBG",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_KGR:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-KGR",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_KGB:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-KGB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_ENABLEV4:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-EnableV4",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_ENABLEDYNCLAMPRBXCL:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-EnableDynClampRBXcl",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_ENABLEDYNCLAMPGXCL:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-EnableDynClampGXcl",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_LAMBDAG:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-LambdaG",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_LAMBDARB:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-LambdaRB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_WGR1:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-Wgr1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_PREVIEW_WGR2:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Preview-Wgr2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_WK:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-Lut-Wk",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 18, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_BK:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-Lut-Bk",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 18, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_LK:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-Lut-Lk",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 18, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_TK:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-Lut-Tk",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 18, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_AG:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-Ag",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_BL:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-Bl",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_DISABLEDIRECTIONALG:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-DisableDirectionalG",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_ENABLEDYNCLAMPG:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-EnableDynClampG",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_DISABLEDIRECTIONALRB:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-DisableDirectionalRB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_ENABLEDYNCLAMPRB:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-EnableDynClampRB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_NOISELEVELG:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-NoiseLevelG",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_NOISELEVELRB:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-NoiseLevelRB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_BU:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-bU",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_DBLU:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-dbLU",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_KRG:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-KRG",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_KBG:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-KBG",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_KGR:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-KGR",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_KGB:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-KGB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_ENABLEV4:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-EnableV4",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_ENABLEDYNCLAMPRBXCL:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-EnableDynClampRBXcl",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_ENABLEDYNCLAMPGXCL:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-EnableDynClampGXcl",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_LAMBDAG:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-LambdaG",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_LAMBDARB:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-LambdaRB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_WGR1:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-Wgr1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMOSAIC_SNAPSHOT_WGR2:
        eztune_set_item_data(&eztune_item,
                "Demosaic-Snapshot-Wgr2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CLFILTER_ENABLE:
        eztune_set_item_data(&eztune_item,
                "CLF-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CLFILTER_CONTROLENABLE:
        eztune_set_item_data(&eztune_item,
                "CLF-ControlEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CHROMAFILTER_PREVIEW_H:
        eztune_set_item_data(&eztune_item,
                "CLF-ChromaFilter-Preview-H",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 4, 0);
        break;
    case EZT_PARMS_CHROMAFILTER_PREVIEW_V:
        eztune_set_item_data(&eztune_item,
                "CLF-ChromaFilter-Preview-V",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 2, 0);
        break;
    case EZT_PARMS_CHROMAFILTER_SNAPSHOT_H:
        eztune_set_item_data(&eztune_item,
                "CLF-ChromaFilter-Snapshot-H",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 4, 0);
        break;
    case EZT_PARMS_CHROMAFILTER_SNAPSHOT_V:
        eztune_set_item_data(&eztune_item,
                "CLF-ChromaFilter-Snapshot-V",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 2, 0);
        break;
    case EZT_PARMS_LUMAFILTER_PREVIEW_THRESHOLDRED:
        eztune_set_item_data(&eztune_item,
                "CLF-LumaFilter-Preview-thresholdRed",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 3, 0);
        break;
    case EZT_PARMS_LUMAFILTER_PREVIEW_SCALEFACTORRED:
        eztune_set_item_data(&eztune_item,
                "CLF-LumaFilter-Preview-scalefactorRed",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 2, 0);
        break;
    case EZT_PARMS_LUMAFILTER_PREVIEW_THRESHOLDGREEN:
        eztune_set_item_data(&eztune_item,
                "CLF-LumaFilter-Preview-thresholdGreen",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 3, 0);
        break;
    case EZT_PARMS_LUMAFILTER_PREVIEW_SCALEFACTORGREEN:
        eztune_set_item_data(&eztune_item,
                "CLF-LumaFilter-Preview-scalefactorGreen",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 2, 0);
        break;
    case EZT_PARMS_LUMAFILTER_PREVIEW_THRESHOLDBLUE:
        eztune_set_item_data(&eztune_item,
                "CLF-LumaFilter-Preview-thresholdBlue",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 3, 0);
        break;
    case EZT_PARMS_LUMAFILTER_PREVIEW_SCALEFACTORBLUE:
        eztune_set_item_data(&eztune_item,
                "CLF-LumaFilter-Preview-scalefactorBlue",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 2, 0);
        break;
    case EZT_PARMS_LUMAFILTER_PREVIEW_A:
        eztune_set_item_data(&eztune_item,
                "CLF-LumaFilter-Preview-A",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 2, 0);
        break;
    case EZT_PARMS_LUMAFILTER_PREVIEW_TABLEPOS:
        eztune_set_item_data(&eztune_item,
                "CLF-LumaFilter-Preview-tablePos",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 16, 0);
        break;
    case EZT_PARMS_LUMAFILTER_PREVIEW_TABLENEG:
        eztune_set_item_data(&eztune_item,
                "CLF-LumaFilter-Preview-tableNeg",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_LUMAFILTER_SNAPSHOT_THRESHOLDRED:
        eztune_set_item_data(&eztune_item,
                "CLF-LumaFilter-Snapshot-thresholdRed",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 3, 0);
        break;
    case EZT_PARMS_LUMAFILTER_SNAPSHOT_SCALEFACTORRED:
        eztune_set_item_data(&eztune_item,
                "CLF-LumaFilter-Snapshot-scalefactorRed",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 2, 0);
        break;
    case EZT_PARMS_LUMAFILTER_SNAPSHOT_THRESHOLDGREEN:
        eztune_set_item_data(&eztune_item,
                "CLF-LumaFilter-Snapshot-thresholdGreen",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 3, 0);
        break;
    case EZT_PARMS_LUMAFILTER_SNAPSHOT_SCALEFACTORGREEN:
        eztune_set_item_data(&eztune_item,
                "CLF-LumaFilter-Snapshot-scalefactorGreen",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 2, 0);
        break;
    case EZT_PARMS_LUMAFILTER_SNAPSHOT_THRESHOLDBLUE:
        eztune_set_item_data(&eztune_item,
                "CLF-LumaFilter-Snapshot-thresholdBlue",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 3, 0);
        break;
    case EZT_PARMS_LUMAFILTER_SNAPSHOT_SCALEFACTORBLUE:
        eztune_set_item_data(&eztune_item,
                "CLF-LumaFilter-Snapshot-scalefactorBlue",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 2, 0);
        break;
    case EZT_PARMS_LUMAFILTER_SNAPSHOT_A:
        eztune_set_item_data(&eztune_item,
                "CLF-LumaFilter-Snapshot-A",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 2, 0);
        break;
    case EZT_PARMS_LUMAFILTER_SNAPSHOT_TABLEPOS:
        eztune_set_item_data(&eztune_item,
                "CLF-LumaFilter-Snapshot-tablePos",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 16, 0);
        break;
    case EZT_PARMS_LUMAFILTER_SNAPSHOT_TABLENEG:
        eztune_set_item_data(&eztune_item,
                "CLF-LumaFilter-Snapshot-tableNeg",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_CAC_NZHV:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-Current-NZHV",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_THRESHEDGE:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-Current-ThreshEdge",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_THRESHSATURATION:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-Current-ThreshSaturation",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_POSSCAPPIXELS_RGHIGHTHRESH:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-Current-PossCapPixelsRGHighThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_POSSCAPPIXELS_RGLOWTHRESH:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-Current-PossCapPixelsRGLowThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_POSSCAPPIXELS_BGHIGHTHRESH:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-Current-PossCapPixelsBGHighThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_POSSCAPPIXELS_BGLOWTHRESH:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-Current-PossCapPixelsBGLowThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_RGDIFFHIGHTHRESH:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-Current-RGDifferenceHighThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_RGDIFFLOWTHRESH:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-Current-RGDifferenceLowThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_BGDIFFHIGHTHRESH:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-Current-BGDifferenceHighThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_BGDIFFLOWTHRESH:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-Current-BGDifferenceLowThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_EDGEHV:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-EdgeHV",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_EDGEDIAG:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-EdgeDiag",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_NZDIAG:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-NZDiag",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_FORCEOFF:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-ForceOff",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_INVSLOPERG:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-InvSlopeRG",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_QISLOPERG:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-QiSlopeRG",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_INVSLOPEBG:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-InvSlopeBG",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_QISLOPEBG:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-QiSlopeBG",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_GAING:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-GainG",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_OFFSETG:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-OffsetG",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_GAINB:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-GainB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_OFFSETB:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-OffsetB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_GAINR:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-GainR",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_CAC_OFFSETR:
        eztune_set_item_data(&eztune_item,
                "PostProc-CAC-OffsetR",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_LINEAR_ENABLE:
        eztune_set_item_data(&eztune_item,
                "Linearization-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_LINEAR_CONTROLENABLE:
        eztune_set_item_data(&eztune_item,
                "Linearization-ControlEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_RLUTPL:
        eztune_set_item_data(&eztune_item,
                "Linearization-Preview-Red-lutPl",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_RLUTBASE:
        eztune_set_item_data(&eztune_item,
                "Linearization-Preview-Red-lutBase",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 9, 0);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_RLUTDELTA:
        eztune_set_item_data(&eztune_item,
                "Linearization-Preview-Red-lutDelata",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 9, 0);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_GRLUTPL:
        eztune_set_item_data(&eztune_item,
                "Linearization-Preview-GreenR-lutPl",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_GRLUTBASE:
        eztune_set_item_data(&eztune_item,
                "Linearization-Preview-GreenR-lutBase",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 9, 0);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_GRLUTDELTA:
        eztune_set_item_data(&eztune_item,
                "Linearization-Preview-GreenR-lutDelata",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 9, 0);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_GBLUTPL:
        eztune_set_item_data(&eztune_item,
                "Linearization-Preview-GreenB-lutPl",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_GBLUTBASE:
        eztune_set_item_data(&eztune_item,
                "Linearization-Preview-GreenB-lutBase",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 9, 0);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_GBLUTDELTA:
        eztune_set_item_data(&eztune_item,
                "Linearization-Preview-GreenB-lutDelata",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 9, 0);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_BLUTPL:
        eztune_set_item_data(&eztune_item,
                "Linearization-Preview-Blue-lutPl",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_BLUTBASE:
        eztune_set_item_data(&eztune_item,
                "Linearization-Preview-Blue-lutBase",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 9, 0);
        break;
    case EZT_PARMS_LINEAR_PREVIEW_BLUTDELTA:
        eztune_set_item_data(&eztune_item,
                "Linearization-Preview-Blue-lutDelata",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 9, 0);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_RLUTPL:
        eztune_set_item_data(&eztune_item,
                "Linearization-Snapshot-Red-lutPl",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_RLUTBASE:
        eztune_set_item_data(&eztune_item,
                "Linearization-Snapshot-Red-lutBase",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 9, 0);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_RLUTDELTA:
        eztune_set_item_data(&eztune_item,
                "Linearization-Snapshot-Red-lutDelata",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 9, 0);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_GRLUTPL:
        eztune_set_item_data(&eztune_item,
                "Linearization-Snapshot-GreenR-lutPl",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_GRLUTBASE:
        eztune_set_item_data(&eztune_item,
                "Linearization-Snapshot-GreenR-lutBase",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 9, 0);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_GRLUTDELTA:
        eztune_set_item_data(&eztune_item,
                "Linearization-Snapshot-GreenR-lutDelata",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 9, 0);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_GBLUTPL:
        eztune_set_item_data(&eztune_item,
                "Linearization-Snapshot-GreenB-lutPl",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_GBLUTBASE:
        eztune_set_item_data(&eztune_item,
                "Linearization-Snapshot-GreenB-lutBase",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 9, 0);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_GBLUTDELTA:
        eztune_set_item_data(&eztune_item,
                "Linearization-Snapshot-GreenB-lutDelata",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 9, 0);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_BLUTPL:
        eztune_set_item_data(&eztune_item,
                "Linearization-Snapshot-Blue-lutPl",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 8, 0);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_BLUTBASE:
        eztune_set_item_data(&eztune_item,
                "Linearization-Snapshot-Blue-lutBase",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 9, 0);
        break;
    case EZT_PARMS_LINEAR_SNAPSHOT_BLUTDELTA:
        eztune_set_item_data(&eztune_item,
                "Linearization-Snapshot-Blue-lutDelata",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 9, 0);
        break;
    case EZT_PARMS_SCE_DENABLE:
        eztune_set_item_data(&eztune_item,
                "SCE-sceEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_CONTROLENABLE:
        eztune_set_item_data(&eztune_item,
                "SCE-sceControlEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_VER0CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle0-Vertex0-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_VER0CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle0-Vertex0-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_VER1CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle0-Vertex1-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_VER1CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle0-Vertex1-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_VER2CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle0-Vertex2-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_VER2CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle0-Vertex2-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_COEFFA:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle0-CoeffA",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_COEFFB:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle0-CoeffB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_COEFFC:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle0-CoeffC",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_COEFFD:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle0-CoeffD",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_COEFFE:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle0-CoeffE",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_COEFFF:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle0-CoeffF",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_MATRIXSHIFT:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle0-MatrixShift",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG0_OFFSETSHIFT:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle0-OffsetShift",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_VER0CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle1-Vertex0-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_VER0CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle1-Vertex0-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_VER1CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle1-Vertex1-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_VER1CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle1-Vertex1-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_VER2CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle1-Vertex2-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_VER2CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle1-Vertex2-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_COEFFA:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle1-CoeffA",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_COEFFB:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle1-CoeffB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_COEFFC:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle1-CoeffC",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_COEFFD:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle1-CoeffD",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_COEFFE:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle1-CoeffE",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_COEFFF:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle1-CoeffF",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_MATRIXSHIFT:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle1-MatrixShift",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG1_OFFSETSHIFT:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle1-OffsetShift",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_VER0CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle2-Vertex0-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_VER0CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle2-Vertex0-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_VER1CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle2-Vertex1-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_VER1CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle2-Vertex1-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_VER2CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle2-Vertex2-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_VER2CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle2-Vertex2-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_COEFFA:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle2-CoeffA",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_COEFFB:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle2-CoeffB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_COEFFC:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle2-CoeffC",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_COEFFD:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle2-CoeffD",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_COEFFE:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle2-CoeffE",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_COEFFF:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle2-CoeffF",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_MATRIXSHIFT:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle2-MatrixShift",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG2_OFFSETSHIFT:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle2-OffsetShift",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_VER0CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle3-Vertex0-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_VER0CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle3-Vertex0-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_VER1CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle3-Vertex1-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_VER1CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle3-Vertex1-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_VER2CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle3-Vertex2-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_VER2CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle3-Vertex2-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_COEFFA:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle3-CoeffA",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_COEFFB:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle3-CoeffB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_COEFFC:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle3-CoeffC",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_COEFFD:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle3-CoeffD",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_COEFFE:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle3-CoeffE",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_COEFFF:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle3-CoeffF",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_MATRIXSHIFT:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle3-MatrixShift",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG3_OFFSETSHIFT:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle3-OffsetShift",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_VER0CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle4-Vertex0-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_VER0CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle4-Vertex0-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_VER1CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle4-Vertex1-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_VER1CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle4-Vertex1-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_VER2CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle4-Vertex2-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_VER2CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle4-Vertex2-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_COEFFA:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle4-CoeffA",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_COEFFB:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle4-CoeffB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_COEFFC:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle4-CoeffC",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_COEFFD:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle4-CoeffD",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_COEFFE:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle4-CoeffE",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_COEFFF:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle4-CoeffF",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_MATRIXSHIFT:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle4-MatrixShift",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_PREVIEW_TRIANG4_OFFSETSHIFT:
        eztune_set_item_data(&eztune_item,
                "SCE-Preview-Triangle4-OffsetShift",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_VER0CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle0-Vertex0-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_VER0CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle0-Vertex0-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_VER1CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle0-Vertex1-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_VER1CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle0-Vertex1-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_VER2CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle0-Vertex2-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_VER2CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle0-Vertex2-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_COEFFA:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle0-CoeffA",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_COEFFB:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle0-CoeffB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_COEFFC:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle0-CoeffC",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_COEFFD:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle0-CoeffD",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_COEFFE:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle0-CoeffE",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_COEFFF:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle0-CoeffF",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_MATRIXSHIFT:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle0-MatrixShift",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG0_OFFSETSHIFT:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle0-OffsetShift",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_VER0CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle1-Vertex0-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_VER0CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle1-Vertex0-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_VER1CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle1-Vertex1-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_VER1CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle1-Vertex1-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_VER2CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle1-Vertex2-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_VER2CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle1-Vertex2-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_COEFFA:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle1-CoeffA",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_COEFFB:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle1-CoeffB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_COEFFC:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle1-CoeffC",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_COEFFD:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle1-CoeffD",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_COEFFE:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle1-CoeffE",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_COEFFF:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle1-CoeffF",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_MATRIXSHIFT:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle1-MatrixShift",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG1_OFFSETSHIFT:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle1-OffsetShift",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_VER0CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle2-Vertex0-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_VER0CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle2-Vertex0-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_VER1CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle2-Vertex1-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_VER1CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle2-Vertex1-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_VER2CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle2-Vertex2-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_VER2CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle2-Vertex2-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_COEFFA:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle2-CoeffA",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_COEFFB:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle2-CoeffB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_COEFFC:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle2-CoeffC",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_COEFFD:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle2-CoeffD",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_COEFFE:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle2-CoeffE",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_COEFFF:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle2-CoeffF",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_MATRIXSHIFT:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle2-MatrixShift",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG2_OFFSETSHIFT:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle2-OffsetShift",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_VER0CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle3-Vertex0-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_VER0CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle3-Vertex0-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_VER1CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle3-Vertex1-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_VER1CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle3-Vertex1-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_VER2CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle3-Vertex2-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_VER2CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle3-Vertex2-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_COEFFA:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle3-CoeffA",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_COEFFB:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle3-CoeffB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_COEFFC:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle3-CoeffC",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_COEFFD:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle3-CoeffD",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_COEFFE:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle3-CoeffE",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_COEFFF:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle3-CoeffF",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_MATRIXSHIFT:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle3-MatrixShift",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG3_OFFSETSHIFT:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle3-OffsetShift",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_VER0CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle4-Vertex0-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_VER0CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle4-Vertex0-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_VER1CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle4-Vertex1-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_VER1CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle4-Vertex1-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_VER2CR:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle4-Vertex2-Cr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_VER2CB:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle4-Vertex2-Cb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_COEFFA:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle4-CoeffA",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_COEFFB:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle4-CoeffB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_COEFFC:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle4-CoeffC",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_COEFFD:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle4-CoeffD",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_COEFFE:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle4-CoeffE",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_COEFFF:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle4-CoeffF",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_MATRIXSHIFT:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle4-MatrixShift",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_SCE_SNAPSHOT_TRIANG4_OFFSETSHIFT:
        eztune_set_item_data(&eztune_item,
                "SCE-Snapshot-Triangle4-OffsetShift",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;

    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_RECONENABLE:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-ReconEnable",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACENABLE:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-MacEnable",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_RECONLINEARMODE:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-ReconLinearMode",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACLINEARMODE:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-MacLinearMode",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_RECONEDGELPFTAP0:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-ReconEdgeLPFTap0",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACMOTIONDILATION:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-MacMotionDilation",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACSMOOTHENABLE:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-MacSmoothEnable",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_RECONFLATREGIONTHRESH:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-ReconFlatRegionThresh",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_RECONMINFACTOR:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-ReconMinFactor",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_RECONHEDGETHRESH1:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-ReconHEdgeThresh1",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_RECONHEDGELOGTHRESHDIFF:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-ReconHEdgeLogThreshDiff",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_RECONMOTIONTHRESH1:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-ReconMotionThresh1",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_RECONMOTIONLOGTHRESHDIFF:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-ReconMotionLogThreshDiff",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_RECONDARKTHRESH1:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-ReconDarkThresh1",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_RECONDARKLOGTHRESHDIFF:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-ReconDarkLogThreshDiff",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACMOTION0THRESH1:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-MacMotion0Thresh1",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACMOTION0THRESH2:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-MacMotion0Thresh2",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MOTION0DT0:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-Motion0DT0",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACMOTIONSTRENGTH:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-MacMotionStrength",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACLOWLIGHTSTRENGTH:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-MacLowlightStrength",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACLOWLIGHTTHRESH1:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-MacLowlightThresh1",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACLOWLIGHTLOGTHRESHDIFF:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-MacLowlightLogThreshDiff",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACBRIGHTTHRESH1:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-MacBrightThresh1",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACBRIGHTLOGTHRESHDIFF:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-MacBrightLogThreshDiff",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACSMOOTHTAP0:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-MacSmoothTap0",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACSMOOTHTHRESH1:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-MacSmoothThresh1",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_CURRENT_MACSMOOTHLOGTHRESHDIFF:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-Current-MacSmoothLogThreshDiff",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_PIXELPATTERN:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-PixelPattern",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_3DENABLE:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-3DEnable",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_LEFTPANELWIDTH:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-LeftPanelWidth",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_FIRSTFIELD:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-FirstField",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_MSBALIGN:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-MSBAlign",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_EXPRATIO:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-ExpRatio",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_EXPRATIORECIP:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-ExpRatioRecip",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_RGWBGAINRATIO:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-RGWBGainRatio",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_BGWBGAINRATIO:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-BGWBGainRatio",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_GRWBGAINRATIO:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-GRWBGainRatio",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_GBWBGAINRATIO:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-GBWBGainRatio",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_POSTPROC_VIDEOHDR_MACSQRTANALOGGAIN:
        eztune_set_item_data(&eztune_item,
            "PostProc-VideoHDR-MacSqrtAnalogGain",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        break;

    case EZT_PARMS_DEMUXCHGAIN_ENABLE:
        eztune_set_item_data(&eztune_item,
                "Demux-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMUXCHGAIN_CONTROLENABLE:
        eztune_set_item_data(&eztune_item,
                "Demux-ControlEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMUXCHGAIN_PREVIEW_GREENEVENROW:
        eztune_set_item_data(&eztune_item,
                "Demux-Preview-GreenEvenRow",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMUXCHGAIN_PREVIEW_GREENODDROW:
        eztune_set_item_data(&eztune_item,
                "Demux-Preview-GreenOddRow",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMUXCHGAIN_PREVIEW_BLUE:
        eztune_set_item_data(&eztune_item,
                "Demux-Preview-Blue",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMUXCHGAIN_PREVIEW_RED:
        eztune_set_item_data(&eztune_item,
                "Demux-Preview-Red",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMUXCHGAIN_SNAPSHOT_GREENEVENROW:
        eztune_set_item_data(&eztune_item,
                "Demux-Snapshot-GreenEvenRow",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMUXCHGAIN_SNAPSHOT_GREENODDROW:
        eztune_set_item_data(&eztune_item,
                "Demux-Snapshot-GreenOddRow",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMUXCHGAIN_SNAPSHOT_BLUE:
        eztune_set_item_data(&eztune_item,
                "Demux-Snapshot-Blue",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_DEMUXCHGAIN_SNAPSHOT_RED:
        eztune_set_item_data(&eztune_item,
                "Demux-Snapshot-Red",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    /* gic diag */
    case EZT_PARMS_GIC_DIAG_ENABLE:
        eztune_set_item_data(&eztune_item,
                "GIC-Current-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_GIC_DIAG_CONTROLENABLE:
        eztune_set_item_data(&eztune_item,
                "GIC-ControlEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_GIC_SOFTTHRESHNOISESCALE:
        eztune_set_item_data(&eztune_item,
                "GIC-Current-SoftThreshNoiseScale",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_GIC_SOFTTHRESHNOISESHIFT:
        eztune_set_item_data(&eztune_item,
                "GIC-Current-SoftThreshNoiseShift",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_GIC_FILTERSTRENGTH:
        eztune_set_item_data(&eztune_item,
                "GIC-Current-FilterStrength",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_GIC_NOISESCALE0:
        eztune_set_item_data(&eztune_item,
                "GIC-NoiseScale0",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_GIC_NOISESCALE1:
        eztune_set_item_data(&eztune_item,
                "GIC-NoiseScale1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_GIC_NOISEOFFSET:
        eztune_set_item_data(&eztune_item,
                "GIC-NoiseOffset",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_GIC_NOISESTD2LUTLEVEL0:
        eztune_set_item_data(&eztune_item,
                "GIC-Current-NoiseStd2LUTLevel0",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 128, 0);
        break;
    case EZT_PARMS_GIC_SIGNAL2LUTLEVEL0:
        eztune_set_item_data(&eztune_item,
                "GIC-Signal2LUTLevel0",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 32, 0);
        break;
    /* gtm diag */
    case EZT_PARMS_GTM_DIAG_ENABLE:
        eztune_set_item_data(&eztune_item,
                "GTM-Current-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_GTM_DIAG_CONTROLENABLE:
        eztune_set_item_data(&eztune_item,
                "GTM-ControlEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_GTM_AMIDDLETONE:
        eztune_set_item_data(&eztune_item,
                "GTM-Current-AMiddleTone",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_GTM_MAXVALTHRESH:
        eztune_set_item_data(&eztune_item,
                "GTM-Current-MaxValThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_GTM_KEYMINTHRESH:
        eztune_set_item_data(&eztune_item,
                "GTM-Current-KeyMinThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_GTM_KEYMAXTHRESH:
        eztune_set_item_data(&eztune_item,
                "GTM-Current-KeyMaxThresh",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_GTM_KEYHISTBINWEIGHT:
        eztune_set_item_data(&eztune_item,
                "GTM-Current-KeyHistBinWeight",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_GTM_YOUTMAXVAL:
        eztune_set_item_data(&eztune_item,
                "GTM-Current-YoutMaxVal",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_GTM_TEMPORALW:
        eztune_set_item_data(&eztune_item,
                "GTM-Current-TemporalW",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_GTM_MIDDLETONEW:
        eztune_set_item_data(&eztune_item,
                "GTM-Current-MiddleToneW",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_GTM_LUTBANKSEL:
        eztune_set_item_data(&eztune_item,
                "GTM-LUTBankSel",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_GTM_XARR:
        eztune_set_item_data(&eztune_item,
                "GTM-Xarr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 64, 0);
        break;
    case EZT_PARMS_GTM_YRATIOBASE:
        eztune_set_item_data(&eztune_item,
                "GTM-YRatioBase",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 64, 0);
        break;
    case EZT_PARMS_GTM_YRATIOSLOPE:
        eztune_set_item_data(&eztune_item,
                "GTM-YRatioSlope",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 64, 0);
        break;
    /* pedestal diag */
    case EZT_PARMS_PEDESTAL_DIAG_ENABLE:
        eztune_set_item_data(&eztune_item,
                "Pedestal-Current-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_PEDESTAL_DIAG_CONTROLENABLE:
        eztune_set_item_data(&eztune_item,
                "Pedestal-ControlEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_PEDESTAL_HDRENABLE:
        eztune_set_item_data(&eztune_item,
                "Pedestal-HDREnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_PEDESTAL_SCALEBYPASS:
        eztune_set_item_data(&eztune_item,
                "Pedestal-ScaleBypass",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_PEDESTAL_INTPFACTOR:
        eztune_set_item_data(&eztune_item,
                "Pedestal-IntpFactor",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_PEDESTAL_BWIDTH:
        eztune_set_item_data(&eztune_item,
                "Pedestal-BWidth",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_PEDESTAL_BHEIGHT:
        eztune_set_item_data(&eztune_item,
                "Pedestal-BHeight",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_PEDESTAL_XDELTA:
        eztune_set_item_data(&eztune_item,
                "Pedestal-XDelta",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_PEDESTAL_YDELTA:
        eztune_set_item_data(&eztune_item,
                "Pedestal-YDelta",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_PEDESTAL_LEFTIMAGEWD:
        eztune_set_item_data(&eztune_item,
                "Pedestal-LeftImageWD",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_PEDESTAL_ENABLE3D:
        eztune_set_item_data(&eztune_item,
                "Pedestal-Enable3D",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_PEDESTAL_MESHGRIDBWIDTH:
        eztune_set_item_data(&eztune_item,
                "Pedestal-MeshGridBWidth",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_PEDESTAL_MESHGRIDBHEIGHT:
        eztune_set_item_data(&eztune_item,
                "Pedestal-MeshGridBHeight",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_PEDESTAL_LXSTART:
        eztune_set_item_data(&eztune_item,
                "Pedestal-LXStart",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_PEDESTAL_LYSTART:
        eztune_set_item_data(&eztune_item,
                "Pedestal-LYStart",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_PEDESTAL_BXSTART:
        eztune_set_item_data(&eztune_item,
                "Pedestal-BXStart",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_PEDESTAL_BYSTART:
        eztune_set_item_data(&eztune_item,
                "Pedestal-BYStart",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_PEDESTAL_BXD1:
        eztune_set_item_data(&eztune_item,
                "Pedestal-BXD1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_PEDESTAL_BYE1:
        eztune_set_item_data(&eztune_item,
                "Pedestal-BYE1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_PEDESTAL_BYINITE1:
        eztune_set_item_data(&eztune_item,
                "Pedestal-BYInitE1",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_PEDESTAL_MESHTABLET1_R:
        eztune_set_item_data(&eztune_item,
                "Pedestal-MeshTableT1-R",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 130, 0);
        break;
    case EZT_PARMS_PEDESTAL_MESHTABLET1_GR:
        eztune_set_item_data(&eztune_item,
                "Pedestal-MeshTableT1-Gr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 130, 0);
        break;
    case EZT_PARMS_PEDESTAL_MESHTABLET1_GB:
        eztune_set_item_data(&eztune_item,
                "Pedestal-MeshTableT1-Gb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 130, 0);
        break;
    case EZT_PARMS_PEDESTAL_MESHTABLET1_B:
        eztune_set_item_data(&eztune_item,
                "Pedestal-MeshTableT1-B",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 130, 0);
        break;
    case EZT_PARMS_PEDESTAL_MESHTABLET2_R:
        eztune_set_item_data(&eztune_item,
                "Pedestal-MeshTableT2-R",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 130, 0);
        break;
    case EZT_PARMS_PEDESTAL_MESHTABLET2_GR:
        eztune_set_item_data(&eztune_item,
                "Pedestal-MeshTableT2-Gr",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 130, 0);
        break;
    case EZT_PARMS_PEDESTAL_MESHTABLET2_GB:
        eztune_set_item_data(&eztune_item,
                "Pedestal-MeshTableT2-Gb",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 130, 0);
        break;
    case EZT_PARMS_PEDESTAL_MESHTABLET2_B:
        eztune_set_item_data(&eztune_item,
                "Pedestal-MeshTableT2-B",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 130, 0);
        break;

    case EZT_PARMS_AWB_MODE:
        eztune_set_item_data(&eztune_item,
                "AWB-Mode",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_ENABLE:
        eztune_set_item_data(&eztune_item,
                "AWB-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_LOCK:
        eztune_set_item_data(&eztune_item,
                "AWB-Lock",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_PREVIEW_RGAIN:
        eztune_set_item_data(&eztune_item,
                "AWB-Preview-WbRGain",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_PREVIEW_GGAIN:
        eztune_set_item_data(&eztune_item,
                "AWB-Preview-WbGGain",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_PREVIEW_BGAIN:
        eztune_set_item_data(&eztune_item,
                "AWB-Preview-WbBGain",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_SNAPSHOT_RGAIN:
        eztune_set_item_data(&eztune_item,
                "AWB-Snapshot-WbRGain",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_SNAPSHOT_GGAIN:
        eztune_set_item_data(&eztune_item,
                "AWB-Snapshot-WbGGain",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_SNAPSHOT_BGAIN:
        eztune_set_item_data(&eztune_item,
                "AWB-Snapshot-WbBGain",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_COLOR_TEMP:
        eztune_set_item_data(&eztune_item,
                "AWB-ColorTemp",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_DECISION:
        eztune_set_item_data(&eztune_item,
                "AWB-Decision",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_DECISIONSTRING:
        eztune_set_item_data(&eztune_item,
                "AWB-DecisionString",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_SAMPLEDECISION:
        eztune_set_item_data(&eztune_item,
                "AWB-SampleDecision",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 64, 0);
        break;
    case EZT_PARMS_AWB_PREVEXPINDEX:
        eztune_set_item_data(&eztune_item,
                "BAWB-CurrentExpIndex",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_OUTLIER_DIST2_DAYLEFT:
        eztune_set_item_data(&eztune_item,
                "BAWB-OutlierDist2Dayleft",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_OUTLIER_DIST2_DAYTOP:
        eztune_set_item_data(&eztune_item,
                "BAWB-OutlierDist2Daytop",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_VALIDSAMPLECOUNT:
        eztune_set_item_data(&eztune_item,
                "BAWB-ValidSampleCnt",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_NOUTLIER:
        eztune_set_item_data(&eztune_item,
                "BAWB-OutlierSampleCnt",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_DAY_RGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-Day-RGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_DAY_BGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-Day-BGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_DAY_CLUSTER:
        eztune_set_item_data(&eztune_item,
                "BAWB-Day-Cluster",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_DAY_CLUSTER_WEIGHTDISTANCE:
        eztune_set_item_data(&eztune_item,
                "BAWB-Day-ClusterWeight-Distance",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_DAY_CLUSTER_WEIGHTILLUMINANT:
        eztune_set_item_data(&eztune_item,
                "BAWB-Day-ClusterWeightIlluminant",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_DAY_CLUSTER_WEIGHTDISILL:
        eztune_set_item_data(&eztune_item,
                "BAWB-Day-ClusterWeightDistIllu",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_F_RGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-F-RGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_F_BGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-F-BGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_F_CLUSTER:
        eztune_set_item_data(&eztune_item,
                "BAWB-F-Cluster",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_F_CLUSTER_WEIGHTDISTANCE:
        eztune_set_item_data(&eztune_item,
                "BAWB-F-ClusterWeight-Distance",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_F_CLUSTER_WEIGHTILLUMINANT:
        eztune_set_item_data(&eztune_item,
                "BAWB-F-ClusterWeight-Illuminant",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_F_CLUSTER_WEIGHTDISILL:
        eztune_set_item_data(&eztune_item,
                "BAWB-F-ClusterWeight-Dist-Illu",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_A_RGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-A-RGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_A_BGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-A-BGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_A_CLUSTER:
        eztune_set_item_data(&eztune_item,
                "BAWB-A-Cluster",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_A_CLUSTER_WEIGHTDISTANCE:
        eztune_set_item_data(&eztune_item,
                "BAWB-A-Cluster-WeightDistance",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_A_CLUSTER_WEIGHTILLUMINANT:
        eztune_set_item_data(&eztune_item,
                "BAWB-A-ClusterWeightIlluminant",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_A_CLUSTER_WEIGHTDISILL:
        eztune_set_item_data(&eztune_item,
                "BAWB-A-ClusterWeightDistIllu",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_H_RGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-H-RGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_H_BGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-H-BGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_H_CLUSTER:
        eztune_set_item_data(&eztune_item,
                "BAWB-H-Cluster",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_H_CLUSTER_WEIGHTDISTANCE:
        eztune_set_item_data(&eztune_item,
                "BAWB-H-ClusterWeightDistance",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_H_CLUSTER_WEIGHTILLUMINANT:
        eztune_set_item_data(&eztune_item,
                "BAWB-H-ClusterWeightIlluminant",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_H_CLUSTER_WEIGHTDISILL:
        eztune_set_item_data(&eztune_item,
                "BAWB-H-ClusterWeightDistIllu",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_SGW_COUNT:
        eztune_set_item_data(&eztune_item,
                "BAWB-SGW-Cnt",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_SGW_RG_RATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-SGW-RGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_SGW_BG_RATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-SGW-BGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_GREENLINE_MX:
        eztune_set_item_data(&eztune_item,
                "BAWB-GreenLineMx",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_GREENLINE_BX:
        eztune_set_item_data(&eztune_item,
                "BAWB-GreenLineBx",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_GREENZONE_TOP:
        eztune_set_item_data(&eztune_item,
                "BAWB-GreenZoneTop",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_GREENZONE_BOTTOM:
        eztune_set_item_data(&eztune_item,
                "BAWB-GreenZoneBottom",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_GREENZONE_LEFT:
        eztune_set_item_data(&eztune_item,
                "BAWB-GreenZoneLeft",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_GREENZONE_RIGHT:
        eztune_set_item_data(&eztune_item,
                "BAWB-GreenZoneRight",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_OUTDOOR_GREEN_RGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-GreenRGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_OUTDOOR_GREEN_BGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-GreenBGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_OUTDOOR_GREEN_GREY_RGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-GreenProj-RGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_OUTDOOR_GREEN_GREY_BGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-GreenProj-BGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_OUTDOOR_GREEN_COUNT:
        eztune_set_item_data(&eztune_item,
                "BAWB-GreenCnt",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_GREEN_PERCENT:
        eztune_set_item_data(&eztune_item,
                "BAWB-GreenPercent",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_SLOPE_FACTOR_M:
        eztune_set_item_data(&eztune_item,
                "BAWB-GreenSlopeFactorM",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_EXTREME_BMAG:
        eztune_set_item_data(&eztune_item,
                "BAWB-ExtremeBMag",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_NONEXTREME_BMAG:
        eztune_set_item_data(&eztune_item,
                "BAWB-NonExtremeBMag",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_AVE_RGRATIOX:
        eztune_set_item_data(&eztune_item,
                "BAWB-AveRGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_AVE_BGRATIOX:
        eztune_set_item_data(&eztune_item,
                "BAWB-AveBGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_WEIGHTEDSAMPLE_RGGRID:
        eztune_set_item_data(&eztune_item,
                "BAWB-Weighted-GreyRGGrid",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_WEIGHTEDSAMPLE_BGGRID:
        eztune_set_item_data(&eztune_item,
                "BAWB-Weighted-GreyBGGrid",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_WEIGHTEDSAMPLE_DAY_RGRATIO:
        eztune_set_item_data(&eztune_item,
                "AWB-WeightedSample-DayRGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_WEIGHTEDSAMPLE_DAY_BGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-Weighted-DayBGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_WEIGHTEDSAMPLE_DAY_SHADE_RGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-Weighted-DayShadeRGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_WEIGHTEDSAMPLE_DAY_SHADE_BGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-Weighted-DayShadeBGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_WEIGHTEDSAMPLE_DAY_D50_RGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-Weighted-DayD50RGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_WEIGHTEDSAMPLE_DAY_D50_BGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-Weighted-DayD50BGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_WEIGHTEDSAMPLE_FAH_RGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-Weighted-FAHRGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_WEIGHTEDSAMPLE_FAH_BGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-Weighted-FAHBGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_WHITE_RGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-WhiteRGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_WHITE_BGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-WhiteBGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_WHITE_STATYTHRESHOLDLOW:
        eztune_set_item_data(&eztune_item,
                "BAWB-WhiteStat-YThresholdLow",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_UNSAT_YMINTHRESHOLD:
        eztune_set_item_data(&eztune_item,
                "BAWB-WhiteStat-YMin",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_UNSAT_Y_MAX:
        eztune_set_item_data(&eztune_item,
                "BAWB-WhiteStat-YMax",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_UNSAT_Y_MID:
        eztune_set_item_data(&eztune_item,
                "BAWB-WhiteStat-YMid",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_UNSAT_Y_DAY_MAX:
        eztune_set_item_data(&eztune_item,
                "BAWB-WhiteStat-YDayMax",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_UNSAT_Y_F_MAX:
        eztune_set_item_data(&eztune_item,
                "BAWB-WhiteStat-YFMax",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_UNSAT_Y_A_MAX:
        eztune_set_item_data(&eztune_item,
                "BAWB-WhiteStat-YAMax",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_UNSAT_Y_H_MAX:
        eztune_set_item_data(&eztune_item,
                "BAWB-WhiteStat-YHMax",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_SAT_DAY_RGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-WhiteStat-DayRGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_SAT_DAY_BGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-WhiteStat-DayBGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_SAT_DAY_CLUSTER:
        eztune_set_item_data(&eztune_item,
                "BAWB-WhiteStat-DayCluster",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_SAT_F_RGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-WhiteStat-FRGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_SAT_F_BGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-WhiteStat-FBGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_SAT_F_CLUSTER:
        eztune_set_item_data(&eztune_item,
                "BAWB-WhiteStat-FCluster",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_SAT_A_RGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-WhiteStat-ARGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_SAT_A_BGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-WhiteStat-ABGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_SAT_A_CLUSTER:
        eztune_set_item_data(&eztune_item,
                "BAWB-WhiteStat-ACluster",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_SAT_H_RGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-WhiteStat-HRGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_SAT_H_BGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-WhiteStat-HBGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_SAT_H_CLUSTER:
        eztune_set_item_data(&eztune_item,
                "BAWB-WhiteStat-HCluster",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_MAX_COMPACT_CLUSTER:
        eztune_set_item_data(&eztune_item,
                "BAWB-MaxCompactCluster",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_COUNT_EXTREMEB_MCC:
        eztune_set_item_data(&eztune_item,
                "BAWB-Count-ExtremeBMcc",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_GREENZONE_RIGHT2:
        eztune_set_item_data(&eztune_item,
                "BAWB-GreenZoneRight2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_GREENLINE_BX2:
        eztune_set_item_data(&eztune_item,
                "BAWB-GreenLineBx2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_GREENZONE_BOTTOM2:
        eztune_set_item_data(&eztune_item,
                "BAWB-GreenZoneBottom2",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_OUTPUT_ISCONFIDENT:
        eztune_set_item_data(&eztune_item,
                "BAWB-Output-IsConfident",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_OUTPUT_SAMPLEDECISION:
        eztune_set_item_data(&eztune_item,
                "BAWB-Output-SampleDecision",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_OUTPUT_WBGAIN_R:
        eztune_set_item_data(&eztune_item,
                "BAWB-Output-WBGainR",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_OUTPUT_WBGAIN_G:
        eztune_set_item_data(&eztune_item,
                "BAWB-Output-WBGainG",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_OUTPUT_WBGAIN_B:
        eztune_set_item_data(&eztune_item,
                "BAWB-Output-WBGainB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_REGULAR_AVE_RGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-Output-WBRGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_REGULAR_AVE_BGRATIO:
        eztune_set_item_data(&eztune_item,
                "BAWB-Output-WBBGRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_CCT_AWBBAYER:
        eztune_set_item_data(&eztune_item,
                "BAWB-Cct",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_COUNT_EXTREMEB:
        eztune_set_item_data(&eztune_item,
                "BAWB-Count-ExtremeB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AWB_BAYER_MIXLEDTABLE_INDEXOVERRIDE:
        eztune_set_item_data(&eztune_item,
                "AWB-Bayer-MixLEDTable-IndexOverride",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_ENABLE:
        eztune_set_item_data(&eztune_item,
                "AEC-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_LUMA:
        eztune_set_item_data(&eztune_item,
                "AEC-Luma",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_PREVIEW_REALGAIN:
        eztune_set_item_data(&eztune_item,
                "AEC-Preview-RealGain",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_PREVIEW_LINECOUNT:
        eztune_set_item_data(&eztune_item,
                "AEC-Preview-LineCount",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_PREVIEW_EXPOSURETIME:
        eztune_set_item_data(&eztune_item,
                "AEC-Preview-ExposureTime msec",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_SNAPSHOT_REALGAIN:
        eztune_set_item_data(&eztune_item,
                "AEC-Snapshot-SensorGain",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_SNAPSHOT_LINECOUNT:
        eztune_set_item_data(&eztune_item,
                "AEC-Snapshot-LineCount",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_SNAPSHOT_EXPOSURETIME:
        eztune_set_item_data(&eztune_item,
                "AEC-Snapshot-ExposureTime msec",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_LOCK:
        eztune_set_item_data(&eztune_item,
                "AEC-Lock",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_SNAPSHOT_FORCEEXP:
        eztune_set_item_data(&eztune_item,
                "AEC-Snapshot-ForceExposure",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_FORCE_SNAPGAIN:
        eztune_set_item_data(&eztune_item,
                "AEC-Snapshot-ForceGain",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_FORCE_SNAPLINECOUNT:
        eztune_set_item_data(&eztune_item,
                "AEC-Snapshot-ForceLineCount",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_PREVIEW_FORCEEXP:
        eztune_set_item_data(&eztune_item,
                "AEC-Preview-ForceExposure",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_FORCE_PREVGAIN:
        eztune_set_item_data(&eztune_item,
                "AEC-Preview-ForceGain",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_FORCE_PREVLINECOUNT:
        eztune_set_item_data(&eztune_item,
                "AEC-Preview-ForceLineCount",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_EXPINDEX:
        eztune_set_item_data(&eztune_item,
                "AEC-ExpIndex",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_LUXINDEX:
        eztune_set_item_data(&eztune_item,
                "AEC-LuxIndex",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_TOUCH_ROILUMA:
        eztune_set_item_data(&eztune_item,
                "AEC-TouchAEC-ROILuma",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_TESTAEC_ENABLE:
        eztune_set_item_data(&eztune_item,
                "AEC-TestAEC-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_FORCEDEXPOSURE_GAIN:
        eztune_set_item_data(&eztune_item,
                "AEC-ForcedExposure-Gain",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_FORCEDEXPOSURE_LINECOUNT:
        eztune_set_item_data(&eztune_item,
                "AEC-ForcedExposure-LineCount",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_HYBRIDLUMA:
        eztune_set_item_data(&eztune_item,
                "AEC-HybridLuma",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_AVERAGELUMA:
        eztune_set_item_data(&eztune_item,
                "AEC-AverageLuma",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_BRIGHTREGIONS:
        eztune_set_item_data(&eztune_item,
                "AEC-BrightRegions",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_REDSEVERITY:
        eztune_set_item_data(&eztune_item,
                "AEC-RedSeverity",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_GREENSEVERITY:
        eztune_set_item_data(&eztune_item,
                "AEC-GreenSeverity",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_BLUESEVERITY:
        eztune_set_item_data(&eztune_item,
                "AEC-BlueSeverity",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_BIMODALSEVERITY:
        eztune_set_item_data(&eztune_item,
                "AEC-BimodalSeverity",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_COMPENSATEDTARGET:
        eztune_set_item_data(&eztune_item,
                "AEC-CompensatedTarget",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_DEFAULTTARGET:
        eztune_set_item_data(&eztune_item,
                "AEC-DefaultTarget",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_CURRENTBRIGHTLEVEL:
        eztune_set_item_data(&eztune_item,
                "AEC-CurrentBrightLevel",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_CURRENTBRIGHTWEIGHT:
        eztune_set_item_data(&eztune_item,
                "AEC-CurrentBrightWeight",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_CURRENTEXPOSURETIME:
        eztune_set_item_data(&eztune_item,
                "AEC-CurrentExposureTime",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_ENTROPYLUMAOFFSET:
        eztune_set_item_data(&eztune_item,
                "AEC-EntropyLumaOffset",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_FLASHOFFEXPINDEX:
        eztune_set_item_data(&eztune_item,
                "AEC-FlashOffExpIndex",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_FLASHLOEXPINDEX:
        eztune_set_item_data(&eztune_item,
                "AEC-FlashLoExpIndex",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_FLASHLOLUMA:
        eztune_set_item_data(&eztune_item,
                "AEC-FlashLoLuma",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_FLASHSENSITIVITYOFF:
        eztune_set_item_data(&eztune_item,
                "AEC-FlashSensitivityOff",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_FLASHSENSITIVITYLO:
        eztune_set_item_data(&eztune_item,
                "AEC-FlashSensitivityLo",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_FLASHSENSITIVIEYHI:
        eztune_set_item_data(&eztune_item,
                "AEC-FlashSensitivityHi",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_FLATSEVERITY:
        eztune_set_item_data(&eztune_item,
                "AEC-FlatSeverity",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_FLATNEARSENSITIVITY:
        eztune_set_item_data(&eztune_item,
                "AEC-FlatNearSeverity",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_FLATBRIGHTDETECTED:
        eztune_set_item_data(&eztune_item,
                "AEC-FlatBrightDetected",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_FLATDARKDETECTED:
        eztune_set_item_data(&eztune_item,
                "AEC-FlatDarkDetected",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_FLATLUMARATIO:
        eztune_set_item_data(&eztune_item,
                "AEC-FlatLumaRatio",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_SSDLUMATARGETOFFSET:
        eztune_set_item_data(&eztune_item,
                "AEC-SSDLumaTargetOffset",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_BSDLUMATARGETOFFSET:
        eztune_set_item_data(&eztune_item,
                "AEC-BSDLumaTargetOffset",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AEC_FORCEDEXPOSURE_ENABLE:
        eztune_set_item_data(&eztune_item,
                "AEC-ForcedExposure-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_ENABLE:
        eztune_set_item_data(&eztune_item,
                "AF-Enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_PEAKLOCATIONINDEX:
        eztune_set_item_data(&eztune_item,
                "AF-PeakLocationIndex",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_ROI_LEFT:
        eztune_set_item_data(&eztune_item,
                "AF-config-offset-x",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_ROI_TOP:
        eztune_set_item_data(&eztune_item,
                "AF-config-offset-y",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_ROI_WIDTH:
        eztune_set_item_data(&eztune_item,
                "AF-config-width",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_ROI_HEIGHT:
        eztune_set_item_data(&eztune_item,
                "AF-config-height",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_GRIDINFO_HNUM:
        eztune_set_item_data(&eztune_item,
                "AF-config-grid-x",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_GRIDINFO_VNUM:
        eztune_set_item_data(&eztune_item,
                "AF-config-grid-y",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_R_FVMIN:
        eztune_set_item_data(&eztune_item,
                "AF-config-Fvmin-R",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_GR_FVMIN:
        eztune_set_item_data(&eztune_item,
                "AF-config-Fvmin-GR",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_GB_FVMIN:
        eztune_set_item_data(&eztune_item,
                "AF-config-Fvmin-GB",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_B_FVMIN:
        eztune_set_item_data(&eztune_item,
                "AF-config-Fvmin-B",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_HPF:
        eztune_set_item_data(&eztune_item,
                "AF-config-HPF",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 10, 0);
        break;
    case EZT_PARMS_AF_MODE:
        eztune_set_item_data(&eztune_item,
                "AF-mode",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_STATUS:
        eztune_set_item_data(&eztune_item,
                "AF-status",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_FAREND:
        eztune_set_item_data(&eztune_item,
                "AF-farend",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_NEAREND:
        eztune_set_item_data(&eztune_item,
                "AF-nearend",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_HYPPOS:
        eztune_set_item_data(&eztune_item,
                "AF-panfocus",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_STATE:
        eztune_set_item_data(&eztune_item,
                "SAF-state",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_STATS_INDEX:
        eztune_set_item_data(&eztune_item,
                "AF-index",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_STATS_POS:
        eztune_set_item_data(&eztune_item,
                "AF-lenspos",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_STATS_FV:
        eztune_set_item_data(&eztune_item,
                "AF-FV",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 50, 0);
        break;
    case EZT_PARMS_AF_STATS_MAXFV:
        eztune_set_item_data(&eztune_item,
                "AF-maxFv",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_STATS_MINFV:
        eztune_set_item_data(&eztune_item,
                "AF-minFv",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_FRAMEDELAY:
        eztune_set_item_data(&eztune_item,
                "AF-framedelay",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_ENABLEMULTIWINDOW:
        eztune_set_item_data(&eztune_item,
                "SAF-mv-enable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_MWIN:
        eztune_set_item_data(&eztune_item,
                "SAF-mv-fv",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                1, 14, 0);
        break;
    case EZT_PARMS_AF_NUMDOWNHILL:
        eztune_set_item_data(&eztune_item,
                "AF-NumDownHill",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_CURRLUMA:
        eztune_set_item_data(&eztune_item,
                "AF-cur-luma",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_EXPINDEX:
        eztune_set_item_data(&eztune_item,
                "AF-exp-index",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_LUMASETTLEDCOUNT:
        eztune_set_item_data(&eztune_item,
                "AF-luma-settle-cnt",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_AVEFV:
        eztune_set_item_data(&eztune_item,
                "AF-AveFv",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_CAF_STATE:
        eztune_set_item_data(&eztune_item,
                "CAF-state",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_CAF_PANNING_UNSTABLECOUNT:
        eztune_set_item_data(&eztune_item,
                "CAF-panning-unstable-cnt",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_CAF_PANNING_STABLECOUNT:
        eztune_set_item_data(&eztune_item,
                "CAF-panning-stable-cnt",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_CAF_PANNING_STABLE:
        eztune_set_item_data(&eztune_item,
                "CAF-PanningStable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_CAF_SAD_CHANGE:
        eztune_set_item_data(&eztune_item,
                "CAF-SAD-flag",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_CAF_EXP_CHANGE:
        eztune_set_item_data(&eztune_item,
                "CAF-exp-change-flag",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_CAF_LUMACHNG_DURINGSRCH:
        eztune_set_item_data(&eztune_item,
                "CAF-luma-change-flag",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_CAF_TRIG_REFOCUS:
        eztune_set_item_data(&eztune_item,
                "CAF-refocus-flag",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AF_CAF_GYRO_ASSISTED_PANNING:
        eztune_set_item_data(&eztune_item,
                "CAF-gyro-refocus",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ASD_BLS_DETECTED:
        eztune_set_item_data(&eztune_item,
                "ASD-BacklitScene-AWBBacklitDetected",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ASD_BLS_HISTBLKLITDETECTED:
        eztune_set_item_data(&eztune_item,
                "ASD-BacklitScene-HistogramBacklitDetected",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ASD_BLS_SEVERITY:
        eztune_set_item_data(&eztune_item,
                "ASD-BacklitScene-Severity",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ASD_BLS_LUMACOMPENSATIONOFFSET:
        eztune_set_item_data(&eztune_item,
                "ASD-BacklitScene-LumaCompensationOffset",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ASD_SNS_LUMATHRESHOLD:
        eztune_set_item_data(&eztune_item,
                "ASD-SnowScene-LumaThreshold",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ASD_SNS_AWBYMAXINGRAY:
        eztune_set_item_data(&eztune_item,
                "ASD-SnowScene-AWBYmaxInGray",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ASD_SNS_MINSAMPLETHRESHOLD:
        eztune_set_item_data(&eztune_item,
                "ASD-SnowScene-MinSampleThreshold",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ASD_SNS_EXTREAMSAMPLETHRESHOLD:
        eztune_set_item_data(&eztune_item,
                "ASD-SnowScene-ExtremeSampleThreshold",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ASD_SNS_SEVERITY:
        eztune_set_item_data(&eztune_item,
                "ASD-SnowScene-Severity",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ASD_SNS_INITIAL_LUMAOFFSET:
        eztune_set_item_data(&eztune_item,
                "ASD-SnowScene-InitialLumaOffset",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ASD_SNS_SCALED_LUMAOFFSET:
        eztune_set_item_data(&eztune_item,
                "ASD-SnowScene-ScaledLumaOffset",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ASD_SNS_FILTERED_LUMAOFFSET:
        eztune_set_item_data(&eztune_item,
                "ASD-SnowScene-FilteredLumaOffset",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AFD_PREVIEW_FLICKERDECTIONAPPLIED:
        eztune_set_item_data(&eztune_item,
                "AFD-FlickerDetApplied",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AFD_FLICKERFREQ:
        eztune_set_item_data(&eztune_item,
                "AFD-FlickerFreq",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AFD_STATUS:
        eztune_set_item_data(&eztune_item,
                "AFD-Status",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AFD_STDWIDTH:
        eztune_set_item_data(&eztune_item,
                "AFD-StdWidth",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AFD_MULTIPLEPEAKALGO:
        eztune_set_item_data(&eztune_item,
                "AFD-MultiplePeakAlgo",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AFD_ACTUALPEAKS:
        eztune_set_item_data(&eztune_item,
                "AFD-ActualPeaks",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ANTIBANDING_PIXELCLK:
        eztune_set_item_data(&eztune_item,
                "Antibanding-PixelClk",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_ANTIBANDING_PIXELCLKPERLINE:
        eztune_set_item_data(&eztune_item,
                "Antibanding-PixelClkPerLine",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;

    case EZT_PARMS_MISC_FLASHMODE:
        eztune_set_item_data(&eztune_item,
                "Misc-FlashMode",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MISC_CHIPSETID:
        eztune_set_item_data(&eztune_item,
                "Misc-ChipsetId",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MISC_TUNINGDATATIMESTAMP:
        eztune_set_item_data(&eztune_item,
                "Misc-TuningDataTimeStamp",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MISC_TUNINGDATANAME:
        eztune_set_item_data(&eztune_item,
                "Misc-TuningDataName",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MISC_TUNINGDATAUPDATEDBY:
        eztune_set_item_data(&eztune_item,
                "Misc-TuningDataUpdatedBy",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_MISC_PARAMETERRETENTIONENABLE:
        eztune_set_item_data(&eztune_item,
                "Misc-ParameterRetentionEnable",
                EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_WRITE_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;

    /* AF-Tune fields required by AF-tuning */
    case EZT_PARMS_AFTUNE_TUNING_REGIONSIZE:
        eztune_set_item_data(&eztune_item,
                "Tuning-RegionParams-ValidSize",
                EZT_D_INVALID, EZT_T_AUTOFOCUS, EZT_AUTOFOCUS_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AFTUNE_TUNING_REGIONPARAMS_MACROSTEPBOUND:
        eztune_set_item_data(&eztune_item,
                "Tuning-RegionParams-StepBound-Macro",
                EZT_D_INVALID, EZT_T_AUTOFOCUS, EZT_AUTOFOCUS_FLAG,
                MAX_ACTUATOR_REGION, MAX_ACTUATOR_REGION, 0);
        break;
    case EZT_PARMS_AFTUNE_TUNING_REGIONPARAMS_INFSTEPBOUND:
        eztune_set_item_data(&eztune_item,
                "Tuning-RegionParams-StepBound-Infinity",
                EZT_D_INVALID, EZT_T_AUTOFOCUS, EZT_AUTOFOCUS_FLAG,
                MAX_ACTUATOR_REGION, MAX_ACTUATOR_REGION, 0);
        break;
    case EZT_PARMS_AFTUNE_TUNING_INITIALCODE:
        eztune_set_item_data(&eztune_item,
                "Tuning-InitialCode",
                EZT_D_INVALID, EZT_T_AUTOFOCUS, EZT_AUTOFOCUS_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AFTUNE_TUNING_TEST_DEFFOCUS_ENABLE:
        eztune_set_item_data(&eztune_item,
                "Tuning-Test-EnableDefaultFocus",
                EZT_D_INVALID, EZT_T_AUTOFOCUS, EZT_AUTOFOCUS_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AFTUNE_TUNING_TEST_MOVEFOCUS_ENABLE:
        eztune_set_item_data(&eztune_item,
                "Tuning-Test-MoveFocus-Enable",
                EZT_D_INVALID, EZT_T_AUTOFOCUS, EZT_AUTOFOCUS_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AFTUNE_TUNING_TEST_MOVEFOCUS_DIR:
        eztune_set_item_data(&eztune_item,
                "Tuning-Test-MoveFocus-Dir",
                EZT_D_INVALID, EZT_T_AUTOFOCUS, EZT_AUTOFOCUS_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;
    case EZT_PARMS_AFTUNE_TUNING_TEST_MOVEFOCUS_STEPS:
        eztune_set_item_data(&eztune_item,
                "Tuning-Test-MoveFocus-NumSteps",
                EZT_D_INVALID, EZT_T_AUTOFOCUS, EZT_AUTOFOCUS_FLAG,
                0, EZT_SIZE_DIAG, 0);
        break;

    default:
        EZLOGV("eztune_get_diag_item: invalid id %d!!!", i);
        eztune_set_item_data(&eztune_item,
            "unknown",
            EZT_D_INVALID, EZT_T_DIGANOSTIC, EZT_READ_FLAG,
            0, EZT_SIZE_DIAG, 0);
        snprintf(eztune_item.name, EZTUNE_FORMAT_MAX,
            "unknown_id_%d", eztune_item.id);
        break;
    }
    EZLOGV("eztune_get_diag_item: index=%d name=%s\n", eztune_item.id, eztune_item.name);
    return eztune_item;
}

