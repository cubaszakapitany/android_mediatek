/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/byteorder/generic.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif 
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/rtpm_prio.h>
#include "tpd_custom_cy8ctst242.h"
#include <linux/i2c.h>

#include "tpd.h"
#include <cust_eint.h>
#include <mach/mt_pm_ldo.h>
#ifndef TPD_NO_GPIO 
#include "cust_gpio_usage.h"
#endif
#ifdef TPD_HAVE_BUTTON 
#include <mach/mt_boot.h>
#endif
#include <linux/miscdevice.h>


#ifdef TPD_HAVE_BUTTON 
static int boot_mode;
#endif

#define LCT_ADD_TP_VERSION		1	//gpg add 

static DEFINE_MUTEX(cyps242_sensor_mutex);
#ifdef TPD_PROXIMITY	// add by gpg
#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/wakelock.h>
#include "cyttsp.h"

static int g_bPsSensorOpen = 0;
static u8 g_nPsSensorDate = 1;
static u8 g_nTPSleep = 0;
static u8 g_nProximitySleep = 0;
static int g_bSuspend = 0;
static struct wake_lock ps_lock;

static int cyps242_enable_ps(int enable);
static void tpd_initialize_ps_sensor_function(void);
#endif /* TPD_PROXIMITY */

static u8 cyttsp_vendor_id = 0;
static u8 cyttsp_firmware_version = 0;

#define I2C_DMA_MODE	//gpg

#define FIRMWARE_UPGRADE_FAIL_FLAG 0x10//cyttsp_bootloader_data_t.bl_file

#ifdef I2C_DMA_MODE
#include <linux/dma-mapping.h>

static u8 *CTPI2CDMABuf_va = NULL;
static u32 CTPI2CDMABuf_pa = NULL;

typedef unsigned char         FTS_BYTE;     //8 bit
typedef unsigned short        FTS_WORD;    //16 bit
typedef unsigned int          FTS_DWRD;    //16 bit
typedef unsigned char         FTS_BOOL;    //8 bit
#endif /* I2C_DMA_MODE */

extern struct tpd_device *tpd;

static struct i2c_client *i2c_client = NULL;
static struct task_struct *thread = NULL;

static DECLARE_WAIT_QUEUE_HEAD(waiter);

static struct early_suspend early_suspend;

#ifdef CONFIG_HAS_EARLYSUSPEND
static void tpd_early_suspend(struct early_suspend *handler);
static void tpd_late_resume(struct early_suspend *handler);
#endif 

static int cyttsp_has_bootloader = 0;

#ifdef ENABLE_AUTO_UPDATA
static struct cyttsp_bootloader_data_t g_bl_data;
static void cyttspfw_upgrade_start(const u8 *data,int data_len, bool force);
#endif

static void tpd_eint_interrupt_handler(void);
static int tpd_get_bl_info(int show);
static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
static int __devexit tpd_remove(struct i2c_client *client);
static int touch_event_handler(void *unused);
static int tpd_initialize(struct i2c_client * client);

enum wk_wdt_type {
	WK_WDT_LOC_TYPE,
	WK_WDT_EXT_TYPE,
	WK_WDT_LOC_TYPE_NOLOCK,
	WK_WDT_EXT_TYPE_NOLOCK,
};
extern void mtk_wdt_restart(enum wk_wdt_type type);
volatile static int tpd_flag = 0;

#ifdef TPD_HAVE_BUTTON 
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;
#endif

#define TPD_OK 					0
//#define TPD_EREA_Y                             799
//#define TPD_EREA_X                             479
#define TPD_EREA_Y                             479
#define TPD_EREA_X                             319

#define TPD_DISTANCE_LIMIT                     100

#define TPD_REG_BASE 0x00
#define TPD_SOFT_RESET_MODE 0x01
#define TPD_OP_MODE 0x00
#define TPD_LOW_PWR_MODE 0x04
#define TPD_SYSINFO_MODE 0x10
#define GET_HSTMODE(reg)  ((reg & 0x70) >> 4)  // in op mode or not 
#define GET_BOOTLOADERMODE(reg) ((reg & 0x10) >> 4)  // in bl mode 
//#define GPIO_CTP_EN_PIN_M_GPIO 0
//#define GPIO_CTP_EN_PIN 0xff
// For Firmware Update 
#define CYPRESS_IOCTLID				0xD1
#define IOCTL_FW_UPDATE  			_IOR(CYPRESS_IOCTLID,  1, int)
#define IOCTL_FW_VER  				_IOR(CYPRESS_IOCTLID,  2, int)

// command sequence for exiting from bootloader mode
static u8 bl_cmd[] = {
    0x00, 0x00, 0xFF, 0xA5,
    0x00, 0x01, 0x02,
    0x03, 0x04, 0x05,
    0x06, 0x07};

struct tpd_operation_data_t{
    U8 hst_mode;
    U8 tt_mode;
    U8 tt_stat;

    U8 x1_M,x1_L;
    U8 y1_M,y1_L;
    U8 z1;
    U8 touch12_id;

    U8 x2_M,x2_L;
    U8 y2_M,y2_L;
    U8 z2;
    U8 gest_cnt;
    U8 gest_id;
    U8 gest_set; 


    U8 x3_M,x3_L;
    U8 y3_M,y3_L;
    U8 z3;
    U8 touch34_id;

    U8 x4_M,x4_L;
    U8 y4_M,y4_L;
    U8 z4;
};

struct tpd_bootloader_data_t{
    U8 bl_file;
    U8 bl_status;
    U8 bl_error;
    U8 blver_hi,blver_lo;
    U8 bld_blver_hi,bld_blver_lo;

    U8 ttspver_hi,ttspver_lo;
    U8 appid_hi,appid_lo;
    U8 appver_hi,appver_lo;

    U8 cid_0;
    U8 cid_1;
    U8 cid_2;

};

struct tpd_sysinfo_data_t{
    U8   hst_mode;
    U8  mfg_cmd;
    U8  mfg_stat;
    U8 cid[3];
    u8 tt_undef1;

    u8 uid[8];
    U8  bl_verh;
    U8  bl_verl;

    u8 tts_verh;
    u8 tts_verl;

    U8 app_idh;
    U8 app_idl;
    U8 app_verh;
    U8 app_verl;

    u8 tt_undef2[6];
    U8  act_intrvl;
    U8  tch_tmout;
    U8  lp_intrvl;	 

};

struct touch_info {
    int x1, y1;
    int x2, y2;
    int p1, p2;
    int id1,id2;
    int count;
};

struct id_info{
    int pid1;
    int pid2;
    int reportid1;
    int reportid2;
    int id1;
    int id2;

};
static struct tpd_operation_data_t g_operation_data;
static struct tpd_bootloader_data_t g_bootloader_data;
static struct tpd_sysinfo_data_t g_sysinfo_data;
static const struct i2c_device_id tpd_id[] = {{TPD_DEVICE,0},{}};
#define CUST_ICS //add by zhaofei 2012-06-12 18:23:26
#ifdef CUST_ICS
static const struct i2c_device_id cyttsp_tpd_id[] = {{TPD_DEVICE,0},{}};
static struct i2c_board_info __initdata cyttsp_i2c_tpd={ I2C_BOARD_INFO(TPD_DEVICE, 0x24)};
#else
static unsigned short force[] = {0,0x48,I2C_CLIENT_END,I2C_CLIENT_END};
static const unsigned short * const forces[] = { force, NULL };
static struct i2c_client_address_data addr_data = { .forces = forces, };
#endif
int cy8ctst_get_fw();

static struct i2c_driver tpd_i2c_driver = {
    .probe = tpd_probe,
    .remove = __devexit_p(tpd_remove),
    .driver.name = "cy8ctst242", 
    .id_table = tpd_id,
    .detect = tpd_detect,
#ifndef CUST_ICS
    .address_data = &addr_data,
#endif
};

struct miscdevice firmware_Cypress;

int Cypress_iap_open(struct inode *inode, struct file *filp){ 
                 
      return 0;
}

int Cypress_iap_release(struct inode *inode, struct file *filp){    
      return 0;
}

static ssize_t Cypress_iap_write(struct file *filp, const char *buff, size_t count, loff_t *offp){  
   return 0;
}

ssize_t Cypress_iap_read(struct file *filp, char *buff, size_t count, loff_t *offp){    
   return 0;
         
}

static long Cypress_iap_ioctl(/*struct inode *inode,*/ struct file *filp,    unsigned int cmd, unsigned long arg){

    int __user *ip = (int __user *)arg;
    printk("[Cypress]into Cypress_iap_ioctl\n");
	printk("cmd value %x, %ld\n", cmd, arg);
         switch (cmd) {        
         
                   case IOCTL_FW_VER:
                            return cy8ctst_get_fw();
                            break;

                   case IOCTL_FW_UPDATE:
                           #ifdef ENABLE_AUTO_UPDATA
							cyttspfw_upgrade_start(CY242_update_bin, sizeof(CY242_update_bin), 0);	
                           #endif

                   default:            
                            break;   
         }       
         return 0;
}

