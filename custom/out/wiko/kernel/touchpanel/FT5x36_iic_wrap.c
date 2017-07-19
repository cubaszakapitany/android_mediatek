 
#include "tpd.h"
#include <linux/interrupt.h>
#include <cust_eint.h>
#include <linux/i2c.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/delay.h>
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
#define    BL_VERSION_LZ4        0
#define    BL_VERSION_Z7         1
#define    BL_VERSION_GZF        2

#define    FTS_PACKET_LENGTH        128
#define    FTS_PACKET_NO_DMA_LENGTH        128
static unsigned char *tpDMABuf_va = NULL;
static u32 tpDMABuf_pa = 0;
u8 *I2CDMABuf_va = NULL;
volatile u32 I2CDMABuf_pa = NULL;
u8 auc_i2c_write_buf[10] = {0};


#define IC_FT5X06	0
#define IC_FT5606	1
#define IC_FT5316	2
#define IC_FT5X36	3

#define DEVICE_IC_TYPE	IC_FT5X36
extern u8 is_5336_fwsize_30;
extern u8 is_5336_new_bootloader;

//static tinno_ts_data *g_pts = NULL;
static int ft5x36_i2c_txdata(char *txdata, int length)
{
	int ret;

	struct i2c_msg msg[] = {
		{
			.addr	= g_pts->client->addr,
			.flags	= 0,
			.len	= length,
			.buf	= txdata,
			.timing = 0,
		},
	};

   	//msleep(1);
	ret = i2c_transfer(g_pts->client->adapter, msg, 1);
	if (ret < 0)
		pr_err("%s i2c write error: %d\n", __func__, ret);

	return ret;
}
int ft5x0x_i2c_txdata(char *txdata, int length)

{
	int ret;

	struct i2c_msg msg[] = {
		{
			.addr	= g_pts->client->addr, //ts->client
			.flags	= 0,
			.len	= length,
			.buf	= txdata,
		},
	};
   
   	//msleep(1);
	ret = i2c_transfer(g_pts->client->adapter, msg, 1);
	if (ret < 0)
		pr_err("%s i2c write error: %d\n", __func__, ret);

	return ret;
}
////////////////////////////////////////////////////////////////

/******************************************************************************
*****************
Name	:	 ft5x36_write_reg

Input	:	addr -- address
                     para -- parameter

Output	:	

function	:	write register of ft5x0x

*******************************************************************************
****************/
static int ft5x36_write_reg(u8 addr, u8 para)
{
    u8 buf[3];
    int ret = -1;

    buf[0] = addr;
    buf[1] = para;
    ret = ft5x36_i2c_txdata(buf, 2);
    if (ret < 0) {
        pr_err("write reg failed! %#x ret: %d", buf[0], ret);
        return -1;
    }
    
    return 0;
}

///////////////////////////////////////////////////////////////
FTS_BOOL i2c_read_interface(FTS_BYTE bt_ctpm_addr, FTS_BYTE* pbt_buf, 
FTS_DWRD dw_lenth)
{
    int ret = 0;
	struct i2c_msg msgs[2];

	memset(msgs,0,sizeof(msgs));
#if 0    
    ret=i2c_master_recv(this_client, pbt_buf, dw_lenth);
#else
	msgs[0].addr = g_pts->client->addr;
	msgs[0].flags = I2C_M_RD;
	msgs[0].len = dw_lenth;
	msgs[0].timing = 0;
	msgs[0].buf = pbt_buf;
	ret = i2c_transfer(g_pts->client->adapter, msgs, 1);
	if (ret < 0)
		printk("device addr = 0x%x, write addr = 0x%x error in %s func: %d\n", 
g_pts->client->addr, pbt_buf[0], __func__, ret);	
#endif

    if(ret<=0)
    {
        printk("[FTS]i2c_read_interface error\n");
        return FTS_FALSE;
    }
  
    return FTS_TRUE;
}

