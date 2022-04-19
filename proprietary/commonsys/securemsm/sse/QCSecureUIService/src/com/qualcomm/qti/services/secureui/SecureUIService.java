/*
 * Copyright (c) 2014, 2016-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.services.secureui;

import com.android.internal.telephony.cat.AppInterface;
import android.app.PendingIntent;
import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.UserHandle;
import android.os.Handler;
import android.util.Log;
import android.view.Gravity;
import android.widget.Toast;
import android.os.PowerManager;
import java.util.HashMap;
import android.telephony.TelephonyManager;
import android.provider.Settings;

/* Secure UI Service gathers events from the Android framework
 * and forwards them to the native SecureUI entities.
 * Blocks WFD when a secure UI session is starting,
 * and resumes it when it's done.
 *
 * This service is started on BOOT_COMPLETED
 */
public class SecureUIService extends Service implements Runnable{
    public static final String TAG = "SUI_service";

    public static final String ACTION_SECURE_DISPLAY_ACTIVE =
        "org.codeaurora.intent.action.SECURE_DISPLAY_ACTIVE";
    public static final String EXTRA_SECURE_DISPLAY_STATE = "state";
    public static final String INTENT_CATEGORY = "android.intent.category.HOME";


    private native int init();
    private native int terminate();
    private native byte[] waitForMessage(byte[] input);
    private native byte[] getSource();
    private native int sendResponse(int id, int payload, byte[] remote);
    private native int pauseExternal(int display, int doPause);
    private native int externalIsConnected();
    private native int getdispprop(int id, int payload,byte [] input, byte [] remote);
    private native int startdisp(int id, int payload,byte [] input, byte [] remote);
    private native int secuidequeue(int id, int payload,byte [] input, byte [] remote);
    private native int secuienqueue(int id, int payload,byte [] input, byte [] remote);
    private native int stopdisp(int id, int payload,byte [] input, byte [] remote);
    public static native int sendNotification(int id, int payload, byte[] remote);

    public static byte[] TOUCH_LIB_ADDR = {'s', 'u', 'i', 't', 'c', 'h'};
    private boolean running = false;
    private boolean secureDisplayActive = false;
    private boolean orientationActivityOpen = false;
    private static int rotation = 0;
    private static final Object LOCK = new Object();
    private static PowerManager.WakeLock screen_wl;
    private int sys_rotation;

    private static final HashMap<Integer, String> msgNameMap = new HashMap<>();

    /** DO NOT CHANGE THE FOLLOWING. Refer to readme.txt for an explanation */
    /** Bytes in a message */
    private static final int BYTES_IN_MEX = 4;
    private static final int BYTES_IN_INPUT = 32;
    /** Message type */
    private static final int SUI_MSG_CMD_MSK = 0x08;
    private static final int SUI_MSG_RSP_MSK = 0x04;
    private static final int SUI_MSG_NOT_MSK = 0x02;
    /** Message ID */
    public static final int SUI_MSG_ID_CALL_STATUS = 0x01;
    public static final int SUI_MSG_ID_ABORT = 0x02;
    public static final int SUI_MSG_ID_SCREEN_STATUS = 0x03;
    public static final int SUI_MSG_ID_SD_STARTING = 0x10;
    public static final int SUI_MSG_ID_SD_STOPPED = 0x11;
    public static final int SUI_MSG_SEC_DISP_PROPERTIES = 0x12;
    public static final int SUI_MSG_SEC_DISP_ENQUEUE = 0x13;
    public static final int SUI_MSG_SEC_DISP_DEQUEUE = 0x14;
    public static final int SUI_MSG_SEC_DISP_START = 0x15;
    public static final int SUI_MSG_SEC_DISP_STOP = 0x16;

    /** Message payload */
    public static final int SUI_MSG_RET_OK = 0x00;
    public static final int SUI_MSG_RET_ABORT = 0x01;
    public static final int SUI_MSG_RET_NOP = 0x0F;

    static {
        System.loadLibrary("secureuisvc_jni");
    }

    public static Context context;

