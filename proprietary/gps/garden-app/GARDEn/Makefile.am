AM_CFLAGS = \
    -DDEBUG \
    -I./gnss_case \
    -I./geofence_case \
    -I./flp_case \
    -I./locationapi_api \
    -I./locationclient_api \
    -I./ulp_case \
    -I./clientapi_case \
    -I./gnss_multiclient_case \
    -I./garden_framework \
    $(EVENTOBSERVER_CFLAGS) \
    $(GPSUTILS_CFLAGS) \
    $(LOCCORE_CFLAGS) \
    $(LOCATIONBATCHING_CFLAGS) \
    $(LOCATIONSERVICE_CFLAGS) \
    $(QMIFW_CFLAGS)\
    $(QMI_CFLAGS)\
    $(LOCGLUE_CFLAGS) \
    $(LOCATIONCORE_CFLAGS) \
    $(LOCCLIENTAPI_CFLAGS) \
    $(LOCHALDAEMON_CFLAGS) \
    $(LOCDIAGIFACE_CFLAGS) \
    $(DIAG_CFLAGS) \
    $(NTRIPCLIENT_CFLAGS) \
    -std=c++11

AM_CPPFLAGS = $(AM_CFLAGS)

required_libs = \
    $(GPSUTILS_LIBS) \
    $(LOCCORE_LIBS) \
    $(LOCHAL_LIBS) \
    $(LOCATIONAPI_LIBS) \
    $(LOCATIONSERVICE_LIBS) \
    $(QMIFW_LIBS) \
    $(QMI_LIBS) \
    $(LOCGLUE_LIBS) \
    $(LOCCLIENTAPI_LIBS) \
    $(GPSUTILS_LIBS) \
    $(NTRIPCLIENT_LIBS)

libgarden_h_sources = \
    garden_framework/IGardenCase.h \
    garden_framework/GardenFramework.h \
    garden_framework/IGnssAPI.h \
    garden_framework/GnssCbBase.h \
    garden_framework/GardenUtil.h \
    gnss_multiclient_case/GnssMultiClientCaseUtils.h \
    locationapi_api/GnssLocationAPI.h \
    locationapi_api/GnssLocationAPIClient.h \
    locationapi_api/GeofenceLocationAPIClient.h \
    locationapi_api/GnssMultiClientLocationAPIClient.h \
    locationapi_api/FlpLocationAPIClient.h \
    locationclient_api/GnssLocationClientAPI.h

libgarden_cases_h_sources = \
    gnss_case/GnssCase.h \
    gnss_case/GnssCb.h \
    geofence_case/GeofenceCb.h \
    geofence_case/GeofenceCase.h \
    flp_case/FlpCb.h \
    flp_case/FlpCase.h \
    clientapi_case/LocationClientApiCase.h \
    gnss_multiclient_case/GnssMultiClientCase.h \
    gnss_multiclient_case/GnssMultiClientCaseUtils.h \
    gnss_multiclient_case/GnssMultiClientCb.h \
    gnss_multiclient_case/GnssMultiClientFileTest.h

libgarden_c_sources = \
    garden_framework/GardenFramework.cpp \
    garden_framework/GardenUtil.cpp \
    garden_framework/GnssCbBase.cpp \
    locationapi_api/GnssLocationAPI.cpp \
    locationapi_api/GnssLocationAPIClient.cpp \
    locationapi_api/FlpLocationAPIClient.cpp \
    locationapi_api/GnssMultiClientLocationAPIClient.cpp \
    locationapi_api/GeofenceLocationAPIClient.cpp \
    locationclient_api/GnssLocationClientAPI.cpp

libgarden_cases_c_sources = \
    gnss_case/GnssCase.cpp \
    gnss_case/GnssCb.cpp \
    geofence_case/GeofenceCb.cpp \
    geofence_case/GeofenceCase.cpp \
    flp_case/FlpCase.cpp \
    flp_case/FlpCb.cpp \
    clientapi_case/LocationClientApiCase.cpp \
    gnss_multiclient_case/GnssMultiClientCase.cpp \
    gnss_multiclient_case/GnssMultiClientCaseUtils.cpp \
    gnss_multiclient_case/GnssMultiClientCb.cpp \
    gnss_multiclient_case/GnssMultiClientFileTest.cpp

