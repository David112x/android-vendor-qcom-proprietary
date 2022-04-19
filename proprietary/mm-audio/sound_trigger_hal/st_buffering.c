/*
 * This file contrains the functions for controlling the
 * internal circular buffer within the sound trigger HAL.
 * The purpose is to keep all the different read and write
 * commands from different threads synchronized.
 *
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#define LOG_TAG "sound_trigger_hw"
#define ATRACE_TAG (ATRACE_TAG_HAL)
/* #define LOG_NDEBUG 0 */
#define LOG_NDDEBUG 0

#include <errno.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/prctl.h>
#include <sys/ioctl.h>
#include <cutils/log.h>
#include <cutils/atomic.h>
#include <cutils/trace.h>
#include <system/thread_defs.h>
#include <hardware/sound_trigger.h>

#include "st_buffering.h"
#include "st_hw_common.h"

static inline uint8_t* circ_buff_move_ptr_fwd(uint8_t *circ_buff_start,
    uint8_t *ptr, uint32_t bytes_num, uint32_t size)
{
    return ((ptr + bytes_num) < (circ_buff_start + size)) ?
           (ptr + bytes_num) : (ptr + bytes_num - size);
}

st_buffer_t* st_buffer_init(uint32_t size)
{
    int status = 0;
    st_buffer_t *buf_obj = NULL;

    buf_obj = calloc(1, sizeof(st_buffer_t));
    if (!buf_obj) {
        ALOGE("%s: failed to allocate buf_obj", __func__);
        status = -ENOMEM;
        goto exit;
    }

    buf_obj->size = size;

    buf_obj->buf_start = calloc(1, size);
    if (!buf_obj->buf_start) {
        ALOGE("%s: failed to allocate buf_obj->buf_start", __func__);
        status = -ENOMEM;
        goto exit;
    }

    pthread_mutex_init(&buf_obj->lock, NULL);
    st_buffer_reset(buf_obj);

exit:

    if (status && buf_obj) {
        free(buf_obj);
        buf_obj = NULL;
    }

    return buf_obj;
}

void st_buffer_deinit(st_buffer_t *buf_obj)
{
    if (buf_obj) {
        if (buf_obj->buf_start) {
            free(buf_obj->buf_start);
            buf_obj->buf_start = NULL;
        }
        pthread_mutex_destroy(&buf_obj->lock);
        free(buf_obj);
    }
}

void st_buffer_reset(st_buffer_t *buf_obj)
{
    pthread_mutex_lock(&buf_obj->lock);
    buf_obj->buf_end = buf_obj->buf_start + buf_obj->size;
    buf_obj->wr_ptr = buf_obj->buf_start;
    buf_obj->rd_ptr = buf_obj->buf_start;
    buf_obj->unread_bytes = 0;
    pthread_mutex_unlock(&buf_obj->lock);
}

uint8_t* st_buffer_get_wr_ptr(st_buffer_t *buf_obj)
{
    return buf_obj->wr_ptr;
}

int st_buffer_write(st_buffer_t *buf_obj, uint8_t *src, uint32_t size)
{
    uint32_t bytes_to_tail = 0, free_bytes = 0;
    int status = 0;

    pthread_mutex_lock(&buf_obj->lock);
    free_bytes = buf_obj->size - buf_obj->unread_bytes;
    bytes_to_tail = buf_obj->buf_end - buf_obj->wr_ptr;
    if (bytes_to_tail < size) {
        memcpy(buf_obj->wr_ptr, src, bytes_to_tail);
        memcpy(buf_obj->buf_start, src + bytes_to_tail,
            (size - bytes_to_tail));
    } else {
        memcpy(buf_obj->wr_ptr, src, size);
    }

    buf_obj->wr_ptr = circ_buff_move_ptr_fwd(buf_obj->buf_start,
        buf_obj->wr_ptr, size, buf_obj->size);

    if (free_bytes < size) {
        ALOGW("%s: buffer overflow: buf_size %d, free_bytes %d",
            __func__, size, free_bytes);
        buf_obj->rd_ptr = buf_obj->wr_ptr;
        buf_obj->unread_bytes = buf_obj->size;
        status = -EOVERFLOW;
    } else {
        buf_obj->unread_bytes += size;
    }
    pthread_mutex_unlock(&buf_obj->lock);

    return status;
}

int st_buffer_read(st_buffer_t *buf_obj, uint8_t *dst, uint32_t size,
    uint8_t **rd_ptr, bool flush)
{
    uint32_t bytes_to_tail = 0;

    pthread_mutex_lock(&buf_obj->lock);
    /*
     * There are 2 types of clients accessing the buffer:
     *
     *    1. Read Only - These clients pass in their own rd_ptr. When reading,
     *       the data is not flushed from the buffer. Second stage sessions
     *       are an example of this type of client.
     *
     *    2. Read and Flush - These clients do not pass in their own rd_ptr
     *       because they can use buf_obj->rd_ptr. When reading, the data is
     *       also flushed from the buffer. App clients are an example of this
     *       type of client.
     */
    if (!rd_ptr) {
        if (flush) {
            rd_ptr = &buf_obj->rd_ptr;
        } else {
            ALOGE("%s: Error. Need to pass in rd_ptr when not flushing",
                __func__);
            pthread_mutex_unlock(&buf_obj->lock);
            return -EINVAL;
        }
    } else if (flush) {
        ALOGE("%s: Error. Cannot pass in rd_ptr and flush",
            __func__);
        pthread_mutex_unlock(&buf_obj->lock);
        return -EINVAL;
    }

    if (buf_obj->buf_end < (*rd_ptr + size)) {
        bytes_to_tail = buf_obj->buf_end - *rd_ptr;
        memcpy(dst, *rd_ptr, bytes_to_tail);
        memcpy(dst + bytes_to_tail, buf_obj->buf_start,
            size - bytes_to_tail);
    } else {
        memcpy(dst, *rd_ptr, size);
    }

    *rd_ptr = circ_buff_move_ptr_fwd(buf_obj->buf_start,
        *rd_ptr, size, buf_obj->size);

    /* Allow threads to read without removing the data from the buffer. */
    if (flush)
        buf_obj->unread_bytes -= size;
    pthread_mutex_unlock(&buf_obj->lock);

    return 0;
}

void st_buffer_flush(st_buffer_t *buf_obj, uint32_t size)
{
    pthread_mutex_lock(&buf_obj->lock);
    buf_obj->rd_ptr = circ_buff_move_ptr_fwd(buf_obj->buf_start,
        buf_obj->rd_ptr, size, buf_obj->size);
    buf_obj->unread_bytes -= size;
    pthread_mutex_unlock(&buf_obj->lock);
}
