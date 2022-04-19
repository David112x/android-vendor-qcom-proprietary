/**
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.iwlan;

import android.util.Log;
import android.os.SystemProperties;

public class IWlanLog {
    private static boolean DBG_PROPERTY = SystemProperties.getBoolean("persist.vendor.iwlan.logging.logcat", false);

    private static int MAX_BUFFER_SIZE = 200;

    public LogBuffer dataLogBuffer;
    public LogBuffer networkLogBuffer;
    public LogBuffer qnpLogBuffer;

    public IWlanLog() {
        dataLogBuffer = new LogBuffer(MAX_BUFFER_SIZE);
        networkLogBuffer = new LogBuffer(MAX_BUFFER_SIZE);
        qnpLogBuffer = new LogBuffer(MAX_BUFFER_SIZE);
    }

    //print only if sysprop is present
    public void v(String tag, String msg) {
        if (DBG_PROPERTY) Log.v(tag, msg);
        addToBuffer(tag, msg);
    }
    //print both, via sysprop
    public void d(String tag, String msg) {
        if (DBG_PROPERTY) Log.d(tag, msg);
        addToBuffer(tag, msg);
    }
    //print both, via sysprop
    public void i(String tag, String msg) {
        if (DBG_PROPERTY) Log.i(tag, msg);
        addToBuffer(tag, msg);
    }
    //print to both logcat and buffer
    public void w(String tag, String msg) {
        Log.w(tag, msg);
        addToBuffer(tag, msg);
    }
    //print to both logcat and buffer
    public void e(String tag, String msg) {
        Log.e(tag, msg);
        addToBuffer(tag, msg);
    }

    private void addToBuffer(String tag, String msg) {
        switch(tag) {
            case "IWlanDataService":
                dataLogBuffer.addLog(msg);
                break;
            case "IWlanNetworkService":
                networkLogBuffer.addLog(msg);
                break;
            case "QualifiedNetworksServiceImpl":
                qnpLogBuffer.addLog(msg);
                break;
        }
    }
}
