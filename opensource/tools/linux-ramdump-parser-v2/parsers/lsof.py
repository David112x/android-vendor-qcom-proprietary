"""
Copyright (c) 2016, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""

from parser_util import register_parser, RamParser
DNAME_INLINE_LEN = 40
TASK_NAME_LENGTH = 16


def do_dump_lsof_info(self, ramdump, lsof_info):
    for task_struct in ramdump.for_each_process():
        parse_task(self, ramdump, task_struct, lsof_info)
        lsof_info.write("\n*********************************")


def parse_task(self, ramdump, task, lsof_info):
    index = 0
    if self.ramdump.arm64:
        addressspace = 8
    else:
        addressspace = 4

    task_comm_offset = ramdump.field_offset(
                        'struct task_struct',  'comm')
    task_comm_offset = task + task_comm_offset
    client_name = ramdump.read_cstring(
                    task_comm_offset, TASK_NAME_LENGTH)
    task_pid = ramdump.read_structure_field(
                    task, 'struct task_struct', 'pid')
    files = ramdump.read_structure_field(
                    task, 'struct task_struct', 'files')
    str_task_file = '\n Task: {0:x}, comm: {1}, pid : {2:1}, files : {3:x}'
    lsof_info.write(str_task_file.format(
                    task, client_name, task_pid, files))
    fdt = ramdump.read_structure_field(
                    files, 'struct files_struct', 'fdt')
    max_fds = ramdump.read_structure_field(
                    fdt, 'struct fdtable', 'max_fds')
    fd = ramdump.read_structure_field(
                    fdt, 'struct fdtable', 'fd')
    ion_str = "\n [{0}] file : 0x{1:x} {2} {3} client : 0x{4:x}"
    str = "\n [{0}] file : 0x{1:x} {2} {3}"

    while index < max_fds:
        file = ramdump.read_word(fd + (index * addressspace))
        if file != 0:
            fop = ramdump.read_structure_field(
                        file, 'struct file', 'f_op')
            priv_data = ramdump.read_structure_field(
                        file, 'struct file', 'private_data')
            look = ramdump.unwind_lookup(fop)
            if look is None:
                index = index + 1
                continue
            fop, offset = look

            f_pathoffset = ramdump.field_offset(
                            'struct file', 'f_path')
            f_path = f_pathoffset + file
            dentry = ramdump.read_structure_field(
                        f_path, 'struct path', 'dentry')
            dentry_iname_offset = ramdump.field_offset(
                                'struct dentry', 'd_iname')
            iname_address = dentry + dentry_iname_offset
            iname = ramdump.read_cstring(
                    iname_address, DNAME_INLINE_LEN)
            if iname != "null":
                look = ramdump.unwind_lookup(iname_address)
                if look is not None:
                    iname, offset = look
            if iname == "ion":
                lsof_info.write(ion_str.format(
                        index, file, fop, iname, priv_data))
            else:
                lsof_info.write(str.format(index, file, fop, iname))
        index = index + 1


@register_parser('--print-lsof',  'Print list of open files',  optional=True)
class DumpLsof(RamParser):

    def parse(self):
        with self.ramdump.open_file('lsof.txt') as lsof_info:
            if (self.ramdump.kernel_version < (3, 18, 0)):
                lsof_info.write('Kernel version 3.18 \
                and above are supported, current version {0}.\
                {1}'.format(self.ramdump.kernel_version[0],
                            self.ramdump.kernel_version[1]))
                return
            do_dump_lsof_info(self, self.ramdump, lsof_info)
