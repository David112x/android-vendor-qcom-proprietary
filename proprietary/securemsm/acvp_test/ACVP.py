'''
ACVP.py version 1.0.2

GENERAL DESCRIPTION
   Automation script for ACVP testing for TZ, and GPCE

Copyright (c) 2019 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
'''

import os
import sys
import threading
import subprocess
import shutil
import json
from datetime import datetime
from optparse import OptionParser
from distutils.version import StrictVersion

VERSION = "1.0.2"

USAGE = "Script Usage:\n\
-c <Crypto> -r <Engine> -o <Output Directory> -t <Input Directory> \n\
-e <Expected Result Directory [optional]> -l [optional] -v [optional] -p [optional]\n\
-l: Produce logging output, -v: Print Version, -p: Push Test Binary\n\
Crypto: All, AES, TDES, CMAC, HMAC, XTS, SHA, CCM, GCM, KDF, RSA, ECC\n\
Engine: All, TZSW64, TZSW32, GPCE, TZ_KERNEL\n"

CRYPTO_TYPES = ["AES", "TDES", "CMAC", "HMAC", "XTS", "SHA", "CCM", "GCM", "KDF", "RSA", "ECC"]
ENGINES = ["TZSW32", "TZSW64", "TZ_KERNEL", "GPCE"]
NONDETERMINISTIC_TESTS = [
    "RSA-KEYGEN", "RSA-SIGGEN", "RSA-SIGGEN-PSS",
    "ECDSA-KEYGEN", "ECDSA-SIGGEN", "ECDSA-SIGGEN-COMPONENT",
    "KAS-ECC-P224", "KAS-ECC-P256", "KAS-ECC-P384", "KAS-ECC-P521",
]

ENGINES_USING_FIPSTESTAPP = ["TZSW32", "TZ_KERNEL", "GPCE"]
ENGINES_USING_FIPSTESTAPP64 = ["TZSW64"]

PREFIX_TO_CRYPTO_MAP = {
    'AES-CBC' : 'AES',
    'AES-ECB' : 'AES',
    'AES-CTR' : 'AES',
    'AES-CCM' : 'CCM',
    'AES-GCM' : 'GCM',
    'AES-GMAC' : 'GCM',
    'AES-XTS' : 'AES',
    'ECDSA' : 'ECC',
    'HMAC' : 'HMAC',
    'RSA' : 'RSA',
    'KAS-ECC' : 'ECC',
    'SHA' : 'SHA',
    'TDES' : 'TDES',
    'KDF' : 'KDF',
    'CMAC' : 'CMAC'
}

ENGINE_TO_DIRECTORY_MAP = {
    "TZSW32" : "TZ_SW/32bit",
    "TZSW64" : "TZ_SW/64bit",
    "TZ_KERNEL" : "TZ_SW/kernel",
    "GPCE" : "GPCE"
}

COMBO_ENGINES_MAP = {
    "TZSW" : ["TZSW32", "TZSW64"],
    "TZ" : ["TZSW32", "TZSW64", "TZ_KERNEL"]
}

SUPPORTED_CRYPTO_MAP = {
    "TZSW32" : ["AES", "CCM", "ECC", "GCM", "HMAC", "RSA", "SHA", "TDES", "XTS"],
    "TZSW64" : ["AES", "CCM", "ECC", "GCM", "HMAC", "RSA", "SHA", "TDES", "XTS"],
    "TZ_KERNEL" : ["KDF"],
    "TZHW" : ["AES", "CMAC", "HMAC", "SHA", "TDES", "XTS", "CCM"],
    "GPCE" : ["AES", "CMAC", "HMAC", "SHA", "TDES", "XTS", "CCM"],
}

DEVICE_BINARY_NAME = "acvp_test"
DEVICE_BINARY_PATH = "/vendor/bin/acvp_test"
DEVICE_DATA_PATH = "/data/acvp"

REPORT_FOLDER_NAME = "diff_report"
REPORT_FILE_NAME = "diff_report.txt"
CONSOLIDATED_FILE_NAME = "test_report.txt"

