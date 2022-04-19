# verify it isn't a simulator build
ifeq ($(call is-vendor-board-platform,QCOM),true)
ifeq ($(call is-board-platform-in-list,apq8098_latv sdmshrike msmnile kona),true)

ifneq ($(call is-product-in-list, qssi qssi_32),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := spdaemon.c
ifeq ($(call is-board-platform-in-list,kona sm8250),true)
LOCAL_CFLAGS+= -DFEATURE_UIM_REMOTE
LOCAL_SRC_FILES +=  sp_jc_msg.c
LOCAL_SRC_FILES +=  sp_uim_remote.cpp
endif

ifeq ($(call is-board-platform-in-list,kona sm8250),true)
LOCAL_CFLAGS+= -DFEATURE_IAR
endif

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../securemsm/spcomlib/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/libmdmdetect/inc/
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/libperipheralclient/inc/

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES := libspcom libutils libcutils liblog libmdmdetect

LOCAL_INIT_RC := init.spdaemon.rc

ifneq ($(call is-board-platform-in-list,msmnile sdmshrike  kona),true)
# to use the old PIL (not from kernel)
LOCAL_SHARED_LIBRARIES += libperipheral_client
endif
ifeq ($(call is-board-platform-in-list,kona sm8250),true)
LOCAL_SHARED_LIBRARIES += libhidlbase
LOCAL_SHARED_LIBRARIES += libhidltransport
LOCAL_SHARED_LIBRARIES += libhwbinder
LOCAL_SHARED_LIBRARIES += vendor.qti.hardware.radio.uim_remote_client@1.0
LOCAL_SHARED_LIBRARIES += vendor.qti.hardware.radio.uim_remote_client@1.1
LOCAL_SHARED_LIBRARIES += vendor.qti.hardware.radio.uim_remote_client@1.2
endif

LOCAL_CFLAGS += -fno-exceptions -fno-short-enums -DANDROID

ifeq ($(TARGET_BUILD_VARIANT),user)
LOCAL_CFLAGS+= -DUSER_BUILD
endif

ifeq ($(call is-board-platform-in-list,kona),true)
    LOCAL_CFLAGS += -DSPSS_COMPONENT_API=2
    LOCAL_CFLAGS += -DSPSS_TARGET=8250
else
    LOCAL_CFLAGS += -DSPSS_COMPONENT_API=1
    LOCAL_CFLAGS += -DSPSS_TARGET=8150
endif

LOCAL_MODULE := spdaemon

LOCAL_MODULE_TAGS := optional

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qti

ifeq ($(PRODUCT_VENDOR_MOVE_ENABLED),true)
LOCAL_PROPRIETARY_MODULE := true
endif

include $(BUILD_EXECUTABLE)

endif
endif
endif

