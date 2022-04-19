/* Copyright (c) 2014, 2016, 2019 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.ims.vt;

import android.app.admin.DevicePolicyManager;
import android.content.Context;
import android.hardware.camera2.CameraAccessException;
import android.os.SystemProperties;

import com.android.internal.annotations.VisibleForTesting;
import com.qualcomm.ims.utils.Log;

import org.codeaurora.telephony.utils.Preconditions;

/**
 * CameraManager selects ImsCamera or Camera2
 */
public class CameraManager {
    private static final String TAG = "CameraManager";

    static IFactory mFactory;
    private final Context mContext;
    private DevicePolicyManager mDpm;

    private static CameraManager sCameraManager;

    private CameraManager(Context context) {
        mContext = context;
        Preconditions.checkNotNull(mContext);

        setFactory(shallUseCamera2() ? new Camera2Factory() : new Camera1Factory());
        mDpm = (DevicePolicyManager) mContext.getSystemService(Context.DEVICE_POLICY_SERVICE);
    }

    public interface IFactory {
        Camera create(String id, Camera.Listener listener) throws CameraAccessException;
    }

    private class Camera2Factory implements IFactory {
        @Override
        public Camera create(String id, Camera.Listener listener) throws CameraAccessException {
            return new Camera2(mContext, id, listener);
        }
    };

    private class Camera1Factory implements IFactory {
        @Override
        public Camera create(String id, Camera.Listener listener) throws CameraAccessException {
            return new ImsCamera(mContext, id, listener);
        }
    };

    public static synchronized void init(Context context) {
        if (sCameraManager == null) {
            sCameraManager = new CameraManager(context);
        }
    }

    @VisibleForTesting
    public static void setFactory(IFactory factory) {
        mFactory = factory;
    }

    public static Camera open(String id, Camera.Listener listener) throws CameraAccessException {
        Log.v(TAG, "open cameraid= " + id + " listener= " + listener);

        Preconditions.checkNotNull(sCameraManager.mDpm);
        Preconditions.checkNotNull(listener);

        if (sCameraManager.mDpm.getCameraDisabled(null)) {
            throw new CameraAccessException(CameraAccessException.CAMERA_ERROR,
                    "Camera is disabled.");
        }

        Camera camera = sCameraManager.mFactory.create(id, listener);
        Preconditions.checkNotNull(camera);
        camera.open();
        return camera;
    }

    private boolean shallUseCamera2() {
        final int camera1 = 1;  // Camera1
        final int camera2 = 2;  // Any camera interface other than Camera1
        final int cameraInterface = SystemProperties.getInt(
                 "persist.vendor.qti.telephony.vt_cam_interface",camera2);
        if (cameraInterface != camera2 && cameraInterface != camera1) {
             Log.w(TAG, "cameraInterface = " + cameraInterface);
        }
        return cameraInterface != camera1;
    }
}