struct file_operations Cypress_touch_fops = {    
        .open =            	Cypress_iap_open,    
        .write =         	Cypress_iap_write,    
        .read =          	Cypress_iap_read,    
        .release =        	Cypress_iap_release,    
        .unlocked_ioctl = 	Cypress_iap_ioctl, 
 };

#ifdef TPD_PROXIMITY
static int cyttsp_read_reg(struct i2c_client *client,u8 addr, u8 *pdata)
{
	int ret;
	u8 buf[2] = {0};
	struct i2c_msg msgs[] = {
		{
			.addr	= client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= buf,
		},
		{
			.addr	= client->addr,
			.flags	= I2C_M_RD,
			.len	= 1,
			.buf	= buf,
		},
	};

	buf[0] = addr;
	ret = i2c_transfer(client->adapter, msgs, 2);
	if (ret < 0)
	{
		TPD_DMESG("[CY8CTST242][PROXIMITY]i2c read error!ret:%d\n", ret);
		return ret;
	}

	*pdata = buf[0];
	return ret;
  
}

static int cyttsp_write_reg(struct i2c_client *client,u8 addr, u8 para)
{
	u8 buf[3];
	int ret = -1;

	//buf[0] = addr;
	//buf[1] = para;

	struct i2c_msg msg[] = {
		{
			.addr	= client->addr,
			.flags	= 0,
			.len	= 2,
			.buf	= buf,
		},
	};

	buf[0] = addr;
	buf[1] = para;

	msg[0].buf = buf;

	ret = i2c_transfer(client->adapter, msg, 1);
	if (ret < 0) {
		TPD_DMESG("[CY8CTST242][PROXIMITY]i2c write error!ret:%d,addr:%x,para:%x\n", ret,buf[0],buf[1]);
		return -1;
	}
	
	return 0;
}
#endif /* TPD_PROXIMITY */

int cy8ctst_get_fw()
{
	uint8_t fw;
	int ret = 0;
	//ret = cy8ctst_read_reg(CY8CTST_FW_REG, &fw);
	ret = cyttsp_read_reg(i2c_client, 0x11, &fw);
	if(ret < 0) {
		//mt6575_touch_info("%s: get fw version error--Liu\n", __func__);
		return -1;
	}
	return (int)fw;
}

#ifdef I2C_DMA_MODE
static int CTPDMA_i2c_write(struct i2c_client *client,FTS_BYTE* pbt_buf, FTS_DWRD dw_len)
{
	int i = 0;
	int add_bak = client->addr;
	int ret = 0;
	
	for(i=0 ; i<dw_len; i++)
	{
		CTPI2CDMABuf_va[i] = pbt_buf[i];
	}

	if(dw_len <= 8)
	{
		//i2c_client->addr = i2c_client->addr & I2C_MASK_FLAG;
		//MSE_ERR("Sensor non-dma write timing is %x!\r\n", this_client->timing);
		return i2c_master_send(client, pbt_buf, dw_len);
	}
	else
	{
		client->addr = client->addr & I2C_MASK_FLAG | I2C_DMA_FLAG;
		//MSE_ERR("Sensor dma timing is %x!\r\n", this_client->timing);
		ret = i2c_master_send(client, CTPI2CDMABuf_pa, dw_len);
		client->addr = add_bak;
		return ret;
	}
}
#endif

#if 1
#include <linux/sched.h>   //wake_up_process()  
#include <linux/kthread.h> //kthread_create()¡¢kthread_run()  
//#include <err.h> //IS_ERR()¡¢PTR_ERR()  

static void cyttsp_sw_reset(void);
static struct task_struct *esd_task;  
volatile bool need_rst_flag = 0;
volatile int tp_interrupt_flag = 0;
volatile int tp_suspend_flag = 0;
volatile int tp_reseting_flag = 0;
volatile int tp_ps_enable_flag = 0;


void cyttsp_print_reg(struct i2c_client *client)
{
    char buffer[20];
    int status1 = -1;
	int status2 = -1;
	int i;
	
    status1 = i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 8, &(buffer[0]));
    status2 = i2c_smbus_read_i2c_block_data(i2c_client, 0x08, 8, &(buffer[8]));

	TPD_DMESG("[CY8CTST242][I2C]register read status1:%x,status2:%x\n",status1,status2);
	for(i=0; i<16;i++)
		TPD_DMESG("[CY8CTST242][I2C]addr:%02x,para:%02x", i,buffer[i]);
	TPD_DMESG("\n");
}

void enter_boot_mode(void)
{
	mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	
	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
	mdelay(10);
	cyttsp_sw_reset();
	TPD_DMESG("[CY8CTST242][FIRMWARE]enter into bootloader mode!\n");
}

void exit_boot_mode(void)
{
	#if 1
	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
    mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
    mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);
	cyttsp_sw_reset();
	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	TPD_DMESG("[CY8CTST242][FIRMWARE]exit from bootloader mode!\n");
	#else
	char buffer[2];
	int status=0;
	status = i2c_smbus_read_i2c_block_data(i2c_client, 0x01, 1, &(buffer[0]));
	if (status < 0)
	{
		TPD_DMESG("[CY8CTST242][REGISTER]exit_boot_mode failed 1st\n");
	}
	else
	{
		if (buffer[0]&0x10)
		{
			#ifdef I2C_DMA_MODE 
			status = CTPDMA_i2c_write(i2c_client, bl_cmd, 12);
			#else	
			status = i2c_master_send(i2c_client, bl_cmd, 12);
			#endif
			if( status < 0)
			{
				TPD_DMESG("[CY8CTST242][REGISTER]exit_boot_mode failed 2nd\n");
			}
			else
			{
				TPD_DMESG("[CY8CTST242][REGISTER]exit_boot_mode succeeded\n");
			}
			msleep(300);
			status = i2c_smbus_read_i2c_block_data(i2c_client, 0x01, 1, &(buffer[0]));
			if (status < 0) {
				TPD_DMESG("[CY8CTST242][REGISTER]exit_boot_mode succeeded\n");
			}
//			printk("++++exit_boot_mode  set: 0x%x\n",buffer[0]);
			//cyttsp_print_reg(i2c_client);
		}
		//else
		//{
		//	printk("++++exit_boot_mode-- not in bootmode\n");
		//}
	}
	#endif
}

void esd_check(void)
{
	if(need_rst_flag)
	{
		if(tp_suspend_flag == 0)
		{
			TPD_DMESG("[CY8CTST242][ESD]esd_check\n");
			//mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);  
			tp_reseting_flag = 1;
			cyttsp_sw_reset();
			tp_reseting_flag = 0;
			//mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);  
		}
		need_rst_flag = 0;
	}
}
static int fp_count = 0;
void esd_thread(void)
{
	static int i = 0, j = 0;
	
	while(1)
	{
		TPD_DMESG("[CY8CTST242][ESD]esd_thread, need_rst_flag=%d, fp_count=%d\n", need_rst_flag,fp_count);  
		fp_count = 0;
		#if 0
		if(need_rst_flag)
		{
			j = 0;
			while(tp_interrupt_flag==1 && j<200)	//wujinyou
			{
				j ++;
				if(tp_suspend_flag)
					msleep(1000);
				else
					msleep(10);
			}
			if(tp_suspend_flag == 0)
			{
				printk("++++esd_thread, start reset, mask int\n");  
				mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);  
				tp_reseting_flag = 1;
				cyttsp_sw_reset();
				i = 0;
				need_rst_flag = 0;
				tp_reseting_flag = 0;
				mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);  
			}
		}
		#endif
		msleep(1000);
		i ++;
		if(i == 10)
		{
			i = 0;
			//cyttsp_sw_reset();
			//need_rst_flag = 1;
		}
	}
}

static int esd_init_thread(void)  
{  
	int err;
	
	TPD_DMESG("[CY8CTST242][ESD]%s, line %d----\n", __FUNCTION__, __LINE__);

	esd_task = kthread_create(esd_thread, NULL, "esd_task");  

	if(IS_ERR(esd_task)){  
		TPD_DMESG("[CY8CTST242][ESD]failed to init esd thread\n");  
		err = PTR_ERR(esd_task);  
		esd_task = NULL;  
		return err;  
	}  

	wake_up_process(esd_task);  

	return 0;
}  
#endif  

static void tpd_down(int x, int y, int p) {

fp_count++;
//printk("++++tpd_down: %d,%d,%d\n", x,  y, p);
#ifdef TPD_HAVE_BUTTON
    if (boot_mode != NORMAL_BOOT) {
	    if(y > 480) {
		    tpd_button(x, y, 1);
	    }
    }
#endif

    input_report_abs(tpd->dev, ABS_PRESSURE,p);
    input_report_key(tpd->dev, BTN_TOUCH, 1);
    //input_report_abs(tpd->dev,ABS_MT_TRACKING_ID,i);
    input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 1);
    input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
    input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
    //TPD_DEBUG("Down x:%4d, y:%4d, p:%4d \n ", x, y, p);
    input_mt_sync(tpd->dev);
    TPD_DOWN_DEBUG_TRACK(x,y);
}

