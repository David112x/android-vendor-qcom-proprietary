/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package com.android.bluetooth.ba;

import android.util.Log;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.List;

/**
 * Class used to send Broadcast Audio Stream Service Records.
 */
public final class BluetoothBAStreamServiceRecord {
    public static final String TAG = "BluetoothBAStreamServiceRecord";

    public static final int BSSR_TYPE_STREAM_ID       = 0;
    public static final int BSSR_TYPE_STREAM_ID_LEN   = 1;
    // actual values would be returned

    // security fields
    public static final int BSSR_TYPE_SECURITY_ID     = 1;
    public static final int BSSR_TYPE_SECURITY_ID_LEN = 2;
    //This stream uses the private key
    public static final long BSSR_SECURITY_KEY_TYPE_PRIVATE    = 0x0001;
    //This stream uses the temporary key
    public static final long BSSR_SECURITY_KEY_TYPE_TEMP       = 0x0002;
    //This stream does not use encryption
    public static final long BSSR_SECURITY_ENCRYPT_TYPE_NONE   = 0x0100;
    //This stream uses AESCCM encryption
    public static final long BSSR_SECURITY_ENCRYPT_TYPE_AESCCM = 0x0200;

    // codec type fields
    public static final int BSSR_TYPE_CODEC_TYPE_ID            = 2;
    public static final int BSSR_TYPE_CODEC_TYPE_ID_LEN        = 1;
    public static final long BSSR_CODEC_TYPE_CELT              = 0x01;       // CELT CODEC

    // CELT config values, defined below.
    public static final int BSSR_TYPE_CODEC_CONFIG_CELT_ID     = 3;// these values are further divided
    // into 3 values freq, frame_size and frame_samples.
    public static final int BSSR_TYPE_CODEC_CONFIG_CELT_ID_LEN = 6;

    public static final int BSSR_TYPE_SCMST_SUPPORT_ID         = 5;
    public static final int BSSR_TYPE_SCMST_SUPPORT_ID_LEN     = 1;
    //The recipient must not copy the data in this stream
    public static final long BSSR_SCMST_SUPPORT_COPY           = 0x01;
    //The recipient must not forward the data in this stream.
    public static final long BSSR_SCMST_SUPPORT_FORWARD        = 0x02;

    // Erasure Coding values
    public static final int BSSR_TYPE_ERASURE_CODE_ID     = 6;
    public static final int BSSR_TYPE_ERASURE_CODE_ID_LEN = 1;
    public static final long BSSR_ERASURE_CODE_NONE       = 0x00;//No erasure coding in this stream
    public static final long BSSR_ERASURE_CODE_2_5        = 0x01;//The stream has a 2,5 coding scheme
    public static final long BSSR_ERASURE_CODE_3_7        = 0x02;//The stream has a 3,7 coding scheme
    public static final long BSSR_ERASURE_CODE_3_8        = 0x03;//The stream has a 3,8 coding scheme
    public static final long BSSR_ERASURE_CODE_3_9        = 0x04;//The stream has a 3,9 coding scheme

    public static final int BSSR_TYPE_CHANNELS_ID         = 7;
    public static final int BSSR_TYPE_CHANNELS_ID_LEN     = 2;
    public static final long BSSR_CHANNELS_MONO           = 0x0001;//This stream is mono
    public static final long BSSR_CHANNELS_STEREO         = 0x0004;//This stream is stereo

    public static final int BSSR_TYPE_SAMPLE_SIZE_ID      = 8;
    public static final int BSSR_TYPE_SAMPLE_SIZE_ID_LEN  = 1;
    public static final long BSSR_SAMPLE_SIZE_8_BIT       = 0x01;//This stream is 8-bit samples
    public static final long BSSR_SAMPLE_SIZE_16_BIT      = 0x02;//This stream is 16-bit samples
    public static final long BSSR_SAMPLE_SIZE_24_BIT      = 0x04;//This stream id 24-bit samples

