#!/bin/bash
#
# Copyright (c) 2019, The Linux Foundation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#     * Neither the name of The Linux Foundation nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
# ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# This script is mainly to compile QSSI targets. For other targets, usage
# of regular "make" is recommended.
#
# To run this script, do the following:
#
#  source build/envsetup.sh
#  lunch <target>-userdebug
#  ./vendor/qcom/opensource/core-utils/build/build.sh <make options>
#
# Note: For QSSI targets, this script cannot be used to compile individual images
#

###########################
# Build.sh versioning:
###########################
# build.sh supports '--version' option, returns the version number.
# Version number is based on the features/commands supported by it.
# The file - './vendor/qcom/opensource/core-utils/build/build.sh.versioned' indicates that build.sh
# supports versioning. So, it's required to first check for this file's existence before
# calling with '--version', since this versioning support didn't exist from the beginning of this script.
#
# Version 0:
#     Supports just the basic make commands (passes on all args like -j32 to the make command).
# Version 1:
#     Supports dist command as well - needed for target-files/ota generation.
#     Usage: ./build.sh dist -j32
#     This triggers make dist for qssi and target lunch, generates target-files, merges them
#     and triggers ota generation.
# Version 2:
#     Supports custom copy paths for dynamic patition images when compiled with dist.
#     option : --dp_images_path=<custom-copy-path>
# Version 3:
#     Supports segmenting the build into qssi only, target only and merge only steps and
#     enabling users to call specific steps or full build by giving no separate steps.
#     options: --qssi_only, --target_only, --merge_only
#     Usage: ./build.sh dist -j32 --qssi_only (for only qssi build) or ./build.sh dist -j32 (for full build)
#     Note: qssi_only and target_only options can be given together but merge_only should not be combined with
#           any other options.
BUILD_SH_VERSION=3
if [ "$1" == "--version" ]; then
    return $BUILD_SH_VERSION
    # Above return will work only if someone source'ed this script (which is expected, need to source the script).
    # Add extra exit 0 to ensure script doesn't proceed further (if someone didn't use source but passed --version)
    exit 0
fi
###########################

#Sanitize host toolsi
LS=`which ls`
LS=${LS:-ls}
MV=`which mv`
MV=${MV:-mv}
RM=`which rm`
RM=${RM:-rm}
CAT=`which cat`
CAT=${CAT:-cat}
CUT=`which cut`
CUT=${CUT:-cut}
REV=`which rev`
REV=${REV:-rev}
SED=`which sed`
SED=${SED:-sed}
DIFF=`which diff`
DIFF=${DIFF:-diff}
ECHO=`which echo`
ECHO=${ECHO:-echo}
FIND=`which find`
FIND=${FIND:-find}
GREP=`which grep`
GREP=${GREP:-grep}

MAKE_ARGUMENTS=()
MERGE_ONLY=0
QSSI_ONLY=0
TARGET_ONLY=0
FULL_BUILD=0
USE_CCACHE=1

while [[ $# -gt 0 ]]
    do
    arg="$1"
    case $arg in
        *merge_only)
            MERGE_ONLY=1
            shift # go to next argument
            ;;
        *qssi_only)
            QSSI_ONLY=1
            shift
            ;;
        *target_only)
            TARGET_ONLY=1
            shift
            ;;
        *)  # all other option
            MAKE_ARGUMENTS+=("$1") # save it in an array to pass to make later
            shift
            ;;
    esac
done
set -- "${MAKE_ARGUMENTS[@]}" # restore the argument list ($@) to be set to MAKE_ARGUMENTS

# If none of the discrete options are passed, this is a full build
if [[ "$MERGE_ONLY" != 1 && "$QSSI_ONLY" != 1 && "$TARGET_ONLY" != 1 ]]; then
    FULL_BUILD=1
fi

if [[ "$MERGE_ONLY" == 1 ]]; then
    if [[ "$QSSI_ONLY" == 1 || "$TARGET_ONLY" == 1 ]]; then
        echo "merge_only cannot be passed along with qssi_only or target_only options"
        exit 1
    fi
fi

QSSI_TARGETS_LIST=("lahaina" "sdm710" "sdm845" "msmnile" "sm6150" "kona" "atoll" "trinket" "trinket_32" "lito" "bengal" "bengal_32" "bengal_32go")
QSSI_TARGET_FLAG=0

