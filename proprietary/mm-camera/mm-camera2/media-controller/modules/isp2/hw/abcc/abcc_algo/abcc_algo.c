/* abcc_algo.c
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* std headers */
#include <unistd.h>

/* isp headers */
#include "abcc_algo.h"
#include "isp_log.h"

#define NUM_NEIGBOR_ROWS 9
#define NUM_NEIGBOR_COLS 2

#define SWAP(a,b) ({ \
  typeof(a) _t = a; \
  a = b; \
  b = _t; \
})

/*CorrMatrix: kernel index and the pixel to be indexed,
  1 = center, 0 = neighbor*/
typedef struct {
  int kern_index;
  int centre;
}correct_matrix_t;

/*neighbor: contain all 8 neighbor coor,
  last one represent single pixel corr*/
const pix_t neighbors[9] = {
  {-2, -2}, {-2, 0}, {-2, 2},
  { 0, -2}, { 0, 2}, {2, -2},
  { 2,  0}, { 2, 2}, { 0, 0}
};

static correct_matrix_t const corr_matrix[] = {
  {0, 1}, {1, 1}, {2, 1},
  {3, 1}, {3, 0}, {2, 0},
  {1, 0}, {0, 0}, {4, 1}
};

/** abcc_algo_deinit:
 *
 *  @info: abcc algo info
 *
 *  de initialiaze the algo data
 *
 *  Returns nothing
 **/
void abcc_algo_deinit(abcc_algo_t *info)
{
  if (info->corr_table.val) {
    free(info->corr_table.val);
    info->corr_table.val = NULL;
    info->corr_table.count = 0;
  }
  if (info->lut.lut_list.entry) {
    free(info->lut.lut_list.entry);
    info->lut.lut_list.entry = NULL;
    info->lut.lut_list.count = 0;
  }
  if (info->lut.processed_list.entry) {
    free(info->lut.processed_list.entry);
    info->lut.processed_list.entry = NULL;
    info->lut.processed_list.count = 0;
  }
  if (info->lut.single_pix_list.entry) {
    free(info->lut.single_pix_list.entry);
    info->lut.single_pix_list.entry = NULL;
    info->lut.single_pix_list.count = 0;
  }
  if (info->lut.final_lut_list.entry) {
    free(info->lut.final_lut_list.entry);
    info->lut.final_lut_list.entry = NULL;
    info->lut.final_lut_list.count = 0;
  }
}/* end abcc_algo_deinit */

/** abcc_algo_init:
 *
 *  @info: abcc algo info
 *
 *  initialiaze the algo data
 *
 *  Returns nothing
 **/
boolean abcc_algo_init(abcc_algo_t *info)
{
  int i = 0, j = 0;
  int size = 0;

  ISP_DBG("%s: num_pixels %d", __func__, info->defect_pixels.count);
  size = sizeof(abcc_corr_tab_entry_t) * info->defect_pixels.count;
  info->corr_table.val = (abcc_corr_tab_entry_t *)CAM_MALLOC(size);
  if (!info->corr_table.val) {
    ISP_ERR("failed memory allocation for info->corr_table.val");
    goto ERROR1;
  }

  info->corr_table.count = info->defect_pixels.count;
  memset(info->corr_table.val, 0x0, size);

  size = sizeof(abcc_entry_t) * info->defect_pixels.count;
  info->lut.lut_list.entry = (abcc_entry_t *)CAM_MALLOC(size);
  if (!info->lut.lut_list.entry) {
    ISP_ERR("failed memory allocation for info->lut.lut_list.entry");
    goto ERROR2;
  }
  info->lut.lut_list.count = info->defect_pixels.count;
  info->lut.lut_list.actual_count = 0;
  memset(info->lut.lut_list.entry, 0x0, size);

  size = sizeof(abcc_proc_entry_t) * info->defect_pixels.count;
  info->lut.processed_list.entry = (abcc_proc_entry_t *)CAM_MALLOC(size);
  if (!info->lut.processed_list.entry) {
    ISP_ERR("failed memory allocation for info->lut.processed_list.entry");
    goto ERROR3;
  }
  info->lut.processed_list.count = info->defect_pixels.count;
  info->lut.processed_list.actual_count = 0;
  memset(info->lut.processed_list.entry, 0x0, size);

  size = sizeof(abcc_single_pix_entry_t) * info->defect_pixels.count;
  info->lut.single_pix_list.entry = (abcc_single_pix_entry_t *)CAM_MALLOC(size);
  if (!info->lut.single_pix_list.entry) {
    ISP_ERR("failed memory allocation for nfo->lut.single_pix_list.entry");
    goto ERROR4;
  }
  info->lut.single_pix_list.count = info->defect_pixels.count;
  info->lut.single_pix_list.actual_count = 0;
  memset(info->lut.single_pix_list.entry, 0x0, size);

  memset(&info->unprocessed, 0x0, sizeof(defective_pix_array_t));
  return TRUE;

ERROR4:
  free(info->lut.processed_list.entry);
ERROR3:
  free(info->lut.lut_list.entry);
ERROR2:
  free(info->corr_table.val);
ERROR1:
  return FALSE;

} /* end abcc_algo_init */

