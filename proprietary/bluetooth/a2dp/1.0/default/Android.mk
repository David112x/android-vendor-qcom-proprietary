LOCAL_PATH := $(call my-dir)
ifeq ($(TARGET_HAS_SPLIT_A2DP_FEATURE),true)
include $(CLEAR_VARS)

LOCAL_MODULE := android.hardware.bluetooth.a2dp@1.0-impl-qti
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

LOCAL_SRC_FILES := \
    BluetoothAudioOffload.cpp

LOCAL_SHARED_LIBRARIES := \
    libhidlbase \
    libhidltransport \
    libutils \
    liblog \
    libhwbinder \
    libbase \
    libcutils \
    libhardware \
    android.hardware.bluetooth.a2dp@1.0

include $(BUILD_SHARED_LIBRARY)
endif # TARGET_HAS_SPLIT_A2DP_FEATURE
