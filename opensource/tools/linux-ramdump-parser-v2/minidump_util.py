# Copyright (c) 2012-2017, The Linux Foundation. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 and
# only version 2 as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

import sys
import re
import os
from print_out import print_out_str


def minidump_virt_to_phys(ebi_files,addr):
    pa_addr = None
    for a in ebi_files:
        idx, pa, end_addr, va,size = a
        if addr >= va and addr <= va +  size:
            offset = addr - va
            pa_addr = pa + offset
            return pa_addr
    return pa_addr

def read_physical_minidump(ebi_files,ebi_files_ramfile,elffile,addr,length):
    ebi = [-1, -1, -1, -1, -1]
    for a in ebi_files:
        idx, start, end, va, size = a
        if addr >= start and addr <= end:
            ebi = a
            break
    if ebi[0] != -1:
        idx = ebi[0]
        textSec = elffile.get_segment(idx)
        off = addr - ebi[1]
        elf_content = bytearray(a[4])
        val = textSec.data()
        elf_content[0:a[4]] = val
        data = elf_content[off:]
        return data[:length]
    else:
        ebi = (-1, -1, -1)
        for a in ebi_files_ramfile:
            fd, start, end, path = a
            if addr >= start and addr <= end:
                ebi = a
                break
        if ebi[0] is -1:
            return None
        offset = addr - ebi[1]
        ebi[0].seek(offset)
        a = ebi[0].read(length)
        return a
