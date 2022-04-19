THERMAL_ENGINE := thermal-engine
THERMAL_ENGINE += libthermalclient
THERMAL_ENGINE += libthermalioctl
THERMAL_ENGINE += init_thermal-engine.rc
THERMAL_ENGINE += thermal-engine.conf

PRODUCT_PACKAGES += $(THERMAL_ENGINE)
