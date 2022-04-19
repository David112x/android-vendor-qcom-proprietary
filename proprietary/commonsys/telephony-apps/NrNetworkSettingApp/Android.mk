LOCAL_PATH:= $(call my-dir)

##############################################################
ifneq ($(TARGET_HAS_LOW_RAM),true)
ifneq ($(TARGET_SUPPORTS_ANDROID_WEAR),true)

include $(CLEAR_VARS)

res_dirs := res
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-java-files-under, src/org/codeaurora/qti/nrNetworkSettingApp/)

LOCAL_PACKAGE_NAME := NrNetworkSettingApp
LOCAL_JAVA_LIBRARIES += telephony-ext \
                        telephony-common \
                        framework \
                        android.hardware.radio-V1.0-java \
                        android.hidl.manager-V1.0-java \

LOCAL_RESOURCE_DIR := $(addprefix $(LOCAL_PATH)/, $(res_dirs))

LOCAL_PROGUARD_ENABLED := disabled
LOCAL_PRIVILEGED_MODULE := true
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PRODUCT_MODULE := true

LOCAL_STATIC_JAVA_LIBRARIES := vendor.qti.hardware.radio.qtiradio-V2.0-java

LOCAL_CERTIFICATE := platform
include $(BUILD_PACKAGE)

endif
endif
##############################################################

# This finds and builds the test apk as well, so a single make does both.
include $(call all-makefiles-under,$(LOCAL_PATH))
