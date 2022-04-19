/* st_buffering.h
 *
 * This file contains a sound trigger buffer abstraction. This
 * abstraction represents a single circular buffer that is shared
 * among various threads in the sound trigger HAL.
 *
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#ifndef ST_BUFFERING_H
#define ST_BUFFERING_H

#include <stdint.h>
#include <pthread.h>

typedef struct st_buffer {
    pthread_mutex_t lock;
    uint8_t *buf_start;
    uint8_t *buf_end;
    uint8_t *rd_ptr;
    uint8_t *wr_ptr;
    uint32_t size;
    uint32_t unread_bytes;
}st_buffer_t;

st_buffer_t* st_buffer_init(uint32_t size);
void st_buffer_deinit(st_buffer_t *buf_obj);
void st_buffer_reset(st_buffer_t *buf_obj);
uint8_t* st_buffer_get_wr_ptr(st_buffer_t *buf_obj);
int st_buffer_write(st_buffer_t *buf_obj, uint8_t *src, uint32_t size);
int st_buffer_read(st_buffer_t *buf_obj, uint8_t *dst, uint32_t size,
    uint8_t **rd_ptr, bool flush);
void st_buffer_flush(st_buffer_t *buf_obj, uint32_t size);

#endif /* ST_BUFFERING_H */
