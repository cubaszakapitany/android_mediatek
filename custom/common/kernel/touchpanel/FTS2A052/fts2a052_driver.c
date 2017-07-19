/*
 * FTS Capacitive touch screen controller (FingerTipS)
 *
 * Copyright (C) 2013 TCL Inc.
 * Author: xiaoyang <xiaoyang.zhang@tcl.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
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
#include <linux/byteorder/generic.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif 
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/rtpm_prio.h>
#include <linux/proc_fs.h>
#include <linux/jiffies.h>
#include <linux/firmware.h>
#include <linux/earlysuspend.h>
#include <linux/irq.h>
#include <linux/input/mt.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/completion.h>
#include <linux/init.h>

#include <asm/uaccess.h>
#include <cust_eint.h>
#include <asm/unaligned.h>
#include <mach/eint.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_pm_ldo.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/kobject.h>

#if 0 //def MT6589
#include <mach/mt_boot.h>
#endif

#include "tpd.h"
#include "fts2a052_driver.h"
#include "tpd_custom_fts2a052.h"

#if 0 //def MT6589
	extern void mt_eint_unmask(unsigned int line);
	extern void mt_eint_mask(unsigned int line);
	extern void mt_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
	extern unsigned int mt_eint_set_sens(unsigned int eint_num, unsigned int sens);
	extern void mt_eint_registration(unsigned int eint_num, unsigned int is_deb_en, unsigned int pol, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
#endif

extern struct tpd_device *tpd;
static DECLARE_WAIT_QUEUE_HEAD(st_waiter);
//static DEFINE_MUTEX(melfas_tp_mutex);

extern int TPD_RES_X;
extern int TPD_RES_Y;

struct i2c_client *st_i2c_client = NULL;
static int st_tpd_flag = 0;
static u8 * DMAbuffer_va = NULL;
static dma_addr_t DMAbuffer_pa = NULL;
static int interrupt_count = 0;

static volatile unsigned char wr_buffer[16];
#define ST_MAX_I2C_TRANSFER_SIZE	8
#define ST_I2C_MASTER_CLOCK		100
#define FTS_EVENT_WAIT_MAX                  20//WORKAROUND reduce wake time //200
#define I2C_RETRY_CNT		3

static int fts_forcecal_ready = 0;
static int fts_mutualTune_ready = 0;
static int fts_selfTune_ready = 0;
static int fts_tuningbackup_ready = 0;

static unsigned char old_buttons = 0;
static unsigned char modebyte = MODE_NORMAL;
unsigned int              chip_fw_version;
unsigned int              code_fw_version;

#ifdef TPD_HAVE_BUTTON 

static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;
#endif

static const struct i2c_device_id st_fts_tpd_id[] = {{"st_fts",0},{}};
static struct i2c_board_info __initdata st_fts_i2c_tpd={ I2C_BOARD_INFO("st_fts", (0x92>>1))};

static int tpd_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_i2c_remove(struct i2c_client *client);
static int tpd_i2c_detect(struct i2c_client *client, struct i2c_board_info *info);

static void st_i2c_tpd_eint_interrupt_handler(void);
static int fts_get_fw_version(void);
static int fts_systemreset(void);
static int fts_init_flash_reload(void);

const u8 st_firmware[] = {
#include "st_1204_1.bin.h"
};

struct firmware fw_info_st = 
{
	.size = sizeof(st_firmware),
	.data = &st_firmware[0],
};

static struct i2c_driver tpd_i2c_driver =
{                       
    .probe = tpd_i2c_probe,                                   
    .remove = __devexit_p(tpd_i2c_remove),                           
    .detect = tpd_i2c_detect,                           
    .driver.name = "mtk-tpd", 
    .id_table = st_fts_tpd_id,                             
/*#ifndef CONFIG_HAS_EARLYSUSPEND
    .suspend = melfas_ts_suspend, 
    .resume = melfas_ts_resume,
#endif*/    
}; 


static int fts_read_reg(unsigned char *reg, int cnum,
                        unsigned char *buf,
                        int num)
{    	
    	u8 retry = 0;
    	if (cnum > ST_MAX_I2C_TRANSFER_SIZE) {
    		TPD_DMESG("Line %s, input parameter error\n", __LINE__);
    		return -1;
    	}
    	if (num > ST_MAX_I2C_TRANSFER_SIZE) {
    		TPD_DMESG("Line %s, output parameter error\n", __LINE__);
    		return -1;
    	}
    	
    	memset(wr_buffer, 0, 16);
    	memcpy(wr_buffer, reg, cnum);

    	struct i2c_msg msgs[] =
    	{
        	{            
            	.addr = st_i2c_client->addr,
            	//.flags = 0,
            	.len =  ((num<<8) | cnum),
            	.timing = I2C_MASTER_CLOCK_ST,
            	.buf = wr_buffer,
            	.ext_flag = (((st_i2c_client->ext_flag ) & I2C_MASK_FLAG ) | I2C_WR_FLAG | I2C_RS_FLAG),
        	},
    	};	
    	
    	for (retry = 0; retry < I2C_RETRY_CNT; retry++) {
    		if (i2c_transfer(st_i2c_client->adapter, msgs, 1) == 1)
    			break;
		mdelay(5);
		TPD_DMESG("Line %s, i2c_transfer error, retry = %d\n", __LINE__, retry);
		}
	if (retry == I2C_RETRY_CNT) {
		printk(KERN_ERR "fts_read_reg retry over %d\n",I2C_RETRY_CNT);
		return -1;
	}
	
    	memcpy(buf, wr_buffer, num);
    	/*for(i=0; i<num; i++)
        	buf[i]=wr_buffer[i];*/
        	
    	/*TPD_DMESG("READ DATA:%02x %02x %02x %02x %02x %02x %02x %02x\n",
           wr_buffer[0],wr_buffer[1],wr_buffer[2],wr_buffer[3],
           wr_buffer[4],wr_buffer[5],wr_buffer[6],wr_buffer[7]);*/
        /*TPD_DMESG("READ DATA:%02x %02x %02x %02x %02x %02x %02x %02x\n",
           buf[0],buf[1],buf[2],buf[3],
           buf[4],buf[5],buf[6],buf[7]);*/
              
    	return 0;
}