case "$TARGET_PRODUCT" in
    *_32)
        TARGET_QSSI="qssi_32"
        ;;
    *_32go)
        TARGET_QSSI="qssi_32go"
        ;;
    *)
        TARGET_QSSI="qssi"
        ;;
esac

# Export BUILD_DATETIME so that both Qssi and target images get the same timestamp
DATE=`which date`
DATE=${DATE:-date}
EPOCH_TIME=`${DATE} +%s`
export BUILD_DATETIME="$EPOCH_TIME"

NON_AB_TARGET_LIST=("bengal_32go")
for NON_AB_TARGET in "${NON_AB_TARGET_LIST[@]}"
do
    if [ "$TARGET_PRODUCT" == "$NON_AB_TARGET" ]; then
        log "${TARGET_PRODUCT} found in Non-A/B Target List"
        ENABLE_AB=false
        break
    fi
done

# Default A/B configuration flag for all QSSI targets (not used for legacy targets).
ENABLE_AB=${ENABLE_AB:-true}
ARGS="$@"
QSSI_ARGS="$ARGS ENABLE_AB=$ENABLE_AB"

# OTA/Dist related variables
#This flag control dynamic partition enablement
BOARD_DYNAMIC_PARTITION_ENABLE=false

# OTA/Dist related variaibles
QSSI_OUT="out/target/product/$TARGET_QSSI"
DIST_COMMAND="dist"
DIST_ENABLED=false
QSSI_ARGS_WITHOUT_DIST=""
DIST_DIR="out/dist"
MERGED_TARGET_FILES="$DIST_DIR/merged-qssi_${TARGET_PRODUCT}-target_files.zip"
MERGED_OTA_ZIP="$DIST_DIR/merged-qssi_${TARGET_PRODUCT}-ota.zip"
DIST_ENABLED_TARGET_LIST=("lahaina" "kona" "sdm710" "sdm845" "msmnile" "sm6150" "trinket" "trinket_32" "lito" "bengal" "bengal_32" "atoll" "sdm660_64" "bengal_32go")
DYNAMIC_PARTITION_ENABLED_TARGET_LIST=("lahaina" "kona" "msmnile" "sdm710" "lito" "trinket" "trinket_32" "atoll" "sdm660_64" "bengal" "bengal_32" "bengal_32go")
DYNAMIC_PARTITIONS_IMAGES_PATH=$OUT
DP_IMAGES_OVERRIDE=false
function log() {
    ${ECHO} "============================================"
    ${ECHO} "[build.sh]: $@"
    ${ECHO} "============================================"
}

for DP_TARGET in "${DYNAMIC_PARTITION_ENABLED_TARGET_LIST[@]}"
do
    if [ "$TARGET_PRODUCT" == "$DP_TARGET" ]; then
        log "${TARGET_PRODUCT} found in Dynamic Parition Enablement List"
        BOARD_DYNAMIC_PARTITION_ENABLE=true
        break
    fi
done

# Set Dynamic Partitio value
QSSI_ARGS="$QSSI_ARGS BOARD_DYNAMIC_PARTITION_ENABLE=$BOARD_DYNAMIC_PARTITION_ENABLE"

# Set Shipping API level on target basis.
SHIPPING_API_P="28"
SHIPPING_API_Q="29"
SHIPPING_API_P_TARGET_LIST=("sdm845" "sm6150")
SHIPPING_API_LEVEL=$SHIPPING_API_Q
for P_API_TARGET in "${SHIPPING_API_P_TARGET_LIST[@]}"
do
    if [ "$TARGET_PRODUCT" == "$P_API_TARGET" ]; then
        SHIPPING_API_LEVEL=$SHIPPING_API_P
        break
    fi
done
QSSI_ARGS="$QSSI_ARGS SHIPPING_API_LEVEL=$SHIPPING_API_LEVEL"

for ARG in $QSSI_ARGS
do
    if [ "$ARG" == "$DIST_COMMAND" ]; then
        DIST_ENABLED=true
    elif [[ "$ARG" == *"--dp_images_path"* ]]; then
        DP_IMAGES_OVERRIDE=true
        DYNAMIC_PARTITIONS_IMAGES_PATH=$(${ECHO} "$ARG" | ${CUT} -d'=' -f 2)
    else
        QSSI_ARGS_WITHOUT_DIST="$QSSI_ARGS_WITHOUT_DIST $ARG"
    fi