    public static final int BSSR_TYPE_AFH_UPDATE_METHOD_ID           = 9;
    public static final int BSSR_TYPE_AFH_UPDATE_METHOD_ID_LEN       = 1;
    //This stream does not support AFH channel map updates
    public static final long BSSR_AFH_CHANNEL_MAP_UPDATE_METHOD_NONE = 0x00;
    //This stream uses SCM to transport AFH channel map updates from broadcaster to receivers.
    public static final long BSSR_AFH_CHANNEL_MAP_UPDATE_METHOD_SCM  = 0x01;
    //This stream uses the triggered CSB sync train method to transport AFH channel map
    // updates from broadcaster to receivers.
    public static final long BSSR_AFH_CHANNEL_MAP_UPDATE_METHOD_TRIGGERED_SYNC_TRAIN = 0x02;

    public static final int BSSR_TYPE_CODEC_CONFIG_CELT_FREQ_ID     = 10;
    public static final int BSSR_TYPE_CODEC_CONFIG_CELT_FREQ_ID_LEN = 2;
    public static final long BSSR_CODEC_FREQ_8KHZ                   = 0x0001;
    public static final long BSSR_CODEC_FREQ_11025HZ                = 0x0002;
    public static final long BSSR_CODEC_FREQ_12KHZ                  = 0x0004;
    public static final long BSSR_CODEC_FREQ_16KHZ                  = 0x0008;
    public static final long BSSR_CODEC_FREQ_22050HZ                = 0x0010;
    public static final long BSSR_CODEC_FREQ_24KHZ                  = 0x0020;
    public static final long BSSR_CODEC_FREQ_32KHZ                  = 0x0040;
    public static final long BSSR_CODEC_FREQ_44100HZ                = 0x0080;
    public static final long BSSR_CODEC_FREQ_48KHZ                  = 0x0100;
    public static final long BSSR_CODEC_FREQ_64KHZ                  = 0x0200;
    public static final long BSSR_CODEC_FREQ_88200HZ                = 0x0400;
    public static final long BSSR_CODEC_FREQ_96KHZ                  = 0x0800;
    public static final long BSSR_CODEC_FREQ_128KHZ                 = 0x1000;
    public static final long BSSR_CODEC_FREQ_176400HZ               = 0x2000;
    public static final long BSSR_CODEC_FREQ_192KHZ                 = 0x4000;

    public static final int BSSR_TYPE_CODEC_CONFIG_CELT_FRAME_SIZE_ID        = 11;
    public static final int BSSR_TYPE_CODEC_CONFIG_CELT_FRAME_SIZE_ID_LEN    = 2;
    // actual values would be returned

    public static final int BSSR_TYPE_CODEC_CONFIG_CELT_FRAME_SAMPLES_ID     = 12;
    public static final int BSSR_TYPE_CODEC_CONFIG_CELT_FRAME_SAMPLES_ID_LEN = 2;
    // actual values would be returned

    /*
    * Every single record will be a HashMap of a number of record.
    * We will have a List of HashMap to send all possible values of stream records.
    */
    //private Map<Integer, Long> mServiceRecord = new HashMap<Integer, Long>();
    int mNumRecords;
    private List<Map<Integer, Long>> mServiceRecordList = new ArrayList<Map<Integer, Long>>();

    /**
     * Create a new BA Service Record Object.
     *
     * @param number of records
     */
    public BluetoothBAStreamServiceRecord(int numRec) {
        mNumRecords = numRec;
    }

    /**
     * Get number of records.
     *
     * @return number of records
     */
    public int getNumRecords() {
        return mNumRecords;
    }

