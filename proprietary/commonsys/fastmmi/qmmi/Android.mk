TOP_LOCAL_PATH := $(call my-dir)
#
# Build JNI library
#
include $(TOP_LOCAL_PATH)/jni/Android.mk

#
# Build sample application package
#
LOCAL_PATH:= $(TOP_LOCAL_PATH)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-subdir-java-files)
LOCAL_PACKAGE_NAME := Qmmi
LOCAL_CERTIFICATE := platform
LOCAL_PRIVATE_PLATFORM_APIS := true

#LOCAL_STATIC_JAVA_LIBRARIES := wigig_manager \

LOCAL_STATIC_JAVA_LIBRARIES := vendor.qti.hardware.sensorscalibrate-V1.0-java \
                               android.hidl.manager-V1.0-java \
                               android.hardware.light-V2.0-java \
                               vendor.qti.hardware.factory-V1.1-java \
                               vendor.qti.hardware.fingerprint-V1.0-java \
                               android-support-v4

LOCAL_JNI_SHARED_LIBRARIES := libmmi_jni

LOCAL_MODULE_TAGS := optional

LOCAL_PROGUARD_ENABLED := disabled
include $(BUILD_PACKAGE)
