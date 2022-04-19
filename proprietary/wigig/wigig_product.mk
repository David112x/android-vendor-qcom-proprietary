#packages from device-vendor-qssi.mk
PRODUCT_PACKAGES += ftmtest
PRODUCT_PACKAGES += wigig_supplicant.conf
PRODUCT_PACKAGES += wigig_p2p_supplicant.conf
PRODUCT_PACKAGES += wigighalsvc
PRODUCT_PACKAGES += vendor.qti.hardware.wigig.supptunnel@1.0
PRODUCT_PACKAGES += vendor.qti.hardware.wigig.supptunnel-V1.0-java
PRODUCT_PACKAGES += wigignpt
PRODUCT_PACKAGES += vendor.qti.hardware.wigig.netperftuner@1.0
PRODUCT_PACKAGES += vendor.qti.hardware.wigig.netperftuner-V1.0-java
#packages from base.mk
PRODUCT_PACKAGES += fstman
PRODUCT_PACKAGES += fstman.ini
PRODUCT_PACKAGES += host_manager_11ad
PRODUCT_PACKAGES += shell_11ad
PRODUCT_PACKAGES += wigig_remoteserver
PRODUCT_PACKAGES += wigig_wiburn
PRODUCT_PACKAGES += wigig_logcollector
PRODUCT_PACKAGES += wigig_logcollector.ini
PRODUCT_PACKAGES += libwigig_utils
PRODUCT_PACKAGES += libwigig_flashaccess
PRODUCT_PACKAGES += libwigig_pciaccess
PRODUCT_PACKAGES += libwigigsensing
PRODUCT_PACKAGES += sensingdaemon
PRODUCT_PACKAGES += libwigig_ftm_flows
PRODUCT_PACKAGES += ftm_flows_test

#Capability Config Store
PRODUCT_COPY_FILES += \
    vendor/qcom/proprietary/wigig/configs/common/wigig.xml:$(TARGET_COPY_OUT_VENDOR)/etc/configstore/wigig.xml

# create symbolic links for INI file
$(shell mkdir -p $(TARGET_OUT_VENDOR)/firmware/wigig/)
PRODUCT_COPY_FILES += \
    vendor/qcom/proprietary/wigig/configs/common/wigig.ini:$(TARGET_COPY_OUT_VENDOR)/firmware/wigig/wigig.ini

# target specific wigig configuration
ifeq ($(TARGET_BOARD_PLATFORM),kona)
PRODUCT_COPY_FILES += \
    vendor/qcom/proprietary/wigig/configs/kona/init.target.wigig.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.target.wigig.rc
endif
