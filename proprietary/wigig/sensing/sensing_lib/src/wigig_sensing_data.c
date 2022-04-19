/******************************************************************************
  @file    wigig_sensing_data.c
  @brief   The WiGig Sensing QMI Library Data Handling C file.

  DESCRIPTION
  Create, handle, and free wigig sensing data buffers.

  ---------------------------------------------------------------------------
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
*******************************************************************************/

#include "wigig_sensing_data.h"

static wigig_sensing_data lib_data = { 0 };
static pthread_mutex_t data_mutex;

inline static bool check_buf_full()
{
    if (lib_data.curr_buf_size == lib_data.cyclic_buffer_size)
        return true;

    return false;
}

inline static void advance_pointer
(
    uint32_t    *input_pointer,
    uint32_t    size
)
{
    /* wrap around pointer */
    *input_pointer = (*input_pointer + size) % lib_data.cyclic_buffer_size;
}

#define VAL_PER_LINE (8)
void wigig_sensing_dump_buff
(
    uint8_t *data,
    uint32_t data_len,
    uint32_t num_lines
)
{
  uint32_t line;
  uint32_t val[VAL_PER_LINE] = {0};
  uint32_t i;
  uint32_t offset = 0;

  if (data == NULL)
    return;

  if (data_len < num_lines*8*4)
    num_lines = ceil(data_len / (8.0*4));

  for (line = 0; line < num_lines; line++)
  {
    for (i = 0; i < VAL_PER_LINE; i++)
    {
        val[i] = data[offset + 4 * i] << 24 | data[offset + 4 * i + 1] << 16 | data[offset + 4 * i + 2] << 8 | data[offset + 4 * i + 3];
    }

    syslog(LOG_DEBUG, "%u: 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x",
           line,
           val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7]);
    offset = offset + 4*VAL_PER_LINE;
  }
}

void wigig_sensing_process_single_burst
(
    uint64_t timestamp,
    uint32_t data_len,
    uint8_t *data
)
{
    pthread_mutex_lock(&data_mutex);

    /* check if buffer is copyable*/
    if (lib_data.burst_buffer) {

        /*check to see if this is the first fragment of the burst to extract timestamp*/
        if (lib_data.burst_buffer_size == 0)
            lib_data.burst_timestamp = timestamp;

        memcpy((char*)lib_data.burst_buffer + lib_data.burst_buffer_size,
            (const void*)data, data_len);
        lib_data.burst_buffer_size += data_len;

        syslog(LOG_INFO, "data_len=%d", data_len);
    }
    else
        syslog(LOG_ERR, "Got data when there was no buffer, cannot proceed to copy");

    pthread_mutex_unlock(&data_mutex);
}

static int copy_to_buf
(
    uint64_t timestamp,
    uint32_t data_len,
    uint8_t *data
)
{
    pthread_mutex_lock(&data_mutex);

    /* check if buffer is copyable */
    if (!lib_data.cyclic_buffer) {
        syslog(LOG_ERR, "Got data when there was no buffer, cannot proceed to copy");
        pthread_mutex_unlock(&data_mutex);
        return WIGIG_SENSING_LIB_MEMORY_ERROR;
    }

    /* check if buffer is full */
    if (check_buf_full()) {
        advance_pointer(&lib_data.read_pointer, lib_data.burst_size);
        lib_data.curr_buf_size -= lib_data.burst_size;
        lib_data.dropped_cnt_from_last_read++;
    }

    memcpy((char*)lib_data.cyclic_buffer + lib_data.write_pointer,
        (const void*)data, data_len);
    advance_pointer(&lib_data.write_pointer, data_len);

    lib_data.curr_buf_size += data_len;

    if (timestamp) {
        wigig_sensing_burst_header* hdr = (wigig_sensing_burst_header*)data;
        lib_data.dri_tsf_arr[hdr->burst_index % lib_data.max_num_bursts_in_buff] = timestamp;
        syslog(LOG_DEBUG, "%s: processing burst_index=%d", __FUNCTION__, hdr->burst_index);
    }

    lib_data.burst_buffer_size = 0;

    syslog(LOG_DEBUG, "curr_buf_size=%d, write_pointer=%d, read_pointer=%d",
           lib_data.curr_buf_size, lib_data.write_pointer, lib_data.read_pointer);

    pthread_mutex_unlock(&data_mutex);

    return WIGIG_SENSING_LIB_NO_ERR;
}

