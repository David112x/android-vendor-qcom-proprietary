/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 *
 * Copyright (C) 2006 The Android Open Source Project
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

package vendor.qti.iwlan;

import android.util.Log;

import java.io.PrintWriter;
import java.time.LocalDateTime;
import java.util.ArrayDeque;

public class LogBuffer {
    private int max_size;
    private ArrayDeque<String> logs;

    public LogBuffer(int size) {
        max_size = size;
        logs = new ArrayDeque<String>(max_size);
    }

    public void addLog(String log) {
        synchronized (logs) {
            if(logs.size() == max_size)
                logs.remove();
            logs.add(LocalDateTime.now().toString() + " " + log);
        }
    }

    //Dumps all logs stored in buffer and clears buffer
    public void dump(PrintWriter pw) {
        synchronized(logs) {
            while (!logs.isEmpty()) {
                pw.println(logs.remove());
            }
            pw.flush();
        }
    }
}