static int fts_dma_read_reg(unsigned char *reg, int cnum,
                         unsigned char *buf,
                         int num)
{
	int retry = 0;
    
    	memset(DMAbuffer_va, 0, 256);
    	memcpy(DMAbuffer_va, reg, cnum);
   	struct i2c_msg msgs[] =
    	{
        	{
            	//.addr = (((fts_i2c_client->addr ) & I2C_MASK_FLAG ) | I2C_WR_FLAG | I2C_RS_FLAG),
            	.addr = st_i2c_client->addr,
            	//.flags = 0,
            	.timing = I2C_MASTER_CLOCK_ST,
            	.len =  ((num<<8) | cnum),
            	.buf = DMAbuffer_pa,
            	.ext_flag = (((st_i2c_client->ext_flag ) & I2C_MASK_FLAG ) | I2C_WR_FLAG | I2C_RS_FLAG | I2C_DMA_FLAG),
        	},
    	};
    	for (retry = 0; retry < I2C_RETRY_CNT; retry++) {
		if (i2c_transfer(st_i2c_client->adapter, msgs, 1) == 1)
			break;
		mdelay(5);
		TPD_DMESG("Line %s, i2c dma transfer error, retry = %d\n", __LINE__, retry);
	}
	if (retry == I2C_RETRY_CNT) {
		printk(KERN_ERR "fts_dma_read_reg retry over %d\n",I2C_RETRY_CNT);
		return -1;
	}
    
    	memcpy(buf, DMAbuffer_va, num);
    	/*TPD_DMESG("READ DMA DATA:%02x %02x %02x %02x %02x %02x %02x %02x\n",
           DMAbuffer_va[0],DMAbuffer_va[1],DMAbuffer_va[2],DMAbuffer_va[3],
           DMAbuffer_va[4],DMAbuffer_va[5],DMAbuffer_va[6],DMAbuffer_va[7]);*/
           
    	return 0;
}

static int fts_write_reg(unsigned char *reg,
                         unsigned short num_com)
{
    	u8 retry = 0;
    	struct i2c_msg msgs[] =
    	{
        	{
            	//.addr = fts_i2c_client->addr & I2C_MASK_FLAG,  //77,75
            	.addr = st_i2c_client->addr,
            	.flags = 0,
            	.timing = I2C_MASTER_CLOCK_ST,
            	.len = num_com,
            	.buf = reg,
            	.ext_flag = ((st_i2c_client->ext_flag ) & I2C_MASK_FLAG ), //msz 6572
        	}
    	};
    	for (retry = 0; retry < I2C_RETRY_CNT; retry++) {
    		if (i2c_transfer(st_i2c_client->adapter, msgs, 1) == 1)
    			break;
		mdelay(5);
		TPD_DMESG("Line %s, i2c_transfer error, retry = %d\n", __LINE__, retry);
	}
	if (retry == I2C_RETRY_CNT) {
		printk(KERN_ERR "fts_write_reg error %d\n",I2C_RETRY_CNT);
		return -1;
	}
    
    	return 0;
}

static int fts_dma_write_reg( unsigned char *reg,
                          unsigned short num_com)
{

	int retry = 0;
	memset(DMAbuffer_va, 0, 256);
	memcpy(DMAbuffer_va, reg, num_com);
	struct i2c_msg msgs[] =
	{
		{
		.addr = st_i2c_client->addr,
		.len =  num_com,
		.timing = I2C_MASTER_CLOCK_ST,
		.buf = DMAbuffer_pa,
		.ext_flag = (((st_i2c_client->ext_flag ) & I2C_MASK_FLAG ) | I2C_DMA_FLAG),
        	},
	};
	for (retry = 0; retry < I2C_RETRY_CNT; retry++) {
		if (i2c_transfer(st_i2c_client->adapter, msgs, 1) == 1)
			break;
		mdelay(5);
		TPD_DMESG("Line %s, i2c_transfer error, retry = %d\n", __LINE__, retry);
	}
	if (retry == I2C_RETRY_CNT) {
		printk(KERN_ERR "fts_dma_write_reg retry over %d\n",I2C_RETRY_CNT);
		return -1;
	}
	
	return 0;
}

static int fts_command( unsigned char cmd)
{
    unsigned char regAdd = 0;
    int ret = 0;

    regAdd = cmd;
    ret = fts_write_reg(&regAdd, 1);
    TPD_DMESG("FTS Command (%02X) , ret = %d \n", cmd, ret);
    return ret;
}


static ssize_t tp_value_show(struct kobject *kobj, struct kobj_attribute *attr,
			  char *buf)
{
	char *s = buf;
	unsigned char val[5];
	unsigned char regAdd[3];
	int error;
	int ret;
	//int result;
	int code_fw_ver = 0;
	int chip_fw_ver = 0;
	/* Read out multi-touchscreen chip ID */
	regAdd[0] = 0xB6;
	regAdd[1] = 0x00;
	regAdd[2] = 0x07;
	
	error = fts_read_reg(regAdd, sizeof(regAdd), val, sizeof(val));
	if (error < 0) {
		printk("Cannot read fts device id\n");
		//return -ENODEV;
	}
	error = fts_get_fw_version();
	if (error < 0) {
		printk("Cannot read fts firmware version\n");
		//return -ENODEV;
	}
	s += sprintf(s, "CTP Vendor: %s\n", "ST");
	s += sprintf(s, "CTP Type: %s\n", "Interrupt trigger");
	s += sprintf(s, "Chip ID: %x %x\n", val[1], val[2]);
	s += sprintf(s, "Firmware Version: 0x%x\n", chip_fw_version);
	
	return (s - buf);
}

static ssize_t tp_value_store(struct kobject *kobj, struct kobj_attribute *attr,
			   const char *buf, size_t n)
{
	int save;
	
	if (sscanf(buf, "%d", &save)==0) {
		printk(KERN_ERR "%s -- invalid save string '%s'...\n", __func__, buf);
		return -EINVAL;
	}
	return n;
}

static struct kobject *tpswitch_ctrl_kobj;

#define tpswitch_ctrl_attr(_name) \
static struct kobj_attribute _name##_attr = {	\
	.attr	= {				\
		.name = __stringify(_name),	\
		.mode = 0644,			\
	},					\
	.show	= _name##_show,			\
	.store	= _name##_store,		\
}


tpswitch_ctrl_attr(tp_value);
static struct attribute *g_attr[] = {
	&tp_value_attr.attr,	
	NULL,
};

static struct attribute_group tpswitch_attr_group = {
	.attrs = g_attr,
};

static int tpswitch_sysfs_init(void)
{ 
	tpswitch_ctrl_kobj = kobject_create_and_add("tp_compatible", NULL);
	if (!tpswitch_ctrl_kobj)
		return -ENOMEM;

	return sysfs_create_group(tpswitch_ctrl_kobj, &tpswitch_attr_group);
}

static void tpswitch_sysfs_exit(void)
{
	sysfs_remove_group(tpswitch_ctrl_kobj, &tpswitch_attr_group);

	kobject_put(tpswitch_ctrl_kobj);
}

