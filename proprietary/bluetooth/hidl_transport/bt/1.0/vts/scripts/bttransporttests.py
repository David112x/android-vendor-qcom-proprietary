#!python

#
# Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.
#

# Description:
#   Script for executing BT Host Transport Tests
#   Help - "python bttransporttests.py --help"
#   List of test cases -"python bttransporttests.py --listtests"
#   Execute tests: Execute BT On/Off tests 2 times in USB diconnected
#                  mode and send results to specified email addresses
#     python bttransporttests.py  --testfilter="*.BtOnOff"  --usbDisc
#           --mailresults -repeatcount=2 --mailsender="xy@abc.com"
#           --mailreceivers="aaaa@abc.com,bbbb@abc.com"
#
# For USB disconnected tests(-usbdisc option), alpaca python modules
# need to be installed:
#   python -m pip install alpaca --upgrade --find-links
#        \\ben\corebsp_labdata_0001\hwengines\adc\Alpaca\scripts\python\dist
#
import sys
import time
import os
import shutil
import argparse
import subprocess
import datetime

combTestLogFileName = "combTestLog.txt"
testResult = "All Tests Passed"
testFailed = False
dateandtime = ""
logPath = ""

def main():
    parseArguments()

    setUpEnvironment()

    for count in range(repeatCount):
        executeTests(count)
        collectLogs(count)
        if exitOnFail and testFailed:
            break

    emailResults()

    cleanup()

def parseArguments():
    global listTestCases
    global exitOnFail
    global mailResults
    global mailSender
    global mailReceivers
    global smtpServer
    global adbDevice
    global logDirPrefix
    global baseLogPath
    global repeatCount
    global loop
    global usbDisc
    global testfilter
    global maxappsleeptimer
    global minappsleeptimer
    global maxiterations
    global maxmsgsperiteration
    global logcommands
    global logevents
    global logdata
    global consolelog
    global exitOnFail
    global testWaitTime

    ap = argparse.ArgumentParser()

    defaultMailSender = "uvats@qti.qual" + "comm.com"
    defaultMailReceiver = "uvats@qti.qual" + "comm.com"
    defaultSmtpServer = "smtphost.qual" + "comm.com"
    defaultDevice = subprocess.check_output(["adb", "devices"]).decode('utf8').strip().replace("List of devices attached","").strip()[:-7]

    ap.add_argument("--listtests", action='store_true', help="List of Testcases", default=False)
    ap.add_argument("--exitonfail", action='store_true', help="Exit test execution when  first repetion fails", default=False)
    ap.add_argument("--mailresults", action='store_true', help="Send email when tests finish", default=False)
    ap.add_argument("--mailsender", help="Mail Sener", default=defaultMailSender)
    ap.add_argument("--mailreceivers", help="List of Mail Receivers", default=defaultMailReceiver)
    ap.add_argument("--smtpserver", help="SMTP Server", default=defaultSmtpServer)
    ap.add_argument("--repeatcount", type=int, help="Repetition count for all tests", default=1)
    ap.add_argument("--adbdevice", help="ADB Device Id", default=defaultDevice)
    ap.add_argument("--usbdisc", action='store_true', help="Is USB disconnected", default=False)
    ap.add_argument("--loop", type=int, help="How many times a each test will loop", default=1)
    ap.add_argument("--testfilter", help="Tests to execute", default=str("BluetoothTransportTest.*"))
    ap.add_argument("--maxappsleep", type=int, help="Maximum App Sleep Timer(ms)", default=30000)
    ap.add_argument("--minappsleep", type=int, help="Minimum App Sleep Timer(ms)", default=10000)
    ap.add_argument("--maxiter", type=int, help="Maximum Iterations", default=2)
    ap.add_argument("--maxmsgsperiter", type=int, help="Maximum msgs per iteration", default=3)
    ap.add_argument("--logcommands", type=int, help="Enable command logs", default=1)
    ap.add_argument("--logevents", type=int, help="Enable event logs", default=1)
    ap.add_argument("--logdata", type=int, help="Enable data logs", default=1)
    ap.add_argument("--consolelog", type=int, help="Enable console logs", default=1)
    ap.add_argument("--logpath", help="Path for the logs", default=os.getcwd())
    ap.add_argument("--logdirprefix", help="Prefix for the log dir", default=None)
    args = vars(ap.parse_args())
    print(args)

    listTestCases = args['listtests']
    exitOnFail = args['exitonfail']
    mailResults = args['mailresults']
    mailSender = args['mailsender']
    mailReceivers = args['mailreceivers']
    smtpServer = args['smtpserver']
    adbDevice = args['adbdevice']
    logDirPrefix = args['logdirprefix']
    baseLogPath = args['logpath']
    repeatCount = args['repeatcount']
    loop = args['loop']
    usbDisc = args['usbdisc']
    testfilter = args['testfilter']
    maxappsleeptimer = args['maxappsleep']
    minappsleeptimer = args['minappsleep']
    maxiterations = args['maxiter']
    maxmsgsperiteration = args['maxmsgsperiter']
    logcommands = args['logcommands']
    logevents = args['logevents']
    logdata = args['logdata']
    consolelog = args['consolelog']

    # testWaitTime is valid only for USB
    if usbDisc:
        testWaitTime = 0.4 * (maxappsleeptimer * maxiterations) / 1000
        print("Execution Time(sec):", testWaitTime)

    if mailResults:
        import smtplib
        from os.path import basename
        from email.mime.text import MIMEText
        from email.mime.multipart import MIMEMultipart
        from email.mime.application import MIMEApplication
        global smtplib
        global basename
        global MIMEText
        global MIMEMultipart
        global MIMEApplication

