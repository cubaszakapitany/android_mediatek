ifeq ($(MTK_SPI_NAND_SUPPORT),yes)
obj-$(CONFIG_MTK_MTD_NAND) += mtk_snand_k.o
obj-$(CONFIG_MTK_MTD_NAND) += bmt.o
obj-$(CONFIG_MTK_MTD_NAND) += partition_mt.o
else
ifneq ($(strip $(TN_DISPLAY)), yes)
obj-$(CONFIG_MTK_MTD_NAND) += mtk_nand.o
endif
obj-$(CONFIG_MTK_MTD_NAND) += bmt.o
obj-$(CONFIG_MTK_MTD_NAND) += partition_mt.o
endif
