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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import com.android.bluetooth.btservice.ProfileService;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.content.Intent;
import android.media.MediaMetadata;
import android.os.Message;
import android.util.Log;

public class CoverArtUtils {

    private final String TAG = "AvrcpControllerCoverArt";

    private static final boolean DBG = true;

    private final String EXTRA_METADATA_IS_INVALID_HANDLE = "is_invalid_handle";

    static final int MESSAGE_BIP_CONNECTED = 500;
    static final int MESSAGE_BIP_DISCONNECTED = 501;
    static final int MESSAGE_BIP_THUMB_NAIL_FETCHED = 502;
    static final int MESSAGE_BIP_IMAGE_FETCHED = 503;

    public void broadcastInValidHandle(Context ctx, AvrcpControllerService avrcpService,
            boolean streamAvailable) {
        if ((avrcpService == null || avrcpService.getConnectedDevices().size() <= 1)
                && !streamAvailable) {
            broadcastInValidHandle(ctx, new MediaMetadata.Builder().build());
        }
    }

    private void broadcastInValidHandle(Context ctx, MediaMetadata metadata) {
        /*Intent intent = new Intent(AvrcpControllerService.ACTION_TRACK_EVENT);
        intent.putExtra(AvrcpControllerService.EXTRA_METADATA, metadata);
        intent.putExtra(EXTRA_METADATA_IS_INVALID_HANDLE, true);
        if (DBG) {
            Log.d(TAG, " broadcastInValidHandle = " + metadata.getDescription());
        }
        intent.addFlags(Intent.FLAG_RECEIVER_FOREGROUND);
        ctx.sendBroadcast(intent, ProfileService.BLUETOOTH_PERM); */
    }

    public void onElementAttributeUpdate(byte[] address, byte numAttributes, int[] attributes,
            String[] attribVals, BluetoothDevice connectedDevice,
            AvrcpControllerStateMachine avrcpCtSm) {
        if (DBG) {
            Log.d(TAG, "onElementAttributeUpdate numAttributes :" + numAttributes);
        }
        BluetoothDevice device = BluetoothAdapter.getDefaultAdapter().getRemoteDevice(address);
        if (device != null && !device.equals(connectedDevice)) {
            Log.e(TAG, "onElementAttributeUpdate device not found " + address);
            return;
        }
        List<Integer> attrList = new ArrayList<>();
        for (int attr : attributes) {
           attrList.add(attr);
        }
        List<String> attrValList = Arrays.asList(attribVals);
        //TrackInfo trackInfo = new TrackInfo(attrList, attrValList);
        //if (DBG) {
        //    Log.d(TAG, "onElementAttributeUpdate " + trackInfo);
        //}
        //Message msg = avrcpCtSm.obtainMessage(
        //        AvrcpControllerStateMachine.MESSAGE_PROCESS_TRACK_CHANGED, trackInfo);
        //avrcpCtSm.sendMessage(msg);
    }

    public void msgDisconnectBip(AvrcpControllerBipStateMachine bipStateMachine,
            RemoteDevice remoteDevice) {
        if (bipStateMachine != null && remoteDevice != null) {
                bipStateMachine.sendMessage(AvrcpControllerBipStateMachine.MESSAGE_DISCONNECT_BIP,
                    remoteDevice.mBTDevice);
        }
    }

    public void msgProcessRcFeatures(AvrcpControllerBipStateMachine bipStateMachine,
            RemoteDevice remoteDevice, int remotePsm) {
        if (bipStateMachine != null && remoteDevice != null
                && remoteDevice.isCoverArtSupported()) {
            remoteDevice.setRemoteBipPsm(remotePsm);
            bipStateMachine.sendMessage(AvrcpControllerBipStateMachine.MESSAGE_CONNECT_BIP,
                remoteDevice.getRemoteBipPsm(), 0, remoteDevice.mBTDevice);
        }
    }

