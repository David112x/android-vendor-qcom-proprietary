/*
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *           * Redistributions of source code must retain the above copyright
 *             notice, this list of conditions and the following disclaimer.
 *           * Redistributions in binary form must reproduce the above
 *           * copyright notice, this list of conditions and the following
 *             disclaimer in the documentation and/or other materials provided
 *             with the distribution.
 *           * Neither the name of The Linux Foundation nor the names of its
 *             contributors may be used to endorse or promote products derived
 *             from this software without specific prior written permission.
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
package com.android.bluetooth.avrcpcontroller;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Process;
import android.os.Bundle;
import android.os.Looper;
import android.os.Message;
import android.os.Environment;
import android.util.Log;
import javax.obex.ClientSession;
import javax.obex.ClientOperation;
import javax.obex.HeaderSet;
import javax.obex.ObexTransport;
import javax.obex.ObexHelper;
import javax.obex.ResponseCodes;
import android.bluetooth.BluetoothAvrcpController;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import com.android.bluetooth.BluetoothObexTransport;
import java.io.FileOutputStream;
import java.io.FileNotFoundException;
import java.io.File;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.StringWriter;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;
import org.xmlpull.v1.XmlSerializer;

import com.android.internal.util.FastXmlSerializer;

import java.util.HashMap;
import java.util.ArrayList;

/* Bluetooth/AVRCPController/BipObexHandler is responsible
 * for CONNECT, DISCONNECT , FETCH IMAGE and THUMBNAIL
 * from the AVRCP TARGET when commanded.
 */

class AvrcpControllerBipObexHandler extends Handler {
    private static final String TAG = "AvrcpControllerBipObexHandler";
    private static final boolean DBG = true;
    private static final boolean VDBG = AvrcpControllerService.VDBG;
    private final Handler  mAvrcpBipSMHandler;
    private ObexTransport mTransport;
    private ClientSession mSession;
    private boolean mConnected = false ;
    /** AVRCP BIP Target Header referred from AVRCP1.6 Spec, Sec 5.14.2.1 */
    private static final byte[] avrcpBipRsp = new byte[] {
         (byte)0x71, (byte)0x63, (byte)0xDD, (byte)0x54,
         (byte)0x4A, (byte)0x7E, (byte)0x11, (byte)0xE2,
         (byte)0xB4, (byte)0x7C, (byte)0x00, (byte)0x50,
         (byte)0xC2, (byte)0x49, (byte)0x00, (byte)0x48
    };
    private BluetoothSocket mSocket = null;
    private BluetoothDevice mDevice;
    private AvrcpControllerService mAvrcpControllerService;
    /** OBEX Header Identifier or OpCode for Image Handle */
    private static final byte OAP_TAGID_IMG_HANDLE = 0x30;
    /** OBEX Header Identifier or OpCode for Image descriptor */
    private static final byte OAP_TAGID_IMG_DESCRIPTOR = 0x71;
    /**Default value for INVALID L2CAP PSM */
    private static final int L2CAP_INVALID_PSM = -1;
    private static int mL2capPsm = L2CAP_INVALID_PSM;
    /** Where we store temp Bluetooth received files on the external storage */
    public static final String DEFAULT_STORE_SUBDIR = "/bluetooth";
    /** Default prefix for thumnail files stored on external storage */
    final private String THUMB_PREFIX = "AVRCP_BIP_THUMB_";
    /** Default prefix for image files stored on external storage */
    final private String IMG_PREFIX = "AVRCP_BIP_IMG_";
    /** Default Transformation supported for Image retreival */
    final private String DEF_TRANSFORMATION_SUPPORTED = "stretch";

    /** Creates new AvrcpControllerBipObexHandler
        @param looper Looper to attach this class Handler.
        @param handler BIPStateMachine Handler to post back the message status.
     */
    public AvrcpControllerBipObexHandler(Looper looper, Handler handler) {
        super(looper);
        mAvrcpBipSMHandler = handler;
    }

