ifneq ($(TARGET_NO_TELEPHONY), true)

ifeq ($(TARGET_FWK_SUPPORTS_FULL_VALUEADDS), true)
 LOCAL_PATH:= $(call my-dir)
 include $(CLEAR_VARS)

 LOCAL_SRC_FILES := $(call all-subdir-java-files)

 LOCAL_JAVA_LIBRARIES := telephony-common telephony-ext
 LOCAL_STATIC_JAVA_LIBRARIES := android-support-v4

 LOCAL_PACKAGE_NAME :=SimSettings

 LOCAL_CERTIFICATE := platform
 LOCAL_PRIVATE_PLATFORM_APIS := true
 LOCAL_PRODUCT_MODULE := true

 include $(BUILD_PACKAGE)
endif

endif # TARGET_NO_TELEPHONY
