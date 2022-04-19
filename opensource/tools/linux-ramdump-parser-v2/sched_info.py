# Copyright (c) 2019, The Linux Foundation. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 and
# only version 2 as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

from parser_util import register_parser, RamParser
from print_out import print_out_str

DEFAULT_MIGRATION_NR=32
DEFAULT_MIGRATION_COST=500000

def mask_bitset_pos(cpumask):
    obj = [i for i in range(cpumask.bit_length()) if cpumask & (1<<i)]
    if len(obj) == 0:
        return None
    else:
        return obj

def verify_active_cpus(ramdump):
    cpu_topology_addr = ramdump.address_of('cpu_topology')
    cpu_topology_size = ramdump.sizeof('struct cpu_topology')

    if (ramdump.kernel_version >= (4, 19, 0)):
        cluster_id_off = ramdump.field_offset('struct cpu_topology', 'package_id')
        core_sib_off = ramdump.field_offset('struct cpu_topology', 'core_possible_sibling')
    else:
        cluster_id_off = ramdump.field_offset('struct cpu_topology', 'cluster_id')
        core_sib_off = ramdump.field_offset('struct cpu_topology', 'core_sibling')

    if (ramdump.kernel_version >= (4, 9, 0)):
        cpu_online_bits = ramdump.read_word('__cpu_online_mask')
    else:
        cpu_online_bits = ramdump.read_word('cpu_online_bits')
    if (ramdump.kernel_version >= (4, 9, 0)):
        cpu_isolated_bits = ramdump.read_word('__cpu_isolated_mask')
    elif (ramdump.kernel_version >= (4, 4, 0)):
        cpu_isolated_bits = ramdump.read_word('cpu_isolated_bits')
    else:
        cpu_isolated_bits = 0

    if(cpu_isolated_bits is None): #QCS404/None handling
        cpu_isolated_bits = 0

    nr_cpus = ramdump.get_num_cpus()

    if (cluster_id_off is None):
        print_out_str("\n Invalid cluster topology detected\n")

    # INFO: from 4.19 onwards, core_sibling mask contains only online cpus,
    #       find out cluster cpus dynamically.

    cluster_nrcpus = [0]
    for j in range(0, nr_cpus):
        c_id = ramdump.read_int(cpu_topology_addr + (j * cpu_topology_size) + cluster_id_off)
        if len(cluster_nrcpus) <= c_id :
            cluster_nrcpus.extend([0])
        cluster_nrcpus[c_id] += 1

    next_cluster_cpu = 0
    for i in range(0, len(cluster_nrcpus)):
        cluster_cpus = ramdump.read_word(cpu_topology_addr +
                                        (next_cluster_cpu * cpu_topology_size) + core_sib_off)
        cluster_online_cpus = cpu_online_bits & cluster_cpus
        cluster_nr_oncpus = bin(cluster_online_cpus).count('1')
        cluster_isolated_cpus = cpu_isolated_bits & cluster_cpus
        cluster_nr_isocpus = bin(cluster_isolated_cpus).count('1')

        #print_out_str("Cluster fist cpu {0} cpu_mask {1:b}".format(next_cluster_cpu , cluster_cpus))
        next_cluster_cpu += cluster_nrcpus[i]

        if (cluster_nrcpus[i] > 2):
            min_req_cpus = 2
        else:
            min_req_cpus = 1

        if ((cluster_nr_oncpus - cluster_nr_isocpus) < min_req_cpus):
                print_out_str("\n" + "*" * 10 + " WARNING " + "*" * 10 + "\n")
                print_out_str("\tMinimum active cpus are not available in the cluster {0}\n".format(i))

                print_out_str("\tCluster cpus: {0}  Online cpus: {1} Isolated cpus: {2}\n".format(
                                mask_bitset_pos(cluster_cpus),
                                mask_bitset_pos(cluster_online_cpus),
                                mask_bitset_pos(cluster_isolated_cpus)))
                print_out_str("*" * 10 + " WARNING " + "*" * 10 + "\n")

@register_parser('--sched-info', 'Verify scheduler\'s various parameter status')
class Schedinfo(RamParser):
    def parse(self):
        # Active cpu check verified by default!
        #verify_active_cpus(self.ramdump)

        # verify nr_migrates
        sched_nr_migrate = self.ramdump.read_u32('sysctl_sched_nr_migrate')
        if (sched_nr_migrate != DEFAULT_MIGRATION_NR):
            print_out_str("*" * 5 + " WARNING:" + "\n")
            print_out_str("\t sysctl_sched_nr_migrate has changed!!\n")
            print_out_str("\t If it is single digit, scheduler's load balancer has broken in the dump\n")

        # verify migration cost
        sched_migration_cost = self.ramdump.read_u32('sysctl_sched_migration_cost')
        if (sched_migration_cost != DEFAULT_MIGRATION_COST):
            print_out_str("*" * 5 + " WARNING:" + "\n")
            print_out_str("\t sysctl_sched_migration_cost has changed!!\n")
            print_out_str("\t\tDefault: 500000 and Value in dump:{0}\n".format(sched_migration_cost))

        # verify CFS BANDWIDTH enabled
        cfs_bandwidth_enabled = self.ramdump.read_u32('sysctl_sched_cfs_bandwidth_slice')
        if cfs_bandwidth_enabled is not None:
            print_out_str("*" * 5 + " INFORMATION:" + "\n")
            print_out_str("\tCFS_BANDWIDTH is enabled in the dump!!\n")
            print_out_str("\tBandwidth slice: {0}\n".format(cfs_bandwidth_enabled))

        # verify rq root domain
        if (self.ramdump.kernel_version >= (4, 9, 0)):
            cpu_online_bits = self.ramdump.read_word('__cpu_online_mask')
        else:
            cpu_online_bits = self.ramdump.read_word('cpu_online_bits')

        runqueues_addr = self.ramdump.address_of('runqueues')
        rd_offset = self.ramdump.field_offset('struct rq', 'rd')
        sd_offset = self.ramdump.field_offset('struct rq', 'sd')
        def_rd_addr = self.ramdump.address_of('def_root_domain')
        for cpu in (mask_bitset_pos(cpu_online_bits)):
            rq_addr = runqueues_addr + self.ramdump.per_cpu_offset(cpu)
            rd = self.ramdump.read_word(rq_addr + rd_offset)
            sd = self.ramdump.read_word(rq_addr + sd_offset)
            if rd == def_rd_addr :
                print_out_str("*" * 5 + " WARNING:" + "\n")
                print_out_str("Online cpu:{0} has attached to default sched root domain {1:x}\n".format(cpu, def_rd_addr))
            if sd == 0 or sd == None:
                print_out_str("*" * 5 + " WARNING:" + "\n")
                print_out_str("Online cpu:{0} has Null sched_domain!!\n".format(cpu))
