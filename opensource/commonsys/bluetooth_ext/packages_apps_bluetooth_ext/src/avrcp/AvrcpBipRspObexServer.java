/*
* Copyright (c) 2015-2016, 2018 The Linux Foundation. All rights reserved.
* Not a Contribution.
*
* Copyright (C) 2014 Samsung System LSI
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

package com.android.bluetooth.avrcp;

import java.io.IOException;
import java.io.OutputStream;
import java.util.Arrays;

import javax.obex.HeaderSet;
import javax.obex.Operation;
import javax.obex.ResponseCodes;
import javax.obex.ServerRequestHandler;

import android.content.Context;
import android.text.TextUtils;
import android.util.Log;

public class AvrcpBipRspObexServer extends ServerRequestHandler {

    private String TAG = "AvrcpBipObex";
    private final boolean D = true;
    private static final boolean V = AvrcpBipRsp.V;
    private final int LENGTH_UUID = 16;
    private Context mContext = null;
    private AvrcpBipRspParser mAvrcpBipRspParser;
    private boolean mAborted;
    private boolean mConnected;

    // 128 bit UUID for Cover Art
    private final byte[] BIP_RESPONDER = new byte[] {
             (byte)0x71, (byte)0x63, (byte)0xDD, (byte)0x54,
             (byte)0x4A, (byte)0x7E, (byte)0x11, (byte)0xE2,
             (byte)0xB4, (byte)0x7C, (byte)0x00, (byte)0x50,
             (byte)0xC2, (byte)0x49, (byte)0x00, (byte)0x48
             };

    /* Get Request types */
    private final String TYPE_GET_IMAGE               = "x-bt/img-img";
    private final String TYPE_GET_IMAGE_PROPERTIES    = "x-bt/img-properties";
    private final String TYPE_GET_LINKED_THUMBNAIL    = "x-bt/img-thm";

    /**
     * Represents the OBEX Image handle header. This is (null terminated, UTF-16 encoded Unicode
     * text length prefixed with a two-byte unsigned integer.
     * <P>
     * The value of <code>IMG_HANDLE</code> is 0x30 (48).
     */
    private final int IMG_HANDLE = 0x30;

    /**
     * Represents the OBEX Image Descriptor header. This is the byte sequence, length prefixed
     * with a two-byte unsigned integer.
     * <P>
     * The value of <code>IMG_DESCRIPTOR</code> is 0x71 (113).
     */
    private final int IMG_DESCRIPTOR = 0x71;

    private AvrcpTgBipStateMachine mStateMachine;

    public AvrcpBipRspObexServer(Context context, AvrcpTgBipStateMachine stateMachine) {
        mContext = context;
        mStateMachine = stateMachine;
        TAG += mStateMachine.getSmTag();
        mAvrcpBipRspParser = new AvrcpBipRspParser(mContext,
                mStateMachine.getRemoteDevice().getAddress());
        if (V) Log.v(TAG," AvrcpBipRspObexServer");
    }


    @Override
    public boolean isSrmSupported() {
        return true;
    }

    @Override
    public int onConnect(final HeaderSet request, HeaderSet reply) {
        if (D) Log.d(TAG, "onConnect");
        if (V) logHeader(request);
        try {
            byte[] uuid = (byte[])request.getHeader(HeaderSet.TARGET);
            if (uuid == null) {
                return ResponseCodes.OBEX_HTTP_NOT_ACCEPTABLE;
            }
            StringBuilder sb = new StringBuilder();
            for (byte b : uuid) {
                sb.append(String.format("%02X ", b));
            }
            if (D) Log.d(TAG, "onConnect uuid=" + sb.toString());

            if (uuid.length != LENGTH_UUID || !Arrays.equals(uuid, BIP_RESPONDER)) {
                Log.w(TAG, "invalid UUID");
                return ResponseCodes.OBEX_HTTP_NOT_ACCEPTABLE;
            }
            reply.setHeader(HeaderSet.WHO, uuid);
        } catch (IOException e) {
            Log.e(TAG,"onConnect OBEX_HTTP_INTERNAL_ERROR:", e);
            return ResponseCodes.OBEX_HTTP_INTERNAL_ERROR;
        }

        try {
            byte[] remote = (byte[])request.getHeader(HeaderSet.WHO);
            if (remote != null) {
                if (V) Log.d(TAG, "onConnect(): remote=" + Arrays.toString(remote));
                reply.setHeader(HeaderSet.TARGET, remote);
            }
        } catch (IOException e) {
            Log.e(TAG,"Exception during onConnect:", e);
            return ResponseCodes.OBEX_HTTP_INTERNAL_ERROR;
        }
        if (D) Log.d(TAG, "onConnect(): returning OBEX_HTTP_OK");
        mConnected = true;
        mAborted = false;
        return ResponseCodes.OBEX_HTTP_OK;
    }

    @Override
    public void onDisconnect(final HeaderSet req, final HeaderSet resp) {
        if (D) Log.d(TAG, "onDisconnect ");
        resp.responseCode = ResponseCodes.OBEX_HTTP_OK;
        mConnected = false;
    }

    @Override
    public int onAbort(HeaderSet request, HeaderSet reply) {
        if (D) Log.d(TAG, "onAbort ");
        mAborted = true;
        return ResponseCodes.OBEX_HTTP_OK;
    }

    @Override
    public void onClose() {
        if (D) Log.d(TAG, "onClose ");
        mStateMachine.sendMessage(AvrcpTgBipStateMachine.DISCONNECT);
        mConnected = false;
    }

    @Override
    public int onGet(Operation op) {
        HeaderSet request;
        String type;
        String imgHandle = null;
        int responseCode = ResponseCodes.OBEX_HTTP_BAD_REQUEST;
        if (D) Log.d(TAG, "onGet() start ");
        try {
            request = op.getReceivedHeader();
            type = (String)request.getHeader(HeaderSet.TYPE);
            imgHandle = request.getHeader(IMG_HANDLE).toString();
            if (V) logHeader(request);
            if (D) Log.d(TAG, "OnGet type :" + type + " imgHandle :" + imgHandle);
            if ( TextUtils.isEmpty(type) || TextUtils.isEmpty(imgHandle)) {
                Log.w(TAG, "type or IMG_HANDLE is null, returning OBEX_HTTP_BAD_REQUEST");
                responseCode = ResponseCodes.OBEX_HTTP_BAD_REQUEST;
            } else if(!(mAvrcpBipRspParser.isImgHandleValid(imgHandle))){
                Log.w(TAG, "invalid imgHandle = " + imgHandle);
                responseCode = ResponseCodes.OBEX_HTTP_NOT_ACCEPTABLE;
            } else if(type.equals(TYPE_GET_IMAGE)) {
                String imgDescXmlString = null;
                if (request.getHeader(IMG_DESCRIPTOR) != null) {
                    imgDescXmlString = new String((byte [])request.getHeader(IMG_DESCRIPTOR));
                    if (D) Log.d(TAG, "imgDescXmlString = "  + imgDescXmlString);
                }
                responseCode = getImgRsp(op, imgHandle, imgDescXmlString);
            } else if (type.equals(TYPE_GET_IMAGE_PROPERTIES)) {
                responseCode = getImgPropertiesRsp(op, imgHandle);
            } else if (type.equals(TYPE_GET_LINKED_THUMBNAIL)) {
                responseCode = getImgThumbRsp(op, imgHandle);
            }
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "IllegalArgumentException :", e);
            responseCode = ResponseCodes.OBEX_HTTP_PRECON_FAILED;
        } catch (Exception e) {
            Log.e(TAG, "Exception occured while handling request", e);
        }
        if(D) Log.d(TAG," onGet return responseCode :" + responseCode);
        return responseCode;
    }

    String getImgHandleFromTitle(String title) {
        if (!mConnected) {
            if (D) Log.d(TAG, "getImgHandleFromTitle, Session not established retrun empty");
            return "";
        }
        return mAvrcpBipRspParser.getImgHandleFromTitle(title);
    }

    String getImgHandle(String albumName) {
        if (D) Log.d(TAG, "getImgHandle albumName: " + albumName);
        if (mAvrcpBipRspParser == null || albumName == null || (!mConnected)) {
            if (D) Log.d(TAG, "getImgHandle: mAvrcpBipRspParser :" + mAvrcpBipRspParser
                    + " albumName :" + albumName + " mConnected :" + mConnected);
            return "";
        }
        return mAvrcpBipRspParser.getImgHandle(albumName);
    }

    private final void logHeader(HeaderSet hs) {
        Log.v(TAG, "Dumping HeaderSet " + hs.toString());
        try {
            Log.v(TAG, "CONNECTION_ID : " + hs.getHeader(HeaderSet.CONNECTION_ID));
            Log.v(TAG, "NAME : " + hs.getHeader(HeaderSet.NAME));
            Log.v(TAG, "TYPE : " + hs.getHeader(HeaderSet.TYPE));
            Log.v(TAG, "TARGET : " + hs.getHeader(HeaderSet.TARGET));
            Log.v(TAG, "WHO : " + hs.getHeader(HeaderSet.WHO));
            Log.v(TAG, "IMAGE HANDLE : " + hs.getHeader(IMG_HANDLE));
            Log.v(TAG, "IMAGE DESCRIPTOR : " + hs.getHeader(IMG_DESCRIPTOR));
        } catch (IOException e) {
            Log.e(TAG, "dump HeaderSet error " + e);
        }
    }

    private int getImgPropertiesRsp(Operation op, String imgHandle) {
        OutputStream outStream = null;
        byte[] outBytes = null;
        int maxChunkSize, bytesWritten = 0;
        int bytesToWrite;
        try {
            outBytes = mAvrcpBipRspParser.encode(imgHandle);
            outStream = op.openOutputStream();
        } catch (IOException e1) {
            Log.w(TAG, "getImgPropertiesRsp: sending OBEX_HTTP_BAD_REQUEST ", e1);
            return ResponseCodes.OBEX_HTTP_BAD_REQUEST;
        } catch (IllegalArgumentException e1) {
            Log.w(TAG,"getImgPropertiesRsp: sending OBEX_HTTP_PRECON_FAILED", e1);
            if(outStream != null) {
                try {
                    outStream.close();
                } catch (IOException e) {
                    Log.w(TAG,"getImgPropertiesRsp", e);
                }
            }
            return ResponseCodes.OBEX_HTTP_PRECON_FAILED;
        }

        maxChunkSize = op.getMaxPacketSize(); // This must be called after setting the headers.
        if(outBytes != null) {
            try {
                while (bytesWritten < outBytes.length) {
                    bytesToWrite = Math.min(maxChunkSize, outBytes.length - bytesWritten);
                    outStream.write(outBytes, bytesWritten, bytesToWrite);
                    bytesWritten += bytesToWrite;
                }
            } catch (IOException e) {
                // We were probably aborted or disconnected
                Log.w(TAG,"getImgPropertiesRsp: IOException:", e);
            } finally {
                if(outStream != null) { try { outStream.close(); } catch (IOException e) {} }
            }
            if(V)
                Log.v(TAG,"getImgPropertiesRsp sent " + bytesWritten +" bytes out of "
                + outBytes.length);
            if(bytesWritten == outBytes.length)
                return ResponseCodes.OBEX_HTTP_OK;
            else
                return ResponseCodes.OBEX_HTTP_BAD_REQUEST;
        }
        return ResponseCodes.OBEX_HTTP_BAD_REQUEST;

    }

    private int getImgThumbRsp(Operation op, String imgHandle) {
        OutputStream outStream = null;
        try {
            outStream = op.openOutputStream();
        } catch (IOException e) {
            Log.w(TAG,"getImgThumbRsp: sending OBEX_HTTP_BAD_REQUEST", e);
            return ResponseCodes.OBEX_HTTP_BAD_REQUEST;
        }
        if (D) Log.d(TAG,"getImgThumbRsp: imgHandle = " + imgHandle);
        if (mAvrcpBipRspParser.getImgThumb(outStream, imgHandle)) {
            if (!mAborted && mConnected) {
                if (D) Log.d(TAG,"getImgThumbRsp: returning OBEX_HTTP_OK");
                return ResponseCodes.OBEX_HTTP_OK;
            } else {
                /* abort was issued while get was being requested. Return error now */
                mAborted = false;
                if (D) Log.d(TAG,"getImgThumbRsp: mAborted received  return OBEX_HTTP_BAD_REQUEST");
                return ResponseCodes.OBEX_HTTP_BAD_REQUEST;
            }
        }
        if (D) Log.w(TAG,"getImgThumbRsp: returning OBEX_HTTP_BAD_REQUEST");
        return ResponseCodes.OBEX_HTTP_BAD_REQUEST;
    }

    private int getImgRsp(Operation op, String imgHandle, String imgDescXmlString) {
        OutputStream outStream = null;
        try {
            outStream = op.openOutputStream();
        } catch (IOException e) {
            Log.w(TAG,"getImgRsp: IOException" +
                    " - sending OBEX_HTTP_BAD_REQUEST Exception:", e);
            return ResponseCodes.OBEX_HTTP_BAD_REQUEST;
        }

        if (D) Log.d(TAG,"getImgRsp: imgHandle = " + imgHandle);
        if (mAvrcpBipRspParser.getImg(outStream, imgHandle, imgDescXmlString)) {
            if (!mAborted && mConnected) {
                if (V) Log.d(TAG,"getImgRsp: returning OBEX_HTTP_OK");
                return ResponseCodes.OBEX_HTTP_OK;
            } else {
                /* abort was issued while get was being requested. Return error now */
                mAborted = false;
                return ResponseCodes.OBEX_HTTP_BAD_REQUEST;
            }
        }
        if (D) Log.d(TAG,"getImgRsp: returning OBEX_HTTP_BAD_REQUEST");
        return ResponseCodes.OBEX_HTTP_BAD_REQUEST;
    }
}
