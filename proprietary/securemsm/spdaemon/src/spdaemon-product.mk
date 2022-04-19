ifeq ($(call is-vendor-board-platform,QCOM),true)
ifeq ($(call is-board-platform-in-list, msmnile sdmshrike kona),true)

SECUREMSM += init.spdaemon.rc

PRODUCT_PACKAGES += $(SECUREMSM)

endif
endif
