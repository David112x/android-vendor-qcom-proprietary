/***************************************************************************
* Copyright (c) 2013-2014, 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
***************************************************************************/
#include <cutils/properties.h>
#include <linux/media.h>
#include "mct_module.h"
#include "module_faceproc.h"

#define FACEPROC_FACE_SAME   0
#define FACEPROC_FACE_BEFORE 1
#define FACEPROC_FACE_AFTER  2

/* Macro used to apply temporal filter */
#define FD_STAB_APPLY_TEMP_FILTER(first, weight_first, second, weight_second) \
  ((first) = (((second) * (weight_second)) + ((first) * (weight_first))) / \
  ((weight_first) + (weight_second)))

/* Macro used to calculate threshold from 0.1 percentage to value */
#define FD_STAB_CALC_THRESHOLD(value, per) \
  ((((uint32_t)value) * (per)) / 1000)

/* Macro used to calculate distance between two pixels */
#define FD_STAB_CALC_DISTANCE(a, b) \
  (sqrt((uint32_t)(pow((abs((int32_t)(a).x - (int32_t)(b).x)), 2) \
  + pow((abs((int32_t)(a).y - (int32_t)(b).y)), 2))))

/**
* Function: module_faceproc_stab_is_face_detected
*
* Description: Check detected face confidence threshold limit.
*
* Arguments:
*   @roi - faceproc_info_t Roi
*   @p_chromx - Pointer to chromatix
*
* Return values:
*  TRUE if face is detected.
*
* Notes: none
**/
static boolean is_face_detected(faceproc_info_t *roi, fd_chromatix_t *p_chromx)
{
  if (p_chromx->stab_conf.enable) {
    if ((uint32_t)roi->fd_confidence < p_chromx->stab_conf.hyst.end_A) {
      return FALSE;
    }
  }

  return TRUE;
}

/**
* Function: module_faceproc_stab_is_face_lost
*
* Description: Check lost face confidence threshold limit.
*
* Arguments:
*   @roi - faceproc_info_t Roi
*   @p_chromx - Pointer to chromatix
*
* Return values:
*  TRUE if face is lost.
*
* Notes: none
**/
static boolean is_face_lost(faceproc_info_t *roi, fd_chromatix_t *p_chromx)
{
  if (p_chromx->stab_conf.enable) {
    if ((uint32_t)roi->fd_confidence < p_chromx->stab_conf.hyst.start_A) {
      return TRUE;
    }
  }

  return FALSE;
}

  /**
* Function: module_faceproc_stab_check_face
*
* Description:
*   This function will check if the face is the same or new face is detected
*
* Arguments:
*   @roi - Face detection result
*   @history - Face detection history
*
* Return values:
*   FACEPROC_FACE_SAME - Faces are the same
*   FACEPROC_FACE_BEFORE - Face is before with coordinates then reference face
*   FACEPROC_FACE_AFTER - Face is after with coordinates then reference face
* Notes: none
**/
static int module_faceproc_stab_check_face(faceproc_info_t *roi,
  faceproc_faces_history_t *history)
 {
  int result;
  uint32_t treshold_x, treshold_y;
  uint32_t center1_x, center1_y, center2_x, center2_y;

  treshold_x = (history->face_size.stable_entry.x < roi->face_boundary.dx) ?
    history->face_size.stable_entry.x : roi->face_boundary.dx;
  treshold_y = (history->face_size.stable_entry.y < roi->face_boundary.dy) ?
    history->face_size.stable_entry.y : roi->face_boundary.dy;

  center1_x = history->face_position.stable_entry.x;
  center1_y = history->face_position.stable_entry.y;

  center2_x = roi->face_boundary.x + (roi->face_boundary.dx / 2);
  center2_y = roi->face_boundary.y + (roi->face_boundary.dy / 2);

  if (((roi->unique_id == 0) || (history->id == 0)) &&
    ((uint32_t)abs((int)center1_x - (int)center2_x) < treshold_x) &&
    ((uint32_t)abs((int)center1_y - (int)center2_y) < treshold_y)) {

    result = FACEPROC_FACE_SAME;

  } else if (((roi->unique_id != 0) && (history->id != 0)) &&
    (history->id == (uint32_t)abs(roi->unique_id))) {

    result = FACEPROC_FACE_SAME;

  } else if ((center1_x + (center1_y * MAX_FD_WIDTH)) <=
             (center2_x + (center2_y * MAX_FD_WIDTH))) {

    result = FACEPROC_FACE_BEFORE;

  } else {

    result = FACEPROC_FACE_AFTER;

  }
  return result;
}

