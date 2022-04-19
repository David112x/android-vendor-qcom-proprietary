LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := capabilityconfigstoretest
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := qti
LOCAL_SRC_FILES := capabilitytest.cpp
LOCAL_SHARED_LIBRARIES := \
                       liblog \
                       libcutils \
                       libdl \
                       libbase \
                       libutils \
                       libhardware \
                       libhidlbase \
                       libhidltransport \
                       libutils \
                       vendor.qti.hardware.capabilityconfigstore@1.0

include $(BUILD_EXECUTABLE)