FTS_BOOL i2c_write_interface(FTS_BYTE bt_ctpm_addr, FTS_BYTE* pbt_buf, 
FTS_DWRD dw_lenth)
{
    int ret;
	struct i2c_msg msgs[2];

	memset(msgs,0,sizeof(msgs));
	    
#if 0  
    ret=i2c_master_send(this_client, pbt_buf, dw_lenth);
#else
	msgs[0].addr = g_pts->client->addr;
	msgs[0].flags = 0;
	msgs[0].len = dw_lenth;
	msgs[0].timing = 0;
	msgs[0].buf = pbt_buf;
	ret = i2c_transfer(g_pts->client->adapter, msgs, 1);
	if (ret < 0)
		printk("device addr = 0x%x, write addr = 0x%x error in %s func: %d\n", 
g_pts->client->addr, pbt_buf[0], __func__, ret);	
#endif

    if(ret<=0)
    {
        printk("[FTS]i2c_write_interface error line = %d, ret = %d\n", 
__LINE__, ret);
        return FTS_FALSE;
    }

    return FTS_TRUE;
}	

FTS_BOOL cmd_write(FTS_BYTE btcmd,FTS_BYTE btPara1,FTS_BYTE btPara2,FTS_BYTE btPara3,FTS_BYTE num)
{
    FTS_BYTE write_cmd[4] = {0};

    write_cmd[0] = btcmd;
    write_cmd[1] = btPara1;
    write_cmd[2] = btPara2;
    write_cmd[3] = btPara3;
    return i2c_write_interface(TPD_I2C_SLAVE_ADDR, write_cmd, num);
}
FTS_BOOL byte_write(FTS_BYTE* pbt_buf, FTS_DWRD dw_len)
{
    
    return i2c_write_interface(TPD_I2C_SLAVE_ADDR, pbt_buf, dw_len);
}
FTS_BOOL byte_read(FTS_BYTE* pbt_buf, FTS_BYTE bt_len)

{
    return i2c_read_interface(TPD_I2C_SLAVE_ADDR, pbt_buf, bt_len);
}
int fts_write_reg(u8 addr, u8 para)
{
	int rc;
	char buf[3];

	buf[0] = addr;
	buf[1] = para;
	mutex_lock(&g_pts->mutex);
	rc = i2c_master_send(g_pts->client, buf, 2);
	mutex_unlock(&g_pts->mutex);
	return rc;
}
int fts_ISP_write_reg(u8 addr, u8 para)
{
	int rc;
	char buf[3];

	buf[0] = addr;
	buf[1] = 0x00;
	mutex_lock(&g_pts->mutex);
	rc = i2c_master_send(g_pts->client, buf, 1);
	mutex_unlock(&g_pts->mutex);
	return rc;
}



