/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.controller;

import android.content.Context;
import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaPlayer;
import android.media.MediaRecorder;
import android.net.Uri;

import com.qualcomm.qti.sva.data.ISettingsModel;
import com.qualcomm.qti.sva.data.SettingsModel;
import com.qualcomm.qti.sva.utils.FileUtils;
import com.qualcomm.qti.sva.utils.LogUtils;
import com.qualcomm.qti.sva.wrapper.AudioAttributesBuilderWrapper;
import com.qualcomm.qti.sva.wrapper.AudioRecordWrapper;

import java.io.BufferedOutputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.ByteOrder;
import java.nio.ShortBuffer;
import java.util.Arrays;

public class RecordingsMgr {
    //LAB buffer params definition
    public static final short NUM_CHANNELS = 1; // mono
    public static final int BITS_PER_SAMPLE = 16; // in bits
    public static final int SAMPLE_RATE = 16000; // 16kHz
    public static final int SAMPLE_SIZE_SENCOND = (BITS_PER_SAMPLE * SAMPLE_RATE) / 8;
    public static final int WAV_HEADER_LENGTH = 44;
    private static final int RECORD_BUFFER_SIZE = 2 * SAMPLE_SIZE_SENCOND;
    private static final int READ_BUFFER_SIZE = 120 * (SAMPLE_SIZE_SENCOND / 1000);

    // sound model training
    public static final int DATA_READ_INTERVAL = 120; // read recording data period 120ms
    public final static int NUM_RECORDINGS_REQUIRED = 5;
    private final static String FILE_NAME_PREFIX_OF_TRAINING_RECORDING = "training";
    private final String TAG = RecordingsMgr.class.getSimpleName();
    private AudioRecord mTrainingAudioRecord;
    private Thread mTrainingRecordingThread;
    private int mTrainingBufferSize = 0;
    private ShortBuffer[] mUserRecordings = new ShortBuffer[NUM_RECORDINGS_REQUIRED];
    private int mRecordingCounter = 0;
    private boolean mIsTerminatedRecording;

    private MediaPlayer mMediaPlayer;

    public RecordingsMgr() {
    }

    public void playbackRecording(Context context, String filePath) {
        LogUtils.d(TAG, "playbackRecording: filePath = " + filePath);
        if (null == filePath || !FileUtils.isExist(filePath)) {
            LogUtils.e(TAG, "playbackRecording: invalid param");
            return;
        }

        Uri uri = Uri.fromFile(new File(filePath));
        LogUtils.d(TAG, "playbackRecording: mMediaPlayer = " + mMediaPlayer);
        if (null != mMediaPlayer) {
            if (mMediaPlayer.isPlaying()) {
                mMediaPlayer.stop();
            }
            mMediaPlayer.release();
            mMediaPlayer = null;
        }

        mMediaPlayer = MediaPlayer.create(context, uri);
        mMediaPlayer.start();
    }

    public void stopPlaybackRecording() {
        LogUtils.d(TAG, "stopPlaybackRecording: mMediaPlayer = " + mMediaPlayer);
        if (null != mMediaPlayer) {
            if (mMediaPlayer.isPlaying()) {
                mMediaPlayer.stop();
            }
            mMediaPlayer.release();
            mMediaPlayer = null;
        }
    }

