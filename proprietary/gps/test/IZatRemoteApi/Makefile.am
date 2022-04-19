AM_CFLAGS = \
    $(GPSUTILS_CFLAGS) \
    -std=c++11

required_libs = \
    $(IZATCLIENTAPI_LIBS) \
    $(GPSUTILS_LIBS)

izat_remote_api_test_c_sources = \
     IZatRemoteApiTest.cpp

izat_remote_api_test_SOURCES = $(izat_remote_api_test_c_sources)

if USE_GLIB
izat_remote_api_test_CFLAGS = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
izat_remote_api_test_LDFLAGS = -lstdc++ -g -lpthread @GLIB_LIBS@ -shared -version-info 1:0:0
izat_remote_api_test_CPPFLAGS = -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS) @GLIB_CFLAGS@
else
izat_remote_api_test_CFLAGS = $(AM_CFLAGS)
izat_remote_api_test_LDFLAGS = -lpthread -shared -version-info 1:0:0
izat_remote_api_test_CPPFLAGS = $(AM_CFLAGS) $(AM_CPPFLAGS)
endif

izat_remote_api_test_LDADD = -lcutils -ldl $(required_libs)

bin_PROGRAMS = izat_remote_api_test
