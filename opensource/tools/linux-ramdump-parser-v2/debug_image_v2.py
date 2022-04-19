# Copyright (c) 2012-2019, The Linux Foundation. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 and
# only version 2 as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

import struct
import linux_list as llist
import re
import shutil
import os
import platform
import random
import subprocess
import sys
import time
import local_settings
from scandump_reader import Scandump_v2

from dcc import DccRegDump, DccSramDump
from pmic import PmicRegDump
from print_out import print_out_str, print_out_exception
from qdss import QDSSDump
from watchdog_v2 import TZRegDump_v2
from cachedumplib import lookup_cache_type
from tlbdumplib import lookup_tlb_type
from vsens import VsensData
from sysregs import SysRegDump
from fcmdump import FCM_Dump

MEMDUMPV2_MAGIC = 0x42445953
MAX_NUM_ENTRIES = 0x162
IMEM_OFFSET_MEM_DUMP_TABLE = 0x3f010

class client(object):
    MSM_DUMP_DATA_CPU_CTX = 0x00
    MSM_DUMP_DATA_L1_INST_TLB = 0x20
    MSM_DUMP_DATA_L1_DATA_TLB = 0x40
    MSM_DUMP_DATA_L1_INST_CACHE = 0x60
    MSM_DUMP_DATA_L1_DATA_CACHE = 0x80
    MSM_DUMP_DATA_ETM_REG = 0xA0
    MSM_DUMP_DATA_L2_CACHE = 0xC0
    MSM_DUMP_DATA_L3_CACHE = 0xD0
    MSM_DUMP_DATA_OCMEM = 0xE0
    MSM_DUMP_DATA_DBGUI_REG = 0xE5
    MSM_DUMP_DATA_MISC = 0xE8
    MSM_DUMP_DATA_VSENSE = 0xE9
    MSM_DUMP_DATA_TMC_ETF = 0xF0
    MSM_DUMP_DATA_TMC_ETF_SWAO = 0xF1
    MSM_DUMP_DATA_TMC_REG = 0x100
    MSM_DUMP_DATA_TMC_ETR_REG = 0x100
    MSM_DUMP_DATA_TMC_ETF_REG = 0x101
    MSM_DUMP_DATA_TMC_ETF_SWAO_REG = 0x102
    MSM_DUMP_DATA_LOG_BUF = 0x110
    MSM_DUMP_DATA_LOG_BUF_FIRST_IDX = 0x111
    MSM_DUMP_DATA_L2_TLB = 0x120
    MSM_DUMP_DATA_SCANDUMP = 0xEB
    MSM_DUMP_DATA_RPMH = 0xEC
    MSM_DUMP_DATA_FCMDUMP = 0xEE
    MSM_DUMP_DATA_CPUSS = 0xEF
    MSM_DUMP_DATA_SCANDUMP_PER_CPU = 0x130
    MSM_DUMP_DATA_LLC_CACHE = 0x140
    MSM_DUMP_DATA_MHM = 0x161
    MSM_DUMP_DATA_MAX = MAX_NUM_ENTRIES

# Client functions will be executed in top-to-bottom order
client_types = [
    ('MSM_DUMP_DATA_SCANDUMP', 'parse_scandump'),
    ('MSM_DUMP_DATA_SCANDUMP_PER_CPU', 'parse_scandump'),
    ('MSM_DUMP_DATA_FCMDUMP', 'parse_fcmdump'),
    ('MSM_DUMP_DATA_CPUSS', 'parse_cpuss'),
    ('MSM_DUMP_DATA_CPU_CTX', 'parse_cpu_ctx'),
    ('MSM_DUMP_DATA_L1_INST_TLB', 'parse_tlb_common'),
    ('MSM_DUMP_DATA_L1_DATA_TLB', 'parse_tlb_common'),
    ('MSM_DUMP_DATA_L1_INST_CACHE', 'parse_cache_common'),
    ('MSM_DUMP_DATA_L1_DATA_CACHE', 'parse_cache_common'),
    ('MSM_DUMP_DATA_L2_CACHE', 'parse_cache_common'),
    ('MSM_DUMP_DATA_L3_CACHE', 'parse_l3_cache'),
    ('MSM_DUMP_DATA_OCMEM', 'parse_ocmem'),
    ('MSM_DUMP_DATA_DBGUI_REG', 'parse_qdss_common'),
    ('MSM_DUMP_DATA_VSENSE', 'parse_vsens'),
    ('MSM_DUMP_DATA_PMIC', 'parse_pmic'),
    ('MSM_DUMP_DATA_DCC_REG', 'parse_dcc_reg'),
    ('MSM_DUMP_DATA_DCC_SRAM', 'parse_dcc_sram'),
    ('MSM_DUMP_DATA_TMC_ETF', 'parse_qdss_common'),
    ('MSM_DUMP_DATA_TMC_ETF_SWAO', 'parse_qdss_common'),
    ('MSM_DUMP_DATA_TMC_ETR_REG', 'parse_qdss_common'),
    ('MSM_DUMP_DATA_TMC_ETF_SWAO_REG', 'parse_qdss_common'),
    ('MSM_DUMP_DATA_TMC_REG', 'parse_qdss_common'),
    ('MSM_DUMP_DATA_L2_TLB', 'parse_tlb_common'),
    ('MSM_DUMP_DATA_LLC_CACHE', 'parse_system_cache_common'),
    ('MSM_DUMP_DATA_MISC', 'parse_sysdbg_regs'),
    ('MSM_DUMP_DATA_MHM', 'parse_mhm_dump')
]

qdss_tag_to_field_name = {
    'MSM_DUMP_DATA_TMC_ETR_REG': 'tmc_etr_start',
    'MSM_DUMP_DATA_TMC_REG': 'tmc_etr_start',
    'MSM_DUMP_DATA_TMC_ETF': 'etf_start',
    'MSM_DUMP_DATA_TMC_ETF_SWAO': 'tmc_etf_swao_start',
    'MSM_DUMP_DATA_TMC_ETF_SWAO_REG': 'tmc_etf_swao_reg_start',
    'MSM_DUMP_DATA_DBGUI_REG': 'dbgui_start',
}

