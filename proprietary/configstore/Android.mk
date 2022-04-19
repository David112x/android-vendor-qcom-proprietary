LOCAL_PATH := $(call my-dir)

# For HIDL Impl library
include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.capabilityconfigstore@1.0-impl
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qti

LOCAL_SRC_FILES := \
    CapabilityConfigStore.cpp

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libbase \
    libutils \
    libhardware \
    libhidlbase \
    libhidltransport \
    libxml2 \
    vendor.qti.hardware.capabilityconfigstore@1.0


include $(BUILD_SHARED_LIBRARY)

# For HIDL Service bin
include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.capabilityconfigstore@1.0-service
LOCAL_INIT_RC :=vendor.qti.hardware.capabilityconfigstore@1.0-service.rc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := qti
LOCAL_SRC_FILES := \
    service.cpp

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libbase \
    libutils \
    libhardware \
    libhidlbase \
    libhidltransport \
    libxml2 \
    vendor.qti.hardware.capabilityconfigstore@1.0

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := xmlparser.cpp
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_IS_HOST_MODULE := true

LOCAL_MODULE := configstore_xmlparser

LOCAL_SHARED_LIBRARIES := \
    libxml2 \
    liblog

include $(BUILD_HOST_EXECUTABLE)


include $(CLEAR_VARS)
LOCAL_PREBUILT_EXECUTABLES := configparser.sh
LOCAL_MODULE_OWNER := qti
LOCAL_IS_HOST_MODULE := true

include $(BUILD_HOST_PREBUILT)


# Create dir configstore where xml's will reside
$(shell mkdir -p $(TARGET_OUT_VENDOR_ETC)/configstore)


#Example of what to add to  Android.mk to get the configstore xml's installed for configstore hal
#Please add and uncomment below lines in your Android.mk also please refer to the audio.xml located in this dir

#include $(CLEAR_VARS)
#LOCAL_MODULE := audio.xml
#LOCAL_SRC_FILES := $(LOCAL_MODULE)
#LOCAL_MODULE_CLASS := ETC
#LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)/configstore
#include $(BUILD_PREBUILT)

include $(LOCAL_PATH)/CapabilityConfigStoreTestapp/Android.mk
