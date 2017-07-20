cmd_mediatek/custom/out/wiko/kernel/accelerometer/kxtj2_cust_acc.o := arm-linux-androideabi-gcc -Wp,-MD,mediatek/custom/out/wiko/kernel/accelerometer/.kxtj2_cust_acc.o.d  -nostdinc -isystem /home/ubuntu/Project3/WIKO/S9111_WIKO/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.6/bin/../lib/gcc/arm-linux-androideabi/4.6.x-google/include -I/home/ubuntu/projects/kernal_source/S9203/alps/kernel/arch/arm/include -Iarch/arm/include/generated -Iinclude  -include /home/ubuntu/projects/kernal_source/S9203/alps/kernel/include/linux/kconfig.h -I../mediatek/custom///common -I../mediatek/platform/mt6589/kernel/core/include/ -I../mediatek/kernel/include/ -I../mediatek/custom/wiko/common/ -I../mediatek/custom/out/wiko/kernel/dct/ -I../mediatek/custom/out/wiko/kernel/alsps/ -I../mediatek/custom/out/wiko/kernel/vibrator/ -I../mediatek/custom/out/wiko/kernel/battery/ -I../mediatek/custom/out/wiko/kernel/accelerometer/ -I../mediatek/custom/out/wiko/kernel/core/ -I../mediatek/custom/out/wiko/kernel/flashlight/ -I../mediatek/custom/out/wiko/kernel/rtc/ -I../mediatek/custom/out/wiko/kernel/kpd/ -I../mediatek/custom/out/wiko/kernel/leds/ -I../mediatek/custom/out/wiko/kernel/camera/ -I../mediatek/custom/out/wiko/kernel/headset/ -I../mediatek/custom/out/wiko/kernel/magnetometer/ -I../mediatek/custom/out/wiko/kernel/touchpanel/ -I../mediatek/custom/out/wiko/kernel/usb/ -I../mediatek/custom/out/wiko/kernel/sound/inc/ -I../mediatek/custom/out/wiko/kernel/sound/ -I../mediatek/custom/out/wiko/kernel/alsps/inc/ -I../mediatek/custom/out/wiko/kernel/barometer/inc/ -I../mediatek/custom/out/wiko/kernel/accelerometer/inc/ -I../mediatek/custom/out/wiko/kernel/eeprom/ -I../mediatek/custom/out/wiko/kernel/eeprom/inc/ -I../mediatek/custom/out/wiko/kernel/flashlight/inc/ -I../mediatek/custom/out/wiko/kernel/imgsensor/ -I../mediatek/custom/out/wiko/kernel/imgsensor/inc/ -I../mediatek/custom/out/wiko/kernel/ssw/inc/ -I../mediatek/custom/out/wiko/kernel/ssw/ -I../mediatek/custom/out/wiko/kernel/ccci/ -I../mediatek/custom/out/wiko/kernel/cam_cal/ -I../mediatek/custom/out/wiko/kernel/cam_cal/inc/ -I../mediatek/custom/out/wiko/kernel/hdmi/inc/ -I../mediatek/custom/out/wiko/kernel/leds/inc/ -I../mediatek/custom/out/wiko/kernel/./ -I../mediatek/custom/out/wiko/kernel/lens/ -I../mediatek/custom/out/wiko/kernel/lens/inc/ -I../mediatek/custom/out/wiko/kernel/jogball/inc/ -I../mediatek/custom/out/wiko/kernel/magnetometer/inc/ -I../mediatek/custom/out/wiko/kernel/gyroscope/inc/ -I../mediatek/custom/out/wiko/kernel/lcm/ -I../mediatek/custom/out/wiko/kernel/lcm/inc/ -I../mediatek/custom/out/wiko/hal/imgsensor/ -I../mediatek/custom/out/wiko/hal/audioflinger/ -I../mediatek/custom/out/wiko/hal/camera/ -I../mediatek/custom/out/wiko/hal/lens/ -I../mediatek/custom/out/wiko/hal/sensors/ -I../mediatek/custom/out/wiko/hal/camera/inc/ -I../mediatek/custom/out/wiko/hal/inc/ -I../mediatek/custom/out/wiko/hal/eeprom/ -I../mediatek/custom/out/wiko/hal/ant/ -I../mediatek/custom/out/wiko/hal/flashlight/ -I../mediatek/custom/out/wiko/hal/vt/ -I../mediatek/custom/out/wiko/hal/matv/ -I../mediatek/custom/out/wiko/hal/cam_cal/ -I../mediatek/custom/out/wiko/hal/combo/ -I../mediatek/custom/out/wiko/hal/fmradio/ -I../mediatek/custom/out/wiko/hal/bluetooth/ -I../mediatek/custom/out/wiko/common -D__KERNEL__ -mlittle-endian -DMODEM_3G -Imediatek/platform/mt6589/kernel/core/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -O2 -fno-pic -marm -fno-dwarf2-cfi-asm -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -DCUSTOM_WIKO_VERSION -DMTK_SPM_MCDI_ENABLE -DMTK_LCEEFT_SUPPORT -DMTK_USES_VR_DYNAMIC_QUALITY_MECHANISM -DMTK_PLATFORM_OPTIMIZE -DMTK_PQ_SUPPORT -DMTK_AUDIO_ADPCM_SUPPORT -DHAVE_ADPCMENCODE_FEATURE -DMTK_WEB_NOTIFICATION_SUPPORT -DMTK_VOICE_UNLOCK_SUPPORT -DMTK_WFD_SUPPORT -DMTK_KERNEL_POWER_OFF_CHARGING -DMTK_FLIGHT_MODE_POWER_OFF_MD -DMTK_ENABLE_MD1 -DMTK_BT_PROFILE_AVRCP -DMTK_GPS_SUPPORT -DMTK_FM_SUPPORT -DMTK_USES_HD_VIDEO -DMTK_BT_PROFILE_MANAGER -DMTK_FM_RECORDING_SUPPORT -DMTK_BT_PROFILE_HIDH -DMTK_BT_PROFILE_PBAP -DMTK_BT_PROFILE_HFP -DMTK_COMBO_QUICK_SLEEP_SUPPORT -DMTK_THEMEMANAGER_APP -DMTK_HDR_SUPPORT -DMTK_MERGE_INTERFACE_SUPPORT -DHAVE_AACENCODE_FEATURE -DMTK_WIFI_HOTSPOT_SUPPORT -DMTK_COMBO_SUPPORT -DMTK_BT_PROFILE_OPP -DMTK_2SDCARD_SWAP -DMTK_MULTI_STORAGE_SUPPORT -DCUSTOM_KERNEL_ALSPS -DMTK_ENABLE_VIDEO_EDITOR -DCUSTOM_KERNEL_ACCELEROMETER -DMTK_DUAL_MIC_SUPPORT -DMTK_WAPI_SUPPORT -DMTK_FD_SUPPORT -DMTK_BT_SUPPORT -DMTK_FACEBEAUTY_SUPPORT -DMTK_BT_40_SUPPORT -DMTK_HIGH_QUALITY_THUMBNAIL -DMTK_SPH_EHN_CTRL_SUPPORT -DHAVE_XLOG_FEATURE -DMTK_IPV6_SUPPORT -DMTK_AUTORAMA_SUPPORT -DMTK_EAP_SIM_AKA -DMTK_MATV_ANALOG_SUPPORT -DMTK_BT_PROFILE_A2DP -DCUSTOM_KERNEL_MAGNETOMETER -DMTK_BT_PROFILE_PAN -DMTK_CAMERA_APP_3DHW_SUPPORT -DMTK_WLAN_SUPPORT -DMTK_TETHERINGIPV6_SUPPORT -DMTK_AUTO_DETECT_ACCELEROMETER -DMTK_PRODUCT_INFO_SUPPORT -DMTK_M4U_SUPPORT -DMTK_BT_PROFILE_SPP -DMTK_BT_30_SUPPORT -DMTK_ION_SUPPORT -DHAVE_AWBENCODE_FEATURE -DMTK_CAMERA_BSP_SUPPORT -DMTK_FM_RX_SUPPORT -DMTK_WB_SPEECH_SUPPORT -DMTK_VT3G324M_SUPPORT -DMTK_SENSOR_SUPPORT -DMTK_EMMC_SUPPORT -DMTK_BT_21_SUPPORT -DMTK_COMBO_CORE_DUMP_SUPPORT -DMTK_DHCPV6C_WIFI -DMTK_BQ24158_SUPPORT -DHD720 -DMTK_AUDIO_BLOUD_CUSTOMPARAMETER_V4 -DMT6589 -DDUMMY_LENS -DFM50AF_IMX179 -DOV8835AF -DOV12830AF -DOV12830TRULYAF -DSSD2075_HD720_DSI_VDO_TRULY -DHX8394A_DSI_VDO -DMODEM_3G -DOV8835_MIPI_RAW -DIMX179_MIPI_RAW -DOV12830_MIPI_RAW -DOV12830_MIPI_RAW_TRULY -DMTK_MT6628 -DMT6628 -DHI253_YUV -DCONSTANT_FLASHLIGHT -DOV2659_YUV -DDUMMY_LENS -DOV8835_MIPI_RAW -DOV2659_YUV -DHI253_YUV -DIMX179_MIPI_RAW -DOV12830_MIPI_RAW -DOV12830_MIPI_RAW_TRULY -DFM_ANALOG_INPUT -DMT6628 -DMTK_GPS_MT6628 -DDUMMY_LENS -DFM50AF_IMX179 -DOV8835AF -DOV12830AF -DOV12830TRULYAF -DFM_ANALOG_OUTPUT -DMT6628_FM -DHD720 -DMTK_SIM1_SOCKET_TYPE=\"1\" -DMTK_SIM2_SOCKET_TYPE=\"1\" -DCUSTOM_KERNEL_SSW=\"ssw_generic\" -DMTK_LCM_PHYSICAL_ROTATION=\"0\" -DLCM_WIDTH=\"720\" -DMTK_SHARE_MODEM_SUPPORT=\"2\" -DMTK_NEON_SUPPORT=\"yes\" -DMTK_SHARE_MODEM_CURRENT=\"2\" -DLCM_HEIGHT=\"1280\" -Wframe-larger-than=2048 -fno-stack-protector -Wno-unused-but-set-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(kxtj2_cust_acc)"  -D"KBUILD_MODNAME=KBUILD_STR(kxtj2_cust_acc)" -c -o mediatek/custom/out/wiko/kernel/accelerometer/kxtj2_cust_acc.o mediatek/custom/out/wiko/kernel/accelerometer/kxtj2_cust_acc.c

