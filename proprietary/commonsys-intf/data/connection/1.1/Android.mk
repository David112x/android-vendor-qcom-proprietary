
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := vendor.qti.hardware.data.connection-V1.1-java.xml

LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_PRODUCT_ETC)/permissions
LOCAL_PRODUCT_MODULE := true

LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)
include $(call all-makefiles-under,$(LOCAL_PATH))
