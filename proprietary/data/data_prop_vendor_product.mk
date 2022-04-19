#DATA_PROP
ifneq ($(TARGET_SUPPORTS_WEARABLES),true)
DATA_PROP := ATFWD-daemon
endif
DATA_PROP += dsi_config.xml
DATA_PROP += libconfigdb
DATA_PROP += libdsutils
DATA_PROP += libnetmgr
DATA_PROP += libnetmgr_common
DATA_PROP += libdsi_netctrl
DATA_PROP += libqdi
DATA_PROP += libqdp
DATA_PROP += netmgr_config.xml
DATA_PROP += netmgrd
DATA_PROP += netmgrd.rc
DATA_PROP += port-bridge
DATA_PROP += port-bridge.rc
#IPACM-diag
DATA_PROP += ipacm-diag
DATA_PROP += ipacm-diag.rc
#IPA_FWS
IPA_FWS := ipa_fws
IPA_FWS += ipa_fws.elf
IPA_FWS += ipa_fws.rc
IPA_FWS += ipa_uc.elf
IPA_FWS += ipa_uc

#DATA_PROP_HY11_HY22_diff
DATA_PROP_HY11_HY22_diff := libqdi
DATA_PROP_HY11_HY22_diff += libqdp

PRODUCT_PACKAGES += $(IPA_FWS)
PRODUCT_PACKAGES += $(DATA_PROP)
PRODUCT_PACKAGES += $(DATA_PROP_HY11_HY22_diff)

# Ipsec_tunnels feature
ifneq ($(TARGET_KERNEL_VERSION), 3.18)
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.software.ipsec_tunnels.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.ipsec_tunnels.xml
endif
