if ENABLE_MPEGH

AM_CFLAGS =  -O0 -g -Wall -Werror -Wno-sign-compare -Wno-missing-field-initializers

OmxMpeghEncSwTest_Inc  := -I $(PKG_CONFIG_SYSROOT_DIR)/usr/include/mpegh-enc
OmxMpeghEncSwTest_Inc  += -I $(PKG_CONFIG_SYSROOT_DIR)/usr/include/mm-core/omxcore

test_sources := tst/omx_mpegh_enc_test.cpp
test_sources +=tst/InWavFile_c.c
test_sources +=tst/MPEGHEncFileUtil_c.c
test_sources +=tst/wavefile.c
test_sources +=tst/wavIO.c

bin_PROGRAMS = mm-aenc-omxmpegh-sw-test
mm_aenc_omxmpegh_sw_test_SOURCES = $(test_sources)
mm_aenc_omxmpegh_sw_test_LDADD = -lOmxMpeghEncSw -llog -lmm-omxcore $(GLIB_LIBS)
mm_aenc_omxmpegh_sw_test_CFLAGS = $(AM_CFLAGS) -DPCM24_SUPPORT -DPCM16_SUPPORT -Wno-strlcpy-strlcat-size $(OmxMpeghEncSwTest_Inc)
mm_aenc_omxmpegh_sw_test_CFLAGS += $(GLIB_CFLAGS) -Dstrlcat=g_strlcat -Dstrlcpy=g_strlcpy -include glib.h
mm_aenc_omxmpegh_sw_test_CPPFLAGS = $(AM_CFLAGS) -DPCM24_SUPPORT -DPCM16_SUPPORT -Wno-strlcpy-strlcat-size $(OmxMpeghEncSwTest_Inc) -D__unused=__attribute__\(\(__unused__\)\)
mm_aenc_omxmpegh_sw_test_LDFLAGS = -module

endif