int wigig_sensing_process_data_ready()
{
    /* Check if burst buffer is a whole burst, drop if it is not.
       This could happen if SLPI didn't send the whole burst, or
       burst buffer was cleared before receiving full burst. */
    if (lib_data.burst_buffer_size != lib_data.burst_size) {
        lib_data.burst_buffer_size = 0;
        syslog(LOG_INFO, "Burst received was not a full burst, dropping..");
        return WIGIG_SENSING_LIB_NO_ERR;
    }

    return copy_to_buf(lib_data.burst_timestamp, lib_data.burst_buffer_size, lib_data.burst_buffer);
}

static int copy_from_buf
(
    void        *data_buf,
    uint32_t    data_buf_len,
    uint32_t    *data_buf_recv_len,
    uint32_t    *num_of_bursts,
    uint64_t    *dri_tsf_arr,
    uint32_t    dri_tsf_arr_size
)
{
    pthread_mutex_lock(&data_mutex);

    /* Calculate the number of bursts that can fit into the given data_buf_len.
       Actual number of burts to return is the smaller between
       the bumber of bursts, remaining burts, and the given dri_tsf_arr_size */
    uint32_t max_num_burst_to_return = data_buf_len / lib_data.burst_size;
    uint32_t remaining_bursts = lib_data.curr_buf_size / lib_data.burst_size;
    uint32_t num_burst_to_return = fmin(remaining_bursts, max_num_burst_to_return);
    num_burst_to_return = fmin(num_burst_to_return, dri_tsf_arr_size/sizeof(uint64_t));

   syslog(LOG_DEBUG, "data_buf_len=%d, dri_tsf_arr_size=%d",
         data_buf_len, dri_tsf_arr_size);
   syslog(LOG_DEBUG, "curr_buf_size=%d, write_pointer=%d, read_pointer=%d",
          lib_data.curr_buf_size, lib_data.write_pointer, lib_data.read_pointer);
   syslog(LOG_DEBUG, "max_num_burst_to_return=%d, remaining_bursts=%d, num_burst_to_return=%d",
          max_num_burst_to_return, remaining_bursts, num_burst_to_return);

    *num_of_bursts = 0;
    *data_buf_recv_len = 0;

    for (uint32_t i = 0; i < num_burst_to_return; i++) {

        wigig_sensing_burst_header* hdr = (wigig_sensing_burst_header*)((char*)lib_data.cyclic_buffer + lib_data.read_pointer);
        syslog(LOG_DEBUG, "%s: processing burst_index=%d", __FUNCTION__, hdr->burst_index);

        //copy a burst into the user buffer
        memcpy((char*)data_buf + (i * lib_data.burst_size), hdr, lib_data.burst_size);

        //copy to user given tsf arr
        dri_tsf_arr[i] = lib_data.dri_tsf_arr[hdr->burst_index % lib_data.max_num_bursts_in_buff];

        advance_pointer(&lib_data.read_pointer, lib_data.burst_size);
        lib_data.curr_buf_size -= lib_data.burst_size;

        (*num_of_bursts)++;
        *data_buf_recv_len += lib_data.burst_size;
    }

    pthread_mutex_unlock(&data_mutex);

    return WIGIG_SENSING_LIB_NO_ERR;
}

int wigig_sensing_process_read
(
    void            *data_buf,
    uint32_t        data_buf_len,
    uint32_t        *data_buf_recv_len,
    uint64_t        *dri_tsf_arr,
    uint32_t        dri_tsf_arr_size,
    uint32_t        *num_of_bursts
)
{
    return copy_from_buf(data_buf, data_buf_len, data_buf_recv_len, num_of_bursts, dri_tsf_arr, dri_tsf_arr_size);
}

int wigig_sensing_process_stats
(
    uint32_t        *dropped_cnt_from_last_read,
    uint32_t        *remaining_bursts
)
{
    *remaining_bursts = lib_data.curr_buf_size / lib_data.burst_size;

    *dropped_cnt_from_last_read = lib_data.dropped_cnt_from_last_read;
    lib_data.dropped_cnt_from_last_read = 0;

    return WIGIG_SENSING_LIB_NO_ERR;
}