    /**
     * Add a record value.
     *
     * @param streamId: streamId of the record.
     * @param recordAttribId: one of the record attribute as mentioned above.
     * @param recordAttribVal: one of the record attribute values as mentioned above.
     */
    public void addServiceRecordValue(Long streamId, int recordAttribId, Long recordAttribVal) {
        // find streamId in the list.
        if (!mServiceRecordList.isEmpty()) {
            for (Map<Integer, Long> mServiceRecord: mServiceRecordList) {
                if (mServiceRecord.containsKey(BSSR_TYPE_STREAM_ID) &&
                        mServiceRecord.get(BSSR_TYPE_STREAM_ID).equals(streamId)) {
                    mServiceRecord.put(recordAttribId, recordAttribVal);
                    return;
                }
            }
        }
        // either list is empty or matching record not found.
        Map<Integer, Long> mServiceRecord = new HashMap<Integer, Long>();
        mServiceRecord.put(BSSR_TYPE_STREAM_ID, streamId);
        mServiceRecord.put(recordAttribId, recordAttribVal);
        mServiceRecordList.add(mServiceRecord);
    }

    /**
     * Add a record .
     *
     * @param serviceRecord: a Map of service attribute id and attribute values.
     */
    public void addServiceRecord(Map<Integer, Long> mServiceRecord) {
        // if a record with same stream_id is existing, we will remove old record and add new one.
        // We are not going to change this record.
        if(mServiceRecordList.isEmpty()) {
            mServiceRecordList.add(mServiceRecord);
            return;
        }
        // check if we have record with same stream id
        for (Map<Integer, Long> mRecord: mServiceRecordList) {
            if (mRecord.containsKey(BSSR_TYPE_STREAM_ID) &&
                mRecord.get(BSSR_TYPE_STREAM_ID).equals(mServiceRecord.get(BSSR_TYPE_STREAM_ID))) {
                // delete this record from List
                mServiceRecordList.remove(mRecord);
            }
        }
        // either record is not found, or removed.
        mServiceRecordList.add(mServiceRecord);
    }

    /**
     * Get record values.
     *
     * @param streamId: streamId of the record.
     * @param recordAttribId: one of the record attribute as mentioned above.
     * @return one of the record attribute values as mentioned above, 0 otherwise
     */
    public Long getServiceRecordValue(Long streamId, int recordAttribId) {
        // find streamId in the list.
        if (!mServiceRecordList.isEmpty()) {
            for (Map<Integer, Long> mServiceRecord: mServiceRecordList) {
                if (mServiceRecord.containsKey(BSSR_TYPE_STREAM_ID) &&
                        mServiceRecord.get(BSSR_TYPE_STREAM_ID).equals(streamId)) {
                    return mServiceRecord.get(recordAttribId);
                }
            }
        }
        // either list is empty or matching record not found.
        return new Long(0);
    }

    /**
     * Get record .
     *
     * @param streamId: streamId of the Record to be fetched.
     * @return servicerecord if streamId matches, empty record otherwise;
     */
    public Map<Integer, Long> getServiceRecord( Long streamId) {
        if(mServiceRecordList.isEmpty())
            return null;
        for (Map<Integer, Long> mServiceRecord: mServiceRecordList) {
            if (mServiceRecord.containsKey(BSSR_TYPE_STREAM_ID) &&
                    mServiceRecord.get(BSSR_TYPE_STREAM_ID).equals(streamId)) {
                return mServiceRecord;
            }
        }
        // can't find record, return empty record
        return null;
    }

    /**
     * Get all stored streamIds .
     *
     * @return array of all streamIds
     */
    public Long[] getStreamIds() {
        if(mServiceRecordList.isEmpty())
            return null;
        Long[] streamIdList = new Long[mServiceRecordList.size()];
        int k = 0;
        for (Map<Integer, Long> mServiceRecord: mServiceRecordList) {
            if (mServiceRecord.containsKey(BSSR_TYPE_STREAM_ID))
                streamIdList[k++] = mServiceRecord.get(BSSR_TYPE_STREAM_ID);
        }
        return streamIdList;
    }
}