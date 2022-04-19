#!/usr/bin/python
#===============================================================================
#
# MBN TOOLS
#
# GENERAL DESCRIPTION
#    Contains all MBN Utilities for image generation
#
# Copyright (c) 2010-2019 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.
#
#-------------------------------------------------------------------------------
#
#  $Header: //components/dev/core.mpss/2.2/mmiranda.CORE.MPSS.2.2.mba_10_3/securemsm/mba/tools/mba_elf_builder.py#1 $
#  $DateTime: 2014/10/03 15:59:17 $
#  $Author: mmiranda $
#                      EDIT HISTORY FOR FILE
#
#  This section contains comments describing changes made to the module.
#  Notice that changes are listed in reverse chronological order.
#
# when       who     what, where, why
# --------   ---     ---------------------------------------------------------
# Dec/2018	 gfodi	 Add usage for creating elf file for IPA uC image
#
#===============================================================================

import stat
import csv
import itertools
import struct
import os
import shutil
import hashlib
import subprocess
import string
import re
import sys
import getopt
from optparse import OptionParser


#----------------------------------------------------------------------------
# GLOBAL VARIABLES BEGIN
#----------------------------------------------------------------------------
PAGE_SIZE               = 4096

# ELF Definitions
ELF_HDR_SIZE              = 52
ELF_PHDR_SIZE             = 32
ELFINFO_MAG0_INDEX        = 0
ELFINFO_MAG1_INDEX        = 1
ELFINFO_MAG2_INDEX        = 2
ELFINFO_MAG3_INDEX        = 3
ELFINFO_MAG0              = '\x7F\x45\x4c\x46\x01\x01\x01'
ELFINFO_CLASS_INDEX       = 4
ELFINFO_CLASS_32          = '\x01'
ELFINFO_VERSION_INDEX     = 6
ELFINFO_VERSION_CURRENT   = '\x01'
MAX_PHDRS_COUNT           = 100
ELF_BLOCK_ALIGN           = 0x1000
LOAD_TYPE                 = 0x1
PHDR_TYPE                 = 0x6
PROGRAM_HEADER_LEN        = 8
IMAGE_ALLIGMNET           = 0x1000

def concat_file (target, src_fname):
   with open(target,'ab') as target_file:
      with open(src_fname,'rb') as dst_file:
         while True:
            bin_data = dst_file.read()
            if not bin_data:
               break
            target_file.write(bin_data)

#----------------------------------------------------------------------------
# Header Class Notes:
# In order to properly read and write the header structures as binary data,
# the Python Struct library is used to align and package up the header objects.
# All Struct objects are initialized by a special string with the following
# notation. These structure objects are then used to decode binary data in order
# to fill out the appropriate class in Python, or they are used to package up
# the Python class so that we may write the binary data out.
#----------------------------------------------------------------------------
"""
      Format | C Type         | Python Type | Standard Size
      -----------------------------------------------------
    1) 'X's  | char *         | string      | 'X' bytes
    2) H     | unsigned short | integer     | 2 bytes
    3) I     | unsigned int   | integer     | 4 bytes

"""

#----------------------------------------------------------------------------
# ELF Header Class
#----------------------------------------------------------------------------
class Elf32_Ehdr:
   # Structure object to align and package the ELF Header

   def __init__(self, entry_address, init_val):
      self.e_ident = ELFINFO_MAG0
      self.e_type = 0x2
      self.e_machine = 0xA4
      self.e_version = 0x1
      #self.e_entry = entry_address
      self.e_entry = 0x0
      self.e_phoff = 52
      self.e_shoff = 0
      self.e_flags = 3
      self.e_ehsize = 52
      self.e_phentsize = 32
      self.e_phnum = 1
      self.e_shentsize = 0
      self.e_shnum = 0
      self.e_shstrndx = 0

   def printValues(self):
      print "ATTRIBUTE / VALUE"
      for attr, value in self.__dict__.iteritems():
         print attr, value

   def writePackedData(self, target):
      values = [self.e_ident,
                self.e_type,
                self.e_machine,
                self.e_version,
                self.e_entry,
                self.e_phoff,
                self.e_shoff,
                self.e_flags,
                self.e_ehsize,
                self.e_phentsize,
                self.e_phnum,
                self.e_shentsize,
                self.e_shnum,
                self.e_shstrndx
               ]

      s = struct.Struct('16sHHIIIIIHHHHHH')
      packed_data = s.pack(*values)
      with open(target,'ab') as fp:
         fp.write(packed_data)

      return s.size