    @Override
    public void run() {
        int ret = -1;

        Log.d(TAG, "message thread started...");

        ret = getsession();
        if (ret != 0) {
            Log.e(TAG, "failed to get session, message thread exiting...");
            return;
        }
        running = true;

        CallReceiver mCallReceiver = new CallReceiver();
        IntentFilter filter2 = new IntentFilter(TelephonyManager.ACTION_PHONE_STATE_CHANGED);
        filter2.addCategory(INTENT_CATEGORY);
        registerReceiver(mCallReceiver, filter2);

        PowerReceiver mPowerReceiver = new PowerReceiver();
        IntentFilter filter1 = new IntentFilter(Intent.ACTION_SHUTDOWN);
        filter1.addCategory(INTENT_CATEGORY);
        registerReceiver(mPowerReceiver, filter1);

        ScreenReceiver mScreenReceiver = new ScreenReceiver();
        IntentFilter filter = new IntentFilter(Intent.ACTION_SCREEN_OFF);
        filter.addAction(Intent.ACTION_SCREEN_ON);
        filter.addAction(Intent.ACTION_USER_PRESENT);
        registerReceiver(mScreenReceiver, filter);

        // Initialize the wake lock.
        // The wakelock can be used to prevent android from dimming
        // or turning off the dispay during secure UI session.
        // The responsibility can be moved into application/other service as needed
        PowerManager pm = (PowerManager) context.getSystemService(
            Context.POWER_SERVICE);
        screen_wl = pm.newWakeLock(PowerManager.SCREEN_BRIGHT_WAKE_LOCK |
                                   PowerManager.ON_AFTER_RELEASE, TAG);

        do {
            byte[] input = new byte[BYTES_IN_INPUT];
            byte[] mex;

            // On tui_comm hal death, retry acquiring
            // session from the new instance
            while((mex = waitForMessage(input)) == null) {
                Log.w(TAG, "tui_comm dead, trying to acquire session again");
                // hal is expected to restart; keep retrying
                getsession();
            }

            if (mex.length != BYTES_IN_MEX) {
                Log.w(TAG, "invalid message len: " + mex.length + " vs " + BYTES_IN_MEX);
                Log.w(TAG, "Mex: " + String.format("%02X %02X %02X %02X",
                                                   mex[0], mex[1], mex[2], mex[3]));
                continue;
            }

            if (mex[0] == SUI_MSG_NOT_MSK) {
                ret = ProcessNotification(mex[1], mex[2]);
                if (ret != 0) {
                    Log.w(TAG, "failed to process notification " +
                          String.format("%02X %02X: %d", mex[1], mex[2], ret));
                    continue;
                }
            } else {
                byte[] source = getSource();
                ret = ProcessCommand(mex, input, source);
                if (ret != 0) {
                    Log.w(TAG, "failed to process command " +
                          String.format("%02X %02X: %d", mex[1], mex[2], ret));
                    continue;
                }
            }
        } while (true);
    }

    int getsession() {
        int ret = -1;
        int num_try = 0;

        // try cleaning previous stale svc connection
        terminate();
        do {
            ret = init();
            if (ret != 0) {
                Log.d(TAG, "svc_sock_init() failed, re-attempt, ret = " + ret);
                if (num_try++ > 10) {
                    break;
                }
                continue;
            }
            break;
        } while (true);

        // Try to abort previous touch session (if any)
        sendNotification(SUI_MSG_ID_ABORT, SUI_MSG_RET_NOP,
                         TOUCH_LIB_ADDR);
        return ret;
    }

    void setSecureDisplayActive(boolean active) {
        if (secureDisplayActive != active) {
            secureDisplayActive = active;

            Intent intent = new Intent(ACTION_SECURE_DISPLAY_ACTIVE);
            intent.putExtra(EXTRA_SECURE_DISPLAY_STATE, active);
            sendBroadcastAsUser(intent, UserHandle.ALL);

            // activate wakelock on SD start and release on deactivation
            if (active) {
                Log.d(TAG,"wakelock acquire");
                screen_wl.acquire();
            } else {
                Log.d(TAG,"wakelock release");
                screen_wl.release();
            }
        }
    }

    static public void setRotation(int rot) {
        rotation = rot;
        synchronized(LOCK) {
            LOCK.notifyAll();
        }
    }

    int setOrientationActivityOn(boolean active) {
      rotation = 0xFF;

      if (active == true) {
        try {
          sys_rotation = Settings.System.getInt(getContentResolver(), Settings.System.ACCELEROMETER_ROTATION);
          Settings.System.putInt(getContentResolver(), Settings.System.ACCELEROMETER_ROTATION, 0);
        }
        catch (Settings.SettingNotFoundException ex) {
          Log.w(TAG, "Orientation setting not found");
        }
        // start activity to get the real orientation
        Intent intent = new Intent(this, OrientationActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK |
                        Intent.FLAG_ACTIVITY_CLEAR_TOP |
                        Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS |
                        Intent.FLAG_ACTIVITY_NEW_TASK |
                        Intent.FLAG_ACTIVITY_NO_ANIMATION |
                        Intent.FLAG_ACTIVITY_NO_HISTORY |
                        Intent.FLAG_ACTIVITY_NO_USER_ACTION |
                        Intent.FLAG_ACTIVITY_SINGLE_TOP |
                        Intent.FLAG_FROM_BACKGROUND);
        this.startActivity(intent);
        // wait for rotation to be returned
        if (rotation == 0xFF) {
          try {
            synchronized(LOCK) {
              LOCK.wait(500);
            }
          } catch (InterruptedException e) {
              Log.d(TAG, "Interrupted!");
          }
        }
      }
      else  {
        Log.d(TAG, "Restoring screen rotation");
        Settings.System.putInt(getContentResolver(), Settings.System.ACCELEROMETER_ROTATION, sys_rotation);
      }
      return rotation;
    }

