/*
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.qualcomm.qti.wigig;

import android.os.Parcel;
import android.os.Parcelable;

import java.io.ByteArrayOutputStream;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CoderResult;
import java.nio.charset.CodingErrorAction;
import java.util.Locale;

/**
 * Stores SSID octets and handles conversion.
 *
 * For Ascii encoded string, any octet < 32 or > 127 is encoded as
 * a "\x" followed by the hex representation of the octet.
 * Exception chars are ", \, \e, \n, \r, \t which are escaped by a \
 * See src/utils/common.c for the implementation in the supplicant.
 *
 * @hide
 */
public class WigigSsid implements Parcelable {
    private static final String TAG = "WigigSsid";

    public final ByteArrayOutputStream octets = new ByteArrayOutputStream(32);

    private static final int HEX_RADIX = 16;
    public static final String NONE = "<unknown ssid>";

    private WigigSsid() {
    }

    public static WigigSsid createFromAsciiEncoded(String asciiEncoded) {
        WigigSsid a = new WigigSsid();
        a.convertToBytes(asciiEncoded);
        return a;
    }

    public static WigigSsid createFromHex(String hexStr) {
        WigigSsid a = new WigigSsid();
        if (hexStr == null) return a;

        if (hexStr.startsWith("0x") || hexStr.startsWith("0X")) {
            hexStr = hexStr.substring(2);
        }

        for (int i = 0; i < hexStr.length()-1; i += 2) {
            int val;
            try {
                val = Integer.parseInt(hexStr.substring(i, i + 2), HEX_RADIX);
            } catch(NumberFormatException e) {
                val = 0;
            }
            a.octets.write(val);
        }
        return a;
    }

    /* This function is equivalent to printf_decode() at src/utils/common.c in
     * the supplicant */
    private void convertToBytes(String asciiEncoded) {
        int i = 0;
        int val = 0;
        while (i< asciiEncoded.length()) {
            char c = asciiEncoded.charAt(i);
            switch (c) {
                case '\\':
                    i++;
                    switch(asciiEncoded.charAt(i)) {
                        case '\\':
                            octets.write('\\');
                            i++;
                            break;
                        case '"':
                            octets.write('"');
                            i++;
                            break;
                        case 'n':
                            octets.write('\n');
                            i++;
                            break;
                        case 'r':
                            octets.write('\r');
                            i++;
                            break;
                        case 't':
                            octets.write('\t');
                            i++;
                            break;
                        case 'e':
                            octets.write(27); //escape char
                            i++;
                            break;
                        case 'x':
                            i++;
                            try {
                                val = Integer.parseInt(asciiEncoded.substring(i, i + 2), HEX_RADIX);
                            } catch (NumberFormatException e) {
                                val = -1;
                            }
                            if (val < 0) {
                                val = Character.digit(asciiEncoded.charAt(i), HEX_RADIX);
                                if (val < 0) break;
                                octets.write(val);
                                i++;
                            } else {
                                octets.write(val);
                                i += 2;
                            }
                            break;
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                            val = asciiEncoded.charAt(i) - '0';
                            i++;
                            if (asciiEncoded.charAt(i) >= '0' && asciiEncoded.charAt(i) <= '7') {
                                val = val * 8 + asciiEncoded.charAt(i) - '0';
                                i++;
                            }
                            if (asciiEncoded.charAt(i) >= '0' && asciiEncoded.charAt(i) <= '7') {
                                val = val * 8 + asciiEncoded.charAt(i) - '0';
                                i++;
                            }
                            octets.write(val);
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    octets.write(c);
                    i++;
                    break;
            }
        }
    }

    @Override
    public String toString() {
        byte[] ssidBytes = octets.toByteArray();
        // Supplicant returns \x00\x00\x00\x00\x00\x00\x00\x00 hex string
        // for a hidden access point. Make sure we maintain the previous
        // behavior of returning empty string for this case.
        if (octets.size() <= 0 || isArrayAllZeroes(ssidBytes)) return "";
        // TODO: Handle conversion to other charsets upon failure
        Charset charset = Charset.forName("UTF-8");
        CharsetDecoder decoder = charset.newDecoder()
                .onMalformedInput(CodingErrorAction.REPLACE)
                .onUnmappableCharacter(CodingErrorAction.REPLACE);
        CharBuffer out = CharBuffer.allocate(32);

        CoderResult result = decoder.decode(ByteBuffer.wrap(ssidBytes), out, true);
        out.flip();
        if (result.isError()) {
            return NONE;
        }
        return out.toString();
    }

    private boolean isArrayAllZeroes(byte[] ssidBytes) {
        for (int i = 0; i< ssidBytes.length; i++) {
            if (ssidBytes[i] != 0) return false;
        }
        return true;
    }

    /** @hide */
    public boolean isHidden() {
        return isArrayAllZeroes(octets.toByteArray());
    }

    /** @hide */
    public byte[] getOctets() {
        return octets.toByteArray();
    }

    /** @hide */
    public String getHexString() {
        String out = "0x";
        byte[] ssidbytes = getOctets();
        for (int i = 0; i < octets.size(); i++) {
            out += String.format(Locale.US, "%02x", ssidbytes[i]);
        }
        return out;
    }

    /** Implement the Parcelable interface {@hide} */
    public int describeContents() {
        return 0;
    }

    /** Implement the Parcelable interface {@hide} */
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(octets.size());
        dest.writeByteArray(octets.toByteArray());
    }

    /** Implement the Parcelable interface {@hide} */
    public static final Creator<WigigSsid> CREATOR =
        new Creator<WigigSsid>() {
            public WigigSsid createFromParcel(Parcel in) {
                WigigSsid ssid = new WigigSsid();
                int length = in.readInt();
                byte b[] = new byte[length];
                in.readByteArray(b);
                ssid.octets.write(b, 0, length);
                return ssid;
            }

            public WigigSsid[] newArray(int size) {
                return new WigigSsid[size];
            }
        };
}