    public boolean startHotWordRecording(Context context, String smName,
                                         int sessionId, String savePath) {
        /* Open AudioRecord Session using sessionId returned by detection event */
        AudioAttributesBuilderWrapper audAttBldr = new AudioAttributesBuilderWrapper();
        audAttBldr.setUsage(AudioAttributes.USAGE_MEDIA);
        audAttBldr.setContentType(AudioAttributes.CONTENT_TYPE_SPEECH);

        // %%% MediaRecorder.HOTWORD
        // %%% HOTWORD is current private so hardcored value is used
        audAttBldr.setInternalCapturePreset(1999);
        AudioAttributes audAtt = audAttBldr.build();

        int minBuffSize = AudioRecord.getMinBufferSize(SAMPLE_RATE,
                AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT);

        AudioFormat.Builder audFormatBldr = new AudioFormat.Builder();
        audFormatBldr.setEncoding(AudioFormat.ENCODING_PCM_16BIT);
        audFormatBldr.setSampleRate(SAMPLE_RATE);
        audFormatBldr.setChannelMask(AudioFormat.CHANNEL_IN_MONO);
        AudioFormat audFormat = audFormatBldr.build();

        int streamBufferSizeInBytes;
        if (minBuffSize > RECORD_BUFFER_SIZE) {
            streamBufferSizeInBytes = minBuffSize;
        } else {
            streamBufferSizeInBytes = RECORD_BUFFER_SIZE;
        }

        AudioRecord audioRecord =
                new AudioRecordWrapper(audAtt, audFormat,
                        streamBufferSizeInBytes, sessionId).getAudioRecordInstance();
        if (audioRecord.getState() != AudioRecord.STATE_INITIALIZED) {
            LogUtils.d(TAG, "startHotWordRecording: getAudioRecordInstance fail");
            return false;
        }

        int bufferSize = getHotWordRecordingBufferSize(context, smName);
        byte[] buffer = new byte[bufferSize];
        try {
            audioRecord.startRecording();
        } catch (IllegalStateException e) {
            LogUtils.d(TAG, "startHotWordRecording: startRecording IllegalStateException");
            e.printStackTrace();
            return false;
        }

        int curBytesRead = 0;
        while (curBytesRead < bufferSize) {
            LogUtils.d(TAG, "startHotWordRecording: curBytesRead = " + curBytesRead);
            int bufferReadResult = audioRecord.read(buffer, curBytesRead, READ_BUFFER_SIZE);
            if (bufferReadResult < 0) {
                LogUtils.d(TAG, "startHotWordRecording: read error");
                break;
            } else {
                LogUtils.d(TAG, "startHotWordRecording: read = "
                        + bufferReadResult + " bytes");
                curBytesRead += bufferReadResult;
            }
        }

        LogUtils.d(TAG, "startHotWordRecording: curBytesRead = " + curBytesRead);

        audioRecord.stop();
        audioRecord.release();

        // save recording file
        if (null != savePath) {
            if (curBytesRead > 0) {
                saveHotWordRecording(buffer, bufferSize, savePath);
            }
        }

        return true;
    }

    private int getHotWordRecordingBufferSize(Context context, String smName) {
        ISettingsModel settings = new SettingsModel(context, smName);
        int labSize = (SAMPLE_SIZE_SENCOND / 1000) * settings.getVoiceRequestLength();
        int histBuffSize = SAMPLE_RATE * settings.getHistBufferTime() / 1000
                * BITS_PER_SAMPLE / 8;
        boolean labEnabled = settings.getVoiceRequestEnabled();
        boolean histEnabled = settings.getOpaqueDataTransferEnabled();

        LogUtils.d(TAG, "getHotWordRecordingBufferSize: labSize = " + labSize
                + " histBuffSize = " + histBuffSize + " labEnabled = " + labEnabled
                + " histEnabled = " + histEnabled);

        int hotWordBufferSize = 0;
        if (labEnabled) {
            hotWordBufferSize += labSize;
        }

        if (histEnabled) {
            hotWordBufferSize += histBuffSize;
        }

        LogUtils.d(TAG, "getHotWordRecordingBufferSize: hotWordBufferSize = "
                + hotWordBufferSize);

        return hotWordBufferSize;
    }

    private void saveHotWordRecording(byte[] buffer, int bufferSize, String filePath) {
        LogUtils.v(TAG, "saveHotWordRecording filePath = " + filePath);
        writeBufferToWavFile(buffer, bufferSize, filePath, false);
    }