static void tpd_up(int x, int y,int p) {

    input_report_abs(tpd->dev, ABS_PRESSURE, 0);
    input_report_key(tpd->dev, BTN_TOUCH, 0);
   // input_report_abs(tpd->dev,ABS_MT_TRACKING_ID,i);
    input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
    input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
    input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
    //TPD_DEBUG("Up x:%4d, y:%4d, p:%4d \n", x, y, 0);
    input_mt_sync(tpd->dev);
    TPD_UP_DEBUG_TRACK(x,y);
}
void test_retval(s32 ret)
{
	if(ret < 0)
	{
		need_rst_flag = 1;
	}
}
static int tpd_touchinfo(struct touch_info *cinfo, struct touch_info *pinfo)
{
    s32 retval;
    static u8 tt_mode;

#ifdef TPD_PROXIMITY
	if (g_bPsSensorOpen)
	{
	    extern void ctp_ps_event_handler(void);
		u8 nPsDate;
		cyttsp_read_reg(i2c_client, 0x13, &nPsDate);
		TPD_DMESG("[CY8CTST242][PROXIMITY] nPsDate=%d g_bPsSensorOpen=%d,g_nPsSensorDate=%d\n",g_bPsSensorOpen,g_nPsSensorDate,nPsDate);
		if(nPsDate)
		{		
			if(g_nPsSensorDate == 1)
			{
				mutex_lock(&cyps242_sensor_mutex);
				g_nPsSensorDate = 0;
				mutex_unlock(&cyps242_sensor_mutex);
				ctp_ps_event_handler();
			}
			return 0;			
		}
		else
		{
			//TPD_DMESG("[CY8CTST242][PROXIMITY]g_bPsSensorOpen=%d,g_nPsSensorDate=%d\n",g_bPsSensorOpen,g_nPsSensorDate );
			if(g_nPsSensorDate == 0)
			{
				mutex_lock(&cyps242_sensor_mutex);
				g_nPsSensorDate = 1;
				mutex_unlock(&cyps242_sensor_mutex);
				ctp_ps_event_handler();
			}		
		}
		/*
		if(g_nPsSensorDate != g_nPsSensorDatePre)
		{
			extern void ctp_ps_event_handler(void);
			TPD_DMESG("[CY8CTST242][PROXIMITY] ctp_ps_event_handler");
			ctp_ps_event_handler();
			g_nPsSensorDatePre=g_nPsSensorDate;
		}*/
	}
#endif /* TPD_PROXIMITY */

    memcpy(pinfo, cinfo, sizeof(struct touch_info));
    memset(cinfo, 0, sizeof(struct touch_info));
   
    //TPD_DEBUG("pinfo->x1 = %3d, pinfo->y1 = %3d,  pinfo->p1 = %3d\n", pinfo->x1, pinfo->y1,  pinfo->p1);
    //TPD_DEBUG("pinfo->x2 = %3d, pinfo->y2 = %3d,  pinfo->p2 = %3d\n", pinfo->x2, pinfo->y2,  pinfo->p2);
    retval = i2c_smbus_read_i2c_block_data(i2c_client, TPD_REG_BASE, 8, (u8 *)&g_operation_data);
	test_retval(retval);
    retval += i2c_smbus_read_i2c_block_data(i2c_client, TPD_REG_BASE + 8, 8, (((u8 *)(&g_operation_data)) + 8));
    retval += i2c_smbus_read_i2c_block_data(i2c_client, TPD_REG_BASE + 16, 8, (((u8 *)(&g_operation_data)) + 16));
    retval += i2c_smbus_read_i2c_block_data(i2c_client, TPD_REG_BASE + 24, 4, (((u8 *)(&g_operation_data)) + 24));
	test_retval(retval);

	//cyttsp_print_reg(i2c_client);
    //TPD_DEBUG("received raw data from touch panel as following:\n");

    TPD_DEBUG("hst_mode = %02X, tt_mode = %02X, tt_stat = %02X\n", \
            g_operation_data.hst_mode,\
            g_operation_data.tt_mode,\
            g_operation_data.tt_stat);

    cinfo->count = (g_operation_data.tt_stat & 0x0f) ; //point count

    TPD_DEBUG("cinfo->count =%d\n",cinfo->count);

    TPD_DEBUG("Procss raw data...\n");

    cinfo->x1 = (( g_operation_data.x1_M << 8) | ( g_operation_data.x1_L)); //point 1		
    cinfo->y1  = (( g_operation_data.y1_M << 8) | ( g_operation_data.y1_L));
    cinfo->p1 = g_operation_data.z1;

    if(cinfo->x1 < 1) cinfo->x1 = 1;
    if(cinfo->y1 < 1) cinfo->y1 = 1;

    cinfo->id1 = ((g_operation_data.touch12_id & 0xf0) >>4) -1;


    TPD_DEBUG("Before: cinfo->x1= %3d, 	cinfo->y1 = %3d, cinfo->p1 = %3d\n", cinfo->x1, cinfo->y1 , cinfo->p1);		
    TPD_DEBUG("Before: cinfo->x1= %3d, 	cinfo->y1 = %3d, cinfo->p1 = %3d\n", cinfo->x1, cinfo->y1 , cinfo->p1);		

    //cinfo->x1 =  cinfo->x1 * 480 >> 11; //calibrate
    //cinfo->y1 =  cinfo->y1 * 800 >> 11; 

    TPD_DEBUG("After:	cinfo->x1 = %3d, cinfo->y1 = %3d, cinfo->p1 = %3d\n", cinfo->x1 ,cinfo->y1 ,cinfo->p1);

    if(cinfo->count >1) {

        cinfo->x2 = (( g_operation_data.x2_M << 8) | ( g_operation_data.x2_L)); //point 2
        cinfo->y2 = (( g_operation_data.y2_M << 8) | ( g_operation_data.y2_L));
        cinfo->p2 = g_operation_data.z2;

        if(cinfo->x2< 1) cinfo->x2 = 1;
        if(cinfo->y2 < 1) cinfo->y2 = 1;

        cinfo->id2 = (g_operation_data.touch12_id & 0x0f)-1;

        TPD_DEBUG("before:	 cinfo->x2 = %3d, cinfo->y2 = %3d,  cinfo->p2 = %3d\n", cinfo->x2, cinfo->y2,  cinfo->p2);	  
        //cinfo->x2 =  cinfo->x2 * 480 >> 11; //calibrate
        //cinfo->y2 =  cinfo->y2 * 800 >> 11; 
        TPD_DEBUG("After:	 cinfo->x2 = %3d, cinfo->y2 = %3d,  cinfo->p2 = %3d\n", cinfo->x2, cinfo->y2, cinfo->p2);	
    }

    if (!cinfo->count) return true; // this is a touch-up event

    if (g_operation_data.tt_mode & 0x20) {
        TPD_DEBUG("uffer is not ready for use!\n");
        memcpy(cinfo, pinfo, sizeof(struct touch_info));
        return false;
    }//return false; // buffer is not ready for use// buffer is not ready for use

    // data toggle 
	u8 data0,data1;

	data0 = i2c_smbus_read_i2c_block_data(i2c_client, TPD_REG_BASE, 1, (u8*)&g_operation_data);
    //TPD_DEBUG("before hst_mode = %02X \n", g_operation_data.hst_mode);

	if((g_operation_data.hst_mode & 0x80)==0)
		g_operation_data.hst_mode = g_operation_data.hst_mode|0x80;
	else
		g_operation_data.hst_mode = g_operation_data.hst_mode & (~0x80);

	//TPD_DEBUG("after hst_mode = %02X \n", g_operation_data.hst_mode);
	data1 = i2c_smbus_write_i2c_block_data(i2c_client, TPD_REG_BASE, sizeof(g_operation_data.hst_mode), &g_operation_data.hst_mode);


	if (tt_mode == g_operation_data.tt_mode) {
            TPD_DEBUG("sampling not completed!\n");
            memcpy(cinfo, pinfo, sizeof(struct touch_info));
            return false; 
    }// sampling not completed
    else 
		tt_mode = g_operation_data.tt_mode;

    return true;

};

