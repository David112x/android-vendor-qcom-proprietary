/*============================================================================
@file SensorLowLat.java

@brief
Java Native interface to enable the Sensors (SNS) Low Latency stream.

@attention
This is the JNI interface file for SNS Low Latency stream.

Copyright (c) 2017 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/
package com.qualcomm.qti.sensors.core.sensortest;

import java.io.IOException;
import android.util.Log;
import com.qualcomm.qti.sensors.qsensortest.R;

public class SensorLowLat{

  // Default parameters
  private static final int defaultCircBufferSize = 0x1000;
  // The default sensors are accel and gyro
  private static final long[] defaultSensorSuid = {0x4b118d0883777081L, 0x35bcdb5f5574ab55L};
  private static final int defaultSamplePeriodUs = 1250; // 800 Hz
  private static final int defaultFlags = 0;

  // Tag to use when logging messages to the android device (viewable through logcat).
  private static final String TAG = "SensorLowLat";

  // Handle used to identify this low-lat client with the sns_low_lat_stream
  // library. This is set by the library within the "init" call.
  private static int handle;
  // True if the stream is active, false otherwise.
  private static boolean isActive = false;

  /**
   * Load the native functions.  The library will be named on the file system
   * "libsensor_low_lat.so", and must be pushed to the device before
   * use (to system/lib, or [app_data]/lib).
   */
  static {
    System.loadLibrary("sensor_low_lat");
  }

  /**
   * Initializes the stream state with the framework.
   *
   * @param bufferSize The desired size of the circular buffer for the output samples.
   * @throws
   */
  public native static void init(int bufferSize);

  public static void init() {
    try {
      init(defaultCircBufferSize);
    } catch (IllegalArgumentException e) {
      Log.e(TAG, e.getMessage());
      e.printStackTrace();
    } catch (IllegalStateException e) {
      Log.e(TAG, e.getMessage());
      e.printStackTrace();
    } catch (NullPointerException e) {
      Log.e(TAG, e.getMessage());
      e.printStackTrace();
    }
  }

  /**
   * Starts the stream with the specified sensor(s) at the specified rate.
   *
   * @param sensorSuid The sensor(s) that the user wishes to stream, identified by its SUID. See sns_smgr_common_v01.h.
   * @param samplePeriodUs The sampling period for the sensor(s) in microseconds.
   * @param flags Bitmask used to specify various stream options. See sns_low_lat.h.
   * @throws
   */
  public native static void start(long[] sensorSuid, int samplePeriodUs, int flags);

  public static void start() {
    if (!isActive)
    {
      try {
        Log.i(TAG, "Starting low latency stream. Handle " + handle);
        start(defaultSensorSuid, defaultSamplePeriodUs, defaultFlags);
        isActive = true;
      } catch (IllegalArgumentException e) {
        Log.e(TAG, e.getMessage());
        e.printStackTrace();
      } catch (IllegalStateException e) {
        Log.e(TAG, e.getMessage());
        e.printStackTrace();
      }
    }
  }

  /**
   * Stops the stream.
   *
   * @throws
   */
  public native static void stop();

  /**
   * Destroys the stream state with the framework.
   *
   * @throws
   */
  public native static void deinit();

  public static void enable() {
    if (!isActive)
    {
      Log.i(TAG, "Setting up the low latency stream");
      init();
      Log.i(TAG, "Starting low latency stream. Handle " + handle);
      start();
    }
    else
    {
      Log.e(TAG, "Cannot enable low lat stream. It is already enabled. Handle " + handle);
    }
  }

  public static void disable() {
    if (isActive)
    {
      Log.i(TAG, "Stopping low latency stream. Handle " + handle);
      isActive = false;

      try {
        stop();
      } catch (IllegalArgumentException e) {
        Log.e(TAG, e.getMessage());
        e.printStackTrace();
      } catch (IllegalStateException e) {
        Log.e(TAG, e.getMessage());
        e.printStackTrace();
      }

      try {
        deinit();
      } catch (IllegalArgumentException e) {
        Log.e(TAG, e.getMessage());
        e.printStackTrace();
      } catch (IllegalStateException e) {
        Log.e(TAG, e.getMessage());
        e.printStackTrace();
      }
    }
    else
    {
      Log.e(TAG, "Cannot disable low lat stream. There is no stream to disable.");
    }
  }
}
