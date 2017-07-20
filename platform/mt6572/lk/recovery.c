/*
 * (C) Copyright 2008
 * MediaTek <www.mediatek.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <sys/types.h>
#include <debug.h>
#include <err.h>
#include <reg.h>

#include <platform/mt_typedefs.h>
#include <platform/boot_mode.h>
#include <platform/mt_reg_base.h>
#include <platform/mtk_key.h>
#include <target/cust_key.h>
#include <platform/recovery.h>
#include <platform/mt_rtc.h>
#include <mt_partition.h>

#ifndef MTK_EMMC_SUPPORT
#ifdef MTK_SPI_NAND_SUPPORT
#include "snand_device_list.h"
#else
#include "nand_device_list.h"
#endif
#endif

extern int mboot_recovery_load_misc(unsigned char *misc_addr, unsigned int size);

#define MODULE_NAME "[RECOVERY]"

//#define LOG_VERBOSE

//#define MSG printf
#define MSG

#ifdef LOG_VERBOSE
static void dump_data(const char *data, int len) {
    int pos;
    for (pos = 0; pos < len; ) {
        MSG("%05x: %02x", pos, data[pos]);
        for (++pos; pos < len && (pos % 24) != 0; ++pos) {
            MSG(" %02x", data[pos]);
        }
        MSG("\n");
    }
}
#endif

BOOL recovery_check_key_trigger(void)
{
    //wait
    ulong begin = get_timer(0);
	//Start modified by mickal.ma
    //if it equal to menu key, skip the key check.
    /*if(MT65XX_RECOVERY_KEY == MT65XX_BOOT_MENU_KEY)
    {
      return false;
    }*/
    //End modified by mickal.ma
	printf("\n%s Check recovery boot\n",MODULE_NAME);
	printf("%s Wait 50ms for special keys\n",MODULE_NAME);

	if(mtk_detect_pmic_just_rst())
	{
	  return false;
	}
	
#ifdef MT65XX_RECOVERY_KEY
    while(get_timer(begin)<50)
    {
    	if(mtk_detect_key(MT65XX_RECOVERY_KEY))
    	{
    		printf("%s Detect cal key\n",MODULE_NAME);
    		printf("%s Enable recovery mode\n",MODULE_NAME);
    		g_boot_mode = RECOVERY_BOOT;
    		//video_printf("%s : detect recovery mode !\n",MODULE_NAME);
    		return TRUE;
    	}
    }
#endif

    return FALSE;
}

#ifndef MTK_EMMC_SUPPORT
#ifdef MTK_SPI_NAND_SUPPORT
extern snand_flashdev_info devinfo;
#else
extern flashdev_info devinfo;
#endif
#endif

BOOL recovery_check_command_trigger(void)
{
	struct misc_message misc_msg;
	struct misc_message *pmisc_msg = &misc_msg;
#ifndef MTK_EMMC_SUPPORT
	const unsigned int size = (devinfo.pagesize) * MISC_PAGES;
#else
	const unsigned int size = 2048 * MISC_PAGES;
#endif
	unsigned char *pdata;
    	int ret;

	pdata = (uchar*)malloc(sizeof(uchar)*size);

	ret = mboot_recovery_load_misc(pdata, size);

    if (ret < 0)
    {
    	return FALSE;
    }

#ifdef LOG_VERBOSE
    MSG("\n--- get_bootloader_message ---\n");
    dump_data(pdata, size);
    MSG("\n");
#endif

#ifndef MTK_EMMC_SUPPORT //wschen 2012-01-12 eMMC did not need 2048 byte offset
	memcpy(pmisc_msg, &pdata[(devinfo.pagesize) * MISC_COMMAND_PAGE], sizeof(misc_msg));
#else
	memcpy(pmisc_msg, pdata, sizeof(misc_msg));
#endif
	MSG("Boot command: %.*s\n", sizeof(misc_msg.command), misc_msg.command);
	MSG("Boot status: %.*s\n", sizeof(misc_msg.status), misc_msg.status);
	MSG("Boot message\n\"%.20s\"\n", misc_msg.recovery);

	if(strcmp(misc_msg.command, "boot-recovery")==0)
	{
	  g_boot_mode = RECOVERY_BOOT;
           return TRUE;
	}

	return FALSE;
}

/**********************************************************
 * Routine: recovery_detection
 *
 * Description: check recovery mode
 *
 * Notice: the recovery bits of RTC PDN1 are set as 0x10 only if
 *			(1) user trigger factory reset
 *
 **********************************************************/
BOOL recovery_detection(void)
{
	//if ((DRV_Reg16(RTC_PDN1) & 0x0030) == 0x0010) {	/* factory data reset */
	if(Check_RTC_Recovery_Mode())
	{	
		g_boot_mode = RECOVERY_BOOT;
		return TRUE;
	}
	//Start modified by mickal.ma
    /*if(recovery_check_key_trigger())
    {
    	return TRUE;
    }*/
    //End modified by mickal.ma
     return recovery_check_command_trigger();

/*
	recovery_check_command_trigger();


	if (g_boot_mode == RECOVERY_BOOT)
	{
	  return TRUE;
	}
	else
	{
	  return FALSE;
	}
*/

}



