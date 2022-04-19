"""
===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================
"""

from __future__ import print_function
import os, errno
import time
import re
import sys
from config import blacklist,ignore_paths

violation_list = []
timestamp_file          = "out/restriction_checker/checker_timestamp"
checker_violations_file = "out/restriction_checker/checker_violations_file"
log_file                = "out/restriction_checker/checker_log"
log_file_fd = None

TAG   = "Restriction_Checker"
DEBUG = False
TEST  = False # Used to test restriction checker

#
# Category of violations:
#
# TARGET_NAME_VIOLATION:
#  TARGET_NAME_VIOLATION corresponds to lunch/board name (msmexample) usage violation in makefile.
#
# PATH_VIOLATION:
#  Path referencing should not be done through $(MSMEXAMPLE), since, when we rename the lunch,
#  path will be broken. So, occurances like xx/$(MSMEXAMPLE)/xx is a PATH_VIOLATION.
#
TARGET_NAME_VIOLATION = 1
PATH_VIOLATION        = 2


# TARGET_NAME_VIOLATION regex
regex_left_type1  = r'(([^A-Za-z0-9$][^/A-Za-z0-9(_-])|([^$][,=\() ]))'
regex_right_type1 = r'[^./A-Za-z0-9_-]'

# PATH_VIOLATION regex
regex_left_type2  = r'([/][A-Za-z0-9_-]*)'
regex_right_type2 = r'([A-Za-z0-9_-]*[/])'


def log(msg, console, log_to_file):
    if console or DEBUG:
        print("["+TAG+"]"+ " " +msg)
    if log_to_file and log_file_fd is not None:
        log_file_fd.write("["+TAG+"]"+ " " +msg+"\n")

def validate_line(line, file):
    if line is None:
        return
    original_line = line
    if "@bypass_restriction_checker" in original_line: return
    # Remove commented lines
    line = line.split('#')[0]
    # Add extra space at the beginning for regex to work well (to detect violations
    # starting from 0th char.)
    line = "  " + line

    for elem in blacklist.keys():
        # Check for TARGET_NAME_VIOLATIONs
        if re.search(regex_left_type1 + elem + regex_right_type1, line):
            violation_list.append((elem,original_line,blacklist[elem],file,TARGET_NAME_VIOLATION))

        # Check for PATH_VIOLATIONs
        if re.search(regex_left_type2 + re.escape(blacklist[elem]), line) or \
             re.search(re.escape(blacklist[elem]) + regex_right_type2, line):
            violation_list.append((elem,original_line,blacklist[elem],file,PATH_VIOLATION))

def validate_file(file, dir=None):
    if file is None:
        return
    if dir is None:
        try:
            file_path = open(str(file), "r")
        except:
            print("Failed to Open: "+str(file))
            sys.exit(1)
    else:
        try:
            file_path = open(dir + "/" + str(file), "r")
        except:
            print("Failed to Open: "+str(file))
            sys.exit(1)
    for line in file_path:
        validate_line(line, str(file))

def call_tests(tree_root, lines):

# This is to test if this tool is working well.
# Input : A file with mgrep'ed occurances of msmexample or MSMEXAMPLE or both, run on
# the whole workspace, like:
# <path/to/tree_root/>$ mgrep -i "msmsteppe" | tee mgrep_output
# This test displays the remaining/left out ones for us to cross-check (if the
# restriction_checker is not missing any violation nor throwing any false positives).

    log("Starting Tests..", True, True)
    try:
        mgrep_output_file = open(tree_root + "/mgrep_output", 'r')
    except:
        log("Provide input mgrep file at <tree_root>/mgrep_output.", True, True)
        log("Eg: <path/to/tree_root/>$ mgrep -i \"msmsteppe\" | tee mgrep_output", True, True)
        sys.exit(1)

    log("Following lines are not flagged as violations by restriction_checker:"+ \
            "(manually check if it's expected)", True, True)
    count = 0
    mgrep_count = 0
    for line in mgrep_output_file:
       mgrep_count += 1
       a = line.find(':')
       b = line[a+1:].find(':')
       if line[a+b+2:].strip('\n') not in lines:
            count += 1
            log("line:"+line, True, True)
    log("Total occurances(from mgrep) = "+str(mgrep_count), True, True)
    log("Non violation Occurances Count = "+str(count), True, True)

