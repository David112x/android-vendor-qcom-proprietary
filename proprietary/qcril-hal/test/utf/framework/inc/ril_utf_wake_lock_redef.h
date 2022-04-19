/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_wake_lock_redef.h
  @brief   RIL UTF local definition for wake lock

  DESCRIPTION
  to eliminate external file dep
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef _RIL_UTF_WAKE_LOCK_REDEF_H
#define _RIL_UTF_WAKE_LOCK_REDEF_H

enum {
    PARTIAL_WAKE_LOCK = 1,  // the cpu stays on, but the screen is off
    FULL_WAKE_LOCK = 2      // the screen is also on
};

// while you have a lock held, the device will stay on at least at the
// level you request.
int acquire_wake_lock(int lock, const char* id);
int release_wake_lock(const char* id);

#endif //_RIL_UTF_WAKE_LOCK_REDEF_H
