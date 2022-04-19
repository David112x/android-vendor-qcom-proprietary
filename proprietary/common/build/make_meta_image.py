# ===============================================================================
#
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#
# ===============================================================================

import os
import sys
import subprocess
import shutil
import json
import logging
import re
from argparse import ArgumentParser


logger = logging.getLogger(__name__)


class APPS_META:
    def __init__(self, meta_path, dest_path, meta_tool, storage_type, img_version):
        self.meta_path = meta_path
        self.dest_path = dest_path
        self.meta_tool = meta_tool
        self.storage_type = storage_type
        self.img_version = img_version
         # meta script to invoke
        self.mscript = os.path.join(meta_path, 'common/build/app/meta_cli.py')
        if not os.path.isfile(self.mscript):
            logger.info('ERROR: Cannot find meta file path')
            raise Exception('ERROR: Cannot find meta file path {}'.format(mscript))


    def generate_image(self, image_map, image_name, image_path):
        image = "{0}/{1}".format(image_path, image_name)
        logger.info('Generating Image : %s', image)
        cmdstr = "{0} {1} -o {2}".format(self.meta_tool, image_map, image)
        if self.img_version:
            cmdstr = "{0} {1} {2} {3} -o {4}".format(self.meta_tool, image_map, '-v', self.img_version, image)
        logger.info('Executing: %s', cmdstr)
        output = subprocess.check_call(cmdstr.split())


    def generate_bootloader_radio_images(self):
        # get binary paths
        try:
            fblist = subprocess.check_output([self.mscript, 'get_file_vars', 'attr="fastboot"', "storage={0}".format(self.storage_type)])
            fbclist = subprocess.check_output([self.mscript, 'get_file_vars', 'attr="fastboot_complete"', "storage={0}".format(self.storage_type)])
            app_id = subprocess.check_output([self.mscript, 'get_build_id', 'tag="common"'])
            logger.info("APP ID:%s", app_id)
        except subprocess.CalledProcessError, e:
            output = e.output.strip()
            logger.info(output)
            raise Exception('get_files Failed for fastboot')

        # load fastboot images
        fbdict = json.loads(fblist)

        #load fastboot complete images
        fbcdict = json.loads(fbclist)

        # combine fastboot and complete lists
        fbdict = dict(fbdict.items() + fbcdict.items())

        fbcdict = {}
        # clear any true values
        for partition_name, partition_path in fbdict.iteritems():
            if partition_name == "true":
                for item in partition_path[:]:
                    new_partition_name = (item.split("/")[-1]).split(".")[0]
                    fbcdict[new_partition_name]=[item]
            else:
               fbcdict[partition_name]=partition_path


        # remove app image pairs (system.img,userdata,img) from the dict.
        fbcdict = {partition_name:partition_path for partition_name, partition_path in fbcdict.items() if not partition_path[0].endswith('.img')}

        radio_map = ""
        bootloader_map = ""

        for partition_name, partition_path in fbcdict.iteritems() :
            if partition_name.endswith('_a'):
                line = "{0} {1} ".format(partition_name.split('_a')[0], partition_path[0])
            else:
                line = "{0} {1} ".format(partition_name, partition_path[0])
            if partition_name.startswith('modem'):
                radio_map = ''.join([radio_map, line])
            elif partition_name.startswith('abl'):
                bootloader_map = ''.join([bootloader_map, " abl {0}/signed/abl.elf ".format(self.dest_path)])
            elif partition_name.startswith('aboot'):
                bootloader_map = ''.join([bootloader_map, " aboot {0}/signed/emmc_appsboot.mbn ".format(self.dest_path)])
            else:
                bootloader_map = ''.join([bootloader_map, line])

        # adding persist.img as part of bootlaoder.img to support specific customer requirement
        # This persist should be part of bootloader.img only for flashing  brand new or
        # bricked device. Please do not include persist.img while generating bootloader.img for
        # incremental flashing or reflashing or OTA update
        persist_image = "{0}/persist.img".format(self.dest_path)
        if not os.path.exists(persist_image):
            logger.info('Persist image not exists... !')
        else:
            line = "persist {0} ".format(persist_image)
            bootloader_map = ''.join([bootloader_map, line])

        # adding mdtp.img to bootlaoder.map
        mdtp_image = "{0}/mdtp.img".format(self.dest_path)
        if not os.path.exists(mdtp_image):
            logger.info('MDTP image not exists... !')
        else:
            line = "mdtp {0} ".format(mdtp_image)
            bootloader_map = ''.join([bootloader_map, line])

        logger.info('\nRADIO MAP:')
        logger.info(radio_map)

        logger.info('\nBOOTLOADER_MAP:')
        logger.info(bootloader_map)
        self.generate_image(radio_map, "radio.img", self.dest_path)
        self.generate_image(bootloader_map, "bootloader.img", self.dest_path)


def main():
    #parsing input args
    parser = ArgumentParser(description='Get Recent Metabuilds')
    parser.add_argument(
        '--meta_path', required=True,
        help='Meta folder base path e.g. \'SDM845.LA.2.0-00022-STD.INT-1\'')
    parser.add_argument(
        '--dest_path',
        default='$(PRODUCT_OUT)',
        help='Location where meta bins are copied.')
    parser.add_argument(
        '--meta_tool',
        default='$(HOST_OUT_EXECUTABLES)/meta_image',
        help='Meta image generation tool')
    parser.add_argument(
        '--storage_type',
        default='emmc',
        help='target storage type')
    parser.add_argument(
        '--img_version',
        default='',
        help='target image version')


    args = parser.parse_args()

    #logging configuration
    log_file = "{0}/meta.log".format(args.dest_path)
    logging.basicConfig(filename=log_file, filemode='w', level=logging.INFO)
    logging.info('Generating Meta Images')

    logger = logging.getLogger('apps_meta')
    fh = logging.FileHandler(log_file)
    fh.setLevel(logging.DEBUG)
    logger.addHandler(fh)

    #image version check
    img_version_pattern = re.compile('^[a-zA-Z0-9]+([\.|_|-]?[a-zA-Z0-9]+)*$')
    if args.img_version:
        if not img_version_pattern.match(args.img_version):
            logger.info('ERROR: invalid image version')
            raise Exception('ERROR: Invalid image version {}. plase check'.format(args.img_version))

    # meta generation
    apps_meta = APPS_META(args.meta_path, args.dest_path, args.meta_tool, args.storage_type,args.img_version)
    if not os.path.exists(args.dest_path):
        os.makedirs(args.dest_path)
    apps_meta.generate_bootloader_radio_images()


if __name__ == '__main__':
    main()
