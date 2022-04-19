/**********************************************************************
*  Copyright (c) 2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

void mDummyProc();

enum {
    RET_OK = 0,
    RET_NG = -1,
};

void mfpdummy_init();
int32_t mfpdummy_process();
void mfpdummy_deinit();
