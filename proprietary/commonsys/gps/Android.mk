ifneq ($(BUILD_TINY_ANDROID),true)
ifneq ($(BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE),)
ifneq ($(BOARD_VENDOR_QCOM_LOC_PDK_FEATURE_SET),true)

#List of supported board platform targets
LOC_BOARD_PLATFORM_LIST := msm8960
LOC_BOARD_PLATFORM_LIST += msm8974
LOC_BOARD_PLATFORM_LIST += msm8226
LOC_BOARD_PLATFORM_LIST += msm8610
LOC_BOARD_PLATFORM_LIST += apq8084
LOC_BOARD_PLATFORM_LIST += msm8916
LOC_BOARD_PLATFORM_LIST += msm8994
LOC_BOARD_PLATFORM_LIST += msm8909
LOC_BOARD_PLATFORM_LIST += msm8952
LOC_BOARD_PLATFORM_LIST += msm8992
LOC_BOARD_PLATFORM_LIST += msm8996
LOC_BOARD_PLATFORM_LIST += msm8937
LOC_BOARD_PLATFORM_LIST += msm8953
LOC_BOARD_PLATFORM_LIST += msm8998
LOC_BOARD_PLATFORM_LIST += apq8098_latv
LOC_BOARD_PLATFORM_LIST += sdm660
LOC_BOARD_PLATFORM_LIST += sdm845
LOC_BOARD_PLATFORM_LIST += msmpeafowl
LOC_BOARD_PLATFORM_LIST += sdm710
LOC_BOARD_PLATFORM_LIST += qcs605
LOC_BOARD_PLATFORM_LIST += msmnile
LOC_BOARD_PLATFORM_LIST += sdmshrike
LOC_BOARD_PLATFORM_LIST += $(MSMSTEPPE)
LOC_BOARD_PLATFORM_LIST += $(TRINKET)
LOC_BOARD_PLATFORM_LIST += kona
LOC_BOARD_PLATFORM_LIST += lito
LOC_BOARD_PLATFORM_LIST += bengal
LOC_BOARD_PLATFORM_LIST += atoll

GNSS_CFLAGS := \
    -Werror \
    -Wno-error=unused-parameter \
    -Wno-error=macro-redefined \
    -Wno-error=reorder \
    -Wno-error=missing-braces \
    -Wno-error=self-assign \
    -Wno-error=enum-conversion \
    -Wno-error=logical-op-parentheses \
    -Wno-error=null-arithmetic \
    -Wno-error=null-conversion \
    -Wno-error=parentheses-equality \
    -Wno-error=undefined-bool-conversion \
    -Wno-error=tautological-compare \
    -Wno-error=switch \
    -Wno-error=date-time

ifeq ($(TARGET_DEVICE),apq8026_lw)
LW_FEATURE_SET := true
endif

ifeq ($(TARGET_SUPPORTS_ANDROID_WEAR),true)
LW_FEATURE_SET := true
endif

LOCAL_PATH := $(call my-dir)

ifneq ($(LW_FEATURE_SET),true)
ifneq (,$(filter $(LOC_BOARD_PLATFORM_LIST),$(TARGET_BOARD_PLATFORM)))
include $(LOCAL_PATH)/*/Android.mk
endif # is-board-platform-in-list
endif # LW_FEATURE_SET

endif # BOARD_VENDOR_QCOM_LOC_PDK_FEATURE_SET
endif # BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE
endif # BUILD_TINY_ANDROID