/**
* Function: module_faceproc_within_limit
*
* Description: Check if face is within the limit.
*
* Arguments:
*   @rect1 - First rectangle
*   @rect2 - Second rectangle
*   @threshold: Variation threshold
*
* Return values:
*  TRUE/FALSE
*
* Notes: none
**/
static boolean module_faceproc_within_limit(faceproc_history_entry_t *rect1,
  faceproc_history_entry_t *rect2, int threshold)
{
  if (abs((int)rect1->x - (int)rect2->x) < threshold &&
      abs((int)rect1->y - (int)rect2->y) < threshold) {
    return TRUE;
  }
  return FALSE;
}

/**
* Function: module_faceproc_stab_add_face
*
* Description: Add new face to the internal history,
*   internal history is ring buffer.
*
* Arguments:
*   @history - Fsces history
*   @roi - Result of face detection algo for this face
*   @p_fd_chromatix: faceproc chromatix pointer
*
* Return values:
*  None
* Notes: none
**/
static void module_faceproc_stab_add_face(faceproc_faces_history_t *history,
  faceproc_info_t *roi, fd_chromatix_t *p_fd_chromatix)
{
  history->id = (uint32_t)abs(roi->unique_id);

  /* Fill face positions */
  if (p_fd_chromatix->stab_pos.enable) {
    history->face_position.index = (history->face_position.index + 1) %
      p_fd_chromatix->stab_history;
    history->face_position.entry[history->face_position.index].x =
      roi->face_boundary.x + (roi->face_boundary.dx / 2);
    history->face_position.entry[history->face_position.index].y =
      roi->face_boundary.y + (roi->face_boundary.dy / 2);

    history->face_position.history_size = p_fd_chromatix->stab_history;
    if (history->face_position.faces_inside < p_fd_chromatix->stab_history) {
      history->face_position.faces_inside++;
    }

    IDBG_MED("%s:%d] Add face pos %dx%d faces insight %d", __func__, __LINE__,
      roi->face_boundary.x, roi->face_boundary.y,
      history->face_position.faces_inside);
  }

  /* Fill face size */
  if (p_fd_chromatix->stab_size.enable) {
    history->face_size.index = (history->face_size.index + 1) %
      p_fd_chromatix->stab_history;
    history->face_size.entry[history->face_size.index].x =
      roi->face_boundary.dx;
    history->face_size.entry[history->face_size.index].y =
      roi->face_boundary.dy;

    history->face_size.history_size = p_fd_chromatix->stab_history;
    if (history->face_size.faces_inside < p_fd_chromatix->stab_history) {
      history->face_size.faces_inside++;
    }

    IDBG_MED("%s:%d] Add face size %dx%d faces insight %d",
      __func__, __LINE__, roi->face_boundary.dx, roi->face_boundary.dy,
      history->face_position.faces_inside);
  }

  /* Fill mouth position */
  if (p_fd_chromatix->stab_mouth.enable) {
    history->mouth_position.index = (history->mouth_position.index + 1) %
      p_fd_chromatix->stab_history;
    history->mouth_position.entry[history->mouth_position.index].x =
      (uint32_t)roi->fp.face_pt[FACE_PART_MOUTH].x;
    history->mouth_position.entry[history->mouth_position.index].y =
      (uint32_t)roi->fp.face_pt[FACE_PART_MOUTH].y;

    history->mouth_position.history_size = p_fd_chromatix->stab_history;
    if (history->mouth_position.faces_inside < p_fd_chromatix->stab_history) {
      history->mouth_position.faces_inside++;
    }

    IDBG_MED("%s:%d] Add mouth position %dx%d faces insight %d",
      __func__, __LINE__, roi->fp.face_pt[FACE_PART_MOUTH].x,
      roi->fp.face_pt[FACE_PART_MOUTH].y, history->face_position.faces_inside);
  }

  /* Fill smile degree position */
  if (p_fd_chromatix->stab_smile.enable) {
    history->smile_degree.index = (history->smile_degree.index + 1) %
      p_fd_chromatix->stab_history;
    history->smile_degree.entry[history->smile_degree.index].x =
      (uint32_t)roi->sm.smile_degree;
    history->smile_degree.entry[history->smile_degree.index].y =
      (uint32_t)roi->sm.smile_degree;

    history->smile_degree.history_size = p_fd_chromatix->stab_history;
    if (history->smile_degree.faces_inside < p_fd_chromatix->stab_history) {
      history->smile_degree.faces_inside++;
    }

    IDBG_MED("%s:%d] Add smile degree %d faces insight %d",
      __func__, __LINE__, roi->sm.smile_degree,
      history->face_position.faces_inside);
  }

  IDBG_MED("%s:%d] Added face id %d confidance %d",
    __func__, __LINE__, history->id, roi->fd_confidence);
}

