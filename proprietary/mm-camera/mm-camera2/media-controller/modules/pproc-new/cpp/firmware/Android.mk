LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE       := cpp_firmware_v1_1_1.fw
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR)/firmware
LOCAL_SRC_FILES    := $(LOCAL_MODULE)

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := cpp_firmware_v1_1_6.fw
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR)/firmware
LOCAL_SRC_FILES    := $(LOCAL_MODULE)

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := cpp_firmware_v1_2_0.fw
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR)/firmware
LOCAL_SRC_FILES    := $(LOCAL_MODULE)

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := cpp_firmware_v1_6_0.fw
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR)/firmware
LOCAL_SRC_FILES    := $(LOCAL_MODULE)

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := cpp_firmware_v1_8_0.fw
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR)/firmware
LOCAL_SRC_FILES    := $(LOCAL_MODULE)

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := cpp_firmware_v1_4_0.fw
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR)/firmware
LOCAL_SRC_FILES    := $(LOCAL_MODULE)

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := cpp_firmware_v1_10_0.fw
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR)/firmware
LOCAL_SRC_FILES    := $(LOCAL_MODULE)

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := cpp_firmware_v1_12_1.fw
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR)/firmware
LOCAL_SRC_FILES    := $(LOCAL_MODULE)

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif


include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := cpp_firmware_v1_12_0.fw
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR)/firmware
LOCAL_SRC_FILES    := $(LOCAL_MODULE)

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := cpp_firmware_v1_5_0.fw
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR)/firmware
LOCAL_SRC_FILES    := $(LOCAL_MODULE)

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := cpp_firmware_v1_5_1.fw
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR)/firmware
LOCAL_SRC_FILES    := $(LOCAL_MODULE)

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := cpp_firmware_v1_5_2.fw
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR)/firmware
LOCAL_SRC_FILES    := $(LOCAL_MODULE)

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_PREBUILT)
