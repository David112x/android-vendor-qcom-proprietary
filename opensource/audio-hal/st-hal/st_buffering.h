/* st_buffering.h
 *
 * This file contains a sound trigger buffer abstraction. This
 * abstraction represents a single circular buffer that is shared
 * among various threads in the sound trigger HAL.
 *
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
