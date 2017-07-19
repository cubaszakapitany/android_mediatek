 
#include "tpd.h"
#include <linux/interrupt.h>
#include <cust_eint.h>
#include <linux/i2c.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/wait.h>

#include <linux/dma-mapping.h>
#include "tpd_custom_FT5x36.h"
#if defined(MT6577)
#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>
#elif defined(MT6575)
#include <mach/mt6575_pm_ldo.h>
#include <mach/mt6575_typedefs.h>
#include <mach/mt6575_boot.h>
#elif defined(CONFIG_ARCH_MT6573)
#include <mach/mt6573_boot.h>
#endif

#include <linux/miscdevice.h>
#include <asm/uaccess.h>


extern void mt65xx_eint_unmask(unsigned int line);
extern void mt65xx_eint_mask(unsigned int line);
extern char *fts_get_vendor_name(int vendor_id);
#define ISP_FLASH_SIZE	0x8000 //32KB

#define FT5x36_FIRMWAIR_VERSION_D

/*==========================================*/
//static tinno_ts_data *g_pts = NULL;
FTS_BYTE reg_val[2] = {0,0};
u8 is_5336_fwsize_30 = 0;
u8 is_5336_new_bootloader = 0;


#define    BL_VERSION_LZ4        0
#define    BL_VERSION_Z7         1
#define    BL_VERSION_GZF        2


////////////////////////////////////////////////////////////////////
//add by liyaohua



/*
FTS_BOOL i2c_read_interface(FTS_BYTE bt_ctpm_addr, FTS_BYTE* pbt_buf, 
FTS_DWRD dw_lenth)
{
    int ret = 0;
	struct i2c_msg msgs[2];

	memset(msgs,0,sizeof(msgs));
#if 0    
    ret=i2c_master_recv(this_client, pbt_buf, dw_lenth);
#else
	msgs[0].addr = this_client->addr;
	msgs[0].flags = I2C_M_RD;
	msgs[0].len = dw_lenth;
	msgs[0].timing = 0;
	msgs[0].buf = pbt_buf;
	ret = i2c_transfer(this_client->adapter, msgs, 1);
	if (ret < 0)
		printk("device addr = 0x%x, write addr = 0x%x error in %s func: %d\n", 
this_client->addr, pbt_buf[0], __func__, ret);	
#endif

    if(ret<=0)
    {
        printk("[FTS]i2c_read_interface error\n");
        return FTS_FALSE;
    }
  
    return FTS_TRUE;
}


FTS_BOOL byte_read(FTS_BYTE* pbt_buf, FTS_BYTE bt_len)
{
    //return i2c_read_interface(I2C_CTPM_ADDRESS, pbt_buf, bt_len);
    return i2c_read_interface(0, pbt_buf, bt_len);
}
FTS_BOOL cmd_write(FTS_BYTE btcmd,FTS_BYTE btPara1,FTS_BYTE btPara2,FTS_BYTE 
btPara3,FTS_BYTE num)
{
    FTS_BYTE write_cmd[4] = {0};

    write_cmd[0] = btcmd;
    write_cmd[1] = btPara1;
    write_cmd[2] = btPara2;
    write_cmd[3] = btPara3;
//    return i2c_write_interface(I2C_CTPM_ADDRESS, write_cmd, num);
    return i2c_write_interface(0, write_cmd, num);
   //return ft6x06_i2c_Write(i2c_client,write_cmd, num);

}*/

////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
static int fts_i2c_read_a_byte_data(struct i2c_client *client, char reg)
{
	int ret;
	uint8_t iic_data;
	ret = i2c_smbus_read_i2c_block_data(client, reg, 1, &iic_data);
	if (iic_data < 0 || ret < 0){
		CTP_DBG("%s: i2c error, ret=%d\n", __func__, ret);
		return -1;
	}
	return (int)iic_data;
}

static inline int _lock(atomic_t *excl)
{
	if (atomic_inc_return(excl) == 1) {
		return 0;
	} else {
		atomic_dec(excl);
		return -1;
	}
}

static inline void _unlock(atomic_t *excl)
{
	atomic_dec(excl);
}

static ssize_t fts_isp_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{	
	//ret = copy_to_user(buf,&acc, sizeof(acc));
	CTP_DBG("");
	return -EIO;
}