/** bpc47_cmd_config:
 *
 *  @pix: pix array
 *
 *  check if (x, y) is present in pix array
 *
 *  Returns TRUE if success/ FALSE is failed
 **/
static boolean abcc_check_if_present(pix_t *pix, int x, int y, int *index, int count)
{
  int i;
  for (i = 0; i < count; i++) {
    if ((x == pix[i].x) && (y == pix[i].y)) {
      *index = i;
      return TRUE;
    }
  }
  return FALSE;
} /* end abcc_check_if_present */

/** abcc_list_isolated_or_clustered:
 *
 *  @proc_list: proc list
 *  @isolated_list: isolated list
 *
 *  check if isolated
 *
 *  Returns TRUE if success/ FALSE is failed
 **/
static int abcc_list_isolated_or_clustered(abcc_proc_list_t *proc_list, int *isolated_list,
  int is_isolate)
{
  int i;
  int curr_index = 0;
  abcc_proc_entry_t *entry = proc_list->entry;
  for (i = 0; i < proc_list->actual_count; i++) {
    if (entry[i].single_defective_pix == is_isolate) {
      isolated_list[curr_index++] = i;
    }
  }
  return curr_index;
} /* end abcc_list_isolated_or_clustered */

/** abcc_sort_LUT:
 *
 *  @abcc_list_t: abcc lut list
 *
 *  Sort lut list
 *
 *  Returns nothing
 **/
static void abcc_sort_LUT(abcc_list_t *lut_list, int w, int count)
{
  int i, j;
  abcc_entry_t *entry = lut_list->entry;
  for(i = 0; i < count; i++) {
    for(j = i+1; j < count; j++) {
      if (PIX_IND(entry[j].pixel_index, w) < PIX_IND(entry[i].pixel_index, w))
        SWAP(entry[i], entry[j]);
    }
  }

} /* end abcc_sort_LUT */

/** abcc_sort_pix_array:
 *
 *  @pix: pixels to be sorted
 *  @count: number of pixels
 *
 *  Sort Pixel array
 *
 *  Returns nothing
 **/
static void abcc_sort_pix_array(pix_t *pix, int w, int count)
{
  int i, j;
  pix_t tmp;

  for(i = 0; i <  count; i++) {
    for(j = i+1; j < count ; j++) {
      if (PIX_IND(pix[j], w) > PIX_IND(pix[i], w))
        SWAP(pix[i], pix[j]);
    }
  }

  for(i = 0, j=count-1; i < (int)count/2; i++, j--) {
    tmp=pix[j];
    pix[j]=pix[i];
    pix[i]=tmp;
  }

} /* end: abcc_sort_pix_array */

/** abcc_copy_LUT:
 *
 *  @new_list: new abcc lut list
 *  @old_list: old abcc lut list
 *
 *  Copy from old lut list to new lut list *
 *  Returns TRUE if success/ FALSE is failed
 **/
static int abcc_copy_LUT(abcc_list_t *new_list, abcc_list_t *old_list,
  int *copy_index, int num_of_copy)
{
  int i;
  int curr_index;
  int start = new_list->actual_count;

  new_list->count = old_list->count;

  for (i = 0; i < num_of_copy; i++) {
    new_list->entry[start + i] = old_list->entry[copy_index[i]];
    new_list->actual_count++;
  }

  return new_list->actual_count;
}/* end abcc_copy_LUT*/