static void st_ts_release_all_finger(void)
{
	int i;
	TPD_DMESG("[st_tpd] %s\n", __func__);
	for (i = 0; i < FINGER_MAX; i++)
	{
		input_mt_slot(tpd->dev, i);
		input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, -1);
	}
	input_sync(tpd->dev);
}

static void fts_delay(unsigned int ms)
{
    msleep(ms);
}

static int fts_get_fw_version(void)
{
	int ret;
	unsigned char data[FTS_EVENT_SIZE];
	unsigned char regAdd[3] = {0xB6, 0x00, 0x07};

	ret = fts_read_reg(regAdd, sizeof(regAdd), data, sizeof(data));

	if (ret)
		chip_fw_version = 0;
	else
		chip_fw_version = (data[5] << 8) | data[4];

	return ret;
}

static int fts_flash_status(unsigned int timeout, unsigned int steps)
{
	int ret, status;
	unsigned char data;
	unsigned char regAdd[2];

	do {
		regAdd[0] = FLASH_READ_STATUS;
		regAdd[1] = 0;
		
		msleep(20);

		ret = fts_read_reg(regAdd, sizeof(regAdd), &data, sizeof(data));
		if (ret)
			status = FLASH_STATUS_UNKNOWN;
		else
			status = (data & 0x01) ? FLASH_STATUS_BUSY : FLASH_STATUS_READY;

		if (status == FLASH_STATUS_BUSY) {
			timeout -= steps;
			msleep(steps);
		}

	} while ((status == FLASH_STATUS_BUSY) && (timeout));

	return status;
}

static int fts_flash_unlock(void)
{
	int ret;
	unsigned char regAdd[4] = { FLASH_UNLOCK,
				FLASH_UNLOCK_CODE_0,
				FLASH_UNLOCK_CODE_1,
				0x00 };

	ret = fts_write_reg(regAdd, sizeof(regAdd));

	if (ret)
		printk(KERN_ERR, "Cannot unlock flash\n");
	else
		{
		//msleep(FTS_FLASH_COMMAND_DELAY);
		TPD_DMESG( "Flash unlocked\n");
	}

	return ret;
}

static int fts_flash_load(int cmd, int address, const char *data, int size)
{
	int ret;
	unsigned char *cmd_buf;
	unsigned int loaded;

	cmd_buf = kmalloc(FLASH_LOAD_COMMAND_SIZE, GFP_KERNEL);
	if (cmd_buf == NULL) {
		TPD_DMESG("FTS Out of memory when programming flash\n");
		return -1;
	}
	TPD_DMESG("FTS flash size is: %x\n",size);
	loaded = 0;
	while (loaded < size) {
		// TPD_DMESG("FTS flash loaded %x\n",loaded);
#if 1
		cmd_buf[0] = cmd;
		cmd_buf[1] = (address >> 8) & 0xFF;
		cmd_buf[2] = (address) & 0xFF;

		memcpy(&cmd_buf[3], data, FLASH_LOAD_CHUNK_SIZE);
		ret = fts_dma_write_reg(cmd_buf, FLASH_LOAD_COMMAND_SIZE);
		if (ret) {
			TPD_DMESG("FTS Cannot load firmware in RAM\n");
			break;
		}

		data += FLASH_LOAD_CHUNK_SIZE;
		loaded += FLASH_LOAD_CHUNK_SIZE;
		address += FLASH_LOAD_CHUNK_SIZE;
#else
		/* use B3/B1 commands */
		unsigned char cmd_buf_b3[3];

		cmd_buf_b3[0] = 0xB3;
		cmd_buf_b3[1] = 0x00;
		cmd_buf_b3[2] = 0x00;
		ret = fts_write_reg(info, cmd_buf_b3, 3);

		cmd_buf[0] = 0xB1;
		cmd_buf[1] = (address >> 8) & 0xFF;
		cmd_buf[2] = (address) & 0xFF;

		memcpy(&cmd_buf[3], data, FLASH_LOAD_CHUNK_SIZE);
		ret = fts_write_reg(info, cmd_buf, FLASH_LOAD_COMMAND_SIZE);
		if (ret) {
			dev_err(info->dev, "Cannot load firmware in RAM\n");
			break;
		}

		data += FLASH_LOAD_CHUNK_SIZE;
		loaded += FLASH_LOAD_CHUNK_SIZE;
		address += FLASH_LOAD_CHUNK_SIZE;
#endif
	}

	kfree(cmd_buf);

	return (loaded == size) ? 0 : -1;
}


static int fts_flash_erase(int cmd)
{
	int ret;
	unsigned char regAdd = cmd;

	ret = fts_write_reg(&regAdd, sizeof(regAdd));

	if (ret)
		printk(KERN_ERR, "Cannot erase flash\n");
	else
		{
		//msleep(FTS_FLASH_COMMAND_DELAY);
		TPD_DMESG("Flash erased\n");
	}

	return ret;
}

static int fts_flash_program(int cmd)
{
	int ret;
	unsigned char regAdd = cmd;

	ret = fts_write_reg(&regAdd, sizeof(regAdd));

	if (ret)
		printk(KERN_ERR, "Cannot program flash\n");
	else
		{		
		TPD_DMESG("Flash programmed\n");
	}
	return ret;
}


