# Copyright (c) 2017, The Linux Foundation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#
#   * Redistributions in binary form must reproduce the above
#     copyright notice, this list of conditions and the following
#     disclaimer in the documentation and/or other materials provided
#     with the distribution.
#
#   * Neither the name of The Linux Foundation nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
# ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import parser_util
import local_settings
import os
import subprocess

from parser_util import register_parser, RamParser
from print_out import print_out_str
from tempfile import NamedTemporaryFile

@register_parser(
    '--dump-ftrace',
    'Use \'crash\' to extract ftrace and trace-cmd to parse it.',
    optional=True)
class FtraceParser(RamParser):

    def __init__(self, *args):
        super(FtraceParser, self).__init__(*args)

    def parse(self):
        if parser_util.get_system_type() != 'Linux':
            print_out_str("Run the ramdump parser on Linux to get ftrace")
            return False

        try:
            crashtool = local_settings.crashtool
            trace_ext = local_settings.trace_ext
            tracecmdtool = local_settings.tracecmdtool
        except AttributeError:
            print_out_str("One of crashtool, the trace extension or" +
                          " trace-cmd is missing from local-settings.py")
            return False

        if not os.path.exists(crashtool):
            print_out_str("Couldn't find the crash tool")
            return False
        if not os.path.exists(trace_ext):
            print_out_str("Couldn't find the crash tool trace extension")
            return False
        if not os.path.exists(tracecmdtool):
            print_out_str("Couldn't find the trace-cmd tool")
            return False

        print_out_str(crashtool)
        dumps=""
        for (f, start, end, filename) in self.ramdump.ebi_files:
                if "DDR" in filename or "dram" in filename:
                    dumps += '{0}@0x{1:x},'.format(filename, start)
        pagesize = "-p 4096"

        commandsfile = NamedTemporaryFile(mode='w', delete=False,
                              dir=self.ramdump.outdir)
        commandsfile.write("extend " + trace_ext + "\n")
        commandsfile.write("trace dump -t " + self.ramdump.outdir +
                       "/rawtracedata\n")
        commandsfile.write("quit\n")
        commandsfile.close()

        commands = "-i " + commandsfile.name

        crashargs = [crashtool]

        kaslr_offset = self.ramdump.get_kaslr_offset()
        if kaslr_offset != 0:
            kaslroffset = "--kaslr={0}".format(hex(kaslr_offset))
            crashargs.append(kaslroffset)

        if self.ramdump.kimage_voffset is not None:
            kimagevoff="kimage_voffset={0}".format(hex(self.ramdump.kimage_voffset).replace('L',''))
            crashargs.append("--machdep")
            crashargs.append(kimagevoff)

        crashargs.extend([dumps, self.ramdump.vmlinux,
                     pagesize, commands])

        print_out_str('args to crash: {0}'.format(crashargs))

        sp = subprocess.Popen(crashargs,
                              stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE)
        out, err = sp.communicate()

        if out:
             print_out_str("crash standard output recorded.")
             std = self.ramdump.open_file('stdout_crashtool.txt')
             std.write(out);
             std.close();
        if err:
             print_out_str("crash standard error recorded.")
             std = self.ramdump.open_file('stderr_crashtool.txt')
             std.write(err);
             std.close();

        os.remove(commandsfile.name)

        if not os.path.exists(self.ramdump.outdir + "/rawtracedata"):
             print_out_str("crash failed to extract raw ftrace data")
             return False

        tracecmd_arg = self.ramdump.outdir + "/rawtracedata"
        sp = subprocess.Popen([tracecmdtool, "report", "-l", tracecmd_arg],
                              stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE)
        out, err = sp.communicate();

        if out:
             ftrace_out = self.ramdump.open_file('ftrace.txt')
             ftrace_out.write(out);
             ftrace_out.close();
             print_out_str("Ftrace successfully extracted.");
        if err:
             print_out_str("trace-cmd standard error recorded.")
             std = self.ramdump.open_file('stderr_tracecmd.txt')
             std.write(err);
             std.close();

        return True
