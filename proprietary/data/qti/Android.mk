TARGET_DISABLE_QTI := false

ifeq ($(TARGET_USES_QMAA),true)
ifneq ($(TARGET_USES_QMAA_OVERRIDE_DATA),true)
	TARGET_DISABLE_QTI := true
endif #TARGET_USES_QMAA_OVERRIDE_DATA
endif #TARGET_USES_QMAA

ifneq ($(TARGET_DISABLE_QTI),true)
BOARD_PLATFORM_LIST := msm8916
ifneq ($(call is-board-platform-in-list,$(BOARD_PLATFORM_LIST)),true)
include $(call all-subdir-makefiles)
endif # QCOM_TARGET_PRODUCT
endif # TARGET_DISABLE_QTI
