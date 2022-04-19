ifneq ($(TARGET_HAS_LOW_RAM), true)
LOCAL_PATH := $(call my-dir)
SRC_PATH := /src/com/qualcomm/qti/services/secureui/
include $(CLEAR_VARS)

LOCAL_PACKAGE_NAME := com.qualcomm.qti.services.secureui

LOCAL_MODULE_TAGS := optional

#LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_SRC_FILES := $(SRC_PATH)/BootReceiver.java \
                   $(SRC_PATH)/CallReceiver.java \
                   $(SRC_PATH)/OrientationActivity.java \
                   $(SRC_PATH)/PowerReceiver.java \
                   $(SRC_PATH)/ScreenReceiver.java \
                   $(SRC_PATH)/SecureUIService.java \

LOCAL_PROGUARD_FLAG_FILES := proguard.flags
LOCAL_CERTIFICATE := platform
LOCAL_PRODUCT_MODULE := true
LOCAL_JAVA_LIBRARIES := telephony-common

LOCAL_PRIVATE_PLATFORM_APIS := true

include $(BUILD_PACKAGE)

include $(LOCAL_PATH)/jni_service/Android.mk
endif
