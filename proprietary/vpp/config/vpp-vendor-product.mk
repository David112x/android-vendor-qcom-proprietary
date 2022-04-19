# APKs
PRODUCT_PACKAGES += SSMEditor

# Native
PRODUCT_PACKAGES += libOmxVpp
PRODUCT_PACKAGES += libvppclient
PRODUCT_PACKAGES += libvpplibrary
PRODUCT_PACKAGES += vppservice

# HIDL
PRODUCT_PACKAGES += vendor.qti.hardware.vpp@1.1
PRODUCT_PACKAGES += vendor.qti.hardware.vpp@1.1.vendor
PRODUCT_PACKAGES += vendor.qti.hardware.vpp@1.2
PRODUCT_PACKAGES += vendor.qti.hardware.vpp@1.2.vendor

# Configurations
PRODUCT_PACKAGES += vpp.configstore.xml

# HY11_HY22_diff
PRODUCT_PACKAGES += vendor.qti.hardware.vpp@1.1_vendor
PRODUCT_PACKAGES += vendor.qti.hardware.vpp@1.2_vendor

# Debug
PRODUCT_PACKAGES_DEBUG += libvpptestutils
PRODUCT_PACKAGES_DEBUG += vppipcunittest
PRODUCT_PACKAGES_DEBUG += vpplibraryfunctionaltest
PRODUCT_PACKAGES_DEBUG += vpplibraryunittest