def main(argv):
    start_time = time.time()
    scan_dirs = []
    ignore_full_paths = []
    tree_root = sys.argv[1]

    full_tree_scan = False
    for x in sys.argv[2:]:
        scan_dirs.append(x)
        if x in tree_root:
            full_tree_scan = True

    timestamp_file_path =  tree_root +"/"+ timestamp_file
    checker_violations_file_path = tree_root + "/" + checker_violations_file
    checker_violations_file_exists = os.path.exists(checker_violations_file_path)
    log_file_path = tree_root +"/"+ log_file

    try:
        os.makedirs(os.path.dirname(timestamp_file_path))
    except OSError as exc:
        if exc.errno != errno.EEXIST:
            print("Exception Occured while trying to create Dirs")
            raise

    # Setup logging
    global log_file_fd
    try:
        log_file_fd = open(log_file_path, "w")
    except:
        print("Failed to open log file")
        sys.exit(1)

    log("********************************************************************", False, True)
    log("Starting Up Restriction Checker! (Please visit go/restriction_checker for more details)", True, True)
    log("Tree_root: "+tree_root, True, True)

    log("Input Directories for scanning:", True, True)
    for dir in scan_dirs:
        log(dir, True, True)

    # Get last scanned time
    if os.path.exists(timestamp_file_path):
        try:
            fd = open(timestamp_file_path, "r")
            last_scan_time = float(fd.readline().replace("\n",""))
        except:
            print("Unable to open timestamp file, exiting")
            sys.exit(1)
        fd.close()
        log("TimeStamp file exists", False, True)
    else:
        last_scan_time = float("-inf")
        log("TimeStamp file doesn't exist", False, True)

    # Write present time to timestamp file only if it's a full scan
    if full_tree_scan:
        try:
            fd = open(timestamp_file_path, 'w')
            fd.write(str(time.time()))
        except:
            print("Unable to open timestamp file, exiting")
            sys.exit(1)
        fd.close()

    log("Last Scan time: "+ str(last_scan_time), False, True)
    log("Present time: "+ str(time.time()), False, True)

    # Get ignore paths
    log("Ignore directories:", False, True)
    for i in ignore_paths:
        ignore_full_paths.append(tree_root + "/" + i)
        log(tree_root + "/" + i, False, True)

    # Start scanning
    for dir in scan_dirs:
        for root, dirs, filenames in os.walk(dir):
            if root in ignore_full_paths:
                log("Ignoring Scan of dir: "+root, False, True)
                dirs[:] = []
                filenames[:] = []
            [dirs.remove(d) for d in list(dirs) if d in ".git"]
            for filename in filenames:
                if filename is not None:
                    file_path = os.path.join(root,filename)
                    if file_path.strip('\n') in ignore_full_paths:
                        log("Ignoring Scan of file: "+file_path.strip('\n'), False, True)
                        continue
                    if file_path.endswith(".mk"):
                        # Printing the files being scanned in the same line,
                        # so that user knows .mks are parsed.
                        print_value = "Scanning: "+file_path.strip('\n')+"\r"
                        print(print_value, end="")
                        sys.stdout.flush()
                        print(len(print_value)*" "+"\r",end="")

                        if os.stat(file_path).st_mtime > last_scan_time or \
                            not checker_violations_file_exists:
                            log("Scanning: "+file_path, False, True)
                            validate_file(file_path)

    old_violations = {}
    if checker_violations_file_exists:
        try:
            checker_violations_file_fd = open(checker_violations_file_path, 'r')
        except:
            print("Failed to open violations file, exiting")
            sys.exit(1)

        for file_name in checker_violations_file_fd:

            if not os.path.exists(file_name.strip('\n')):
                # File could have been renamed, save it anyway to avoid missing it in
                # incremental builds(as we rely on timestamp),so that we check it
                # when it's renamed back without content/timestamp modification.
                old_violations[file_name.strip('\n')] = 1
                continue

            # Check only if old violation file is in our scan dirs.
            check_file = False
            for dir in scan_dirs:
                if dir in file_name:
                    check_file = True

            # Save old violations, which were outside of present scan dirs
            if not check_file:
                old_violations[file_name.strip('\n')] = 1

            # Check if it was already scanned by this point
            for violation in violation_list:
                if file_name.strip('\n') in violation[3]:
                    check_file = False

            if check_file == True:
                log("Checking old-violation file: "+file_name, False, True)
                validate_file(file_name.strip('\n'))
        checker_violations_file_fd.close()

    if TEST:
        lines = []

    # Print all the violations.
    if len(violation_list) > 0:
        log("***************************ERROR**************************************", True, True)

    num = 0
    for violation in violation_list:
        log("Adding new violation"+violation[3], False, True)
        num +=1
        log("Violation - " + str(num), True, True)
        if violation[4] == TARGET_NAME_VIOLATION:
            log("Wrong Usage of \""+violation[0]+"\" detected in .mk file." + \
                    " Use "+violation[2]+" instead.", True, True)
        elif violation[4] == PATH_VIOLATION:
            log("Wrong Usage of \""+violation[2]+"\" detected in .mk file." + \
                    " Do not use "+violation[2]+" for path referencing.", True, True)
        log("File: "+violation[3], True, True)
        log("Line: "+violation[1].strip("\n")+"\n", True, True)

        if TEST:
            lines.append(violation[1].strip("\n"))

    if len(violation_list) > 0:
        log("***************************ERROR**************************************", True, True)
        log("Please fix above violations.", True, True)
    else:
        log("No Violations detected!", True, True)

    log("Please visit go/restriction_checker for more details.", True, True)

    # Add back all violations found in the present scan to checker_violations_file
    try:
        checker_violations_file_fd = open(checker_violations_file_path, 'w')
    except:
        print("Unable to open violations file, exiting")
        sys.exit(1)

    remove_dups = {}
    for violation in violation_list:
        remove_dups[violation[3]] = 1
    for violation in remove_dups.keys():
        log("Adding new violator file:"+violation, False, True)
        checker_violations_file_fd.write(violation+"\n")

    # Add old violations
    for old_violation in old_violations.keys():
        log("Adding back old violator file:"+old_violation, False, True)
        checker_violations_file_fd.write(old_violation+"\n")
    checker_violations_file_fd.close()

    log("Time Taken:"+ str(time.time() - start_time), True, True)
    log("Exiting..", True, True)

    if TEST:
        call_tests(tree_root, lines)

    log_file_fd.close()

    if len(violation_list) > 0:
        sys.exit(1)
    else:
        sys.exit(0)

if __name__ == "__main__":
    main(sys.argv)