    private int ProcessCommand(byte[] mex, byte[] input, byte[] source) {
        int ret = 0;
        int id = (int) mex[1];
        int payload = (int) mex[2];

        Log.e(TAG, "ProcessCommand: " + msgNameMap.get(id));

        switch (id) {
        case SUI_MSG_ID_SD_STARTING:
            setSecureDisplayActive(true);
            rotation = setOrientationActivityOn(true);
            ret = sendResponse(id, rotation, source);
            break;
        case SUI_MSG_ID_CALL_STATUS:
            int call_status = (CallReceiver.callActive()) ?
                SUI_MSG_RET_ABORT : SUI_MSG_RET_OK;
            ret = sendResponse(id, call_status, source);
            break;
        case SUI_MSG_ID_SCREEN_STATUS:
            int screen_status = (ScreenReceiver.screenOn()) ?
                SUI_MSG_RET_OK : SUI_MSG_RET_ABORT;
            ret = sendResponse(id, screen_status, source);
            break;
        case SUI_MSG_SEC_DISP_PROPERTIES:
            ret = getdispprop(id, SUI_MSG_RET_OK, input, source);
            break;
        case SUI_MSG_SEC_DISP_ENQUEUE:
            ret = secuienqueue(id, SUI_MSG_RET_OK, input, source);
            break;
        case SUI_MSG_SEC_DISP_DEQUEUE:
            ret = secuidequeue(id, SUI_MSG_RET_OK, input, source);
            break;
        case SUI_MSG_SEC_DISP_START:
            ret = startdisp(id, SUI_MSG_RET_OK, input, source);
            break;
        case SUI_MSG_SEC_DISP_STOP:
            ret = stopdisp(id, SUI_MSG_RET_OK,  input, source);
            break;
        default:
            Log.e(TAG, "invalid command");
        }

        return ret;
    }

    private int ProcessNotification(int id, int payload) {
        int ret = 0;

        if (id == SUI_MSG_ID_SD_STOPPED) {
            Log.d(TAG, "ProcessNotification: " + "SD_Stopped");
            setOrientationActivityOn(false);

            // broadcasting intent
            setSecureDisplayActive(false);

            // show toast msg on stop SD
            Handler hdl = new Handler(SecureUIService.this.getMainLooper());
            hdl.post(new Runnable() {
                @Override
                public void run() {
                    Toast t = Toast.makeText(SecureUIService.this,
                                             R.string.sd_stop_msg,
                                             Toast.LENGTH_SHORT);
                    t.setGravity(Gravity.CENTER, 0, 0);
                    t.show();
                }
            });
        } else {
            Log.d(TAG, "Process (unhandled) Notification: " + String.format("%02X %02X", id, payload));
        }
        return ret;
    }

    private void InitMsgMap() {
        msgNameMap.clear();
        msgNameMap.put(SUI_MSG_ID_SD_STARTING, "SD_Starting");
        msgNameMap.put(SUI_MSG_ID_CALL_STATUS, "Call_Status");
        msgNameMap.put(SUI_MSG_ID_SCREEN_STATUS, "Screen_Status");
        msgNameMap.put(SUI_MSG_SEC_DISP_PROPERTIES, "Disp_Props");
        msgNameMap.put(SUI_MSG_SEC_DISP_ENQUEUE, "Disp_Enqueue");
        msgNameMap.put(SUI_MSG_SEC_DISP_DEQUEUE, "Disp_Dequeue");
        msgNameMap.put(SUI_MSG_SEC_DISP_START, "Disp_Start");
        msgNameMap.put(SUI_MSG_SEC_DISP_STOP, "Disp_Stop");
    }

    @Override
    public void onCreate() {
        super.onCreate();
        SecureUIService.context = this;
        InitMsgMap();
        if (!running) {
            new Thread(this).start();
        }
    }

    @Override
    public int onStartCommand(Intent intent,
                              int flags,
                              int startId)
    {
        Log.d(TAG, "Service started flags " + flags + " startId " + startId);

        // We want this service to continue running until it is explicitly
        // stopped, so return sticky.
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        sendNotification(SUI_MSG_ID_ABORT,
                         SUI_MSG_RET_NOP,
                         TOUCH_LIB_ADDR);
        sendNotification(SUI_MSG_ID_ABORT,
                         SUI_MSG_RET_NOP, null);
        screen_wl.release();
        terminate();
        Log.d(TAG, "onDestroy");
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind");
        return null;
    }
}