LOG_FOLDER_NAME = "logs"
LOG_FILE_NAME = "testvector-logs.txt"

REQUEST_FILE_NAME = "testvector-request.json"
RESPONSE_FILE_NAME = "testvector-response.json"
EXPECTED_FILE_NAME = "testvector-expected.json"

DEFAULT_OUTPUT_DIRECTORY_NAME = "acvp_results"

SUCCESS_LOG_MESSAGE = "ACVP test run succeeded"
ERROR_LOG_MESSAGE = "ACVP test run failed"

FIPSTESTAPP_PATH = "/vendor/firmware_mnt/image/fipstestapp.b00"
FIPSTESTAPP64_PATH = "/vendor/firmware_mnt/image/fipstestapp64.b00"

def check_for_errors(log_file, command):
    '''Check command's log file to see if there are errors'''
    found_success_message = False
    found_error_message = False
    with open(log_file, 'r') as log:
        for line in log:
            if SUCCESS_LOG_MESSAGE in line:
                found_success_message = True
            if ERROR_LOG_MESSAGE in line:
                found_error_message = True
    if found_error_message:
        print("ERROR: Error message found in output of command {0}".format(command))
        print("Look in log file {0} for error information.".format(os.path.abspath(log_file)))
    elif not found_success_message:
        print("WARNING: Success message not found in output of command. {0}".format(command))
        print("Look in log file {0} to ensure command ran and completed successfully.".format(os.path.abspath(log_file)))


def run_shell_command(command, log_file=None):
    '''Runs a shell command and returns its output.'''
    try:
        with open(log_file if log_file else os.devnull, 'w+') as output:
            subprocess.call(command, shell=True, stdout=output, stderr=output)
    except subprocess.CalledProcessError as error:
        output.write("Error: \"{0}\"\n".format(error.output))
    if log_file:
        check_for_errors(log_file, command)


def mkdir_if_not_exist(directory):
    '''Creates a directory at the input path location if it does not exist'''
    if not os.path.exists(directory):
        try:
            os.makedirs(directory)
        except WindowsError:
            print("Could not make directory {0}. Please close all open files in that directory".format(directory))


def parse_command_line_options():
    '''Parse command line options'''
    parser = OptionParser(usage=USAGE, version="ACVP.py {0}".format(VERSION))
    parser.add_option("-c", "--crypto", dest="crypto", action="append", help="Test to run")
    parser.add_option("-e", "--exp_result", dest="expected_dir", help="Expected result path")
    parser.add_option("-o", "--output", dest="resp_dir", help="Output Directory")
    parser.add_option("-t", "--test_vec", dest="req_dir", help="Test vector path")
    parser.add_option("-r", "--engine", dest="engine", action="append", help="Engine")
    parser.add_option("-l", "--logs", dest="logs", action='store_true', help="Save test results to Log Files. Recommended if running monte carlo tests.")
    parser.add_option("-m", "--mct", dest="ignore_monte_carlo", action='store_true', help="Skip Monte Carlo Tests")
    parser.add_option("-k", "--engineering", dest="eng_build", action='store_true', help="Engineering build required to run this option")
    parser.add_option("-v", "--vers", dest="version", action='store_true', help="Print Version")
    parser.add_option("-p", "--push", dest="push", action='store_true', help="Push acvp_test binary to device")
    (options, _) = parser.parse_args()
    return options


def validate_options(options):
    '''Validate command line options'''
    if not options.resp_dir:
        default_output_directory = os.path.join(os.getcwd(), DEFAULT_OUTPUT_DIRECTORY_NAME)
        print("Missing output directory option. Using {0} as output directory".format(default_output_directory))
        options.resp_dir = default_output_directory
        mkdir_if_not_exist(default_output_directory)
    if not options.crypto:
        print("Missing crypto selection")
        print(USAGE)
        exit(1)
    if not options.engine:
        print("Missing engine selection")
        print(USAGE)
        exit(1)
    if not os.path.isdir(options.req_dir):
        print("Input directory does not exist")
        print(USAGE)
        exit(1)
    for engine in options.engine:
        if engine not in ENGINES and engine.upper() != "ALL" and engine not in COMBO_ENGINES_MAP:
            print("Engine: {0} not supported".format(engine))
            print(USAGE)
            exit(1)
    for crypto in options.crypto:
        if crypto not in CRYPTO_TYPES and crypto.upper() != "ALL":
            print("Crypto: {0} not supported".format(crypto))
            print(USAGE)
            exit(1)


