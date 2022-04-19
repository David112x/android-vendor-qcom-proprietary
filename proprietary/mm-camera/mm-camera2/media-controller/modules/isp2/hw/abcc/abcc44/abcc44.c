/* abcc44.c
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* std headers */
#include <unistd.h>
#include <math.h>

/* mctl headers */

/* isp headers */
#include "isp_log.h"
#include "isp_defs.h"
#include "isp_sub_module_util.h"
#include "abcc44.h"
#include "abcc_algo.h"
#include "isp_pipeline_reg.h"

/* #define ABCC44_DEBUG */
#ifdef ABCC44_DEBUG
#undef ISP_DBG
#define ISP_DBG ISP_HIGH
#endif

#define CLAMP(a,b,c) (((a) <= (b))? (b) : (((a) >= (c))? (c) : (a)))

/** abcc44_set_stream_config:
 *
 *  @module: module
 *  @isp_sub_module: isp sub module
 *  @event: mct event
 *
 *  copy sensor out dimention info
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean abcc44_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  abcc44_t                 *abcc2 = NULL;
  sensor_out_info_t       *sensor_out_info = NULL;

  if (!isp_sub_module || !event || !module) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, event, module);
    return FALSE;
  }

  sensor_out_info =
    (sensor_out_info_t *)event->u.module_event.module_event_data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  abcc2 = (abcc44_t *)isp_sub_module->private_data;
  if (!abcc2) {
    ISP_ERR("failed: abcc2 %p", abcc2);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  abcc2->abcc_subblock.abcc_info.sensor_width =
    sensor_out_info->request_crop.last_pixel
    - sensor_out_info->request_crop.first_pixel + 1;

  abcc2->abcc_subblock.abcc_info.sensor_height =
    sensor_out_info->request_crop.last_line -
    sensor_out_info->request_crop.first_line + 1;

  abcc2->abcc_subblock.lut_bank_select = 0;

  abcc2->abcc_subblock.input_width =
    abcc2->abcc_subblock.abcc_info.sensor_width;
  abcc2->abcc_subblock.input_height =
    abcc2->abcc_subblock.abcc_info.sensor_height;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* abcc44_set_stream_config */

/** abcc44_remove_duplicate_defective_pixels:
 *
 *  @defective_pixels: defective pixel array from sensor
 *
 *  remove duplicate pixels if any in the input defective pix
 *  array
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean abcc44_remove_duplicate_defective_pixels(
  defective_pix_array_t *defective_pixels)
{
  defective_pix_array_t out_defective_pixels;
  pix_t *pix;
  int count;
  int i = 0, j, k = 0;

  if (!defective_pixels) {
    ISP_ERR("failed: defective_pixels %p", defective_pixels);
    return FALSE;
  }

  count = defective_pixels->count;
  pix = defective_pixels->pix;

  /* Can be optimized by doing this in place*/
  while (i < count) {
    if((pix[i].x == 0xffff) && (pix[i].y == 0xffff))
  {
    i++;
    continue;
  }
    out_defective_pixels.pix[k].x = pix[i].y;
    out_defective_pixels.pix[k].y = pix[i].x;
    k++;
    for(j = i+1; j < count ; j++) {
      if (PIX_CMP(pix[i], pix[j]))
    {
      pix[j].x = 0xffff;
      pix[j].y = 0xffff;
    }
    }
  i++;
  }

  ISP_DBG("%s: count %d", __func__, k);
  out_defective_pixels.count = k;

  *defective_pixels = out_defective_pixels;

  return TRUE;
} /* end abcc44_remove_duplicate_defective_pixels */

