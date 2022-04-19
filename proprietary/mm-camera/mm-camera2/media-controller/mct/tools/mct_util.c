/*Copyright (c) 2015, 2018 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *mct_util.c
 *
 * This file contains  helper functions which can be used by other modules.
 *
 */
#include "mct_util.h"
#include "camera_dbg.h"
#include <media/msm_cam_sensor.h>
#include <linux/media.h>

timer_t mct_timerid;

/** mct_util_calculate_lcm:
 *
 *This function calculates lcm when both numbers provided
 *are non-zero.
 *Incase when one of them is zero, it returns value of the
 *second argument.
 *When both the data are zero, it returns zero.
 *
 *@data1: data provided by user
 *@data2: data provided by user
 *
 *Return: calculated LCM
 **/

int mct_util_calculate_lcm(int data1, int data2)
{

  int lcm = 0;

  data1 = abs(data1);
  data2 = abs(data2);

  if ((data1 == 0) && (data2 == 0)) {
    /*Return zero for non positive numbers*/
    lcm = 0;
    goto end;
  }
  else if (data1 == 0) {
    lcm = data2;
    goto end;
  }
  else if (data2 == 0) {
    lcm = data1;
    goto end;
  }

  lcm = MAX(data1, data2);

  while (1) {
    if (lcm%data1 == 0 && lcm%data2 == 0) {
      break;
    }
    lcm++;
  }

end:
  return lcm;
}

/** Name: mct_util_get_timeout
 *
 *  Arguments/Fields:
 *    @timeout_interval: Timeout interval in seconds
 *    @ts: Reference arg to timespec structure
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *  Description:
 *    This method calculates the end-time for timeout
 *    based on the current system time and timeout interval.
 *    Returns a reference to the end-time which is a timespec structure.
 **/
boolean mct_util_get_timeout(uint32_t timeout_interval,
  struct timespec *ts)
{
  if (!ts) {
    CLOGE(CAM_MCT_MODULE, "NULL timeout arg");
    return FALSE;
  }

  clock_gettime(CLOCK_MONOTONIC, ts);
  /* timeout_interval is int. So its precision is in number of seconds. */
  ts->tv_sec = ts->tv_sec + timeout_interval;

  return TRUE;
}

/** Name: mct_spawn_thread
 *
 *  Arguments/Fields:
 *    @t_func: Function name of thread
 *    @args: Agruments for pthread_create
 *    @t_name: Thread name
 *    @t_state: thread state
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *
 *  Description:
 *    This wrapper function creates thread using pthread_create
 **/
boolean mct_spawn_thread(void *(*t_func)(void *),
 void *args,  const char *t_name, int t_state)
{
  int t_status;
  pthread_attr_t attr;
  pthread_t t_id;
  /* An attribute object is opaque,
   * and can be modified by calling function provided.
   */
  pthread_attr_init(&attr);
  /* By default it is joinable so set only when it is detached state */
  if (t_state ==  PTHREAD_CREATE_DETACHED) {
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  }

  t_status = pthread_create(&t_id, &attr, t_func, (void *)args);
  if (t_status) {
    CLOGE(CAM_MCT_MODULE, "pthread_create failed for %s", t_name);
    return FALSE;
  }
  pthread_setname_np(t_id, t_name);
  return TRUE;
}

/*FPS-Related Compute functions */

/** mct_util_sort_fps_entries:
 *    @src: input fps value
 *    @fps_table: pointer to fps table to be sorted
 *    @num_entries: number of entries in fps table
 *
 *  Return: TRUE for success, FALSE for failure
 *
 *  Description:
 *    Sorts the input fps table. Uses bubble-sort technique
 **/
void mct_util_sort_fps_entries (
    cam_fps_range_t* fps_table, uint8_t num_entries)
{
  uint8_t i = 0, j = 0;
  cam_fps_range_t temp;

  if (!fps_table) {
    CLOGE(CAM_MCT_MODULE, "NULL fps table");
    return;
  }
  for (i=0; i<num_entries -1; i++) {
    for (j = num_entries -1; j > i; j--) {
      if ( (fps_table[j].max_fps < fps_table[j-1].max_fps) ||
        ( (fps_table[j].max_fps == fps_table[j-1].max_fps) &&
          (fps_table[j].min_fps < fps_table[j-1].min_fps) ) ) {
          /* Swap */
          temp = fps_table[j];
          fps_table[j] = fps_table[j-1];
          fps_table[j-1] = temp;
      }
    }
  }
}


/** mct_util_find_closest_fps:
 *    @src: input fps value
 *    @ref_table: pointer to reference fps table for matching
 *    @num_entries: number of entries in reference table
 *    @compare_type: Choose how to find the closest reference fps.
 *
 *  Return: index of closest matching entry from reference fps table to src_fps
 *
 **/
