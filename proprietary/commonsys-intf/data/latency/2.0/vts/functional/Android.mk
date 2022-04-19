LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE        := VtsHalDataLatencyV2_0_IFactoryV2_0TargetTest
LOCAL_MODULE_TAGS   := optional
LOCAL_CFLAGS        := -O0 -g

LOCAL_SRC_FILES     += VtsHalDataLatencyV2_0_IFactoryV2_0TargetTest.cpp

LOCAL_SHARED_LIBRARIES := liblog \
                          libhidlbase \
                          libhidltransport \
                          libutils \
                          vendor.qti.data.factory@2.0 \
                          vendor.qti.latency@2.0

LOCAL_STATIC_LIBRARIES := VtsHalHidlTargetTestBase

LOCAL_MODULE_OWNER := qti

include $(BUILD_NATIVE_TEST)
