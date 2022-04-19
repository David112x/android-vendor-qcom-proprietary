/*==============================================================================
 * Copyright (c) 2010-2011 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
==============================================================================*/

package com.qualcomm.qti.touch2track;

import com.qualcomm.qti.touch2track.R;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.Display;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.WindowManager;
import android.util.Log;
import android.content.res.Configuration;


/** The splash screen activity for FastCV sample application */
public class SplashScreen extends Activity
{
    private static final String         TAG               = "FastCVSample";
    private WindowManager               mWindowManager;
    private SplashScreenView            mHomeView;
    public static Display               sDisplay;
    private int counter = 0;//for print screen

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mWindowManager = (WindowManager) getSystemService(WINDOW_SERVICE);
        sDisplay = mWindowManager.getDefaultDisplay();

        // Initialize UI
        mHomeView = new SplashScreenView(this);
        setContentView( mHomeView );
    }

    @Override
    protected void onPause()
    {
       super.onPause();
    }
    /** Called when the option menu is created. */
    @Override
    public boolean onCreateOptionsMenu( Menu menu )
    {
       MenuInflater inflater = getMenuInflater();
       inflater.inflate( R.menu.splashmenu, menu );
       return true;
    }


    @Override
    public void openOptionsMenu()
    {
        Configuration config = getResources().getConfiguration();
        if((config.screenLayout & Configuration.SCREENLAYOUT_SIZE_MASK) > Configuration.SCREENLAYOUT_SIZE_LARGE)
        {
            int originalScreenLayout = config.screenLayout;
            config.screenLayout = Configuration.SCREENLAYOUT_SIZE_LARGE;
            super.openOptionsMenu();
            config.screenLayout = originalScreenLayout;
        }
        else
        {
            super.openOptionsMenu();
        }
    }

     /** User Option selection menu
     */
    @Override
    public boolean onOptionsItemSelected( MenuItem item )
    {
       // Handle item selection
       switch( item.getItemId() )
       {
       case R.id.main_start:
           Intent startActivity = new Intent(getBaseContext(), FastCVSample.class);
           //startActivity.putExtra("max_prints", "100");  //max print count format
           //startActivity.putExtra("resolution", "200_300"); //preview resolution format
           //startActivity.putExtra("folder_name", "snaps_"+counter);  //snapshot folder name.
           //startActivity.putExtra("T2TMode", "0"); //to set appropriate T2T mode.
           startActivity( startActivity );
           counter++;

           return true;

       case R.id.settings:
           Intent settingsActivity = new Intent( getBaseContext(), Preferences.class );
           startActivity( settingsActivity );

           return true;

       case R.id.about:
          Intent aboutActivity = new Intent( getBaseContext(), About.class );
          startActivity( aboutActivity );

          return true;

       default:
          return super.onOptionsItemSelected(item);
       }
    }
}
