/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.controller;

import android.content.Context;
import android.content.res.AssetManager;

import com.qualcomm.qti.sva.data.ISmModel;
import com.qualcomm.qti.sva.utils.FileUtils;
import com.qualcomm.qti.sva.utils.LogUtils;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class AssetsFileMgr {
    private final String TAG = AssetsFileMgr.class.getSimpleName();

    public AssetsFileMgr() {
    }

    public void copyAssetsIfNotExists(Context context, String destDir) {
        LogUtils.d(TAG, "copyAssetsIfNotExists: destDir = " + destDir);
        if (null == destDir) {
            LogUtils.e(TAG, "copyAssetsIfNotExists: invalid param");
            return;
        }

        FileUtils.createDirIfNotExists(destDir);

        if (FileUtils.isExist(destDir)) {
            copyAssets(context, destDir);
        } else {
            LogUtils.e(TAG, "copyAssetsIfNotExists: create destDir failure");
        }
    }

    private String[] copyAssets(Context context, String destDir) {
        if (null == destDir || !FileUtils.isExist(destDir)) {
            LogUtils.e(TAG, "copyAssets: invalid param");
            return null;
        }

        AssetManager assetManager = context.getAssets();
        String[] assetFiles = null;

        // Get asset files.
        try {
            assetFiles = assetManager.list("");
        } catch (IOException e) {
            e.printStackTrace();
        }

        if (null == assetFiles || 0 == assetFiles.length) {
            return null;
        }

        int index = 0;
        String[] copiedFilePaths = new String[assetFiles.length];

        //copy asset files
        try {
            for (String filename : assetFiles) {
                String outputFilePath = destDir + "/" + filename;
                LogUtils.d(TAG, "copyAssets: outputFilePath = " + outputFilePath);
                if (new File(outputFilePath).exists()) {
                    LogUtils.d(TAG, "copyAssets: exists yet, ignore");
                    continue;
                }

                LogUtils.d(TAG, "copyAssets: copy filename =  "
                        + filename + " to destDir");

                //filter only sound model and language model files
                if (filename.endsWith(ISmModel.SUFFIX_DEFAULT_SOUND_MODEL)
                        || filename.endsWith(ISmModel.SUFFIX_LANGUAGE_MODEL)
                        || filename.endsWith(ISmModel.SUFFIX_TRAINED_SOUND_MODEL)
                        || filename.endsWith(ISmModel.SUFFIX_MERGED_LANGUAGE_MODEL)) {
                    copySingleAssetFile(context, filename, outputFilePath);
                    copiedFilePaths[index++] = outputFilePath;
                } else {
                    LogUtils.d(TAG, "copyAssets: non sound model or language model files, ignore");
                    continue;
                }
            }

            LogUtils.d(TAG, "copyAssets: copy file number = " + index);
            return copiedFilePaths;
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }

    private void copySingleAssetFile(Context context, String filename,
                                           String outputFilePath) throws IOException {
        AssetManager assetManager = context.getAssets();
        InputStream in = assetManager.open(filename);
        OutputStream out = new FileOutputStream(outputFilePath);
        byte[] buffer = new byte[1024];
        int readCount;
        while ((readCount = in.read(buffer)) != -1) {
            out.write(buffer, 0, readCount);
        }
        in.close();
        out.flush();
        out.close();
    }
}