static int st_fw_update_controller(const struct firmware *fw, struct i2c_client *client)
{
	int ret;	
	int status;
	unsigned char *data;
	unsigned int size;
	int program_command, erase_command, load_command, load_address;
	
	data = (unsigned char *)fw->data;
	size = fw->size;
	ret = fts_get_fw_version();
	if (ret) {
		printk(KERN_ERR,
			"fts Cannot retrieve current firmware version, not upgrading it\n");
		return ret;
	}
	
	code_fw_version = (data[5] << 8) | data[4];
	TPD_DMESG("FTS code firmware version is %x.\n",code_fw_version);
	TPD_DMESG("FTS old chip firmware ver is %x.\n",chip_fw_version);

	if (chip_fw_version < code_fw_version) {
	data += 32;
	size = size - 32;
	program_command = FLASH_PROGRAM;
	erase_command = FLASH_ERASE;
	load_command = FLASH_LOAD_FIRMWARE;
	load_address = FLASH_LOAD_FIRMWARE_OFFSET;			
	
	TPD_DMESG("FTS Flash size %x...\n",size);
	TPD_DMESG("FTS DATA:%02x %02x %02x %02x %02x %02x %02x %02x\n",
           data[0],data[1],data[2],data[3],
           data[4],data[5],data[6],data[7]);
	//if (fw_version < new_fw_version) 
	{
		TPD_DMESG("FTS Flash programming...\n");
		
		TPD_DMESG("FTS 1) checking for status.\n");
		status = fts_flash_status(1000, 100);
		if ((status == FLASH_STATUS_UNKNOWN) || (status == FLASH_STATUS_BUSY)) {
			printk(KERN_ERR, "Wrong flash status\n");
			goto fw_done;
		}
			
		TPD_DMESG("FTS 2) unlock the flash.\n");
		ret = fts_flash_unlock();
		if (ret) {
			printk(KERN_ERR, "Cannot unlock the flash device\n");
			goto fw_done;
		}
		/* wait for a while */
		msleep(FTS_FLASH_COMMAND_DELAY);
		
		TPD_DMESG("FTS 3) load the program.\n");
		ret = fts_flash_load(load_command, load_address, data, size);
		if (ret) {
			printk(KERN_ERR,
			"Cannot load program to for the flash device\n");
			goto fw_done;
		}
		/* wait for a while */
		msleep(FTS_FLASH_COMMAND_DELAY);
	
		TPD_DMESG("FTS 4) erase the flash.\n");
		ret = fts_flash_erase(erase_command);
		if (ret) {
			printk(KERN_ERR, "Cannot erase the flash device\n");
			goto fw_done;
		}
		/* wait for a while */
		msleep(FTS_FLASH_COMMAND_DELAY);

		TPD_DMESG("FTS 5) checking for status.\n");
		status = fts_flash_status(1000, 100);
		if ((status == FLASH_STATUS_UNKNOWN) || (status == FLASH_STATUS_BUSY)) {
			printk(KERN_ERR, "Wrong flash status\n");
			goto fw_done;
		}
		/* wait for a while */
		msleep(FTS_FLASH_COMMAND_DELAY);

		TPD_DMESG("FTS 6) program the flash.\n");
		ret = fts_flash_program(program_command);
		if (ret) {
			printk(KERN_ERR, "Cannot program the flash device\n");
			goto fw_done;
		}
		/* wait for a while */
		msleep(FTS_FLASH_COMMAND_DELAY);

		TPD_DMESG("FTS , Flash programming: done.\n");
	
		TPD_DMESG("Perform a system reset\n");
		ret = fts_systemreset();
		if (ret) {
			printk(KERN_ERR, "Cannot reset the device\n");
			goto fw_done;
		}

		ret = fts_init_flash_reload();
		if (ret) {
			printk(KERN_ERR, "Cannot initialize the hardware device\n");
			goto fw_done;
		}

		ret = fts_get_fw_version();
		if (ret) {
			printk(KERN_ERR, "Cannot retrieve firmware version\n");
			goto fw_done;
		}
		TPD_DMESG("FTS , new chip firmware ver is:0x%x\n",chip_fw_version);
		/*dev_info(info->dev,
			"New firmware version 0x%04x installed\n",
		info->fw_version);	*/

	}
fw_done:

	return ret;
	}
	else {
		TPD_DMESG("FTS PASS firmware update..\n");
		return 0;
	}
}

static int fts_fw_upgrade(void)
{
	int ret;
	mt_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	ret = st_fw_update_controller(&fw_info_st,st_i2c_client);
	
	msleep(10);
   	//irq unmask
	mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	return ret;
}

static int fts_systemreset(void)
{
	int ret;
	unsigned char regAdd[4] =
	{
        0xB6, 0x00, 0x23, 0x01
	};

	TPD_DMESG("FTS SystemReset\n");
	ret = fts_write_reg(&regAdd[0], 4);
	fts_delay(20);
	return ret;
}

unsigned char fts_read_modebyte()
{
    int rc;
    unsigned char data[8];

    unsigned char regAdd[4] =
    {
        0xB2, 0x00, 0x02, 0x01
    };

    fts_write_reg(&regAdd[0], 4);
    fts_delay(10);

    regAdd[0] = READ_ONE_EVENT;
    rc = fts_read_reg( &regAdd[0], 1, (unsigned char *)data,
                      FTS_EVENT_SIZE);

    return data[3];
}

static void fts_wait_sleepout_ready()
{
    int i;

    /*polling maximum time*/
    for (i = 0; i < FTS_EVENT_WAIT_MAX; i++)
    {
        /*sleepout generates forcecal event*/
        if (fts_forcecal_ready)
        {
            fts_forcecal_ready = 0;
            return;
        }
        fts_delay(5);
    }

    TPD_DMESG("fts fts_wait_sleepout_ready end polling\n");
    return;
}

//static void fts_wait_forcecal_ready(struct fts_ts_info *info)
static void fts_wait_forcecal_ready(void)
{
    int i;

    /*polling maximum time*/
    for (i = 0; i < FTS_EVENT_WAIT_MAX; i++)
    {
        /*sleepout generates forcecal event*/
        if (fts_forcecal_ready)
        {
            fts_forcecal_ready = 0;
            return;
        }
        fts_delay(5);
    }

    TPD_DMESG("fts fts_wait_forcecal_ready end polling\n");
    return;
}

//static void fts_wait_Mutual_atune_ready(struct fts_ts_info *info)
static void fts_wait_Mutual_atune_ready(void)
{
    int i;

    /*polling maximum time*/
    for (i = 0; i < FTS_EVENT_WAIT_MAX; i++)
    {
        /*sleepout generates forcecal event*/
        if (fts_mutualTune_ready)
        {
            fts_mutualTune_ready = 0;
            return;
        }
        fts_delay(5);
    }

    TPD_DMESG("fts fts_wait_Mutual_atune_ready end polling\n");
    return;
}

//static void fts_wait_Self_atune_ready(struct fts_ts_info *info)
static void fts_wait_Self_atune_ready(void)
{
    int i;

    /*polling maximum time*/
    for (i = 0; i < FTS_EVENT_WAIT_MAX; i++)
    {
        /*sleepout generates forcecal event*/
        if (fts_selfTune_ready)
        {
            fts_selfTune_ready = 0;
            return;
        }
        fts_delay(5);
    }

    TPD_DMESG("fts fts_wait_Self_atune_ready end polling\n");
    return;
}

static void fts_wait_tuning_backup_ready(void)
{
    int i;

    /*polling maximum time*/
    for (i = 0; i < FTS_EVENT_WAIT_MAX; i++)
    {
        /*sleepout generates forcecal event*/
        if (fts_tuningbackup_ready)
        {
            fts_tuningbackup_ready = 0;
            return;
        }
        fts_delay(5);
    }

    TPD_DMESG("fts fts_wait_tuning_backup_ready end polling\n");
    return;
}

