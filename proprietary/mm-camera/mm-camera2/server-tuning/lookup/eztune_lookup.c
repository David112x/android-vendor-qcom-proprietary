/***************************************************************************
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ***************************************************************************/

#include <stddef.h>
#include <string.h>
#include <assert.h>

#include "eztune.h"
#include "eztune_logs.h"
#include "eztune_lookup.h"
#include "eztune_lookup_internal.h"
#include "eztune_items_diag.h"

//////////////////////////////////////////////////////////
// local (static) function delarations & forward declares

static eztune_t ezctrl_local;
static eztune_t *ezctrl = &ezctrl_local;

static char *eztune_get_chromatix_ptr(
    eztune_t *ezctrl,
    eztune_item_t *item,
    int32_t table_index);

// forward declare diag accessors
extern eztune_item_t eztune_diag_get_item(
    int i);

extern int eztune_diag_get_item_value(
    eztune_t *ezctrl,
    eztune_item_t *item,
    int32_t table_index,
    char *output_buf);

extern void eztune_diag_set_item_value(
    eztune_t *ezctrl,
    eztune_item_t *item,
    eztune_set_val_t *item_data);

// forward declare chromatix look-up-table
extern const eztune_item_t lookup_items_chromatix[];

//////////////////////////////////////////////////////////
// function implemenataions

void eztune_lookup_init(eztune_init_t *eztune_init)
{
    /* sanity check - on table indexes */
    EZ_ASSERT(lookup_items_chromatix[EZT_PREV_CHROMATIX_HEADER_MARKER].id ==
        EZT_PREV_CHROMATIX_HEADER_MARKER,
        "EZTune: chromatix look-up-table alignment issue");
    EZ_ASSERT(lookup_items_chromatix[EZT_CHROMATIX_PARAMS_MAX - 1].id ==
        (EZT_CHROMATIX_PARAMS_MAX - 1),
        "EZTune: chromatix look-up-table alignment issue");

    ezctrl->metadata = eztune_init->metadata;

    ezctrl->chromatixptr = eztune_init->chromatixptr;
    ezctrl->snap_chromatixptr = eztune_init->snap_chromatixptr;
    ezctrl->common_chromatixptr = eztune_init->common_chromatixptr;
    ezctrl->cpp_chromatixptr = eztune_init->cpp_chromatixptr;
    ezctrl->snap_cpp_chromatixptr = eztune_init->snap_cpp_chromatixptr;
    ezctrl->swpp_chromatixptr = eztune_init->swpp_chromatixptr;
    ezctrl->aaa_chromatixptr = eztune_init->aaa_chromatixptr;

    ezctrl->af_driver_ptr = (actuator_driver_params_t*)eztune_init->af_driver_ptr;
    ezctrl->af_tuning_ptr = (ez_af_tuning_params_t*)eztune_init->af_tuning_ptr;

    ezctrl->tuning_set_vfe = (void (*)(vfemodule_t, optype_t, int32_t))eztune_init->tuning_set_vfe;
    ezctrl->tuning_set_pp = (void (*)(pp_module_t, optype_t, int32_t))eztune_init->tuning_set_pp;
    ezctrl->tuning_set_3a = (void (*)(aaa_set_optype_t, int32_t))eztune_init->tuning_set_3a;
    ezctrl->tuning_set_focus = (void (*)(void*, aftuning_optype_t, int32_t))eztune_init->tuning_set_focus;
    ezctrl->tuning_post_bus_msg = (void (*)(eztune_bus_msg_t, int32_t))eztune_init->tuning_post_bus_msg;

    EZLOGI("EZTune: eztune_lookup_init done meta_ptr=0x%p chromatix_prt=0x%p af_ptr=%p",
        ezctrl->metadata, ezctrl->chromatixptr, ezctrl->af_driver_ptr);
}

void eztune_lookup_get_range_tags(
  int32_t *out_id_chromatix_start,
  int32_t *out_id_chromatix_end,
  int32_t *out_id_diags_start,
  int32_t *out_id_diags_end)
{
    *out_id_chromatix_start = EZT_PREV_CHROMATIX_HEADER_MARKER;
    *out_id_chromatix_end   = EZT_CHROMATIX_PARAMS_MAX;
    *out_id_diags_start     = EZT_PARMS_DIAG_HEADER_MARKER;
    *out_id_diags_end       = EZT_PARMS_DIAG_MAX;
}