def get_chipset_name():
    '''Gets the name of the target chipset'''
    return subprocess.check_output("adb shell getprop 'ro.board.platform'", shell=True)[:-2]


def find_acvp_test_binary():
    '''Find the acvp_test binary in relation to the ACVP.py script'''
    script_dir = os.path.dirname(os.path.abspath(__file__))
    chipset_name = get_chipset_name().decode("utf-8")
    target = ""
    if os.path.join("vendor", "qcom", "proprietary", "securemsm", "acvp_test") in script_dir:
        target = os.path.join(script_dir, "../../../../../out/target/product/{0}/vendor/bin/{1}".format(chipset_name, DEVICE_BINARY_NAME))
    if os.path.join("out", "host", "linux-x86", "bin") in script_dir:
        target = os.path.join(script_dir, "../../../target/product/{0}/vendor/bin/{1}".format(chipset_name, DEVICE_BINARY_NAME))
    if os.path.isfile(target):
        return target
    return None


def prepare_device(options):
    '''Root device, remount, remove test vectors currently on device, and push testing binary to device'''
    run_shell_command("adb root")
    run_shell_command("adb remount")
    run_shell_command("adb shell rm -rf {0}".format(DEVICE_DATA_PATH))
    run_shell_command("adb shell mkdir {0}".format(DEVICE_DATA_PATH))

    if options.push:
        path = find_acvp_test_binary()
        if path:
            run_shell_command("adb push {0} {1}".format(path, DEVICE_BINARY_PATH))
            run_shell_command("adb shell chmod 777 {0}".format(DEVICE_BINARY_PATH))


def parse_engines_list(engines):
    '''Return a list of selected engines'''
    result = []

    if "ALL" in engines:
        return ENGINES

    for engine in engines:
        if engine in COMBO_ENGINES_MAP:
            for eng in COMBO_ENGINES_MAP[engine]:
                result.append(eng)
        else:
            result.append(engine)
    return result


def parse_options_lists(crypto_list, engine_list):
    '''Get lists of engines and cryptos to be tested'''
    cryptos = [c.upper() for c in crypto_list]
    engines = [e.upper() for e in engine_list]

    if "ALL" in cryptos:
        cryptos = CRYPTO_TYPES

    return cryptos, parse_engines_list(engines)


def rmdir_if_exists(directory):
    '''Remove a directory if it exists'''
    if os.path.isdir(directory):
        try:
            shutil.rmtree(directory)
        except WindowsError:
            print("Could not clean directory {0}. Please close all open files in that directory".format(directory))


def folder_has_targeted_crypto(folder, cryptos):
    '''Check if folder has crypto which we are testing'''
    for prefix in PREFIX_TO_CRYPTO_MAP:
        if folder.startswith(prefix) and PREFIX_TO_CRYPTO_MAP[prefix] in cryptos:
            return True
    return False


def initialize_logging(options, log_dir, eng_dir, folder):
    '''Create logging directory and return logging file name. Return None if logging is not enabled'''
    if options.logs:
        mkdir_if_not_exist(log_dir)
        mkdir_if_not_exist(os.path.join(log_dir, eng_dir))
        mkdir_if_not_exist(os.path.join(log_dir, eng_dir, folder))
        return os.path.join(log_dir, eng_dir, folder, LOG_FILE_NAME)
    return None


