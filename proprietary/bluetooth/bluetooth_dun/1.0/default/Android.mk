ifeq ($(TARGET_HAS_DUN_HIDL_FEATURE), true)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := vendor.qti.hardware.bluetooth_dun@1.0-impl
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true

LOCAL_SRC_FILES := BluetoothDunServer.cpp

PORT_BRIDGE_INTF_DIR:= vendor/qcom/proprietary/data/port-bridge/inc
LOCAL_C_INCLUDES += $(PORT_BRIDGE_INTF_DIR)
LOCAL_SHARED_LIBRARIES := \
    libbase \
    libcutils \
    libhidlbase \
    libhidltransport \
    liblog \
    libutils \
    vendor.qti.hardware.bluetooth_dun@1.0

include $(BUILD_SHARED_LIBRARY)

#service

include $(CLEAR_VARS)
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE := vendor.qti.hardware.bluetooth_dun@1.0-service
LOCAL_INIT_RC := vendor.qti.hardware.bluetooth_dun@1.0-service.rc
LOCAL_SRC_FILES := \
  service.cpp

LOCAL_SHARED_LIBRARIES := \
  liblog \
  libcutils \
  libutils \
  libhwbinder \
  libhidlbase \
  libhidltransport \
  vendor.qti.hardware.bluetooth_dun@1.0

ifeq ($(TARGET_USE_QTI_VND_FWK_DETECT),true)
LOCAL_SHARED_LIBRARIES += libqti_vndfwk_detect
LOCAL_CPPFLAGS += -DQTI_VND_FWK_DETECT_SUPPORTED
endif # TARGET_USE_QTI_VND_FWK_DETECT

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
ifeq ($(GENERIC_ODM_IMAGE), true)
LOCAL_MODULE := vendor.qti.hardware.bluetooth_dun@1.0-service.disable.rc
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_PATH := $(TARGET_OUT_ODM_ETC)/init
include $(BUILD_PREBUILT)
endif

endif #TARGET_HAS_DUN_HIDL_FEATURE