    @Override
    public void handleMessage(Message msg) {
        if (DBG) Log.d(TAG, "Handler(): got msg=" + msg.what);
        Bundle imageData ;
        Message message;
        switch (msg.what) {
            case AvrcpControllerBipStateMachine.MESSAGE_OBEX_CONNECT:
                mL2capPsm = (int)msg.arg1;
                mDevice = (BluetoothDevice)msg.obj;
                connectBip();
                if (mConnected) {
                    mAvrcpBipSMHandler
                            .obtainMessage(AvrcpControllerBipStateMachine.MESSAGE_OBEX_CONNECTED)
                            .sendToTarget();
                } else {
                    mAvrcpBipSMHandler
                            .obtainMessage(AvrcpControllerBipStateMachine.MESSAGE_OBEX_DISCONNECTED)
                            .sendToTarget();
                }
                break;

            case AvrcpControllerBipStateMachine.MESSAGE_OBEX_DISCONNECT:
                disconnectBip();
                mAvrcpBipSMHandler
                        .obtainMessage(AvrcpControllerBipStateMachine.MESSAGE_OBEX_DISCONNECTED)
                        .sendToTarget();
                break;

            case AvrcpControllerBipStateMachine.MESSAGE_OBEX_THUMBNAIL_FETCH:
                String imgThmbNailHandle = (String)msg.obj;
                imageData = new Bundle();
                imageData.putString(AvrcpControllerBipStateMachine.COVER_ART_HANDLE,
                    imgThmbNailHandle);
                message = mAvrcpBipSMHandler
                    .obtainMessage(AvrcpControllerBipStateMachine.MESSAGE_OBEX_THUMBNAIL_FETCHED);
                imageData.putString(AvrcpControllerBipStateMachine.COVER_ART_IMAGE_LOCATION,
                    getLinkedThumbnail(imgThmbNailHandle));
                message.setData(imageData);
                message.sendToTarget();
                break;

            case AvrcpControllerBipStateMachine.MESSAGE_OBEX_IMAGE_FETCH:
                String imgHandle = msg.getData()
                        .getString(AvrcpControllerBipStateMachine.COVER_ART_HANDLE);
                long height =  AvrcpControllerBipStateMachine.mSupportedCovertArtHeight;
                long width = AvrcpControllerBipStateMachine.mSupportedCoverArtWidth;
                StringBuffer pixel = new StringBuffer();
                     pixel.append(width); pixel.append("*"); pixel.append(height);
                String encoding =  AvrcpControllerBipStateMachine.mSupportedCoverArtMimetype;
                long maxSize =  AvrcpControllerBipStateMachine.mSupportedCoverArtMaxSize;
                imageData = getImage(imgHandle, encoding, pixel.toString(), maxSize);
                //Should not happen
                if (imageData == null) {
                    imageData = new Bundle();
                    imageData.putString(AvrcpControllerBipStateMachine.COVER_ART_IMAGE_LOCATION,
                            null);
                }
                message = mAvrcpBipSMHandler
                    .obtainMessage(AvrcpControllerBipStateMachine.MESSAGE_OBEX_IMAGE_FETCHED);
                message.setData(imageData);
                message.sendToTarget();
                break;
        }
    }

    /** Connect over L2Cap Socket and create new Obex Client session.
     */
    private void connectBip() {
        if (VDBG) Log.v(TAG, "connectBIP:");
        // Connect to L2CAPSocket with PSM provided from BIPSM.
        if (connectL2capSocket()) {
            BluetoothObexTransport mTransport = new BluetoothObexTransport(mSocket);
            if (mTransport == null) {
                Log.w(TAG, "Cannot connect: No Transport available");
                return;
            }
            try {
                // Start new Obex Client Session on successfull l2cap connect.
                mSession = new ClientSession(mTransport);
                HeaderSet headerset = new HeaderSet();
                headerset.setHeader(HeaderSet.TARGET, avrcpBipRsp);
                headerset = mSession.connect(headerset);
                if (DBG) Log.d(TAG," Rsp Code: " + headerset.getResponseCode());
                if (headerset.getResponseCode() == ResponseCodes.OBEX_HTTP_OK) {
                    mConnected = true;
                } else {
                    // Force Close the socket for failure response
                    closeSocket();
                    mSession = null;
                }
            } catch (IOException e) {
                Log.w(TAG, "handle connect exception: ", e);
            }
        } else {
            Log.w(TAG, "connectBIP: FAIL");
        }
    }

    /** Establish connection over L2Cap channel.
     */
    private boolean connectL2capSocket() {
        try {
            // Use BluetoothSocket to connect
            if (DBG) Log.d(TAG,"connectL2capSocket: PSM: " + mL2capPsm);
            //TODO: Handle multiple connect reject to same device or different device
            if (mDevice != null && mL2capPsm != L2CAP_INVALID_PSM) {
                mSocket = mDevice.createL2capSocket(mL2capPsm);
                if (mSocket != null ) {
                    mSocket.connect();
                } else  {
                    Log.w(TAG, " create L2cap socket failed");
                    return false;
                }
            } else {
                Log.w(TAG, " connect L2cap socket failed: mDevice = " + mDevice + " mL2capPsm = "
                        + mL2capPsm);
                return false;
            }
        } catch (IOException e) {
            Log.e(TAG, "Error when creating/connecting L2cap socket", e);
            // Ensure  socket close for any exceptions.
            closeSocket();
            return false;
        }
        return true;
    }