int32_t eztune_lookup_item(
  int32_t id,
  char *output,
  int32_t output_size)
{
    int32_t i, offset;
    eztune_item_t item;

    item = eztune_get_item(id);

    /* ignore all preview fields */
    if(strstr(item.name, "preview") != NULL) {
        return 0;
    }

    /* generate string - note NULL ptrs will return 0 */
    offset = 0;
    offset += snprintf(&output[offset], (output_size - offset), "%s", item.name);
    for(i = 0; i < item.entry_count; i++) {
        offset += snprintf(&output[offset], (output_size - offset), "%s", (i==0 ? "=" : ","));
        offset += eztune_get_item_value(&item, i, &output[offset]);
    }
    return offset;
}

static char *eztune_get_chromatix_ptr(
    eztune_t *ezctrl,
    eztune_item_t *item,
    int32_t table_index)
{
    char *ptr = NULL;

    switch(item->base_type) {
    case PREV_CHROMATIX_PARMS_TYPE:
        ptr = (char *)ezctrl->chromatixptr;
        break;
    case SNAP_CHROMATIX_PARMS_TYPE:
        ptr = (char *)ezctrl->snap_chromatixptr;
        break;

    case PREV_CHROMATIX_VFE_COMMON_TYPE:
    case SNAP_CHROMATIX_VFE_COMMON_TYPE:
        ptr = (char *)ezctrl->common_chromatixptr;
        break;

    case PREV_CHROMATIX_CPP_TYPE:
        ptr = (char *)ezctrl->cpp_chromatixptr;
        break;
    case SNAP_CHROMATIX_CPP_TYPE:
         ptr = (char *)ezctrl->snap_cpp_chromatixptr;
        break;

    case PREV_CHROMATIX_SW_POSTPROC_TYPE:
    case SNAP_CHROMATIX_SW_POSTPROC_TYPE:
        ptr = (char *)ezctrl->swpp_chromatixptr;
        break;

    case PREV_CHROMATIX_3A_PARMS_TYPE:
    case SNAP_CHROMATIX_3A_PARMS_TYPE:
        ptr = (char *)ezctrl->aaa_chromatixptr;
        break;

    default:
        EZLOGE("Invalid item->base_type");
        ptr = NULL;
        break;
    };

    if(ptr != NULL) {
        ptr = ptr + item->offset + table_index * item->step_size;
    }
    return ptr;
}

eztune_item_t eztune_get_item(int i)
{
    eztune_item_t item;

    if(i < EZT_CHROMATIX_PARAMS_MAX) { /* chromatix && 3A */
        EZ_ASSERT(i == lookup_items_chromatix[i].id,
            "EZTune: chromatix look-up-table alignment issue");
        return lookup_items_chromatix[i];
    } else if (i < EZT_PARMS_DIAG_MAX) { /* diagnostics */
        return eztune_diag_get_item(i);
    } else {
        eztune_item_t item;
        memset(&item, 0, sizeof(item));
        EZLOGE("Invalid item.id");
        return item;
    }
}

int32_t eztune_get_item_value(
    eztune_item_t *item,
    uint32_t table_index,
    char *output_buf)
{
    int32_t rc = -1;

    if(item->id < EZT_CHROMATIX_PARAMS_MAX) { /* chromatix && 3A */
        char *ptr = eztune_get_chromatix_ptr(ezctrl, item, table_index);
        if(ptr != NULL) {
            switch (item->format) {
            case EZT_D_FLOAT:
                rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%f", *((float *)ptr));
                break;
            case EZT_D_INT8:
                rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", *((int8_t *)ptr));
                break;
            case EZT_D_INT16:
                rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", *((int16_t *)ptr));
                break;
            case EZT_D_INT32:
                rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", *((int32_t *)ptr));
                break;
            case EZT_D_UINT8:
                rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", *((uint8_t *)ptr));
                break;
            case EZT_D_UINT16:
                rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", *((uint16_t *)ptr));
                break;
            case EZT_D_UINT32:
                rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", *((uint32_t *)ptr));
                break;
            case EZT_D_DOUBLE:
                rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%lf", *((double *)ptr));
                break;
            default:
                ALOGE("Invalid item.data");
                break;
            } // switch (item->format)
        } //if(ptr != NULL)
    } else if (item->id < EZT_PARMS_DIAG_MAX) { /* diagnostics */
        rc = eztune_diag_get_item_value(ezctrl, item, table_index, output_buf);
    } else {
        EZLOGE("Error: Invalid item.id");
    }

    if (rc <= 0) {
        EZLOGE("Error: Invalid item (%s). Writing 0 to avoid crash: "
               "item.id (%d), table_index(%d), item.format(%d)",
                item->name, item->id, table_index, item->format);
        rc = snprintf(output_buf, EZTUNE_FORMAT_MAX, "%d", 0);
    }

    return rc;
}