static int touch_event_handler(void *unused)
{
    struct touch_info cinfo, pinfo;
 
    struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };
    sched_setscheduler(current, SCHED_RR, &param);

    do
    {
        mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
        set_current_state(TASK_INTERRUPTIBLE); 
	tp_interrupt_flag = 0;
	//printk("++++%s, line %d----end\n", __FUNCTION__, __LINE__);	
        wait_event_interruptible(waiter,tpd_flag!=0);
	//printk("++++%s, line %d----start\n", __FUNCTION__, __LINE__);

        tpd_flag = 0;
      
        set_current_state(TASK_RUNNING);
		
		//exit_boot_mode();
        if (tpd_touchinfo(&cinfo, &pinfo)) {
            if(cinfo.count >0) {
                tpd_down(cinfo.x1, cinfo.y1, cinfo.p1);//cinfo.id1);
                if(cinfo.count == 1 && pinfo.count == 2){
                    if (cinfo.id1 == pinfo.id1 && cinfo.id1 != pinfo.id2)//(pinfo.x1 == cinfo.x2) && (pinfo.y1 ==cinfo.y2))
                        tpd_up(pinfo.x2, pinfo.y2, pinfo.p2);
                    else
                        tpd_up(pinfo.x1, pinfo.y1, pinfo.p1);
                }
                if(cinfo.count>1)
                    tpd_down(cinfo.x2, cinfo.y2, cinfo.p2);
            }else{
		#ifdef TPD_HAVE_BUTTON
				if (boot_mode != NORMAL_BOOT) {
					if(cinfo.y1  > 480) {
						tpd_button(cinfo.x1,  cinfo.y1, 0);
				}
						}
		#endif
                if(pinfo.count >0){
                    tpd_up(pinfo.x1, pinfo.y1, pinfo.p1);
                    if(pinfo.count > 1)
                        tpd_up(pinfo.x2, pinfo.y2, pinfo.p2);
                }
            }
            input_sync(tpd->dev);
        }

	esd_check();
    }while(!kthread_should_stop());
	tp_interrupt_flag = 0;

    return 0;
}

static int tpd_detect(struct i2c_client *client, int kind, struct i2c_board_info *info)
{
    strcpy(info->type, "mtk-tpd");
    return 0;
}


static void tpd_eint_interrupt_handler(void)
{
	//mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	//TPD_DMESG("[CY8CTST242][FUNCTION]tpd_flag:%d\n",tpd_flag);
	tp_interrupt_flag = 1;
	tpd_flag = 1;
	wake_up_interruptible(&waiter);
}  
static void ctp_power_on(kal_bool on)
{
	if(on)
	{		
		if(TRUE != hwPowerOn(MT6323_POWER_LDO_VGP2, VOL_2800, "TP"))
		{
			TPD_DMESG("[ctp_power_on] Fail to enable MT65XX_POWER_LDO_VGP1 power\n");
		}

		//if(TRUE != hwPowerOn(MT6323_POWER_LDO_VCAM_AF, VOL_1800,"TP"))
		//{
		//	TPD_DMESG("[ctp_power_on] Fail to enable MT6323_POWER_LDO_VCAM_AF power\n");
		//}            

	}
	else
	{
		if(TRUE != hwPowerDown(MT6323_POWER_LDO_VGP2, "TP")) {
			TPD_DMESG("[ctp_power_on] Fail to disable MT65XX_POWER_LDO_VGP1 power\n");
		}
		//if(TRUE != hwPowerDown(MT6323_POWER_LDO_VCAM_AF,"TP")) {
		//	TPD_DMESG("[ctp_power_on] Fail to disable MT6323_POWER_LDO_VCAM_AF power\n");
		//}
	}
}

void cyttsp_sw_reset(void)
{
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	mdelay(20);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);    
	mdelay(100);
}

#ifdef TPD_PROXIMITY

static u8  tst242_get_ps(void)
{
	TPD_DMESG("[CY8CTST242][PROXIMITY] tst242_get_ps %d",g_nPsSensorDate);
	return  g_nPsSensorDate;
}
static int cyps242_enable_ps(int enable)
{
	u8 ps_store_data[4];

	mutex_lock(&cyps242_sensor_mutex);

	TPD_DMESG("[CY8CTST242][PROXIMITY]enable:%d, current state:%d\n", enable, g_bPsSensorOpen);
	if(enable == 1)
	{
		if(g_bPsSensorOpen == 0)
		{	
			u8 nSetPsEnable;
			TPD_DMESG("[CY8CTST242][PROXIMITY]do enable:%d, current state:%d\n", enable, g_bPsSensorOpen);
			g_bPsSensorOpen = 1;
			nSetPsEnable = (u8)g_bPsSensorOpen;
			cyttsp_write_reg(i2c_client, 0x12, nSetPsEnable);
		}
	}
	else
	{	
		if(g_bPsSensorOpen == 1)
		{
			u8 nSetPsEnable;
			TPD_DMESG("[CY8CTST242][PROXIMITY]do enable:%d, current state:%d\n", enable, g_bPsSensorOpen);
			g_bPsSensorOpen = 0;
			nSetPsEnable = (u8)g_bPsSensorOpen;
			cyttsp_write_reg(i2c_client, 0x12, nSetPsEnable);	
		}
		//g_nPsSensorDate = 1;
	}
	mutex_unlock(&cyps242_sensor_mutex);
	return 0;
}

static void tst242_ctp_convert(int mode,bool force)
{
	if((mode==1)&&(g_nTPSleep==1))
		g_nProximitySleep = 1;
	else
		cyps242_enable_ps(mode);
}
int cyps242_ps_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value;
	hwm_sensor_data* sensor_data;

	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				err = -EINVAL;
			}
			// Do nothing
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				err = -EINVAL;
			}
			else
			{				
				value = *(int *)buff_in;
				if(value)
				{
					//wake_lock(&ps_lock);		//wujinyou
					if(err = cyps242_enable_ps(1))
					{
						return -1;
					}
					g_bPsSensorOpen = 1;
				}
				else
				{
					//wake_unlock(&ps_lock);		//wujinyou
					if(err = cyps242_enable_ps(0))
					{
						return -1;
					}
					g_bPsSensorOpen = 0;
				}
			}
			break;

		case SENSOR_GET_DATA:
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				err = -EINVAL;
			}
			else
			{
				sensor_data = (hwm_sensor_data *)buff_out;	
				sensor_data->values[0] = g_nPsSensorDate;
				sensor_data->value_divide = 1;
				sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;			
			}
			break;
		default:
			err = -1;
			break;
	}
	
	return err;
}

static void tpd_initialize_ps_sensor_function(void)
{
	struct hwmsen_object obj_ps = {0};
	int err = 0;
	
	//g_nPsSensorDate = 1;

	obj_ps.self = NULL;	// no use
	obj_ps.polling = 1;
	obj_ps.sensor_operate = cyps242_ps_operate;

//	wake_lock_init(&ps_lock,WAKE_LOCK_SUSPEND,"ps wakelock"); //shaohui add
		
	/*if(err = hwmsen_attach(ID_PROXIMITY, &obj_ps))
	{
		TPD_DMESG("[CY8CTST242][PROXIMITY]failed to init proximity function\n");
		return;
	}*/
}
#endif /* TPD_PROXIMITY */

#if LCT_ADD_TP_VERSION
#define CTP_PROC_FILE "tp_info"
static struct proc_dir_entry *g_ctp_proc = NULL;

static int ctp_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int cnt= 0;
	TPD_DMESG("[CY8CTST242][ENGINEER]Enter ctp_proc_read.\n");
	if(off != 0)
		return 0;
	cnt = sprintf(page, "cyttsp TP, vid:0x%04x,firmware:0x%04x\n",cyttsp_vendor_id, cyttsp_firmware_version);
	*eof = 1;
//	printk("vid:0x%04x,firmware:0x%04x\n",curr_ic_major, curr_ic_major);
	TPD_DMESG("[CY8CTST242][ENGINEER]Leave ctp_proc_read. cnt = %d\n", cnt);
	return cnt;
}
#endif

static struct tpd_driver_t tpd_device_driver;
static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id)
{	 

    int retval = TPD_OK;
    i2c_client = client;

    char buffer[18];
    int status=0;

	TPD_DMESG("[CY8CTST242][FUNCTION]enter %s: %d",__FUNCTION__, __LINE__);

	//ctp_power_on(1);	//wujinyou
	msleep(1000);

	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);

	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
    mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
    mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);
	msleep(200);

	status = i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 8, &(buffer[0]));
	if(status < 0) {
		TPD_DMESG("[CY8CTST242][FUNCTION]fail to prob cy8ctst242 1st\n");
		status = i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 8, &(buffer[0]));
		if(status < 0) {
			TPD_DMESG("[CY8CTST242][FUNCTION]fail to prob cy8ctst242 2nd\n");
			return status;
		}
	}

	//cyttsp_has_bootloader  = ((buffer[1] & 0x10) >> 4);
	//TPD_DMESG("[CY8CTST242][FUNCTION]cyttsp_has_bootloader=%d\n",cyttsp_has_bootloader);

#if LCT_ADD_TP_VERSION
	i2c_smbus_read_i2c_block_data(i2c_client, 0x10, 8, &(buffer[8]));
	do {
	    cyttsp_vendor_id = buffer[8];
		cyttsp_firmware_version =  buffer[9];
		TPD_DMESG("[CY8CTST242][ENGINEER]module:%x,firmware version:%x:\n",cyttsp_vendor_id,cyttsp_firmware_version);
	} while(0);
#endif

#ifdef ENABLE_AUTO_UPDATA
{
    uint8_t status;
	int ret = 0;
	ret = cyttsp_read_reg(i2c_client, 0x1, &status);
	if(status==0x10)
		cyttspfw_upgrade_start(CY242_update_bin, sizeof(CY242_update_bin), 1);
	else
		cyttspfw_upgrade_start(CY242_update_bin, sizeof(CY242_update_bin), 0);
}
#endif

