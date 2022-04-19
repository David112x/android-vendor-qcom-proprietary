LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libpanoramaref_jni

LOCAL_SRC_FILES := StandalonePanoramaEngine.cpp rpcmem_android.c

LOCAL_C_INCLUDES := vendor/qcom/proprietary/scve/reference/Panorama/jni/util \
                    vendor/qcom/proprietary/scve/reference/Panorama/jni/exifhelper \
                    vendor/qcom/proprietary/adsprpc/inc

LOCAL_HEADER_LIBRARIES := libfastrpc_headers

LOCAL_SHARED_LIBRARIES := liblog libdl vendor.qti.hardware.scve.panorama@1.0 \
                          libutils libcutils libhardware libhidlbase libhidltransport libhwbinder

LOCAL_STATIC_LIBRARIES := libpanoramaref_util libpanoramaref_exifhelper

include vendor/qcom/proprietary/scve/reference/common.mk

LOCAL_PRIVATE_PLATFORM_APIS := true

include $(BUILD_SHARED_LIBRARY)
