LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.bluetooth_sar@1.0-impl
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true

LOCAL_SRC_FILES := \
    BluetoothSar.cpp

BT_HAL_DIR:= vendor/qcom/proprietary/bluetooth/hidl_transport/bt/1.0/default

LOCAL_C_INCLUDES +=  $(BT_HAL_DIR)
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc

LOCAL_SHARED_LIBRARIES := \
    libhidlbase \
    libhidltransport \
    libutils \
    liblog \
    libhwbinder \
    libbase \
    libcutils \
    libhardware \
    vendor.qti.hardware.bluetooth_sar@1.0 \
    android.hardware.bluetooth@1.0 \
    android.hardware.bluetooth@1.0-impl-qti \

include $(BUILD_SHARED_LIBRARY)