# Client functions will be executed in top-to-bottom order
minidump_dump_table_type = [
    ('MSM_DUMP_DATA_SCANDUMP', 'KSCANDUMP'),
    ('MSM_DUMP_DATA_CPU_CTX', 'KCPU_CTX'),
    ('MSM_DUMP_DATA_L1_INST_TLB', 'KCPUSS'),
    ('MSM_DUMP_DATA_L1_DATA_TLB','KCPUSS'),
    ('MSM_DUMP_DATA_L1_INST_CACHE', 'KCPUSS'),
    ('MSM_DUMP_DATA_L1_DATA_CACHE', 'KCPUSS'),
    ('MSM_DUMP_DATA_L2_CACHE', 'KCPUSS'),
    ('MSM_DUMP_DATA_L3_CACHE', 'KCPUSS'),
    ('MSM_DUMP_DATA_VSENSE', 'KVSENSE'),
    ('MSM_DUMP_DATA_PMIC', 'KPMIC'),
    ('MSM_DUMP_DATA_DCC_REG', 'KDCC_REG'),
    ('MSM_DUMP_DATA_DCC_SRAM', 'KDCC_SRAM'),
    ('MSM_DUMP_DATA_TMC_ETF', 'KTMC_ETF'),
    ('MSM_DUMP_DATA_TMC_ETR_REG', 'KTMC_REG'),
    ('MSM_DUMP_DATA_TMC_REG', 'KTMC_REG'),
    ('MSM_DUMP_DATA_MISC', 'KMISC')

]

