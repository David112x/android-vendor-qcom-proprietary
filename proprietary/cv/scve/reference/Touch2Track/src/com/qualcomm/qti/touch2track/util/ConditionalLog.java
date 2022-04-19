/*==============================================================================
Copyright (c) 2010-2011, 2018 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
==============================================================================*/
package com.qualcomm.qti.touch2track.util;

//for logging
public class ConditionalLog {
 static final boolean LOG_ENABLED = true; // set this to false to stop
            // logging

 public static void d(String tag, String text) {
  if (LOG_ENABLED) {
   android.util.Log.d(tag, text);
  }
 }

 public static void e(String tag, String text) {
  if (LOG_ENABLED) {
   android.util.Log.e(tag, text);
  }
 }
}
// end
