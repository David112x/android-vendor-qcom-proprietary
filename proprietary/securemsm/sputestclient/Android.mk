ifeq ($(call is-vendor-board-platform,QCOM),true)
ifeq ($(call is-board-platform-in-list, kona),true)
ifneq ($(call is-product-in-list, qssi qssi_32),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := vendor.qti.spu@1.0-test-client
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := qti

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/inc \
    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
    $(TARGET_OUT_HEADERS)/common/inc \
    $(LOCAL_PATH)/../../common/qti-utils

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_CFLAGS+= -Werror -Wall -Wno-error=date-time

LOCAL_SRC_FILES := src/client.cpp src/SpcomSSREvent.cpp src/SPUHidlClientTest.cpp

ifeq ($(call is-board-platform-in-list,kona),true)
    LOCAL_CFLAGS += -DSPSS_TARGET=8250
endif

LOCAL_SHARED_LIBRARIES := \
    libhidlbase \
    libhidltransport \
    liblog \
    libutils \
    libqti-utils \
    vendor.qti.spu@1.0 \
    libhidlmemory \
    android.hidl.memory@1.0 \
    android.hidl.allocator@1.0

include $(BUILD_EXECUTABLE)

endif
endif
endif
