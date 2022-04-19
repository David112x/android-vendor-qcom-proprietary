/*============================================================================
@file CommandReceiver.java

@brief
Receives and processes broadcast commands.

Copyright (c) 2012-2017 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/
package com.qualcomm.qti.sensors.core.qsensortest;

import java.util.List;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.Sensor;
import android.util.Log;
import android.widget.Toast;

import com.qualcomm.qti.sensors.core.stream.SensorAdapter;
import com.qualcomm.qti.sensors.core.sensortest.SensorLowLat;

public class CommandReceiver extends BroadcastReceiver {
  public CommandReceiver() {}

  private static final String TAG = QSensorContext.TAG;

  @Override
  public void onReceive(Context context, Intent intent) {
    if(intent.getAction().contentEquals("com.qualcomm.qti.sensors.qsensortest.intent.STREAM")) {
      CharSequence sensorName = intent.getCharSequenceExtra("sensorname");
      int sensorType = intent.getIntExtra("sensortype", 0);
      int rate = intent.getIntExtra("rate", 60);
      int batchRate = intent.getIntExtra("batch", -1);
      boolean isSensorFound = false;

      Log.i(TAG, "received stream intent with parameters: (sensorname: " + sensorName + ", sensortype: " + sensorType + ", rate: " + rate + " batch: " + batchRate + ")");

      List<SensorAdapter> sensorAdapters = QSensorContext.sensorAdapterList();
      for(SensorAdapter adapter : sensorAdapters) {
        Sensor sensor = adapter.sensor();
        if(sensor.getName().contentEquals(sensorName) &&
           sensor.getType() == sensorType &&
           SettingsDatabase.getSettings().getSensorSetting(sensor).getEnableStreaming()) {
          isSensorFound = true;
          Log.d(QSensorContext.TAG, "CR setting stream rate for " + sensorName + ": " + rate + ", batch:" + batchRate);
          adapter.streamRateIs(rate, batchRate, true);
          break;
        }
      }
      if (isSensorFound == false) {
          Log.e(QSensorContext.TAG, "improper command for STREAM, Mismatch of sensorname or sensortype , please check name and type in available sensor list ");
      }
    }
    if ( intent.getAction().contentEquals("com.qualcomm.qti.sensors.qsensortest.intent.LOW_LAT")) {
      int  Enabled = intent.getIntExtra("Enabled", 0);
      Log.i(TAG,"Enabled = "+Enabled);
      if ( 1 == Enabled ) {
          SensorLowLat.enable();
          Toast.makeText(context,"Enabled Successfully",Toast.LENGTH_SHORT).show();
      } else {
          SensorLowLat.disable();
          Toast.makeText(context,"Disabled Successfully",Toast.LENGTH_SHORT).show();
      }
    }
    if (intent.getAction().contentEquals("com.qualcomm.qti.sensors.qsensortest.intent.SENSORLIST")) {
        List<SensorAdapter> sensorAdapters = QSensorContext.sensorAdapterList();
        for (SensorAdapter adapter : sensorAdapters) {
            Sensor sensor = adapter.sensor();
            Log.d(QSensorContext.TAG,"< "+sensor.getStringType()+" > " +" Sensor type:" + "\"" +sensor.getType() + "\"" + " SensorName:" + "\""+sensor.getName()+"\"" );
        }
    }
  }

  public IntentFilter getIntentFilter() {
    IntentFilter intentFilter = new IntentFilter();
    intentFilter.addAction("com.qualcomm.qti.sensors.qsensortest.intent.STREAM");
    Log.d(QSensorContext.TAG, "Available: com.qualcomm.qti.sensors.qsensortest.intent.STREAM");
    intentFilter.addAction("com.qualcomm.qti.sensors.qsensortest.intent.LOW_LAT");
    Log.d(QSensorContext.TAG, "Available: com.qualcomm.qti.sensors.qsensortest.intent.LOW_LAT");
    intentFilter.addAction("com.qualcomm.qti.sensors.qsensortest.intent.SENSORLIST");
    Log.d(QSensorContext.TAG,"Available: com.qualcomm.qti.sensors.qsensortest.intent.SENSORLIST");
    intentFilter.addCategory("com.qualcomm.qti.sensors.qsensortest.intent.category.DEFAULT");
    return intentFilter;
  }
}