def log_diff(expected, result, key, tgid, tcid, report, is_str):
    '''Log a difference between 2 keys'''
    report.write("\r\n")
    report.write("Difference in Test Group {0}, Test Case {1} for key <{2}>\r\n".format(tgid, tcid, key))
    expected_out = expected[key].upper() if is_str else expected[key]
    result_out = result[key].upper() if is_str else result[key]
    report.write("Expected: {0}\r\n".format(expected_out))
    report.write("Result: {0}\r\n".format(result_out))


def diff_mct_test_cases(result, expected_result, report, tgid, tcid, ignore_monte_carlo):
    '''Diff 2 MCT test cases based on their resultsArray'''
    if "resultsArray" not in result:
        if ignore_monte_carlo:
            return 0
        else:
            report.write("Missing Monte Carlo results in Test Group {0}", tcid)
            return 1

    differences = 0
    for (res, exp_res) in zip(result["resultsArray"], expected_result["resultsArray"]):
        for key in exp_res:
            if key not in res:
                differences += 1
                report.write("\r\n")
                report.write("Missing key {0} in Test Group {1}, Test Case {2}\r\n".format(key, tgid, tcid))
            if res[key].upper() != exp_res[key].upper():
                differences += 1
                log_diff(res, exp_res, key, tgid, tcid, report, True)
    return differences


def is_string(value):
    '''Check if value is a string. Compatible with python 2 and python 3'''
    if sys.version_info.major >= 3:
        return isinstance(value, str)
    return isinstance(value, unicode) or isinstance(value, str)


def diff_standard_test_cases(result, expected_result, report, tgid, tcid, deterministic):
    '''Diff 2 test cases'''
    differences = 0
    for key in expected_result:
        if key not in result:
            differences += 1
            report.write("\r\n")
            report.write("Missing key {0} in Test Group {1}, Test Case {2}\r\n".format(key, tgid, tcid))
        elif (isinstance(expected_result[key], int) or isinstance(expected_result[key], bool)) and result[key] != expected_result[key]:
            differences += 1
            log_diff(expected_result, result, key, tgid, tcid, report, False)
        elif is_string(expected_result[key]) and result[key].upper() != expected_result[key].upper():
            if deterministic:
                differences += 1
                log_diff(expected_result, result, key, tgid, tcid, report, True)
    return differences


def diff_json_files(resp, expected, report, deterministic, folder, ignore_monte_carlo):
    '''Diff 2 JSON files'''
    differences = 0

    try:
        resp_dict = json.load(resp)
    except ValueError as error:
        report.write("Response file is not a valid json string\r\n")
        report.write("{0}\r\n".format(error))
        return 1
    expected_dict = json.load(expected)

    resp_test_groups = sorted(resp_dict[1]["testGroups"], key=lambda t : t['tgId'])
    expected_test_groups = sorted(expected_dict[1]["testGroups"], key=lambda t : t['tgId'])

    if len(resp_test_groups) != len(expected_test_groups):
        report.write("Number of test groups does not match. {0} present, expected {1}\r\n".format(len(resp_test_groups), len(expected_test_groups)))
        return differences + 1

    for (resp_group, expected_group) in zip(resp_test_groups, expected_test_groups):
        if len(resp_group) != len(expected_group):
            report.write("Number of tests in group {0} does not match. {1} present, expected {2}\r\n".format(resp_group["tgId"], len(resp_group), len(expected_group)))
            return differences + 1

        resp_test_cases = sorted(resp_group['tests'], key=lambda t : t['tcId'])
        expected_test_cases = sorted(expected_group['tests'], key=lambda t : t['tcId'])

        for (result, expected_result) in zip(resp_test_cases, expected_test_cases):
            if "resultsArray" in expected_result and not folder.startswith("RSADP"): # MCT test case
                differences += diff_mct_test_cases(result, expected_result, report, resp_group["tgId"], result["tcId"], ignore_monte_carlo)
            else:
                differences += diff_standard_test_cases(result, expected_result, report, resp_group["tgId"], result["tcId"], deterministic)

    return differences


