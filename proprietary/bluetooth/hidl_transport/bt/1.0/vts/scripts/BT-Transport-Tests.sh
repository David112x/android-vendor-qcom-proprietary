#!/bin/sh

#
# Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.
#

# Description:
#   Script used by bttransporttests.py for
#   executing test cases
#

if [ $7 -eq "1" ]
then
logcommands="--logCommands"
else
logcommands=""
fi
if [ $8 -eq "1" ]
then
logevents="--logEvents"
else
logevents=""
fi
if [ $9 -eq "1" ]
then
logdata="--logData"
else
logdata=""
fi
if [ ${10} -eq "1" ]
then
consolelogs="--consoleLog"
else
consolelogs=""
fi
rm -rf  /data/nativetest64/VtsHalBluetooth_TransportTest/tstlog.txt
for i in $(seq 1 $1)
do
echo Loop $i >> /data/nativetest64/VtsHalBluetooth_TransportTest/tstlog.txt
/data/nativetest64/VtsHalBluetooth_TransportTest/VtsHalBluetooth_TransportTest --gtest_filter=$2 --maxAppSleepTimer=$3 --minAppSleepTimer=$4 --maxIterations=$5 --maxMsgsPerIteration=$6 $logcommands $logevents $logdata $consolelogs >> /data/nativetest64/VtsHalBluetooth_TransportTest/tstlog.txt
done
