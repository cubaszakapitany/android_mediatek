#!/bin/bash


echo "512+512 config start copy .."
echo "copy custom_MemoryDevice.h to mediatek/custom/s3511/preloader/inc/custom_MemoryDevice.h"
cp -f mediatek/config/s3511/config_nand_512/custom_MemoryDevice.h  mediatek/custom/s3511/preloader/inc/custom_MemoryDevice.h 
echo "copy project to mediatek/config/s3511/autoconfig/kconfig/project"
cp -f mediatek/config/s3511/config_nand_512/project mediatek/config/s3511/autoconfig/kconfig/project
echo "copy ProjectConfig.mk to mediatek/config/s3511/ProjectConfig.mk"
cp -f mediatek/config/s3511/config_nand_512/ProjectConfig.mk mediatek/config/s3511/ProjectConfig.mk

echo "512+512 config copy done!"