static DECLARE_WAIT_QUEUE_HEAD(waiter_write);
static volatile	int write_flag;

 static int isp_thread(void *para)
 {
	int rc = 0;
	struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };
	tinno_ts_data *ts = (tinno_ts_data *)para;
	sched_setscheduler(current, SCHED_RR, &param);
	set_current_state(TASK_RUNNING);
		
	rc = //
	fts_i2c_write_block(ts->isp_pBuffer, ts->isp_code_count);
	if (rc != ts->isp_code_count) {
		CTP_DBG("i2c_transfer failed(%d)", rc);
		ts->isp_code_count = -EAGAIN;
	} else{
		ts->isp_code_count = rc;
	}
	write_flag = 1;
	wake_up_interruptible(&waiter_write);
		
	return 0;
 }

static ssize_t fts_isp_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
	int rc = 0;
	tinno_ts_data *ts = file->private_data;
	char __user *start = buf;
	CTP_DBG("count = %d, offset=%d", count, (int)(*offset));
	
	if ( count > ISP_FLASH_SIZE ){
		CTP_DBG("isp code is too long.");
		return -EDOM;
	}

	if ( copy_from_user(ts->isp_pBuffer, start, count) ){
		CTP_DBG("copy failed(%d)", rc);
		return -EACCES;
	}

	ts->isp_code_count = count;
	write_flag = 0;
	ts->thread_isp= kthread_run(isp_thread, ts, TPD_DEVICE"-ISP");
	if (IS_ERR(ts->thread_isp)){ 
		rc = PTR_ERR(ts->thread_isp);
		CTP_DBG(" failed to create kernel thread: %d\n", rc);
		return rc;
	} 

	//block user thread
	wait_event_interruptible(waiter_write, write_flag!=0);
	
	return ts->isp_code_count;
}

static int fts_isp_open(struct inode *inode, struct file *file)
{
	CTP_DBG("try to open isp.");

	if ( atomic_read( &g_pts->ts_sleepState ) ){
		CTP_DBG("TP is in sleep state, please try again latter.");
		return -EAGAIN;
	}

	if (_lock(&g_pts->isp_opened)){
		CTP_DBG("isp is already opened.");
		return -EBUSY;
	}
		
	file->private_data = g_pts;

	g_pts->isp_pBuffer = (uint8_t *)kmalloc(ISP_FLASH_SIZE, GFP_KERNEL);
	if ( NULL == g_pts->isp_pBuffer ){
		_unlock ( &g_pts->isp_opened );
		CTP_DBG("no memory for isp.");
		return -ENOMEM;
	}
	
//	mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	
//	FT5x36_complete_unfinished_event();
	
#ifdef CONFIG_TOUCHSCREEN_FT5X05_DISABLE_KEY_WHEN_SLIDE
		fts_5x36_key_cancel();
#endif

	wake_lock(&g_pts->wake_lock);

	CTP_DBG("isp open success.");
	return 0;
}

static int fts_isp_close(struct inode *inode, struct file *file)
{
	tinno_ts_data *ts = file->private_data;
	
	CTP_DBG("try to close isp.");
	
	if ( !atomic_read( &g_pts->isp_opened ) ){
		CTP_DBG("no opened isp.");
		return -ENODEV;
	}
	
	kfree(ts->isp_pBuffer);
	ts->isp_pBuffer = NULL;
	
	file->private_data = NULL;
	
	_unlock ( &ts->isp_opened );
	
	wake_unlock(&ts->wake_lock);
	
//	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);  
	
	CTP_DBG("close isp success!");
	return 0;
}