int8_t mct_util_find_closest_fps (
    float src_fps, cam_fps_range_t* ref_table, uint8_t num_entries,
    mct_util_comparison_type_t compare_type)
{
  uint8_t index;
  int8_t matching_index = -1;
  float score = 1000.0; /* A very large fps difference */
  float deviation = 0.0;

  if(!ref_table || (0 == num_entries))
  {
    CLOGE(CAM_MCT_MODULE, "Invalid reference table");
    return matching_index;
  }
  for (index = 0; (index < num_entries && index < MAX_SIZES_CNT); index++) {
    if ( ( (COMPARE_LOWER == compare_type) &&
      (ref_table[index].max_fps <= src_fps) ) ||
      ( (COMPARE_HIGHER == compare_type) &&
      (ref_table[index].max_fps >= src_fps) ) ) {
      deviation = src_fps - ref_table[index].max_fps;
      if (deviation <= score) {
        score = deviation;
        matching_index = index;
      }
    }
  }
  if(matching_index != -1) {
    /* Valid match has been found */
    CLOGD(CAM_MCT_MODULE,
          "Closest matching entry: ref_table[%d].max_fps = [%f]",
          matching_index, ref_table[matching_index].max_fps);
  }
  else {
    CLOGD(CAM_MCT_MODULE, "No match found for src_fps [%f]", src_fps);
  }
  return matching_index;
}

/** mct_util_check_matching_fps:
 *    @input_fps: input fps value
 *    @ref_table: pointer to reference fps table for matching
 *    @num_entries: number of entries in reference table
 *
 *  Return: TRUE if input_fps is found in ref_table.
 *             FALSE if no match is found for input_fps in ref_table.
 **/
boolean mct_util_check_matching_fps (
    cam_fps_range_t input_fps, cam_fps_range_t* ref_table, uint8_t num_entries)
{
  uint8_t index;
  if(!ref_table)
  {
    CLOGE(CAM_MCT_MODULE, "Invalid reference table");
    return FALSE;
  }
  for (index = 0; index < num_entries; index++) {
    if (((uint32_t)(input_fps.max_fps) == (uint32_t)(ref_table[index].max_fps))
        &&
       ((uint32_t)(input_fps.min_fps) == (uint32_t)(ref_table[index].min_fps)))
      return TRUE;
  }
  return FALSE;
}

/** mct_util_pick_lowest_fps:
 *    @crucible: Array of fps entries from all contributors
 *    @num_entries: Size of crucible array
 *
 *  Description:
 *    Picks the 'lowest' fps range among elements of the array
 *    'lowest' fps is the fps range with the lowest (min_fps, max_fps)
 *    combination with preference on max_fps comparison.
 *    Ignores zeroed out entries.
 *
 *  Return: SUCCESS: Index of lowest fps entry in array.
 *          FAILURE: -1
 **/

int8_t mct_util_pick_lowest_fps(cam_fps_range_t *crucible,
  uint8_t num_entries)
{
  int8_t matching_index = -1;
  uint8_t i = 0;
  cam_fps_range_t temp_fps;
  /* Initialize temp_fps with very high values */
  temp_fps.min_fps = 1000.0;
  temp_fps.max_fps = 1000.0;

  if (!crucible || (0 == num_entries)) {
    CLOGE(CAM_MCT_MODULE, "Invalid args: crucible [%p], num_entries %d",
          crucible, num_entries);
    return matching_index;
  }

  for (i = 0; i < num_entries; i++) {
    if ( ( (crucible[i].max_fps !=0) && (crucible[i].min_fps !=0) )
      && ( (crucible[i].max_fps < temp_fps.max_fps) ||
      ( (crucible[i].max_fps == temp_fps.max_fps) &&
      (crucible[i].min_fps <= temp_fps.min_fps) ) ) ) {
      temp_fps = crucible[i];
      matching_index = i;
    }
  }
  CLOGD(CAM_MCT_MODULE, "Selected crucible[%d]: (%f, %f)",
    matching_index, temp_fps.min_fps, temp_fps.max_fps);
  return matching_index;
}

/* END of FPS-Related Compute functions */

/** Name: mct_util_find_v4l2_subdev
 *
 *  Description:
 *    update sensor probe done once module init completes
 **/
