#
# sensor_sdk.pl
#
# Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.

#!/usr/bin/perl

print "\n\n\n";
print "               ****************************************************************************\n";
print "               * Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved. *\n";
print "               * Qualcomm Technologies Proprietary and Confidential.                      *\n";
print "               *                                                                          *\n";
print "               *                                                                          *\n";
print "               *           C A M E R A  S E N S O R  D R I V E R  U T I L I T Y           *\n";
print "               *                                                                          *\n";
print "               ****************************************************************************\n";
print "\n\n\n";

print "*****************PREREQUISITES*****************\n\n";
print "   1) Install appropriate Android NDK installer from https://developer.android.com/tools/sdk/ndk/index.html\n";
print "   2) Install File::Copy::Recursive and Switch modules if not already installed.\n";
print "       In command prompt run these commands to install the modules.\n";
print "           cpan File::Copy::Recursive\n";
print "       cpan will be automaticaly installed with ActiveState perl.\n";
print "       Install latest ActiveState perl if cpan is not installed.\n\n";
print "***********************************************\n\n";

print "After installation please press Enter to continue: ";
<STDIN>;

use File::Copy;
use File::Copy::Recursive qw(dircopy);

my $valid = 0;
my $step = 1;
my @sdk_files = ("sensor_sdk_common.h", "sensor_lib.h", "flash_lib.h", "eeprom_util.h",
  "eeprom_lib.h", "debug_lib.h", "eeprom_util.h", "ois_driver.h", "chromatix_params.h", "actuator_driver.h");
$path_separator = "\\";
my $selected_index = 555;

print "Please provide the location of project : ";
$project_location = <STDIN>;
validate_project_path();

print "Please provide the location of mm-camerasdk : ";
$sdk_location = <STDIN>;

$valid = validate_sdk_path();
if ($valid == 0) {
  print "Please provide valid SDK path\n";
  exit;
}

# Step 1) Generate project directories
generate_project_directories();
print "\n*** PROJECT DIRECTORY GENERATION SUCCESSFUL ***\n";


# If this flag is set it will continue copying even if it fails for any particular file.
local $File::Copy::Recursive::SkipFlop = 1;

# Step 2) Copy include folder.
$project_include_path = $jni_path . $path_separator . "includes";
my $num_of_files_and_dirs = dircopy($sdk_include_path, $project_include_path);
if ($num_of_files_and_dirs == 0) {
  print "Includes directory copy failed\n";
  exit;
}

print "\n*** PROPRIETARY INCLUDE FILES COPY SUCCESSFUL ***\n";

do {
  chomp($selected_index);
    if ($selected_index == 0)
    {
      print "\n               ******************************************\n";
      print "                   THANK YOU FOR USING THE APPLICATION.  \n";
      print "               ******************************************\n";
      exit;
    }
    elsif ($selected_index == 1)
    {
      generate_library("Sensor", "mmcamera_", "_lib", 1);
    }
    elsif ($selected_index == 2)
    {
      generate_library("Actuator", "actuator_", "_actuator", 0);
    }
    else {}
  print_menu();
}while($selected_index = <STDIN>);