//#ifdef I2C_DMA_MODE
//	status = CTPDMA_i2c_write(i2c_client, bl_cmd, 12);
//#else
//	status = i2c_master_send(i2c_client, bl_cmd, 12);
//#endif
//	if( status < 0)
//	{
//		ctp_power_on(0);
//		msleep(10);
//		printk("++++ [mtk-tpd], cyttsp tpd exit bootloader mode failed--1!\n");
//		TPD_DEBUG("<<<<<<<< [mtk-tpd], cyttsp tpd exit bootloader mode failed!\n");
//		return status;
//	}
//	if( status < 0)
//	{
//		TPD_DEBUG("<<<<<<<< [mtk-tpd], cyttsp tpd exit bootloader mode failed!\n");
//		return status;
//	}
//	msleep(1000);

    thread = kthread_run(touch_event_handler, 0, TPD_DEVICE);
    if (IS_ERR(thread)) { 
        retval = PTR_ERR(thread);
		TPD_DMESG("[CY8CTST242][FUNCTION]failed to create kernel thread: %d\n", retval);
		return retval;
    }

#ifdef TPD_PROXIMITY
	tpd_initialize_ps_sensor_function();
#endif

	mt_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, EINTF_TRIGGER_FALLING, tpd_eint_interrupt_handler, 1);
	mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);

    tpd_load_status = 1;

#if LCT_ADD_TP_VERSION
		g_ctp_proc = create_proc_entry(CTP_PROC_FILE, 0444, NULL);
		if (g_ctp_proc == NULL) {
			TPD_DMESG("[CY8CTST242][ENGINEER]create_proc_entry failed\n");
		} else {
			g_ctp_proc->read_proc = ctp_proc_read;
			g_ctp_proc->write_proc = NULL;
			//g_ctp_proc->owner = THIS_MODULE;
			TPD_DMESG("[CY8CTST242][ENGINEER]create_proc_entry success\n");
		}
#endif
    TPD_DMESG("[CY8CTST242][FUNCTION]cyttsp tpd_i2c_probe success!!\n");

 

	// Firmware Update
         // MISC
         firmware_Cypress.minor = MISC_DYNAMIC_MINOR;
         firmware_Cypress.name = "Cypress-iap";
         firmware_Cypress.fops = &Cypress_touch_fops;
         firmware_Cypress.mode = S_IRWXUGO; 
         
         if (misc_register(&firmware_Cypress) < 0)
                   printk("mtk-tpd:[Cypress] misc_register failed!!\n");
         else
                   printk("[Cypress] misc_register finished!!\n"); 
   // End Firmware Update

	msleep(100);
   //zx++
    {
	extern char tpd_desc[50];
	extern int tpd_fw_version;
	sprintf(tpd_desc,"YEJI"  );
	tpd_fw_version = cy8ctst_get_fw();
    }
    //zx--

    return retval;

}


static int __devexit tpd_remove(struct i2c_client *client)

{
    //int error;

#ifdef CONFIG_HAS_EARLYSUSPEND
    unregister_early_suspend(&early_suspend);
#endif /* CONFIG_HAS_EARLYSUSPEND */

    TPD_DEBUG("TPD removed\n");

    return 0;
}


static int tpd_local_init(void)
{
	TPD_DMESG("Cypress cy8ctst242 I2C Touchscreen Driver (Built %s @ %s)\n", __DATE__, __TIME__);
	
    if(i2c_add_driver(&tpd_i2c_driver)!=0) {
        TPD_DEBUG("unable to add i2c driver.\n");
        return -1;
    }
	
	if(tpd_load_status == 0){
    	TPD_DEBUG("add error touch panel driver.\n");
    	i2c_del_driver(&tpd_i2c_driver);
    	return -1;
    }

#ifdef TPD_HAVE_BUTTON
    tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);// initialize tpd button data
	boot_mode = get_boot_mode();
#endif

}

static void tpd_resume(struct i2c_client *client)
{
//	int retval = TPD_OK;
//	int status = 0;

#ifdef TPD_PROXIMITY
	if(g_bPsSensorOpen == 1)// && (!g_bSuspend)
	{
		//TPD_DMESG("msg sensor resume in calling tp no need to resume\n");
		return;
	}
	//g_bSuspend = 0;
#endif

	//printk("++++sleep:%s, line %d----,ps_flag=%d\n", __FUNCTION__, __LINE__, tp_ps_enable_flag);
	//if(tp_ps_enable_flag)
	//	return;
	//mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);  
	//msleep(100);
	#ifdef CONFIG_TOUCHSCREEN_POWER_DOWN_WHEN_SLEEP
	ctp_power_on(1);//yixuhong 20131115 add,tp power on when resume
	#endif
	
	//msleep(1);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(1);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);    
	msleep(100);


	//TPD_DEBUG("TPD wake up\n");
//#ifdef I2C_DMA_MODE
//	status = CTPDMA_i2c_write(i2c_client, bl_cmd, 12);
//#else
//	status = i2c_master_send(i2c_client, bl_cmd, 12);
//#endif
//	if( status < 0)
//	{
		//printk("++++ [mtk-tpd], cyttsp tpd exit bootloader mode failed--tpd_resume!\n");
//		return status;
//	}

//	msleep(300);
	mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);  
	tp_suspend_flag = 0;
	g_nTPSleep = 0;
	if(g_nProximitySleep==1)
	{
		mdelay(10);
		cyps242_enable_ps(1);
		g_nProximitySleep = 0;
	}
}

static void tpd_suspend(struct i2c_client *client, pm_message_t message)
{
	int i = 0;
	int tries = 0;
	int retval = TPD_OK;
	u8 sleep_mode = 0x02;	// 0x02--CY_DEEP_SLEEP_MODE,   0x04--CY_LOW_PWR_MODE
	//TPD_DEBUG("TPD enter sleep\n");

#ifdef TPD_PROXIMITY
	if(g_bPsSensorOpen == 1)
	{
		//TPD_DMESG("msg suspend in calling tp no need to suspend\n");
		//g_bSuspend = 0;
		return;
	}
	//g_bSuspend = 1;
#endif

	//printk("++++sleep:%s, line %d----,ps_flag=%d\n", __FUNCTION__, __LINE__, tp_ps_enable_flag);
	//if(tp_ps_enable_flag)
		//return;
	mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);

	while((tp_reseting_flag || tp_interrupt_flag) && i<30)
	{
		i++;
		msleep(100);
	}
	tp_suspend_flag = 1;
#if 1
	mutex_lock(&cyps242_sensor_mutex);

	do {
		retval = i2c_smbus_write_i2c_block_data(i2c_client,0x00,sizeof(sleep_mode), &sleep_mode);
		msleep(1);
		if (retval < 0)
			msleep(1);
	} while (tries++ < 10 && (retval < 0));
	mutex_unlock(&cyps242_sensor_mutex);
	g_nTPSleep = 1;
	#ifdef CONFIG_TOUCHSCREEN_POWER_DOWN_WHEN_SLEEP
	ctp_power_on(0);//yixuhong 20131115 add ,tp power down when sleep
	#endif
	//mdelay(1);
#else
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);    
#endif
}


#ifdef ENABLE_AUTO_UPDATA
static int str2uc(char *str, u8 *val)
{
	char substr[3];
	unsigned long ulval;
	int rc;

	if (!str && strlen(str) < 2)
		return -EINVAL;

	substr[0] = str[0];
	substr[1] = str[1];
	substr[2] = '\0';

	rc = strict_strtoul(substr, 16, &ulval);
	if (rc != 0)
		return rc;

	*val = (u8) ulval;

	return 0;
}


//static bool cyttsp_IsTpInBootloader(void)
//{	  
//    int retval = -1;
//	u8 tmpData[18] = {0};
//	retval = i2c_smbus_read_i2c_block_data(i2c_client,CY_REG_BASE,8, tmpData);  
//	if(retval < 0)
//	{
//		printk(KERN_ERR"cyttsp_IsTpInBootloader read version and module error\n");
//		return false;
//	}
//	else
//	{
//		retval = 0;
//		retval  = ((tmpData[1] & 0x10) >> 4);

//		printk(KERN_ERR"cyttsp_IsTpInBootloader tmpData[1]:%x  retval:%x\n",tmpData[1],retval);
//	}
//	if(retval == 0)
//	{
//		return false;
//	}
//	return true;
//}

