/***************************************************************************
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ***************************************************************************/
#ifndef __EZTUNE_LOGS_H__
#define __EZTUNE_LOGS_H__

#ifdef _ANDROID_
#include <log/log.h>
#endif

#undef EZLOGE
#undef EZLOGW
#undef EZLOGI
#undef EZLOGV
#undef EZLOGB
#undef EZ_ASSERT

#ifdef _ANDROID_

#undef LOG_NIDEBUG
#define LOG_NIDEBUG 0

#undef LOG_TAG
#define LOG_TAG "mm-camera-eztune-lookup"

#define EZLOGE(fmt, ...) ALOGE("%s(%d): " #fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define EZLOGW(fmt, ...) ALOGW("%s(%d): " #fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define EZLOGI(fmt, ...) ALOGI("%s(%d): " #fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define EZLOGV(fmt, ...) ALOGV("%s(%d): " #fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define EZLOGB(fmt, ...) ALOGV(fmt ,##__VA_ARGS__)

#else

#define KMAG  "\x1B[35m"
#define KRED  "\x1B[31m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KWHT  "\x1B[37m"

#define EZLOGE(fmt, ...) printf("%s%s(%d): " #fmt "\n", KRED, __FILE__, __LINE__, ##__VA_ARGS__)
#define EZLOGW(fmt, ...) printf("%s%s(%d): " #fmt "\n", KMAG, __FILE__, __LINE__, ##__VA_ARGS__)
#define EZLOGI(fmt, ...) printf("%s%s(%d): " #fmt "\n", KYEL, __FILE__, __LINE__, ##__VA_ARGS__)
#define EZLOGV(fmt, ...) printf("%s%s(%d): " #fmt "\n", KBLU, __FILE__, __LINE__, ##__VA_ARGS__)
#define EZLOGB(fmt, ...) printf("%s" fmt, KWHT, ##__VA_ARGS__)

#endif //_ANDROID_

#define EZ_ASSERT(TST, fmt, ...) do { \
  if(!(TST)) { \
    EZLOGE(Assert Failed: #fmt, ##__VA_ARGS__); \
    abort(); \
    } \
  } while(0)

#endif  /* __EZTUNE_LOGS_H__ */