/** abcc_find_defectives_new:
 *
 *  @info: abcc algo data
 *
 *  to output the number of pixels to be skipped and the
 *  corresponding pixel indices.
 *
 *  Returns TRUE if success/ FALSE is failed
 **/
static boolean abcc_find_defectives_new(abcc_algo_t *info, pix_t center,
  int kernel_idx, int examinedRow, int *skip_index, int *noSkips)
{
  int neighbor_index, neighbor_defective;
  int x1, y1, m, neighbor_corr_updated;
  int i, curr_index = 0;
  int num_kernel_neighbor = 0;
  int width = info->sensor_width;
  int height = info->sensor_height;

  ISP_DBG("%s: center %d %d kern %d examinedRow %d", __func__,
    center.x, center.y, kernel_idx, examinedRow);
  pix_t *kernel_neighbors;
  pix_t kernel0_Neighbors[] =
    {{-4, -2}, {-2, -4}, {-2, 0}, {0, -2}, {0, 2}, {2, 0}};
  pix_t kernel1_Neighbors[] =
    {{-4, 0}, {-2, -2}, {-2, 2}, {0, -2}, {0, 2}, {2, 0}};
  pix_t kernel2_Neighbors[] =
    {{-4, 2}, {-2, 0}, {-2, 4}, {0, -2}, {0, 2}, {2, 0}};
  pix_t kernel3_Neighbors[] =
    {{-2, -2}, {-2, 0}, {0, -4}, {0, 2}, {2, -2}, {2, 0}};
  pix_t kernel4_Neighbors[] =
    {{-2, 0}, {0, -2}, {0, 2}, {2, 0}};

  /*not looks this part is not used*/
  /*int defectNeighbors[6][2] = {{-2, -2}, {-2, 0}, {-2, 2}, {0, -2}, {0, 0},
    {0, 2}} */;

  switch(kernel_idx) {
    case 0:
      kernel_neighbors = kernel0_Neighbors;
      num_kernel_neighbor = 6;
      break;
    case 1:
      kernel_neighbors = kernel1_Neighbors;
      num_kernel_neighbor = 6;
      break;
    case 2:
      kernel_neighbors = kernel2_Neighbors;
      num_kernel_neighbor = 6;
      break;
    case 3:
      kernel_neighbors = kernel3_Neighbors;
      num_kernel_neighbor = 6;
      break;
    case 4:
      kernel_neighbors = kernel4_Neighbors;
      num_kernel_neighbor = 4;
      break;
    default:
      ISP_ERR("%s: kernel index error", __func__);
      return FALSE;
  }

  /*in the original matlab implement, but looks not needed*
    x1= center[0] + defectNeighbors[kernel_idx + 1][0];
    y1= center[1] + defectNeighbors[kernel_idx + 1][1];    */

  curr_index = 0;
  *noSkips = 0;
  skip_index[0] = 0;
  skip_index[1] = 0;

  for(i = 0; i < num_kernel_neighbor; i++) {
    neighbor_corr_updated = 0;

    x1 = center.x + kernel_neighbors[i].x;
    y1 = center.y + kernel_neighbors[i].y;
    ISP_DBG("%s: neighbor %d %d", __func__, x1, y1);

    /*1.skip all the non-corrected and non-updated pixel
        get the skip indexs and number of skip.
      2.skip the out of boundry pixels as well*/
    if(x1 < 0 || x1 > (height -1) || y1 < 0 || y1 > (width -1)) {
      if(*noSkips < 2)
        skip_index[curr_index++] = i+1;
        *noSkips = *noSkips + 1;
    } else {
      neighbor_defective = abcc_check_if_present(info->defect_pixels.pix, x1, y1,
        &neighbor_index, info->defect_pixels.count);
      ISP_DBG("%s: neighbor_defective %d ind %d", __func__,
        neighbor_defective, neighbor_index);
      if (neighbor_defective == 1) {
        neighbor_corr_updated = info->corr_table.val[neighbor_index].corrected
          && info->corr_table.val[neighbor_index].present_update;

        if ((x1 >= examinedRow) ||
          ((x1 == (examinedRow - 2)) && !neighbor_corr_updated)) {
          if(*noSkips < 2)
            skip_index[curr_index++] = i+1;
            *noSkips = *noSkips + 1;
        }
      }
    }
  }

  if(*noSkips == 0) {
    skip_index[0] = 0;
    skip_index[1] = 0;
  }
  else if(*noSkips == 1) {
    skip_index[1] = 0;
  }
  ISP_DBG("%s: noSkips %d", __func__, *noSkips);

  return TRUE;
}