/**
* Function: module_faceproc_stab_init_face
*
* Description: Add new face and reset the faces history
*
* Arguments:
*   @history - Faces history
*   @roi - New face which need to be added
*   @p_fd_chromatix: faceproc chromatix pointer
*
* Return values:
*   None
* Notes: none
**/
static void module_faceproc_stab_init_face(faceproc_faces_history_t *history,
  faceproc_info_t *roi, fd_chromatix_t *p_chromatix)
{
  memset(history, 0x0, sizeof(*history));

  if (p_chromatix->stab_pos.enable) {
    history->face_position.max_state_count = p_chromatix->stab_pos.state_cnt;
    history->face_position.state = FD_STAB_STATE_STABILIZE;
  }

  if (p_chromatix->stab_size.enable) {
    history->face_size.max_state_count = p_chromatix->stab_size.state_cnt;
    history->face_size.state = FD_STAB_STATE_STABILIZE;
  }

  if (p_chromatix->stab_mouth.enable) {
    history->mouth_position.max_state_count = p_chromatix->stab_mouth.state_cnt;
    history->mouth_position.state = FD_STAB_STATE_STABILIZE;
  }

  if (p_chromatix->stab_smile.enable) {
    history->smile_degree.max_state_count = p_chromatix->stab_smile.state_cnt;
    history->smile_degree.state = FD_STAB_STATE_STABILIZE;
  }

  module_faceproc_stab_add_face(history, roi, p_chromatix);

  if (p_chromatix->stab_pos.enable) {
    history->face_position.stable_entry =
      history->face_position.entry[history->face_position.index];
  }

  if (p_chromatix->stab_size.enable) {
    history->face_size.stable_entry =
      history->face_size.entry[history->face_size.index];
  }

  if (p_chromatix->stab_mouth.enable) {
    history->mouth_position.stable_entry =
      history->mouth_position.entry[history->mouth_position.index];
  }

  if (p_chromatix->stab_smile.enable) {
    history->smile_degree.stable_entry =
      history->smile_degree.entry[history->smile_degree.index];
  }
}

/**
* Function: module_faceproc_stab_is_continues
*
* Description: Return true if stabilization of the values is working in
*   continues mode
*
* Return values:
*   TRUE - If stabilization is operating in continues mode
*
* Notes: none
**/
static boolean module_faceproc_stab_is_continues(fd_face_stab_mode_t stab_mode)
{
  boolean mode_continues;
  switch (stab_mode) {
  case FD_STAB_CONTINUES_EQUAL:
  case FD_STAB_CONTINUES_SMALLER:
  case FD_STAB_CONTINUES_BIGGER:
  case FD_STAB_CONTINUES_CLOSER_TO_REFERENCE:
    mode_continues = TRUE;
    break;
  case FD_STAB_BIGGER:
  case FD_STAB_SMALLER:
  case FD_STAB_EQUAL:
  case FD_STAB_WITHIN_THRESHOLD :
    return FALSE;
  default:
    mode_continues = TRUE;
    break;
  }
  return mode_continues;
}

/**
* Function: module_faceproc_stab_is_stable
*
* Description: Function which checks if values are stable
*
* Return values:
*   TRUE - if values are stable
* Notes: none
**/
static boolean module_faceproc_stab_is_stable(
  faceproc_history_holder_t *history, faceproc_history_entry_t *refer,
  fd_face_stab_params_t *p_stab_params)
{
  boolean stable;

  switch (p_stab_params->mode) {
  case FD_STAB_CONTINUES_SMALLER:
  case FD_STAB_SMALLER:
    stable = (history->stable_entry.x < history->entry[history->index].x &&
      history->stable_entry.y < history->entry[history->index].y);
    break;
  case FD_STAB_CONTINUES_BIGGER:
  case FD_STAB_BIGGER:
    stable = (history->stable_entry.x > history->entry[history->index].x &&
      history->stable_entry.y > history->entry[history->index].y);
    break;
  case FD_STAB_CONTINUES_CLOSER_TO_REFERENCE:
  case FD_STAB_CLOSER_TO_REFERENCE:
    stable = TRUE;
    if (refer) {
      stable = FD_STAB_CALC_DISTANCE(*refer, history->stable_entry) <
               FD_STAB_CALC_DISTANCE(*refer, history->entry[history->index]);
    }
    break;
  case FD_STAB_WITHIN_THRESHOLD: {
    stable = module_faceproc_within_limit(
      &history->entry[history->index], &history->stable_entry,
      p_stab_params->stable_threshold);
    IDBG_MED("history=%p, stable=%d, threshold=%d, "
      "new=(%d, %d), stable=(%d, %d)",
      history, stable, p_stab_params->stable_threshold,
      history->entry[history->index].x, history->entry[history->index].y,
      history->stable_entry.x, history->stable_entry.y);
  }
  break;
  case FD_STAB_EQUAL:
  default:
    stable = (history->stable_entry.x == history->entry[history->index].x &&
      history->stable_entry.y == history->entry[history->index].y);
    break;
  }
  return stable;
}