class DebugImage_v2():

    def __init__(self, ramdump):
        self.qdss = QDSSDump()
        self.dump_type_lookup_table = []
        self.dump_table_id_lookup_table = []
        self.dump_data_id_lookup_table  = {}
        if ramdump.kernel_version > (3, 9, 9):
            self.event_call = 'struct trace_event_call'
            self.event_class = 'struct trace_event_class'
        else:
            self.event_call = 'struct ftrace_event_call'
            self.event_class = 'struct ftrace_event_class'

    def parse_scandump(self, version, start, end, client_id, ram_dump):
        scandump_file_prefix = "scandump_core"
        core_bin_prefix = "core"
        chipset = ram_dump.hw_id
        try:
            scan_wrapper_path = local_settings.scandump_parser_path
        except AttributeError:
            print_out_str('Could not find scandump_parser_path . Please define scandump_parser_path in local_settings')
            return
        if ram_dump.arm64:
            arch = "aarch64"
        else:
            arch = "aarch32"
        if client_id == client.MSM_DUMP_DATA_SCANDUMP:
            output = os.path.join(ram_dump.outdir, scandump_file_prefix)
            input = os.path.join(ram_dump.outdir, "core.bin")
            core_num = client_id & 0xF
        elif client_id >= client.MSM_DUMP_DATA_SCANDUMP_PER_CPU:
            core_num = client_id & 0xF
            output = '{0}_{1:x}'.format(scandump_file_prefix, core_num)
            output = os.path.join(ram_dump.outdir, output)

            input_filename = '{0}_{1:x}.bin'.format(core_bin_prefix, core_num)
            input = os.path.join(ram_dump.outdir, input_filename)
        print_out_str(
            'Parsing scandump context start {0:x} end {1:x} {2} {3}'.format(start, end, output, input))
        header_bin = ram_dump.open_file(input)

        it = range(start, end)
        for i in it:
            val = ram_dump.read_byte(i, False)
            header_bin.write(struct.pack("<B", val))
        header_bin.close()
        subprocess.call('python {0} -d {1} -o {2} -f {3} -c {4}'.format(scan_wrapper_path, input, output, arch, chipset))
        sv2 = Scandump_v2(core_num,ram_dump,version)
        reg_info = sv2.prepare_dict()
        if reg_info is not None:
            sv2.dump_core_pc(ram_dump)
            sv2.dump_all_regs(ram_dump)
        return
    def parse_cpuss(self, version, start, end, client_id, ram_dump):
        client_name = self.dump_data_id_lookup_table[client_id]
        cpuss_file_prefix = "cpuss_reg_dump.xml"
        print_out_str(
                'Parsing {0} context start {1:x} end {2:x}'.format(client_name, start, end))
        try:
            cpuss_parser_path = local_settings.cpuss_parser_path
            cpuss_parser_json = local_settings.cpuss_parser_json
        except AttributeError:
            print_out_str('Could not find cpuss_parser_path . Please define cpuss_parser_path in local_settings')
            return
        offset = None
        for eb_file in ram_dump.ebi_files:
            if start >= eb_file[1] and start <= eb_file[2]:
                input = eb_file[3]
                offset = start - eb_file[1]
                break
        if offset is None:
            print_out_str("parse_cpuss start address {0} not found".format(start))
        print_out_str("parse_cpuss offset address = {0} input = {1} cpuss_parser_json = {2}".format(hex(int(offset)),input,cpuss_parser_json))
        subprocess.call('python {0} -i {1} -O {2} -o {3} -j {4}'.format(cpuss_parser_path, input, hex(int(offset)), ram_dump.outdir, cpuss_parser_json))

    def parse_fcmdump(self, version, start, end, client_id, ram_dump):
        client_name = self.dump_data_id_lookup_table[client_id]

        print_out_str(
                'Parsing {0} context start {1:x} end {2:x}'.format(client_name, start, end))

        fcmdumps = FCM_Dump(start, end)
        if fcmdumps.dump_fcm_img(ram_dump) is False:
            print_out_str('!!! Could not dump FCM')

        return

    def parse_cpu_ctx(self, version, start, end, client_id, ram_dump):
        core = client_id - client.MSM_DUMP_DATA_CPU_CTX

        print_out_str(
            'Parsing CPU{2} context start {0:x} end {1:x}'.format(start, end, core))

        regs = TZRegDump_v2()
        if regs.init_regs(version, start, end, core, ram_dump) is False:
            print_out_str('!!! Could not get registers from TZ dump')
            return
        regs.dump_core_pc(ram_dump)
        regs.dump_all_regs(ram_dump)

    def parse_pmic(self, version, start, end, client_id, ram_dump):
        client_name = self.dump_data_id_lookup_table[client_id]

        print_out_str(
            'Parsing {0} context start {1:x} end {2:x}'.format(client_name, start, end))

        regs = PmicRegDump(start, end)
        if regs.parse_all_regs(ram_dump) is False:
            print_out_str('!!! Could not get registers from PMIC dump')
            return

        regs.dump_all_regs(ram_dump)

    def parse_dcc_reg(self, version, start, end, client_id, ram_dump):
        client_name = self.dump_data_id_lookup_table[client_id]

        print_out_str(
            'Parsing {0} context start {1:x} end {2:x}'.format(client_name, start, end))

        regs = DccRegDump(start, end)
        if regs.parse_all_regs(ram_dump) is False:
            print_out_str('!!! Could not get registers from DCC register dump')
            return

        regs.dump_all_regs(ram_dump)
        return

    def parse_dcc_sram(self, version, start, end, client_id, ram_dump):
        client_name = self.dump_data_id_lookup_table[client_id]

        print_out_str(
            'Parsing {0} context start {1:x} end {2:x}'.format(client_name, start, end))

        regs = DccSramDump(start, end, ram_dump)
        if regs.dump_sram_img(ram_dump) is False:
            print_out_str('!!! Could not dump SRAM')
        else:
            ram_dump.dcc = True
        return

    def parse_sysdbg_regs(self, version, start, end, client_id, ram_dump):
        client_name = self.dump_data_id_lookup_table[client_id]

        print_out_str(
            'Parsing {0} context start {1:x} end {2:x}'.format(client_name, start, end))

        sysregs = SysRegDump(start, end)
        if sysregs.dump_sysreg_img(ram_dump) is False:
            print_out_str('!!! Could not dump sysdbg_regs')
        else:
            ram_dump.sysreg = True
        return

    def parse_vsens(self, version, start, end, client_id, ram_dump):
        client_name = self.dump_data_id_lookup_table[client_id]

        print_out_str(
            'Parsing {0} context start {1:x} end {2:x}'.format(client_name, start, end))

        regs = VsensData()
        if regs.init_dump_regs(start, end, ram_dump) is False:
            print_out_str('!!! Could not get registers from Vsens Dump')
            return
        regs.print_vsens_regs(ram_dump)

    def parse_qdss_common(self, version, start, end, client_id, ram_dump):
        client_name = self.dump_data_id_lookup_table[client_id]

        print_out_str(
            'Parsing {0} context start {1:x} end {2:x}'.format(client_name, start, end))

        if client_id == client.MSM_DUMP_DATA_TMC_ETF_REG:
            setattr(self.qdss, 'tmc_etf_start', start)
        else:
            setattr(self.qdss, qdss_tag_to_field_name[client_name], start)

    def parse_cache_common(self, version, start, end, client_id, ramdump):
        client_name = self.dump_data_id_lookup_table[client_id]
        core = client_id & 0xF
        filename = '{0}_0x{1:x}'.format(client_name, core)
        outfile = ramdump.open_file(filename)
        cache_type = lookup_cache_type(ramdump.hw_id, client_id, version)
        try:
            cache_type.parse(start, end, ramdump, outfile)
        except NotImplementedError:
            print_out_str('Cache dumping not supported for %s on this target'
                          % client_name)
        except:
            print_out_str('!!! Unhandled exception while running {0}'.format(client_name))
            print_out_exception()
        outfile.close()

    def parse_system_cache_common(self, version, start, end, client_id, ramdump):
        client_name = self.dump_data_id_lookup_table[client_id]
        bank_number = client_id - client.MSM_DUMP_DATA_LLC_CACHE
        filename = '{0}_0x{1:x}'.format(client_name, bank_number)
        outfile = ramdump.open_file(filename)
        cache_type = lookup_cache_type(ramdump.hw_id, client_id, version)
        try:
            cache_type.parse(start, end, ramdump, outfile)
        except NotImplementedError:
            print_out_str('System cache dumping not supported %s'
                          % client_name)
        except:
            print_out_str('!!! Unhandled exception while running {0}'.format(client_name))
            print_out_exception()
        outfile.close()

    def parse_tlb_common(self, version, start, end, client_id, ramdump):
        client_name = self.dump_data_id_lookup_table[client_id]
        core = client_id & 0xF
        filename = '{0}_0x{1:x}'.format(client_name, core)
        outfile = ramdump.open_file(filename)
        cache_type = lookup_tlb_type(ramdump.hw_id, client_id, version)
        try:
            cache_type.parse(start, end, ramdump, outfile)
        except NotImplementedError:
            print_out_str('TLB dumping not supported for %s on this target'
                          % client_name)
        except:
            print_out_str('!!! Unhandled exception while running {0}'.format(client_name))
            print_out_exception()
        outfile.close()

    def ftrace_field_func(self, common_list, ram_dump):
        name_offset = ram_dump.field_offset('struct ftrace_event_field', 'name')
        type_offset = ram_dump.field_offset('struct ftrace_event_field', 'type')
        filter_type_offset = ram_dump.field_offset('struct ftrace_event_field', 'filter_type')
        field_offset = ram_dump.field_offset('struct ftrace_event_field', 'offset')
        size_offset = ram_dump.field_offset('struct ftrace_event_field', 'size')
        signed_offset = ram_dump.field_offset('struct ftrace_event_field', 'is_signed')

        name = ram_dump.read_word(common_list + name_offset)
        field_name = ram_dump.read_cstring(name, 256)
        type_name = ram_dump.read_word(common_list + type_offset)
        type_str = ram_dump.read_cstring(type_name, 256)
        offset = ram_dump.read_u32(common_list + field_offset)
        size = ram_dump.read_u32(common_list + size_offset)
        signed = ram_dump.read_u32(common_list + signed_offset)

        if re.match('(.*)\[(.*)', type_str) and not(re.match('__data_loc', type_str)):
            s = re.split('\[', type_str)
            s[1] = '[' + s[1]
            self.formats_out.write("\tfield:{0} {1}{2};\toffset:{3};\tsize:{4};\tsigned:{5};\n".format(s[0], field_name, s[1], offset, size, signed))
        else:
            self.formats_out.write("\tfield:{0} {1};\toffset:{2};\tsize:{3};\tsigned:{4};\n".format(type_str, field_name, offset, size, signed))

    def ftrace_events_func(self, ftrace_list, ram_dump):
        event_offset = ram_dump.field_offset(self.event_call, 'event')
        fmt_offset = ram_dump.field_offset(self.event_call,'print_fmt')
        class_offset = ram_dump.field_offset(self.event_call, 'class')
        flags_offset = ram_dump.field_offset(self.event_call, 'flags')
        flags = ram_dump.read_word(ftrace_list + flags_offset)
        if ram_dump.kernel_version >= (4, 14):
            TRACE_EVENT_FL_TRACEPOINT = 0x10
        elif ram_dump.kernel_version >= (4, 9):
            TRACE_EVENT_FL_TRACEPOINT = 0x20
        else:
            TRACE_EVENT_FL_TRACEPOINT = 0x40
        if (ram_dump.kernel_version >= (3, 18) and (flags & TRACE_EVENT_FL_TRACEPOINT)):
            tp_offset = ram_dump.field_offset(self.event_call, 'tp')
            tp_name_offset = ram_dump.field_offset('struct tracepoint', 'name')
            tp = ram_dump.read_word(ftrace_list + tp_offset)
            name = ram_dump.read_word(tp + tp_name_offset)
        else:
            name_offset = ram_dump.field_offset(self.event_call, 'name')
            name = ram_dump.read_word(ftrace_list + name_offset)

        type_offset = ram_dump.field_offset('struct trace_event', 'type')
        fields_offset = ram_dump.field_offset(self.event_class, 'fields')
        common_field_list = ram_dump.address_of('ftrace_common_fields')
        field_next_offset = ram_dump.field_offset('struct ftrace_event_field', 'link')

        name_str = ram_dump.read_cstring(name, 512)
        event_id = ram_dump.read_word(ftrace_list + event_offset + type_offset)
        fmt = ram_dump.read_word(ftrace_list + fmt_offset)
        fmt_str = ram_dump.read_cstring(fmt, 2048)

        self.formats_out.write("name: {0}\n".format(name_str))
        self.formats_out.write("ID: {0}\n".format(event_id))
        self.formats_out.write("format:\n")

        list_walker = llist.ListWalker(ram_dump, common_field_list, field_next_offset)
        list_walker.walk_prev(common_field_list, self.ftrace_field_func, ram_dump)
        self.formats_out.write("\n")

        event_class = ram_dump.read_word(ftrace_list + class_offset)
        field_list =  event_class + fields_offset
        list_walker = llist.ListWalker(ram_dump, field_list, field_next_offset)
        list_walker.walk_prev(field_list, self.ftrace_field_func, ram_dump)
        self.formats_out.write("\n")
        self.formats_out.write("print fmt: {0}\n".format(fmt_str))

    def collect_ftrace_format(self, ram_dump):
        formats = os.path.join('qtf', 'map_files', 'formats.txt')
        formats_out = ram_dump.open_file(formats)
        self.formats_out = formats_out

        ftrace_events_list = ram_dump.address_of('ftrace_events')
        next_offset = ram_dump.field_offset(self.event_call, 'list')
        list_walker = llist.ListWalker(ram_dump, ftrace_events_list, next_offset)
        list_walker.walk_prev(ftrace_events_list, self.ftrace_events_func, ram_dump)

        self.formats_out.close

    def wait_for_completion_timeout(self, task, timeout):
        delay = 2.0
        #while the process is still executing and we haven't timed-out yet
        while task.poll() is None and timeout > 0:
            time.sleep(delay)
            timeout -= delay
        if timeout <= 0:
            print_out_str("QTF command timed out")
            task.kill()

    def parse_qtf(self, ram_dump):
        out_dir = ram_dump.outdir
        if platform.system() != 'Windows':
            return

        qtf_path = ram_dump.qtf_path
        if qtf_path is None:
            try:
                import local_settings
                try:
                    qtf_path = local_settings.qtf_path
                except AttributeError as e:
                    print_out_str("attribute qtf_path in local_settings.py looks bogus. Please see README.txt")
                    print_out_str("Full message: %s" % e.message)
                    return
            except ImportError:
                print_out_str("missing qtf_path local_settings.")
                print_out_str("Please see the README for instructions on setting up local_settings.py")
                return

        if qtf_path is None:
            print_out_str("!!! Incorrect path for qtf specified.")
            print_out_str("!!! Please see the README for instructions on setting up local_settings.py")
            return

        if not os.path.exists(qtf_path):
            print_out_str("!!! qtf_path {0} does not exist! Check your settings!".format(qtf_path))
            return

        if not os.access(qtf_path, os.X_OK):
            print_out_str("!!! No execute permissions on qtf path {0}".format(qtf_path))
            return

        if os.path.getsize(os.path.join(out_dir, 'tmc-etf.bin')) > 0:
            trace_file = os.path.join(out_dir, 'tmc-etf.bin')
        elif os.path.getsize(os.path.join(out_dir, 'tmc-etr.bin')) > 0:
            trace_file = os.path.join(out_dir, 'tmc-etr.bin')
        else:
            return

        port = None
        server_proc = None
        qtf_success = False
        max_tries = 3
        qtf_dir = os.path.join(out_dir, 'qtf')
        workspace = os.path.join(qtf_dir, 'qtf.workspace')
        qtf_out = os.path.join(out_dir, 'qtf.txt')
        chipset = ram_dump.hw_id
        if "sdm" not in ram_dump.hw_id.lower() and \
          "qcs" not in ram_dump.hw_id.lower():
            chipset = "msm" + ram_dump.hw_id
        hlos = 'LA'

        #Temp change to handle descripancy between tools usage
        if chipset == 'msmcobalt':
            chipset = 'msm8998'

        # Resolve any port collisions with other running qtf_server instances
        for tries in range(max_tries):
            port = random.randint(12000, 13000)
            server_proc = subprocess.Popen(
                [qtf_path, '-s', '{:d}'.format(port)], shell=True,stderr=subprocess.PIPE)
            time.sleep(15)
            server_proc.poll()
            if server_proc.returncode == 1:
                server_proc.terminate()
                continue
            else:
                qtf_success = True
                break
        if not qtf_success:
            server_proc.terminate()
            print_out_str('!!! Could not open a QTF server instance with a '
                          'unique port (last port tried: '
                          '{0})'.format(str(port)))
            print_out_str('!!! Please kill all currently running qtf_server '
                          'processes and try again')
            return
        #subprocess.call('{0} -c {1} new workspace {2} {3} {4}'.format(qtf_path, port, qtf_dir, chipset, hlos))
        server_proc1 = subprocess.Popen(
                   [qtf_path, '-c', '{:d}'.format(port),'new workspace {0} {1} {2}'.format(qtf_dir, chipset, hlos)], shell=True,stderr=subprocess.PIPE,stdout=subprocess.PIPE)
        server_proc1.communicate()
        self.collect_ftrace_format(ram_dump)
        server_proc1.kill()

        p = subprocess.Popen('{0} -c {1} open workspace {2}'.format(qtf_path, port, workspace))
        self.wait_for_completion_timeout(p,60)
        p = subprocess.Popen('{0} -c {1} open bin {2}'.format(qtf_path, port, trace_file))
        self.wait_for_completion_timeout(p,90)
        p = subprocess.Popen('{0} -c {1} stream trace table {2}'.format(qtf_path, port, qtf_out))
        self.wait_for_completion_timeout(p,300)
        p = subprocess.Popen('{0} -c {1} close'.format(qtf_path, port))
        self.wait_for_completion_timeout(p,60)
        p = subprocess.Popen('{0} -c {1} exit'.format(qtf_path, port))
        self.wait_for_completion_timeout(p,60)
        try:
            import psutil
            pid = server_proc.pid
            pid = int(pid)
            parent = psutil.Process(pid)
            for child in parent.children(recursive=True):				# or parent.children() for recursive=False
                print_out_str("child process = {0} which needs to be killed forcefully after QTF timeout".format(child))
                if (psutil.pid_exists(child.pid)):
                    try:
                        child.kill()
                    except Exception as e:
                        print_out_str("Error: {0} while killing the child of QTF process".format(e))
                        pass
            server_proc.kill()
        except Exception as e1:
            print_out_str("psutil module import error = {0}".format(e1))
            pass

    def parse_dcc(self, ram_dump):
        out_dir = ram_dump.outdir
        if ram_dump.ram_addr is None:
            bin_dir = ram_dump.autodump
        else:
            bin_dir = ram_dump.ram_addr
            bin_dir="\\".join(bin_dir[0][0].split('\\')[:-1])
        dcc_parser_path = os.path.join(os.path.dirname(__file__), '..', 'dcc_parser', 'dcc_parser.py')
        if dcc_parser_path is None:
            print_out_str("!!! Incorrect path for DCC specified.")
            return

        if not os.path.exists(dcc_parser_path):
            print_out_str("!!! dcc_parser_path {0} does not exist! Check your settings!".format(dcc_parser_path))
            return

        if (os.path.isfile(os.path.join(bin_dir, 'DCC_SRAM.BIN'))):
            sram_file = os.path.join(bin_dir, 'DCC_SRAM.BIN')
            cmd = ["-s ", sram_file, " --out-dir ", out_dir, " --config-offset ", "0x6000", " --v2"]
            p = subprocess.Popen([sys.executable, dcc_parser_path, cmd], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            print_out_str('--------')
            print_out_str(p.communicate()[0])
        elif os.path.isfile(os.path.join(out_dir, 'sram.bin')):
            sram_file = os.path.join(out_dir, 'sram.bin')
            p = subprocess.Popen([sys.executable, dcc_parser_path, '-s', sram_file, '--out-dir', out_dir],
                    stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            print_out_str('--------')
            print_out_str(p.communicate()[0])
        else:
            print_out_str('DCC SRAM data is not populated!!')
            return

    def parse_sysreg(self,ram_dump):
        out_dir = ram_dump.outdir
        sysreg_parser_path_minidump = os.path.join(os.path.dirname(__file__), '..', 'dcc_parser',
                                                'sysregs_parser_minidump.py')
        if sysreg_parser_path_minidump is None:
            print_out_str("!!! Incorrect path for SYSREG specified.")
            return
        if not os.path.exists(sysreg_parser_path_minidump):
            print_out_str("!!! sysreg_parser_path_minidump {0} does not exist! "
                          "Check your settings!"
                          .format(sysreg_parser_path_minidump))
            return
        if os.path.getsize(os.path.join(out_dir, 'sysdbg_regs.bin')) > 0:
            sysdbg_file = os.path.join(out_dir, 'sysdbg_regs.bin')
        else:
            return
        p = subprocess.Popen([sys.executable, sysreg_parser_path_minidump, '-s', sysdbg_file, '--out-dir', out_dir],
                             stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

        print_out_str('--------')
        print_out_str(p.communicate()[0])

    def sorted_dump_data_clients(self, ram_dump, table, table_num_entries):
        """ Returns a sorted list of (client_name, func, client_address) where

        client_address --
            the (struct msm_dump_entry*) which contains a client_id mapping to
            client_name

        func --
            registered function in client_types to parse entries of
            this type

        the return value is sorted in the same order as the client names
        in client_types
        """

        dump_entry_id_offset = ram_dump.field_offset(
            'struct msm_dump_entry', 'id')
        dump_entry_size = ram_dump.sizeof('struct msm_dump_entry')
        results = list()

        client_table = dict(client_types)
        # get first column of client_types
        client_names = zip(*client_types)[0]

        for j in range(0, table_num_entries):
            client_entry = table + j * dump_entry_size
            client_id = ram_dump.read_u32(
                            client_entry + dump_entry_id_offset, False)
            if client_id in self.dump_data_id_lookup_table:
                client_name = self.dump_data_id_lookup_table[client_id]
                if client_name not in client_table:
                    print_out_str(
                        '!!! client_id = {0} client_name = {1} Does not have an associated function. Skipping!'.format(client_id,client_name))
                    continue
            else:
                print_out_str(
                    '!!! Invalid dump client id found {0:x}'.format(client_id))
                continue

            results.append((client_name, client_table[client_name], client_entry))

        results.sort(key=lambda(x): client_names.index(x[0]))
        return results
    def minidump_data_clients(self, ram_dump, client_id,entry_pa_addr,
                                      end_addr):
        results = list()
        client_table = dict(client_types)
        # get first column of client_types

        client_name = self.dump_data_id_lookup_table[client_id]

        if client_name not in client_table:
            print_out_str(
                '!!! {0} Does not have an associated function. Skipping!'.format(client_name))
            return None

        results.append((client_name, client_id,client_table[client_name], entry_pa_addr,end_addr))
        return results
    def parse_mhm_dump(self, version, start, end, client_id, ram_dump):
        if ram_dump.arm64:
            arch = "aarch64"
        else:
            arch = "aarch32"
        if client_id == client.MSM_DUMP_DATA_MHM:
            input = os.path.join(ram_dump.outdir, "mhm_scandump.bin")
        print_out_str(
            'Parsing mhm dump start {0:x} end {1:x} {2}'.format(start, end, input))
        header_bin = ram_dump.open_file(input)
        it = range(start, end)
        for i in it:
            val = ram_dump.read_byte(i, False)
            header_bin.write(struct.pack("<B", val))
        header_bin.close()
        return
    class MsmDumpTable(object):
        def __init__(self):
            self.name = "Anon"
            self.phys_addr = 0x0
            self.version = 0x0
            self.num_entries = 0x0

    """ Create an instance of MsmDumpTable, or None on error """
    def validateMsmDumpTable(self, ram_dump, name, table_phys):
        if table_phys is None:
            print_out_str('debug_image.py: Table {}: Unable to read dump table base address'.format(name))
            return None

        version = ram_dump.read_structure_field(
                    table_phys, 'struct msm_dump_table', 'version',
                    virtual = False)
        if version is None:
            print_out_str('Table {}: Version is bogus! Can\'t parse debug image'.format(name))
            return None

        num_entries = ram_dump.read_structure_field(
                        table_phys, 'struct msm_dump_table', 'num_entries',
                        virtual = False)
        if num_entries is None or num_entries > 100:
            print_out_str('Table {}: num_entries is bogus! Can\'t parse debug image'.format(name))
            return None

        table = self.MsmDumpTable()
        table.name = name
        table.phys_addr = table_phys
        table.version = version
        table.num_entries = num_entries
        return table

    def parse_dump_v2(self, ram_dump):
        self.dump_type_lookup_table = ram_dump.gdbmi.get_enum_lookup_table(
            'msm_dump_type', 2)
        self.dump_table_id_lookup_table = ram_dump.gdbmi.get_enum_lookup_table(
            'msm_dump_table_ids', MAX_NUM_ENTRIES)
        cpus = ram_dump.get_num_cpus()
        # per cpu entries
        for i in range(0, cpus):

                self.dump_data_id_lookup_table[
                    client.MSM_DUMP_DATA_CPU_CTX + i] = 'MSM_DUMP_DATA_CPU_CTX'
                self.dump_data_id_lookup_table[
                    client.MSM_DUMP_DATA_L1_INST_TLB + i] = 'MSM_DUMP_DATA_L1_INST_TLB'
                self.dump_data_id_lookup_table[
                    client.MSM_DUMP_DATA_L1_DATA_TLB + i] = 'MSM_DUMP_DATA_L1_DATA_TLB'
                self.dump_data_id_lookup_table[
                    client.MSM_DUMP_DATA_L1_INST_CACHE + i] = 'MSM_DUMP_DATA_L1_INST_CACHE'
                self.dump_data_id_lookup_table[
                    client.MSM_DUMP_DATA_L1_DATA_CACHE + i] = 'MSM_DUMP_DATA_L1_DATA_CACHE'
                self.dump_data_id_lookup_table[
                    client.MSM_DUMP_DATA_L2_CACHE + i] = 'MSM_DUMP_DATA_L2_CACHE'
                self.dump_data_id_lookup_table[
                    client.MSM_DUMP_DATA_L3_CACHE + i] = 'MSM_DUMP_DATA_L3_CACHE'
                self.dump_data_id_lookup_table[
                    client.MSM_DUMP_DATA_ETM_REG + i] = 'MSM_DUMP_DATA_ETM_REG'
                self.dump_data_id_lookup_table[
                    client.MSM_DUMP_DATA_SCANDUMP_PER_CPU + i] = 'MSM_DUMP_DATA_SCANDUMP_PER_CPU'

        for i in range(0, 4):
                self.dump_data_id_lookup_table[
                    client.MSM_DUMP_DATA_LLC_CACHE + i] = 'MSM_DUMP_DATA_LLC_CACHE'
        self.dump_data_id_lookup_table[
            client.MSM_DUMP_DATA_OCMEM] = 'MSM_DUMP_DATA_OCMEM'
        self.dump_data_id_lookup_table[
            client.MSM_DUMP_DATA_DBGUI_REG] = 'MSM_DUMP_DATA_DBGUI_REG'
        self.dump_data_id_lookup_table[
            client.MSM_DUMP_DATA_MISC] = 'MSM_DUMP_DATA_MISC'
        self.dump_data_id_lookup_table[
            client.MSM_DUMP_DATA_VSENSE] = 'MSM_DUMP_DATA_VSENSE'
        self.dump_data_id_lookup_table[
            client.MSM_DUMP_DATA_TMC_ETF] = 'MSM_DUMP_DATA_TMC_ETF'
        self.dump_data_id_lookup_table[
            client.MSM_DUMP_DATA_TMC_ETF_SWAO] = 'MSM_DUMP_DATA_TMC_ETF_SWAO'
        self.dump_data_id_lookup_table[
            client.MSM_DUMP_DATA_TMC_ETF_REG] = 'MSM_DUMP_DATA_TMC_ETF_REG'
        self.dump_data_id_lookup_table[
            client.MSM_DUMP_DATA_SCANDUMP] = 'MSM_DUMP_DATA_SCANDUMP'
        self.dump_data_id_lookup_table[
            client.MSM_DUMP_DATA_LLC_CACHE] = 'MSM_DUMP_DATA_LLC_CACHE'
        self.dump_data_id_lookup_table[
            client.MSM_DUMP_DATA_TMC_ETF_SWAO_REG] = 'MSM_DUMP_DATA_TMC_ETF_SWAO_REG'
        self.dump_data_id_lookup_table[
            client.MSM_DUMP_DATA_TMC_REG] = 'MSM_DUMP_DATA_TMC_REG'

        self.dump_data_id_lookup_table[
            client.MSM_DUMP_DATA_FCMDUMP] = 'MSM_DUMP_DATA_FCMDUMP'
        self.dump_data_id_lookup_table[
            client.MSM_DUMP_DATA_CPUSS] = 'MSM_DUMP_DATA_CPUSS'
        # 0x100 - tmc-etr registers and 0x101 - for tmc-etf registers
        self.dump_data_id_lookup_table[
            client.MSM_DUMP_DATA_TMC_ETR_REG + 1] = 'MSM_DUMP_DATA_TMC_ETR_REG'
        self.dump_data_id_lookup_table[
            client.MSM_DUMP_DATA_LOG_BUF] = 'MSM_DUMP_DATA_LOG_BUF'
        self.dump_data_id_lookup_table[
            client.MSM_DUMP_DATA_LOG_BUF_FIRST_IDX] = 'MSM_DUMP_DATA_LOG_BUF_FIRST_IDX'
        self.dump_data_id_lookup_table[
            client.MSM_DUMP_DATA_MHM] = 'MSM_DUMP_DATA_MHM'
	for i in range(0, cpus):
		self.dump_data_id_lookup_table[
		    client.MSM_DUMP_DATA_L2_TLB + i] = 'MSM_DUMP_DATA_L2_TLB'

        if not ram_dump.minidump:
            dump_table_ptr_offset = ram_dump.field_offset(
                'struct msm_memory_dump', 'table')
            dump_table_version_offset = ram_dump.field_offset(
                'struct msm_dump_table', 'version')
            dump_table_num_entry_offset = ram_dump.field_offset(
                'struct msm_dump_table', 'num_entries')
            dump_table_entry_offset = ram_dump.field_offset(
                'struct msm_dump_table', 'entries')
            dump_entry_id_offset = ram_dump.field_offset(
                'struct msm_dump_entry', 'id')
            dump_entry_name_offset = ram_dump.field_offset(
                'struct msm_dump_entry', 'name')
            dump_entry_type_offset = ram_dump.field_offset(
                'struct msm_dump_entry', 'type')
            dump_entry_addr_offset = ram_dump.field_offset(
                'struct msm_dump_entry', 'addr')
            dump_data_version_offset = ram_dump.field_offset(
                'struct msm_dump_data', 'version')
            dump_data_magic_offset =  ram_dump.field_offset(
                'struct msm_dump_data', 'magic')
            dump_data_name_offset = ram_dump.field_offset(
                'struct msm_dump_data', 'name')
            dump_data_addr_offset = ram_dump.field_offset(
                'struct msm_dump_data', 'addr')
            dump_data_len_offset = ram_dump.field_offset(
                'struct msm_dump_data', 'len')
            dump_data_reserved_offset = ram_dump.field_offset(
                'struct msm_dump_data', 'reserved')
            dump_entry_size = ram_dump.sizeof('struct msm_dump_entry')
            dump_data_size = ram_dump.sizeof('struct msm_dump_data')

            """
            Some multi-guest hypervisor systems override the imem location
            with a table for a crashed guest. So the value from IMEM may
            not match the value saved in the linux variable 'memdump'.
            """
            table_phys = ram_dump.read_word(
                ram_dump.board.imem_start + IMEM_OFFSET_MEM_DUMP_TABLE,
                virtual = False)
            root_table = self.validateMsmDumpTable(ram_dump, "IMEM", table_phys)

            if root_table is None:
                table_phys = ram_dump.read_structure_field(
                    'memdump', 'struct msm_memory_dump', 'table_phys')
                root_table = self.validateMsmDumpTable(ram_dump, "RAM", table_phys)

            if root_table is None:
                return

            print_out_str('\nDebug image version: {0}.{1} Number of table entries {2}'.format(
                root_table.version >> 20, root_table.version & 0xFFFFF, root_table.num_entries))
            print_out_str('--------')

            for i in range(0, root_table.num_entries):
                this_entry = root_table.phys_addr + dump_table_entry_offset + \
                    i * dump_entry_size
                entry_id = ram_dump.read_u32(this_entry + dump_entry_id_offset, virtual = False)
                entry_type = ram_dump.read_u32(this_entry + dump_entry_type_offset, virtual = False)
                entry_addr = ram_dump.read_word(this_entry + dump_entry_addr_offset, virtual = False)

                if entry_id < 0 or entry_id > len(self.dump_table_id_lookup_table):
                    print_out_str(
                        '!!! Invalid dump table entry id found {0:x}'.format(entry_id))
                    continue

                if entry_type > len(self.dump_type_lookup_table):
                    print_out_str(
                        '!!! Invalid dump table entry type found {0:x}'.format(entry_type))
                    continue

                table_version = ram_dump.read_u32(
                    entry_addr + dump_table_version_offset, False)
                if table_version is None:
                    print_out_str('Dump table entry version is bogus! Can\'t parse debug image')
                    return
                table_num_entries = ram_dump.read_u32(
                    entry_addr + dump_table_num_entry_offset, False)
                if table_num_entries is None or table_num_entries > 100:
                    print_out_str('Dump table entry num_entries is bogus! Can\'t parse debug image')
                    return

                print_out_str(
                    'Debug image version: {0}.{1} Entry id: {2} Entry type: {3} Number of entries: {4}'.format(
                        table_version >> 20, table_version & 0xFFFFF, self.dump_table_id_lookup_table[entry_id],
                        self.dump_type_lookup_table[entry_type], table_num_entries))

                lst = self.sorted_dump_data_clients(
                        ram_dump, entry_addr + dump_table_entry_offset,
                        table_num_entries)
                for (client_name, func, client_entry) in lst:
                    print_out_str('--------')
                    client_id = ram_dump.read_u32(
                                    client_entry + dump_entry_id_offset, False)
                    client_type = ram_dump.read_u32(
                                    client_entry + dump_entry_type_offset, False)
                    client_addr = ram_dump.read_word(
                                    client_entry + dump_entry_addr_offset, False)

                    if client_type > len(self.dump_type_lookup_table):
                        print_out_str(
                            '!!! Invalid dump client type found {0:x}'.format(client_type))
                        continue

                    dump_data_magic = ram_dump.read_u32(
                                    client_addr + dump_data_magic_offset, False)
                    dump_data_version = ram_dump.read_u32(
                                    client_addr + dump_data_version_offset, False)
                    dump_data_name = ram_dump.read_cstring(
                            client_addr + dump_data_name_offset,
                            ram_dump.sizeof('((struct msm_dump_data *)0x0)->name'),
                            False)
                    dump_data_addr = ram_dump.read_dword(
                                        client_addr + dump_data_addr_offset, False)
                    dump_data_len = ram_dump.read_dword(
                                        client_addr + dump_data_len_offset, False)
                    print_out_str('Parsing debug information for {0}. Version: {1} Magic: {2:x} Source: {3}'.format(
                        client_name, dump_data_version, dump_data_magic,
                        dump_data_name))

                    if dump_data_magic is None:
                        print_out_str("!!! Address {0:x} is bogus! Can't parse!".format(
                                    client_addr + dump_data_magic_offset))
                        continue

                    if dump_data_magic != MEMDUMPV2_MAGIC:
                        print_out_str("!!! Magic {0:x} doesn't match! No context will be parsed".format(dump_data_magic))
                        continue

                    getattr(DebugImage_v2, func)(
                        self, dump_data_version, dump_data_addr,
                        dump_data_addr + dump_data_len, client_id, ram_dump)
        else:
            dump_table_num_entry_offset = ram_dump.field_offset(
                'struct md_table', 'num_regions')
            dump_table_entry_offset = ram_dump.field_offset(
                'struct md_table', 'entry')
            dump_entry_name_offset = ram_dump.field_offset(
                'struct md_region', 'name')
            dump_entry_id_offset = ram_dump.field_offset(
                'struct md_region', 'id')
            dump_entry_va_offset = ram_dump.field_offset(
                'struct md_region', 'virt_addr')
            dump_entry_pa_offset = ram_dump.field_offset(
                'struct md_region', 'phys_addr')
            dump_entry_size_offset = ram_dump.field_offset(
                'struct md_region', 'size')

            dump_entry_size = ram_dump.sizeof('struct md_region')

            mem_dump_data = ram_dump.address_of('minidump_table')

            mem_dump_table = ram_dump.read_word(
                mem_dump_data + dump_table_entry_offset)

            mem_table_num_entry = ram_dump.read_u32(
                mem_dump_data + dump_table_num_entry_offset)

            print_out_str('--------')

            for i in range(0, mem_table_num_entry):
                this_entry = mem_dump_data + dump_table_entry_offset + \
                             i * dump_entry_size
                entry_id = ram_dump.read_u32(this_entry + dump_entry_id_offset)
                entry_va_addr = ram_dump.read_u64(this_entry + dump_entry_va_offset)
                entry_pa_addr = ram_dump.read_u64(this_entry + dump_entry_pa_offset)
                entry_size = ram_dump.read_u64(this_entry + dump_entry_size_offset)

                if entry_id < 0 or entry_id > len(self.dump_table_id_lookup_table):
                    print_out_str(
                        '!!! Invalid dump table entry id found {0:x}'.format(entry_id))
                    continue
                end_addr = entry_pa_addr + entry_size
                minidump_dump_table_value = dict(minidump_dump_table_type)
                if entry_pa_addr in ram_dump.ebi_pa_name_map:
                    section_name = ram_dump.ebi_pa_name_map[entry_pa_addr]
                    section_name = re.sub("\d+", "", section_name)
                    #if section_name in minidump_dump_table_value.values():
                    lst = self.minidump_data_clients(
                        ram_dump, entry_id,entry_pa_addr,end_addr)
                    if lst:
                        client_name, client_id,func,\
                            client_entry,client_end = lst[0]
                        print_out_str('--------')
                        getattr(DebugImage_v2, func)(
                            self, 20, client_entry,
                            client_end, client_id, ram_dump)

        self.parse_dcc(ram_dump)
        if ram_dump.sysreg:
            self.parse_sysreg(ram_dump)
        self.qdss.dump_standard(ram_dump)
        if not ram_dump.skip_qdss_bin:
            self.qdss.save_etf_bin(ram_dump)
            self.qdss.save_etf_swao_bin(ram_dump)
            self.qdss.save_etr_bin(ram_dump)
        if ram_dump.qtf:
            self.parse_qtf(ram_dump)

