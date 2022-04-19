ifneq ($(TARGET_HAS_LOW_RAM), true)
LOCAL_PATH := $(call my-dir)
SRC_PATH := /src/com/qualcomm/qti/services/systemhelper/
include $(CLEAR_VARS)

LOCAL_PACKAGE_NAME := com.qualcomm.qti.services.systemhelper

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(SRC_PATH)/BootReceiver.java \
                   $(SRC_PATH)/CallReceiver.java \
                   $(SRC_PATH)/PowerReceiver.java \
                   $(SRC_PATH)/ScreenReceiver.java \
                   $(SRC_PATH)/SysHelperService.java

LOCAL_JNI_SHARED_LIBRARIES := libsystemhelper_jni
LOCAL_CERTIFICATE := platform
LOCAL_PRODUCT_MODULE := true

LOCAL_PRIVATE_PLATFORM_APIS := true

include $(BUILD_PACKAGE)

include $(LOCAL_PATH)/jni_service/Android.mk
endif
