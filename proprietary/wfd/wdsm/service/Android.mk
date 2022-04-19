LOCAL_PATH := $(call my-dir)

#=============================================
#  Wfd Service APK
#=============================================
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_PRIVATE_PLATFORM_APIS := true

LOCAL_SRC_FILES := $(call all-java-files-under, src/com/qualcomm/wfd) \
                src/com/qualcomm/compat/VersionedInputManager.java \
                src/com/qualcomm/wfd/service/ISessionManagerService.aidl \
                src/com/qualcomm/wfd/service/IWfdActionListener.aidl \
                src/com/qualcomm/wfd/service/IHIDEventListener.aidl \
				src/com/qualcomm/wfd/service/IWfdSession.aidl

ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)
LOCAL_SRC_FILES += src/com/qualcomm/compat/IcsInputManager.java
else
LOCAL_SRC_FILES += src/com/qualcomm/compat/JbInputManager.java
endif

LOCAL_AIDL_INCLUDES += frameworks/native/aidl/gui

LOCAL_PACKAGE_NAME := WfdService

LOCAL_CERTIFICATE := platform

# Usage ex: mmm wdsm/service/ WFD_DISABLE_JACK=1
$(info JACK DISABLED in WFD Service = $(WFD_DISABLE_JACK))
ifeq ($(WFD_DISABLE_JACK),1)
LOCAL_JACK_ENABLED := disabled
endif
LOCAL_PRIVILEGED_MODULE := true
include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
