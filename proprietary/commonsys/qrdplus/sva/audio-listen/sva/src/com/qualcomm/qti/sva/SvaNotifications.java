/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;

public final class SvaNotifications {
    public static final int SVA_FOREGROUND_NOTIFICATION_ID = Integer.MAX_VALUE - 1;
    private final static String SVA_CHANNEL_ID = "SVA_CHANNEL";

    public static Notification buildWakeupServiceNotification(Context context) {
        String channelName = context.getString(R.string.channel_name);
        String description = context.getString(R.string.channel_description);
        NotificationChannel channel = new NotificationChannel(
                SVA_CHANNEL_ID, channelName, NotificationManager.IMPORTANCE_NONE);
        channel.setDescription(description);

        NotificationManager manager =
                (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        if (null != manager) {
            manager.createNotificationChannel(channel);
        }

        return new Notification.Builder(
                context.getApplicationContext(), SVA_CHANNEL_ID)
                .setContentText(context.getString(R.string.notification_text))
                .setSmallIcon(R.drawable.launcher)
                .setContentIntent(null)
                .build();
    }

    public static void notifySVAActive(Context context) {
        /* change the wakeup service to foreground, so will use the foreground notification
        cancelNotification(context);
        Notification notification = buildWakeupServiceNotification(context);
        NotificationManager manager =
                (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        if (null != manager) {
            manager.notify(SVA_FOREGROUND_NOTIFICATION_ID, notification);
        }
        */
    }

    public static void cancelNotification(Context context) {
        /* change the wakeup service to foreground, so will use the foreground notification
        NotificationManager manager =
                (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        if (null != manager) {
            manager.cancelAll();
        }
        */
    }
}