static int fts_interrupt_set(int enable)
{
	int ret;
    unsigned char regAdd[4] =
    {
        0
    };

    regAdd[0] = 0xB6;
    regAdd[1] = 0x00;
    regAdd[2] = 0x1C;
    regAdd[3] = enable;

    if (enable)
    {
        TPD_DMESG("FTS INT Enable\n");
    }
    else
    {
        TPD_DMESG("FTS INT Disable\n");
    }

    ret = fts_write_reg(&regAdd[0], 4);
	return ret;
}

static int fts_init(void)
{
    	unsigned char val[16];
    	unsigned char regAdd[8];
    	int rc = 0;

    	TPD_DMESG("fts_init called\n");

    	/* TS Chip ID */
	regAdd[0] = 0xB6;
    	regAdd[1] = 0x00;
	regAdd[2] = 0x07;
	rc = fts_read_reg(regAdd, 3, (unsigned char *)val, 8);
	TPD_DMESG("FTS %02X%02X%02X = %02x %02x %02x %02x %02x %02x %02x %02x\n",
		regAdd[0], regAdd[1], regAdd[2], 
		val[0], val[1], val[2], val[3], 
		val[4], val[5], val[6], val[7]);
	if ((val[1] != FTS_ID0) || (val[2] != FTS_ID1)) {
		return 1;
	}

	rc += fts_fw_upgrade();
	rc += fts_systemreset();
	
	rc += fts_interrupt_set(INT_DISABLE);
	
	/*mt_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE);
	mt_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
	mt_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_POLARITY_LOW, st_i2c_tpd_eint_interrupt_handler, 1); 
	mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);*/
	//CUST_EINT_TOUCH_PANEL_TYPE is level
	mt_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
	mt_eint_registration(CUST_EINT_TOUCH_PANEL_NUM,  CUST_EINT_TOUCH_PANEL_TYPE, st_i2c_tpd_eint_interrupt_handler, 0); 
	mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
    
	rc += fts_interrupt_set(INT_ENABLE);

	rc += fts_command(SLEEPOUT);
	msleep(20);

	rc += fts_command(CX_TUNNING);
	fts_wait_Mutual_atune_ready();

	rc += fts_command(SELF_TUNING);
	fts_wait_Self_atune_ready();

	rc += fts_command(FORCECALIBRATION);
	fts_wait_forcecal_ready();

	rc += fts_command(SENSEON);
	
	rc += fts_command(BUTTON_ON);
	// rc += fts_command(FLUSHBUFFER);
	if (rc)
    		TPD_DMESG(KERN_ERR "fts initialized failed\n");
	else
		TPD_DMESG(KERN_ERR "fts initialized successful\n");
		
	return rc;
}

static int fts_init_flash_reload(void)
{
    	int rc = 0;

	rc += fts_command(SLEEPOUT);
	msleep(20);

	rc += fts_command(CX_TUNNING);
	fts_wait_Mutual_atune_ready();

	rc += fts_command(SELF_TUNING);
	fts_wait_Self_atune_ready();

	rc += fts_command(FORCECALIBRATION);
	fts_wait_forcecal_ready();
	
	rc += fts_command(TUNING_BACKUP);
	fts_wait_tuning_backup_ready();
	
	rc += fts_command(SENSEON);
	
	rc += fts_command(BUTTON_ON);
	
	rc += fts_command(FLUSHBUFFER);
	// rc += fts_command(FLUSHBUFFER);
	if (rc)
    		TPD_DMESG(KERN_ERR "fts initialized failed\n");
	else
		TPD_DMESG(KERN_ERR "fts initialized successful\n");
		
	return rc;	
}

void fts_reboot(void)
{
    TPD_DMESG("fts_reboot\n");
    #if 0
    hwPowerDown(MT65XX_POWER_LDO_VGP4, "TP");
    //init GPIO pin
    //init CE //GPIO_CTP_RST_PIN, is CE
    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
    //init EINT, mask CTP EINT //GPIO_CTP_EINT_PIN, is RSTB
    mt_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
    mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
    mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
    mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_DOWN);
    msleep(10);  //dummy delay here

    //turn on VDD33, LDP_VGP4
    hwPowerOn(MT65XX_POWER_LDO_VGP4, VOL_3300, "TP");
    msleep(20);  //tce, min is 0, max is ?

    //set CE to HIGH
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
    msleep(50);  //tpor, min is 1, max is 5

    //set RSTB to HIGH 
    mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);
    msleep(50);//t boot_core, typicl is 20, max is 25ms
    msleep(300);
    #endif
}

static void fts_enter_pointer_event_handler(unsigned char *event)
{
	unsigned char touch_id;
	int x, y, z;

	//dev_dbg(info->dev, "Received event 0x%02x\n", event[0]);
	//TPD_DMESG("fts Received event 0x%02x\n", event[0]);
	touch_id = event[1] & 0x0F;
	//__set_bit(touchId, &info->touch_id);
	x = (event[2] << 4) | ((event[4] & 0xF0) >> 4);
	y = (event[3] << 4) | (event[4] & 0x0F);
	z = (event[5] & 0x3F);

	if (x == X_AXIS_MAX)
		x--;
	if (y == Y_AXIS_MAX)
		y--;

	//input_mt_slot(tpd->dev, touch_id);
	input_report_key(tpd->dev, BTN_TOUCH, 1);
	input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, touch_id);
	input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, z);
	input_report_abs(tpd->dev, ABS_MT_PRESSURE, z);

	/*TPD_DMESG("Event 0x%02x - ID[%d], (x, y, z) = (%3d, %3d, %3d)\n",
			 *event, touch_id, x, y, z);*/

	return;
}

static void fts_motion_pointer_event_handler(unsigned char *event)
{
	unsigned char touch_id;
	int x, y, z;

	//dev_dbg(info->dev, "Received event 0x%02x\n", event[0]);
	//TPD_DMESG("fts Received event 0x%02x\n", event[0]);
	touch_id = event[1] & 0x0F;
	//__set_bit(touchId, &info->touch_id);
	x = (event[2] << 4) | ((event[4] & 0xF0) >> 4);
	y = (event[3] << 4) | (event[4] & 0x0F);
	z = (event[5] & 0x3F);

	if (x == X_AXIS_MAX)
		x--;
	if (y == Y_AXIS_MAX)
		y--;

	//input_mt_slot(tpd->dev, touch_id);
	input_report_key(tpd->dev, BTN_TOUCH, 1);
	input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, touch_id);
	input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, z);
	input_report_abs(tpd->dev, ABS_MT_PRESSURE, z);

	//TPD_DMESG("Event 0x%02x - ID[%d], (x, y, z) = (%3d, %3d, %3d)\n",
			 //*event, touch_id, x, y, z);

	return;
}

