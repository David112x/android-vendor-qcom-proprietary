#!/bin/bash

# Copyright (c) 2019 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.

CMD=QtiBusMessengerTest

TIMES=100

if [ "0$1" -ne "0" ]; then
    TIMES=$1
fi
i=1;
echo "Deleting shmbus file"
rm -f shmbus
while [ "${i}" -le "${TIMES}" ]; do
    if [ -w QtiBusMessenger1.log ]; then
        mv QtiBusMessenger1.log QtiBusMessenger1.old.log
    fi
    if [ -w QtiBusMessenger2.log ]; then
        mv QtiBusMessenger2.log QtiBusMessenger2.old.log
    fi
    echo ""
    echo "Attempt number ${i}"
    echo -n "Starting instance1: "
    time ${CMD} > QtiBusMessenger1.log 2>&1 &
    echo "pid: $!"
    echo -n "Starting instance2: "
    time ${CMD} -r > QtiBusMessenger2.log 2>&1 &
    echo "pid: $!"
    wait
    i=$((i+1))
done
