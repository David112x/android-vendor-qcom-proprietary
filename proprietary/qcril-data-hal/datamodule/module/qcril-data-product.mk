PRODUCT_PACKAGES += init-qcril-data.rc

ifeq ($(TARGET_FWK_SUPPORTS_FULL_VALUEADDS),true)
  ifeq ($(call is-board-platform-in-list,msm8998 msm8996 sdm660 sdm845 sm6150 sdm710 trinket msm8953 msm8937 msm8909 atoll bengal),true)
    PRODUCT_PROPERTY_OVERRIDES += ro.telephony.iwlan_operation_mode=legacy
  endif
else
  ifeq ($(call is-board-platform-in-list,sm6150), true)
    PRODUCT_PROPERTY_OVERRIDES += ro.telephony.iwlan_operation_mode=legacy
  endif
endif
