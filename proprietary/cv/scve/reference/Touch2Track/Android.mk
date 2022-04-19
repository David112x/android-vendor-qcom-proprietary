LOCAL_PATH:= $(call my-dir)

#
# Build JNI library
#
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE:= false

LOCAL_CFLAGS           := -Wno-write-strings -DUSE_OPENGL_ES_2_0 #-Werror -std=gnu++11
LOCAL_SHARED_LIBRARIES := liblog libGLESv2 libdl libcutils libutils vendor.qti.hardware.scve.objecttracker@1.0 \
                          libhardware libhidlbase libhidltransport libhwbinder

LOCAL_HEADER_LIBRARIES := libfastrpc_headers

LOCAL_MODULE           := libfastcvsample

LOCAL_SRC_FILES        := jni/FastCVSample.cpp \
                          jni/FPSCounter.cpp \
                          jni/CameraRendererRGB565GL2.cpp \
                          jni/CameraUtil.cpp \
                          jni/rpcmem_android.c \
                          jni/FastCVSampleRenderer.cpp

LOCAL_C_INCLUDES       := vendor/qcom/proprietary/fastcv-noship/inc/fastcv \
                          vendor/qcom/proprietary/scve-noship/ObjectTracker/AL/api \
                          vendor/qcom/proprietary/scve-noship/common/AL/api \
                          vendor/qcom/proprietary/scve-noship/common/AL/Context \
                          vendor/qcom/proprietary/scve-noship/common/utils/inc \
                          vendor/qcom/proprietary/adsprpc/inc

LOCAL_MULTILIB := 32

include $(BUILD_SHARED_LIBRARY)

#
# Build sample application package
#
include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(PRODUCT_OUT)/system/app
LOCAL_STATIC_JAVA_LIBRARIES := android-support-v4
LOCAL_JNI_SHARED_LIBRARIES  := libfastcvsample
LOCAL_SRC_FILES             := $(call all-subdir-java-files)
LOCAL_PACKAGE_NAME          := Touch2Track
LOCAL_CERTIFICATE := platform
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE_TAGS := optional
#TARGET_BUILD_APPS           := true

ifeq ($(SCVE_MAKE_STANDALONE_APK),true)
   LOCAL_JNI_SHARED_LIBRARIES  += libjpeg \
                                  libc++ \
                                  libcutils \
                                  libutils \
                                  libbacktrace \
                                  libbase \
                                  libunwind \
                                  liblzma \
                                  libion
endif

LOCAL_MULTILIB := 32

include $(BUILD_PACKAGE)
