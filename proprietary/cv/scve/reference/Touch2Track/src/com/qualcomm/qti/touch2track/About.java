/*==============================================================================
 * Copyright (c) 2010-2011 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
==============================================================================*/

package com.qualcomm.qti.touch2track;

import com.qualcomm.qti.touch2track.R;

import android.os.Bundle;
import android.preference.PreferenceActivity;

/**
 * Basic empty About class. All configurations for
 * About menu is done in the xml file.
 *
 */
public class About extends PreferenceActivity
{
   /** Called when the activity is first created. */
   @Override
   protected void onCreate( Bundle savedInstanceState )
   {
      super.onCreate(savedInstanceState);
      addPreferencesFromResource( R.layout.about );
   }
}