/** abcc44_abcc_prepare_lut_tables:
 *
 *  @abcc: abcc subblock data
 *
 *  prepare LUT tables for HW config
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean abcc44_abcc_prepare_lut_tables(abcc_t *abcc)
{
  abcc_packed_lut_t *packed_lut;
  int i = 0;
  abcc_entry_t *entry;
  int pix_index;
  int width;
  int count;

  if (!abcc) {
    ISP_ERR("failed: abcc %p", abcc);
    return FALSE;
  }

  packed_lut = &abcc->packed_lut;
  width = abcc->input_width;
  count = abcc->abcc_info.lut.final_lut_list.actual_count;

  ISP_DBG("count %d", count);

  for (i = 0; i < count; i++) {
    entry = &(abcc->abcc_info.lut.final_lut_list.entry[i]);
    pix_index = PIX_IND(entry->pixel_index, width);
    packed_lut->lut[i] = ((uint32_t)pix_index << 9);
    packed_lut->lut[i] |= ((uint32_t)(entry->skip_index[1]) << 6);
    packed_lut->lut[i] |= ((uint32_t)(entry->skip_index[0]) << 3);
    packed_lut->lut[i] |= (uint32_t)(entry->kernel_index);
    ISP_DBG("Final LUT[%d] pix_index %d skip_index %d %d kern_ind %d final %llx",
      i, pix_index, entry->skip_index[0], entry->skip_index[1],
      entry->kernel_index, packed_lut->lut[i]);
  }
  for (; i < ABCC_LUT_COUNT; i++) {
    entry = &(abcc->abcc_info.lut.final_lut_list.entry[count-1]);
    pix_index = PIX_IND(entry->pixel_index, width);
    packed_lut->lut[i] = ((uint32_t)pix_index << 9);
    packed_lut->lut[i] |= ((uint32_t)(entry->skip_index[1]) << 6);
    packed_lut->lut[i] |= ((uint32_t)(entry->skip_index[0]) << 3);
    packed_lut->lut[i] |= (uint32_t)(entry->kernel_index);

    ISP_DBG("final_lut_list[%d] pix_index %d skip_index %d %d kern_ind %d final %llx",
      i, pix_index, entry->skip_index[0], entry->skip_index[1],
      entry->kernel_index, packed_lut->lut[i]);
  }
  return TRUE;
}

/** abcc44_run_abcc_algo:
 *
 *  @abcc: abcc sub block info
 *  Runs the abcc algo and prepares the LUT tab
 *
 **/
static boolean abcc44_run_abcc_algo(abcc_t *abcc)
{
  int ret = TRUE;
  if (!abcc) {
    ISP_ERR("failed: abcc %p", abcc);
    return FALSE;
  }

  if (abcc->abcc_info.defect_pixels.count == 0) {
    ISP_ERR("No defective pixels set by sensor event");
    return FALSE;
  }
  abcc->abcc_info.forced_correction = ABCC_FORCED_CORRECTION_ENABLE;

  ret = abcc44_remove_duplicate_defective_pixels(
    &abcc->abcc_info.defect_pixels);
  if (ret == FALSE) {
    ISP_ERR("failed abcc44_remove_duplicate_defective_pixels");
    return FALSE;
  }
  ISP_HIGH("%s: algo start", __func__);
  /* Based on the number of defect pixels received from sensor allocate memory
     for the algo params*/
  ret = abcc_algo_init(&abcc->abcc_info);
  if (ret == FALSE) {
    ISP_ERR("failed abcc_algo_init");
    return FALSE;
  }

  ret = abcc_algo_process(&abcc->abcc_info, ABCC_LUT_COUNT);
  if (ret == FALSE) {
    ISP_ERR("abcc_algo_process failed");
    abcc_algo_deinit(&abcc->abcc_info);
    return FALSE;
  }

  ret = abcc44_abcc_prepare_lut_tables(abcc);
  if (ret == FALSE) {
    ISP_ERR("failed: abcc44_abcc_prepare_lut_tables");
  }
  /* Algo params are no more needed as this is run only once per session so
     so free the memory*/
  abcc_algo_deinit(&abcc->abcc_info);

  ISP_HIGH("%s: algo end", __func__);

  return ret;
}/* abcc44_abcc_config */

/** abcc44_write_lut_to_dmi:
 *
 *  @isp_sub_module: isp base module
 *  @mod: module data
 *
 *  writes abcc LUT table to DMI
 *
 **/
boolean abcc44_write_lut_to_dmi(isp_sub_module_t *isp_sub_module, abcc_t *mod)
{
  boolean   ret = TRUE;
  uint32_t  lut_size = sizeof(uint64_t) * ABCC_LUT_COUNT;

 if (ABCC_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      ABCC_CGC_OVERRIDE_REGISTER, ABCC_CGC_OVERRIDE_BIT, TRUE);
    if (ret == FALSE) {
      ISP_ERR("failed: enable ABCC cgc");
    }
  }
  /* ABCC is only configured once during streamon. Hence always use the
     same bank0 */
  ret = isp_sub_module_util_write_dmi(
    (void*)&mod->packed_lut.lut[0], lut_size,
    DEMOSAIC_LUT_RAM_BANK0,
    VFE_WRITE_DMI_64BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: abcc44 isp_sub_module_util_write_dmi");
  }

 if (ABCC_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      ABCC_CGC_OVERRIDE_REGISTER, ABCC_CGC_OVERRIDE_BIT, FALSE);
    if (ret == FALSE) {
      ISP_ERR("failed: disable ABCC cgc");
    }
  }

  return ret;
}

