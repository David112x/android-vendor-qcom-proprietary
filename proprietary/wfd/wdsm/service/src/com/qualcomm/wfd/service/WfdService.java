/* ==============================================================================
 * WfdService.java
 *
 * Copyright (c) 2012,2014-2016, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ============================================================================== */

package com.qualcomm.wfd.service;

import android.annotation.TargetApi;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.os.Build;
import android.os.IBinder;
import android.os.Process;
import android.util.Log;
/**
 * Main service entry point for Session Manager Service
 *
 * @author sachins
 */
public class WfdService extends Service {

    private static final String TAG = "WfdService";

    private ISessionManagerService.Stub mBinder;

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "onStart()");
        Notification.Builder nb = new Notification.Builder(
                getApplicationContext())
            .setContentTitle(getResources().getString(R.string.notif_title))
            .setContentText(getResources().getString(R.string.notif_text))
            .setSmallIcon(R.drawable.icon);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            nb.setChannelId(createNotificationChannel(TAG,
                    getResources().getString(R.string.channel_name))
                    .getId());
        }
        startForeground(Process.myPid(), nb.build());
        return super.onStartCommand(intent, flags, startId);
    }

    @TargetApi(Build.VERSION_CODES.O)
    private NotificationChannel createNotificationChannel(
            String channelId, String channelName){
        NotificationChannel channel = new NotificationChannel(channelId,
                channelName, NotificationManager.IMPORTANCE_NONE);
        NotificationManager service = (NotificationManager)
            getSystemService(Context.NOTIFICATION_SERVICE);
        service.createNotificationChannel(channel);
        return channel;
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind()");
        if (mBinder == null) {
            Context c = getApplicationContext();
            Log.d(TAG, "Creating SessionManagerService with context:" + c);
            mBinder = new SessionManagerService(c);
        }
        return mBinder;
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy()");
        super.onDestroy();
        if (mBinder != null && mBinder instanceof SessionManagerService) {
            Log.d(TAG, "Unregister callbacks");
            ((SessionManagerService) mBinder).destroyService();
            mBinder = null;
        }
    }

}
