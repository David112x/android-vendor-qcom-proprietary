# QC_PROP_ROOT is already set when we reach here:
# PATH for reference: QC_PROP_ROOT := vendor/qcom/proprietary
$(call inherit-product-if-exists, $(QC_PROP_ROOT)/android-perf/profiles.mk)

PRODUCT_PACKAGES += \
    libqti-iopd \
    vendor.qti.hardware.iop@2.0 \
    vendor.qti.hardware.iop@2.0-service.rc \
    init.pixel.vendor.qti.hardware.iop@2.0-service.rc \
    vendor.qti.hardware.iop@2.0-impl \
    vendor.qti.hardware.iop@2.0-service \
    libqti-perfd \
    libqti-util \
    vendor.qti.hardware.perf@2.0 \
    vendor.qti.hardware.perf@2.0-service.rc \
    init.pixel.vendor.qti.hardware.perf@2.0-service.rc \
    vendor.qti.hardware.perf@2.0-impl \
    vendor.qti.hardware.perf@2.0-service \
    android.hardware.tests.libhwbinder@1.0-impl \
    libperfgluelayer \
    liblearningmodule \
    libmeters \
    libreffeature \
    ReferenceFeature.xml \
    AdaptLaunchFeature.xml \
    AppClassifierFeature.xml \
    GameOptimizationFeature.xml

PRODUCT_PACKAGES_DEBUG += \
    perflock_native_test \
    perflock_native_stress_test \
    perfunittests \
    boostConfigParser \
    libqti-tests-mod1 \
    libqti-tests-mod2 \
    libqti-tests-mod3 \
    libqti-tests-mod4 \
    libqti-tests-mod5 \
    libqti-tests-mod6
