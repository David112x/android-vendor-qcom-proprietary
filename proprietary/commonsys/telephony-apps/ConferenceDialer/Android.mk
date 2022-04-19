ifneq ($(TARGET_NO_TELEPHONY), true)
ifneq ($(TARGET_HAS_LOW_RAM),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

BUILD_TOP := ../../../../../../
phone_common_dir := $(BUILD_TOP)/packages/apps/PhoneCommon
chips_dir := $(BUILD_TOP)/frameworks/opt/chips
$(warning $(phone_common_dir))

src_dirs := src  $(phone_common_dir)/src
res_dirs := res $(phone_common_dir)/res $(chips_dir)/res

LOCAL_SRC_FILES := $(call all-java-files-under, $(src_dirs))
LOCAL_RESOURCE_DIR := $(addprefix $(LOCAL_PATH)/, $(res_dirs))

LOCAL_AAPT_FLAGS := \
    --auto-add-overlay \
    --extra-packages com.android.contacts.common \
    --extra-packages com.android.ex.chips \
    --extra-packages com.android.phone.common

LOCAL_JAVA_LIBRARIES := telephony-common
LOCAL_STATIC_JAVA_LIBRARIES := \
    com.android.vcard \
    android-common \
    guava \
    libchips \
    libphonenumber \

LOCAL_PACKAGE_NAME := ConferenceDialer
LOCAL_CERTIFICATE := platform
LOCAL_PRIVILEGED_MODULE := false
LOCAL_PROGUARD_FLAG_FILES := proguard.flags
LOCAL_PRODUCT_MODULE := true
LOCAL_PRIVATE_PLATFORM_APIS := true

include $(BUILD_PACKAGE)

# Use the following include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))

endif
endif # TARGET_NO_TELEPHONY