void eztune_set_item_value(
    eztune_item_t *item,
    eztune_set_val_t *item_data)
{
    int32_t rc = 0;
    static uint8_t aec_force_lock = 0, aec_enable = 1;

    EZLOGV("eztune_set_item_value: name=%s id=%d index=%d value=%s",
           item->name, item->id, item_data->table_index, item_data->value_string);

    if (item->id < EZT_CHROMATIX_PARAMS_MAX) { /* chromatix && 3A */
        char *ptr = eztune_get_chromatix_ptr(ezctrl, item, item_data->table_index);
        if(ptr == NULL) {
            EZLOGE("Error: eztune_set_item_value ptr NULL");
            rc = -1;
            goto end;
        }

        switch (item->format) {
        case EZT_D_FLOAT:
            EZLOGV("case EZT_D_FLOAT string %s value %f",
                 item_data->value_string, (float)atof(item_data->value_string));
            *((float *) ptr) = atof(item_data->value_string);
            break;
        case EZT_D_INT8:
            EZLOGV("case EZT_D_INT8 string %s value %d",
                 item_data->value_string, (int8_t)atoi(item_data->value_string));
            *((int8_t *) ptr) = atoi(item_data->value_string);
            break;
        case EZT_D_INT16:
            EZLOGV("case EZT_D_INT16 string %s value %d",
                 item_data->value_string, (int16_t)atoi(item_data->value_string));
            *((int16_t *) ptr) = atoi(item_data->value_string);
            break;
        case EZT_D_INT32:
             EZLOGV("case EZT_D_INT32 string %s value %d",
                 item_data->value_string, atoi(item_data->value_string));
            *((int32_t *) ptr) = atoi(item_data->value_string);
            break;
        case EZT_D_UINT8:
             EZLOGV("case EZT_D_UINT8 string %s value %d",
                 item_data->value_string, (uint8_t)atoi(item_data->value_string));
            *((uint8_t *) ptr) = atoi(item_data->value_string);
            break;
        case EZT_D_UINT16:
             EZLOGV("case EZT_D_UINT16 string %s value %d",
                 item_data->value_string, (uint16_t)atoi(item_data->value_string));
            *((uint16_t *) ptr) = atoi(item_data->value_string);
            break;
        case EZT_D_UINT32:
             EZLOGV("case EZT_D_UINT32 string %s value %d",
                 item_data->value_string, atoi(item_data->value_string));
            *((uint32_t *) ptr) = atoi(item_data->value_string);
            break;
        case EZT_D_DOUBLE:
             EZLOGV("case EZT_D_DOUBLE string %s value %f",
                 item_data->value_string, atof(item_data->value_string));
            *((double *) ptr) = atof(item_data->value_string);
            break;
        default:
            EZLOGE("Error: eztune_set_item_value not supported (%d %s)",
                item->id, item->name);
            rc = -1;
            break;
        }
    } else if (item->id < EZT_PARMS_DIAG_MAX) { /* diagnostics */
        eztune_diag_set_item_value(ezctrl, item, item_data);
    } else {
        EZLOGE("Error: eztune_set_item_value id %d too large (max %d)",
            item->id, EZT_PARMS_DIAG_MAX);
        rc = -1;
    }

end:
    if(rc < 0) {
        EZLOGE("Error: eztune_set_item_value item_id=%d item_name=%s",
            item->id, item->name);
    }
}