void wigig_sensing_release_buffers()
{
    pthread_mutex_lock(&data_mutex);

    free(lib_data.cyclic_buffer);

    free(lib_data.dri_tsf_arr);

    free(lib_data.burst_buffer);

    //zero entire lib_data content
    memset(&lib_data, 0, sizeof(lib_data));

    syslog(LOG_INFO, "Released all data buffers");

    pthread_mutex_unlock(&data_mutex);
}

int wigig_sensing_data_mutex_init()
{
    return pthread_mutex_init(&data_mutex, NULL);
}

int wigig_sensing_data_mutex_uninit()
{
    return pthread_mutex_destroy(&data_mutex);
}

int allocate_cyclic_buffer
(
    uint32_t                    burst_size
)
{
    if (!burst_size) {
        syslog(LOG_ERR, "%s: invalid burst size", __FUNCTION__);
        return WIGIG_SENSING_LIB_INVALID_BURST_SIZE;
    }

    /* Allocate cyclic_buffer */
    if (burst_size > CYCLIC_BUF_MAX_SIZE){
        syslog(LOG_ERR, "burst size bigger than max buf size, failed to allocate");
        return WIGIG_SENSING_LIB_INVALID_BURST_SIZE;
    }

    lib_data.max_num_bursts_in_buff = CYCLIC_BUF_MAX_SIZE / burst_size;

    lib_data.cyclic_buffer_size = lib_data.max_num_bursts_in_buff * burst_size;
    lib_data.cyclic_buffer = malloc(lib_data.cyclic_buffer_size + burst_size); //allocate memory for an additional burst

    if (!lib_data.cyclic_buffer) {
        syslog(LOG_ERR, "failed to allocat cyclic_buffer");
        lib_data.cyclic_buffer_size = 0;
        return WIGIG_SENSING_LIB_MEMORY_ERROR;
    }

    memset(lib_data.cyclic_buffer, 0, lib_data.cyclic_buffer_size);

    /*Allocate dri_tsf_arr*/
    lib_data.tsf_arr_size = lib_data.max_num_bursts_in_buff * sizeof(uint64_t);
    lib_data.dri_tsf_arr = malloc(lib_data.tsf_arr_size + sizeof(uint64_t)); //allocate memory for an additional burst

    if (!lib_data.dri_tsf_arr) {
        syslog(LOG_ERR, "failed to allocat dri_tsf_arr");
        wigig_sensing_release_buffers();
        return WIGIG_SENSING_LIB_MEMORY_ERROR;
    }

    memset(lib_data.dri_tsf_arr, 0, lib_data.tsf_arr_size);

    lib_data.burst_buffer = malloc(burst_size);

    if (!lib_data.burst_buffer) {
        syslog(LOG_ERR, "failed to allocat burst_buffer");
        wigig_sensing_release_buffers();
        return WIGIG_SENSING_LIB_MEMORY_ERROR;
    }

    memset(lib_data.burst_buffer, 0, burst_size);

    lib_data.burst_size = burst_size;

    return WIGIG_SENSING_LIB_NO_ERR;
}

inline void wigig_sensing_lib_get_avail_bursts_slpi
(
    uint32_t    *num_avail_cirs
)
{
    *num_avail_cirs = lib_data.curr_buf_size / lib_data.burst_size;
}

inline void wigig_sensing_set_burst_size
(
    uint32_t    burst_size
)
{
    lib_data.burst_size = burst_size;
}

wigig_sensing_lib_error_type
wigig_sensing_lib_clear_data_slpi()
{
    pthread_mutex_lock(&data_mutex);

    /* reset read/write pointers and relevent sizes to reflect clear data */
    lib_data.burst_buffer_size = 0;
    lib_data.curr_buf_size = 0;
    lib_data.read_pointer = 0;
    lib_data.write_pointer = 0;

    syslog(LOG_INFO, "Clear all previous data");

    pthread_mutex_unlock(&data_mutex);

    return WIGIG_SENSING_LIB_NO_ERR;
}

uint32_t wigig_sensing_get_buffer_size()
{
    return lib_data.curr_buf_size;
}