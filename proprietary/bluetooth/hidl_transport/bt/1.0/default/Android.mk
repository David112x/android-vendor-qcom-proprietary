LOCAL_PATH:= $(call my-dir)

#If you want to use the legacy way(by using NV) of set the BT address
#You can set the value to 0
BT_QSOC_GET_ITEMS_FROM_PROPERTY = 0
BT_QSOC_GET_ITEMS_FROM_PERSIST = 1

include $(CLEAR_VARS)

LOCAL_COPY_HEADERS    := bt_nv.h

LOCAL_C_INCLUDES:= $(LOCAL_PATH)

LOCAL_SRC_FILES:=  bt_nv.cpp
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc \

LOCAL_MODULE:= libbtnv
LOCAL_MODULE_PATH_32 := $(TARGET_OUT_VENDOR)/lib
LOCAL_MODULE_PATH_64 := $(TARGET_OUT_VENDOR)/lib64
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.bluetooth@1.0-impl-qti
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(UART_BAUDRATE_3_0_MBPS),true)
LOCAL_CPPFLAGS := -DUART_BAUDRATE_3_0_MBPS
endif

LOCAL_SRC_FILES := \
    bluetooth_hci.cpp \
    bluetooth_address.cpp \
    async_fd_watcher.cpp \
    hci_packetizer.cpp \
    data_handler.cpp \
    uart_controller.cpp \
    patch_dl_manager.cpp \
    hci_uart_transport.cpp \
    power_manager.cpp \
    ibs_handler.cpp \
    logger.cpp \
    uart_ipc.cpp \
    ring_buffer.cpp \
    wake_lock.cpp \
    diag_interface.cpp \
    mct_controller.cpp \
    hci_mct_transport.cpp \
    nvm_tags_manager.cpp \
    health_info_log.cpp \
    state_info.cpp

LOCAL_CFLAGS += -DDIAG_ENABLED
LOCAL_CFLAGS += -Werror=unused-variable
# disable below flag to disable IBS
LOCAL_CFLAGS += -DWCNSS_IBS_ENABLED
# Disable this flag for disabling wakelocks
LOCAL_CFLAGS += -DWAKE_LOCK_ENABLED

ifeq ($(BOARD_HAVE_QTI_BT_LAZY_SERVICE),true)
LOCAL_CFLAGS += -DLAZY_SERVICE
endif

ifneq (,$(filter userdebug eng,$(TARGET_BUILD_VARIANT)))
LOCAL_CFLAGS += -DDUMP_IPC_LOG -DDUMP_RINGBUF_LOG -DDETECT_SPURIOUS_WAKE -DENABLE_HEALTH_TIMER
#LOCAL_CFLAGS += -DDUMP_HEALTH_INFO_TO_FILE
#LOCAL_C_INCLUDES += vendor/qcom/proprietary/bt/hci_qcomm_init
LOCAL_CFLAGS += -DENABLE_FW_CRASH_DUMP -DUSER_DEBUG
endif

LOCAL_C_INCLUDES += vendor/qcom/proprietary/qmi/inc
LOCAL_C_INCLUDES += vendor/qcom/proprietary/qmi/platform
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi-framework/inc
LOCAL_C_INCLUDES += vendor/qcom/proprietary/qcril-qmi-services/
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_ADDITIONAL_DEPENDENCIES += \
$(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES := \
    libbase \
    libcutils \
    libhidlbase \
    libhidltransport \
    liblog \
    libutils \
    libdiag \
    libqmiservices \
    libqmi_cci \
    libbtnv \
    android.hardware.bluetooth@1.0

LOCAL_HEADER_LIBRARIES := libril-qc-qmi-services-headers

include $(BUILD_SHARED_LIBRARY)

#service

include $(CLEAR_VARS)
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
ifeq ($(BOARD_HAVE_QTI_BT_LAZY_SERVICE),true)
LOCAL_MODULE := android.hardware.bluetooth@1.0-service-qti-lazy
LOCAL_INIT_RC := android.hardware.bluetooth@1.0-service-qti-lazy.rc
LOCAL_CPPFLAGS += -DLAZY_SERVICE
else
LOCAL_MODULE := android.hardware.bluetooth@1.0-service-qti
LOCAL_INIT_RC := android.hardware.bluetooth@1.0-service-qti.rc
endif #BOARD_HAVE_QTI_BT_LAZY_SERVICE
LOCAL_SRC_FILES := \
  service.cpp

LOCAL_SHARED_LIBRARIES := \
  liblog \
  libcutils \
  libutils \
  libhwbinder \
  libhidlbase \
  libhidltransport \
  android.hardware.bluetooth@1.0 \

# include FM HAL only for non-Pronto/non-ROME/non-Hastings targets
ifneq ($(filter msm8909 msm8952 msm8937 msm8953 msm8992 msm8994 msm8996 kona,$(TARGET_BOARD_PLATFORM)),)
else
ifeq ($(BOARD_HAVE_QCOM_FM),true)
LOCAL_SHARED_LIBRARIES += vendor.qti.hardware.fm@1.0
LOCAL_CPPFLAGS += -DQCOM_FM_SUPPORTED
endif #BOARD_HAVE_QCOM_FM
endif

ifneq ($(filter msm8909 msm8937 msm8953,$(TARGET_BOARD_PLATFORM)),)
else
ifeq ($(BOARD_ANT_WIRELESS_DEVICE),"qualcomm-hidl")
LOCAL_SHARED_LIBRARIES += com.qualcomm.qti.ant@1.0
LOCAL_CPPFLAGS += -DQCOM_ANT_SUPPORTED
endif
endif

ifeq ($(TARGET_USE_QTI_BT_SAR),true)
LOCAL_SHARED_LIBRARIES += vendor.qti.hardware.bluetooth_sar@1.0
LOCAL_CPPFLAGS += -DQTI_BT_SAR_SUPPORTED
endif # TARGET_USE_QTI_BT_SAR

ifeq ($(TARGET_USE_QTI_BT_CONFIGSTORE),true)
LOCAL_SHARED_LIBRARIES += vendor.qti.hardware.btconfigstore@1.0
LOCAL_CPPFLAGS += -DQTI_BT_CONFIGSTORE_SUPPORTED
endif # TARGET_USE_QTI_BT_CONFIGSTORE

ifeq ($(TARGET_USE_QTI_VND_FWK_DETECT),true)
LOCAL_SHARED_LIBRARIES += libqti_vndfwk_detect
LOCAL_CPPFLAGS += -DQTI_VND_FWK_DETECT_SUPPORTED
endif # TARGET_USE_QTI_VND_FWK_DETECT

include $(BUILD_EXECUTABLE)
