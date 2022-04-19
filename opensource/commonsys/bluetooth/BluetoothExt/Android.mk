LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := \
        $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := BluetoothExt
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_CERTIFICATE := platform
LOCAL_JAVA_LIBRARIES := javax.obex
LOCAL_STATIC_JAVA_LIBRARIES := vendor.qti.hardware.bluetooth_dun-V1.0-java

LOCAL_PROGUARD_ENABLED := disabled
include $(BUILD_PACKAGE)
include $(call all-makefiles-under,$(LOCAL_PATH))
