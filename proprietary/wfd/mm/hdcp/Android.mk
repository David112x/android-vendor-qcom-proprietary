
WFD_HDCP22_DISABLED := true

ifeq ($(call is-vendor-board-platform,QCOM),true)
ifneq ($(call is-board-platform,copper),true)
ifneq ($(WFD_HDCP22_DISABLED),true)

include $(call all-subdir-makefiles)

endif
endif
endif
