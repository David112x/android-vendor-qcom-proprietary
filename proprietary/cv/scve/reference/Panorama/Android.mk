TOP_LOCAL_PATH := $(call my-dir)

#
# Build JNI library
#

include $(call all-subdir-makefiles)

#
# Build sample application package
#

LOCAL_PATH:= $(TOP_LOCAL_PATH)
include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(PRODUCT_OUT)/system/app
LOCAL_STATIC_JAVA_LIBRARIES += android-support-v4

LOCAL_JNI_SHARED_LIBRARIES  := libpanoramaref_imagebufferutils \
                               libpanoramaref_khr_egl_extensions \
                               libpanoramaref_khr_gles_extensions \
                               libpanoramaref_jni
LOCAL_SRC_FILES             := $(call all-subdir-java-files)
LOCAL_PACKAGE_NAME          := PanoramaRef
LOCAL_CERTIFICATE := platform
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE_TAGS := optional
#TARGET_BUILD_APPS           := true
LOCAL_PROGUARD_FLAG_FILES   := proguard-project.txt

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
   ifeq ($(CDSP_ENABLE),true)
      LOCAL_JNI_SHARED_LIBRARIES += libcdsprpc
   else
      LOCAL_JNI_SHARED_LIBRARIES += libadsprpc
   endif
endif

include vendor/qcom/proprietary/scve/reference/common.mk

LOCAL_PRIVATE_PLATFORM_APIS := true

include $(BUILD_PACKAGE)