static int fts_switch_to_update(tinno_ts_data *ts)
{
	int ret = 0, i=0;
	//uint8_t arrCommand[] = {0x55, 0xaa};
	uint8_t arrCommand[] = {0x55};
	uint8_t arrCommand1[] = {0xaa};
	CTP_DBG("fts_switch_to_update");
	
	
	/*write 0xaa to register 0xfc*/
/*	ret = fts_write_reg(0xFC, 0x55);
	if (ret < 0) {
		CTP_DBG("write 0xaa to register 0xfc failed");
		goto err;
	}
	msleep(50);*/

	/*write 0x55 to register 0xfc*/
/*
	ret = fts_write_reg(0xFC, 0xaa);
	if (ret < 0) {
		CTP_DBG("write 0x55 to register 0xfc failed");
		goto err;
	}
	msleep(40);
*/
	do{
		mutex_lock(&ts->mutex);
		//ret = i2c_master_send(ts->client, (const char*)arrCommand, sizeof(arrCommand));
		ret = ft5x0x_i2c_txdata(arrCommand, 1);
		msleep(5);
		ret = ft5x0x_i2c_txdata(arrCommand1, 1);
		msleep(10);
		mutex_unlock(&ts->mutex);
		++i;
	}while(ret < 0 && i < 5);

	//ret = 0;
//err:
	return ret;
}

static int fts_mode_switch(tinno_ts_data *ts, int iMode)
{
	int ret = 0;
	
	CTP_DBG("iMode=%d", iMode);
	
	if ( FTS_MODE_OPRATE == iMode ){
	}
	else if (FTS_MODE_UPDATE == iMode){
		ret = fts_switch_to_update(ts);
	}
	else if (FTS_MODE_SYSTEM == iMode){
	}
	else{
		CTP_DBG("unsupport mode %d", iMode);
	}
	return ret;
}

static int fts_ctpm_auto_clb(void)
{
    unsigned char uc_temp[1];
    unsigned char i ;

    CTP_DBG("start auto CLB.\n");
    msleep(200);
    fts_write_reg(0, 0x40);  
    mdelay(100);   //make sure already enter factory mode
    fts_write_reg(2, 0x4);  //write command to start calibration
    mdelay(300);
    for(i=0;i<100;i++)
    {
        fts_read_reg(0,uc_temp);
        if ( ((uc_temp[0]&0x70)>>4) == 0x0)  //return to normal mode, calibration finish
        {
            break;
        }
        mdelay(200);
        CTP_DBG("waiting calibration %d\n",i);
        
    }
    CTP_DBG("calibration OK.\n");
    
    msleep(300);
    fts_write_reg(0, 0x40);  //goto factory mode
    mdelay(100);   //make sure already enter factory mode
    fts_write_reg(2, 0x5);  //store CLB result
    mdelay(300);
    fts_write_reg(0, 0x0); //return to normal mode 
    msleep(300);
    CTP_DBG("store CLB result OK.\n");
    return 0;
}

