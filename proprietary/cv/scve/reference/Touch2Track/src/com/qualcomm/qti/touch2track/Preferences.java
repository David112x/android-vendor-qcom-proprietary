/*==============================================================================
 * Copyright (c) 2010-2011 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
==============================================================================*/

package com.qualcomm.qti.touch2track;

import java.util.List;

import com.qualcomm.qti.touch2track.R;

import android.hardware.Camera;
import android.hardware.Camera.Size;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;
import android.util.Log;

/**
 * Basic empty Preferences class. All configurations for Setting menu is done in
 * the xml file.
 *
 */
public class Preferences extends PreferenceActivity
{
   private static final String TAG = "PreferenceActivity";

   /** Called when the activity is first created. */
   @Override
   protected void onCreate(Bundle savedInstanceState) {
      super.onCreate(savedInstanceState);
      addPreferencesFromResource(R.layout.preferences);

      // For preview resolution
      Camera cam = Camera.open();
      if (cam != null)
      {
         Camera.Parameters pm = cam.getParameters();
         cam.release();
         List<Size> mSupportedPreviewSize = pm.getSupportedPreviewSizes();

         // for preview resolution - this loop eliminates any preview size
         // greater than 1920 X 1080
         for (int i = mSupportedPreviewSize.size() - 1; i >= 0; i--)
         {
            Size temp = mSupportedPreviewSize.get(i);
            if (temp.width * temp.height > (1920 * 1080))
            {
               mSupportedPreviewSize.remove(i);
            }
         }

         CharSequence[] entries = new String[mSupportedPreviewSize.size()];
         for (int i = 0; i < mSupportedPreviewSize.size(); i++)
         {
            Size temp = mSupportedPreviewSize.get(i);
            entries[i] = temp.width + "_" + temp.height;
         }

         PreferenceScreen root = getPreferenceScreen();
         PreferenceCategory resCat = new PreferenceCategory(this);
         resCat.setTitle("Preview Resolution");
         root.addPreference(resCat); // Adding a category
         // List preference under the category
         ListPreference listPref = new ListPreference(this);
         listPref.setKey("resolution"); // Refer to get the pref value
         listPref.setEntries(entries);
         listPref.setEntryValues(entries);
         listPref.setDialogTitle("Select preview resolution");
         listPref.setTitle("Preview resolution");
         listPref.setSummary("Preview resolution");
         resCat.addPreference(listPref);
      }
      else
      {
         Log.e(TAG, "Camera object not obained");
      }
   }
}
