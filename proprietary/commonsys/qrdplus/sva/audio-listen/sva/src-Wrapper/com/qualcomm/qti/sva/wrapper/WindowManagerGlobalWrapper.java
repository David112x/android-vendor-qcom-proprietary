/*
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.sva.wrapper;

import android.os.Build;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class WindowManagerGlobalWrapper {

    public void dismissKeyguard() {
        try {
            Class windowManagerGlobal = Class.forName("android.view.WindowManagerGlobal");
            Method getWindowManagerService = windowManagerGlobal.
                    getMethod("getWindowManagerService");
            Object wmInstance = getWindowManagerService.invoke(windowManagerGlobal);
            if (wmInstance != null){
                Class IWindowManager = Class.forName("android.view.IWindowManager");
                if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O){
                    Method unlockMethod = IWindowManager.getMethod("dismissKeyguard");
                    unlockMethod.invoke(wmInstance);
                } else if (Build.VERSION.SDK_INT < Build.VERSION_CODES.P) {
                    Class callback
                        = Class.forName("com.android.internal.policy.IKeyguardDismissCallback");
                    Method unlockMethod = IWindowManager.getMethod("dismissKeyguard", callback);
                    Object param = null;
                    unlockMethod.invoke(wmInstance, param);
                } else {
                    Class callback
                        = Class.forName("com.android.internal.policy.IKeyguardDismissCallback");
                    Method unlockMethod = IWindowManager.getMethod("dismissKeyguard", callback,
                        CharSequence.class);
                    Object callbackParam = null;
                    Object messageParam = null;
                    unlockMethod.invoke(wmInstance, callbackParam, messageParam);
                }
            }
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        } catch (SecurityException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        }
    }
}