def determine_expected_file_name(expected_dir, folder):
    '''Determine whether the response files are samples or validated results'''
    sample_file_name = os.path.join(expected_dir, folder, EXPECTED_FILE_NAME)
    return sample_file_name if os.path.isfile(sample_file_name) else os.path.join(expected_dir, folder, RESPONSE_FILE_NAME)


def generate_diff_report(engine, options, cryptos, consolidated_report, ignore_monte_carlo):
    '''Generate a diff report of all results compared to their expected results'''
    eng_dir = ENGINE_TO_DIRECTORY_MAP[engine]
    resp_dir = os.path.join(options.resp_dir, eng_dir)
    expected_dir = os.path.join(options.expected_dir, eng_dir)
    diff_report_dir = os.path.join(options.resp_dir, REPORT_FOLDER_NAME, eng_dir)
    differences = 0

    mkdir_if_not_exist(diff_report_dir)

    if not os.path.isdir(resp_dir):
        print("ERROR: Missing response directory {0}. Skipping diff for these results".format(os.path.realpath(resp_dir)))
        return 1

    folders = [f for f in os.listdir(resp_dir) if folder_has_targeted_crypto(f, cryptos)]
    for folder in folders:
        print("Generating diff report for {0}".format(folder))
        rmdir_if_exists(os.path.join(diff_report_dir, folder))
        mkdir_if_not_exist(os.path.join(diff_report_dir, folder))
        resp_file = os.path.join(resp_dir, folder, RESPONSE_FILE_NAME)
        expected_file = determine_expected_file_name(expected_dir, folder)

        diff_report_file = os.path.join(diff_report_dir, folder, REPORT_FILE_NAME)
        deterministic = folder not in NONDETERMINISTIC_TESTS

        with open(resp_file, "r") as resp, open(expected_file, "r") as expected, open(diff_report_file, "w+") as report:
            report.write("Diff of ACVP testing files\r\n")
            report.write("Response file: {0}\r\n".format(os.path.realpath(resp_file)))
            report.write("Expected file: {0}\r\n".format(os.path.realpath(expected_file)))

            file_diffs = diff_json_files(resp, expected, report, deterministic, folder, ignore_monte_carlo)
            if file_diffs > 0:
                print("Differences found in {0}".format(folder))
                consolidated_report.write("{0} : FAILED. View differences in {1}\r\n".format(folder, os.path.realpath(diff_report_file)))
            else:
                consolidated_report.write("{0} : PASSED.\r\n".format(folder))
            differences += file_diffs

    return differences


def check_for_binary(name, path):
    '''Check if binary is on device'''
    try:
        output = subprocess.check_output("adb shell stat {0}".format(path), shell=True)
    except subprocess.CalledProcessError as error:
        print("ERROR: {0} binary is missing from the device. Expected to find at {1}".format(name, path))
        exit(1)


def sanity_check_device(options, engines):
    '''Make sure device is connected and has all of the right testing binaries'''
    try:
        output = subprocess.check_output("adb get-state", shell=True)
    except subprocess.CalledProcessError as error:
        print("ERROR: Could not find device connected through adb")
        exit(1)

    prepare_device(options)
    check_for_binary("acvp_test", DEVICE_BINARY_PATH)

    if len(set(ENGINES_USING_FIPSTESTAPP).intersection(set(engines))) != 0:
        check_for_binary("fipstestapp", FIPSTESTAPP_PATH)

    if len(set(ENGINES_USING_FIPSTESTAPP64).intersection(set(engines))) != 0:
        check_for_binary("fipstestapp64", FIPSTESTAPP64_PATH)