source_mediatek/custom/out/wiko/kernel/accelerometer/kxtj2_cust_acc.o := mediatek/custom/out/wiko/kernel/accelerometer/kxtj2_cust_acc.c

deps_mediatek/custom/out/wiko/kernel/accelerometer/kxtj2_cust_acc.o := \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/arch/dma/addr/t/64bit.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  /home/ubuntu/projects/kernal_source/S9203/alps/kernel/arch/arm/include/asm/types.h \
  include/asm-generic/int-ll64.h \
  arch/arm/include/generated/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
  include/linux/posix_types.h \
  include/linux/stddef.h \
  include/linux/compiler.h \
    $(wildcard include/config/sparse/rcu/pointer.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc4.h \
  /home/ubuntu/projects/kernal_source/S9203/alps/kernel/arch/arm/include/asm/posix_types.h \
  include/asm-generic/posix_types.h \
  ../mediatek/custom/out/wiko/kernel/accelerometer/inc/cust_acc.h \
  ../mediatek/platform/mt6589/kernel/core/include/mach/mt_pm_ldo.h \
  ../mediatek/platform/mt6589/kernel/core/include/mach/mt_typedefs.h \
  include/linux/bug.h \
    $(wildcard include/config/generic/bug.h) \
  /home/ubuntu/projects/kernal_source/S9203/alps/kernel/arch/arm/include/asm/bug.h \
    $(wildcard include/config/bug.h) \
    $(wildcard include/config/thumb2/kernel.h) \
    $(wildcard include/config/debug/bugverbose.h) \
    $(wildcard include/config/arm/lpae.h) \
  include/linux/linkage.h \
  /home/ubuntu/projects/kernal_source/S9203/alps/kernel/arch/arm/include/asm/linkage.h \
  include/asm-generic/bug.h \
    $(wildcard include/config/generic/bug/relative/pointers.h) \
    $(wildcard include/config/smp.h) \

mediatek/custom/out/wiko/kernel/accelerometer/kxtj2_cust_acc.o: $(deps_mediatek/custom/out/wiko/kernel/accelerometer/kxtj2_cust_acc.o)

$(deps_mediatek/custom/out/wiko/kernel/accelerometer/kxtj2_cust_acc.o):