/** abcc_process_defective_pixels_new:
 *
 *  @info: abcc algo data
 *
 *  process defective pixels and prepare LUT list
 *
 *  Returns TRUE if success/ FALSE is failed
 **/
static boolean abcc_process_defective_pixels_new(abcc_algo_t *info)
{
  int ret = 0;
  int i, j, x, y, x1, y1, m, k;
  int singleDefectivePixel, noCorrected;
  boolean neighbor_defective = FALSE, neighbor_index = 0;
  pix_t center, neighbor;
  int noSkips, skip_index[2];
  int is_single_pix_list_empty = 1;
  int is_single_corrected = 0;

  abcc_sort_pix_array(info->defect_pixels.pix, info->sensor_width,
    info->defect_pixels.count);

  for(m = 0; m < info->defect_pixels.count; m++) {
    if (info->corr_table.val[m].checked)
      continue;

    info->lut.single_pix_list.actual_count = 0;
    is_single_pix_list_empty = 1;

    singleDefectivePixel = 1;
    x = info->defect_pixels.pix[m].x;
    y = info->defect_pixels.pix[m].y;
    ISP_DBG("%s: pix %d %d", __func__, x, y);

    for(k = 0; k < NUM_NEIGBOR_ROWS; k++) {
      noCorrected = 1;
      neighbor_defective = FALSE;

      x1 = x + neighbors[k].x;
      y1 = y + neighbors[k].y;
      ISP_DBG("%s: neighbor pix %d %d", __func__, x1, y1);

      /*check if the neighbor is in the DefectList*/
      neighbor_defective = abcc_check_if_present(info->defect_pixels.pix, x1, y1,
        &neighbor_index, info->defect_pixels.count);

      /*check if out of boundry*/
      if ((x1 < 0) || (x1 > (info->sensor_height - 1)) ||
        (y1 < 0) || (y1 > (info->sensor_width - 1))) {
        if (!corr_matrix[k].centre)
          continue;

      } else {
        if (x1 != x || y1 != y) {
          /*check if the neighbor is the pixel itself */
          if (neighbor_defective) {
            singleDefectivePixel = 0;
            /* corrected as cluster, not as single */
            if (!info->corr_table.val[neighbor_index].corrected)
              noCorrected = 2;
          } else {
            if(info->forced_correction == 0)
              continue;
          }
        }
      }

      /*decide centre pixel */
      if (corr_matrix[k].centre) {
        center.x = x;
        center.y = y;
        neighbor.x = x1;
        neighbor.y = y1;
      } else {
        center.x = x1;
        center.y = y1;
        neighbor.x = x;
        neighbor.y = y;
      }

      /*find number of skips, and skip indexes*/
      ret = abcc_find_defectives_new(info, center, corr_matrix[k].kern_index,
        x, skip_index, &noSkips);
      if (ret == FALSE) {
        ISP_DBG("abcc_find_defectives_new failed");
        return FALSE;
      }

      ISP_DBG("%s: noCorrected %d noSkips %d", __func__, noCorrected, noSkips);
      /*correct 2 pixels*/
      if ((noCorrected == 2) && (noSkips <= 2)) {
        int ind = 0;
        ind = info->lut.lut_list.actual_count;
        info->lut.lut_list.entry[ind].pixel_index.x = center.x;
        info->lut.lut_list.entry[ind].pixel_index.y = center.y;
        info->lut.lut_list.entry[ind].skip_index[0] = skip_index[0];
        info->lut.lut_list.entry[ind].skip_index[1] = skip_index[1];
        info->lut.lut_list.entry[ind].kernel_index = corr_matrix[k].kern_index;
        info->lut.lut_list.actual_count++;

        ind = info->lut.processed_list.actual_count;
        info->lut.processed_list.entry[ind].center = center;
        info->lut.processed_list.entry[ind].neighbor = neighbor;
        info->lut.processed_list.entry[ind].single_defective_pix =
          singleDefectivePixel;
        info->lut.processed_list.actual_count++;

        if(corr_matrix[k].centre) {
          info->corr_table.val[m].checked = TRUE;
          info->corr_table.val[m].corrected = TRUE;
          info->corr_table.val[m].present_update = TRUE;
          info->corr_table.val[neighbor_index].checked = TRUE;
          info->corr_table.val[neighbor_index].corrected = TRUE;
          info->corr_table.val[neighbor_index].present_update = FALSE;
        } else {
          info->corr_table.val[m].checked = TRUE;
          info->corr_table.val[m].corrected = TRUE;
          info->corr_table.val[m].present_update = FALSE;
          info->corr_table.val[neighbor_index].checked = TRUE;
          info->corr_table.val[neighbor_index].corrected = TRUE;
          info->corr_table.val[neighbor_index].present_update = TRUE;
        }
        break;
      }

      /*will only correct one pixel*/
      if ((noCorrected == 1) && (noSkips <= 2)) {
        int ind = 0;
        ind = info->lut.single_pix_list.actual_count;
        info->lut.single_pix_list.entry[ind].center = center;
        info->lut.single_pix_list.entry[ind].neighbor = neighbor;
        info->lut.single_pix_list.entry[ind].kernel_index =
          corr_matrix[k].kern_index;
        info->lut.single_pix_list.entry[ind].skip_index[0] = skip_index[0];
        info->lut.single_pix_list.entry[ind].skip_index[1] = skip_index[1];
        info->lut.single_pix_list.entry[ind].single_defective_pix =
          singleDefectivePixel;
        info->lut.single_pix_list.actual_count++;
        is_single_pix_list_empty = 0;
        ISP_DBG("%s: Line %d", __func__, __LINE__);
      }
    }
    is_single_corrected = 0;

    /*none of the double-pixel kernel founded*/
    if(!info->corr_table.val[m].corrected) {
      if(!is_single_pix_list_empty) {
        int lut_actual_count = info->lut.lut_list.actual_count;
        int proc_actual_count = info->lut.processed_list.actual_count;
        int singleList_actual_count = info->lut.single_pix_list.actual_count;
        ISP_DBG("%s: single_pix %d %d %d", __func__,
          lut_actual_count, proc_actual_count, singleList_actual_count);
        int ind = 0;

        for(ind = 0; ind < singleList_actual_count; ind++) {
          if(info->lut.single_pix_list.entry[ind].kernel_index == 4) {
            info->lut.lut_list.entry[lut_actual_count].pixel_index =
              info->lut.single_pix_list.entry[ind].center;
            info->lut.lut_list.entry[lut_actual_count].kernel_index =
              info->lut.single_pix_list.entry[ind].kernel_index;
            info->lut.lut_list.entry[lut_actual_count].skip_index[0] =
              info->lut.single_pix_list.entry[ind].skip_index[0];
            info->lut.lut_list.entry[lut_actual_count].skip_index[1] =
              info->lut.single_pix_list.entry[ind].skip_index[1];
            info->lut.lut_list.actual_count++;

            info->lut.processed_list.entry[proc_actual_count].center =
              center;
            info->lut.processed_list.entry[proc_actual_count].neighbor =
              center;
            info->lut.processed_list.entry[proc_actual_count].
              single_defective_pix = singleDefectivePixel;
            info->lut.processed_list.actual_count++;

            is_single_corrected = 1;
            ISP_DBG("%s: single corrected", __func__);
          }
        }

        /*to enter this part:
          1. not corrected yet
          2. noCorrect = 1, defective neighbor may already be corrected
          3. not run to the case neighbor 9(itself, kernel 4),
             or the noSkips > 2 when run the neighbor 9
          4. have some defective neighbor, but some of them are already corrected.*/
        if(!is_single_corrected) {
          info->lut.lut_list.entry[lut_actual_count].pixel_index =
            info->lut.single_pix_list.entry[0].center;
          info->lut.lut_list.entry[lut_actual_count].kernel_index =
            info->lut.single_pix_list.entry[0].kernel_index;
          info->lut.lut_list.entry[lut_actual_count].skip_index[0] =
            info->lut.single_pix_list.entry[0].skip_index[0];
          info->lut.lut_list.entry[lut_actual_count].skip_index[1] =
            info->lut.single_pix_list.entry[0].skip_index[1];
          info->lut.lut_list.actual_count++;

          info->lut.processed_list.entry[proc_actual_count].center =
            center;
          info->lut.processed_list.entry[proc_actual_count].neighbor =
            info->lut.single_pix_list.entry[0].neighbor;
          info->lut.processed_list.entry[proc_actual_count].
            single_defective_pix = singleDefectivePixel;
          info->lut.processed_list.actual_count++;
        }

        info->corr_table.val[m].checked = TRUE;
        info->corr_table.val[m].corrected = TRUE;
        info->corr_table.val[m].present_update = TRUE;
      }
      ISP_DBG("%s: %d", __func__, __LINE__);
    }
    info->corr_table.val[m].checked = TRUE;
    ISP_DBG("%s: %d loop %d done", __func__, __LINE__, m);
  }

  ISP_DBG("%s: algo completed", __func__);
  /*SORT the LUT*/
  abcc_sort_LUT(&(info->lut.lut_list), info->sensor_width,
    info->lut.lut_list.actual_count);
  return TRUE;
}