static void fts_leave_pointer_event_handler(unsigned char *event)
{
	unsigned char touch_id;

	//dev_dbg(info->dev, "Received event 0x%02x\n", event[0]);
	//TPD_DMESG("fts Received event 0x%02x\n", event[0]);
	touch_id = event[1] & 0x0F;
	//__clear_bit(touchId, &info->touch_id);

	//input_mt_slot(tpd->dev, touch_id);
	input_report_key(tpd->dev, BTN_TOUCH, 0);

	//TPD_DMESG("Event 0x%02x - release ID[%d]\n", event[0], touch_id);
	return;
}

static void fts_unknown_event_handler(unsigned char *event)
{
    	TPD_DMESG("fts unknown event : %02X %02X %02X %02X %02X %02X %02X %02X\n",
           event[0], event[1], event[2], event[3],
           event[4], event[5], event[6], event[7]);
}

static void fts_print_event_handler(unsigned char *event)
{
    	printk(KERN_ERR "fts event : %02X %02X %02X %02X %02X %02X %02X %02X\n",
           event[0], event[1], event[2], event[3],
           event[4], event[5], event[6], event[7]);
}

static void fts_status_event_handler(unsigned char *event)
{
	unsigned char status_event = 0;
	status_event = event[1];
	switch (status_event)
                {
                    case FTS_STATUS_MUTUAL_TUNE:
                        fts_mutualTune_ready = 1;
                        break;

                    case FTS_STATUS_SELF_TUNE:
                        fts_selfTune_ready = 1;
                        break;

                    case FTS_FORCE_CAL_SELF_MUTUAL:
                        fts_forcecal_ready = 1;
                        break;

                    default:
                        TPD_DMESG("fts not handled event code\n");
                        break;
                }	
	return;
}

static void fts_button_status_event_handler(unsigned char *event)
{
	int i;
	unsigned char buttons, changed, touch_id;
	unsigned char key;
	bool on;
	
	TPD_DMESG("fts received event 0x%02x\n", event[0]);
	/* get current buttons status */
	/* buttons = event[1]; */
	/* mutual capacitance key */
	buttons = event[2];
	//TPD_DMESG("fts button old_button 0x%02x 0x%02x\n", buttons, old_buttons);
	/* check what is changed */
	changed = buttons ^ old_buttons;

	//dev_dbg(info->dev, "Received event 0x%02x\n", event[0]);
	//TPD_DMESG("fts Received event 0x%02x\n", event[0]);
	touch_id = event[1] & 0x0F;

	
	for (i = 0; i < 8; i++) {
		if (changed & (1 << i)) {
			
			key = (1 << i);
			on = ((buttons & key) >> i);
			//TPD_DMESG("fts key on/off 0x%02x 0x%02x\n", key, on);
			switch(key)
				{
        			case 1:
            				TPD_DMESG("ST_KEY_EVENT BACK, %d \n",on);
							if(on){
									input_mt_slot(tpd->dev, touch_id);
									input_report_key(tpd->dev, BTN_TOUCH, 1);
									input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, touch_id);
									input_report_abs(tpd->dev, ABS_MT_POSITION_X, 270);
									input_report_abs(tpd->dev, ABS_MT_POSITION_Y, 1980);
								}else{
									input_mt_slot(tpd->dev, touch_id);
									input_report_key(tpd->dev, BTN_TOUCH, 0);
									input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, -1);
								}
        				break;
        
        			case 2:
            				TPD_DMESG("ST_KEY_EVENT HOME, %d \n",on);
							if(on){
									input_mt_slot(tpd->dev, touch_id);
									input_report_key(tpd->dev, BTN_TOUCH, 1);
									input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, touch_id);
									input_report_abs(tpd->dev, ABS_MT_POSITION_X, 540);
									input_report_abs(tpd->dev, ABS_MT_POSITION_Y, 1980);
								}else{
									input_mt_slot(tpd->dev, touch_id);
									input_report_key(tpd->dev, BTN_TOUCH, 0);
									input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, -1);
								}

        				break;

        			case 4:
            				TPD_DMESG("ST_KEY_EVENT MENU, %d \n",on);
							if(on){
									input_mt_slot(tpd->dev, touch_id);
									input_report_key(tpd->dev, BTN_TOUCH, 1);
									input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, touch_id);
									input_report_abs(tpd->dev, ABS_MT_POSITION_X, 810);
									input_report_abs(tpd->dev, ABS_MT_POSITION_Y, 1980);
								}else{
									input_mt_slot(tpd->dev, touch_id);
									input_report_key(tpd->dev, BTN_TOUCH, 0);
									input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, -1);
								}

        				break;
        
        			default:
        				break;
				}
		}
			/*input_report_key(tpd->dev,
				BTN_0 + i,
				(!(info->buttons & (1 << i))));*/			
	}
	
	old_buttons = buttons;
	return;	
}

