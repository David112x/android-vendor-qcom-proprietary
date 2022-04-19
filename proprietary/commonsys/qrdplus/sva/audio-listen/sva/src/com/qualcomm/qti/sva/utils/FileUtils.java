/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.utils;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;

public class FileUtils {
    private final static String TAG = FileUtils.class.getSimpleName();

    public static boolean isExist(String path) {
        if (null == path) {
            LogUtils.e(TAG, "isExist: invalid input param");
            return false;
        }

        File target = new File(path);
        return target.exists();
    }

    public static void createDirIfNotExists(String dir) {
        File directoryFile = new File(dir);
        if (!directoryFile.isDirectory()) {
            directoryFile.mkdirs();
        }
    }

    public static void deleteFile(String filePath) {
        try {
            File file = new File(filePath);
            if (!file.delete()) {
                LogUtils.d(TAG, "deleteFile: delete fail for file "
                        + filePath);
            }
        } catch (NullPointerException  e) {
            LogUtils.e(TAG, "deleteFile: invalid file path");
            e.printStackTrace();
        } catch (SecurityException e) {
            LogUtils.e(TAG, "deleteFile: delete failed");
            e.printStackTrace();;
        }
    }

    public static ByteBuffer readFileToByteBuffer(String filePath) throws IOException {
        File file = new File(filePath);
        RandomAccessFile raf = new RandomAccessFile(file, "r");
        ByteBuffer byteBuffer;
        try {
            long longLength = raf.length();
            int length = (int) longLength;
            if (length != longLength) throw new IOException("File size >= 2 GB");

            byte[] data = new byte[length];
            raf.readFully(data);
            byteBuffer = ByteBuffer.allocateDirect(data.length);
            byteBuffer.put(data);
        } finally {
            raf.close();
        }
        return byteBuffer;
    }

    public static boolean saveByteBufferToFile(ByteBuffer data, String filePath) {
        boolean bSuccess = false;
        FileChannel channel;
        try {
            channel = new FileOutputStream(filePath, false).getChannel();
            data.flip();
            channel.write(data);
            channel.close();
            bSuccess = true;
        } catch (FileNotFoundException e) {
            LogUtils.d(TAG, "saveByteBufferToFile: FileNotFoundException "
                    + e.getMessage());
        } catch (IOException e) {
            LogUtils.d(TAG, "saveByteBufferToFile: IOException "
                    + e.getMessage());
        }

        return bSuccess;
    }

    public static byte[] readFileToByteArray(String fileStr) throws IOException {
        File file = new File(fileStr);
        RandomAccessFile raf = new RandomAccessFile(file, "r");
        byte[] data;
        try {
            long longLength = raf.length();
            int length = (int) longLength;
            if (length != longLength) throw new IOException("File size >= 2 GB");

            data = new byte[length];
            raf.readFully(data);
        } finally {
            raf.close();
        }
        return data;
    }

}