if USE_FEATURE_AUTOMOTIVE
    AM_CFLAGS += -I./cdfwapi_case \
                 $(CDFW_CFLAGS) \
                 $(CDFWREMOTEAPI_CFLAGS) \
                 -DFEATURE_AUTOMOTIVE
    AM_CPPFLAGS += -DFEATURE_AUTOMOTIVE
    required_libs += $(CDFW_LIBS)
    required_libs += $(CDFWREMOTEAPI_LIBS)
    libgarden_h_sources += cdfwapi_case/cdfwApiCase.h
    libgarden_cases_h_sources += cdfwapi_case/cdfwApiCase.h
    libgarden_c_sources += cdfwapi_case/cdfwApiCase.cpp
    libgarden_cases_c_sources += cdfwapi_case/cdfwApiCase.cpp
endif

library_includedir = $(pkgincludedir)
library_include_HEADERS = $(libgarden_h_sources)

######################
# Build libgarden
######################
libgarden_la_SOURCES = $(libgarden_c_sources)

if USE_GLIB
libgarden_la_CFLAGS = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
libgarden_la_LDFLAGS = -lstdc++ -lpthread @GLIB_LIBS@ -shared -avoid-version
libgarden_la_CPPFLAGS = -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS) @GLIB_CFLAGS@
else
libgarden_la_CFLAGS = $(AM_CFLAGS)
libgarden_la_LDFLAGS = -lpthread -shared -version-info 1:0:0
libgarden_la_CPPFLAGS = $(AM_CFLAGS) $(AM_CPPFLAGS)
endif

libgarden_la_LIBADD = -lcutils -ldl $(LOC_LIBS) $(required_libs)
library_include_HEADERS = $(libgarden_h_sources)

lib_LTLIBRARIES = libgarden.la

######################
# Build libgarden_haltests_e2e
######################
libgarden_haltests_e2e_la_SOURCES = $(libgarden_cases_c_sources)

if USE_GLIB
libgarden_haltests_e2e_la_CFLAGS = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
libgarden_haltests_e2e_la_LDFLAGS = -lstdc++ -lpthread @GLIB_LIBS@ -shared -avoid-version
libgarden_haltests_e2e_la_CPPFLAGS = -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS) @GLIB_CFLAGS@
else
libgarden_haltests_e2e_la_CFLAGS = $(AM_CFLAGS)
libgarden_haltests_e2e_la_LDFLAGS = -lpthread -shared -version-info 1:0:0
libgarden_haltests_e2e_la_CPPFLAGS = $(AM_CFLAGS) $(AM_CPPFLAGS)
endif

libgarden_haltests_e2e_la_LIBADD = libgarden.la -lcutils -ldl $(LOC_LIBS) $(required_libs)
library_include_HEADERS = $(libgarden_cases_h_sources)

lib_LTLIBRARIES += libgarden_haltests_e2e.la

######################
# Build sample_case
######################
libgardencasesample_la_SOURCES = sample_case/SampleCase.cpp

if USE_GLIB
libgardencasesample_la_CFLAGS = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
libgardencasesample_la_LDFLAGS = -lstdc++ -lpthread @GLIB_LIBS@ -shared -avoid-version
libgardencasesample_la_CPPFLAGS = -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS) @GLIB_CFLAGS@
else
libgardencasesample_la_CFLAGS = $(AM_CFLAGS)
libgardencasesample_la_LDFLAGS = -lpthread -shared -version-info 1:0:0
libgardencasesample_la_CPPFLAGS = $(AM_CFLAGS) $(AM_CPPFLAGS)
endif

libgardencasesample_la_LIBADD = libgarden.la -lcutils -ldl $(LOC_LIBS) $(required_libs)
lib_LTLIBRARIES += libgardencasesample.la

######################
# Build garden_app
######################
garden_app_SOURCES = main.cpp

if USE_GLIB
garden_app_CFLAGS = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
garden_app_LDFLAGS = -lstdc++ -g -Wl,-z,defs -lpthread @GLIB_LIBS@ -shared -version-info 1:0:0
garden_app_CPPFLAGS = -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS) @GLIB_CFLAGS@
else
garden_app_CFLAGS = $(AM_CFLAGS)
garden_app_LDFLAGS = -Wl,-z,defs -lpthread -shared -version-info 1:0:0
garden_app_CPPFLAGS = $(AM_CFLAGS) $(AM_CPPFLAGS)
endif

garden_app_LDADD = libgarden.la libgarden_haltests_e2e.la -lcutils -ldl $(required_libs)

bin_PROGRAMS = garden_app
