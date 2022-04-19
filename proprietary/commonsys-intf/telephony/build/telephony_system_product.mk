

#Disable telephony-ext temporarly here, its already added in qssi,
# once all changes merged enable it again
#PRODUCT_BOOT_JARS += telephony-ext

PRODUCT_PACKAGES += \
    CellBroadcastReceiver \
    Mms \
    QtiDialer \
    Stk \
    telephony-ext \
    imssettings \
    ims-ext-common \
    ims_ext_common.xml \
    ConfURIDialer

XML_CONF_PATH := $(QCPATH_COMMONSYS)/telephony-apps/etc

  #Add HY22 support
  HY22_XML_CONF_PATH := $(QC_PROP_ROOT)/prebuilt_HY22/target/product/$(PREBUILT_BOARD_PLATFORM_DIR)/system/etc
  ifneq ($(wildcard $(HY22_XML_CONF_PATH)),)
    PRODUCT_COPY_FILES += $(HY22_XML_CONF_PATH)/apns-conf.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/apns-conf.xml
    PRODUCT_COPY_FILES += $(HY22_XML_CONF_PATH)/cdma_call_conf.xml:system/etc/cdma_call_conf.xml
    PRODUCT_COPY_FILES += $(HY22_XML_CONF_PATH)/spn-conf.xml:system/etc/spn-conf.xml
    ifneq ($(wildcard $(HY22_XML_CONF_PATH)/permissions/qti_libpermissions.xml),)
      PRODUCT_COPY_FILES += $(HY22_XML_CONF_PATH)/permissions/qti_libpermissions.xml:system/etc/permissions/qti_libpermissions.xml
    endif
  endif

  ifneq ($(wildcard $(XML_CONF_PATH)),)
    PRODUCT_COPY_FILES += $(XML_CONF_PATH)/apns-conf.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/apns-conf.xml
    PRODUCT_COPY_FILES += $(XML_CONF_PATH)/cdma_call_conf.xml:system/etc/cdma_call_conf.xml
    PRODUCT_COPY_FILES += $(XML_CONF_PATH)/spn-conf.xml:system/etc/spn-conf.xml
    PRODUCT_COPY_FILES += $(XML_CONF_PATH)/qti_libpermissions.xml:system/etc/permissions/qti_libpermissions.xml
  endif

HY11_HY22_diff += LteDirectDiscovery.xml
HY11_HY22_diff += qti_libpermissions.xml
HY11_HY22_diff += cdma_call_conf.xml
HY11_HY22_diff += spn-conf.xml

HY11_HY22_diff_debug += radioconfig
HY11_HY22_diff_debug += radioconfig.xml
HY11_HY22_diff_debug += radioconfiginterface.xml

PRODUCT_PACKAGES += $(HY11_HY22_diff)
PRODUCT_PACKAGES_DEBUG += $(HY11_HY22_diff_debug)



# IMS Telephony Libs
IMS_TEL := ims.xml
IMS_TEL += imslibrary
IMS_TEL += ims

QCRIL += android.hardware.radio@1.1
QCRIL += android.hardware.radio@1.2
QCRIL += android.hardware.radio@1.0
QCRIL += android.hardware.radio.config@1.0
QCRIL += android.hardware.radio.deprecated@1.0
QCRIL += android.hardware.secure_element@1.0


TELEPHONY_APPS += NetworkSetting
TELEPHONY_APPS += xdivert
TELEPHONY_APPS += CallFeaturesSetting
TELEPHONY_APPS += datastatusnotification
TELEPHONY_APPS += QtiTelephonyService
TELEPHONY_APPS += telephonyservice.xml
TELEPHONY_APPS += SimContacts
TELEPHONY_APPS += PrimaryCardController
TELEPHONY_APPS += SimSettings
TELEPHONY_APPS += ConferenceDialer
TELEPHONY_APPS += uimlpalibrary
TELEPHONY_APPS += lpa.xml
TELEPHONY_APPS += remoteSimLockAuthentication
TELEPHONY_APPS += remotesimlockmanagerlibrary
TELEPHONY_APPS += RemoteSimlockManager.xml
TELEPHONY_APPS += uimremotesimlocklibrary
TELEPHONY_APPS += RemoteSimlock.xml
TELEPHONY_APPS += uimremoteclientlibrary
TELEPHONY_APPS += uimremoteclient
TELEPHONY_APPS += uimremoteclient.xml
TELEPHONY_APPS += uimremoteserverlibrary
TELEPHONY_APPS += uimremoteserver
TELEPHONY_APPS += uimremoteserver.xml
TELEPHONY_APPS += saminterfacelibrary
TELEPHONY_APPS += sammanagerlibrary
TELEPHONY_APPS += sam
TELEPHONY_APPS += sammanager.xml
TELEPHONY_APPS += saminterface.xml
TELEPHONY_APPS += libimscamera_jni
TELEPHONY_APPS += libimsmedia_jni
TELEPHONY_APPS += CallEnhancement
TELEPHONY_APPS += DeviceInfo
TELEPHONY_APPS += QtiSystemService
TELEPHONY_APPS += AutoRegistration
TELEPHONY_APPS += atfwd
TELEPHONY_APPS += uimgbalibrary
TELEPHONY_APPS += UimGba.xml
TELEPHONY_APPS += uimgbamanagerlibrary
TELEPHONY_APPS += UimGbaManager.xml
TELEPHONY_APPS += qcrilmsgtunnel
TELEPHONY_APPS += qcrilhook
TELEPHONY_APPS += qcrilhook.xml
TELEPHONY_APPS += DeviceStatisticsService
#Qc extended functionality of android telephony
ifneq ($(TARGET_SUPPORTS_ANDROID_WEAR),true)
QTI_TELEPHONY_FWK := qti-telephony-common
endif
QTI_TELEPHONY_FWK += QtiTelephonyServicelibrary

#Qc extended telephony framework resource APK
QTI_TELEPHONY_RES := telresources

HIDL_WRAPPER := qti-telephony-hidl-wrapper
HIDL_WRAPPER += qti_telephony_hidl_wrapper.xml

QTI_TELEPHONY_UTILS := qti-telephony-utils
QTI_TELEPHONY_UTILS += qti_telephony_utils.xml

PRODUCT_PACKAGES += $(IMS_TEL)
PRODUCT_PACKAGES += $(QCRIL)
PRODUCT_PACKAGES += $(TELEPHONY_APPS)
PRODUCT_PACKAGES += $(QTI_TELEPHONY_FWK)
PRODUCT_PACKAGES += $(QTI_TELEPHONY_RES)
PRODUCT_PACKAGES += $(HIDL_WRAPPER)
PRODUCT_PACKAGES += $(QTI_TELEPHONY_UTILS)

#TELEPHONY_APPS_DBG
TELEPHONY_APPS_DBG := Presence
TELEPHONY_APPS_DBG += NetworkSetting
TELEPHONY_APPS_DBG += RoamingSettings
TELEPHONY_APPS_DBG += qcom_qmi.xml
TELEPHONY_APPS_DBG += ModemTestMode
TELEPHONY_APPS_DBG += NrNetworkSettingApp
TELEPHONY_APPS_DBG += EuiccResource

PRODUCT_PACKAGES_DEBUG += $(TELEPHONY_APPS_DBG)

#copy telephony app's permissions
PRODUCT_COPY_FILES += $(QCPATH)/commonsys-intf/telephony/build/telephony_product_privapp-permissions-qti.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/permissions/telephony_product_privapp-permissions-qti.xml
