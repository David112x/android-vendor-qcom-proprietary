/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

package com.qualcomm.qti;

interface IPerfManager {
    int perfLockRelease();
    int perfLockReleaseHandler(int handle);
    int perfHint(int hint, String userDataStr, int userData1, int userData2, int tid);
    int perfLockAcquire(int duration, in int[] list);
    int perfUXEngine_events(int opcode, int pid, String pkg_name, int lat);
    int setClientBinder(IBinder client);
    String perfGetProp(String prop_name, String def_val);
}