/** abcc44_set_defective_pixel_for_abcc:
 *
 * @mod: mct module
 * @isp_sub_module: isp_sub_module
 * @event: mct event
 *
 * Handle Set Defective pixel event from sensor
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean abcc44_set_defective_pixel_for_abcc(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  abcc44_t              *abcc2 = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  abcc2 = (abcc44_t *)isp_sub_module->private_data;
  if (!abcc2) {
    ISP_ERR("failed: abcc2 %p", abcc2);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  abcc2->abcc_subblock.abcc_info.defect_pixels =
    *(defective_pix_array_t *)event->u.module_event.module_event_data;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** abcc44_streamon:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function adds ref count for stream on flag and sets
 *  trigger_update_pending flag to TRUE for first STREAM ON
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean abcc44_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                ret = TRUE;
  abcc44_t               *abcc2;
  abcc_t                *abcc;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  abcc2 = (abcc44_t *)isp_sub_module->private_data;
  if (!abcc2) {
    ISP_ERR("failed: abcc2 %p", abcc2);
    return FALSE;
  }
  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  abcc = &abcc2->abcc_subblock;
  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  if (isp_sub_module->stream_on_count == 1)
  {
      abcc->one_time_config_done = FALSE;
  }

  isp_sub_module->trigger_update_pending = TRUE;
  isp_sub_module->config_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
} /* abcc44_streamon */

/** abcc44_trigger_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Perform trigger update if trigger_update_pending flag is
 *  TRUE and append hw update list in global list
 *
 *  Return TRUE on success and FALSE on failure
 **/

boolean abcc44_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                       ret = TRUE;
  abcc44_t                      *abcc2 = NULL;
  isp_private_event_t          *private_event = NULL;
  isp_sub_module_output_t      *output = NULL;
  abcc_t                       *abcc;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (!isp_sub_module->submod_enable ||
      !isp_sub_module->submod_trigger_enable) {
    ISP_DBG("skip trigger update enable %d, trig_enable %d",
      isp_sub_module->submod_enable, isp_sub_module->submod_trigger_enable);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  abcc2 = (abcc44_t *)isp_sub_module->private_data;

  if (!abcc2) {
    ISP_ERR("failed: abcc2 %p", abcc2);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  abcc = &abcc2->abcc_subblock;

  if (abcc2->abcc_subblock.abcc_enable &&
      abcc2->abcc_subblock.one_time_config_done == FALSE) {
       if(abcc2->abcc_subblock.abcc_info.defect_pixels.count > 0) {

        ret = abcc44_run_abcc_algo(abcc);
        if (ret != true) {
          ISP_ERR("failed: run_abcc_algo\n");
          PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
          return FALSE;
        }

        abcc44_write_lut_to_dmi(isp_sub_module, &abcc2->abcc_subblock);

        private_event = (isp_private_event_t *)(event->u.module_event.module_event_data);
        if (!private_event) {
          ISP_ERR("failed: private_event %p", private_event);
          PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
          return FALSE;
        }

        output = (isp_sub_module_output_t *)private_event->data;
        if (!output) {
          ISP_ERR("failed: output %p", output);
          PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
          return FALSE;
        }

        ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
        if (ret == FALSE) {
          ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
          PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
          return FALSE;
        }

        abcc2->abcc_subblock.one_time_config_done = TRUE;
      }
    }
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
}

/** abcc44_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the abcc module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean abcc44_init(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  abcc44_t *abcc = NULL;

  if (!isp_sub_module || !module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  abcc = (abcc44_t *)CAM_MALLOC(sizeof(abcc44_t));
  if (!abcc) {
    ISP_ERR("failed: abcc %p", abcc);
    return FALSE;
  }

  memset(abcc, 0, sizeof(*abcc));

  isp_sub_module->private_data = (void *)abcc;
  /* Disable abcc by default*/
  abcc->abcc_subblock.abcc_enable = ABCC_ENABLE;
  abcc->abcc_subblock.lut_bank_select = 0;

  return TRUE;
}/* abcc44_init */

/** abcc44_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void abcc44_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module || !module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* abcc44_destroy */