def initAlpaca():
    print(usbDisc)
    if (usbDisc):
        import alpaca.tac
        global alpaca
        global tac
        print("Connect Alpaca for USB disconnect/reconnect")
        ports = alpaca.tac.ProbeForDevices()
        for port in ports:
            print("TAC device found on: " + port)
        if len(ports) == 0:
            print("No TAC devices found. Exiting...")
            sys.exit()
        tac = alpaca.tac.Tac(ports[0])
        print("ALPACA version information:")
        print(tac.GetVersion() + "\n")
        # If multiple ALPACA devices are connected you can use UUID to
        # identify a specific one
        print("ALPACA UUID: " + tac.GetUuid() + "\n")

def setUpEnvironment():
    global logPath
    global dateandtime
    subprocess.call("adb root",shell=True)
    time.sleep(3)
    subprocess.call("adb remount",shell=True)
    time.sleep(3)

    # if the below folder doesn't exist on device, uncomment the below line
    subprocess.call("adb shell mkdir /data/nativetest64/VtsHalBluetooth_TransportTest")

    # if the VTS test binary doesn't exist on the DUT, push it to the above path"
    subprocess.call("adb push VtsHalBluetooth_TransportTest /data/nativetest64/VtsHalBluetooth_TransportTest")
    subprocess.call("adb shell chmod 755 /data/nativetest64/VtsHalBluetooth_TransportTest/VtsHalBluetooth_TransportTest")
    if listTestCases:
        subprocess.call("adb shell /data/nativetest64/VtsHalBluetooth_TransportTest/VtsHalBluetooth_TransportTest --gtest_list_tests")
        exit(0)

    # Alpaca is needed only for USB disconnect mode testing
    initAlpaca()

    print("Turning BT OFF")
    subprocess.check_output("adb shell service call bluetooth_manager 8")

    # Push the shell script file for executing the tests
    subprocess.call("adb push BT-Transport-Tests.sh /data/nativetest64/VtsHalBluetooth_TransportTest")
    subprocess.call("adb shell chmod 755 /data/nativetest64/VtsHalBluetooth_TransportTest/BT-Transport-Tests.sh")

    # enable kernel logs
    subprocess.call(["adb", "shell", "echo -n", "file bluetooth-power.c +p", ">", "/sys/kernel/debug/dynamic_debug/control"])

    # disable security so that tombstones can be generated
    global selinuxmode
    selinuxmode = subprocess.check_output(["adb", "shell", "getenforce"]).strip()

    # set ssr level top 3 so that tombstones are generated for some issue
    subprocess.call("adb shell setprop persist.vendor.service.bdroid.ssrlvl 3")

    print("SELinux mode: ", selinuxmode)
    if (selinuxmode != b'Permissive'):
        subprocess.call(["adb", "shell", "setenforce", "0"])
        print("Changed SELinux policy to Permissive")
    else:
        print("Already in Permissive mode")

    global combTestLogFile
    combTestLogFile = open(combTestLogFileName, "w+")

    dateandtime = datetime.datetime.now().strftime("%m%d%Y-%H%M%S")
    if logDirPrefix:
        logPath = os.path.join(baseLogPath, logDirPrefix + "-" + dateandtime)
    else:
        logPath = os.path.join(baseLogPath, dateandtime)
    os.mkdir(logPath)
    print("LogPath: %s" % logPath)

