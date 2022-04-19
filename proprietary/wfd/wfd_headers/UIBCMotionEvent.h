/*=====================================================================
*  @file UIBCMotionEvent.h
*
*  @par  DESCRIPTION:
*        UIBC Motion events Data structure definitions.
*
*
*
*  Copyright (c) 2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
=====================================================================*/

#ifndef UIBC_MOTIONEVENT_H
#define UIBC_MOTIONEVENT_H

#include<input/Input.h>

struct uibc_Fields{
    nsecs_t downTime;
    android::MotionEvent *prevEv;
    bool multiMode;
    android::PointerCoords* prevPtrCoords;
    android::PointerProperties* prevPtrProp;
    //Stores What UIBC Modes were negotiated
    int nUibcInfo;
    uibc_Fields() {
        ALOGE("uibc_Fields_t ctor()");
        downTime = 0;
        prevEv = new android::MotionEvent();
        multiMode = false;
        prevPtrCoords = new android::PointerCoords[UIBC_MAX_TOUCH_INPUTS];
        prevPtrProp = new android::PointerProperties[UIBC_MAX_TOUCH_INPUTS];
        nUibcInfo = 0xFFFF;
    }
    ~uibc_Fields() {
        ALOGE("uibc_Fields_t dtor()");
        if(prevEv) {
           delete prevEv;
        }
        if(prevPtrCoords) {
          delete[] prevPtrCoords;
        }
        if(prevPtrProp) {
          delete[] prevPtrProp;
        }
    }
};
#endif
