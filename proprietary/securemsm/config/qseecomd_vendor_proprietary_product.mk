#QSEECOM module specific libraries and binaries

PRODUCT_PACKAGES += gpfspath_oem_config.xml
PRODUCT_PACKAGES += gpfspath_le_oem_config.xml
PRODUCT_PACKAGES += libdrmfs
PRODUCT_PACKAGES += libdrmMinimalfs
PRODUCT_PACKAGES += libdrmMinimalfsHelper
PRODUCT_PACKAGES += libdrmtime
PRODUCT_PACKAGES += libGPreqcancel
PRODUCT_PACKAGES += libGPreqcancel_svc
PRODUCT_PACKAGES += libQSEEComAPI
PRODUCT_PACKAGES += libQSEEComAPIStatic
PRODUCT_PACKAGES += librpmb
PRODUCT_PACKAGES += librpmbStatic
PRODUCT_PACKAGES += librpmbStaticHelper
PRODUCT_PACKAGES += libssd
PRODUCT_PACKAGES += libssdStatic
PRODUCT_PACKAGES += libssdStaticHelper
PRODUCT_PACKAGES += qseecom_sample_client
PRODUCT_PACKAGES += qseecomd
PRODUCT_PACKAGES += qseecomd.rc

ifeq ($(call is-board-platform-in-list, lito kona), true)
PRODUCT_PACKAGES_DEBUG += acvp_test
PRODUCT_HOST_PACKAGES += ACVP.py
endif