done

#Strip image_path if present
if [ "$DP_IMAGES_OVERRIDE" = true ]; then
    QSSI_ARGS=${QSSI_ARGS//"--dp_images_path=$DYNAMIC_PARTITIONS_IMAGES_PATH"/}
fi

# Check if dist is supported on this target (yet) or not, and override DIST_ENABLED flag.
IS_DIST_ENABLED_TARGET=false
for DIST_TARGET in "${DIST_ENABLED_TARGET_LIST[@]}"
do
    if [ "$TARGET_PRODUCT" == "$DIST_TARGET" ]; then
        IS_DIST_ENABLED_TARGET=true
        break
    fi
done

# Disable dist if it's not supported (yet).
if [ "$IS_DIST_ENABLED_TARGET" = false ] && [ "$DIST_ENABLED" = true ]; then
    QSSI_ARGS="$QSSI_ARGS_WITHOUT_DIST"
    DIST_ENABLED=false
    log "Dist not (yet) enabled for $TARGET_PRODUCT"
fi

function check_return_value () {
    retVal=$1
    command=$2
    if [ $retVal -ne 0 ]; then
        log "FAILED: $command"
        exit $retVal
    fi
}

function command () {
    command=$@
    log "Command: \"$command\""
    time $command
    retVal=$?
    check_return_value $retVal "$command"
}

function check_if_file_exists () {
    if [[ ! -f "$1" ]]; then
        log "Could not find the file: \"$1\", aborting.."
        exit 1
    fi
}

function generate_dynamic_partition_images () {
    log "Generate Dynamic Partition Images for ${TARGET_PRODUCT}"
    # Handling for Dist Enabled targets
    # super.img/super_empty generation
    if [ "$DIST_ENABLED" = true ]; then
       if [ "$DP_IMAGES_OVERRIDE" = true ]; then
           command "mkdir -p $DYNAMIC_PARTITIONS_IMAGES_PATH"
       fi
       command "cp $QSSI_OUT/vbmeta_system.img $OUT/"
       command "unzip -jo $MERGED_TARGET_FILES IMAGES/*.img -x IMAGES/userdata.img -d $DYNAMIC_PARTITIONS_IMAGES_PATH"
       command "./build/tools/releasetools/build_super_image.py $MERGED_TARGET_FILES $DYNAMIC_PARTITIONS_IMAGES_PATH/super.img"
    else
        command "cp $QSSI_OUT/vbmeta_system.img $OUT/"
        command "mkdir -p out/${TARGET_PRODUCT}_dpm"
        check_if_file_exists "$QSSI_OUT/dynamic_partition_metadata.txt"
        check_if_file_exists "$OUT/dynamic_partition_metadata.txt"
        MERGED_DPM_PATH="out/${TARGET_PRODUCT}_dpm/dynamic_partition_metadata.txt"
        MERGE_DYNAMIC_PARTITION_INFO_COMMAND="./build/tools/releasetools/merge_dynamic_partition_metadata.py \
            --qssi-dpm-file=$QSSI_OUT/dynamic_partition_metadata.txt \
            --target-dpm-file=$OUT/dynamic_partition_metadata.txt \
            --merged-dpm-file=$MERGED_DPM_PATH"
        # Temporarily change permission of merge_dynamic_partition_metadata.py till proper fix is merged.
        command "chmod 755 build/tools/releasetools/merge_dynamic_partition_metadata.py"
        command "$MERGE_DYNAMIC_PARTITION_INFO_COMMAND"
        command "./build/tools/releasetools/build_super_image.py $MERGED_DPM_PATH $OUT/super_empty.img"
    fi
}

function generate_ota_zip () {
    log "Processing dist/ota commands:"

    SYSTEM_TARGET_FILES="$(find $DIST_DIR -name "qssi*-target_files-*.zip" -print)"
    log "SYSTEM_TARGET_FILES=$SYSTEM_TARGET_FILES"
    check_if_file_exists "$SYSTEM_TARGET_FILES"

    OTHER_TARGET_FILES="$(find $DIST_DIR -name "${TARGET_PRODUCT}*-target_files-*.zip" -print)"
    log "OTHER_TARGET_FILES=$OTHER_TARGET_FILES"
    check_if_file_exists "$OTHER_TARGET_FILES"

    log "MERGED_TARGET_FILES=$MERGED_TARGET_FILES"

    check_if_file_exists "$DIST_DIR/merge_config_system_misc_info_keys"
    check_if_file_exists "$DIST_DIR/merge_config_system_item_list"
    check_if_file_exists "$DIST_DIR/merge_config_other_item_list"

    MERGE_TARGET_FILES_COMMAND="./build/tools/releasetools/merge_target_files.py \
        --system-target-files $SYSTEM_TARGET_FILES \
        --other-target-files $OTHER_TARGET_FILES \
        --output-target-files $MERGED_TARGET_FILES \
        --system-misc-info-keys $DIST_DIR/merge_config_system_misc_info_keys \
        --system-item-list $DIST_DIR/merge_config_system_item_list \
        --other-item-list $DIST_DIR/merge_config_other_item_list \
        --output-ota  $MERGED_OTA_ZIP"

    if [ "$ENABLE_AB" = false ]; then
        MERGE_TARGET_FILES_COMMAND="$MERGE_TARGET_FILES_COMMAND --rebuild_recovery"
    fi

    command "$MERGE_TARGET_FILES_COMMAND"
}

function build_qssi_only () {
    command "source build/envsetup.sh"
    command "$QTI_BUILDTOOLS_DIR/build/kheaders-dep-scanner.sh"
    command "lunch ${TARGET_QSSI}-${TARGET_BUILD_VARIANT}"
    command "make $QSSI_ARGS"
}

function build_target_only () {
    command "source build/envsetup.sh"
    command "$QTI_BUILDTOOLS_DIR/build/kheaders-dep-scanner.sh"
    command "lunch ${TARGET}-${TARGET_BUILD_VARIANT}"
    command "make $QSSI_ARGS"
}

function merge_only () {
    # DIST/OTA specific operations:
    if [ "$DIST_ENABLED" = true ]; then
        generate_ota_zip
    fi
    # Handle dynamic partition case and generate images
    if [ "$BOARD_DYNAMIC_PARTITION_ENABLE" = true ]; then
        generate_dynamic_partition_images
    fi
}

function full_build () {
    build_qssi_only
    build_target_only
    # Copy Qssi system|product.img to target folder so that all images can be picked up from one folder
    command "cp $QSSI_OUT/system.img $OUT/"
    if [ -f  $QSSI_OUT/product.img ]; then
        command "cp $QSSI_OUT/product.img $OUT/"
    fi
    merge_only
}

if [ "$TARGET_PRODUCT" == "qssi" ]; then
    log "FAILED: lunch option should not be set to qssi. Please set a target out of the following QSSI targets: ${QSSI_TARGETS_LIST[@]}"
    exit 1
fi

# Check if qssi is supported on this target or not.
for QSSI_TARGET in "${QSSI_TARGETS_LIST[@]}"
do
    if [ "$TARGET_PRODUCT" == "$QSSI_TARGET" ]; then
        QSSI_TARGET_FLAG=1
        break
    fi
done

# For non-QSSI targets
if [ $QSSI_TARGET_FLAG -eq 0 ]; then
    log "${TARGET_PRODUCT} is not a QSSI target. Using legacy build process for compilation..."
    command "source build/envsetup.sh"
    command "make $ARGS"
else # For QSSI targets
    log "Building Android using build.sh for ${TARGET_PRODUCT}..."
    log "QSSI_ARGS=\"$QSSI_ARGS\""
    log "DIST_ENABLED=$DIST_ENABLED, ENABLE_AB=$ENABLE_AB"

    TARGET="$TARGET_PRODUCT"
    export TARGET_PARENT_VENDOR="$TARGET_PRODUCT"
    if [[ "$FULL_BUILD" -eq 1 ]]; then
        log "Executing a full build ..."
        full_build
    fi

    if [[ "$QSSI_ONLY" -eq 1 ]]; then
        log "Executing a QSSI only build ..."
        build_qssi_only
    fi

    if [[ "$TARGET_ONLY" -eq 1 ]]; then
        log "Executing a target only build for $TARGET_PRODUCT ..."
        build_target_only
    fi

    if [[ "$MERGE_ONLY" -eq 1 ]]; then
        log "Executing a merge only operation ..."
        merge_only
    fi
fi