    /** DisConnect BIP L2Cap connection and close Obex Client session.
     */
    private void disconnectBip() {
        if (VDBG) Log.v(TAG, "disconnectBIP: ");
        try {
            // Disconnect and Close Obex Client Session if already connected.
            if (mConnected && mSession != null) {
                if(VDBG) Log.v(TAG,"DisconnectBip: Obex ClientSession disc");
                mSession.disconnect(null);
            }
        } catch (IOException e) {
            Log.w(TAG, "handled disconnect exception:", e);
        }
        if (mSession != null) {
            try {
                if(VDBG) Log.v(TAG,"DisconnectBip: Obex ClientSession close");
                    mSession.close();
            } catch (IOException e) {
                Log.w(TAG, "handle session close exception:", e);
            }
            mSession = null;
        }
        // Close L2Cap Channel.
        closeSocket();
        mConnected = false;
    }

    /** Disconnect L2Cap channel.
     */
    private void closeSocket() {
        try {
            // This method is currently invoked only from BipObexHandler and
            // not joined for cleanup.
            if (mSocket != null) {
                mSocket.close();
            }
        } catch (IOException e) {
                Log.e(TAG, "Error when closing socket", e);
        }
        mSocket = null;
    }

    /**    Abort obex client session and close l2cap socket to disconnect.
     */
    public void abort() {
        // Force close socket for time bond cases like pending DISCONNECT from BIP SM.
        closeSocket();
        mConnected = false;
    }

    /** Handle disconnect BIP and remove all temp images on external storage
        to Cleanup AvrcpControllerBipObexHandler.
     */
    public void cleanup() {
        if(DBG) Log.d(TAG,"Cleanup");
         // Force close socket for time bond cases like BT Turn OFF.
       /** Obex Disc is ignored  from obex layer for any ongoing GET Request.
            Hence, only obex session close and socket close are  performed in later case. */
        disconnectBip();
        clearCoverArtMapFromLocalPath();
    }

    /** Remove all temp images fetched and stored on external storage during BIP Session.
     */
    private void clearCoverArtMapFromLocalPath() {
        /*
        * In this case hashmap of handle and images has to be cleared.
        */
        String fileToBeDeletedPrefix = "AVRCP_BIP_";
        File btDir = checkForBluetoothDir();
        if (btDir != null) {
            File file[] = btDir.listFiles();
            if (file != null) {
                for (int i = 0; i < file.length; i++) {
                    if (file[i].getName().startsWith(fileToBeDeletedPrefix)) {
                        Log.e(TAG, "file is deleted  @ " + file[i].getAbsolutePath());
                        file[i].delete();
                    }
                }
            }
        }
    }

    /** Execute BIP Obex Client Request and
     *  Read or Process the Obex Server response.
     *  @param req BIP Obex Client Request.
     *  @return true on successfull execution , false for any exceptions.
     */
    private boolean processBipRequest(AvrcpBipRequest req) {
        if (req == null) {
            Log.w(TAG," processBipRequest: BadRequest");
            return false;
        }
        if (mSession != null) {
            try {
                 req.execute(mSession);
                 return true;
             } catch (IOException e) {
                 Log.e(TAG," processBipReqst fail " + e);
                 return false;
             }
        } else {
            Log.e(TAG," obexSession already disconnected");
            return false;
        }
    }

    /** Check and return file location if image already exists on external storage.
     *  @param imgHandle handle or name of image to be searched.
     *  @param imgPrefix prefix of filename to be searched.
     *  @return absolute file path location of image under bluetooth dir on external storage,
     *          null  if image is not already fetched.
     */
    private String checkCoverArtOnLocalPath(String imgPrefix,String imgHandle) {
        String fileToBeSearch = imgPrefix + imgHandle;
        File btDir = checkForBluetoothDir();
        if (btDir != null) {
            File file[] = btDir.listFiles();
            if (file != null) {
                for (int i = 0; i < file.length; i++) {
                    if (file[i].getName().startsWith(fileToBeSearch)) {
                        if(DBG) Log.d(TAG, "file is present @ " + file[i].getAbsolutePath());
                        return file[i].getAbsolutePath();
                    }
                }
            }
        }
        return null;
    }