int fts_read_reg(u8 addr, unsigned char *pdata)
{
	int rc;
	unsigned char buf[2];

	buf[0] = addr;               //register address

	mutex_lock(&g_pts->mutex);
	i2c_master_send(g_pts->client, &buf[0], 1);
	rc = i2c_master_recv(g_pts->client, &buf[0], 1);
	mutex_unlock(&g_pts->mutex);

	if (rc < 0)
		pr_err("msg %s i2c read error: %d\n", __func__, rc);

	*pdata = buf[0];
	return rc;
}
int fts_i2c_Read(struct i2c_client *client, char *writebuf,
		    int writelen, char *readbuf, int readlen)
{
	int ret,i;
	
#if 0//for normal I2c transfer
	if (writelen > 0) {
		struct i2c_msg msgs[] = {
			{
			 .addr = client->addr,
			 .flags = 0,
			 .len = writelen,
			 .buf = writebuf,
			 },
			{
			 .addr = client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 2);
		if (ret < 0)
			dev_err(&client->dev, "f%s: i2c read error.\n",
				__func__);
	} else {
		struct i2c_msg msgs[] = {
			{
			 .addr = client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 1);
		if (ret < 0)
			dev_err(&client->dev, "%s:i2c read error.\n", __func__);
	}
#else// for DMA I2c transfer
	if(writelen!=0)
	{
		//DMA Write
		if(0)//if(writelen < 8  )
		{
			
			//MSE_ERR("Sensor non-dma write timing is %x!\r\n", this_client->timing);
			ret= i2c_master_send(client, writebuf, writelen);
		}
		else
		{
			for(i = 0 ; i < writelen; i++)
			{
				I2CDMABuf_va[i] = writebuf[i];
			}

			client->addr = client->addr & I2C_MASK_FLAG | I2C_DMA_FLAG;
		
			if((ret=i2c_master_send(client, (unsigned char *)I2CDMABuf_pa, writelen))!=
writelen)
				dev_err(&client->dev, "###%s i2c write len=%x,buffaddr=%x\n", __func__,ret
,I2CDMABuf_pa);
			//MSE_ERR("Sensor dma timing is %x!\r\n", this_client->timing);
			//return ret;
			client->addr = client->addr & I2C_MASK_FLAG &(~ I2C_DMA_FLAG);

		}
	}
	//DMA Read 
	if(readlen!=0)
	{
		if(0)//if (readlen <8) {
		{
			ret = i2c_master_recv(client, (unsigned char *)readbuf, readlen);
		}
		else
		{

			client->addr = client->addr & I2C_MASK_FLAG | I2C_DMA_FLAG;
			ret = i2c_master_recv(client, (unsigned char *)I2CDMABuf_pa, readlen);

			for(i = 0; i < readlen; i++)
	        {
	            readbuf[i] = I2CDMABuf_va[i];
	        }
		client->addr = client->addr & I2C_MASK_FLAG &(~ I2C_DMA_FLAG);

		}
	}
	#endif
	return ret;
}
/*write data by i2c*/

int fts_i2c_Write(struct i2c_client *client, char *writebuf, int writelen)
{
	int ret;
	int i = 0;
	printk("fts_i2c_Write  %d  %x  %x %x %x",writelen,writebuf,I2CDMABuf_va,I2CDMABuf_pa,client);
	
   client->addr = client->addr & I2C_MASK_FLAG;
  // client->ext_flag |= I2C_DIRECTION_FLAG; 
  // client->timing = 100;
    #if 0
	struct i2c_msg msg[] = {
		{
		 .addr = client->addr,
		 .flags = 0,
		 .len = writelen,
		 .buf = writebuf,
		 },
	};

	ret = i2c_transfer(client->adapter, msg, 1);
	if (ret < 0)
		dev_err(&client->dev, "%s i2c write error.\n", __func__);
	#else
	
	if(0)//if(writelen < 8)
	{
		
		//MSE_ERR("Sensor non-dma write timing is %x!\r\n", this_client->timing);
		ret = i2c_master_send(client, writebuf, writelen);
	}
	else
	{
		for(i = 0 ; i < writelen; i++)
		{
			I2CDMABuf_va[i] = writebuf[i];
		}

		client->addr = client->addr & I2C_MASK_FLAG | I2C_DMA_FLAG;

		if((ret=i2c_master_send(client, (unsigned char *)I2CDMABuf_pa, writelen))!=
writelen)
			dev_err(&client->dev, "###%s i2c write len=%x,buffaddr=%x\n", __func__,ret,
I2CDMABuf_pa);
		//MSE_ERR("Sensor dma timing is %x!\r\n", this_client->timing);
		client->addr = client->addr & I2C_MASK_FLAG &(~ I2C_DMA_FLAG);

	} 
	#endif
	return ret;

}
/*
int fts_write_reg(u8 regaddr, u8 regvalue)

{
	unsigned char buf[3] = {0};
	buf[0] = regaddr;
	buf[1] = regvalue;

	return fts_i2c_Write(g_pts->client, buf, sizeof(buf));
}


int fts_read_reg(u8 regaddr, u8 *regvalue)

{
	return fts_i2c_Read(g_pts->client, &regaddr, 1, regvalue, 1);
}*/

//////////////////////////////////////////////////////////////////////////////
u8 fts_cmd_write(u8 btcmd,u8 btPara1,u8 btPara2,u8 btPara3,u8 num)
{
	int rc;
	u8 write_cmd[4] = {0};
	write_cmd[0] = btcmd;
	write_cmd[1] = btPara1;
	write_cmd[2] = btPara2;
	write_cmd[3] = btPara3;
	mutex_lock(&g_pts->mutex);
	g_pts->client->addr = g_pts->client->addr & I2C_MASK_FLAG;
	rc = i2c_master_send(g_pts->client, write_cmd, num);
	mutex_unlock(&g_pts->mutex);
	return rc;
}


#if defined(MT6577)
static ssize_t fts_bin_write_block(uint8_t* pbt_buf, int dw_lenth)
{
	unsigned char reg_val[3] = {0};
	int i = 0, j;
	char buf[256] = {0};
	int  packet_number;
	int  temp;
	int  lenght;
	unsigned char  packet_buf[FTS_PACKET_LENGTH + 6];
	unsigned char bt_ecc;
	int ret;
	bt_ecc = 0;
	CTP_DBG("start upgrade. \n");
	dw_lenth = dw_lenth - 8;
	CTP_DBG("####Packet length = 0x %x\n", dw_lenth);
	packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
	buf[0] = 0xbf;
	buf[1] = 0x00;
	for (j=0;j<packet_number;j++)
	{
		temp = j * FTS_PACKET_NO_DMA_LENGTH;
		buf[2] = (u8)(temp>>8);
		buf[3] = (u8)temp;
		lenght = FTS_PACKET_NO_DMA_LENGTH;
		buf[4] = (u8)(lenght>>8);
		buf[5] = (u8)lenght;

		for (i=0;i<FTS_PACKET_NO_DMA_LENGTH;i++)
		{
			buf[6+i] = pbt_buf[j*FTS_PACKET_NO_DMA_LENGTH + i]; 
			bt_ecc ^= buf[6+i];
		}

		ret = i2c_master_send(g_pts->client, buf, FTS_PACKET_NO_DMA_LENGTH + 6);
	        if (ret < 0) {
	            CTP_DBG("[Error]TP write data error(%d)!! packet_number=%d\n", ret, j);
	            return -1;
	        }
			
		mdelay(50);
		if ((j * FTS_PACKET_NO_DMA_LENGTH % 1024) == 0)
		{
			CTP_DBG("upgrade the 0x%x th byte.\n", ((unsigned int)j) * FTS_PACKET_NO_DMA_LENGTH);
		}
	}

	if ((dw_lenth) % FTS_PACKET_NO_DMA_LENGTH > 0)
	{
		temp = packet_number * FTS_PACKET_NO_DMA_LENGTH;
		buf[2] = (u8)(temp>>8);
		buf[3] = (u8)temp;

		temp = (dw_lenth) % FTS_PACKET_NO_DMA_LENGTH;
		buf[4] = (u8)(temp>>8);
		buf[5] = (u8)temp;

		for (i=0;i<temp;i++)
		{
			buf[6+i] = pbt_buf[ packet_number*FTS_PACKET_NO_DMA_LENGTH + i]; 
			bt_ecc ^= buf[6+i];
		}

		ret = i2c_master_send(g_pts->client, buf, temp+6);
	       if (ret < 0) {
	            CTP_DBG("[Error]TP write data error(%d)!! temp=%d\n", ret, temp);
	            return -1;
	        }
			
		g_pts->client->addr = g_pts->client->addr & I2C_MASK_FLAG;

		mdelay(30);
	}

	//send the last six byte
	for (i = 0; i<6; i++)
	{
		packet_buf[0] = 0xbf;
		packet_buf[1] = 0x00;
		temp = 0x6ffa + i;
		packet_buf[2] = (u8)(temp>>8);
		packet_buf[3] = (u8)temp;
		temp =1;
		packet_buf[4] = (u8)(temp>>8);
		packet_buf[5] = (u8)temp;
		packet_buf[6] = pbt_buf[ dw_lenth + i]; 
		bt_ecc ^= packet_buf[6];

		i2c_master_send(g_pts->client, packet_buf, 7);    

		mdelay(40);
	}

	/*********read out checksum***********************/
	/*send the operation head*/
	fts_cmd_write(0xcc,0x00,0x00,0x00,1);
	g_pts->client->addr = g_pts->client->addr & I2C_MASK_FLAG;
	i2c_master_recv(g_pts->client, &reg_val, 1);
	CTP_DBG("ecc read 0x%x, new firmware 0x%x. \n", reg_val[0], bt_ecc);
	if(reg_val[0] != bt_ecc)
	{
		CTP_DBG("check sum error!!\n");
		return 5;
	}

	return 0;
}
#elif defined(MT6573) || defined(MT6575) || defined(MT6589)|| defined(MT6572) 
static ssize_t fts_dma_write_m_byte(unsigned char*returnData_va, u32 returnData_pa, int  len)
{
    int     rc=0;
    if (len > 0){
		mutex_lock(&g_pts->mutex);
		g_pts->client->addr = (g_pts->client->addr & I2C_MASK_FLAG) | I2C_DMA_FLAG;
        rc = i2c_master_send(g_pts->client, returnData_pa, len);
		g_pts->client->addr = g_pts->client->addr & I2C_MASK_FLAG;
		mutex_unlock(&g_pts->mutex);
        if (rc < 0) {
            printk(KERN_ERR"xxxxfocal write data error!! xxxx\n");
            return 0;
        }
    }
    return 1;
}

static ssize_t fts_dma_read_m_byte(unsigned char cmd, unsigned char*returnData_va, u32 returnData_pa,unsigned char len)
{
    int rc=0;
	unsigned char start_addr = cmd;

	mutex_lock(&g_pts->mutex);
	rc = i2c_master_send(g_pts->client, &start_addr, 1);
    if (rc < 0) {
        printk(KERN_ERR"xxxx focal sends command error!! xxxx\n");
		goto err;
    }

    if (len > 0){
		g_pts->client->addr = (g_pts->client->addr & I2C_MASK_FLAG) | I2C_DMA_FLAG;
        rc = i2c_master_recv(g_pts->client, returnData_pa, len);
		g_pts->client->addr = g_pts->client->addr & I2C_MASK_FLAG;
        if (rc < 0) {
            printk(KERN_ERR"xxxx focal reads data error!! xxxx\n");
			goto err;
        }
    }
	
	
	mutex_unlock(&g_pts->mutex);
    return 1;
err:
	mutex_unlock(&g_pts->mutex);
	return 0;
}

static ssize_t fts_bin_write_block(uint8_t* pbt_buf, int dw_lenth)
{
	unsigned char reg_val[3] = {0};
	int i = 0, j;

	int  packet_number;
	int  temp;
	int  lenght;
	unsigned char  packet_buf[FTS_PACKET_LENGTH + 6];
	unsigned char bt_ecc;
	CTP_DBG("start upgrade. %x\n", is_5336_new_bootloader);
   // is_5336_new_bootloader = BL_VERSION_GZF;
	bt_ecc = 0;
	
if(is_5336_new_bootloader == BL_VERSION_LZ4 || is_5336_new_bootloader == 
BL_VERSION_Z7 )
	{

		dw_lenth = dw_lenth - 8;

	}
	else if(is_5336_new_bootloader == BL_VERSION_GZF) 
	{

		dw_lenth = dw_lenth  - 14;

	}
	CTP_DBG("####Packet length = 0x %x\n", dw_lenth);
	packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
	tpDMABuf_va[0] = 0xbf;
	tpDMABuf_va[1] = 0x00;
	for (j=0;j<packet_number;j++){
		temp = j * FTS_PACKET_LENGTH;
		tpDMABuf_va[2] = (u8)(temp>>8);
		tpDMABuf_va[3] = (u8)temp;
		lenght = FTS_PACKET_LENGTH;
		tpDMABuf_va[4] = (u8)(lenght>>8);
		tpDMABuf_va[5] = (u8)lenght;

		for (i=0;i<FTS_PACKET_LENGTH;i++){
			tpDMABuf_va[6+i] = pbt_buf[j*FTS_PACKET_LENGTH + i]; 
			bt_ecc ^= tpDMABuf_va[6+i];
		}

		fts_i2c_Write(g_pts->client,tpDMABuf_va,  FTS_PACKET_LENGTH + 6);
		mdelay(50);
		if ((j * FTS_PACKET_LENGTH % 1024) == 0){
			CTP_DBG("[FT5x36] upgrade the 0x%x th byte.\n", ((unsigned int)j) * FTS_PACKET_LENGTH);
		}
	}

	if ((dw_lenth) % FTS_PACKET_LENGTH > 0){
		temp = packet_number * FTS_PACKET_LENGTH;
		tpDMABuf_va[2] = (u8)(temp>>8);
		tpDMABuf_va[3] = (u8)temp;

		temp = (dw_lenth) % FTS_PACKET_LENGTH;
		tpDMABuf_va[4] = (u8)(temp>>8);
		tpDMABuf_va[5] = (u8)temp;

		for (i=0;i<temp;i++){
			tpDMABuf_va[6+i] = pbt_buf[ packet_number*FTS_PACKET_LENGTH + i]; 
			bt_ecc ^= tpDMABuf_va[6+i];
		}

		fts_i2c_Write(g_pts->client,tpDMABuf_va, temp+6);    
		//g_pts->client->addr = g_pts->client->addr & I2C_MASK_FLAG;

		mdelay(30);
	}
if(is_5336_new_bootloader == BL_VERSION_LZ4 || is_5336_new_bootloader ==BL_VERSION_Z7 )
	{
	

	//send the last six byte
	for (i = 0; i<6; i++){
		if (is_5336_new_bootloader	== BL_VERSION_Z7 
			&& DEVICE_IC_TYPE==IC_FT5X36) 
		{	
			temp  = 0x7bfa + i;	
		}	
		else if(is_5336_new_bootloader == BL_VERSION_LZ4)	
		{	
			temp = 0x6ffa + i;	
		}
		packet_buf[0] = 0xbf;
		packet_buf[1] = 0x00;
		//temp = 0x6ffa + i;
		packet_buf[2] = (u8)(temp>>8);
		packet_buf[3] = (u8)temp;
		temp =1;
		packet_buf[4] = (u8)(temp>>8);
		packet_buf[5] = (u8)temp;
		packet_buf[6] = pbt_buf[ dw_lenth + i]; 
		bt_ecc ^= packet_buf[6];

		fts_i2c_Write(g_pts->client, packet_buf, 7);    

		mdelay(40);
	}

	/*********read out checksum***********************/
	/*send the operation head*/
/*	fts_cmd_write(0xcc,0x00,0x00,0x00,1);
	g_pts->client->addr = g_pts->client->addr & I2C_MASK_FLAG;
	i2c_master_recv(g_pts->client, &reg_val, 1);
	CTP_DBG("ecc read 0x%x, new firmware 0x%x. \n", reg_val[0], bt_ecc);
	if(reg_val[0] != bt_ecc){
		CTP_DBG("check sum error!!\n");
		return 5;
	}
*/

		

}
else if(is_5336_new_bootloader == BL_VERSION_GZF)
{
	for (i = 0; i<12; i++)

	{
		if (is_5336_fwsize_30 && DEVICE_IC_TYPE==IC_FT5X36) 
		{
			temp = 0x7ff4 + i;
		}
		else if (DEVICE_IC_TYPE==IC_FT5X36)
		{
			temp = 0x7bf4 + i;
		}
		packet_buf[0] = 0xbf;
		packet_buf[1] = 0x00;
		packet_buf[2] = (u8)(temp>>8);
		packet_buf[3] = (u8) temp;
		temp =1;
		packet_buf[4] = (u8)(temp>>8);
		packet_buf[5] = (u8)temp;
		packet_buf[6] = pbt_buf[dw_lenth + i]; 
		bt_ecc ^= packet_buf[6];
		fts_i2c_Write(g_pts->client, packet_buf, 7);    
		msleep(10);
	}

}

	printk(KERN_WARNING "is_5336_fwsize_30 = %d , is_5336_new_bootloader = %d \n",is_5336_fwsize_30,is_5336_new_bootloader);
	u8 auc_i2c_write_buf[10] = {0};
	CTP_DBG("Step 6: read out checksum\n");
	auc_i2c_write_buf[0] = 0xcc;
	fts_i2c_Read(g_pts->client, auc_i2c_write_buf, 1, reg_val, 1);
	if (reg_val[0] != bt_ecc) {
		CTP_DBG("[FTS]--ecc error! FW=%02x bt_ecc=%02x\n",
					reg_val[0],
					bt_ecc);
		return -EIO;
	}


	return 0;
}
#endif

int fts_i2c_write_block( u8 *txbuf, int len )
{
	if ( 5 == fts_bin_write_block(txbuf, len) ){
	       CTP_DBG("[Error]!! \n");
		return 5;
	}
	return len;
}

#if defined(MT6577) || defined(MT6589) || defined(MT6572)
static int fts_read_block( tinno_ts_data *ts, u8 addr, u8 *rxbuf, u8 len )
{
	u8 retry;
	u16 left = len;
	u16 offset = 0;

	if ( rxbuf == NULL )
		return -1;

//	CTP_DBG("device 0x%02X address %04X len %d\n", ts->client->addr, addr, len );

	mutex_lock(&g_pts->mutex);
	while ( left > 0 ){
		if ( left > MAX_I2C_TRANSFER_SIZE ){
			rxbuf[offset] = ( addr+offset ) & 0xFF;
			ts->client->addr = ts->client->addr & I2C_MASK_FLAG | I2C_WR_FLAG | I2C_RS_FLAG;
			retry = 0;
			while ( i2c_master_send(ts->client, &rxbuf[offset], (MAX_I2C_TRANSFER_SIZE << 8 | 1)) < 0 ){
				retry++;

				if ( retry == 5 ){
					ts->client->addr = ts->client->addr & I2C_MASK_FLAG;
					mutex_unlock(&g_pts->mutex);
					CTP_DBG("I2C read 0x%X length=%d failed\n", addr + offset, MAX_I2C_TRANSFER_SIZE);
					return -1;
				}
			}
			left -= MAX_I2C_TRANSFER_SIZE;
			offset += MAX_I2C_TRANSFER_SIZE;
		}
		else{
			rxbuf[offset] = ( addr+offset ) & 0xFF;
			ts->client->addr = ts->client->addr & I2C_MASK_FLAG | I2C_WR_FLAG | I2C_RS_FLAG;

			retry = 0;
			while ( i2c_master_send(ts->client, &rxbuf[offset], (left << 8 | 1)) < 0 ){
				retry++;

				if ( retry == 5 ){
					ts->client->addr = ts->client->addr & I2C_MASK_FLAG;
					mutex_unlock(&g_pts->mutex);
					CTP_DBG("I2C read 0x%X length=%d failed\n", addr + offset, left);
					return -1;
				}
			}
			offset += left;
			left = 0;
		}
	}
	ts->client->addr = ts->client->addr & I2C_MASK_FLAG;

	mutex_unlock(&g_pts->mutex);
	return 0;
}
#else //!MT6577
static int fts_read_block( tinno_ts_data *ts, u8 start_addr, u8 *rxbuf, u8 len )
{
	u8 reg_addr;
	u8 retry;
	u16 left = len;
	u16 offset = 0;

	struct i2c_msg msg[2] =
	{
		{
			.addr = ts->client->addr,
			.flags = 0,
			.buf = &reg_addr,
			.len = 1,
			.timing = I2C_MASTER_CLOCK
		},
		{
			.addr = ts->client->addr,
			.flags = I2C_M_RD,
			.timing = I2C_MASTER_CLOCK
		},
	};

	if ( rxbuf == NULL )
		return -1;

	mutex_lock(&g_pts->mutex);
	while ( left > 0 ){
		reg_addr = ( start_addr+offset ) & 0xFF;

		msg[1].buf = &rxbuf[offset];

		if ( left > MAX_TRANSACTION_LENGTH ){
			msg[1].len = MAX_TRANSACTION_LENGTH;
			left -= MAX_TRANSACTION_LENGTH;
			offset += MAX_TRANSACTION_LENGTH;
		}
		else{
			msg[1].len = left;
			left = 0;
		}

		retry = 0;

		while ( i2c_transfer( ts->client->adapter, &msg[0], 2 ) != 2 ){
			retry++;
			if ( retry == 20 ){
				mutex_unlock(&g_pts->mutex);
				CTP_DBG("I2C read 0x%X length=%d failed\n", start_addr + offset, len);
				return -1;
			}
		}
	}

	mutex_unlock(&g_pts->mutex);
	return 0;
}
#endif

 int tpd_read_touchinfo(tinno_ts_data *ts)
 {
	int ret = 0;
	memset((void *)ts->buffer, FTS_INVALID_DATA, FTS_PROTOCOL_LEN);
#if defined(MT6577) || defined(MT6589) || defined(MT6572)
	ret = fts_read_block(ts,ts->start_reg, ts->buffer, FTS_PROTOCOL_LEN);
	if ( ret ) {
		CTP_DBG("i2c_transfer failed, (%d)", ret);
		return -EAGAIN; 
	} 
#else
	ret = fts_dma_read_m_byte(ts->start_reg, tpDMABuf_va, tpDMABuf_pa, FTS_PROTOCOL_LEN);
	if ( !ret ) {
		CTP_DBG("i2c_transfer failed, (%d)", ret);
		return -EAGAIN; 
	} 
	memcpy((void *)ts->buffer, tpDMABuf_va, FTS_PROTOCOL_LEN);
#endif
	return 0;
}

int fts_iic_init( tinno_ts_data *ts )
{
	CTP_DBG();
#if defined(MT6577)
#elif defined(MT6589)||  defined(MT6572)
	tpDMABuf_va = (unsigned char *)dma_alloc_coherent(NULL, 4096, &tpDMABuf_pa, GFP_KERNEL);
	if(!tpDMABuf_va){
		printk(KERN_ERR"xxxx Allocate DMA I2C Buffer failed!xxxx\n");
		return -1;
	}
#endif
	g_pts = ts;
	return 0;
}