def run_acvp_tests(engine, cryptos, options):
    '''Run all selected ACVP tests'''
    mct_arg = 0 if options.ignore_monte_carlo else 1
    eng_dir = ENGINE_TO_DIRECTORY_MAP[engine]
    req_dir = os.path.join(options.req_dir, eng_dir)
    resp_dir = os.path.join(options.resp_dir, eng_dir)
    log_dir = os.path.join(options.resp_dir, LOG_FOLDER_NAME)

    run_shell_command("adb shell rm -rf {0}/{1}".format(DEVICE_DATA_PATH, eng_dir))
    run_shell_command("adb shell mkdir -p {0}/{1}".format(DEVICE_DATA_PATH, eng_dir))

    folders = [(f, p) for f in os.listdir(req_dir) for p in PREFIX_TO_CRYPTO_MAP if f.startswith(p) and PREFIX_TO_CRYPTO_MAP[p] in cryptos]

    if not folders:
        print("ERROR: did not find any tests.")
        print("Double check that the selected crypto is supported by the crypto engine and that the request directory has test cases for that crypto.")
        print("Tests were expected to be found in {0}".format(req_dir))

    for (folder, prefix) in folders:
        run_shell_command("adb push {0} {1}/{2}".format(os.path.join(req_dir, folder), DEVICE_DATA_PATH, eng_dir, True))
        mkdir_if_not_exist(os.path.join(resp_dir))
        log_file = initialize_logging(options, log_dir, eng_dir, folder)
        crypto = PREFIX_TO_CRYPTO_MAP[prefix]
        req_file = "{0}/{1}/{2}/{3}".format(DEVICE_DATA_PATH, eng_dir, folder, REQUEST_FILE_NAME)
        resp_file = "{0}/{1}/{2}/{3}".format(DEVICE_DATA_PATH, eng_dir, folder, RESPONSE_FILE_NAME)

        print("Running crypto tests for {0}".format(folder))
        command = "adb shell {0} {1} {2} {3} {4} {5}".format(DEVICE_BINARY_PATH, engine, req_file, resp_file, crypto, mct_arg)
        run_shell_command(command, log_file)

        rmdir_if_exists(os.path.join(resp_dir, folder))
        run_shell_command("adb pull {0}/{1}/{2} {3}".format(DEVICE_DATA_PATH, eng_dir, folder, os.path.join(resp_dir, folder)))


def check_adb_version():
    '''Check that adb version is supported. Warn user if lower than supported version'''
    adb_version = subprocess.check_output("adb version", shell=True)
    version = adb_version.split()[4].decode("utf-8")
    if (StrictVersion(version) < StrictVersion("1.0.36")):
        print("WARNING: adb version {0} detected! Lowest version supported is 1.0.36.".format(version))
        print("If you encounter errors with this script, please install adb version 1.0.36 or higher and try again.")


def main():
    '''Entry point for automation script'''
    options = parse_command_line_options()

    if options.version:
        print("ACVP.py version {0}".format(VERSION))

    check_adb_version()

    validate_options(options)

    (cryptos, engines) = parse_options_lists(options.crypto, options.engine)

    mkdir_if_not_exist(options.resp_dir)
    mkdir_if_not_exist(os.path.join(options.resp_dir, REPORT_FOLDER_NAME))
    sanity_check_device(options, engines)

    with open(os.path.join(options.resp_dir, REPORT_FOLDER_NAME, CONSOLIDATED_FILE_NAME), 'w+') as consolidated_report:
        for engine in engines:
            print("\n=================================")
            print("Beginning tests for {0}".format(engine))
            print("=================================\n")

            run_acvp_tests(engine, cryptos, options)

            print("\n=================================")
            print("Finished tests for {0}".format(engine))
            print("=================================\n")

            if options.expected_dir:
                print("=================================")
                print("Generating diff report for {0}".format(engine))
                print("=================================\n")

                differences = generate_diff_report(engine, options, cryptos, consolidated_report, options.ignore_monte_carlo)

                print("\n=================================")
                print("Finished generating diff report for {0}".format(engine))
                print("{0} semantically relevant differences detected".format(differences))
                if differences > 0:
                    report_folder = os.path.realpath(os.path.join(options.resp_dir, REPORT_FOLDER_NAME, ENGINE_TO_DIRECTORY_MAP[engine]))
                    print("Reports are in {0}".format(report_folder))
                print("=================================\n")


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print("Keyboard Interrupt Received. Exiting!")
        print(sys.exc_info()[1])
        sys.exit(1)
