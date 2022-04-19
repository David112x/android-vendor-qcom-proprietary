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
# The Android.mk may contain one or more module.
# If any module includes kernel header file(s) then dependency rule should be added.
# The FAIL case condition: found_c_include=true && found_add_dep=false.
# The pass case condition: found_c_include=true && found_add_dep=true.
# By An 'exit 0' indicates success, while a non-zero exit value means missing dependency.
#

subdir="hardware/qcom vendor/qcom"
cnt_module=0
cnt_error=0
fnd_c_include=false
fnd_add_dep=false

function reset_flags () {
    fnd_c_include=false
    fnd_add_dep=false
}

function check_if_missing_dep() {
    if [[ "$fnd_add_dep" == false && "$fnd_c_include" == true ]]; then
        cnt_error=$((cnt_error+1))
        echo "cnt_error : $cnt_error"
        echo "-----------------------------------------------------"
        echo "Error: Missing LOCAL_ADDITIONAL_DEPENDENCIES in module : $2"
        echo "in file: $1"
        echo "please use LOCAL_ADDITIONAL_DEPENDENCIES += \$(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr"
        echo "-----------------------------------------------------"
    fi
}

function scan_files(){
    for i in "${array[@]}"; do
        while read line
        do
        case $line in
        \#*)
            continue
            ;;
        *CLEAR_VARS*)
            if [[ "$cnt_module" -gt 0 ]]; then
                check_if_missing_dep "$i" "$cnt_module"
            fi
            reset_flags
            cnt_module=$((cnt_module+1))
            continue
            ;;
        LOCAL_C_INCLUDES*KERNEL_OBJ/usr*)
            fnd_c_include=true
            ;;
        LOCAL_ADDITIONAL_DEPENDENCIES*KERNEL_OBJ/usr*)
            fnd_add_dep=true
            ;;
        esac
        done < $i
        check_if_missing_dep "$i" "$cnt_module"
        cnt_module=0
        reset_flags
    done
    if [[ "$cnt_error" -gt 0 ]]; then
        exit 1
    else
        exit 0
    fi
}
echo " Checking dependency on kernel headers ......"
while IFS=  read -r -d $'\0'; do
    array+=("$REPLY")
done < <(find ${subdir} -type f -name "Android.mk" -print0)
scan_files
