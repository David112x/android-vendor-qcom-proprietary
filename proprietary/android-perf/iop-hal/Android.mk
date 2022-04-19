LOCAL_PATH:= $(call my-dir)

    ifeq ($(GENERIC_ODM_IMAGE),true)
    include $(CLEAR_VARS)
    LOCAL_MODULE := init.pixel.vendor.qti.hardware.iop@2.0-service.rc
    LOCAL_MODULE_TAGS := optional
    LOCAL_MODULE_CLASS := ETC
    LOCAL_SRC_FILES := $(LOCAL_MODULE)
    LOCAL_MODULE_PATH := $(TARGET_OUT_ODM_ETC)/init
    include $(BUILD_PREBUILT)
    endif

ifeq ($(call is-board-platform-in-list, kona msmnile sm6150 sdm845 trinket bengal lito atoll ),true)
    include $(CLEAR_VARS)
    LOCAL_MODULE_RELATIVE_PATH := hw
    LOCAL_PROPRIETARY_MODULE := true
    LOCAL_MODULE := vendor.qti.hardware.iop@2.0-service
    LOCAL_INIT_RC := vendor.qti.hardware.iop@2.0-service.rc

    LOCAL_HEADER_LIBRARIES := libcutils_headers

    LOCAL_SRC_FILES := \
            service.cpp

    LOCAL_C_INCLUDES := \
          vendor/qcom/proprietary/android-perf/mp-ctl \
          vendor/qcom/proprietary/commonsys-intf/android-perf/mp-ctl

    LOCAL_SHARED_LIBRARIES := \
            liblog \
            libcutils \
            libdl \
            libbase \
            libutils \
            libhardware_legacy \
            libhardware \
            libhwbinder \
            libhidlbase \
            libhidltransport \
            libqti-perfd-client \
            vendor.qti.hardware.iop@2.0 \

    include $(BUILD_EXECUTABLE)
endif
