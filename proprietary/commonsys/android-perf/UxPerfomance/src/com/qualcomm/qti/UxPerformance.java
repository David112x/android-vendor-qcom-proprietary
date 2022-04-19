/*
 * Copyright (c) 2015,2017,2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Copyright (c) 2011-2014, The Linux Foundation. All rights reserved.
 */

package com.qualcomm.qti;

import android.util.Log;
import java.util.ArrayList;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.RandomAccessFile;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;
import android.util.Slog;
import java.io.FileDescriptor;
import java.io.IOException;
import java.lang.reflect.Method;
import android.os.Trace;

public class UxPerformance
{
    private static final String TAG_TASKS = "UxPerf";

    /** @hide */
    public UxPerformance() {
        QLogD( "UX Perf module initialized");

        try {
              Class sPerfClass = Class.forName("com.qualcomm.qti.Performance");
              Class[] argClasses = new Class[] {String.class, String.class};
              Method sPerfGetPropFunc = sPerfClass.getMethod("perfGetProp", argClasses);
              Object mPerf = sPerfClass.newInstance();
              Object retVal = sPerfGetPropFunc.invoke(mPerf, "vendor.perf.iop_v3.enable", "false");
              EnablePrefetch = Boolean.parseBoolean(retVal.toString());
              Object retVal_2 = sPerfGetPropFunc.invoke(mPerf, "vendor.perf.iop_v3.enable.debug", "false");
              EnableDebug = Boolean.parseBoolean(retVal_2.toString());
        } catch(Exception e) {
              Log.e(TAG_TASKS, "Couldn't load Performance Class");
        }
    }
    /** @hide */
    private void QLogI(String msg)
    {
        if(EnableDebug) Slog.i(TAG_TASKS, msg);
        return;
    }
     /** @hide */
    private void QLogD(String msg)
    {
        if(EnableDebug) Slog.d(TAG_TASKS, msg);
        return;
    }
    /** @hide */
    private void QLogV(String msg)
    {
        if(EnableDebug) Slog.v(TAG_TASKS, msg);
        return;
    }
    /** @hide */
    private void QLogE(String msg)
    {
        if(EnableDebug) Slog.e(TAG_TASKS, msg);
        return;
    }

    public DexPrefetchThread PreDexThread = null;
    public boolean EnablePrefetch = false;
    public boolean EnableDebug = false;

    /* The following are the PerfLock API return values*/
    /** @hide */ public static final int REQUEST_FAILED = -1;
    /** @hide */ public static final int REQUEST_SUCCEEDED = 0;

    /* The following functions are the UX APIs*/
    /** @hide */
    public int perfIOPrefetchStart(int PId, String PkgName, String CodePath)
    {
        if(EnablePrefetch)
        {
           QLogI("DexPrefetchThread Feature is Enable ");
           PreDexThread = new DexPrefetchThread(CodePath);
            if(PreDexThread != null)
                (new Thread(PreDexThread)).start();
        }
        else
        {
            QLogI("DexPrefetchThread Feature is disabled ");
            return REQUEST_FAILED;
        }
        return REQUEST_SUCCEEDED;
    }

    private class DexPrefetchThread implements Runnable {
        public String CodePath;
        public DexPrefetchThread(String CodePath) {
            this.CodePath = CodePath;
        }

        public void LoadFiles(String path,String FileName)
        {
            String[] Files = {".art", ".odex", ".vdex"};
            QLogD(" LoadFiles() path is " + path);
            for ( int i = 0; i< Files.length; i++ )
            {
                File MyFile = new File(path + FileName + Files[i]);
                if(MyFile.exists())
                {
                    try {
                        FileChannel fileChannel = new RandomAccessFile(MyFile, "r").getChannel();
                        MappedByteBuffer buffer = fileChannel.map(FileChannel.MapMode.READ_ONLY, 0, fileChannel.size());
                        if(EnableDebug)
                            QLogD( "Before Is Buffer Loaded " + buffer.isLoaded());
                        QLogD( "File is " + path + FileName + Files[i]);
                        buffer.load();
                        if(EnableDebug)
                            QLogD( "After Is Buffer Loaded " + buffer.isLoaded());
                    } catch (FileNotFoundException e) {
                        QLogE( "DexPrefetchThread Can not find file " + FileName + Files[i] + "at " + path);
                    } catch (IOException e) {
                        QLogE( "DexPrefetchThread IO Error file " + FileName + Files[i] + "at " + path);
                    }
                }
            }
        }
        public void run() {
            //Fix me : This loop is added just to comply with thread run functions.
            while(true) {
                Trace.traceBegin(Trace.TRACE_TAG_ACTIVITY_MANAGER, "DexPrefetchThread");
                if ( CodePath.startsWith("/data") )
                {
                    //Check for 32/64 bit apps
                    QLogD( "Data pkg ");
                    if((new File(CodePath + "/oat/arm64/")).exists())
                    {
                        LoadFiles(CodePath + "/oat/arm64/", "base");
                    }
                    else if((new File(CodePath + "/oat/arm/")).exists())
                    {
                        LoadFiles(CodePath + "/oat/arm/", "base");
                    }
                }
                else
                {
                    // Get the name of package
                    QLogD( "system/vendor pkg ");
                    String[] SplitPath = CodePath.split("/");
                    String PkgName = SplitPath[SplitPath.length-1];
                    QLogD( "PKgNAme : " + PkgName);

                    //Check for 32/64 bit apps
                    if((new File(CodePath + "/oat/arm64/")).exists())
                    {
                        LoadFiles(CodePath + "/oat/arm64/", PkgName);
                    }
                    else if((new File(CodePath + "/oat/arm/")).exists())
                    {
                        LoadFiles(CodePath + "/oat/arm/", PkgName);
                    }
                }
                Trace.traceEnd(Trace.TRACE_TAG_ACTIVITY_MANAGER);
                return;

            }
        }
    }
}
