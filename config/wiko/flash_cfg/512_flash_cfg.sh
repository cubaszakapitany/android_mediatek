#!/bin/bash

echo "512+512 config start copy .."

cp mediatek/config/s4011/flash_cfg/custom_MemoryDevice_512.h  mediatek/custom/s4011/preloader/inc/custom_MemoryDevice.h 
cp mediatek/config/s4011/flash_cfg/project_512 mediatek/config/s4011/autoconfig/kconfig/project
cp mediatek/config/s4011/flash_cfg/ProjectConfig_512.mk mediatek/config/s4011/ProjectConfig.mk

echo "512+512 config copy done!"