/** abcc_find_unprocessed:
 *
 *  @info: abcc algo data
 *
 *  to find the unprocessed pixel:
 *  compare the indices of all corrected pixels with the list of
 *  defective pixels.
 *
 *  Returns TRUE if success/ FALSE is failed
 **/
static boolean abcc_find_unprocessed(abcc_algo_t *info)
{
  int ret = TRUE;
  int i, j;
  int *ind;
  pix_t *allEntries;
  int num_of_allEntries = 0;
  int num_of_ind = 0;
  int is_processed = 0;
  int is_unique = 1;
  ind = (int *)CAM_MALLOC(info->lut.final_lut_list.actual_count *
   sizeof(int));
  if (!ind) {
    ISP_ERR("memory allocation failed for ind");
    return FALSE;
  }
  allEntries = (pix_t *)CAM_MALLOC(info->lut.final_lut_list.actual_count *
   sizeof(pix_t) * 2);
  if (!allEntries) {
    ISP_ERR("memory allocation failed for allEntries");
    free(ind);
    return FALSE;
  }

  /*to find pixel in processed_list shows up in new_LUT_list
    1. the processed_list is the pixels "can" be corrected by old LUT
    2. the new_LUT are the pixels which are actually going to be corrected*/
  for (i = 0; i < info->lut.processed_list.actual_count; i++) {
    for (j = 0; j < info->lut.final_lut_list.actual_count; j++) {
      if(PIX_CMP(info->lut.processed_list.entry[i].center,
        info->lut.final_lut_list.entry[j].pixel_index)) {
        ind[num_of_ind++] = i;
      }
    }
  }
  /*all entries contain all the pixels which are gonna be corrected
    including center and also neighbor in the LUT table
    need to put the neighbor into the allEntries as well*/
  for (i = 0; i < num_of_ind; i++) {
    allEntries[num_of_allEntries++] =
      info->lut.processed_list.entry[ind[i]].center;
  }

  for (i = 0; i < num_of_ind; i++) {
    // check if the corrected neighbor is in the all entry list
    is_unique = 1;

    for(j = 0; j < num_of_ind; j++) {
      if (PIX_CMP(info->lut.processed_list.entry[ind[i]].neighbor,
        allEntries[j]))
        is_unique = 0;
    }

    if (is_unique) {
      allEntries[num_of_allEntries++] =
        info->lut.processed_list.entry[ind[i]].neighbor;
    }
  }

  /*find all the unprocessed pixels*/
  for (i = 0; i < info->defect_pixels.count; i++) {
    is_processed = 0;

    for (j = 0; j < num_of_allEntries; j++) {
      if (PIX_CMP(info->defect_pixels.pix[i], allEntries[j]))
        is_processed = 1;
    }
    if(!is_processed) {
      info->unprocessed.pix[info->unprocessed.count++] =
        info->defect_pixels.pix[i];
    }
  }
  ISP_DBG("%s: unprocessed count %d", __func__, info->unprocessed.count);

  abcc_sort_pix_array(info->unprocessed.pix, info->sensor_width,
    info->unprocessed.count);

  if (ind)
    free(ind);
  if (allEntries)
    free(allEntries);
  return ret;
}