static int cyttsp_putbl(int show,
			int show_status, int show_version, int show_cid)
{
	int retval = CY_OK;

	int num_bytes = (show_status * 3) + (show_version * 6) + (show_cid * 3);

	if (show_cid)
		num_bytes = sizeof(struct cyttsp_bootloader_data_t);
	else if (show_version)
		num_bytes = sizeof(struct cyttsp_bootloader_data_t) - 3;
	else
		num_bytes = sizeof(struct cyttsp_bootloader_data_t) - 9;

	if (show) {
	#ifdef I2C_DMA_MODE
		do{
			int times = num_bytes >> 3;
			int last_len = num_bytes & 0x7;
			int ii = 0;
			u8 *buf = (u8 *)&g_bl_data;
			for(ii = 0 ; ii < times; ii ++)
			{
				retval = i2c_smbus_read_i2c_block_data(i2c_client,CY_REG_BASE+ (ii<<3), 8, (buf+ (ii<<3)));
				if(retval < 0)
				{
					break;
				}
			}
			if(retval < 0)
			{
				break;
			}
			if(last_len > 0)
			{
				retval = i2c_smbus_read_i2c_block_data(i2c_client,CY_REG_BASE+ (ii<<3), last_len, (buf+ (ii<<3)));
			}
		}while(0);
	#else
		retval = i2c_smbus_read_i2c_block_data(i2c_client,CY_REG_BASE, num_bytes, (u8 *)&g_bl_data);
	#endif
		if (show_status) {
			TPD_DMESG("[CY8CTST242][FIRMWARE]BL%d: f=%02X s=%02X err=%02X bl=%02X%02X bld=%02X%02X\n", \
				show, \
				g_bl_data.bl_file, \
				g_bl_data.bl_status, \
				g_bl_data.bl_error, \
				g_bl_data.blver_hi, g_bl_data.blver_lo, \
				g_bl_data.bld_blver_hi, g_bl_data.bld_blver_lo);
		}
		if (show_version) {
			TPD_DMESG("[CY8CTST242][FIRMWARE]BL%d: ttspver=0x%02X%02X appid=0x%02X%02X appver=0x%02X%02X\n", \
				show, \
				g_bl_data.ttspver_hi, g_bl_data.ttspver_lo, \
				g_bl_data.appid_hi, g_bl_data.appid_lo, \
				g_bl_data.appver_hi, g_bl_data.appver_lo);
		}
		if (show_cid) {
			TPD_DMESG("[CY8CTST242][FIRMWARE]BL%d: cid=0x%02X%02X%02X\n", \
				show, \
				g_bl_data.cid_0, \
				g_bl_data.cid_1, \
				g_bl_data.cid_2);
		}
	}

	return retval;
}

static void cyttsp_exit_bl_mode(void)
{
	int retval, tries = 0;

	do {
	#ifdef I2C_DMA_MODE
		do{
			int times = sizeof(bl_cmd) >> 3;
			int last_len = sizeof(bl_cmd) & 0x7;
			int ii = 0;
			u8 *buf = bl_cmd;
			for(ii = 0 ; ii < times; ii ++)
			{
				retval = i2c_smbus_write_i2c_block_data(i2c_client,CY_REG_BASE+ (ii<<3), 8, buf+ (ii<<3));
				if(retval < 0)
				{
					break;
				}
			}
			if(retval < 0)
			{
				break;
			}	
			if(last_len > 0)
			{
				retval = i2c_smbus_write_i2c_block_data(i2c_client,CY_REG_BASE+ (ii<<3), last_len, buf+ (ii<<3));
			}
		}while(0);
	#else
		retval = i2c_smbus_write_i2c_block_data(i2c_client,
			CY_REG_BASE, sizeof(bl_cmd), bl_cmd);
	#endif
		if (retval < 0)
			msleep(20);
	} while (tries++ < 10 && (retval < 0));
}

static void cyttsp_set_sysinfo_mode(void)
{
	int retval, tries = 0;
	u8 host_reg = CY_SYSINFO_MODE;

	do {
		retval = i2c_smbus_write_i2c_block_data(i2c_client,
			CY_REG_BASE, sizeof(host_reg), &host_reg);
		if (retval < 0)
			msleep(20);
	} while (tries++ < 10 && (retval < 0));

	/* wait for TTSP Device to complete switch to SysInfo mode */
	if (!(retval < 0)) {
	#ifdef I2C_DMA_MODE
		do{
			int times = sizeof(struct cyttsp_sysinfo_data_t) >> 3;
			int last_len = sizeof(struct cyttsp_sysinfo_data_t) & 0x7;
			int ii = 0;
			u8 *buf = (u8 *)&g_sysinfo_data;
			for(ii = 0 ; ii < times; ii ++)
			{
				retval = i2c_smbus_read_i2c_block_data(i2c_client,CY_REG_BASE+ (ii<<3), 8, buf+ (ii<<3));
				if(retval <= 0)
				{
					break;
				}
			}
			if(retval < 0)
			{
				break;
			}
			if(last_len > 0)
			{
				retval = i2c_smbus_read_i2c_block_data(i2c_client,CY_REG_BASE+ (ii<<3), last_len, buf+ (ii<<3));
			}	
		}while(0);
	#else
		retval = i2c_smbus_read_i2c_block_data(i2c_client,
				CY_REG_BASE,
				sizeof(struct cyttsp_sysinfo_data_t),
				(u8 *)&g_sysinfo_data);
	#endif
	} else
		printk("%s: failed\n", __func__);
}

static void cyttsp_set_opmode(void)
{
	int retval, tries = 0;
	u8 host_reg = CY_OP_MODE;

	do {
		retval = i2c_smbus_write_i2c_block_data(i2c_client,
				CY_REG_BASE, sizeof(host_reg), &host_reg);
		if (retval < 0)
			msleep(20);
	} while (tries++ < 10 && (retval < 0));
}

static int cyttsp_soft_reset(void)
{
	int retval = 0, tries = 0;
	u8 host_reg = CY_SOFT_RESET_MODE;

	#if 0
          gpio_set_value(ts->platform_data->resout_gpio, 1); //reset high valid
	   msleep(100);
	   gpio_set_value(ts->platform_data->resout_gpio, 0);
          msleep(1000);
	#endif

	do {
		retval = i2c_smbus_write_i2c_block_data(i2c_client,
				CY_REG_BASE, sizeof(host_reg), &host_reg);
		if (retval < 0)
			msleep(20);
	} while (tries++ < 10 && (retval < 0));

	if (retval < 0) {
		printk("%s: failed\n", __func__);
		return retval;
	}

	tries = 0;
	do {
		msleep(20);
		cyttsp_putbl(1, true, true, false);
	} while (g_bl_data.bl_status != 0x10 &&
		g_bl_data.bl_status != 0x11 &&
		tries++ < 100);

	if (g_bl_data.bl_status != 0x11 && g_bl_data.bl_status != 0x10)
		return -EINVAL;

	return 0;
}
#define ID_INFO_OFFSET_IN_REC 77

static int get_hex_fw_ver(u8 *p, u8 *ttspver_hi, u8 *ttspver_lo,
			u8 *appid_hi, u8 *appid_lo, u8 *appver_hi,
			u8 *appver_lo, u8 *cid_0, u8 *cid_1, u8 *cid_2)
{
	int rc;

	p = p + ID_INFO_OFFSET_IN_REC;
	rc = str2uc(p, ttspver_hi);
	if (rc < 0)
		return rc;
	p += 2;
	rc = str2uc(p, ttspver_lo);
	if (rc < 0)
		return rc;
	p += 2;
	rc = str2uc(p, appid_hi);
	if (rc < 0)
		return rc;
	p += 2;
	rc = str2uc(p, appid_lo);
	if (rc < 0)
		return rc;
	p += 2;
	rc = str2uc(p, appver_hi);
	if (rc < 0)
		return rc;
	p += 2;
	rc = str2uc(p, appver_lo);
	if (rc < 0)
		return rc;
	p += 2;
	rc = str2uc(p, cid_0);
	if (rc < 0)
		return rc;
	p += 2;
	rc = str2uc(p, cid_1);
	if (rc < 0)
		return rc;
	p += 2;
	rc = str2uc(p, cid_2);
	if (rc < 0)
		return rc;

	return 0;
}

/* firmware flashing block */
#define BLK_SIZE     16
#define DATA_REC_LEN 64
#define START_ADDR   0x0880//0x0b00
#define BLK_SEED     0xff
#define RECAL_REG    0x1b

enum bl_commands {
	BL_CMD_WRBLK     = 0x39,
	BL_CMD_INIT      = 0x38,
	BL_CMD_TERMINATE = 0x3b,
};
/* TODO: Add key as part of platform data */
#define KEY_CS  (0 + 1 + 2 + 3 + 4 + 5 + 6 + 7)
#define KEY {0, 1, 2, 3, 4, 5, 6, 7}

static const  char _key[] = KEY;
#define KEY_LEN sizeof(_key)


static int rec_cnt;
struct fw_record {
	u8 seed;
	u8 cmd;
	u8 key[KEY_LEN];
	u8 blk_hi;
	u8 blk_lo;
	u8 data[DATA_REC_LEN];
	u8 data_cs;
	u8 rec_cs;
};
#define fw_rec_size (sizeof(struct fw_record))

struct cmd_record {
	u8 reg;
	u8 seed;
	u8 cmd;
	u8 key[KEY_LEN];
};
#define cmd_rec_size (sizeof(struct cmd_record))

#define BL_REC1_ADDR          0x0780
#define BL_REC2_ADDR          0x07c0

#define ID_INFO_REC           ":40078000"
#define ID_INFO_OFFSET_IN_REC 77

