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

import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

/**
 * Class used to identify 128 bit Encryption Key for BA.
 *
 * {@hide}
 */
public final class BluetoothBAEncryptionKey implements Parcelable {
    public static final String TAG = "BluetoothBAEncryptionKey";

    private int mFlagType;
    public static int ENCRYPTION_KEY_LENGTH = 16;// key len in bytes
    public static int SECURITY_KEY_TYPE_PRIVATE = 0x0001;
    public static int SECURITY_KEY_TYPE_TEMP    = 0x0002;
    public static int SECURITY_KEY_FORWARD_ENABLED     = 0x0080;
    private byte[] mEncryptionKey = new byte[ENCRYPTION_KEY_LENGTH];

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
        for (int k =0; k < ENCRYPTION_KEY_LENGTH; k++) {
            out.writeByte(mEncryptionKey[k]);
        }
        out.writeInt(mFlagType);
    }

    public static final Parcelable.Creator<BluetoothBAEncryptionKey> CREATOR
            = new Parcelable.Creator<BluetoothBAEncryptionKey>() {
        public BluetoothBAEncryptionKey createFromParcel(Parcel in) {
            return new BluetoothBAEncryptionKey(in);
        }

        public BluetoothBAEncryptionKey[] newArray(int size) {
            return new BluetoothBAEncryptionKey[size];
        }
    };

    private BluetoothBAEncryptionKey(Parcel in) {
        for (int i = 0; i < ENCRYPTION_KEY_LENGTH; i++) {
            mEncryptionKey[i] = in.readByte();
        }
        mFlagType = in.readInt();
    }

    /**
     * Create a new BluetoothBAEncryptionKey object.
     *
     * @param byte array contianing encryption key
     */
    public BluetoothBAEncryptionKey(byte[] mEncKey, int flagType) {
        for (int i = 0; i < ENCRYPTION_KEY_LENGTH; i++) {
            mEncryptionKey[i] = mEncKey[i];
        }
        mFlagType = flagType;
    }

    /**
     * Get the encryption key.
     *
     * @return byte array containing encryption key.
     */
    public byte[] getEncryptionKey() {
        return mEncryptionKey;
    }

    public int getFlagType() {
        return mFlagType;
    }
}