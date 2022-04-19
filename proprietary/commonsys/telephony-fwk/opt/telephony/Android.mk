ifeq ($(TARGET_FWK_SUPPORTS_FULL_VALUEADDS), true)

ifneq ($(TARGET_NO_TELEPHONY), true)
ifeq ($(TARGET_BUILD_JAVA_SUPPORT_LEVEL),platform)

LOCAL_PATH := $(call my-dir)

# ==========================================================================

include $(CLEAR_VARS)

LOCAL_AIDL_INCLUDES := $(LOCAL_PATH)/src/java
LOCAL_SRC_FILES := $(call all-java-files-under, src) \
	$(call all-Iaidl-files-under, src/java) \
	$(call all-logtags-files-under, src/java)

LOCAL_JAVA_LIBRARIES := voip-common ims-common telephony-common telephony-ext \
                        android.hardware.radio-V1.0-java qti-telephony-hidl-wrapper \
                        qti-telephony-utils

LOCAL_STATIC_JAVA_LIBRARIES := qcrilhook-static vendor.qti.hardware.radio.qtiradio-V1.0-java \
                               vendor.qti.hardware.radio.qtiradio-V2.0-java \
                               vendor.qti.hardware.radio.qtiradio-V2.1-java \
                               vendor.qti.hardware.radio.qtiradio-V2.2-java \
                               vendor.qti.hardware.radio.qtiradio-V2.3-java \
                               vendor.qti.hardware.radio.qtiradio-V2.4-java

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := qti-telephony-common
LOCAL_PRODUCT_MODULE := true

include $(BUILD_JAVA_LIBRARY)

# ==========================================================================

include $(CLEAR_VARS)

LOCAL_MODULE := qti_telephony_common.xml

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_CLASS := ETC

# This will install the file in /product/etc/permissions
#
LOCAL_MODULE_PATH := $(TARGET_OUT_PRODUCT_ETC)/permissions

LOCAL_SRC_FILES := $(LOCAL_MODULE)

include $(BUILD_PREBUILT)

# ==========================================================================

include $(call all-makefiles-under,$(LOCAL_PATH))

# ==========================================================================

endif # JAVA platform
endif#TARGET_USES_QCOM_BSP_ATEL
endif # TARGET_NO_TELEPHONY