/**
* Function: module_faceproc_apply_hyst
*
* Description: Apply hysteresis with given direction
*  @stab_params - Stabilization parameters
*  @curr_entry - current entry
*  @new_entry - new entry
*  @direction_up - 0 direction down, otherwise direction up.
*
* Return values:
*   TRUE - if values are stable
* Notes: none
**/
static void module_faceproc_apply_hyst(fd_face_stab_params_t *stab_params,
  uint32_t *curr_entry, uint32_t *new_entry, int direction_up)
{
  if (direction_up) {
    if (*new_entry > stab_params->hyst.end_B) {
      *curr_entry = *new_entry;
    } else if (*new_entry > stab_params->hyst.start_B) {
      *new_entry = stab_params->hyst.start_B;
    } else if (*new_entry > stab_params->hyst.end_A) {
      *curr_entry = *new_entry;
    } else if (*new_entry > stab_params->hyst.start_A) {
      *new_entry = stab_params->hyst.start_A;
    } else {
      *curr_entry = *new_entry;
    }
  } else {
    if (*new_entry < stab_params->hyst.start_A) {
      *curr_entry = *new_entry;
    } else if (*new_entry < stab_params->hyst.end_A) {
      *new_entry = stab_params->hyst.end_A;
    } else if (*new_entry < stab_params->hyst.start_B) {
      *curr_entry = *new_entry;
    } else if (*new_entry < stab_params->hyst.end_B) {
      *new_entry = stab_params->hyst.end_B;
    } else {
      *curr_entry = *new_entry;
    }
  }
}

/**
* Function: module_faceproc_apply_average
*
* Description: Apply average filter on history collected.
*  @stab_params - Stabilization parameters.
*  @p_history - Pointer to history holder.
*
* Return values: None.
**/
static void module_faceproc_apply_average(fd_face_stab_params_t *stab_params,
  faceproc_history_holder_t *p_history)
{
  uint32_t i, index;
  uint32_t history_length;
  uint64_t sum_x = 0, sum_y = 0;

  if (stab_params->average.history_length < 2) {
    p_history->stable_entry = p_history->entry[p_history->index];
    return;
  }

  history_length = stab_params->average.history_length;
  if (history_length > p_history->faces_inside) {
    history_length = p_history->faces_inside;
  }

  index = (p_history->index + p_history->faces_inside - history_length + 1) %
    p_history->faces_inside;

  for (i = 0; i < history_length; i++) {
    index %= p_history->history_size;
    sum_x += p_history->entry[index].x;
    sum_y += p_history->entry[index].y;
    index++;
  }
  p_history->stable_entry.x = (sum_x + history_length / 2) / history_length;
  p_history->stable_entry.y = (sum_y + history_length / 2) / history_length;
  IDBG_MED("Positon after applying averagein x %d  y %d",
    p_history->stable_entry.x, p_history->stable_entry.y);
}

/**
* Function: module_faceproc_stab_median_sort
*
* Description: CAllback function for qsort used for sorting
*   history for median filter.
*
* Arguments:
*   @arg1 - faceproc_info_t * First Roi
*   @arg2 - faceproc_info_t * Second Roi
*
* Return values:
*   0 - equal
*   1 - value 1 is bigger the value 2
*  -1 - value 1 is smaller then value 2
* Notes: none
**/
static int module_faceproc_stab_median_sort(const void *arg1, const void *arg2)
{
  uint32_t *value1 = (uint32_t *)arg1;
  uint32_t *value2 = (uint32_t *)arg2;

  if (*value1 == *value2) {
    return 0;
  }
  if (*value1 > *value2) {
    return 1;
  }
  return -1;
}

/**
* Description: Function used to apply median filter.
*
* Description: Apply median filter on history collected.
*  @stab_params - Stabilization parameters.
*  @p_history - Pointer to history holder.
*
* Return values: None.
**/
static void module_faceproc_apply_median(fd_face_stab_params_t *stab_params,
  faceproc_history_holder_t *p_history)
{
  uint32_t i, index;
  faceproc_history_entry_t med_entry[stab_params->average.history_length];

  if (stab_params->average.history_length > p_history->faces_inside) {
    p_history->stable_entry = p_history->entry[p_history->index];
    return;
  }

  index = p_history->index;
  for (i = 0; i < stab_params->average.history_length; i++) {
    med_entry[i] = p_history->entry[index++ % p_history->history_size];
  }

  qsort(&med_entry[0].x, stab_params->average.history_length, sizeof(med_entry[0]),
    module_faceproc_stab_median_sort);

  index = (stab_params->average.history_length / 2);
  p_history->stable_entry.x = med_entry[index].x;

  qsort(&med_entry[0].y, stab_params->average.history_length, sizeof(med_entry[0]),
    module_faceproc_stab_median_sort);

  index = (stab_params->average.history_length / 2);
  p_history->stable_entry.y = med_entry[index].y;
}