#define REC_START_CHR     ':'
#define REC_LEN_OFFSET     1
#define REC_ADDR_HI_OFFSET 3
#define REC_ADDR_LO_OFFSET 5
#define REC_TYPE_OFFSET    7
#define REC_DATA_OFFSET    9
#define REC_LINE_SIZE	141


static struct fw_record data_record = {
	.seed = BLK_SEED,
	.cmd = BL_CMD_WRBLK,
	.key = KEY,
};

static const struct cmd_record terminate_rec = {
	.reg = 0,
	.seed = BLK_SEED,
	.cmd = BL_CMD_TERMINATE,
	.key = KEY,
};
static const struct cmd_record initiate_rec = {
	.reg = 0,
	.seed = BLK_SEED,
	.cmd = BL_CMD_INIT,
	.key = KEY,
};


static int flash_block(u8 *blk, int len)
{
	int retval, i, tries = 0;
	char buf[(2 * (BLK_SIZE + 1)) + 1];
	char *p = buf;

	for (i = 0; i < len; i++, p += 2)
		sprintf(p, "%02x", blk[i]);
	TPD_DMESG("[CY8CTST242][FIRMWARE]%s: size %d, pos %ld payload %s\n",__func__, len, (long)0, buf);

	do {
	#ifdef I2C_DMA_MODE
		buf[0] = CY_REG_BASE;
		memcpy((u8 *)(&buf[1]), blk, len);
		CTPDMA_i2c_write(i2c_client, buf, len+1);
	#else
		retval = i2c_smbus_write_i2c_block_data(i2c_client,
			CY_REG_BASE, len, blk);
	#endif
		if (retval < 0)
			msleep(20);
	} while (tries++ < 20 && (retval < 0));

	if (retval < 0) {
		TPD_DMESG("[CY8CTST242][FIRMWARE]%s: failed\n", __func__);
		return retval;
	}

	return 0;
}


static int flash_command(const struct cmd_record *record)
{
	return flash_block((u8 *)record, cmd_rec_size);
}

static void init_data_record(struct fw_record *rec, unsigned short addr)
{
	addr >>= 6;
	rec->blk_hi = (addr >> 8) & 0xff;
	rec->blk_lo = addr & 0xff;
	rec->rec_cs = rec->blk_hi + rec->blk_lo +
			(unsigned char)(BLK_SEED + BL_CMD_WRBLK + KEY_CS);
	rec->data_cs = 0;
}

static int check_record(u8 *rec)
{
	int rc;
	u16 addr;
	u8 r_len, type, hi_off, lo_off;

	rc = str2uc(rec + REC_LEN_OFFSET, &r_len);
	if (rc < 0)
		return rc;

	rc = str2uc(rec + REC_TYPE_OFFSET, &type);
	if (rc < 0)
		return rc;

	if (*rec != REC_START_CHR || r_len != DATA_REC_LEN || type != 0)
		return -EINVAL;

	rc = str2uc(rec + REC_ADDR_HI_OFFSET, &hi_off);
	if (rc < 0)
		return rc;

	rc = str2uc(rec + REC_ADDR_LO_OFFSET, &lo_off);
	if (rc < 0)
		return rc;

	addr = (hi_off << 8) | lo_off;

	if (addr >= START_ADDR || addr == BL_REC1_ADDR || addr == BL_REC2_ADDR)
		return 0;

	return -EINVAL;
}


static struct fw_record *prepare_record(u8 *rec)
{
	int i, rc;
	u16 addr;
	u8 hi_off, lo_off;
	u8 *p;

	rc = str2uc(rec + REC_ADDR_HI_OFFSET, &hi_off);
	if (rc < 0)
		return ERR_PTR((long) rc);

	rc = str2uc(rec + REC_ADDR_LO_OFFSET, &lo_off);
	if (rc < 0)
		return ERR_PTR((long) rc);

	addr = (hi_off << 8) | lo_off;

	init_data_record(&data_record, addr);
	p = rec + REC_DATA_OFFSET;
	for (i = 0; i < DATA_REC_LEN; i++) {
		rc = str2uc(p, &data_record.data[i]);
		if (rc < 0)
			return ERR_PTR((long) rc);
		data_record.data_cs += data_record.data[i];
		data_record.rec_cs += data_record.data[i];
		p += 2;
	}
	data_record.rec_cs += data_record.data_cs;

	return &data_record;
}

static int flash_record(const struct fw_record *record)
{
	int len = fw_rec_size;
	int blk_len, rc;
	u8 *rec = (u8 *)record;
	u8 data[BLK_SIZE + 1];
	u8 blk_offset;

	for (blk_offset = 0; len; len -= blk_len) {
		data[0] = blk_offset;
		blk_len = len > BLK_SIZE ? BLK_SIZE : len;
		memcpy(data + 1, rec, blk_len);
		rec += blk_len;
		rc = flash_block(data, blk_len + 1);
		if (rc < 0)
			return rc;
		blk_offset += blk_len;
	}
	return 0;
}


static int flash_data_rec(u8 *buf)
{
	struct fw_record *rec;
	int rc, tries;

	if (!buf)
		return -EINVAL;

	rc = check_record(buf);

	if (rc < 0) {
		TPD_DMESG("[CY8CTST242][FIRMWARE]%s: record ignored %s", __func__, buf);
		return 0;
	}

	rec = prepare_record(buf);
	if (IS_ERR_OR_NULL(rec))
		return PTR_ERR(rec);

	rc = flash_record(rec);
	if (rc < 0)
		return rc;

	tries = 0;
	do {
		if (rec_cnt%2)
			msleep(20);
		cyttsp_putbl(4, true, false, false);
	} while (g_bl_data.bl_status != 0x10 &&
		g_bl_data.bl_status != 0x11 &&
		tries++ < 100);
	rec_cnt++;
	return rc;
}


static int cyttspfw_flash_firmware(const u8 *data, int data_len)
{
	u8 *buf;
	int i, j;
	int rc, tries = 0;

	/* initiate bootload: this will erase all the existing data */
	rc = flash_command(&initiate_rec);
	if (rc < 0)
		return rc;

	do {
		msleep(100);
		cyttsp_putbl(4, true, false, false);
	} while (g_bl_data.bl_status != 0x10 &&
		g_bl_data.bl_status != 0x11 &&
		tries++ < 100);

	buf = kmalloc(REC_LINE_SIZE + 1, GFP_ATOMIC);;
	if (!buf) {
		TPD_DMESG("[CY8CTST242][FIRMWARE]%s: no memory\n", __func__);
		return -ENOMEM;
	}

	rec_cnt = 0;
	/* flash data records */
	for (i = 0, j = 0; i < data_len; i++, j++) {
		if ((data[i] == REC_START_CHR) && j) {
			buf[j] = 0;
			rc = flash_data_rec(buf);
			if (rc < 0)
				return rc;
			j = 0;
		}
		buf[j] = data[i];
	}

	/* flash last data record */
	if (j) {
		buf[j] = 0;
		rc = flash_data_rec(buf);
		if (rc < 0)
			return rc;
	}

	kfree(buf);

	/* termiate bootload */
	tries = 0;
	rc = flash_command(&terminate_rec);
	do {
		msleep(100);
		cyttsp_putbl(4, true, false, false);
	} while (g_bl_data.bl_status != 0x10 &&
		g_bl_data.bl_status != 0x11 &&
		tries++ < 100);

	return rc;
}