static int FT5x36_get_tp_id(tinno_ts_data *ts, int *ptp_id)
{
	int rc;
	char tp_id[2];
	*ptp_id = -1;
	
/*	CTP_DBG("Try to get TPID!");
	
	rc = fts_cmd_write(0x90, 0x00, 0x00, 0x00, 4);
	if (rc < 4) {
		CTP_DBG("i2c_master_send failed(%d)", rc);
		return -EIO;
	} 
	
	ts->client->addr = ts->client->addr & I2C_MASK_FLAG;
	rc = i2c_master_recv(ts->client, tp_id, 2);
	if (rc < 2) {
		CTP_DBG("i2c_master_recv failed(%d)", rc);
		return -EIO;
*/
{
	CTP_DBG("adfasdfTry to get TPID!");
	int it = 0;
	u8 auc_i2c_write_buf[10] = {0};
	u8 auc_bootdata_buf[10] = {0};
	FTS_BYTE reg_boot_val[4] = {0};
	for(it =0; it<30;it++)
	{
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(10);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
	msleep(10);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	if(it<=15)
	{
	msleep(it*2);
	}
	else
	{
	msleep(30+(it-15)*3);
	}  

		/*********Step 2:Enter upgrade mode *****/
		printk("[FTS] Step 2:Enter upgrade mode \n");

			auc_i2c_write_buf[0] = 0x55;
		     fts_cmd_write(0x55,0x00,0x00,0x00,1);
			//fts_i2c_Write(ts->client, auc_i2c_write_buf, 1);
			msleep(5);
			auc_i2c_write_buf[0] = 0xaa;
			 fts_cmd_write(0xaa,0x00,0x00,0x00,1);
#if 1
		/*********Step 3:check READ-ID***********************/
		msleep(10);
		auc_i2c_write_buf[0] = 0x90;
		auc_i2c_write_buf[1] = auc_i2c_write_buf[2] = auc_i2c_write_buf[3] =0x00;
		fts_i2c_Read(ts->client, auc_i2c_write_buf, 4, reg_val, 2);
		printk("[FTS] Step 3: it %d CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",it,reg_val[0], reg_val[1]);
		if (reg_val[0] == 0x79&& reg_val[1] == 0x11) 
		{
			printk("[FTS] Step it 3:ss CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0], reg_val[1]);
			break;
		} 
		else 
		{
			printk(&ts->client->dev, "[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0], reg_val[1]);
		}
#endif	
	}
		/********read data bootloader *************************/
/*
		auc_bootdata_buf[0] = 0x03;
		auc_bootdata_buf[1] = 0x00;
		auc_bootdata_buf[2] = 0x78;
		auc_bootdata_buf[3] = 0x42;
		fts_i2c_Read(ts->client, auc_bootdata_buf, 4, reg_boot_val, 4);
		printk("[FTS] read data bootloader reg_boot_val[0]:%x reg_boot_val[1]:%x reg_boot_val[2]:%x reg_boot_val[3]:%x\n",reg_boot_val[0],reg_boot_val[1],reg_boot_val[2],reg_boot_val[3]);
		
	auc_i2c_write_buf[0] = 0xcd;
	fts_i2c_Read(ts->client, auc_i2c_write_buf, 1, reg_val, 1);
*/
}
	*ptp_id = (( int )reg_val[0] << 8) | (( int )reg_val[1]);
	printk("*ptp_id =%x reg_val[0]=%x reg_val[1]=%x ",*ptp_id,reg_val[0],reg_val[1]);
	return 0;
}

static int FT5x36_get_fw_version(tinno_ts_data *ts)
{
	int ret;
	uint8_t fw_version;
	ret = fts_read_reg(0xA6, &fw_version);
	if (ret < 0){
		CTP_DBG("i2c error, ret=%d\n", ret);
		return -1;
	}
	CTP_DBG("=======fw_version=0x%X===========\n", fw_version);
	return (int)fw_version;
}

int get_fw_version_ext(void)
{
	int version = -1;
	if(g_pts)
		version = FT5x36_get_fw_version(g_pts);
	CTP_DBG("fw_version=0x%X\n", version);
	return version;
}
EXPORT_SYMBOL(get_fw_version_ext);
char* get_fw_name(void)
{
   char name[128];
	int panel_vendor  = 0;
	int panel_version = 0;
	FT5x36_get_vendor_version(g_pts, &panel_vendor, &panel_version);
	return fts_get_vendor_name(panel_vendor);
}
EXPORT_SYMBOL(get_fw_name);

static int updatefactory(void)
{
		int it =0;
	u8 auc_i2c_write_buf[10] = {0};
	u8 auc_bootdata_buf[10] = {0};
	FTS_BYTE reg_boot_val[4] = {0};
		unsigned char buf[6];
		for(; it<30;it++)
	{
			CTP_DBG("Try to reset TP!");
		{			
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
		msleep(10);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
		msleep(10);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
		}
				if(it<=15)
				{
				msleep(it*2);
				}
				else
				{
				msleep(30+(it-15)*3);
				}  
				CTP_DBG("Try to reset TP!");
		printk("[FTS] Step 2:Enter upgrade mode \n");
			auc_i2c_write_buf[0] = 0x55;
		     fts_cmd_write(0x55,0x00,0x00,0x00,1);
			msleep(5);
			auc_i2c_write_buf[0] = 0xaa;
			 fts_cmd_write(0xaa,0x00,0x00,0x00,1);
#if 1
		msleep(10);
		auc_i2c_write_buf[0] = 0x90;
		auc_i2c_write_buf[1] = auc_i2c_write_buf[2] = auc_i2c_write_buf[3] =0x00;
		fts_i2c_Read(g_pts->client, auc_i2c_write_buf, 4, reg_val, 2);
		CTP_DBG("[FTS] Step 3: it %d CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",it,reg_val[0], reg_val[1]);
		if (reg_val[0] == 0x79&& reg_val[1] == 0x11) 
		{
			CTP_DBG("[FTS] Step it 3:ss CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0], reg_val[1]);
			break;
		} 
		else 
		{
			dev_err(&g_pts->client->dev, "[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0], reg_val[1]);
		}
#endif	
		}
		auc_i2c_write_buf[0] = 0x03;
		auc_i2c_write_buf[1] = 0x00;
		auc_i2c_write_buf[2] = 0x78;
		auc_i2c_write_buf[3] = 0x42;
		fts_i2c_Read(g_pts->client, auc_bootdata_buf, 4, reg_boot_val, 4);
		printk("[FTS] read data bootloader reg_boot_val[0]:%x reg_boot_val[1]:%x reg_boot_val[2]:%x	reg_boot_val[3]:%x\n",reg_boot_val[0],reg_boot_val[1],reg_boot_val[2],reg_boot_val[3]);
		auc_i2c_write_buf[0] = 0xcd;
		fts_i2c_Read(g_pts->client, auc_i2c_write_buf, 1, reg_val, 1);
		CTP_DBG("333333bootloader version = 0x%x\n", reg_val[0]);
		CTP_DBG("333333333");
		buf[0] = 0x3;
		buf[1] = 0x0;
		buf[2] = 0x78;
		buf[3] = 0x0;
		fts_i2c_Read(g_pts->client,buf, 4, buf, 6);
		CTP_DBG("33333333333[FTS] old setting: uc_i2c_addr = 0x%x, uc_io_voltage = %d,uc_panel_factory_id = 0x%x\n",buf[0],  buf[2], buf[4]);
		auc_i2c_write_buf[0] = 0x07;
		fts_i2c_Write(g_pts->client,auc_i2c_write_buf, 1);		
		msleep(200);
		CTP_DBG("33333333333[FTS] old setting: uc_i2c_addr = 0x%x, uc_io_voltage = %d,uc_panel_factory_id = 0x%x\n",buf[0],  buf[2], buf[4]);
		}

static int FT5x36_get_vendor_from_bootloader(tinno_ts_data *ts, uint8_t *pfw_vendor, uint8_t *pfw_version)
{
	int rc = 0, tp_id;
	uint8_t version_id = 0, buf[5];
	
	CTP_DBG("Try to switch to update mode!");
	
	rc = fts_mode_switch(ts, FTS_MODE_UPDATE);
	if(rc)
	{
		CTP_DBG("switch to update mode error");
		goto err;
	}
	
	rc = FT5x36_get_tp_id(ts, &tp_id);
	CTP_DBG("TPID=0x%X!", tp_id);
	if(rc)
	{
		CTP_DBG("Get tp ID error(%d)", rc);
		//goto err;
	}
	if ( FTS_CTP_FIRWARE_ID != tp_id ){
		CTP_DBG("Tp ID is error(0x%x)", tp_id);
		rc = -EINVAL;
		//goto err;
	}

	//Get vendor ID.
	rc = fts_cmd_write(0xcd, 0x00, 0x00, 0x00, 1);
	if (rc < 1) {
		CTP_DBG("i2c_master_send failed(%d)", rc);
		goto err;
	} 
	rc = i2c_master_recv(ts->client, &version_id, 1);
	if (rc < 1) {
		CTP_DBG("i2c_master_recv failed(%d)", rc);
		goto err;
	} 
	
	//*pfw_version = version_id;
	*pfw_version = -1;//Force to update.
	CTP_DBG("bootloader version = 0x%x\n", version_id);

	/* --------- read current project setting  ---------- */
	//set read start address
	rc = fts_cmd_write(0x03, 0x00, 0x78, 0x00, 4);
	if (rc < 4) {
		CTP_DBG("i2c_master_send failed(%d)", rc);
		goto err;
	} 
	
	rc = i2c_master_recv(g_pts->client, buf, sizeof(buf));
	if (rc < 0){
		CTP_DBG("i2c_master_recv failed(%d)", rc);
		goto err;
	}

	CTP_DBG("vendor_id = 0x%x\n", buf[4]);
	
	*pfw_vendor = buf[4];
	
	CTP_DBG("Try to reset TP!");
	rc = fts_cmd_write(0x07,0x00,0x00,0x00,1);
	if (rc < 0) {
		CTP_DBG("reset failed");
		goto err;
	}
	msleep(200);
	
	return 0;
err:
	return rc;
}

int FT5x36_get_vendor_version(tinno_ts_data *ts, uint8_t *pfw_vendor, uint8_t *pfw_version)
{
	int ret;
	*pfw_version = FT5x36_get_fw_version(ts);

	ret = fts_read_reg(0xA8, pfw_vendor);
	if (ret < 0){
		CTP_DBG("i2c error, ret=%d\n", ret);
		return ret;
	}
	if ( 0xA8 == *pfw_vendor)// || 0x00 == *pfw_vendor )
		{
		CTP_DBG("FW in TP has problem, get factory ID from bootloader.\n");
		ret = FT5x36_get_vendor_from_bootloader(ts, pfw_vendor, pfw_version);
		if (ret){
			CTP_DBG("FT5x36_get_vendor_from_bootloader error, ret=%d\n", ret);
			return -EFAULT;
		}
	}
	
	printk("[CTP-FT5x36] fw_vendor=0x%X, fw_version=0x%X\n", *pfw_vendor, *pfw_version);
	return 0;
}
EXPORT_SYMBOL(FT5x36_get_vendor_version);
static int fts_isp_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	tinno_ts_data *ts = file->private_data;
	int flag;
	int rc = 0;
	
	if ( !atomic_read( &g_pts->isp_opened ) ){
		CTP_DBG("no opened isp.");
		return -ENODEV;
	}
	
	/* check cmd */
	if(_IOC_TYPE(cmd) != TOUCH_IO_MAGIC)	
	{
		CTP_DBG("cmd magic type error");
		return -EINVAL;
	}
	if(_IOC_NR(cmd) > FT5X36_IOC_MAXNR)
	{
		CTP_DBG("cmd number error");
		return -EINVAL;
	}

	if(_IOC_DIR(cmd) & _IOC_READ)
		rc = !access_ok(VERIFY_WRITE,(void __user*)arg, _IOC_SIZE(cmd));
	else if(_IOC_DIR(cmd) & _IOC_WRITE)
		rc = !access_ok(VERIFY_READ, (void __user*)arg, _IOC_SIZE(cmd));
	if(rc)
	{
		CTP_DBG("cmd access_ok error");
		return -EINVAL;
	}

	switch (cmd) {
	case FT5X36_IOCTL_SWITCH_TO:
	/*	CTP_DBG("Try to switch to update mode!(%lu)", arg);
		rc = fts_mode_switch(ts, (int)arg);
		if(rc)
		{
			CTP_DBG("switch to update mode error");
			return -EIO;
		}
	*/
	msleep(300);
	updatefactory();
	{
	int it = 0;
	u8 auc_i2c_write_buf[10] = {0};
	u8 auc_bootdata_buf[10] = {0};
	FTS_BYTE reg_boot_val[4] = {0};
	for(it =0; it<30;it++)
	{
	{			
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
		msleep(10);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
		msleep(10);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	}
				if(it<=15)
				{
				msleep(it*2);
				}
				else
				{
				msleep(30+(it-15)*3);
				}  

		/*********Step 2:Enter upgrade mode *****/
		printk("[FTS] Step 2:Enter upgrade mode \n");

			auc_i2c_write_buf[0] = 0x55;
		     fts_cmd_write(0x55,0x00,0x00,0x00,1);
			//fts_i2c_Write(ts->client, auc_i2c_write_buf, 1);
			msleep(5);
			auc_i2c_write_buf[0] = 0xaa;
			 fts_cmd_write(0xaa,0x00,0x00,0x00,1);
#if 1
		/*********Step 3:check READ-ID***********************/
		msleep(10);
		auc_i2c_write_buf[0] = 0x90;
		auc_i2c_write_buf[1] = auc_i2c_write_buf[2] = auc_i2c_write_buf[3] =0x00;
		fts_i2c_Read(ts->client, auc_i2c_write_buf, 4, reg_val, 2);
		CTP_DBG("[FTS] Step 3: it %d CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",it,reg_val[0], reg_val[1]);
		if (reg_val[0] == 0x79&& reg_val[1] == 0x11) 
		{
			CTP_DBG("[FTS] Step it 3:ss CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0], reg_val[1]);
			break;
		} 
		else 
		{
			dev_err(&ts->client->dev, "[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0], reg_val[1]);
		}
#endif	
	}
		/********read data bootloader *************************/
		auc_bootdata_buf[0] = 0x03;
		auc_bootdata_buf[1] = 0x00;
		auc_bootdata_buf[2] = 0x78;
		auc_bootdata_buf[3] = 0x42;
		fts_i2c_Read(ts->client, auc_bootdata_buf, 4, reg_boot_val, 4);
		printk("[FTS] read data bootloader reg_boot_val[0]:%x reg_boot_val[1]:%x reg_boot_val[2]:%x	reg_boot_val[3]:%x\n",reg_boot_val[0],reg_boot_val[1],reg_boot_val[2],reg_boot_val[3]);
		
		auc_i2c_write_buf[0] = 0xcd;
		fts_i2c_Read(ts->client, auc_i2c_write_buf, 1, reg_val, 1);

		}
		break;
	case FT5X36_IOCTL_WRITE_PROTECT:
		CTP_DBG("Try to set write protect mode!(%lu)", arg);
		rc = -EINVAL;
		break;
	case FT5X36_IOCTL_ERASE:
	{
		if(is_5336_fwsize_30)
		{
			u8 auc_i2c_write_buf[10] = {0};
			CTP_DBG("Try to erase flash!");
			auc_i2c_write_buf[0] = 0x61;
			fts_i2c_Write(ts->client, auc_i2c_write_buf, 1);	
			/*erase app area */
			msleep(2000);
			/*erase panel parameter area */
			auc_i2c_write_buf[0] = 0x63;
			fts_i2c_Write(ts->client, auc_i2c_write_buf, 1);
			msleep(100);
		}
		else
		{
			u8 auc_i2c_write_buf[10] = {0};
			CTP_DBG("Try to erase flash!");
			auc_i2c_write_buf[0] = 0x61;
		    fts_i2c_Write(ts->client, auc_i2c_write_buf, 1);	/*erase app area */
		    msleep(2000);
		}
		
	}
		break;
	case FT5X36_IOCTL_GET_STATUS:
		CTP_DBG("Try to get status!");
		flag = fts_i2c_read_a_byte_data(ts->client, 0x05);
		if (flag < 0) {
			CTP_DBG("read check status failed");
		}
		CTP_DBG("status=%d!", flag);
		if(copy_to_user(argp,&flag,sizeof(int))!=0)
		{
			CTP_DBG("copy_to_user error");
			rc = -EFAULT;
		}
		break;
		
	case FT5X36_IOCTL_GET_VENDOR_VERSION:
		{
			int rc;
			uint8_t vendor = 0;
			uint8_t version = 0;
			CTP_DBG("Try to get vendor_version!");
			rc = FT5x36_get_vendor_version(ts, &vendor, &version);
			if ( rc < 0 ){
				rc = -EFAULT;
				break;
			}
			if(!vendor)
					vendor=0x5D;
			CTP_DBG("vendor version =%d, version=%d!", vendor, version);
			flag = (vendor<<8)|version;
			if(copy_to_user(argp,&flag, sizeof(int))!=0)
			{
				CTP_DBG("copy_to_user error");
				rc = -EFAULT;
			}

			break;
		}
	case FT5X36_IOCTL_GET_CHECKSUM:
		{
			uint8_t check_sum;
			CTP_DBG("Try to get checksum!");
			fts_cmd_write(0xCC,0x00,0x00,0x00,1);
			ts->client->addr = ts->client->addr & I2C_MASK_FLAG;
			rc = i2c_master_recv(ts->client, &check_sum, 1);
			if (rc < 0) {
				CTP_DBG("read checksum failed");
			}
			CTP_DBG("checksum=%d!", check_sum);
			flag = check_sum;
			if(copy_to_user(argp,&flag, sizeof(int))!=0)
			{
				CTP_DBG("copy_to_user error");
				rc = -EFAULT;
			}

			break;
		}
	case FT5X36_IOCTL_RESET:
		CTP_DBG("Try to reset TP!");
		//add by liyaohua
		{			
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
		msleep(10);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
		msleep(10);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
		}
    break;
	case FT5X36_IOCTL_AUTO_CAL:
		CTP_DBG("Try to calibrate TP!");
		msleep(300);

		//fts_ctpm_auto_clb();
		
		fts_5x36_hw_reset();
		break;
	case FT5X36_IOCTL_GET_TPID:
		{
			CTP_DBG("\nFT5X36_IOCTL_GET_TPID\n");
			rc = FT5x36_get_tp_id(ts, &flag);
			CTP_DBG("TPID=0x%X!", flag);
			if(rc)
			{
				CTP_DBG("Get tp ID error(%d)", rc);
				rc = -EIO;
			} 
			if(copy_to_user(argp,&flag,sizeof(int))!=0)
			{
				CTP_DBG("copy_to_user error");
				rc = -EFAULT;
			}
			break;
		}
	case FT5X36_IOCTL_LAST_BTYE:
		CTP_DBG("=========FT5X36_IOCTL_LAST_BTYE=0x%X!=========", (int)arg);
		//copy_from_user(&flag,argp,sizeof(int));
		is_5336_fwsize_30 = (int)arg;
		CTP_DBG("=========FT5X36_IOCTL_LAST_BTYE=0x%X! flag=0x%X!=========", 
		is_5336_fwsize_30,(int)arg);
		break;
	case FT5X36_IOCTL_READ_BootPara :
		{
			
	
			u8 i2c_write_buf[10] = {0};
			i2c_write_buf[0] = 0xcd;
			fts_i2c_Read(ts->client, i2c_write_buf, 1, reg_val, 1);
			CTP_DBG("=========FT5X36_IOCTL_READ_BootPara=0x%X!=========",reg_val[0]);
			if (reg_val[0]<= 4)
			{
				is_5336_new_bootloader = BL_VERSION_LZ4 ;
			}
			else if(reg_val[0] == 7)
			{
				is_5336_new_bootloader = BL_VERSION_Z7 ;
			}
			else if(reg_val[0] >= 0x0f)
			{
				is_5336_new_bootloader = BL_VERSION_GZF ;
			}
			flag = is_5336_new_bootloader;
			copy_to_user(argp,&flag, sizeof(int));
			//copy_to_user(argp,&is_5336_new_bootloader,sizeof(int));
			CTP_DBG("=========FT5X36_IOCTL_READ_BootPara=0x%X!flag=0x%X!=========",is_5336_new_bootloader,flag);
			break;
		}
	default:
		CTP_DBG("invalid command %d", _IOC_NR(cmd));
		rc = -EINVAL;
		break;
	}

	return rc;
}

static const struct file_operations fts_isp_fops = {
	.owner = THIS_MODULE,
	.read = fts_isp_read,
	.write = fts_isp_write,
	.open = fts_isp_open,
	.release = fts_isp_close,
	.unlocked_ioctl = fts_isp_ioctl,
};

static struct miscdevice fts_isp_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "fts_isp",
	.fops = &fts_isp_fops,
};

 /* called when loaded into kernel */
 int fts_5x36_isp_init( tinno_ts_data *ts ) 
 {
 	int ret;
	CTP_DBG("MediaTek FT5x36 touch panel isp init\n");
	
	wake_lock_init(&ts->wake_lock, WAKE_LOCK_SUSPEND, "fts_tp_isp");
	
	ret = misc_register(&fts_isp_device);
	if (ret) {
		misc_deregister(&fts_isp_device);
		printk(KERN_ERR "fts_isp_device device register failed (%d)\n", ret);
		goto exit_misc_device_register_failed;
	}
	
	g_pts = ts;
	return 0;
	
exit_misc_device_register_failed:
	misc_deregister(&fts_isp_device);
	fts_isp_device.minor = MISC_DYNAMIC_MINOR;
	wake_lock_destroy(&ts->wake_lock);
	return ret;
 }
 
 /* should never be called */
 void fts_5x36_isp_exit(void) 
{
	CTP_DBG("MediaTek FT5x36 touch panel isp exit\n");
	if ( g_pts ){
		misc_deregister(&fts_isp_device);
		fts_isp_device.minor = MISC_DYNAMIC_MINOR;
		wake_lock_destroy(&g_pts->wake_lock);
		g_pts = NULL;
	}
}
 