static int st_touch_event_handler(void *unused)
{
#if 1
	// u8 buf[TS_READ_REGS_LEN] = { 0 };
	int i, read_num, fingerID, Touch_Type = 0, touchState = 0;//, keyID = 0;
	int id = 0;	
	unsigned char data[FTS_EVENT_SIZE * FTS_FIFO_MAX];
	int rc;
	unsigned char regAdd;
	int left_events = 0;
	int new_left_events = 0;
	int total_events = 1;
	int read_error_flag = 0;
	unsigned char event_id;
	unsigned char tmp_data[8];
	unsigned char firstleftevent = 0;
    	
	struct sched_param param = { .sched_priority = RTPM_PRIO_TPD }; 
	sched_setscheduler(current, SCHED_RR, &param);
	
	do {    
	//mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);	
		set_current_state(TASK_INTERRUPTIBLE);		
		wait_event_interruptible(st_waiter, st_tpd_flag != 0);		

		set_current_state(TASK_RUNNING); 
		
		st_tpd_flag = 0;
		read_error_flag = 0;
		total_events = 1;
		// memset(data, 0x0, FTS_EVENT_SIZE * FTS_FIFO_MAX);
		regAdd = READ_ONE_EVENT;
		rc = fts_read_reg(&regAdd, 1, (unsigned char *)data, FTS_EVENT_SIZE);
		// rc = fts_dma_read_reg(&regAdd, 1, data, FTS_EVENT_SIZE);
		if (rc == 0) 
		{
			left_events = data[7] & 0x1F;

			//read remaining events
			while ((left_events > 0) && (total_events < (FTS_FIFO_MAX-1))) 
			{
				//memset(tmp_data, 0x0, FTS_EVENT_SIZE);
				if (left_events > READ_EVENT_SIZE) 
				{				
					regAdd = READ_ALL_EVENT;
					rc = fts_dma_read_reg(&regAdd, sizeof(regAdd),&data[total_events*FTS_EVENT_SIZE],READ_EVENT_SIZE*FTS_EVENT_SIZE);
					if(rc == 0 )
						total_events += READ_EVENT_SIZE;	
				} else {
					regAdd = READ_ALL_EVENT;
					rc = fts_dma_read_reg(&regAdd, sizeof(regAdd),&data[total_events*FTS_EVENT_SIZE],left_events*FTS_EVENT_SIZE);
					if(rc == 0)
						total_events += left_events;
				}
				
				if (rc != 0) {
					TPD_DMESG("fts read fifo error2\n");
					read_error_flag = 1;
					data[7] &= 0xE0;
				}			
				left_events = data[7] & 0x1F;
				// memcpy(&data[total_events*FTS_EVENT_SIZE], tmp_data, FTS_EVENT_SIZE);
				   			        
				
			} 
			
			// printk(KERN_ERR "fts total events = %d\n",total_events);
			for (i=0; i<total_events; i++) {
				// fts_print_event_handler(&data[i*FTS_EVENT_SIZE]);
				event_id = data[i*FTS_EVENT_SIZE];
				//event_ptr = &data[i*FTS_EVENT_SIZE];

			//	TPD_DMESG("event_id = %d\n", event_id); //zijian debug
				switch(event_id)
				{
				case EVENTID_NO_EVENT:
	        			break;	
				case EVENTID_ENTER_POINTER:
					fts_enter_pointer_event_handler(&data[i*FTS_EVENT_SIZE]);
					input_mt_sync(tpd->dev);
					break;
				case EVENTID_MOTION_POINTER:
					fts_motion_pointer_event_handler(&data[i*FTS_EVENT_SIZE]);
					input_mt_sync(tpd->dev);
					break;
				case EVENTID_LEAVE_POINTER:
					fts_leave_pointer_event_handler(&data[i*FTS_EVENT_SIZE]);
					input_mt_sync(tpd->dev);
					break;
				case EVENTID_BUTTON_STATUS:
					fts_button_status_event_handler(&data[i*FTS_EVENT_SIZE]);
	        		break;
	        	case EVENTID_STATUS:
	        		fts_status_event_handler(&data[i*FTS_EVENT_SIZE]);
	        		break;	
	        		case EVENTID_ERROR:
	        			break;
				default:
					fts_unknown_event_handler(&data[i*FTS_EVENT_SIZE]);
					break;
				}
			}
			
			if ( (tpd != NULL) && (tpd->dev != NULL) )
	    		input_sync(tpd->dev);
			
		} else {
			TPD_DMESG("fts read fifo error1\n");
			read_error_flag = 1;    			
		}
		
		if (read_error_flag == 1) {
			TPD_DMESG("fts read fifo error\n");
			fts_command(FLUSHBUFFER);
		}

	//mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);	
	} while ( !kthread_should_stop() ); 

	return 0;
#endif
}

static void st_i2c_tpd_eint_interrupt_handler(void)
{ 
    //TPD_DMESG_PRINT_INT;
    mt_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
    st_tpd_flag=1;    
    interrupt_count++;
  //  TPD_DMESG("fts,interrupt_count = %d \n",interrupt_count);//zijian for debug
    wake_up_interruptible(&st_waiter);
} 

static int tpd_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{             
 	int err = 0;
 	// u8 chip_info = 0;
 	int i_ret = 0;
 	int retry_count = 0;    
	#define  TPD_MAX_TRY_COUNT  3
	
	struct task_struct *thread = NULL;
	unsigned char val[5];
	unsigned char regAdd[3];
	int error;
	
	TPD_DMESG("[st_tpd] %s\n", __func__);	

	//TP sysfs debug support
	//input_mt_init_slots(tpd->dev, TOUCH_ID_MAX);
	tpswitch_sysfs_init();
	old_buttons = 0;

	//TP DMA support
	if(DMAbuffer_va == NULL)
	DMAbuffer_va = (u8 *)dma_alloc_coherent(NULL, 4096,&DMAbuffer_pa, GFP_KERNEL);

	TPD_DMESG("dma_alloc_coherent va = 0x%8x, pa = 0x%8x \n",DMAbuffer_va,DMAbuffer_pa);
	if(!DMAbuffer_va)
	{
	TPD_DMESG("Allocate DMA I2C Buffer failed!\n");
	return -1;
	}

#ifdef TPD_POWER_SOURCE_CUSTOM
    hwPowerOn(TPD_POWER_SOURCE_CUSTOM, VOL_2800, "TP");
#else
	hwPowerOn(MT65XX_POWER_LDO_VGP2, VOL_2800, "TP");    
#endif

#ifdef TPD_POWER_SOURCE_1800
    hwPowerOn(TPD_POWER_SOURCE_1800, VOL_1800, "TP");
#endif 

reset_proc:
    
	st_i2c_client = client;

	msleep(100);
	
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
	msleep(100);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(20);
    	
    mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
	mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
	mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);
			
	err = fts_init();
	if (err)
	{
		TPD_DMESG(KERN_ERR "FTS fts_init fail# retry_count = %d!\n", retry_count);
		
        if ( retry_count < TPD_MAX_TRY_COUNT )
        {
            retry_count++;
            goto reset_proc;
        }

		TPD_DMESG(KERN_ERR "tpd_i2c_probe fail\n");

		#ifdef TPD_POWER_SOURCE_CUSTOM
			hwPowerDown(TPD_POWER_SOURCE_CUSTOM,  "TP");
		#else
			hwPowerDown(MT65XX_POWER_LDO_VGP2, "TP");    
		#endif

		#ifdef TPD_POWER_SOURCE_1800
			hwPowerDown(TPD_POWER_SOURCE_1800,  "TP");
		#endif 
		return -1;
	}
    	/*
	if (melfas_firmware_update(client) < 0)
	{
	//if firmware update failed, reset IC
        //goto reset_proc;
    	}*/

	/*
	mt_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_LEVEL_SENSITIVE); //level    
    	mt_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
    	mt_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_TOUCH_PANEL_POLARITY, st_i2c_tpd_eint_interrupt_handler, 0); 
    	mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	*/
    	thread = kthread_run(st_touch_event_handler, 0, TPD_DEVICE);

    	if (IS_ERR(thread))
    	{ 
        err = PTR_ERR(thread);
        TPD_DMESG(TPD_DEVICE "[melfas_tpd] failed to create kernel thread: %d\n", err);
    	}    
    	mdelay(5);

			
			TPD_DMESG(KERN_ERR "tpd_i2c_probe success \n");
	
    	#if 0
	/*try 3 times handshake*/
	do {
		try_count++;	
		i_ret = melfas_i2c_read(st_i2c_client, MMS_CHIP_INFO, 1, &chip_info);
		if (i_ret < 0)
			mms_reboot();
	} while( (i_ret < 0) && (try_count <= 3) );
	TPD_DMESG("[melfas_tpd] communication times is %d\n", try_count);
	if (i_ret < 0)
		return -1;
	if (chip_info == MMS244_CHIP)
		TPD_DMESG("[melfas_tpd] MMS-244 was probed successfully \n");
	/*
	sometimes tp do not work,so add rst operation
	*/	
	#endif
    	tpd_load_status = 1;
    	return 0;
}