sub generate_library()
{
  $driver_type = shift;
  $lib_leading_name = shift;
  $file_trailing_name = shift;
  $is_chromatix_needed = shift;

  print "Please enter the $driver_type name: ";
  $driver_name = <STDIN>;
  chomp($driver_name);

  $driver_path = $jni_path . $path_separator . $driver_name;
  unless (-d $driver_path) {
    mkdir($driver_path) or die "Couldn't create $driver_path directory, $!";
  }

  $src_file_name = $driver_name . $file_trailing_name . ".c";
  $inc_file_name = $driver_name . $file_trailing_name . ".h";

  $src_file_path = $driver_path . $path_separator . $src_file_name;
  $inc_file_path = $driver_path . $path_separator . $inc_file_name;

  do {
    print "\nPlease copy $inc_file_name and $src_file_name
      to $driver_path and then press Enter: ";
      <STDIN>;
  } while(!((-f $src_file_path) && (-f $inc_file_path)));

  if ($is_chromatix_needed) {
    do {
      print "\nPlease enter the chromatix version : ";
      $chromatix_version = <STDIN>;

      if (!(-d $driver_path)) {
        $chromatix_include_path = $sdk_location . $path_separator . "sensor" .
          $path_separator . "includes" . $path_separator . $chromatix_version . $path_separator;
        print "Path not found : " . $chromatix_include_path . "\n";
      }
    } while(!(-d $driver_path));
  }

  $driver_name_uc = uc $driver_name;
  $makefile_data = $driver_name_uc . "_SENSOR_LIBS_PATH := \$(call my-dir)\n\n";
  $makefile_data .= "include \$(CLEAR_VARS)\n";
  $makefile_data .= "LOCAL_PATH             := \$(" . $driver_name_uc .
    "_SENSOR_LIBS_PATH)\n";
  $makefile_data .= "LOCAL_C_INCLUDES       := \$(LOCAL_PATH)/../includes/\n";
  $makefile_data .= "LOCAL_C_INCLUDES       += \$(LOCAL_PATH)/../includes/" . $chromatix_version . "\n";
  $makefile_data .= "LOCAL_C_INCLUDES       += $inc_file_name\n";
  $makefile_data .= "LOCAL_SRC_FILES        := $src_file_name\n";
  $makefile_data .= "LOCAL_MODULE           := lib$lib_leading_name$driver_name\n";
  $makefile_data .= "LOCAL_SHARED_LIBRARIES := libcutils\n\n";
  $makefile_data .= "include \$(BUILD_SHARED_LIBRARY)";

  $make_file_path = $driver_path . $path_separator . "Android.mk";
  open(my $fh, '>', $make_file_path) or
    die "Could not open file '$make_file_path' $!";
  print $fh $makefile_data;
  close $fh;

  my $lib_path = $project_location . $path_separator . "libs" .
    $path_separator . "armeabi" . $path_separator .
    "lib$lib_leading_name$driver_name" . ".so";

  do {
    print "\nPlease compile the $driver_name driver: \n\n";
    print "*****************COMPILATION STEPS*****************\n\n";
    print "   1) Open a new command prompt.\n";
    print "   2) run this command: set NDK_PROJECT_PATH=$project_location.\n";
    print "   3) run the ndk-build.cmd batch present in the root directory of installed Android NDK.\n";
    print "   4) This will compile the libraries and fix compilation issues if any.\n";
    print "***************************************************\n\n";

    print "Please press Enter to continue: ";
      <STDIN>;
  } while(!(-f $lib_path));

  print "\n*** DRIVER LIBRARY GENERATION SUCCESFULL : $lib_path ***\n\n";

  do {
    print "Do you want to load the library into device? [1/0] : ";
    $lib_load = <STDIN>;
  } while(!(($lib_load == 1) || ($lib_load == 0)));

  if ($lib_load == 1) {
    print "   1) Connect the device. Executing adb wait-for-device.\n";
    system("adb wait-for-device");
    print "   2) Executing adb root.\n";
    system("adb root");
    print "   3) Connect the device. Executing adb wait-for-device.\n";
    system("adb wait-for-device");
    print "   4) Executing adb remount\n";
    system("adb remount");

    print "   5) Pushing the library to device. Executing adb push $lib_path /system/vendor/lib.\n";
    system("adb push $lib_path /system/vendor/lib");
  }

  do {
    print "Do you want to update module config xml file? [1/0] : ";
    $update_config = <STDIN>;
  } while(!(($update_config == 1) || ($update_config == 0)));

  if ($update_config == 1) {
    print "   1) Connect the device. Executing adb wait-for-device.\n";
    system("adb wait-for-device");
    print "   2) Executing adb root.\n";
    system("adb root");
    print "   3) Connect the device. Executing adb wait-for-device.\n";
    system("adb wait-for-device");
    print "   4) Executing adb remount\n";
    system("adb remount");

    print "   5) Downloading the config file from device. Executing adb pull /system/etc/camera/camera_config.xml .\n";
    system("adb pull /system/etc/camera/camera_config.xml .");
    print "   6) Please update the camera_config.xml file and press Enter : ";
    <STDIN>;
    print "   7) pushing the config file to device. Executing adb push camera_config.xml /system/etc/camera";
    system("adb push camera_config.xml /system/etc/camera");
  }
}

sub print_menu()
{
  print "               ***********\n";
  print "                   MENU   \n";
  print "               ***********\n";
  print "        1. Generate Sensor Library\n";
  print "        2. Generate Actuator Library\n";
  print "        0. Exit the Utility\n\n";
  print "Please choose an option: ";
}

sub generate_project_directories()
{
  my $makefile_data;
  my $make_file_path;

  # Generate project directories.
  $jni_path = $project_location . $path_separator . "jni";
  unless (-d $jni_path) {
    mkdir($jni_path) or die "Couldn't create $jni_path directory, $!";
    print "Created $dir_name\n";
  }

  $dir_name = $project_location . $path_separator . "libs";
  unless (-d $dir_name) {
    mkdir($dir_name) or die "Couldn't create $dir_name directory, $!";
    print "Created $dir_name\n";
  }

  $dir_name = $project_location . $path_separator . "obj";
  unless (-d $dir_name) {
    mkdir($dir_name) or die "Couldn't create $dir_name directory, $!";
    print "Created $dir_name\n";
  }

  $makefile_data = "include \$(call all-subdir-makefiles)";
  $make_file_path = $jni_path . $path_separator . "Android.mk";
  open(my $fh, '>', $make_file_path) or
    die "Could not open file '$make_file_path' $!";
  print $fh $makefile_data;
  close $fh;
}

# Validate the project location.
sub validate_project_path{
  # Remove trailing \ and white spaces
  my $pattern_string = "\\" . $path_separator . "*\\s*\$";
  $project_location =~ s/$pattern_string//;

  # Check if the location is a valid directory
  unless (-d $project_location) {
    print "Path not found: $project_location\n";
    print "Please provide valid Project path\n";
    exit;
  }
}

# Validate the sdk location.
# Check if all the files are present in the sdk path.
sub validate_sdk_path{
  # Remove trailing \ and white spaces
  my $pattern_string = "\\" . $path_separator . "*\\s*\$";
  $sdk_location =~ s/$pattern_string//;

  # Check if the location is a valid directory
  unless (-d $sdk_location) {
    print "Path not found: $sdk_location\n";
    return 0;
  }

  $sdk_include_path = $sdk_location . $path_separator . "sensor" .
    $path_separator . "includes" . $path_separator;

  foreach $file (@sdk_files) {
    $file_path = $sdk_include_path . $file;
    unless (-f $file_path) {
      print "File not found : $file_path\n";
      return 0;
    }
  }
  return 1;
}