def executeTests(count):
    global combinedPath
    global logPath
    global dateandtime

    print("Repetition Cycle:" , (count + 1))
    currLogPath = "Rep" + str(count + 1);
    combinedPath=os.path.join(logPath, currLogPath)
    print("LogPath: %s" % logPath)
    print("Combined Path:", combinedPath)

    # remove dir if it already exists, Then create it
    if os.path.isdir(combinedPath):
        shutil.rmtree(combinedPath, ignore_errors=False, onerror=None)
    os.mkdir(combinedPath)

    # Use below command for running tests in the background (USB disconnected)
    subprocess.call(["adb", "-s", str(adbDevice), "shell", "logcat -c"])
    subprocess.call(["adb", "-s", str(adbDevice), "shell", "rm", "/data/nativetest64/VtsHalBluetooth_TransportTest/tstlog.txt"])
    subprocess.call(["adb", "-s", str(adbDevice), "shell", "rm", "/data/nativetest64/VtsHalBluetooth_TransportTest/logcat.txt"])
    subprocess.call(["adb", "-s", str(adbDevice), "shell", "nohup" , "logcat", "-vthreadtime -b kernel -b main -b system",
                         " > /data/nativetest64/VtsHalBluetooth_TransportTest/logcat.txt 2>&1 < /dev/null &"])
    if usbDisc:
        subprocess.call(["adb", "-s", str(adbDevice), "shell", "nohup" , "/data/nativetest64/VtsHalBluetooth_TransportTest/BT-Transport-Tests.sh",
                         str(loop), str(testfilter), str(maxappsleeptimer), str(minappsleeptimer), str(maxiterations),
                         str(maxmsgsperiteration), str(logcommands),
                         str(logevents), str(logdata), str(consolelog),
                         " > /data/nativetest64/VtsHalBluetooth_TransportTest/tstlog.txt 2>&1 < /dev/null &"])
        print("Disconnecting USB")
        tac.DisconnectUsb()
        time.sleep(testWaitTime)
        print("Reconnecting USB")
        tac.ConnectUsb()
        time.sleep(3)
        subprocess.call(["adb", "-s", str(adbDevice), "root"])
        time.sleep(3)
        subprocess.call(["adb", "-s", str(adbDevice), "remount"])
    else:
        subprocess.call(["adb", "-s", str(adbDevice), "shell", "/data/nativetest64/VtsHalBluetooth_TransportTest/BT-Transport-Tests.sh",
                         str(loop), str(testfilter), str(maxappsleeptimer), str(minappsleeptimer), str(maxiterations),
                         str(maxmsgsperiteration), str(logcommands),
                         str(logevents), str(logdata), str(consolelog),
                         " > /data/nativetest64/VtsHalBluetooth_TransportTest/tstlog.txt 2>&1"])

def collectLogs(count):
    subprocess.call(["adb", "-s", str(adbDevice), "shell", "pkill", "logcat"])
    subprocess.call(["adb", "-s", str(adbDevice), "pull", "/data/nativetest64/VtsHalBluetooth_TransportTest/tstlog.txt", combinedPath])
    subprocess.call(["adb", "-s", str(adbDevice), "pull", "/data/tombstones", combinedPath])
    subprocess.call(["adb", "-s", str(adbDevice), "pull", "/data/vendor/ssrdump", combinedPath])
    subprocess.call(["adb", "-s", str(adbDevice), "pull", "/data/nativetest64/VtsHalBluetooth_TransportTest/logcat.txt", combinedPath])
    subprocess.call(["adb", "-s", str(adbDevice), "shell", "rm -rf /data/vendor/ssrdump/*"])
    subprocess.call(["adb", "-s", str(adbDevice), "shell", "rm -rf /data/tombstones/*"])

    filename = os.path.join(combinedPath, "tstlog.txt")
    with open(filename) as f:
        while 'Global test environment tear-down' not in f.readline():
           continue
        testRepBody = f.readlines()
        combTestLogFile.write("Repetion: %d\n" % (count + 1))
        combTestLogFile.write(' '.join(testRepBody))
        if  '[  FAILED  ]' in str(testRepBody):
            testFailed = True
            testResult = "Some Tests Failed"

    tombstoneDir=os.path.join(combinedPath, "tombstones")
    if not os.listdir(tombstoneDir):
        try:
            os.rmdir(tombstoneDir)
        except OSError:
            pass

    ssrDumpDir=os.path.join(combinedPath, "ssrdump")
    if not os.listdir(ssrDumpDir):
        try:
            os.rmdir(ssrDumpDir)
        except OSError:
            pass

def emailResults():
    # update results in log file too
    combTestLogFile.write("\n");

    # Add unique tags in email subject
    hostName = os.getenv('COMPUTERNAME')
    emailSubject = hostName + ":" + dateandtime + ":" + testResult
    combTestLogFile.write(emailSubject);

    combTestLogFile.close()

    if (mailResults):
        msg = MIMEMultipart()
        msg['Subject'] = emailSubject
        msg['From'] = mailSender
        msg['To'] = mailReceivers

        with open(combTestLogFileName, 'r') as f:
            part = MIMEApplication(f.read(), Name=basename(combTestLogFileName))
        part['Content-Disposition'] = 'attachment; filename="{}"'.format(basename(combTestLogFileName))
        msg.attach(part)
        msg.attach(MIMEText(emailSubject, 'plain'))

        try:
            server = smtplib.SMTP(smtpServer, 2525)
            server.sendmail(mailSender, mailReceivers, msg.as_string())
            print("Successfully sent email")
        finally:
            pass

def cleanup():
    # Change SELinux mode to original
    if (selinuxmode != b'Permissive'):
        subprocess.call(["adb", "shell", "setenforce", "1"])
        print("Changed back SELinux policy to Enforcing")

    print("Turning BT ON")
    subprocess.call("adb shell service call bluetooth_manager 6")

# Application entry point
__name__ = "__main__"
main()