    /** Check for bluetooth dir access on external storage.
     *  @return default bluetooh dir if storage accessible,
     *          null if not accessible.
     */
    private File checkForBluetoothDir() {
        File btDir = null;
        if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
            String root = Environment.getExternalStorageDirectory().getPath();
            btDir = new File(root + DEFAULT_STORE_SUBDIR);
            if (!btDir.isDirectory() && !btDir.mkdir()) {
                if (DBG) Log.d(TAG, "Receive File aborted - can't create base directory "
                            + btDir.getPath());
                return null;
            }
            return btDir;
        } else {
            if (DBG) Log.d(TAG, "Receive File aborted - no external storage");
            return btDir;
        }
    }

    /** Compose GETLinkedThumbnail Request to retreive thumbnail version of the image.
        @param imgHandle handle for the image to be retreived.
        @return location for thumbnail file on external storage.
     */
    public String getLinkedThumbnail(String imgHandle){
        // Check and return Image location if already exists
        String localPath = checkCoverArtOnLocalPath(THUMB_PREFIX,imgHandle);
        if (DBG) Log.d(TAG, "getLinkedThumbnil: localPath " + localPath + " ImgHandle: "
            + imgHandle);
        if (localPath == null) {
            AvrcpBipRequest request = new AvrcpBipGetLinkedThumbnail(imgHandle);
            // GET Request Image Thumbnail
            processBipRequest(request);
            if (request.isSuccess()) {
                localPath = request.mReceiveFilePath;
            } else {
                Log.w(TAG, "getLinkedThumbnail request: failure");
            }
        } else {
            if (VDBG) Log.v(TAG, "getLinkedThumbnil: already exists at localPath " + localPath );

        }
        return localPath;
    }

    /** GETFinalImageFromProperties Request to retreive image from the image properties,
        supply image description along with handle for the image to be fetched.
        @param imgHandle handle for the image to be retreived.
        @param prop ImageProperties object required to compose image description.
        @return Processed BIP response including location for Image fetched.
     */
    private AvrcpBipRequest getFinalImageFromProperties(String imgHandle,
            AvrcpBipGetImageProperties prop) {
        AvrcpBipImgDescriptor imgDesc;
        try {
            imgDesc = getPreferredImgDescriptor(prop);
        } catch (Exception e) {
            Log.e(TAG, "AvrcpBip: getImageProp"  + e);
            // recieve file is set to null, to indicate failure;
            return null;
        }
        byte[] imgDescXmL = createXmlFromImgDescriptor(imgDesc);
        AvrcpBipRequest request = new AvrcpBipGetImage(imgHandle, imgDesc, imgDescXmL);
        //GET Request Image
        processBipRequest(request);
        return request;
    }

    /** GetImage Request to retreive image properties and trigger complete image request.
        @param imgHandle handle for the image to be retreived.
        @param encoding for image to be fetched, provided from BIPSM.
        @param pixel for image to be fetched, provided from BIPSM.
        @param maxSize for the image to be fetched, provided from BIPSM.
        @return Bundle of imageData that is retreived.
     */
    private Bundle getImage(String imgHandle, String encoding, String pixel, long maxSize) {
        Bundle imageData = new Bundle();
        imageData.putString(AvrcpControllerBipStateMachine.COVER_ART_HANDLE, imgHandle);
        // Check and return Image location if already exists
        String localPath = checkCoverArtOnLocalPath(IMG_PREFIX,imgHandle);
        if (DBG) Log.d(TAG, "getImage: localPath " + localPath + " maxSize: "
            + maxSize + " Pixel: "+ pixel);
        if (localPath == null) {
            AvrcpBipRequest prop = new AvrcpBipGetImageProperties(imgHandle,
                new AvrcpBipImgDescriptor(encoding, pixel, Long.toString(maxSize),
                    DEF_TRANSFORMATION_SUPPORTED));
            // GETImageProperties request for image handle.
            if (processBipRequest(prop)) {
                AvrcpBipRequest req  = getFinalImageFromProperties(imgHandle,
                        (AvrcpBipGetImageProperties)prop);
                if (req !=  null) {
                    imageData.putString(AvrcpControllerBipStateMachine.COVER_ART_IMAGE_LOCATION,
                        req.isSuccess() ? req.mReceiveFilePath:null);
                } else {
                    if (DBG) Log.d(TAG, "image fetch failed ");
                    return null;
                }
            } else {
                imageData.putString(AvrcpControllerBipStateMachine.COVER_ART_IMAGE_LOCATION, null);
            }
        } else {
            imageData.putString(AvrcpControllerBipStateMachine.COVER_ART_IMAGE_LOCATION, localPath);
        }
        return imageData;
    }

    /** Get Image description object from ImageProperties object.
     *  @param prop ImageProperties object that contain details of image characteristics.
     *  @return ImageDescription object.
     */
    private AvrcpBipImgDescriptor getPreferredImgDescriptor(AvrcpBipGetImageProperties prop) {
        if (prop.mReqImgDesc != null && prop.mResImgDesc != null) {
            AvrcpBipImgDescriptor resNativeImgDesc = null;
            /* Check if the req pixel fields contains only 1 "*". */
            if ((prop.mReqImgDesc.mPixel.indexOf("*") == -1) ||
                (prop.mReqImgDesc.mPixel.indexOf("*") != prop.mReqImgDesc.mPixel.lastIndexOf("*")))
            {
                Log.e(TAG, "GetPreferredImgDescriptor: req pixel field is invalid:"
                        + prop.mReqImgDesc.mPixel);
                return null;
            }
            int reqWidth = Integer.parseInt(prop.mReqImgDesc.mPixel.substring(
                    0, prop.mReqImgDesc.mPixel.indexOf("*")));
            int reqHeight = Integer.parseInt(prop.mReqImgDesc.mPixel.substring(
                    prop.mReqImgDesc.mPixel.lastIndexOf("*") + 1));

            for (int i = 0; i < prop.mResImgDesc.size(); i++) {
                AvrcpBipImgDescriptor mResImgDesc = prop.mResImgDesc.get(i);
                if (i==0) {
                    /* storing native format for image, incase if UI preferance does not match.*/
                    resNativeImgDesc = mResImgDesc;
                }
                if ((prop.mReqImgDesc.mEncoding).equals(mResImgDesc.mEncoding)) {
                    Log.d(TAG, "GetPreferredImgDescriptor: " + mResImgDesc.mEncoding);

                    if ((mResImgDesc.mPixel.indexOf("-") != -1)) {
                        Log.e(TAG, "GetPreferredImgDescriptor Range");
                        if (mResImgDesc.mPixel.indexOf("-") != mResImgDesc.mPixel.lastIndexOf("-"))
                        {
                            Log.e(TAG, "GetPreferredImgDescriptor: res pixel field is invalid:"
                                    + mResImgDesc.mPixel);
                        } else {
                             String range[] = mResImgDesc.mPixel.split("-",2);
                             if ((range[1].indexOf("*") == -1) ||
                                 (range[1].indexOf("*") != range[1].lastIndexOf("*"))) {
                                 Log.e(TAG, "GetPreferredImgDescriptor: res pixel is invalid:"
                                        + range[1]);
                                 return null;
                             }
                             int highWidth = Integer.parseInt(range[1].substring(
                                     0, range[1].indexOf("*")));
                             int highHeight = Integer.parseInt(range[1].substring(
                                     range[1].lastIndexOf("*") + 1));
                             if (VDBG) Log.v (TAG, "lowest range: " + range[0]);
                             if (range[0].indexOf("**") == -1) {
                                 if ((range[0].indexOf("*") == -1) ||
                                     (range[0].indexOf("*") != range[0].lastIndexOf("*"))) {
                                     Log.e(TAG, "GetPreferredImgDescriptor: res pixel is invalid:"
                                            + range[0]);
                                     return null;
                                 }
                                int lowWidth = Integer.parseInt(range[0].substring(
                                        0, range[0].indexOf("*")));
                                int lowHeight = Integer.parseInt(range[0].substring(
                                        range[0].lastIndexOf("*") + 1));
                                if (VDBG) Log.v(TAG, "reqWidth: " + reqWidth + "reqHeight: "
                                        + reqHeight + "lowWidth: " + lowWidth + "lowHeight: "
                                        + lowHeight+ "highWidth: " + highWidth + "highHeight: "
                                        + highHeight);
                                if ((reqWidth >= lowWidth && reqWidth <= highWidth)
                                    && (reqHeight >= lowHeight && reqHeight <= highHeight)){
                                    Log.e(TAG, "GetPreferredImgDescriptor: Match found @ " + i);
                                    return new AvrcpBipImgDescriptor(prop.mReqImgDesc.mEncoding,
                                            prop.mReqImgDesc.mPixel,mResImgDesc.mSize,
                                            mResImgDesc.mTransformation);
                                }
                             } else {
                                 if ((range[0].indexOf("*") == -1)) {
                                     Log.e(TAG, "GetPreferredImgDescriptor: res field is invalid:"
                                            + range[0]);
                                     return null;
                                 }
                                int lowWidth = Integer.parseInt(range[0].substring(
                                        0, range[0].indexOf("*")));
                                int lowHeight = (lowWidth * highHeight)/highWidth;
                                if (VDBG) Log.v(TAG, "reqWidth: " + reqWidth + "reqHeight: "
                                        + reqHeight + "lowWidth: " + lowWidth + "lowHeight: "
                                        + lowHeight+ "highWidth: " + highWidth + "highHeight: "
                                        + highHeight);

                                if ((reqWidth >= lowWidth && reqWidth <= highWidth)
                                    && (reqHeight >= lowHeight && reqHeight <= highHeight)){
                                    Log.e(TAG, "GetSuiableImgDescrpriptor: Match found @ " + i);
                                    return new AvrcpBipImgDescriptor(prop.mReqImgDesc.mEncoding,
                                            prop.mReqImgDesc.mPixel,mResImgDesc.mSize,
                                            mResImgDesc.mTransformation);
                                }
                             }
                        }
                    } else {

                        int resWidth, resHeight;
                        /* Check if the pixel fields contains only 1 "*". */
                        if ((mResImgDesc.mPixel.indexOf("*") == -1) ||
                            mResImgDesc.mPixel.indexOf("*") != mResImgDesc.mPixel.lastIndexOf("*"))
                        {
                            Log.e(TAG, "GetPreferredImgDescriptor: res pixel field is invalid:"
                                    + mResImgDesc.mPixel);
                            return null;
                        }
                        resWidth = Integer.parseInt(mResImgDesc.mPixel.substring(
                                0, mResImgDesc.mPixel.indexOf("*")));
                        resHeight = Integer.parseInt(mResImgDesc.mPixel.substring(
                                mResImgDesc.mPixel.lastIndexOf("*") + 1));
                        if (VDBG) Log.v(TAG, "reqWidth: " + reqWidth + "reqHeight: " + reqHeight
                                + "resWidth: " + resWidth + "resHeight: " + resHeight);
                        if (reqWidth <= resWidth && reqHeight <= resHeight){
                            Log.e(TAG, "GetPreferredImgDescriptor: Match found @ " + i);
                            return new AvrcpBipImgDescriptor(prop.mReqImgDesc.mEncoding,
                                    prop.mReqImgDesc.mPixel,mResImgDesc.mSize,
                                    mResImgDesc.mTransformation);
                        }
                    }
                }
            }
            if (resNativeImgDesc != null) {
                Log.e(TAG, "No valid match found, seclecting native encoding ");
                return new AvrcpBipImgDescriptor(resNativeImgDesc.mEncoding,
                        resNativeImgDesc.mPixel,resNativeImgDesc.mSize,
                        resNativeImgDesc.mTransformation);
            }
        }
        Log.w(TAG, "Native encoding not found, returning default request param");
        return prop.mReqImgDesc;
    }

    /** Compose XML Image description required for GETImage Request.
     *  @param ImageDescriptor object
     *  @return XML image descriptor,
     *          null for any system exceptions.
     */
    private byte[] createXmlFromImgDescriptor(AvrcpBipImgDescriptor imgDescriptor) {
        StringWriter sw = new StringWriter();
        XmlSerializer xmlMsgElement = new FastXmlSerializer();
        // contruct the XML tag for a imgDescriptor single Msg listing
        try {
            xmlMsgElement.setOutput(sw);
            xmlMsgElement.startDocument("UTF-8", true);
            xmlMsgElement.setFeature("http://xmlpull.org/v1/doc/features.html#indent-output",
                    true);
            xmlMsgElement.startTag(null, "image-descriptor");
            xmlMsgElement.attribute(null, "version",
                imgDescriptor.mVersion);
            xmlMsgElement.startTag(null, "image");
            xmlMsgElement.attribute(null, "encoding",
                imgDescriptor.mEncoding);
            xmlMsgElement.attribute(null, "pixel",
                imgDescriptor.mPixel);
            if (imgDescriptor.mMaxSize != null) {
                xmlMsgElement.attribute(null, "maxsize",
                        imgDescriptor.mMaxSize);
            }
            if (imgDescriptor.mTransformation != null) {
                xmlMsgElement.attribute(null, "transformation",
                        imgDescriptor.mTransformation);
            }
            xmlMsgElement.endTag(null, "image");
            xmlMsgElement.endTag(null, "image-descriptor");
            xmlMsgElement.endDocument();
            if (VDBG) Log.v(TAG, "Image descriptor XML = " + sw.toString());
            return sw.toString().getBytes("UTF-8");
        } catch (IllegalArgumentException e) {
            Log.w(TAG, e);
        } catch (IllegalStateException e) {
            Log.w(TAG, e);
        } catch (IOException e) {
            Log.w(TAG, e);
        }
        return null;
    }

    /** This class is implementation for Image Descriptor Object.
     */
    private class AvrcpBipImgDescriptor {
           public String mVersion;
           public String mEncoding;
           public String mPixel;
           public String mSize;
           public String mMaxSize;
           public String mTransformation;
           public static final String DEFAULT_VERSION = "1.0";
           private String DEFAULT_ENCODING = "JPEG";
           private String DEFAULT_PIXEL = "";

           private AvrcpBipImgDescriptor(String encoding, String pixel,
               String maxSize, String transformation) {
               mVersion = DEFAULT_VERSION;
               mEncoding = encoding;
               mPixel = pixel;
               mMaxSize = maxSize;
               mTransformation = transformation;
               mSize = null;
           }

           private AvrcpBipImgDescriptor(HashMap<String, String> attrs) {
               mVersion = attrs.get("version");
               if (mVersion == null) mVersion = DEFAULT_VERSION;
               mEncoding = attrs.get("encoding");
               mPixel = attrs.get("pixel");
               mSize = attrs.get("size");
               mTransformation = attrs.get("transformation");
               mMaxSize = null;
           }

           public AvrcpBipImgDescriptor() {
               mVersion = DEFAULT_VERSION;
               mEncoding = DEFAULT_ENCODING;
               mPixel = DEFAULT_PIXEL;
           }
    };

    /** A base implementation for Obex BIP Client Request.
     */
    public abstract class AvrcpBipRequest {
        private final static String TAG = "AvrcpBipRequest";
        protected HeaderSet mHeaderSet;
        protected int mResponseCode;
        public  String mReceiveFilePath = null;

        public AvrcpBipRequest() {
            mHeaderSet = new HeaderSet();
        }

        abstract public void execute(ClientSession session) throws IOException;

        protected void executeGet(ClientSession session) throws IOException {
            ClientOperation op = null;

            try {
                op = (ClientOperation) session.get(mHeaderSet);
                op.setGetFinalFlag(true);
                op.continueOperation(true, false);
                readResponseHeaders(op.getReceivedHeader());
                InputStream is = op.openInputStream();
                readResponse(is);
                is.close();
                op.close();
                mResponseCode = op.getResponseCode();
            } catch (IOException e) {
                mResponseCode = ResponseCodes.OBEX_HTTP_INTERNAL_ERROR;
                Log.w(TAG, "executeGet: ", e);
                throw e;
            }
        }

        final public boolean isSuccess() {
            Log.w(TAG, "isSuccess: " + mResponseCode);
            return (mResponseCode == ResponseCodes.OBEX_HTTP_OK);
        }

        protected void readResponse(InputStream stream) throws IOException {
            /* nothing here by default */
        }

        protected void readResponseHeaders(HeaderSet headerset) {
            /* nothing here by default */
        }
    }

    /** This class is implementation for LinkedThumbnail Object.
     */
    final class AvrcpBipGetLinkedThumbnail extends AvrcpBipRequest {

        private static final String TAG = "AvrcpBipGetLinkedThumbnail";
        private static final String TYPE = "x-bt/img-thm";
        private String tmpFilePath ;
        String mImgHandle;

        public AvrcpBipGetLinkedThumbnail(String handle) {
            mImgHandle = handle;
            mHeaderSet.setHeader(HeaderSet.TYPE, TYPE);
            mHeaderSet.setHeader(OAP_TAGID_IMG_HANDLE, mImgHandle);
        }

        @Override
        public void execute(ClientSession session) throws IOException {
            executeGet(session);
        }

        @Override
        protected void readResponse(InputStream is) throws IOException {
            byte[] buffer = new byte[4096]; // To hold file contents
            int bytes_read;
            String tmpFilePath ;
            File btDir;
            FileOutputStream tmp = null;
            btDir = checkForBluetoothDir();
            if (DBG) Log.d(TAG, "mResponseCode: " + mResponseCode + "btDir: " + btDir);
            if (btDir != null) {
                tmpFilePath = btDir.getPath() + "/" + THUMB_PREFIX
                       + mHeaderSet.getHeader(OAP_TAGID_IMG_HANDLE)
                       + ".jpeg";
                try {
                    if (DBG) Log.d(TAG,"readResponse: opening " + tmpFilePath +
                        " file for writing image");
                    tmp = new FileOutputStream(tmpFilePath);
                } catch (FileNotFoundException e) {
                    Log.e(TAG,"readResponse: unable to open tmp File for writing");
                    throw e;
                }
                if (DBG) Log.d(TAG,"readResponse: writing to " + tmpFilePath);
                while ((bytes_read = is.read(buffer)) != -1) {
                    if (VDBG) Log.v(TAG,"bytes_read: " + bytes_read);
                    tmp.write(buffer, 0, bytes_read);
                }
                /* Flush the data to output stream */
                mReceiveFilePath =  tmpFilePath;
                tmp.flush();
                if (tmp != null)
                    tmp.close();
            }
        }
    }

    /** This class is implementation for ImageProperties Object.
     */
    final class AvrcpBipGetImageProperties extends AvrcpBipRequest {

        private static final String TAG = "AvrcpBipGetImageProperties";
        private static final String TYPE = "x-bt/img-properties";
        public AvrcpBipImgDescriptor mReqImgDesc;
        public ArrayList<AvrcpBipImgDescriptor> mResImgDesc;
        public String mImgHandle;

        public AvrcpBipGetImageProperties(String handle, AvrcpBipImgDescriptor reqImgDescriptor) {

            mHeaderSet.setHeader(HeaderSet.TYPE, TYPE);
            mHeaderSet.setHeader(OAP_TAGID_IMG_HANDLE, handle);
            mImgHandle = handle;
            mReqImgDesc = reqImgDescriptor;
            mResImgDesc = new ArrayList<AvrcpBipImgDescriptor>();
        }

        @Override
        public void execute(ClientSession session) throws IOException {
            executeGet(session);
        }

       @Override
       protected void readResponse(InputStream is) throws IOException {
            parseImgPropertiesFromXml(is);
       }

       protected void parseImgPropertiesFromXml(InputStream is) {
            if (is == null) {
                Log.e(TAG, "input stream is null, mResponseCode: " + mResponseCode);
                return;
            }
            if (DBG) Log.d(TAG, "parseImgPropertiesFromXml");
            try {
                XmlPullParser imgDescParser = XmlPullParserFactory.newInstance().newPullParser();
                imgDescParser.setInput(is, "UTF-8");
                HashMap<String, String> attrs = new HashMap<String, String>();
                int event = imgDescParser.getEventType();
                while (event != XmlPullParser.END_DOCUMENT) {
                    switch (event) {
                        case XmlPullParser.START_TAG:
                            if (imgDescParser.getName().equals("image-properties")) {
                                if (VDBG) Log.v(TAG, "image-properties version: " +
                                    imgDescParser.getAttributeValue(0)
                                    + "handle: " + imgDescParser.getAttributeValue(1));
                                attrs.put(imgDescParser.getAttributeName(0),
                                    imgDescParser.getAttributeValue(0));
                                attrs.put(imgDescParser.getAttributeName(1),
                                    imgDescParser.getAttributeValue(1));
                            }
                            if (imgDescParser.getName().equals("native")) {
                                for (int i = 0; i < imgDescParser.getAttributeCount(); i++) {
                                    attrs.put(imgDescParser.getAttributeName(i),
                                        imgDescParser.getAttributeValue(i));
                                    if (VDBG) Log.v(TAG, "native: "
                                                    + imgDescParser.getAttributeName(i) + ":"
                                                    + imgDescParser.getAttributeValue(i));
                                }
                                mResImgDesc.add(new AvrcpBipImgDescriptor(attrs));
                            }
                            if (imgDescParser.getName().equals("variant")) {
                                HashMap<String, String> VarAttrs = new HashMap<String, String>();
                                for (int i = 0; i < imgDescParser.getAttributeCount(); i++) {
                                    VarAttrs.put(imgDescParser.getAttributeName(i),
                                        imgDescParser.getAttributeValue(i));
                                    if (VDBG) Log.v(TAG, "variant: "
                                                    + imgDescParser.getAttributeName(i) + ":"
                                                    + imgDescParser.getAttributeValue(i));
                                }
                                mResImgDesc.add(new AvrcpBipImgDescriptor(VarAttrs));
                            }
                            break;
                    }
                    event = imgDescParser.next();
                }
                if (VDBG) Log.v(TAG, "attrs " + attrs);
            } catch (XmlPullParserException e) {
                Log.e(TAG, "Error when parsing XML", e);
            } catch (IOException e) {
                Log.e(TAG, "I/O error when parsing XML", e);
            } catch (IllegalArgumentException e) {
                Log.e(TAG, "Invalid event received", e);
            }
            return;
        }
    }

    /** This class is implementation for Image Object.
     */
    final class AvrcpBipGetImage extends AvrcpBipRequest {

        private static final String TAG = "AvrcpBipGetImage";
        private static final String TYPE = "x-bt/img-img";
        String mImgHandle;
        byte[] mImgDescXml;
        AvrcpBipImgDescriptor mImgDesc;

        public AvrcpBipGetImage(String handle, AvrcpBipImgDescriptor imgDesc,
                byte[] imgDescXml) {
            mImgHandle = handle;
            mImgDesc = imgDesc;
            mImgDescXml = imgDescXml;
            mHeaderSet.setHeader(HeaderSet.TYPE, TYPE);
            mHeaderSet.setHeader(OAP_TAGID_IMG_HANDLE, mImgHandle);
            mHeaderSet.setHeader(OAP_TAGID_IMG_DESCRIPTOR, mImgDescXml);
        }

        @Override
        public void execute(ClientSession session) throws IOException {
            executeGet(session);
        }

        @Override
        protected void readResponse(InputStream is) throws IOException {
            byte[] buffer = new byte[4096]; // To hold file contents
            int bytes_read;
            String tmpFilePath ;
            File btDir;
            FileOutputStream tmp = null;
            btDir = checkForBluetoothDir();
            if (DBG) Log.d(TAG, "btDir: " + btDir);
            if (btDir != null) {
                tmpFilePath = btDir.getPath() + "/" + IMG_PREFIX
                        + mHeaderSet.getHeader(OAP_TAGID_IMG_HANDLE)
                        + "." + mImgDesc.mEncoding;
                try {
                    if (DBG) Log.d(TAG,"readResponse: opening " + tmpFilePath +
                        " file for writing image");
                    tmp = new FileOutputStream(tmpFilePath);
                } catch (FileNotFoundException e) {
                    Log.e(TAG,"readResponse: unable to open tmp File for writing");
                    throw e;
                }
                if (DBG) Log.d(TAG,"readResponse: writing to " + tmpFilePath);
                while ((bytes_read = is.read(buffer)) != -1) {
                     if (VDBG) Log.v(TAG,"bytes_read: " + bytes_read);
                     tmp.write(buffer, 0, bytes_read);
                }
                /* Flush the data to output stream */
                mReceiveFilePath =  tmpFilePath;
                tmp.flush();
                if (tmp != null)
                    tmp.close();
            }
        }
    }
};