/**
* Function: module_faceproc_apply_filter
*
* Description: Apply filter based on stabilization parameters.
*
* Arguments:
*   @stab_params - Stabilization parameters.
*   @p_history - Pointer to stabilization history.
*   @new_e: New entry need to be filtered.
*
* Return values: None.
**/
static void module_faceproc_apply_filter(fd_face_stab_params_t *stab_params,
  faceproc_history_holder_t *p_history)
{
  faceproc_history_entry_t *curr_e = &p_history->stable_entry;
  faceproc_history_entry_t *new_e = &p_history->entry[p_history->index];

  switch (stab_params->filter_type) {
  case FD_STAB_HYSTERESIS:
    if (new_e->x > curr_e->x) {
      module_faceproc_apply_hyst(stab_params, &curr_e->x, &new_e->x, 1);
    } else {
      module_faceproc_apply_hyst(stab_params, &curr_e->x, &new_e->x, 0);
    }
    if (new_e->y > curr_e->y) {
      module_faceproc_apply_hyst(stab_params, &curr_e->y, &new_e->y, 1);
    } else {
      module_faceproc_apply_hyst(stab_params, &curr_e->y, &new_e->y, 0);
    }
    break;
  case FD_STAB_TEMPORAL:
    FD_STAB_APPLY_TEMP_FILTER(curr_e->x, stab_params->temp.num,
      new_e->x, stab_params->temp.denom);
    FD_STAB_APPLY_TEMP_FILTER(curr_e->y, stab_params->temp.num,
      new_e->y, stab_params->temp.denom);
    break;
  case FD_STAB_AVERAGE:
    module_faceproc_apply_average(stab_params, p_history);
    break;
  case FD_STAB_MEDIAN:
    module_faceproc_apply_median(stab_params, p_history);
    break;
  case FD_STAB_NO_FILTER:
  default:
    *curr_e = *new_e;
    break;
  }
}


/**
* Function: module_faceproc_stab_filter
*
* Description: Stabilization it will stabilize the entry
*   based on reference entry it self, if reference is passed
*   coordinates change will be detected from them
*
* Arguments:
*   @history - History for this entry
*   @stab_params - Stabilization parameters
*   @refer: Reference coordinates for stabilization
*   @threshold: Variation threshold
*    0 - Face is stable when previous face is equal with new
*    1 - Face is stable when previous face is bigger then new
*    2 - Face is stable when previous face is smaller then new
*
* Return values:
*   IMG error codes
**/
static int module_faceproc_stab_filter(faceproc_history_holder_t *history,
  fd_face_stab_params_t *stab_params, faceproc_history_entry_t *refer,
  uint32_t threshold)
{
  faceproc_history_state_t new_state;
  boolean within_limit = FALSE;
  boolean face_stable = FALSE;
  uint32_t last_index;
  int ret = IMG_SUCCESS;

  if (history->faces_inside == 1) {
    IDBG_MED("%s:%d] not enough faces skip", __func__, __LINE__);
    return IMG_SUCCESS;
  }

  /* Do nothing if previous coordinates are the same as new
   * Sometimes stabilization is executed twice for same results */
  last_index =
    (history->index + history->faces_inside - 1) %  history->faces_inside;
  if ((history->entry[history->index].x != history->entry[last_index].x ||
       history->entry[history->index].y != history->entry[last_index].y) &&
      (history->entry[history->index].x == history->stable_entry.x &&
       history->entry[history->index].y == history->stable_entry.y)) {
    IDBG_MED("%s:%d] Stabilization executed twice", __func__, __LINE__);
    return IMG_SUCCESS;
  }

  IDBG_LOW("Current state is %u", history->state);
  /* If there is refer for stabilization use it */
  if (FD_STAB_STATE_STABILIZE != history->state) {
    if (refer) {
      within_limit = module_faceproc_within_limit(refer,
        &history->stable_refer, (int32_t)threshold);
    } else {
      within_limit = module_faceproc_within_limit(
        &history->entry[history->index], &history->stable_entry,
        (int32_t)threshold);
    }
  }

  new_state = history->state;
  switch (history->state) {
  case FD_STAB_STATE_UNSTABLE:
    if (history->state_count >= history->max_state_count) {
      new_state = FD_STAB_STATE_STABILIZE;
    } else if (TRUE == within_limit) {
      new_state = FD_STAB_STATE_STABLE;
    }
    break;
  case FD_STAB_STATE_STABLE:
    if (TRUE == within_limit) {
      if (module_faceproc_stab_is_continues(stab_params->mode)) {
        face_stable = module_faceproc_stab_is_stable(history, refer,
          stab_params);
        if (FALSE == face_stable) {
          module_faceproc_apply_filter(stab_params, history);
        }
      }
      break;
    } else if (history->max_state_count) {
      new_state = FD_STAB_STATE_UNSTABLE;
      break;
    } else {
      /* do not break here go to stabilize state directly */
      new_state = FD_STAB_STATE_STABILIZE;
      history->state_count = 0;
    }
  case FD_STAB_STATE_STABILIZE:
    IDBG_MED("State count value in Stabilize state %d", history->state_count);
    if ((stab_params->min_count_for_stable_state > 0) &&
      (history->state_count < stab_params->min_count_for_stable_state)) {
        face_stable = FALSE;
    } else {
      face_stable = module_faceproc_stab_is_stable(history, refer, stab_params);
    }
    /* If face is stabilized put to the stable state */
    if (face_stable) {
      if (refer) {
        history->stable_refer = *refer;
      }
      new_state = FD_STAB_STATE_STABLE;
    }

    /* If continues mode is selected do not apply filter */
    if (module_faceproc_stab_is_continues(stab_params->mode)) {
      history->stable_entry.x = history->entry[history->index].x;
      history->stable_entry.y = history->entry[history->index].y;
    } else {
      module_faceproc_apply_filter(stab_params, history);
    }
    break;
  default:
    ret = IMG_ERR_GENERAL;
    IDBG_ERROR("Error invalid state something went wrong");
    goto out;
  }

  /* Change state if requested */
  if (new_state != history->state) {
    history->state = new_state;
    history->state_count = 0;
  } else {
    history->state_count++;
  }

out:
  return ret;
}