static void cyttspfw_flash_start(const u8 *data,int data_len, u8 *buf, bool force)
{
	int rc;
	u8 ttspver_hi = 0, ttspver_lo = 0, fw_upgrade = 0;
	u8 appid_hi = 0, appid_lo = 0;
	u8 appver_hi = 0, appver_lo = 0;
	u8 cid_0 = 0, cid_1 = 0, cid_2 = 0;
	char *p = buf;

	int tries = 0;
	u8 tmpData[18] = {0};

	/* get hex firmware version */
	rc = get_hex_fw_ver(p, &ttspver_hi, &ttspver_lo,
		&appid_hi, &appid_lo, &appver_hi,
		&appver_lo, &cid_0, &cid_1, &cid_2);

	if (rc < 0) {
		TPD_DMESG("[CY8CTST242][FIRMWARE]failed to get hex firmware version\n");
		return;
	}

	/* disable interrupts before flashing */
	// todo

	enter_boot_mode();
	
	/* enter bootloader idle mode */
	rc = cyttsp_soft_reset();

	if (rc < 0) {
		TPD_DMESG("[CY8CTST242][FIRMWARE]failed to reset CHIP 1st\n");
		msleep(1000);
		rc = cyttsp_soft_reset();
	}

	if(rc < 0)
	{
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE); //reset high valid
		msleep(100);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
		msleep(100);	

		do {
			msleep(20);
			cyttsp_putbl(1, true, true, false);
		} while (g_bl_data.bl_status != 0x10 &&
			g_bl_data.bl_status != 0x11 &&
			tries++ < 100);

		TPD_DMESG("[CY8CTST242][FIRMWARE]firmware update start tries:%d\n",tries);

		//rc  = i2c_smbus_read_i2c_block_data(i2c_client,CY_REG_BASE,8, tmpData);  
		//if(rc  < 0)
		//{
		//	TPD_DMESG("[CY8CTST242][FIRMWARE]failed to read version and module\n");
		//}
		//else
		//{
		//    TPD_DMESG("[CY8CTST242][FIRMWARE]succeeded to read version and module tmpData[0]:%x,tmpData[1]:%x\n",tmpData[0],tmpData[1]);
		//}

		if(g_bl_data.bl_status != 0x10 && g_bl_data.bl_status != 0x11)
		{
			rc = -1;
		}else{
			rc = 1;
		}
		
	}

	if (rc < 0) {
		TPD_DMESG("[CY8CTST242][FIRMWARE]cyttsp_soft_reset  try again later\n");
		exit_boot_mode();
		return;
	}

	TPD_DMESG("[CY8CTST242][FIRMWARE]Current firmware:%x.%x\n", g_bl_data.appver_hi, g_bl_data.appver_lo);
	TPD_DMESG("[CY8CTST242][FIRMWARE]New firmware:%x.%x\n", appver_hi, appver_lo);

	if (force)
	{
		fw_upgrade = 1;
	}
	else
	{
		#if 1
		// case 1:launch upgrade process when i2c transfer error detected
		if ( (FIRMWARE_UPGRADE_FAIL_FLAG == g_bl_data.bl_file)
		// case 2:do upgrade process when the new version detected
			|| ((appver_hi == g_bl_data.appver_hi) && (appver_lo > g_bl_data.appver_lo)))
			
		{
			fw_upgrade = 1;
		}
		else
		{
			fw_upgrade = 0;
		}
		#else
		if ((appid_hi == g_bl_data.appid_hi) &&
			(appid_lo == g_bl_data.appid_lo)) {
			if (appver_hi > g_bl_data.appver_hi) {
				fw_upgrade = 1;
			} else if ((appver_hi == g_bl_data.appver_hi) &&
					 (appver_lo > g_bl_data.appver_lo)) {
					fw_upgrade = 1;
				} else {
					fw_upgrade = 0;
					printk("%s: Firmware version "
					"lesser/equal to existing firmware, "
					"upgrade not needed\n", __func__);
				}
		} else {
			fw_upgrade = 0;
			printk("%s: Firware versions do not match, "
						"cannot upgrade\n", __func__);
		}
		#endif
	}
	
	if (fw_upgrade) {
		TPD_DMESG("[CY8CTST242][FIRMWARE]firmware upgrade process start\n");
		

		rc = cyttspfw_flash_firmware(data, data_len);
		if (rc < 0)
			TPD_DMESG("[CY8CTST242][FIRMWARE]failed to process firmware upgrade\n");
		else
			TPD_DMESG("[CY8CTST242][FIRMWARE]succeeded to process firmware upgrade\n");

		if((fw_upgrade == 1) && (rc >= 0))
		{
			u8 tmpData[18] = {0};
			rc  = i2c_smbus_read_i2c_block_data(i2c_client,CY_REG_BASE,8, tmpData);  
			rc  = i2c_smbus_read_i2c_block_data(i2c_client,CY_REG_BASE+8,8, (void *)(tmpData+8));  
			rc  = i2c_smbus_read_i2c_block_data(i2c_client,CY_REG_BASE+16,2, (void *)(tmpData+16));  
				
			if(rc  < 0)
			{
				
				TPD_DMESG("[CY8CTST242][FIRMWARE]failed to read version and module\n");
			}
			else
			{
		       cyttsp_vendor_id=tmpData[11];
			   cyttsp_firmware_version = tmpData[12];
			   TPD_DMESG("[CY8CTST242][FIRMWARE]succeeded to read version and module module is:%x,firmware version:%x:\n",tmpData[11],tmpData[12]);
			}
		}
	}

	// exit bl mode
	exit_boot_mode();

	/* enter bootloader idle mode */
	//cyttsp_soft_reset();
	/* exit bootloader mode */
	//exit_boot_mode();
	//cyttsp_exit_bl_mode();
	//msleep(100);
	/* set sysinfo details */
	//cyttsp_set_sysinfo_mode();
	/* enter application mode */
	//cyttsp_set_opmode();


	/* enable interrupts */
}


static void cyttspfw_upgrade_start(const u8 *data,int data_len, bool force)
{
	int i, j;
	u8 *buf;

	buf = kmalloc(REC_LINE_SIZE + 1, GFP_ATOMIC);;
	if (!buf) {
		TPD_DMESG("[CY8CTST242][FIRMWARE]%s: no memory\n", __func__);
		return;
	}

	mtk_wdt_restart(WK_WDT_EXT_TYPE);//kick external WDT 
	mtk_wdt_restart(WK_WDT_LOC_TYPE);//kick local WDT, CPU0/CPU1 kick 
	
	for (i = 0, j = 0; i < data_len; i++, j++) {
		if ((data[i] == REC_START_CHR) && j) {
			buf[j] = 0;
			j = 0;
			if (!strncmp(buf, ID_INFO_REC, strlen(ID_INFO_REC))) {
				cyttspfw_flash_start(data, data_len,
							buf, force);
				break;
			}
		}
		buf[j] = data[i];
	}

	mtk_wdt_restart(WK_WDT_EXT_TYPE);//kick external WDT 
	mtk_wdt_restart(WK_WDT_LOC_TYPE);//kick local WDT, CPU0/CPU1 kick 

	/* check in the last record of firmware */
	if (j) {
		buf[j] = 0;
		if (!strncmp(buf, ID_INFO_REC, strlen(ID_INFO_REC))) {
			cyttspfw_flash_start(data, data_len,
						buf, force);
		}
	}

	mtk_wdt_restart(WK_WDT_EXT_TYPE);//kick external WDT 
	mtk_wdt_restart(WK_WDT_LOC_TYPE);//kick local WDT, CPU0/CPU1 kick 

	kfree(buf);
}


//static void cyttspfw_upgrade(char *pData, int size, int force)
//{
//	int retval = 0;
//	const char *fwfile;
	
//	if(cyttsp_has_bootloader == 0)	
//	{		
//		printk(KERN_ERR"cyttspfw_upgrade  TP firmware is not support bootloader \n");		
//		return;	
//	}	
	/* check and start upgrade */
	//if upgrade failed we should force upgrage when next power up
//	if(cyttsp_IsTpInBootloader() == true)
//	{
//		printk(KERN_ERR"cyttspfw_upgrade we should force upgrade tp fw\n");
//		cyttspfw_upgrade_start(pData, size, true);
//	}
//	else
////	{
//		cyttspfw_upgrade_start(pData, size, force);
//	}
//}
#endif
static void cy8ctst_tpd_get_fw_vendor_name(char * fw_vendor_name)
{
    sprintf(fw_vendor_name, "YEIJI");
}


static struct tpd_driver_t tpd_device_driver = {
    .tpd_device_name = "cy8ctst242",
    .tpd_local_init = tpd_local_init,
    .suspend = tpd_suspend,
    .resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
    .tpd_have_button = 1,
#else
    .tpd_have_button = 0,
#endif	
    .tpd_get_ps  =  tst242_get_ps,
	.tpd_convert =  tst242_ctp_convert,
	.tpd_get_fw_version = cy8ctst_get_fw,
    .tpd_get_fw_vendor_name = cy8ctst_tpd_get_fw_vendor_name,
};

/* called when loaded into kernel */
static int __init tpd_driver_init(void) {
    TPD_DEBUG("MediaTek CY8CTST242 touch panel driver init\n");
#ifdef CUST_ICS
	i2c_register_board_info(TPD_I2C_NUMBER, &cyttsp_i2c_tpd, 1);
#endif

#ifdef I2C_DMA_MODE
// add by gpg
	CTPI2CDMABuf_va = (u8 *)dma_alloc_coherent(NULL, 4096, &CTPI2CDMABuf_pa, GFP_KERNEL);
	if(!CTPI2CDMABuf_va)
	{
		printk("[TSP] dma_alloc_coherent error\n");
	}
// endif
	printk("cyttsp Get PA:0x%08x, VA:0x%08x", CTPI2CDMABuf_pa, CTPI2CDMABuf_va);
#endif

	if(tpd_driver_add(&tpd_device_driver) < 0)
        TPD_DMESG("add CY8CTST242 driver failed\n");
    return 0;
}

/* should never be called */
static void __exit tpd_driver_exit(void) {
    TPD_DMESG("MediaTek CY8CTST242 touch panel driver exit\n");
#ifdef I2C_DMA_MODE
// add by gpg
		if(CTPI2CDMABuf_va)
		{
			dma_free_coherent(NULL, 4096, CTPI2CDMABuf_va, CTPI2CDMABuf_pa);
			CTPI2CDMABuf_va = NULL;
			CTPI2CDMABuf_pa = 0;
		}
// end by gpg
#endif    
    tpd_driver_remove(&tpd_device_driver);
}

module_init(tpd_driver_init);
module_exit(tpd_driver_exit);