int32_t mct_util_find_v4l2_subdev(char *node_name)
{
  int num_media_devices = 0;
  char dev_name[MAX_DEV_NAME_SIZE];
  int dev_fd = 0;
  int rc = 0;
  struct media_device_info mdev_info;
  int num_entities = 0;

  if (!node_name) {
    CLOGE(CAM_MCT_MODULE, "failed");
    return -EINVAL;
  }
  while (1) {
    snprintf(dev_name, sizeof(dev_name), "/dev/media%d", num_media_devices++);
    dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);
    if (dev_fd < 0) {
      /* Done enumerating media devices */
      CLOGD(CAM_MCT_MODULE, "Done enumerating media devices");
      break;
    }
    memset(&mdev_info, 0, sizeof(struct media_device_info));
    rc = ioctl(dev_fd, MEDIA_IOC_DEVICE_INFO, &mdev_info);
    if (rc < 0) {
      CLOGD(CAM_MCT_MODULE, "Done enumerating media devices");
      close(dev_fd);
      break;
    }
    if (strncmp(mdev_info.model, "msm_config", sizeof(mdev_info.model)) != 0) {
      close(dev_fd);
      continue;
    }
    num_entities = 0;
    while (1) {
      struct media_entity_desc entity;
      memset(&entity, 0, sizeof(entity));
      entity.id = num_entities | MEDIA_ENT_ID_FLAG_NEXT;
      rc = ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
      if (rc < 0) {
        CLOGE(CAM_MCT_MODULE, "Done enumerating media entities");
        rc = 0;
        break;
      }
      num_entities = entity.id;
      CLOGE(CAM_MCT_MODULE, "entity name %s type %d group id %d",
        entity.name, entity.type, entity.group_id);
      if (mct_util_match_subdev_type(entity, MSM_CAMERA_SUBDEV_SENSOR_INIT,
        MEDIA_ENT_T_V4L2_SUBDEV)) {
        /* found the video device */
        strlcpy(node_name, entity.name, MAX_DEV_NAME_SIZE);
        CLOGE(CAM_MCT_MODULE, "node_name = %s", node_name);
        break;
      }
    } /* enumerate entites */
    close(dev_fd);
  } /* enumerate media devices */
  return TRUE;
}
/** Name: mct_util_timer_handler
 *
 *  Description:
 *   Timer handler raises sigabrt if timer expires.
 **/
void mct_util_timer_handler(union sigval val __unused)
{
  CLOGE(CAM_MCT_MODULE, "Backend stuck during HAL/DS Command");
#ifdef MCT_STUCK_FLAG
  raise(SIGABRT);
#else
  raise(SIGTERM);
#endif
}

/** Name: mct_util_create_timer
 *
 *  Return:
 *    0 on success, -1 on failure
 *  Description:
 *    create the timer
 **/
int mct_util_create_timer()
{
    struct sigevent mct_t_sig;
    pthread_attr_t mct_t_attr;
    int mct_t_ret = 0;
    struct itimerspec mct_in, mct_out;

    /* create a timer */
    mct_t_sig.sigev_notify = SIGEV_THREAD;
    mct_t_sig.sigev_notify_function = mct_util_timer_handler;
    mct_t_sig.sigev_value.sival_ptr = NULL;
    pthread_attr_init(&mct_t_attr);
    mct_t_sig.sigev_notify_attributes = &mct_t_attr;

    mct_t_ret = timer_create(CLOCK_REALTIME, &mct_t_sig, &mct_timerid);

    if (!mct_t_ret) {
       mct_in.it_value.tv_sec = 0;
       mct_in.it_value.tv_nsec = 0;
       mct_in.it_interval.tv_sec = mct_in.it_value.tv_sec;
       mct_in.it_interval.tv_nsec = mct_in.it_value.tv_nsec;
       timer_settime(mct_timerid, 0, &mct_in, &mct_out);
    }

    return mct_t_ret;
}

/** Name: mct_util_set_timer
 *
 *  Description:
 *    set the timer
 **/
void mct_util_set_timer(struct timespec *timeToWait)
{
    struct itimerspec mct_in, mct_out;

    mct_in.it_value.tv_sec = timeToWait->tv_sec;
    mct_in.it_value.tv_nsec = timeToWait->tv_nsec;
    mct_in.it_interval.tv_sec = mct_in.it_value.tv_sec;
    mct_in.it_interval.tv_nsec = mct_in.it_value.tv_nsec;
    timer_settime(mct_timerid, 0, &mct_in, &mct_out);
}

static int mct_util_match_subdev_49(struct media_entity_desc entity,
  uint32_t gid, uint32_t type __unused)
{
  return (entity.type == gid);
}

static int mct_util_match_subdev(struct media_entity_desc entity,
  uint32_t gid, uint32_t type)
{
  return (entity.type == type && entity.group_id == gid);
}

int mct_util_match_subdev_type(struct media_entity_desc entity,
  uint32_t gid, uint32_t type)
{
  #ifdef USE_4_9_DEFS
  return mct_util_match_subdev_49(entity, gid, type);
  #else
  return mct_util_match_subdev(entity, gid, type);
  #endif
}