/**
* Function: module_faceproc_roi_sort_pos
*
* Description: CAllback function for qsort used for sorting
*   faces based on face x,y position
*
* Arguments:
*   @arg1 - faceproc_info_t * First Roi
*   @arg2 - faceproc_info_t * Second Roi
*
* Return values:
*   0 - equal
*   1 - position of rec1 is after position of rect2
*  -1 - position of rec1 is before position of rect2
* Notes: none
**/
static int module_faceproc_stab_roi_sort_pos(const void *arg1,
  const void *arg2)
{
  faceproc_info_t *first_roi = (faceproc_info_t *)arg1;
  faceproc_info_t *second_roi = (faceproc_info_t *)arg2;
  uint32_t size, diff;

  if (first_roi->face_boundary.dx >  second_roi->face_boundary.dx) {
    size = first_roi->face_boundary.dx;
  } else {
    size = second_roi->face_boundary.dx;
  }

  /* Handle horizontal overlap */
  diff = abs((int)first_roi->face_boundary.y - (int)second_roi->face_boundary.y);
  if (diff < size) {
    if (first_roi->face_boundary.x > second_roi->face_boundary.x) {
      return 1;
    } else {
      return -1;
    }
  }

  /* Handle standart usecase */
  if ((first_roi->face_boundary.x +
      (first_roi->face_boundary.y * MAX_FD_WIDTH)) >
      (second_roi->face_boundary.x +
      (second_roi->face_boundary.y * MAX_FD_WIDTH)))
    return 1;
  else
    return -1;

  return 0;
}

