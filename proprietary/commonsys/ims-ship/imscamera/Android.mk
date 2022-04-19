
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += liblog


LOCAL_SHARED_LIBRARIES += libnativehelper
LOCAL_SHARED_LIBRARIES += libandroid_runtime
LOCAL_SHARED_LIBRARIES += libgui
LOCAL_SHARED_LIBRARIES += libui
LOCAL_SHARED_LIBRARIES += libmedia
LOCAL_SHARED_LIBRARIES += libbinder
#LOCAL_SHARED_LIBRARIES += libcameraservice
LOCAL_SHARED_LIBRARIES += libcamera_client
LOCAL_SHARED_LIBRARIES += libhardware
LOCAL_SHARED_LIBRARIES += \
    liblog \
    libcutils \
    libdl \
    libbase \
    libutils \
    libhardware_legacy \
    libhardware \
    libhidlbase \
    libhidlmemory \
    android.hidl.allocator@1.0 \
    android.hidl.memory@1.0 \
    android.hardware.media.omx@1.0 \
    android.hardware.camera.common@1.0 \
    android.hardware.camera.provider@2.4 \
    android.hardware.camera.device@1.0 \
    vendor.qti.hardware.camera.device@1.0 \
    android.hardware.camera.device@3.2 \
    android.hardware.camera.device@3.4 \
    android.hardware.camera.provider@2.5
ifeq ($(DEFAULT_PLATFORM_VERSION),OPR1)
LOCAL_SHARED_LIBRARIES += android.hidl.manager@1.0
endif
LOCAL_SHARED_LIBRARIES += \
    libhwbinder \
    libhidlbase \
    libhidltransport
LOCAL_SRC_FILES += src/ImsCameraImplementation.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/src/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += frameworks/base/include/binder
LOCAL_C_INCLUDES += frameworks/base/include/gui
LOCAL_C_INCLUDES += frameworks/native/include/binder
LOCAL_C_INCLUDES += frameworks/av/include/camera/android/hardware
LOCAL_C_INCLUDES += frameworks/av/include/camera
LOCAL_C_INCLUDES += system/media/camera/include


ifeq ($(call is-board-platform,msm8974),true)
LOCAL_CFLAGS +=  -DB_FAMILY
LOCAL_CPPFLAGS +=  -DB_FAMILY
endif

ifeq ($(call is-board-platform,msm8226),true)
LOCAL_CFLAGS +=  -DB_FAMILY
LOCAL_CPPFLAGS +=  -DB_FAMILY
endif

ifeq ($(call is-board-platform,apq8084),true)
LOCAL_CFLAGS +=  -DB_FAMILY
LOCAL_CPPFLAGS +=  -DB_FAMILY
endif

LOCAL_C_INCLUDES += frameworks/base/include/media
LOCAL_C_INCLUDES += hardware/libhardware/modules/gralloc/
LOCAL_C_INCLUDES += $(JNI_H_INCLUDE)

LOCAL_C_INCLUDES += frameworks/av/services/camera/libcameraservice/
LOCAL_C_INCLUDES += frameworks/av/services/camera/libcameraservice/device1/
LOCAL_C_INCLUDES += hardware/libhardware/include/hardware/


LOCAL_MODULE:= lib-imscamera
LOCAL_SANITIZE:=integer_overflow

LOCAL_MODULE_TAGS:=optional

LOCAL_PRELINK_MODULE := false
LOCAL_PRODUCT_MODULE := true
include $(BUILD_SHARED_LIBRARY)
