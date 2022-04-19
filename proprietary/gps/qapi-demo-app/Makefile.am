AM_CFLAGS = \
    -DDEBUG \
    $(LOCATIONQAPI_CFLAGS) \
    $(GPSUTILS_CFLAGS) \
    -std=c++11

required_libs = \
    $(LOCATIONQAPI_LIBS) \
    $(GPSUTILS_LIBS)

######################
# Build qapi-demo_app
######################
qapi_demo_app_SOURCES = main.cpp

if USE_GLIB
qapi_demo_app_CFLAGS = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
qapi_demo_app_LDFLAGS = -lstdc++ -g -Wl,-z,defs -lpthread @GLIB_LIBS@ -shared -version-info 1:0:0
qapi_demo_app_CPPFLAGS = -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS) @GLIB_CFLAGS@
else
qapi_demo_app_CFLAGS = $(AM_CFLAGS)
qapi_demo_app_LDFLAGS = -Wl,-z,defs -lpthread -shared -version-info 1:0:0
qapi_demo_app_CPPFLAGS = $(AM_CFLAGS) $(AM_CPPFLAGS)
endif

qapi_demo_app_LDADD = -lcutils -ldl $(required_libs)

bin_PROGRAMS = qapi_demo_app

