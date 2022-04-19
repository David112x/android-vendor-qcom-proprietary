# ---------------------------------------------------------------------------------
# Makefile to compile sample shared library using EnginePluginAPI.h on linux platform
# By default no compiler flags are added to generate debug information. Can be added if required.
# ---------------------------------------------------------------------------------
ALL= libep_ref_simulator.so
all: ${ALL}


CC = arm-linux-gnueabihf-gcc # C compiler # C compiler
CXX = arm-linux-gnueabihf-g++ # C compiler # C compiler
CFLAGS = -fPIC -Wall # C flags
CXXFLAGS = $(CFLAGS) -std=c++11 # C flags
LDFLAGS = -shared -fPIC # linking flags
TARGET_LIB = libep_ref_simulator.so # target lib


LIB_OBJS=$(patsubst %.cpp,%.o,$(wildcard *.cpp))
LIB_OBJS+=$(patsubst %.c,%.o,$(wildcard *.c))

HEADERS=$(wildcard *.h)

%.o: %.cpp
	$(CXX) -c -o $@ ${CXXFLAGS} $(CFLAGS) $<
	@echo "  CXX " $<
%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
	@echo "  CC " $<

libep_ref_simulator.so: $(LIB_OBJS)
	$(CXX) -shared -fPIC -s -o $@ -Wl,-whole-archive $? -Wl,-no-whole-archive -lc

clean:
	rm -f ${ALL} $(LIB_OBJS)
