PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.extension_library=libqti-perfd-client.so

ifeq ($(TARGET_USES_QMAA),true)
  ifneq ($(TARGET_USES_QMAA_OVERRIDE_PERF),true)
    TARGET_DISABLE_PERF_OPTIMIZATIONS := true
  endif # TARGET_USES_QMAA_OVERRIDE_PERF
endif # TARGET_USES_QMAA

ifneq ($(strip $(TARGET_DISABLE_PERF_OPTIMIZATIONS)),true)
ifneq ($(TARGET_BOARD_AUTO),true)
# Preloading QPerformance jar to ensure faster perflocks in Boost Framework
PRODUCT_BOOT_JARS += QPerformance
# Preloading UxPerformance jar to ensure faster UX invoke in Boost Framework
PRODUCT_BOOT_JARS += UxPerformance
endif
endif

ifneq ($(TARGET_BOARD_AUTO),true)
PRODUCT_PACKAGES += \
    UxPerformance \
    QPerformance \
    libqti_performance \
    libqti-at \
    perfservice \
    vendor.qti.hardware.perf@2.0.vendor
else
PRODUCT_PACKAGES += \
    perfservice \
    vendor.qti.hardware.perf@2.0.vendor
endif
