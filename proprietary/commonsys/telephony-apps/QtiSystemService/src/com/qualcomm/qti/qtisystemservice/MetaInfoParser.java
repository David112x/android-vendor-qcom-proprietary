/*
 * Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a contribution.
 *
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.qualcomm.qti.qtisystemservice;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;

import org.json.JSONException;
import org.json.JSONObject;

public class MetaInfoParser {
    // Deprecated reading of vendor firware path "/vendor/firmware_mnt/verinfo/ver_info.txt"
    private static final String FILENAME_META_VERSION = "/data/vendor/modem_config/ver_info.txt";

    private String mMetaInfo;

    private MetaInfoParser() {
        String metaId = "";
        try {
            String meta = readFile(FILENAME_META_VERSION);
            try {
                JSONObject metaInfo = new JSONObject(meta).getJSONObject("Metabuild_Info");;
                metaId = metaInfo.getString("Meta_Build_ID");
            } catch (JSONException e) {
                metaId = readLine(FILENAME_META_VERSION);
            }
        } catch (IOException ioe) {
            // Fail quietly, as the file may not exist on some devices.
        } catch (NumberFormatException nfe) {
            // Fail quietly, returning empty string should be sufficient
        }
        mMetaInfo = metaId;
    }

    public static MetaInfoParser getInstance() {
        return SingletonHolder.sInstance;
    }

    private String readFile(String filePath) throws IOException {
        InputStreamReader inputStreamReader = new InputStreamReader(
                new FileInputStream(filePath), "UTF-8");
        StringBuilder stringBuilder = new StringBuilder();
        BufferedReader bufferedReader = new BufferedReader(inputStreamReader);
        String line;
        while((line = bufferedReader.readLine()) != null) {
            stringBuilder.append(line);
            stringBuilder.append('\n');
        }
        bufferedReader.close();
        return stringBuilder.toString();
    }

    private String readLine(String filename) throws IOException {
        BufferedReader reader = new BufferedReader(new FileReader(filename),
                256);
        try {
            return reader.readLine();
        } finally {
            reader.close();
        }
    }

    public String getMetaInfo() {
        return mMetaInfo;
    }

    private static class SingletonHolder {
        public final static MetaInfoParser sInstance = new MetaInfoParser();
    }
}
