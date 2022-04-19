

PRODUCT_PROPERTY_OVERRIDES += \
    persist.vendor.radio.apm_sim_not_pwdn=1 \
    persist.vendor.radio.sib16_support=1 \
    persist.vendor.radio.custom_ecc=1 \
    rild.libpath=/vendor/lib64/libril-qc-hal-qmi.so \
    persist.vendor.radio.procedure_bytes=SKIP

ifeq ($(TARGET_FWK_SUPPORTS_FULL_VALUEADDS),true)
    PRODUCT_PROPERTY_OVERRIDES += persist.radio.multisim.config=dsds
    PRODUCT_PROPERTY_OVERRIDES += persist.vendor.radio.rat_on=combine
else
    PRODUCT_PROPERTY_OVERRIDES += persist.vendor.radio.rat_on=""
endif

PRODUCT_COPY_FILES += \
frameworks/native/data/etc/android.hardware.telephony.gsm.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.gsm.xml \
frameworks/native/data/etc/android.hardware.telephony.cdma.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.cdma.xml \
frameworks/native/data/etc/android.hardware.telephony.ims.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.ims.xml


ifneq ($(TARGET_SUPPORTS_WEARABLES),true)
PRODUCT_PACKAGES += ATFWD-daemon
endif

# QCRIL
QCRIL := qcril.db
QCRIL += libril-qc-qmi-1
QCRIL += libril-qc-hal-qmi
QCRIL += libril-qcril-hook-oem
QCRIL += qcrild_libril
QCRIL += qcrild_librilutils
QCRIL += libwmsts
QCRIL += libril-qc-radioconfig
QCRIL += libril-qc-ltedirectdisc
QCRIL += librilqmiservices
QCRIL += libsettings
QCRIL += liblqe
QCRIL += lib_remote_simlock
QCRIL += qcrild
QCRIL += qcrild.rc
QCRIL += qtimutex
QCRIL += libqcrilFramework
QCRIL += qcril_database
QCRIL += deviceInfoServiceModule
QCRIL += libqmiextservices

PRODUCT_PACKAGES += $(QCRIL)
PRODUCT_PACKAGES += $(TELEPHONY_APPS)

ENABLE_VENDOR_RIL_SERVICE := true

