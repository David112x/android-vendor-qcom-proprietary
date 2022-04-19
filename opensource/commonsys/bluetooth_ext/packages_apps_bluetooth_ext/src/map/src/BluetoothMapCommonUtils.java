/*
* Copyright (c) 2015, The Linux Foundation. All rights reserved.
* Not a Contribution.
* Copyright (C) 2013 Samsung System LSI
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
package com.android.bluetooth.map;

import java.nio.charset.IllegalCharsetNameException;
import java.nio.charset.Charset;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.Locale;
import java.util.UUID;


import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;

import android.text.util.Rfc822Token;
import android.text.util.Rfc822Tokenizer;
import android.util.Base64;
import java.util.Random;
import android.util.Log;


public abstract class BluetoothMapCommonUtils {

    protected static String TAG = "BluetoothMapCommonUtils";

    static final String EMAIL_UI_PKG = "com.android.email";

    public static class BMsgReaderExt extends BluetoothMapbMessage.BMsgReader{

        public BMsgReaderExt(InputStream is)
        {
            super(is);
        }
        private byte[] getLineTerminatorAsBytes() {
            int readByte;
            // Donot skip Empty Line.
            ByteArrayOutputStream output = new ByteArrayOutputStream();
            try {
                while ((readByte = mInStream.read()) != -1) {
                    if (readByte == '\r') {
                        if ((readByte = mInStream.read()) != -1 && readByte == '\n') {
                            if (output.size() == 0) {
                                Log.v(TAG,"outputsize 0");
                                output.write('\r');
                                output.write('\n');
                            }
                            break;
                        } else {
                            output.write('\r');
                        }
                    }
                    output.write(readByte);
                }
            } catch (IOException e) {
                Log.w(TAG, e);
                return null;
            }
            return output.toByteArray();
        }

        /**
         * Read a line of text from the BMessage including empty lines.
         * @return the next line of text, or null at end of file, or if UTF-8 is not supported.
         * @hide
         */
        public String getLineTerminator() {
            try {
                byte[] line = getLineTerminatorAsBytes();
                if (line == null) {
                    return null;
                } else if (line.length == 0){
                    return null;
                } else {
                    return new String(line, "UTF-8");
                }
            } catch (UnsupportedEncodingException e) {
                Log.w(TAG, e);
                return null;
            }
        }

        /**
         * Read a part of BMessage including empty lines for last occurence of  terminator
         * @return the string till terminator, or null at end of file, or if UTF-8 is not supported
         * @hide
         */
        public String getLastStringTerminator(String terminator) {
            StringBuilder dataStr = new StringBuilder();
            String lineCur = getLineTerminator();
            while ( lineCur != null ) {
                String firstOccur = getStringTerminator(terminator);
                if (firstOccur != null ) {
                    if (dataStr.length() != 0 ) {
                        dataStr.append(terminator);
                        dataStr.append("\r\n");
                    }
                    dataStr.append(lineCur);
                    if (!lineCur.equals("\r\n")) {
                        dataStr.append("\r\n");
                    }
                    dataStr.append(firstOccur);
                } else {
                    //No more occureences of terminator
                    break;
                }
                lineCur = getLineTerminator();
            }
            return dataStr.toString();
        }
        /**
         * Read a part of BMessage including empty lines till terminator
         * @return the string till terminator, or null at end of file, or if UTF-8 is not supported
         * @hide
         */
        public String getStringTerminator(String terminator) {
            StringBuilder dataStr= new StringBuilder();
            String lineCur = getLineTerminator();
            while( lineCur != null && (!lineCur.equals(terminator))) {
                 dataStr.append(lineCur);
                 if (!lineCur.equals("\r\n")) {
                    dataStr.append("\r\n");
                 }
                 lineCur = getLineTerminator();
             }
             //Return string if only terminator is present.
             if ( lineCur != null && lineCur.equals(terminator)) {
                 return dataStr.toString();
             } else {
                 return null;
             }
        }
    }
}
