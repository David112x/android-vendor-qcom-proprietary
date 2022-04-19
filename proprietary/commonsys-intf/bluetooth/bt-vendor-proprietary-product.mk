#ANT
PRODUCT_PACKAGES += com.qualcomm.qti.ant@1.0.vendor
PRODUCT_PACKAGES += com.qualcomm.qti.ant@1.0-impl


#BT
ifeq ($(BOARD_HAVE_QTI_BT_LAZY_SERVICE),true)
PRODUCT_PACKAGES += android.hardware.bluetooth@1.0-service-qti-lazy
PRODUCT_PACKAGES += android.hardware.bluetooth@1.0-service-qti-lazy.rc
#Enable Lazy hal
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.lazyhal=true
else
PRODUCT_PACKAGES += android.hardware.bluetooth@1.0-service-qti
PRODUCT_PACKAGES += android.hardware.bluetooth@1.0-service-qti.rc
endif

PRODUCT_PACKAGES += android.hardware.bluetooth@1.0-impl-qti
PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_audio@2.0-impl
PRODUCT_PACKAGES += android.hardware.bluetooth.a2dp@2.0-impl
PRODUCT_PACKAGES += com.qualcomm.qti.bluetooth_audio@1.0.vendor
PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_audio@2.0.vendor
PRODUCT_PACKAGES += btaudio_offload_if
PRODUCT_PACKAGES += audio.bluetooth.default
PRODUCT_PACKAGES += audio.bluetooth_qti.default
PRODUCT_PACKAGES += libbluetooth_audio_session_qti
PRODUCT_PACKAGES += libbluetooth_audio_session
PRODUCT_PACKAGES += libbt-hidlclient
PRODUCT_PACKAGES += vendor.qti.hardware.btconfigstore@1.0.vendor
PRODUCT_PACKAGES += vendor.qti.hardware.btconfigstore@1.0-impl
PRODUCT_PACKAGES += libbtnv
PRODUCT_PACKAGES_DEBUG += btconfig

#BTSAR
PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_sar@1.0.vendor
PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_sar@1.0-impl

#BTDUN
PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_dun@1.0.vendor
PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_dun@1.0-impl
PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_dun@1.0-service
PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_dun@1.0-service.rc
PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_dun@1.0-service.disable.rc

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.bluetooth.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth_le.xml


#FM
PRODUCT_PACKAGES += fmconfig
PRODUCT_PACKAGES += fmfactorytest
PRODUCT_PACKAGES += fmfactorytestserver
PRODUCT_PACKAGES += fm_qsoc_patches
PRODUCT_PACKAGES += ftm_fm_lib
PRODUCT_PACKAGES += vendor.qti.hardware.fm@1.0.vendor
PRODUCT_PACKAGES += vendor.qti.hardware.fm@1.0-impl
PRODUCT_PACKAGES_DEBUG += hal_ss_test_manual
PRODUCT_PACKAGES += init.qti.fm.sh


#WIPOWER
ifeq ($(BOARD_USES_WIPOWER),true)
PRODUCT_PACKAGES += vendor.qti.hardware.wipower@1.0_vendor
PRODUCT_PACKAGES += vendor.qti.hardware.wipower@1.0-impl
endif #BOARD_USES_WIPOWER


#ANT/BT/FM/WIPOWER PROPERTIES

ifeq ($(TARGET_BOARD_PLATFORM),msmnile) # msmnile specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += vendor.qcom.bluetooth.soc=cherokee
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac-aptxadaptive
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
#aac frame control support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_frm_ctl.enabled=true
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
endif

ifeq ($(TARGET_BOARD_PLATFORM),sdm845) # SDM845 specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += vendor.qcom.bluetooth.soc=cherokee
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
#aac frame control support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_frm_ctl.enabled=true
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
endif

ifeq ($(TARGET_BOARD_PLATFORM),sm6150)  # Talos/sm6150 specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += vendor.qcom.bluetooth.soc=cherokee
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
#aac frame control support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_frm_ctl.enabled=true
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
endif

ifeq ($(TARGET_BOARD_PLATFORM),sdm710)  # sdm710/Warlock specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += vendor.qcom.bluetooth.soc=cherokee
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
#aac frame control support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_frm_ctl.enabled=true
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
endif

ifeq ($(TARGET_BOARD_PLATFORM),qcs605)  # qcs605 specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += vendor.qcom.bluetooth.soc=cherokee
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
#aac frame control support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_frm_ctl.enabled=true
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
endif

ifeq ($(TARGET_BOARD_PLATFORM),kona)  # kona/sm8250 specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += vendor.qcom.bluetooth.soc=hastings
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac-aptxadaptiver2
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
#aac frame control support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_frm_ctl.enabled=true
#A2dp Multicast support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_mcast_test.enabled=false
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
endif

ifeq ($(TARGET_BOARD_PLATFORM), trinket) # trinket specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += vendor.qcom.bluetooth.soc=cherokee
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
endif

ifeq ($(TARGET_BOARD_PLATFORM), lito) # lito specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += vendor.qcom.bluetooth.soc=cherokee
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac-aptxadaptiver2
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
endif

ifeq ($(TARGET_BOARD_PLATFORM), bengal) # bengal specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += vendor.qcom.bluetooth.soc=cherokee
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
endif

ifeq ($(TARGET_BOARD_PLATFORM),atoll) # atoll specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += vendor.qcom.bluetooth.soc=cherokee
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac-aptxadaptive
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
#aac frame control support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_frm_ctl.enabled=true
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
endif

ifeq ($(TARGET_BOARD_PLATFORM),sdm660)  # sdm660
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += vendor.qcom.bluetooth.soc=cherokee
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxhd-aac-ldac
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
endif

ifeq ($(TARGET_BOARD_PLATFORM),msm8998) # MSM8998 specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += vendor.qcom.bluetooth.soc=cherokee
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxhd-aac-ldac
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
endif

ifeq ($(TARGET_BOARD_PLATFORM),msm8996) # MSM8996 specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES +=vendor.qcom.bluetooth.soc=rome
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=false
# Set this true as ROME which is programmed
# as embedded wipower mode by default
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
endif

ifeq ($(TARGET_BOARD_PLATFORM),msm8937) # msm8937 specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += vendor.qcom.bluetooth.soc=pronto
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=false
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
endif

ifeq ($(TARGET_BOARD_PLATFORM),msm8953) # MSM8953 specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += vendor.qcom.bluetooth.soc=pronto
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=false
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
endif

ifeq ($(TARGET_BOARD_PLATFORM),msm8909) # MSM8909 specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += vendor.qcom.bluetooth.soc=pronto
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=false
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
endif

ifeq ($(TARGET_BOARD_PLATFORM),msm8952) # MSM8952 specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += vendor.qcom.bluetooth.soc=pronto
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=false
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
endif
