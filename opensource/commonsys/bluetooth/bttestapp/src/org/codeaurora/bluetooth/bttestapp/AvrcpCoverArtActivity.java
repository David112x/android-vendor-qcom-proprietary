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
package org.codeaurora.bluetooth.bttestapp;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.MediaMetadata;
import android.media.session.PlaybackState;
import android.os.Bundle;
import android.os.SystemProperties;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.Spinner;

public class AvrcpCoverArtActivity extends Activity
        implements OnClickListener, OnItemSelectedListener {

    private final String TAG = "BtTestCoverArt";
    private final String ACTION_TRACK_EVENT =
            "android.bluetooth.avrcp-controller.profile.action.TRACK_EVENT";
    private final String EXTRA_METADATA =
            "android.bluetooth.avrcp-controller.profile.extra.METADATA";
    private final String EXTRA_PLAYBACK =
            "android.bluetooth.avrcp-controller.profile.extra.PLAYBACK";
    private final String EXTRA_METADATA_IS_INVALID_HANDLE = "is_invalid_handle";
    private final String KEY_IMGTYPE = "persist.vendor.service.bt.avrcpct.imgtype";
    private final String KEY_IMG_ENCODE = "persist.vendor.service.bt.avrcpct.imgencode";
    private final String KEY_IMG_WIDTH = "persist.vendor.service.bt.avrcpct.imgwidth";
    private final String KEY_IMG_HEIGHT = "persist.vendor.service.bt.avrcpct.imgheight";
    private final String KEY_IMG_SIZE = "persist.vendor.service.bt.avrcpct.imgsize";

    private String mAlbumTitle = "";

    private boolean isCoverArtSet = false;

    private ImageView mIvCoverArt, mIvThumbNail;
    private Spinner mSpImgType, mSpImgEncode, mSpImgWidth, mSpImgSize, mSpImgheight;
    private Button mBtnConfig, mBtnConfigBase;

    private BluetoothAdapter mAdapter;

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.i(TAG, "action " + action);
            if (action.equals(ACTION_TRACK_EVENT)) {
                MediaMetadata metaData = intent.getParcelableExtra(EXTRA_METADATA);
                PlaybackState state = intent.getParcelableExtra(EXTRA_PLAYBACK);
                boolean isInvalid = intent.getBooleanExtra
                        (EXTRA_METADATA_IS_INVALID_HANDLE, false);
                if (metaData != null && state == null) {
                    setCoverArt(metaData, isInvalid);
                }
            } else if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)
                    && (mAdapter.getState() == BluetoothAdapter.STATE_OFF
                      || mAdapter.getState() == BluetoothAdapter.STATE_TURNING_OFF )) {
                setCoverArt(new MediaMetadata.Builder().build(), true);
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.layout_avrcp_coverart);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        mIvCoverArt = (ImageView) findViewById(R.id.id_iv_fullimage);
        mIvThumbNail = (ImageView) findViewById(R.id.id_iv_thumbnail);
        mBtnConfig = (Button) findViewById(R.id.id_btn_config);
        mBtnConfigBase = (Button) findViewById(R.id.id_btn_config_reset);
        mSpImgType = (Spinner) findViewById(R.id.id_sp_img_type);
        mSpImgType.setOnItemSelectedListener(this);
        mSpImgheight = (Spinner) findViewById(R.id.id_sp_img_height);
        mSpImgEncode = (Spinner) findViewById(R.id.id_sp_img_encode);
        mSpImgWidth = (Spinner) findViewById(R.id.id_sp_img_width);
        mSpImgSize = (Spinner) findViewById(R.id.id_sp_img_maxsize);
        mBtnConfig.setOnClickListener(this);
        mBtnConfigBase.setOnClickListener(this);
        mIvCoverArt.setVisibility(ImageView.GONE);
        mIvThumbNail.setVisibility(ImageView.GONE);
        setSpinners();
        Log.i(TAG, " onCreate");
    }

    @Override
    protected void onResume() {
        super.onResume();
        mAdapter = BluetoothAdapter.getDefaultAdapter();
        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_TRACK_EVENT);
        filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        registerReceiver(mReceiver, filter);
        Log.i(TAG, " onResume");
    }

    @Override
    protected void onPause() {
        super.onPause();
        unregisterReceiver(mReceiver);
        Log.i(TAG, " onPause");
    }

    private void setCoverArt(MediaMetadata metaData, boolean isInvalid) {
        String path = metaData.getString(MediaMetadata.METADATA_KEY_ALBUM_ART_URI);
        Bitmap bitMapThumbNail = metaData
                .getBitmap(MediaMetadata.METADATA_KEY_DISPLAY_ICON);
        String albumTitle = metaData.getString(MediaMetadata.METADATA_KEY_ALBUM);
        Log.i(TAG, "bitMapThumbNail :" + bitMapThumbNail + " Cover art path :" + path
                +" isInvalid :" + isInvalid +" Album title :" + albumTitle
                +" mAlbumTitle :" + mAlbumTitle + " isCoverArtSet :" + isCoverArtSet);
        if (mAlbumTitle.equalsIgnoreCase(albumTitle) && isCoverArtSet) {
            Log.v(TAG, " Already set Ignore "); return;
        }

        if (bitMapThumbNail != null) {
            mIvThumbNail.setVisibility(ImageView.VISIBLE);
            mIvCoverArt.setVisibility(ImageView.GONE);
            mIvThumbNail.setImageBitmap(bitMapThumbNail);
            isCoverArtSet = true;
            mAlbumTitle = albumTitle;
            Log.i(TAG, " BitMap");
        } else if (!TextUtils.isEmpty(path)) {
            mIvThumbNail.setVisibility(ImageView.GONE);
            mIvCoverArt.setVisibility(ImageView.VISIBLE);
            Bitmap bitMap = BitmapFactory.decodeFile(path);
            Log.i(TAG, "CoverArt :");
            mIvCoverArt.setImageBitmap(bitMap);
            isCoverArtSet = true;
            mAlbumTitle = albumTitle;
        } else if (isInvalid) {
            mIvCoverArt.setVisibility(ImageView.GONE);
            mIvThumbNail.setVisibility(ImageView.GONE);
            isCoverArtSet = false;
            mAlbumTitle = "";
        }
    }

    @Override
    public void onClick(View v) {
        if (v == mBtnConfig) {
            SystemProperties.set(KEY_IMGTYPE, getValue(mSpImgType));
            SystemProperties.set(KEY_IMG_ENCODE, getValue(mSpImgEncode));
            SystemProperties.set(KEY_IMG_WIDTH, getValue(mSpImgWidth));
            SystemProperties.set(KEY_IMG_HEIGHT, getValue(mSpImgheight));
            SystemProperties.set(KEY_IMG_SIZE, getValue(mSpImgSize));
        } else if (v == mBtnConfigBase) {
            SystemProperties.set(KEY_IMGTYPE, "Image");
            SystemProperties.set(KEY_IMG_ENCODE, "JPEG");
            SystemProperties.set(KEY_IMG_WIDTH, "500");
            SystemProperties.set(KEY_IMG_HEIGHT, "500");
            SystemProperties.set(KEY_IMG_SIZE, "200000");
            setSpinners();
        }
    }

    private void setSpinners() {
        String type = SystemProperties.get(KEY_IMGTYPE);
        if (TextUtils.isEmpty(type) || type.equalsIgnoreCase("Image")) {
            mSpImgType.setSelection(0);
        } else {
            mSpImgType.setSelection(1);
        }
        String mime = SystemProperties.get(KEY_IMG_ENCODE);
        if (TextUtils.isEmpty(mime) || mime.equalsIgnoreCase("JPEG")) {
            mSpImgEncode.setSelection(0);
        } else {
            mSpImgEncode.setSelection(1);
        }

        int height = SystemProperties.getInt(KEY_IMG_HEIGHT, 500);
        int width = SystemProperties.getInt(KEY_IMG_WIDTH, 500);
        int maxSize = SystemProperties.getInt(KEY_IMG_SIZE, 200000);
        Log.i(TAG, " Type :" + type + " Mime :" + mime + " Height:" + height + ": width :"
                + width + " Max size:" + maxSize);
        mSpImgWidth.setSelection(getIndex(mSpImgWidth, width + ""));
        mSpImgheight.setSelection(getIndex(mSpImgheight, height + ""));
        mSpImgSize.setSelection(getIndex(mSpImgSize, maxSize + ""));
    }

    private int getIndex(Spinner spinner, String myString) {
        int index = 0;
        for (int i = 0; i < spinner.getCount(); i++) {
            if (spinner.getItemAtPosition(i).equals(myString)) {
                index = i;
            }
        }
        return index;
    }

    private String getValue(Spinner spinner) {
        return spinner.getSelectedItem().toString();
    }

    @Override
    public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
        if(mSpImgType.getSelectedItem().toString().equals("Image")) {
            mSpImgEncode.setEnabled(true);
            mSpImgheight.setEnabled(true);
            mSpImgSize.setEnabled(true);
            mSpImgWidth.setEnabled(true);
            Log.v(TAG," Image");
        }else {
            mSpImgEncode.setEnabled(false);
            mSpImgheight.setEnabled(false);
            mSpImgSize.setEnabled(false);
            mSpImgWidth.setEnabled(false);
            Log.v(TAG," Not image");
        }
    }

    @Override
    public void onNothingSelected(AdapterView<?> arg0) {
        // TODO Auto-generated method stub
    }

}
