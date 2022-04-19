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
package com.qualcomm.qti.sensors.ui.low_lat;

import android.app.Activity;
import android.app.Fragment;
import android.content.Context;
import android.graphics.Color;
import android.os.Bundle;
import android.widget.Button;
import android.util.Log;
import android.view.View.OnClickListener;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.qualcomm.qti.sensors.qsensortest.R;
import com.qualcomm.qti.sensors.core.sensortest.SensorLowLat;

public class SensorLowLatActivity extends Fragment {

  // Tag to use when logging messages to the android device (viewable through logcat).
  private static final String TAG = "SensorLowLatActivity";

  private Color c = new Color();
  private TextView textCurActivity;
  private View view;
  private Button buttonEnable, buttonDisable;
  String completeData = new String();  // Activity history managed by one string.
  String values = new String(); // current activity data

  @Override
  public View onCreateView (LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    if (null == this.view) {
      this.view = inflater.inflate(R.layout.activity_attributes_layout, null);
      this.view = inflater.inflate(R.layout.low_lat_layout, null);
      textCurActivity = (TextView) view.findViewById(R.id.text_curVrActivity); // current activity data display text
      buttonEnable = (Button)view.findViewById(R.id.low_lat_enable);
      buttonDisable = (Button)view.findViewById(R.id.low_lat_disable);

      buttonEnable.setOnClickListener(new OnClickListener() {
        @Override
        public void onClick(View view) {
            SensorLowLat.enable();
            Toast.makeText(getActivity(),"Enabled Successfully",Toast.LENGTH_SHORT).show();
            values = "Low Latency Path Enabled";
            textCurActivity.setText(values);
        }
      });
      buttonDisable.setOnClickListener(new OnClickListener(){
        @Override
        public void onClick(View view) {
            SensorLowLat.disable();
            Toast.makeText(getActivity(),"Disabled Successfully",Toast.LENGTH_SHORT).show();
            values = "Low Latency Path Disabled";
            textCurActivity.setText(values);
        }
      });
    }
    return this.view;
  }

  @Override
  public void onDestroy() {
    SensorLowLat.disable();
  }
}