    private void writeBufferToWavFile(byte[] inBuff, int bufferSize, String filePath,
                                      boolean isAppendToFile) {
        DataOutputStream doStream = null;
        try {
            LogUtils.v(TAG, "writeBufferToWavFile: stream created bufferSize "
                    + bufferSize);
            doStream = new DataOutputStream(
                    new BufferedOutputStream(new FileOutputStream(filePath, isAppendToFile)));
            int audioDataLength = bufferSize;
            byte[] wavHeader = getWavHeader(audioDataLength,
                    audioDataLength + WAV_HEADER_LENGTH);
            // write header
            LogUtils.v(TAG, "writeBufferToWavFile: write header");
            doStream.write(wavHeader);
            // write audio samples
            LogUtils.v(TAG, "writeBufferToWavFile: write " + (audioDataLength < 1)
                    + " samples");
            doStream.write(inBuff, 0, audioDataLength);
            LogUtils.v(TAG, "writeBufferToWavFile: complete");
        } catch (FileNotFoundException e) {
            LogUtils.e(TAG, "writeShortBufferToWavFile: FileNotFound: " + e.getMessage());
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                if (null != doStream) {
                    doStream.close();
                    LogUtils.v(TAG, "writeBufferToWavFile: stream close");
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public byte[] getWavHeader(long audioDataLength, long audioDataAndHeaderLength) {
        long blocksPerSecond = SAMPLE_RATE;
        long dataRate = BITS_PER_SAMPLE * SAMPLE_RATE * NUM_CHANNELS / 8;

        byte[] wavHeader = new byte[WAV_HEADER_LENGTH];
        wavHeader[0] = 'R';  // chunk ID = "RIFF"
        wavHeader[1] = 'I';
        wavHeader[2] = 'F';
        wavHeader[3] = 'F';
        wavHeader[4] = (byte) (audioDataAndHeaderLength & 0xff); // chunk size
        wavHeader[5] = (byte) ((audioDataAndHeaderLength >> 8) & 0xff);
        wavHeader[6] = (byte) ((audioDataAndHeaderLength >> 16) & 0xff);
        wavHeader[7] = (byte) ((audioDataAndHeaderLength >> 24) & 0xff);
        wavHeader[8] = 'W'; // wave ID = "WAVE"
        wavHeader[9] = 'A';
        wavHeader[10] = 'V';
        wavHeader[11] = 'E';
        wavHeader[12] = 'f'; // chunk ID = "fmt "
        wavHeader[13] = 'm';
        wavHeader[14] = 't';
        wavHeader[15] = ' ';
        wavHeader[16] = 16;  // chunk size = 16
        wavHeader[17] = 0;
        wavHeader[18] = 0;
        wavHeader[19] = 0;
        wavHeader[20] = 1; // format code (0x0001 is PCM)
        wavHeader[21] = 0;
        wavHeader[22] = (byte) NUM_CHANNELS;  // number of interleaved channels
        wavHeader[23] = 0;
        wavHeader[24] = (byte) (blocksPerSecond & 0xff); // Sampling rate (blocks/sec)
        wavHeader[25] = (byte) ((blocksPerSecond >> 8) & 0xff);
        wavHeader[26] = (byte) ((blocksPerSecond >> 16) & 0xff);
        wavHeader[27] = (byte) ((blocksPerSecond >> 24) & 0xff);
        wavHeader[28] = (byte) (dataRate & 0xff); // Data rate
        wavHeader[29] = (byte) ((dataRate >> 8) & 0xff);
        wavHeader[30] = (byte) ((dataRate >> 16) & 0xff);
        wavHeader[31] = (byte) ((dataRate >> 24) & 0xff);
        wavHeader[32] = (byte) (NUM_CHANNELS * BITS_PER_SAMPLE / 8); // Data block size
        wavHeader[33] = 0;
        wavHeader[34] = BITS_PER_SAMPLE;
        wavHeader[35] = 0;
        wavHeader[36] = 'd'; // chunk ID = "data"
        wavHeader[37] = 'a';
        wavHeader[38] = 't';
        wavHeader[39] = 'a';
        wavHeader[40] = (byte) (audioDataLength & 0xff);
        wavHeader[41] = (byte) ((audioDataLength >> 8) & 0xff);
        wavHeader[42] = (byte) ((audioDataLength >> 16) & 0xff);
        wavHeader[43] = (byte) ((audioDataLength >> 24) & 0xff);

        return wavHeader;
    }

    public ShortBuffer readWavFile(String filePath) throws IOException {
        File file = new File(filePath);
        RandomAccessFile raf = new RandomAccessFile(file, "r");
        int nHdrLen = WAV_HEADER_LENGTH; // RIFF Wav file header length
        try {
            // skip file's header
            raf.skipBytes(nHdrLen);
            long longLength = raf.length();
            int length = (int) longLength;
            if (length != longLength) throw new IOException("File size >= 2 GB");
            length = (int) (longLength - nHdrLen);
            int nShorts = (length + 1) / 2;
            short[] data = new short[nShorts];

            // read bytes as a short taking endianness into account
            short tmpShrt;
            for (int i = 0; i < nShorts; i++) {
                if (ByteOrder.LITTLE_ENDIAN == ByteOrder.nativeOrder()) {
                    tmpShrt = raf.readShort();
                    // flip bytes
                    data[i] = (short) ((tmpShrt & 0x00FF) << 8);
                    data[i] += ((tmpShrt & 0xFF00) >> 8);
                } else {
                    data[i] = raf.readShort();
                }
            }

            ShortBuffer sb = ShortBuffer.allocate(data.length);
            sb.put(data);
            return sb;
        } finally {
            raf.close();
        }
    }

    @Override
    protected void finalize() throws Throwable {
        LogUtils.d(TAG, "finalize: mMediaPlayer = " + mMediaPlayer);
        if (null != mMediaPlayer) {
            if (mMediaPlayer.isPlaying()) {
                mMediaPlayer.stop();
            }
            mMediaPlayer.release();
            mMediaPlayer = null;
        }

        // set terminate thread condition
        mIsTerminatedRecording = true;

        super.finalize();
    }

    public void startTrainingRecording() {
        int channelConfig = AudioFormat.CHANNEL_IN_MONO;
        int audioFormat = AudioFormat.ENCODING_PCM_16BIT;
        int audioSource = MediaRecorder.AudioSource.MIC;
        int minBufferSize = AudioRecord.getMinBufferSize(SAMPLE_RATE, channelConfig, audioFormat);

        mTrainingBufferSize = SAMPLE_RATE * DATA_READ_INTERVAL / 1000 * NUM_CHANNELS
                * BITS_PER_SAMPLE / 8 * 2;
        LogUtils.d(TAG, "startTrainingRecording: minBufferSize = " + minBufferSize
                + " mTrainingBufferSize = " + mTrainingBufferSize);
        if (mTrainingBufferSize < minBufferSize) {
            mTrainingBufferSize = minBufferSize;
        }

        // release last recording
        if (null != mTrainingAudioRecord) {
            try {
                mTrainingAudioRecord.stop();
                mTrainingAudioRecord.release();
                mTrainingAudioRecord = null;
            } catch (IllegalStateException  e) {
                e.printStackTrace();
            }
        }

        mTrainingAudioRecord = new AudioRecord(audioSource, SAMPLE_RATE,
                channelConfig, audioFormat, mTrainingBufferSize);
        if (AudioRecord.STATE_INITIALIZED != mTrainingAudioRecord.getState()) {
            LogUtils.e(TAG, "startTrainingRecording: AudioRecord initialize failed");
            return;
        }

        mIsTerminatedRecording = false;

        mTrainingAudioRecord.startRecording();
        mTrainingRecordingThread = new Thread(new Runnable() {
            @Override
            public void run() {
                writeTrainingRecordingToTmpFile();
            }
        }, "training_recording_thread");

        // start recording thread
        mTrainingRecordingThread.start();
    }

    public void stopTrainingRecording(String outputFilePath) {
        LogUtils.d(TAG, "stopTrainingRecording: outputFilePath = " + outputFilePath);
        if (null == outputFilePath) {
            LogUtils.d(TAG, "stopTrainingRecording: invalid input param");
            return;
        }

        mIsTerminatedRecording = true;

        if (null != mTrainingAudioRecord) {
            try {
                mTrainingAudioRecord.stop();
                mTrainingAudioRecord.release();
            } catch (IllegalStateException  e) {
                e.printStackTrace();
            } finally {
                mTrainingAudioRecord = null;
            }

            // save recording to file
            saveTrainingRecordingToTargetFile(Global.PATH_TRAINING_RECORDING_TEMP_FILE,
                    outputFilePath);

            // delete temp file
            FileUtils.deleteFile(Global.PATH_TRAINING_RECORDING_TEMP_FILE);
        }
    }

    synchronized private void writeTrainingRecordingToTmpFile() {
        LogUtils.d(TAG, "writeTrainingRecordingToTmpFile: enter");
        String filePath = Global.PATH_TRAINING_RECORDING_TEMP_FILE;

        // create trainings directory if not exist
        FileUtils.createDirIfNotExists(Global.PATH_ROOT);
        FileUtils.createDirIfNotExists(Global.PATH_TRAINING_RECORDINGS);

        FileOutputStream outputStream = null;
        try {
            outputStream = new FileOutputStream(filePath, false);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }

        int readCount = 0;
        if (outputStream != null) {
            while (!mIsTerminatedRecording) {
                byte[] data = new byte[mTrainingBufferSize];
                if (null != mTrainingAudioRecord) {
                    readCount = mTrainingAudioRecord.read(data, 0,
                            mTrainingBufferSize);
                }

                LogUtils.d(TAG, "writeTrainingRecordingToTmpFile: readCount = "
                        + readCount);
                if (AudioRecord.ERROR_INVALID_OPERATION != readCount) {
                    try {
                        outputStream.write(data);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }

            try {
                outputStream.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private void saveTrainingRecordingToTargetFile(String inFilePath, String outFilePath) {
        FileInputStream inputStream;
        FileOutputStream outputStream;
        long dataLen;
        long dataAndHeaderLen;
        byte[] data = new byte[mTrainingBufferSize];

        try {
            inputStream = new FileInputStream(inFilePath);
            outputStream = new FileOutputStream(outFilePath);
            dataLen = inputStream.getChannel().size();
            dataAndHeaderLen = dataLen + WAV_HEADER_LENGTH;

            byte[] wavHeader = getWavHeader(dataLen, dataAndHeaderLen);
            outputStream.write(wavHeader, 0, WAV_HEADER_LENGTH);
            while (inputStream.read(data) != -1) {
                outputStream.write(data);
            }

            inputStream.close();
            outputStream.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public ShortBuffer[] getUserRecordings() {
        return mUserRecordings;
    }

    public void addUserRecording() {
        LogUtils.d(TAG, "addUserRecording: before add mRecordingCounter = "
                + mRecordingCounter);
        if (mRecordingCounter >= NUM_RECORDINGS_REQUIRED) {
            LogUtils.e(TAG, "addUserRecording: index is out of bound");
            return;
        }

        try {
            String filePath = getLastUserRecordingFilePath();
            mUserRecordings[mRecordingCounter++] = readWavFile(filePath);
        } catch (FileNotFoundException f) {
            LogUtils.d(TAG, "addUserRecording: error = " + f.getMessage());
        } catch (IOException e) {
            e.printStackTrace();
        }
        LogUtils.d(TAG, "addUserRecording: after add mRecordingCounter = "
                + mRecordingCounter);
    }

    // Returns a shortBuffer of the last user training recorded
    public ShortBuffer getLastUserRecording() {
        int num = getNumUserRecordings();
        LogUtils.d(TAG, "getLastUserRecording: recording number = " + num);
        if (num > NUM_RECORDINGS_REQUIRED) {
            num = NUM_RECORDINGS_REQUIRED;
        }

        if (num > 0) {
            num = num - 1;
        }

        return mUserRecordings[num];
    }

    // Returns a filepath for the last user training recorded
    public String getLastUserRecordingFilePath() {
        String recordingNumber = Integer.toString(mRecordingCounter + 1);
        String filePath = Global.PATH_TRAINING_RECORDINGS + "/"
                + FILE_NAME_PREFIX_OF_TRAINING_RECORDING + recordingNumber +
                Global.SUFFIX_WAV_FILE;
        LogUtils.d(TAG, "getLastUserRecordingFilePath: filePath= " + filePath);
        return filePath;
    }

    // Removes the recording files
    public void removeAllTrainingRecordingFiles() {
        for (int i = 0; i < NUM_RECORDINGS_REQUIRED; i++) {
            String recordingNumber = Integer.toString(i + 1);
            String filePath = Global.PATH_TRAINING_RECORDINGS + "/"
                    + FILE_NAME_PREFIX_OF_TRAINING_RECORDING + recordingNumber +
                    Global.SUFFIX_WAV_FILE;
            LogUtils.d(TAG, "removeAllTrainingRecordingFiles: filePath= "
                    + filePath);
            File recordingFile = new File(filePath);
            if (recordingFile.exists()) {
                recordingFile.delete();
            }
        }
    }

    // Resets the user recordings for the next training session
    public void removeUserRecordings() {
        LogUtils.d(TAG, "removeUserRecordings: getNumUserRecordings() before remove= "
                + getNumUserRecordings());
        Arrays.fill(mUserRecordings, null);
        mRecordingCounter = 0;
        LogUtils.d(TAG, "removeUserRecordings: getNumUserRecordings() after remove= "
                + getNumUserRecordings());
    }

    public int getNumUserRecordings() {
        return mRecordingCounter;
    }

    // Removes the last recording trained by a user when it's unsuccessful
    public void discardLastUserRecording() {
        LogUtils.d(TAG, "discardLastUserRecording: getNumUserRecordings() before discard = "
                + getNumUserRecordings());
        mRecordingCounter--;
        LogUtils.d(TAG, "discardLastUserRecording: getNumUserRecordings() after discard= "
                + getNumUserRecordings());
    }
}
