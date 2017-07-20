obj-$(CONFIG_MTK_MMC) += dbg.o
obj-$(CONFIG_MTK_MMC) += sd.o mt_sd_misc.o


ifeq ($(CONFIG_MTK_AEE_KDUMP),y)
obj-$(CONFIG_MTK_MMC) += mt_dump.o
endif
ifeq ($(MTK_EMMC_SUPPORT_OTP),yes)
#TN801031 songyutao add
#obj-$(CONFIG_MTK_MMC) += emmc_otp.o
#lib-y	+= emmc_otp.o
#TN801031 add
endif