    public boolean msgTrackChanged(Context ctx, AvrcpControllerBipStateMachine bipStateMachine,
            AvrcpPlayer addressedPlayer, RemoteDevice remoteDevice) {
        /*boolean isValidHandle = !addressedPlayer.getCurrentTrack().getCoverArtHandle().isEmpty();
        if (bipStateMachine != null && isValidHandle) {
            int imageOrThumbnail;
            if (AvrcpControllerBipStateMachine.mImageType.equalsIgnoreCase("thumbnail")) {
                imageOrThumbnail = AvrcpControllerBipStateMachine.MESSAGE_FETCH_THUMBNAIL;
            } else {
                imageOrThumbnail = AvrcpControllerBipStateMachine.MESSAGE_FETCH_IMAGE;
            }
            //bipStateMachine.sendMessage(imageOrThumbnail,
            //        addressedPlayer.getCurrentTrack().getCoverArtHandle());
        }
        if (!isValidHandle && remoteDevice != null && remoteDevice.isCoverArtSupported()) {
            if (DBG) {
                Log.d(TAG, " Cover Art Handle not valid ");
            }
            //broadcastInValidHandle(ctx, addressedPlayer.getCurrentTrack().getMediaMetaData());
            return false;
        }*/
        return true;
    }

    public void processBipAction(Context ctx, AvrcpPlayer addressedPlayer,
            RemoteDevice remoteDevice, int bipAction, Message msg) {
        if (DBG) {
            Log.d(TAG, " processBipAction bipAction: " + bipAction);
        }
        switch (bipAction) {
            case MESSAGE_BIP_CONNECTED:
               //if (addressedPlayer.getCurrentTrack().getCoverArtHandle().isEmpty()
               //     && remoteDevice != null) {
                /*
                 * track changed happened before BIP connection. should fetch cover art handle.
                 * NumAttributes = 0 and attributes list as null will fetch all attributes
                 */
                    AvrcpControllerService.getElementAttributesNative(
                            remoteDevice.getBluetoothAddress(), (byte) 0, null);
                //}
            break;
            case MESSAGE_BIP_DISCONNECTED:
                // clear cover art related info for current track.
                //addressedPlayer.getCurrentTrack().clearCoverArtData();
            break;
            case MESSAGE_BIP_IMAGE_FETCHED:
                /*boolean imageUpdated = addressedPlayer.getCurrentTrack().updateImageLocation(
                msg.getData().getString(AvrcpControllerBipStateMachine.COVER_ART_HANDLE),
                msg.getData()
                    .getString(AvrcpControllerBipStateMachine.COVER_ART_IMAGE_LOCATION));
                if (imageUpdated) {
                    broadcastMetaDataChanged(ctx,
                        addressedPlayer.getCurrentTrack().getMediaMetaData());
                } */
            break;
            case MESSAGE_BIP_THUMB_NAIL_FETCHED:
                /*boolean thumbNailUpdated = addressedPlayer.getCurrentTrack().updateThumbNailLocation
                    (msg.getData().getString(AvrcpControllerBipStateMachine.COVER_ART_HANDLE),
                     msg.getData()
                     .getString(AvrcpControllerBipStateMachine.COVER_ART_IMAGE_LOCATION));
            if (thumbNailUpdated) {
                broadcastMetaDataChanged(ctx,
                    addressedPlayer.getCurrentTrack().getMediaMetaData());
            }*/
            break;
        }
    }

    private void broadcastMetaDataChanged(Context ctx, MediaMetadata metadata) {
        /*Intent intent = new Intent(AvrcpControllerService.ACTION_TRACK_EVENT);
        intent.putExtra(AvrcpControllerService.EXTRA_METADATA, metadata);
        if (DBG) {
            Log.d(TAG, " broadcastMetaDataChanged = " + metadata.getDescription());
        }
        ctx.sendBroadcast(intent, ProfileService.BLUETOOTH_PERM); */
    }

    public String dumpMessageString(int message) {
        String str = "";
        switch (message) {
            case MESSAGE_BIP_CONNECTED: str = "BIP_CONNECTED"; break;
            case MESSAGE_BIP_DISCONNECTED: str = "BIP_DISCONNECTED"; break;
            case MESSAGE_BIP_IMAGE_FETCHED: str = "BIP_IMAGE_FETCHED"; break;
            case MESSAGE_BIP_THUMB_NAIL_FETCHED: str = "BIP_THUMB_NAIL_FETCHED";
        }
        return str;
    }

    public void closeBip(AvrcpControllerBipStateMachine bipStateMachine) {
        if (bipStateMachine != null) {
            bipStateMachine.doQuit();
            bipStateMachine = null;
            if (DBG) {
                Log.d(TAG, "mBipStateMachine doQuit ");
            }
        }
    }
}
