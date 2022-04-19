LOCAL_PATH := $(call my-dir)

################################################################################

include $(CLEAR_VARS)
SECUREMSM_SHIP_PATH   := vendor/qcom/proprietary/securemsm
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE := libcryptfshwcommon

LOCAL_C_INCLUDES +=    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
		 +=    $(TARGET_OUT_HEADERS)/common/inc \
		 +=    $(SECUREMSM_SHIP_PATH)/QSEEComAPI \

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SRC_FILES := \
    cryptfs_hw.c \

ifeq ($(TARGET_USES_QMAA),true)
ifneq ($(TARGET_USES_QMAA_OVERRIDE_CRYPTFSHW),true)
LOCAL_CFLAGS += -DFEATURE_QMAA_CRYPTFSHW
endif
endif

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libutils \
    libdl \
    libhardware \
    libQSEEComAPI \

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE       := vendor.qti.hardware.cryptfshw@1.0-service-qti.rc
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)/init
include $(BUILD_PREBUILT)
