TARGET_DISABLE_ADPL := false

ifeq ($(TARGET_USES_QMAA),true)
ifneq ($(TARGET_USES_QMAA_OVERRIDE_DATA),true)
	TARGET_DISABLE_ADPL := true
endif #TARGET_USES_QMAA_OVERRIDE_DATA
endif #TARGET_USES_QMAA

ifneq ($(TARGET_DISABLE_ADPL),true)
BOARD_PLATFORM_LIST := msm8916
BOARD_PLATFORM_LIST += msm8909
BOARD_PLATFORM_LIST += msm8917
ifneq ($(call is-board-platform-in-list,$(BOARD_PLATFORM_LIST)),true)
include $(call all-subdir-makefiles)
endif # QCOM_TARGET_PRODUCT
endif #TARGET_DISABLE_ADPL
