/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __GPQESE_UTILS_H_
#define __GPQESE_UTILS_H_
#endif
#define LOG_TAG "GPQeSE-HAL"
#define DBG true

#include <string>
#include <cstdlib>
#include <cstdint>
#include <utils/Log.h>


bool memsApducmp(const uint8_t *Apdu1, size_t Apdu1length, const uint8_t *Apdu2,
              size_t Apdu2length);
size_t memscpy(void *dst, size_t dst_size, const void *src, size_t src_size);
void print_text(std::string intro_message, unsigned const char *text_addr,
                int size);
