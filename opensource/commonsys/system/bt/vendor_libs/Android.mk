# Common C/C++ compiler flags for test-vendor lib
#
# -Wno-gnu-variable-sized-type-not-at-end is needed, because struct BT_HDR
#  is defined as a variable-size header in a struct.
# -Wno-typedef-redefinition is needed because of the way the struct typedef
#  is done in osi/include header files. This issue can be obsoleted by
#  switching to C11 or C++.
# -Wno-unused-parameter is needed, because there are too many unused
#  parameters in all the code.
#
test-vendor_CFLAGS += \
  -fvisibility=hidden \
  -Wall \
  -Wextra \
  -Werror \
  -Wno-gnu-variable-sized-type-not-at-end \
  -Wno-typedef-redefinition \
  -Wno-unused-parameter \
  -DLOG_NDEBUG=1 \
  -DEXPORT_SYMBOL="__attribute__((visibility(\"default\")))"

test-vendor_CONLYFLAGS += -std=c99

include $(call all-subdir-makefiles)

# Cleanup our locals
test-vendor_CFLAGS :=
test-vendor_CONLYFLAGS :=