static int tpd_i2c_remove(struct i2c_client *client)
{
	TPD_DMESG("[ST_tpd] %s\n", __func__);
	if(DMAbuffer_va) {
		dma_free_coherent(NULL, 4096, DMAbuffer_va, DMAbuffer_pa);
		DMAbuffer_va = NULL;
		DMAbuffer_pa = 0;
	}
	input_mt_destroy_slots(tpd->dev);
	tpswitch_sysfs_exit();
	//TP_sysfs_exit();
	return 0;
}

static int tpd_i2c_detect(struct i2c_client *client, 
			struct i2c_board_info *info)
{
	TPD_DMESG("[ST_tpd] %s\n", __func__);
	strcpy(info->type, "mtk-tpd");
	return 0;
}
 
static int tpd_local_init(void)
{
 
	TPD_DMESG("STtech FTS I2C Touchscreen Driver (Built %s @ %s)\n", __DATE__, __TIME__);
	if(i2c_add_driver(&tpd_i2c_driver)!=0)
   	{
  		TPD_DMESG("fts unable to add i2c driver.\n");
      	return -1;
    	}
    	if(tpd_load_status == 0) 
    	{
    	TPD_DMESG("fts add error touch panel driver.\n");
    	i2c_del_driver(&tpd_i2c_driver);
    	return -1;
    	}    	
//	set_bit(KEY_PALMEVENT, tpd->dev->keybit);
#ifdef TPD_HAVE_BUTTON     
    	tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);// initialize tpd button data
#endif    
	TPD_DMESG("end %s, %d\n", __FUNCTION__, __LINE__);  
	tpd_type_cap = 1;

//set tp resulution 
	TPD_RES_X = X_AXIS_MAX ;
	TPD_RES_Y = Y_AXIS_MAX;
	
	TPD_DMESG("tpd_local_init, change TPD_RES_X to %d,TPD_RES_Y to %d \n",TPD_RES_X,TPD_RES_Y );

	return 0; 
}

static void tpd_resume( struct early_suspend *h )
{
  int rc = 0;
	TPD_DMESG("[st_tpd] %s\n", __FUNCTION__);
	/*power up touch*/	
#ifdef TPD_POWER_SOURCE_CUSTOM
		hwPowerOn(TPD_POWER_SOURCE_CUSTOM, VOL_2800, "TP");
#else
		hwPowerOn(MT65XX_POWER_LDO_VGP2, VOL_2800, "TP");	 
#endif
	
#ifdef TPD_POWER_SOURCE_1800
		hwPowerOn(TPD_POWER_SOURCE_1800, VOL_1800, "TP");
#endif 
	msleep(100);

	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
	msleep(100);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(20);

	/* enable interrupts */
	fts_interrupt_set(INT_ENABLE);

/*****************************************/
rc += fts_command(SLEEPOUT);
msleep(20);

rc += fts_command(CX_TUNNING);
fts_wait_Mutual_atune_ready();

rc += fts_command(SELF_TUNING);
fts_wait_Self_atune_ready();

rc += fts_command(FORCECALIBRATION);
fts_wait_forcecal_ready();

rc += fts_command(SENSEON);

rc += fts_command(BUTTON_ON);

/*****************************************/

	/* put back the device in the original mode (see fts_suspend()) */
	switch (modebyte) {
	case MODE_PROXIMITY:
		fts_command(PROXIMITY_ON);
		break;

	case MODE_HOVER:
		fts_command(HOVER_ON);
		break;

	case MODE_GESTURE:
		fts_command(GESTURE_ON);
		break;

	case MODE_HOVER_N_PROXIMITY:
		fts_command(HOVER_ON);
		fts_command(PROXIMITY_ON);
		break;

	case MODE_GESTURE_N_PROXIMITY:
		fts_command(GESTURE_ON);
		fts_command(PROXIMITY_ON);
		break;

	case MODE_GESTURE_N_PROXIMITY_N_HOVER:
		fts_command(HOVER_ON);
		fts_command(GESTURE_ON);
		fts_command(PROXIMITY_ON);
		break;

	default:
		TPD_DMESG("fts Invalid device mode - 0x%02x\n",
				modebyte);
		break;
	}
	mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
}

static void tpd_suspend( struct early_suspend *h )
{
 
	TPD_DMESG("[st_tpd] %s\n", __FUNCTION__);
	
	old_buttons = 0;		
	mt_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);

    	/* Read out mode byte */
    	modebyte = fts_read_modebyte();
		TPD_DMESG("fts device mode - 0x%02x\n",
				modebyte);

    	fts_command(SLEEPIN);
    	fts_command(FLUSHBUFFER);

    	fts_interrupt_set(INT_DISABLE);
    	st_ts_release_all_finger();    

	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);

#ifdef TPD_POWER_SOURCE_CUSTOM
	hwPowerDown(TPD_POWER_SOURCE_CUSTOM,  "TP");
#else
	hwPowerDown(MT65XX_POWER_LDO_VGP2, "TP");	 
#endif

#ifdef TPD_POWER_SOURCE_1800
	hwPowerDown(TPD_POWER_SOURCE_1800,	"TP");
#endif 

}

static struct tpd_driver_t tpd_device_driver = {
		 .tpd_device_name = "ST_FTS",
		 .tpd_local_init = tpd_local_init,
		 .suspend = tpd_suspend,
		 .resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
		 .tpd_have_button = 1,
#else
		 .tpd_have_button = 0,
#endif		
};
 
/* called when loaded into kernel */
static int __init tpd_driver_init(void) {
	printk("MediaTek st_fts touch panel driver init\n");
	i2c_register_board_info(TPD_I2C_NUMBER, &st_fts_i2c_tpd, 1);
	if(tpd_driver_add(&tpd_device_driver) < 0)
		 TPD_DMESG("add st_fts driver failed\n");
	 return 0;
}
 
/* should never be called */
static void __exit tpd_driver_exit(void) {
	TPD_DMESG("MediaTek st_fts touch panel driver exit\n");
	//input_unregister_device(tpd->dev);
	tpd_driver_remove(&tpd_device_driver);
}

module_init(tpd_driver_init);
module_exit(tpd_driver_exit);
