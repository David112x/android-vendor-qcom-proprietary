ifeq ($(call is-vendor-board-platform,QCOM),true)
ifeq ($(findstring true, $(BOARD_HAVE_QCOM_FM) $(BOARD_HAVE_BLUETOOTH)),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(TARGET_SUPPORTS_WEARABLES),true)
QTI_DIR  := $(BOARD_BT_DIR)/libbt-vendor
BDROID_DIR:= system/bt
else
QTI_DIR := hardware/qcom/bt/libbt-vendor
BDROID_DIR:= vendor/qcom/opensource/commonsys/system/bt
endif

LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(BDROID_DIR)/hci/include
LOCAL_C_INCLUDES += system/bt/hci/include
LOCAL_C_INCLUDES += $(QTI_DIR)/include
ifeq ($(TARGET_SUPPORTS_WEARABLES),true)
LOCAL_C_INCLUDES += $(BOARD_OPENSOURCE_DIR)/bluetooth/tools/hidl_client/inc
else
LOCAL_C_INCLUDES += vendor/qcom/proprietary/bluetooth/hidl_client/inc
LOCAL_C_INCLUDES += hardware/qcom/bt/tools/hidl_client/inc
endif

LOCAL_CFLAGS := -DANDROID

ifneq ($(BOARD_ANT_WIRELESS_DEVICE), )
LOCAL_CFLAGS += -DCONFIG_ANT
endif
LOCAL_SRC_FILES := wds_main.c
LOCAL_SRC_FILES += wds_hci_pfal_linux.c

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_EXECUTABLES)
LOCAL_MODULE := wdsdaemon
ifeq ($(PRODUCT_VENDOR_MOVE_ENABLED),true)
LOCAL_PROPRIETARY_MODULE := true
endif
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libdiag
LOCAL_SHARED_LIBRARIES += libcutils \
                          libdl \
                          libbt-hidlclient

include $(BUILD_EXECUTABLE)
endif  # filter
endif  # is-vendor-board-platform
