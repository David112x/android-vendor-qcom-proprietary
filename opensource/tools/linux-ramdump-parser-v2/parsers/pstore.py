# Copyright (c) 2018, The Linux Foundation. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 and
# only version 2 as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

from parser_util import cleanupString
from parser_util import register_parser, RamParser
from print_out import print_out_str
import re
import collections

@register_parser('--pstore', 'Extract event logs from pstore')
class PStore(RamParser):

    def calculate_percpu_eventbuf_size(self, base_addr):
        try:
            event_zone_addr = self.ramdump.read_u64(base_addr +
                              self.ramdump.field_offset('struct ramoops_context', 'eprzs'))
            event_zone_addr = self.ramdump.read_u64(event_zone_addr)
            start_addr = self.ramdump.read_u32(event_zone_addr +
                         self.ramdump.field_offset('struct persistent_ram_zone', 'paddr'))
            percpu_size = self.ramdump.read_u32(event_zone_addr +
                          self.ramdump.field_offset('struct persistent_ram_zone', 'size'))
        except:
            return None, None
        return start_addr, percpu_size

    def print_console_logs(self, pstore_out, addr, size):
        pstore = self.ramdump.read_physical(addr, size)
        pstore_out.write(cleanupString(pstore.decode('ascii', 'ignore')) + '\n')

    def sort_event_data(self, event_data, pstore_out):
        '''
        Event log buffer is circular, so this function
        sorts the logs in ascending timestamp manner
        '''
        ordered_event_data = collections.OrderedDict(sorted(event_data.items()))
        for ts, log in ordered_event_data.iteritems():
            pstore_out.write(log)

    def print_event_logs(self, pstore_out, addr, size):
        '''
        This function tries to format the event logs before logging them to
        the core specific rtb file. Raw Data will be in the format given below
        io_read: type=readl cpu=1 ts:58270610802 data=0xffffff8009de8614
                 caller=qcom_geni_serial_start_tx+0x114/0x150
        io_write: type=writel cpu=1 ts:58270618875 data=0xffffff8009de880c
                 caller=qcom_geni_serial_start_tx+0x130/0x150

        Timestamp is extracted from raw data and converted to secs format and cpu
        field is removed since it is redundant.

        Final Formatting will be as shown below
        [644.279442] io_write : writel from address 0xffffff8009673120(None) called
                     from qcom_cpufreq_hw_target_index+0x60/0x64
        '''
        pstore = self.ramdump.read_physical(addr, size)
        event_log_data = cleanupString(pstore.decode('ascii', 'ignore'))
        event_data = event_log_data.split('\n')
        formatted_event_data = {}
        for line in event_data:
            expr = r'.*(io_.*):.*type=(.*)cpu=(.*)ts:(.*)data=(.*)caller=(.*).*'
            regEx = re.search(expr, line)
            if regEx:
                event_type = regEx.group(2).strip()
                timestamp = regEx.group(4).strip()
                timestamp = round(float(timestamp)/10**9,9)
                timestamp = format(timestamp,'.9f')
                data = regEx.group(5).strip()
                caller = regEx.group(6).strip()
                log_string = "[{0}] {1} : {2} from address {3} called from {4}\n".format(timestamp,
                              regEx.group(1), event_type, data, caller)
                formatted_event_data[timestamp] = log_string
            else:
                continue
        self.sort_event_data(formatted_event_data, pstore_out)

    def calculate_console_size(self, base_addr):
        console_zone_addr = self.ramdump.read_u64(base_addr +
                            self.ramdump.field_offset('struct ramoops_context', 'cprz'))
        start_addr = self.ramdump.read_u32(console_zone_addr +
                     self.ramdump.field_offset('struct persistent_ram_zone', 'paddr'))
        console_size = self.ramdump.read_u32(console_zone_addr +
                       self.ramdump.field_offset('struct persistent_ram_zone', 'size'))
        return start_addr, console_size

    def extract_console_logs(self, base_addr):
        '''
        Parses the console logs from pstore
        '''
        start_addr, console_size = self.calculate_console_size(base_addr)
        pstore_out = self.ramdump.open_file('console_logs.txt')
        self.print_console_logs(pstore_out, start_addr, console_size)
        pstore_out.close()

    def extract_io_event_logs(self, base_addr):
        '''
        Parses the RTB data (register read/writes) stored in the persistent
        ram zone. Data is extracted on per cpu basis into separate per core
        files.
        '''
        start_addr, percpu_size = self.calculate_percpu_eventbuf_size(base_addr)
        if start_addr is None:
            return
        nr_cpus = self.ramdump.get_num_cpus()
        for cpu in range(0,nr_cpus):
            pstore_out = self.ramdump.open_file('msm_rtb{0}.txt'.format(cpu))
            cpu_offset = percpu_size*cpu
            self.print_event_logs(pstore_out, start_addr+cpu_offset, percpu_size)
            pstore_out.close()

    def print_clockdata_info(self):
        '''
        Extracts the epoch data from clock data struct. This helps in converting
        HLOS timestamps to non-HLOS timestamps and vica-versa. This also
        additionally logs the Linux Banner for ease in post-processing.
        '''
        out_file = self.ramdump.open_file('epoch_info.txt')
        banner_addr = self.ramdump.address_of('linux_banner')
        if banner_addr is not None:
            banner_addr = self.ramdump.kernel_virt_to_phys(banner_addr)
            vm_v = self.ramdump.gdbmi.get_value_of_string('linux_banner')
            if vm_v is not None:
                out_file.write('Linux Banner : {}\n'.format(vm_v))
        epoch_ns = self.ramdump.read_word('cd.read_data[0].epoch_ns')
        epoch_cyc = self.ramdump.read_word('cd.read_data[0].epoch_cyc')
        out_file.write('\nepoch_ns: {0}ns  epoch_cyc: {1}\n'.format(epoch_ns,epoch_cyc))
        out_file.close()

    def parse(self):
        base_addr = self.ramdump.address_of('oops_cxt')
        self.extract_io_event_logs(base_addr)
        self.extract_console_logs(base_addr)
        self.print_clockdata_info()

