/*******************************************************************************
*
* Copyright (c) 2014, 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*
*******************************************************************************/

#ifdef _ANDROID_
#include <log/log.h>
#endif

#include "mmcam_log_utils.h"
#include <string.h>
#include <stdlib.h>

namespace mmcam_utils
{

void PrintByteStream(const char *str, size_t size, const char *comment)
{
    MMCAM_LOGV("%s Size: %zu", comment, size);

    for (size_t i = 0; i < size; i++) {
        MMCAM_LOGB("%hhu ", str[i]);
    }

    MMCAM_LOGB("\n\n");
}

string::string(const char* str)
{
    MMCAM_ASSERT(str != NULL, "error: invalid string request");

    buf_capacity = strlen(str) + 1;
    buf = (char*)malloc(buf_capacity);
    MMCAM_ASSERT(buf != NULL, "error: allocation fail");

    buf_len = 0;
    this->append(str, strlen(str));
}

string::string(int32_t capacity)
{
    MMCAM_ASSERT(capacity != 0, "error: invalid string request");

    buf = (char*)malloc(capacity);
    MMCAM_ASSERT(buf != NULL, "error: allocation fail");

    buf_capacity = capacity;
    buf_len = 0;
    buf[buf_len] = '\0';
}

string::~string()
{
    free(buf);
}

size_t string::length() const
{
    return buf_len;
}

size_t string::size() const
{
    return buf_len;
}

size_t string::capacity() const
{
    return buf_capacity;
}

const char* string::data() const
{
    return buf;
}

char& string::operator[] (size_t pos)
{
    MMCAM_ASSERT(pos < capacity(), "error: invalid string request");

    return buf[pos];
}

void string::clear()
{
    buf_len = 0;
    buf[buf_len] = '\0';
}

string& string::append(const string& str)
{
    return append(str.data(), str.length());
}

string& string::append(const char* str, size_t len)
{
    MMCAM_ASSERT(!(UINT_MAX - len < buf_len), "error: Overflow");
    //try to resize the buffer, if needed
    if (buf_len + len >= buf_capacity) {
        char *buf_temp = NULL;
        if (0 == buf_len) {
            buf_temp = (char*)malloc(len + 1);
            MMCAM_ASSERT(buf_temp != NULL, "error: allocation fail");
            buf_capacity = len + 1;
        } else {
            buf_temp = (char*)malloc(buf_len + len + 1);
            MMCAM_ASSERT(buf_temp != NULL, "error: allocation fail");
            memcpy(buf_temp, buf, buf_len);
            buf_capacity = buf_len + len + 1;
        }
        free(buf);
        buf = buf_temp;
        buf_temp = NULL;
    }

    MMCAM_ASSERT(buf != NULL, "error: allocation fail");
    memcpy(&buf[buf_len], str, len);
    buf_len += len;
    buf[buf_len] = '\0';

    return *this;
}

string& string::append(size_t len, char val)
{
    MMCAM_ASSERT(!(UINT_MAX - len < buf_len), "error: Overflow");
    //try to resize the buffer, if needed
    if (buf_len + len >= buf_capacity) {
        char *buf_temp = NULL;
        if (0 == buf_len) {
            buf_temp = (char*)malloc(len + 1);
            MMCAM_ASSERT(buf_temp != NULL, "error: allocation fail");
            buf_capacity = len + 1;
        } else {
            buf_temp = (char*)malloc(buf_len + len + 1);
            MMCAM_ASSERT(buf_temp != NULL, "error: allocation fail");
            memcpy(buf_temp, buf, buf_len);
            buf_capacity = buf_len + len + 1;
        }
        free(buf);
        buf = buf_temp;
        buf_temp = NULL;
    }

    MMCAM_ASSERT(buf != NULL, "error: allocation fail");
    memset(&buf[buf_len], val, len);
    buf_len += len;
    buf[buf_len] = '\0';
    return *this;
}

string& string::assign(const string& str)
{
    return assign(str.data(), str.size());
}

string& string::assign(const char* str, size_t len)
{
    clear();
    append(str, len);
    return *this;
}

string& string::replace(size_t pos, size_t len, const string& str)
{
    return this->replace(pos, len, str.data());
}

string& string::replace(size_t pos, size_t len, const char *str)
{
    MMCAM_ASSERT(pos < size(), "error: arg out of range");

    size_t copy_len;
    if(pos + len <= buf_len)
        copy_len = len;
    else
        copy_len = buf_len - pos;

    memcpy(&buf[pos], str, copy_len);
    return *this;
}

};
