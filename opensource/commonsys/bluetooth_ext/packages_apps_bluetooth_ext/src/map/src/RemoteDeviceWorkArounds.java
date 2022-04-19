/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of The Linux Foundation nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
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

package com.android.bluetooth.map;

import android.util.Log;
import android.util.Xml;

import com.android.internal.util.FastXmlSerializer;

import java.io.StringWriter;

import org.xmlpull.v1.XmlSerializer;

/*
 * RemoteDeviceWorkArounds is used to provide fix based
 * on BD_ADDRESS of remote device for IOT issues.
 */
public class RemoteDeviceWorkArounds {

    private static final String TAG = "MAP RemoteDeviceWorkArounds";

    public static final String BENZ_CARKIT = "00:26:e8";
    public static final String BREZZA_CARKIT = "28:a1:83";
    public static final String HONDA_CARKIT = "64:D4:BD";
    public static final String PCM_CARKIT = "9C:DF:03";
    public static final String SYNC_CARKIT = "D0:39:72";
    public static final String FORD_SYNC_CARKIT ="00:1E:AE";

    /* added to apply message body fixes depending on remote carkit*/
    public static String smsBodyChecks(String remoteAddress, String smsBody) {
        Log.d(TAG,"messageBodyChecks(): Remote Device BD Address ="+remoteAddress);

        /* Fix IOT issue with PCM carkit where carkit is unable to parse
         * message if carriage return is present in it */
        if (addresStartsWith(remoteAddress,PCM_CARKIT)) {
            smsBody = smsBody.replaceAll("\r", "");
        /* Fix Message Display issue with FORD SYNC carkit -
         * Remove line feed and include only carriage return */
        } else if (addresStartsWith(remoteAddress,FORD_SYNC_CARKIT)) {
            smsBody = smsBody.replaceAll("\n", "");
        /* Fix IOT issue to remove trailing line feeds in the message body */
        } else if (addresStartsWith(remoteAddress,SYNC_CARKIT) && smsBody.length() > 0) {
            int trailingLF = 0;
            while ((smsBody.charAt(smsBody.length() - trailingLF - 1)) == '\n')
                trailingLF ++;
            smsBody = smsBody.substring(0, (smsBody.length() - trailingLF));
        }
        return smsBody;
    }

    /* to find out if bluetooth address belongs to particular remote carkit*/
    public static boolean addresStartsWith(String bdAddress, String remoteAddr) {
           return bdAddress.startsWith(remoteAddr);
    }

    /* to set xml serializer depending on carkit */
    public static XmlSerializer setXmlSerializer(boolean isBenzCarkit) {
        if (isBenzCarkit) {
            Log.d(TAG, "Remote is BENZ CARKIT: " + isBenzCarkit);
            return Xml.newSerializer();
        }
        return new FastXmlSerializer();
    }

    /* Fix IOT issue to replace '&amp;' by '&', &lt; by < and '&gt; by '>' in MessageListing*/
    public static String handleXmlDelimeters(StringWriter sw, String remoteAddress) {
        if (remoteAddress.startsWith(BREZZA_CARKIT))
            return sw.toString().replaceAll("&amp;","&").replaceAll("&lt;","<")
                .replaceAll("&gt;",">");
        return null;
    }
}