/** isp_abcc_algo_process:
 *
 *  @info: abcc algo data
 *  @maxNoEntries: Max number of entries
 *
 *  to generate a new LUT according to the maxNoEntries throw
 *  away extra LUT entries.
 *
 *  Returns TRUE if success/ FALSE is failed
 **/
boolean abcc_algo_process(abcc_algo_t *info, int maxNoEntries)
{
  int ret = TRUE;
  int extra_LUT_entries = 0;
  int size =0;
  int i =0;
  int list_count;
  int *isolated_list = NULL;
  int *clustered_list = NULL;
  int *full_list = NULL;
  int num_isolated, num_clustered;

  ISP_DBG("enter abcc_algo_process");

  ret = abcc_process_defective_pixels_new(info);
  if (ret == FALSE) {
    ISP_DBG("%s: isp_abcc_process_defective_pixels_new failed %d",
      __func__, ret);
    return FALSE;
  }

  isolated_list = (int *)CAM_MALLOC(info->lut.processed_list.actual_count *
   sizeof(int));
  if (!isolated_list) {
    ISP_ERR("failed memory allocation for isolated_list");
    goto ERROR1;
  }
  clustered_list = (int *)CAM_MALLOC(info->lut.processed_list.actual_count *
   sizeof(int));
  if (!clustered_list) {
    ISP_ERR("failed memory allocation for clustered_list");
    goto ERROR2;
  }
  full_list = (int *)CAM_MALLOC(maxNoEntries * sizeof(int));
  if (!full_list) {
    ISP_ERR("failed memory allocation for full_list");
    goto ERROR3;
  }
  /*memory allocation for new_LUT_list*/
  size = sizeof(abcc_entry_t) * info->defect_pixels.count;
  info->lut.final_lut_list.entry = (abcc_entry_t *)CAM_MALLOC(size);
  if (!info->lut.final_lut_list.entry) {
    ISP_ERR("failed memory allocation for info->corr_table.val");
    goto ERROR4;
  }
  info->lut.final_lut_list.count = info->defect_pixels.count;
  info->lut.final_lut_list.actual_count = 0;
  memset(info->lut.final_lut_list.entry, 0x0, size);

  extra_LUT_entries = info->lut.lut_list.actual_count - maxNoEntries;
  ISP_DBG("%s: extra_LUT_entries %d", __func__, extra_LUT_entries);

  if (extra_LUT_entries <= 0) {
    info->lut.final_lut_list.actual_count = info->lut.lut_list.actual_count;
    memcpy(info->lut.final_lut_list.entry, info->lut.lut_list.entry,
      info->lut.lut_list.actual_count * sizeof(abcc_entry_t));
  } else {
    ISP_DBG("%s: the total number of entries required is %d \n",
       __func__,info->lut.lut_list.actual_count);

    num_isolated = abcc_list_isolated_or_clustered(&info->lut.processed_list,
      isolated_list, TRUE);
    num_clustered = abcc_list_isolated_or_clustered(&info->lut.processed_list,
      clustered_list, FALSE);

    ISP_DBG("%s: num_isolated %d num_clustered %d", __func__,
      num_isolated, num_clustered);

    for(i = 0; i < maxNoEntries; i++)
      full_list[i] = i;

    /*if the LUT larger than */
    if (num_isolated == 0) {
      list_count = abcc_copy_LUT(&info->lut.final_lut_list, &info->lut.lut_list,
        full_list, maxNoEntries);
    } else {
      if (num_isolated > extra_LUT_entries) {
        list_count =
          abcc_copy_LUT(&info->lut.final_lut_list, &info->lut.lut_list,
          clustered_list, num_clustered);
        list_count =
          abcc_copy_LUT(&info->lut.final_lut_list, &info->lut.lut_list,
          isolated_list, num_isolated - extra_LUT_entries);

      } else {
        list_count =
          abcc_copy_LUT(&info->lut.final_lut_list, &info->lut.lut_list,
            clustered_list, maxNoEntries);
      }
    }
  }

  ret = abcc_find_unprocessed(info);
  if (ret == FALSE) {
    ISP_ERR("isp_abcc_find_unprocessed failed");
    goto ERROR4;
  }

  abcc_sort_LUT(&(info->lut.final_lut_list), info->sensor_width,
    info->lut.final_lut_list.actual_count);

  if (isolated_list)
    free(isolated_list);
  if (clustered_list)
    free(clustered_list);
  if (full_list)
    free(full_list);
  return ret;
ERROR4:
  free(full_list);
ERROR3:
  free(clustered_list);
ERROR2:
  free(isolated_list);
ERROR1:
  return FALSE;

}/*end abcc_algo_process*/