/**
* Function: module_faceproc_faces_stabilization
*
* Description: Function to faces stabilization
*
* Arguments:
*   @p_stab - Faceproc stabilization handle
*   @p_fd_chromatix - Pointer to faceproc chromatix
*   @p_frame_dim - Frame dimensions
*   @p_result: Faceproc result
*
* Return values:
*   none
*
* Notes: none
**/
int module_faceproc_faces_stabilization(faceproc_stabilization_t *p_stab,
  fd_chromatix_t *p_fd_chromatix, img_size_t *p_frame_dim,
  faceproc_result_t *p_result)
{
  faceproc_history_entry_t refer;
  faceproc_history_entry_t eyes_center;
  faceproc_history_entry_t *p_refer;
  uint32_t i, j, threshold;
  uint32_t eyes_distance, faces_detected;
  int position, ref_available;
  int ret = IMG_SUCCESS;

  if (!p_stab || !p_fd_chromatix || !p_result || !p_frame_dim) {
   IDBG_ERROR("%s:%d]Invalid input", __func__, __LINE__);
   return IMG_ERR_INVALID_INPUT;
  }

  if (p_result->num_faces_detected > MAX_FACE_ROI) {
    IDBG_ERROR("%s:%d] Invalid number of faces %d",
      __func__, __LINE__, p_result->num_faces_detected);
    p_stab->detected_faces = 0;
    return IMG_ERR_INVALID_INPUT;
  }

  if (p_fd_chromatix->stab_history > FACEPROC_STAB_HISTORY) {
    IDBG_ERROR("%s:%d] Invalid history size %d max %d", __func__, __LINE__,
      p_fd_chromatix->stab_history, FACEPROC_STAB_HISTORY);
    return IMG_ERR_INVALID_INPUT;
  }

  if (!p_result->num_faces_detected) {
    IDBG_MED("%s:%d] no faces reset the histories", __func__, __LINE__);
    p_stab->detected_faces = 0;
    return IMG_SUCCESS;
  }

  /* Sort the output ROI */
  qsort(p_result->roi, p_result->num_faces_detected, sizeof(p_result->roi[0]),
    module_faceproc_stab_roi_sort_pos);
  i = 0, j = 0;
  faces_detected = p_result->num_faces_detected;
  while (i < faces_detected) {
    /* If there are new faces put them them in history */
    if (i >= p_stab->detected_faces) {
      /* Mark face as detected only if confidence is within the threshold */
      if (is_face_detected(&p_result->roi[i], p_fd_chromatix)) {
        module_faceproc_stab_init_face(&p_stab->faces[j++], &p_result->roi[i++],
          p_fd_chromatix);
        p_stab->detected_faces++;
      } else {
        faces_detected--;
        /* If not last remove this face from result */
        if (i < faces_detected) {
          memmove(&p_result->roi[i], &p_result->roi[i + 1],
            sizeof(p_result->roi[0]) * (faces_detected - i));
        }
      }
      continue;
    }
    /* This is probably already detected face check for lost threshold */
    if (is_face_lost(&p_result->roi[i], p_fd_chromatix)) {
      faces_detected--;
      /* If not last remove this face from result */
      if (i < faces_detected) {
        memmove(&p_result->roi[i], &p_result->roi[i + 1],
          sizeof(p_result->roi[0]) * (faces_detected - i));
      }
      continue;
    }

    /* Check boundary limit */
    position = module_faceproc_stab_check_face(&p_result->roi[i],
      &p_stab->faces[j]);
    IDBG_LOW("Face position is %d", position);

    if (FACEPROC_FACE_SAME == position) {

      module_faceproc_stab_add_face(&p_stab->faces[j++],
        &p_result->roi[i++], p_fd_chromatix);

    } else if (FACEPROC_FACE_BEFORE == position) {

       /* Move the faces to the right if it is not the last element  */
      if (i < (p_result->num_faces_detected - 1)) {
         memmove(&p_stab->faces[j + 1], &p_stab->faces[j],
           sizeof(p_stab->faces[0]) * (p_stab->detected_faces - j));
         /* No need to increment if we reach max detected faces */
         if (p_stab->detected_faces < MAX_FACES_TO_DETECT) {
           p_stab->detected_faces++;
         }
      }

      /* Put the new face in place */
      module_faceproc_stab_init_face(&p_stab->faces[j++], &p_result->roi[i++],
        p_fd_chromatix);

    } else if (FACEPROC_FACE_AFTER == position) {

       /* Move faces to the left and remove current */
       memmove(&p_stab->faces[j], &p_stab->faces[j + 1],
         sizeof(p_stab->faces[0]) * (p_stab->detected_faces - j));
       p_stab->detected_faces--;
    }
  }

  if (p_stab->detected_faces > faces_detected) {
    p_stab->detected_faces = faces_detected;
  }

  /* Stabilize faces */
  memset(&refer, 0x00, sizeof(refer));
  for (i = 0; i < faces_detected; i++) {
    /* Get eyes distance and center they will be used as reference */
    eyes_distance =
      (uint32_t)FD_STAB_CALC_DISTANCE(p_result->roi[i].fp.face_pt[FACE_PART_LEFT_EYE],
      p_result->roi[i].fp.face_pt[FACE_PART_RIGHT_EYE]);
    eyes_center.x =
      (uint32_t)((p_result->roi[i].fp.face_pt[FACE_PART_LEFT_EYE].x +
      p_result->roi[i].fp.face_pt[FACE_PART_RIGHT_EYE].x) / 2);
    eyes_center.y =
      (uint32_t)((p_result->roi[i].fp.face_pt[FACE_PART_LEFT_EYE].y +
      p_result->roi[i].fp.face_pt[FACE_PART_RIGHT_EYE].y) / 2);

    /* Use reference only if available */
    ref_available = 0;
    if (eyes_center.x && eyes_center.y  && eyes_distance) {
      ref_available = 1;
    }

    /* Stabilize face size */
    if (p_fd_chromatix->stab_size.enable) {
      IDBG_LOW("In stab_size enable");
      p_refer = NULL;
      if (ref_available && p_fd_chromatix->stab_size.use_reference) {
        refer.x = eyes_distance;
        refer.y = eyes_distance;
        p_refer = &refer;
        threshold = FD_STAB_CALC_THRESHOLD(eyes_distance,
          p_fd_chromatix->stab_size.threshold);
      } else {
        threshold = FD_STAB_CALC_THRESHOLD(p_stab->faces[i].face_size.stable_entry.x,
         p_fd_chromatix->stab_size.threshold);
      }

      ret = module_faceproc_stab_filter(&p_stab->faces[i].face_size,
        &p_fd_chromatix->stab_size, p_refer, threshold);
      if (IMG_ERROR(ret)) {
        IDBG_ERROR("Can not apply face size filter");
        goto out;
      }
      /* Fill stable face size */
      p_result->roi[i].face_boundary.dx =
        p_stab->faces[i].face_size.stable_entry.x;
      p_result->roi[i].face_boundary.dy =
        p_stab->faces[i].face_size.stable_entry.y;
    }

    /* Stabilize face position */
    if (p_fd_chromatix->stab_pos.enable) {
      IDBG_LOW("In stab_pos enable");
      uint32_t half_size;
      uint32_t max_allowed_threshold;

      p_refer = NULL;
      if (ref_available && p_fd_chromatix->stab_pos.use_reference) {
        refer.x = eyes_center.x;
        refer.y = eyes_center.y;
        p_refer = &refer;
      }
      threshold = FD_STAB_CALC_THRESHOLD(p_frame_dim->width,
       p_fd_chromatix->stab_pos.threshold);

      max_allowed_threshold = (p_result->roi[i].face_boundary.dx *
        p_fd_chromatix->stab_pos.threshold) / 100;

      // Do not allow ROI displacement of greater than 'threshold' percent
      // of face size.
      // Take min of threshold, max_allowed_threshold as the final threshold.
      if (threshold > max_allowed_threshold) {
        threshold = max_allowed_threshold;
      }

      ret = module_faceproc_stab_filter(&p_stab->faces[i].face_position,
        &p_fd_chromatix->stab_pos, p_refer, threshold);
      if (IMG_ERROR(ret)) {
        IDBG_ERROR("Can not apply face position filter");
        goto out;
      }

      /* Fill stable face position, and handle corner cases */
      half_size = (p_result->roi[i].face_boundary.dx / 2);
      if (p_stab->faces[i].face_position.stable_entry.x > half_size) {
        p_result->roi[i].face_boundary.x =
          p_stab->faces[i].face_position.stable_entry.x - half_size;
      } else {
        p_result->roi[i].face_boundary.x = 0;
      }

      half_size = (p_result->roi[i].face_boundary.dy / 2);
      if (p_stab->faces[i].face_position.stable_entry.y > half_size) {
        p_result->roi[i].face_boundary.y =
          p_stab->faces[i].face_position.stable_entry.y - half_size;
      } else {
        p_result->roi[i].face_boundary.y = 0;
      }
    }

    /* Stabilize mouth */
    if (p_fd_chromatix->stab_mouth.enable) {
      p_refer = NULL;
      if (ref_available && p_fd_chromatix->stab_mouth.use_reference) {
        refer.x = eyes_center.x;
        refer.y = eyes_center.y;
        p_refer = &refer;
      }
      threshold = FD_STAB_CALC_THRESHOLD(p_frame_dim->width,
        p_fd_chromatix->stab_mouth.threshold);

      ret = module_faceproc_stab_filter(&p_stab->faces[i].mouth_position,
        &p_fd_chromatix->stab_mouth, p_refer, threshold);
      if (IMG_ERROR(ret)) {
        IDBG_ERROR("Can not apply mouth position filter");
        goto out;
      }
      /* Fill stable face mouth */
      p_result->roi[i].fp.face_pt[FACE_PART_MOUTH].x =
        p_stab->faces[i].mouth_position.stable_entry.x;
      p_result->roi[i].fp.face_pt[FACE_PART_MOUTH].y =
        p_stab->faces[i].mouth_position.stable_entry.y;
    }

    IDBG_LOW("p_fd_chromatix->stab_smile.enable %d", p_fd_chromatix->stab_smile.enable);
    /* Stabilize smile */
    if (p_fd_chromatix->stab_smile.enable) {
      p_refer = NULL;
      if (ref_available && p_fd_chromatix->stab_smile.use_reference) {
        refer.x = eyes_center.x;
        refer.y = eyes_center.y;
        p_refer = &refer;
      }
      threshold = FD_STAB_CALC_THRESHOLD(p_frame_dim->width,
        p_fd_chromatix->stab_smile.threshold);

      ret = module_faceproc_stab_filter(&p_stab->faces[i].smile_degree,
        &p_fd_chromatix->stab_smile, p_refer, threshold);
      if (IMG_ERROR(ret)) {
        IDBG_ERROR("Can not apply smile degree filter");
        goto out;
      }
      p_result->roi[i].sm.smile_degree =
        (int)p_stab->faces[i].smile_degree.stable_entry.x;
    }
  }
  p_result->num_faces_detected = faces_detected;

out:
 return ret;
}