#----------------------------------------------------------------------------
# ELF Program Header Class
#----------------------------------------------------------------------------
class Elf32_Phdr:

   # Structure object to align and package the ELF Program Header


   def __init__(self, entry_address, start_offset, load_size, mem_size, init_val, region_offset, make_reloc):
      self.p_type         = 1
      # This is where the actual data of this region is located
      self.p_offset       = region_offset
      self.p_vaddr        = entry_address
      self.p_paddr        = start_offset
      self.p_filesz       = load_size
      self.p_memsz        = mem_size
      self.p_flags        = 7
      self.p_align        = 0

      if make_reloc:
          self.p_align = IMAGE_ALLIGMNET
          self.p_flags = 0x8000007
      else:
          self.p_flags        = 7
          self.p_align        = 0


   def printValues(self):
      print "ATTRIBUTE / VALUE"
      for attr, value in self.__dict__.iteritems():
         print attr, value

   def getLength(self):
      return PROGRAM_HEADER_LEN

   def writePackedData(self, target):
      values = [self.p_type,
                self.p_offset,
                self.p_vaddr,
                self.p_paddr,
                self.p_filesz,
                self.p_memsz,
                self.p_flags,
                self.p_align
               ]
      s = struct.Struct('I' * self.getLength())
      packed_data = s.pack(*values)
      with open(target,'ab') as fp:
         fp.write(packed_data)
      return s.size


#----------------------------------------------------------------------------
# build_elf_file
#----------------------------------------------------------------------------
def build_elf_file(output_file_name, uc_fw_filename, uc_fw_address, uc_fw_mem_size):
   #import pdb;pdb.set_trace()
   uc_stat = os.stat(uc_fw_filename)
#----------------------------------------------------------------------------
#	Structure is as follow
#
#	ELF header
#	ELF program header I (IPA uC FW)
#	Padding for alignment to 4K
#	IPA uC FW binary
#	Padding for alignment to 4K
#----------------------------------------------------------------------------

   elf_header = Elf32_Ehdr(uc_fw_address, init_val = int('0x0',16))
   # The offset for the IPA uC FW can be hardcoded, because all the ELF headers come first.
   # It will never be higher than 4K
   current_offset = 0x1000
   uc_fw_header = Elf32_Phdr(uc_fw_address, 0x0, uc_stat.st_size, uc_fw_mem_size, init_val = int('0x0',16), region_offset=current_offset, make_reloc=1)

   # Package up the headers and write to output file
   elf_header.writePackedData(target = output_file_name)
   uc_fw_header.writePackedData(target = output_file_name)

   # Add padding for alignment to a page
   add_padding_to_page_align(output_file_name, os.stat(output_file_name).st_size)

   # Add the binary
   concat_file(output_file_name, uc_fw_filename)

   return 0

#----------------------------------------------------------------------------
# Add padding to align to page size
#----------------------------------------------------------------------------
def add_padding_to_page_align(output_file_name, written_size):

   if written_size < 0:
      raise RuntimeError, "Number of bytes to pad must be greater than zero"

   reduced_size = written_size

   while reduced_size > PAGE_SIZE:
      reduced_size -= PAGE_SIZE

   required_padding = PAGE_SIZE - reduced_size
   with open(output_file_name,'ab') as fp:
      while required_padding > 0:
         fp.write(b'\x00')
         required_padding -= 1

   return

#------------------------------------------------------------------------------
# Parse command line arguments
#------------------------------------------------------------------------------
def parse_args():
   usage = "usage: %prog [options]"
   version = "%prog 1.0"

   parser = OptionParser(usage=usage, version=version)
   parser.add_option("-u", "--uc_fw", dest="uc_fw_filename",
                  help="IPA uC FW binary file", metavar="FILE")
   parser.add_option("-x", "--uc_fw_address", dest="uc_fw_address",
                  help="IPA uC FW binary H/W loading address", metavar="FILE")
   parser.add_option("--uc_fw_mem_size", dest="uc_fw_mem_size",
                  help="IPA uC FW H/W memory (IRAM+DRAM) total size", metavar="FILE")
   parser.add_option("-o", "--outfile", dest="output_filename",
                  help="ELF output file name", metavar="FILE")
   (options, args) = parser.parse_args()

   if options.uc_fw_filename is None:
      parser.error("IPA uC FW file is missing")
      sys.exit(2)
   if options.uc_fw_address is None:
      parser.error("IPA uC FW binary H/W loading address is missing")
      sys.exit(2)
   if options.uc_fw_mem_size is None:
      parser.error("IPA uC FW H/W memory (IRAM+DRAM) total size is missing")
      sys.exit(2)
   if options.output_filename is None:
      parser.error("IPA uC FW H/W memory (IRAM+DRAM) total size is missing")
      sys.exit(2)

   return (options, args)

#------------------------------------------------------------------------------
# main
#------------------------------------------------------------------------------
if __name__ == "__main__":
   # get args from cmd line
   print sys.argv
   (options, args) = parse_args()

   # remove the ELF file if it exists
   if os.path.exists(options.output_filename):
      print "File already exists, deleting..."
      os.remove(options.output_filename)

   # makes the elf header in the target file
   build_elf_file(options.output_filename,
               options.uc_fw_filename, int(options.uc_fw_address, 16),
               int(options.uc_fw_mem_size, 16))
