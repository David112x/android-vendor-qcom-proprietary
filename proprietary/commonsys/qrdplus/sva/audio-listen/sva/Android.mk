ifneq ($(TARGET_HAS_LOW_RAM),true)

LOCAL_PATH := $(call my-dir)

#=============================================
#  Generate the SVAWrapper JACK Library
#=============================================
include $(CLEAR_VARS)

#include all original java files
LOCAL_SRC_FILES += $(call all-java-files-under, src-Wrapper)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := SVAWrapper

# avoid compile error for using hide APIs
LOCAL_PRIVATE_PLATFORM_APIS:=true

include $(BUILD_STATIC_JAVA_LIBRARY)

#=============================================
#  Listen App using SoundTrigger
#=============================================
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_PROGUARD_FLAG_FILES := proguard.flags

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := SnapdragonSVA

LOCAL_OVERRIDES_PACKAGES := SVA

LOCAL_DEX_PREOPT := nostripping

LOCAL_CERTIFICATE := platform

# avoid compile error for using hide APIs
LOCAL_PRIVATE_PLATFORM_APIS:=true

LOCAL_STATIC_JAVA_LIBRARIES := Listen
LOCAL_STATIC_JAVA_LIBRARIES += SVAWrapper

include $(BUILD_PACKAGE)

endif
