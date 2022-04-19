/*
 * Copyright (c) 2017, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 */

/* Copyright 2017 FreshPlanet
 * Copyright 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.qualcomm.qti.qmmi.testcase.Camera;

import java.io.File;
import java.io.InputStream;
import java.io.IOException;
import java.util.List;

import com.qualcomm.qti.qmmi.framework.BaseActivity;
import com.qualcomm.qti.qmmi.utils.LogUtils;
import com.qualcomm.qti.qmmi.R;
import com.qualcomm.qti.qmmi.bean.TestCase;

import android.Manifest;
import android.app.Activity;
import android.content.ClipData;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.ContentValues;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.BitmapFactory.Options;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Bundle;
import android.provider.MediaStore;

import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import android.support.v4.content.FileProvider;

public class CameraActivity extends BaseActivity{

    private static final int OPEN_CAMERA_IMAGE_PATH = 1000;

    private static String FRONT_IMG_NAME = "FRONT_PIC.jpg";
    private static String BACK_MAIN_IMG_NAME = "BACK_MAIN_PIC.jpg";

    private static String CAMERA_BACK_MAIN = "back";
    private static String CAMERA_FRONT = "front";

    private static String EXTRAS_CAMERA_FACING = "android.intent.extras.CAMERA_FACING";
    private static String CAMERA_PKG_NAME = "org.codeaurora.snapcam";

    private Button mOpenBtn;
    private ImageView mSnapShotIV;

    private int mCameraFacing = 0;
    private String mImageName = null;
    private Uri mImageUri = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        initView();
        initData();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        LogUtils.logi("onPause");
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        LogUtils.logi("onDestroy");
        super.onDestroy();
    }

    /**
     * Gets the content:// URI from the corresponding path to a file
     * @return content Uri
     */
    private Uri getImageContentUri() {
        LogUtils.logi("getImageContentUri:  started");
        Uri result;
        result = getContentResolver().insert(MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
                new ContentValues());
        return result;
    }

    private void initData(){
        String cameraType = mTestCase.getParameter().get("type");
        if (cameraType.equalsIgnoreCase(CAMERA_BACK_MAIN)) {
            mImageName = BACK_MAIN_IMG_NAME;
            mCameraFacing = 0;
            LogUtils.logi("CAMERA MAIN BACK Case");
        } else if (cameraType.equalsIgnoreCase(CAMERA_FRONT)) {
            mImageName = FRONT_IMG_NAME;
            mCameraFacing = 1;
            LogUtils.logi("FRONT CAMERA Case");
        }

        LogUtils.logi("Image: " + mImageName);
        LogUtils.logi("Camera Facing: " + mCameraFacing);

        if (mImageName == null){
                Toast.makeText(getApplicationContext(), R.string.camera_wrong_type, Toast.LENGTH_SHORT).show();
            mOpenBtn.setClickable(false);

        }
    }

    private void initView(){
        mOpenBtn = (Button) findViewById(R.id.btn_open_camera);
        mSnapShotIV = (ImageView) findViewById(R.id.iv_snapshot);

        mOpenBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                openCamera();
            }
        });
    }

    private void openCamera(){
        LogUtils.logi("openCamera: started");
        if(this.checkSelfPermission(Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            this.requestPermissions(new String[] { Manifest.permission.CAMERA }, 1);
        } else {
            mImageUri = getImageContentUri();
            LogUtils.logi("mImageUri = " + mImageUri);
            Intent intentPhoto = new Intent("android.media.action.IMAGE_CAPTURE");

            intentPhoto.putExtra(EXTRAS_CAMERA_FACING, mCameraFacing);
            intentPhoto.putExtra(MediaStore.EXTRA_OUTPUT, mImageUri);
            intentPhoto.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);

            startActivityForResult(intentPhoto, OPEN_CAMERA_IMAGE_PATH);
       }
    }

    @Override
    protected int getLayoutId() {
        return R.layout.camera_act;
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        LogUtils.logi( "onActivityResult: requestCode :" + requestCode + " resultCode:" + resultCode);
        if (resultCode == Activity.RESULT_OK) {
            if (requestCode == OPEN_CAMERA_IMAGE_PATH) {
                LogUtils.logi( "Wow!! Snapshot successful and return back");
                Intent intentScan = new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, mImageUri);
                sendBroadcast(intentScan);
            } else {
                LogUtils.logi("Data: ==" + data);
                mImageUri = data.getData();
            }

            displayImage();
        } else {
            LogUtils.logi("Snapshot Fail and return back");
        }
    }

    /**
     * To display original image , which it specify scale factor bitmap and
     * dimension (1024 * 768).
     */
    private void displayImage() {
        LogUtils.logi("displayImage: started");
        int width, height;
        int specifyScaleFactor;

        BitmapFactory.Options option = new BitmapFactory.Options();
        option.inJustDecodeBounds = true;

        try{
            BitmapFactory.decodeStream(
                    getContentResolver().openInputStream(mImageUri), null, option);
        }catch (IOException e){
            LogUtils.logi("Failed to intial openInputStream(mImageUri)");
            return;
        }

        width = option.outWidth;
        height = option.outHeight;
        specifyScaleFactor = Math.min(width/1024, height/768);
        option.inJustDecodeBounds = false;
        option.inSampleSize = specifyScaleFactor;
        Bitmap bitMap = null;

        try{

            bitMap = BitmapFactory.decodeStream(
                    getContentResolver().openInputStream(mImageUri), null, option);

        }catch (IOException e){
            LogUtils.logi("Failed to decodeStream mImageUri to bitMap");
        }

        mSnapShotIV.setImageBitmap(bitMap);
    }
}
