#ANT
ifeq ($(TARGET_FWK_SUPPORTS_FULL_VALUEADDS), true)
PRODUCT_PACKAGES += AntHalService
PRODUCT_PACKAGES += libantradio
PRODUCT_PACKAGES += antradio_app
PRODUCT_PACKAGES += com.qualcomm.qti.ant@1.0
endif #TARGET_FWK_SUPPORTS_FULL_VALUEADDS

#BT
ifeq ($(BOARD_HAVE_BLUETOOTH_QCOM),true)
PRODUCT_PACKAGES += Bluetooth

ifeq ($(TARGET_FWK_SUPPORTS_FULL_VALUEADDS), true)
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := vendor/qcom/opensource/commonsys-intf/bluetooth/build/qva/config
PRODUCT_PACKAGES += libbluetooth_qti
PRODUCT_PACKAGES += bt_logger
PRODUCT_PACKAGES += libbt-logClient
PRODUCT_PACKAGES += BluetoothExt
PRODUCT_PACKAGES += libbtconfigstore
PRODUCT_PACKAGES += vendor.qti.hardware.btconfigstore@1.0
PRODUCT_PACKAGES += com.qualcomm.qti.bluetooth_audio@1.0
PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_audio@2.0
PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_dun-V1.0-java
# BT Related Test app & Tools
PRODUCT_PACKAGES_DEBUG += BATestApp
PRODUCT_PACKAGES_DEBUG += BTTestApp
PRODUCT_PACKAGES_DEBUG += btsnoop
PRODUCT_PACKAGES_DEBUG += gatt_tool_qti_internal
PRODUCT_PACKAGES_DEBUG += l2test_ertm
PRODUCT_PACKAGES_DEBUG += rfc
else
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := vendor/qcom/opensource/commonsys-intf/bluetooth/build/generic/config
endif #TARGET_FWK_SUPPORTS_FULL_VALUEADDS

endif #BOARD_HAVE_BLUETOOTH_QCOM

#FM
ifeq ($(BOARD_HAVE_QCOM_FM), true)
ifeq ($(TARGET_FWK_SUPPORTS_FULL_VALUEADDS), true)
PRODUCT_PACKAGES += libqcomfm_jni
PRODUCT_PACKAGES += libfmjni
PRODUCT_PACKAGES += fm_helium
PRODUCT_PACKAGES += libfm-hci
PRODUCT_PACKAGES += FM2
PRODUCT_PACKAGES += qcom.fmradio
PRODUCT_BOOT_JARS += qcom.fmradio
PRODUCT_PACKAGES += vendor.qti.hardware.fm@1.0
# system prop for fm
PRODUCT_PROPERTY_OVERRIDES += vendor.hw.fm.init=0
endif #TARGET_FWK_SUPPORTS_FULL_VALUEADDS
endif #BOARD_HAVE_QCOM_FM

#WIPOWER
ifeq ($(BOARD_USES_WIPOWER),true)
ifeq ($(TARGET_FWK_SUPPORTS_FULL_VALUEADDS), true)
#WIPOWER, wbc
PRODUCT_PACKAGES += wbc_hal.default
PRODUCT_PACKAGES += com.quicinc.wbc
PRODUCT_PACKAGES += com.quicinc.wbc.xml
PRODUCT_PACKAGES += com.quicinc.wbcservice
PRODUCT_PACKAGES += com.quicinc.wbcservice.xml
PRODUCT_PACKAGES += libwbc_jni
PRODUCT_PACKAGES += com.quicinc.wipoweragent
PRODUCT_PACKAGES += com.quicinc.wbcserviceapp
endif #TARGET_FWK_SUPPORTS_FULL_VALUEADDS
endif #BOARD_USES_WIPOWER
