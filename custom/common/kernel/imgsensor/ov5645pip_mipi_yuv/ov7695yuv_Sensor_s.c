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

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <asm/io.h>
#include <asm/system.h>
#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
#include "kd_camera_feature.h"
#include "ov7695yuv_Sensor_s.h"

#define OV7695PIPYUV_DEBUG
#ifdef OV7695PIPYUV_DEBUG
#define OV7695PIPYUVSENSORDB(fmt, arg...)    printk("[OV7695PIP]%s: " fmt, __FUNCTION__ ,##arg)
#else
#define OV7695PIPYUVSENSORDB(x,...)
#endif

#define OV7695_15FPS

#ifdef SIMULATE_PIP
static void spin_lock(int *lock) {}
static void spin_unlock(int *lock) {}
int ov7695pipyuv_drv_lock=1;
#else
static DEFINE_SPINLOCK(ov7695pipyuv_drv_lock);
#endif
static MSDK_SCENARIO_ID_ENUM CurrentScenarioId = MSDK_SCENARIO_ID_CAMERA_PREVIEW;
extern int iReadReg(u16 a_u2Addr , u8 * a_puBuff , u16 i2cId);
extern int iWriteReg(u16 a_u2Addr , u32 a_u4Data , u32 a_u4Bytes , u16 i2cId);
#define OV7695PIPYUV_write_cmos_sensor(addr, para) iWriteReg((u16) addr , (u32) para ,1,OV7695PIPYUV_WRITE_ID)
kal_uint16 OV7695PIPYUV_read_cmos_sensor(kal_uint32 addr)
{
    kal_uint16 get_byte=0;
    iReadReg((u16) addr ,(u8*)&get_byte,OV7695PIPYUV_WRITE_ID);
    return get_byte;
}

struct
{
    kal_uint8   Banding;
    kal_bool      NightMode;
    kal_bool      VideoMode;
    kal_uint16  Fps;
    kal_uint16  ShutterStep;
    kal_uint8   IsPVmode;
    kal_uint32  PreviewDummyPixels;
    kal_uint32  PreviewDummyLines;
    kal_uint32  CaptureDummyPixels;
    kal_uint32  CaptureDummyLines;
    kal_uint32  PreviewPclk;
    kal_uint32  CapturePclk;
    kal_uint32  PreviewShutter;
    kal_uint32  PreviewExtraShutter;
    kal_uint32  SensorGain;
    OV7695PIPYUV_SENSOR_MODE SensorMode;
    kal_uint8   awb[6];
    kal_bool      bSeneorExist;
} OV7695PIPYUVSensor;

/* Global Valuable */
static kal_uint32 zoom_factor = 0;
//static kal_int8 OV7695PIPYUV_DELAY_AFTER_PREVIEW = -1;
//static kal_uint8 OV7695PIPYUV_Banding_setting = AE_FLICKER_MODE_50HZ;
static kal_bool OV7695PIPYUV_AWB_ENABLE = KAL_TRUE;
static kal_bool OV7695PIPYUV_AE_ENABLE = KAL_TRUE;
MSDK_SENSOR_CONFIG_STRUCT OV7695PIPYUVSensorConfigData;

void OV7695PIPYUVSetSeneorExist(kal_bool bSeneorExist)
{
	spin_lock(&ov7695pipyuv_drv_lock);
	OV7695PIPYUVSensor.bSeneorExist = bSeneorExist;
	spin_unlock(&ov7695pipyuv_drv_lock);
}

void OV7695PIPYUVInitialVariable(void)
{
	spin_lock(&ov7695pipyuv_drv_lock);
	OV7695PIPYUVSensor.VideoMode = KAL_FALSE;
	OV7695PIPYUVSensor.NightMode = KAL_FALSE;
	OV7695PIPYUVSensor.Fps = 100;
	OV7695PIPYUVSensor.ShutterStep= 0xde;
	OV7695PIPYUVSensor.CaptureDummyPixels = 0;
	OV7695PIPYUVSensor.CaptureDummyLines = 0;
	OV7695PIPYUVSensor.PreviewDummyPixels = 0;
	OV7695PIPYUVSensor.PreviewDummyLines = 0;
	OV7695PIPYUVSensor.SensorMode= OV7695PIPYUV_SENSOR_MODE_INIT;
	OV7695PIPYUVSensor.bSeneorExist = KAL_TRUE;
	spin_unlock(&ov7695pipyuv_drv_lock);
}
/*************************************************************************
* FUNCTION
*    OV7695PIPYUV_set_dummy
*
* DESCRIPTION
*    This function set the dummy pixels(Horizontal Blanking) & dummy lines(Vertical Blanking), it can be
*    used to adjust the frame rate or gain more time for back-end process.
*
*    IMPORTANT NOTICE: the base shutter need re-calculate for some sensor, or else flicker may occur.
*
* PARAMETERS
*    1. kal_uint32 : Dummy Pixels (Horizontal Blanking)
*    2. kal_uint32 : Dummy Lines (Vertical Blanking)
*
* RETURNS
*    None
*
*************************************************************************/

void OV7695PIPYUVSetDummy(kal_uint32 dummy_pixels, kal_uint32 dummy_lines, kal_uint8 IsPVmode)
{
    if (/*OV7695PIPYUVSensor.*/ IsPVmode)
    {
        dummy_pixels = dummy_pixels+OV7695PIPYUV_PV_PERIOD_PIXEL_NUMS;
        OV7695PIPYUV_write_cmos_sensor(0x343,( dummy_pixels&0xFF));
        OV7695PIPYUV_write_cmos_sensor(0x342,(( dummy_pixels&0xFF00)>>8));

        dummy_lines= dummy_lines+OV7695PIPYUV_PV_PERIOD_LINE_NUMS;
        OV7695PIPYUV_write_cmos_sensor(0x341,(dummy_lines&0xFF));
        OV7695PIPYUV_write_cmos_sensor(0x340,((dummy_lines&0xFF00)>>8));
    }
    else
    {
        dummy_pixels = dummy_pixels+OV7695PIPYUV_FULL_PERIOD_PIXEL_NUMS;
        OV7695PIPYUV_write_cmos_sensor(0x343,( dummy_pixels&0xFF));
        OV7695PIPYUV_write_cmos_sensor(0x342,(( dummy_pixels&0xFF00)>>8));

        dummy_lines= dummy_lines+OV7695PIPYUV_FULL_PERIOD_LINE_NUMS;
        OV7695PIPYUV_write_cmos_sensor(0x341,(dummy_lines&0xFF));
        OV7695PIPYUV_write_cmos_sensor(0x340,((dummy_lines&0xFF00)>>8));
    }
}    /* OV7695PIPYUV_set_dummy */

/*************************************************************************
* FUNCTION
*    OV7695PIPYUVWriteShutter
*
* DESCRIPTION
*    This function used to write the shutter.
*
* PARAMETERS
*    1. kal_uint32 : The shutter want to apply to sensor.
*
* RETURNS
*    None
*
*************************************************************************/
void OV7695PIPYUVWriteShutter(kal_uint32 shutter, kal_uint8 IsPVmode)
{
    kal_uint32 extra_exposure_lines = 0;
    if (shutter < 1)
    {
        shutter = 1;
    }
#if 0
    if (IsPVmode)
    {
        if (shutter <= OV7695PIPYUV_PV_EXPOSURE_LIMITATION)
        {
            extra_exposure_lines = 0;
        }
        else
        {
            extra_exposure_lines=shutter - OV7695PIPYUV_PV_EXPOSURE_LIMITATION;
        }
    }
    else
    {
        if (shutter <= OV7695PIPYUV_FULL_EXPOSURE_LIMITATION)
        {
            extra_exposure_lines = 0;
        }
        else
        {
            extra_exposure_lines = shutter - OV7695PIPYUV_FULL_EXPOSURE_LIMITATION;
        }
    }
#endif
    shutter*=16;
    OV7695PIPYUV_write_cmos_sensor(0x3502, (shutter & 0x00FF));           //AEC[7:0]
    OV7695PIPYUV_write_cmos_sensor(0x3501, ((shutter & 0x0FF00) >>8));  //AEC[15:8]
    OV7695PIPYUV_write_cmos_sensor(0x3500, ((shutter & 0xFF0000) >> 16));
}    /* OV7695PIPYUV_write_shutter */

/*************************************************************************
* FUNCTION
*    OV7695PIPYUVWriteWensorGain
*
* DESCRIPTION
*    This function used to write the sensor gain.
*
* PARAMETERS
*    1. kal_uint32 : The sensor gain want to apply to sensor.
*
* RETURNS
*    None
*
*************************************************************************/
void OV7695PIPYUVWriteSensorGain(kal_uint32 gain)
{
    kal_uint16 temp_reg = 0;

    if(gain > 1024)  ASSERT(0);
    temp_reg = 0;
    temp_reg=gain&0x0FF;
    OV7695PIPYUV_write_cmos_sensor(0x350B,temp_reg);
}  /* OV7695PIPYUV_write_sensor_gain */

/*************************************************************************
* FUNCTION
*    OV7695PIPYUVReadShutter
*
* DESCRIPTION
*    This function read current shutter for calculate the exposure.
*
* PARAMETERS
*    None
*
* RETURNS
*    kal_uint16 : The current shutter value.
*
*************************************************************************/
kal_uint32 OV7695PIPYUVReadShutter(void)
{
    kal_uint16 temp_reg1, temp_reg2 ,temp_reg3;

    temp_reg1 = OV7695PIPYUV_read_cmos_sensor(0x3500);    // AEC[b19~b16]
    temp_reg2 = OV7695PIPYUV_read_cmos_sensor(0x3501);    // AEC[b15~b8]
    temp_reg3 = OV7695PIPYUV_read_cmos_sensor(0x3502);    // AEC[b7~b0]
    spin_lock(&ov7695pipyuv_drv_lock);
    OV7695PIPYUVSensor.PreviewShutter  = (temp_reg1 <<12)| (temp_reg2<<4)|(temp_reg3>>4);
    spin_unlock(&ov7695pipyuv_drv_lock);
    return OV7695PIPYUVSensor.PreviewShutter;
}    /* OV7695PIPYUV_read_shutter */

/*************************************************************************
* FUNCTION
*    OV7695PIPYUVReadSensorGain
*
* DESCRIPTION
*    This function read current sensor gain for calculate the exposure.
*
* PARAMETERS
*    None
*
* RETURNS
*    kal_uint16 : The current sensor gain value.
*
*************************************************************************/
kal_uint32 OV7695PIPYUVReadSensorGain(void)
{
    kal_uint32 sensor_gain = 0;
    sensor_gain=(OV7695PIPYUV_read_cmos_sensor(0x350B)&0xFF);
    return sensor_gain;
}  /* OV7695PIPYUVReadSensorGain */

/*************************************************************************
* FUNCTION
*    OV7695PIPYUV_set_AE_mode
*
* DESCRIPTION
*    This function read current sensor gain for calculate the exposure.
*
* PARAMETERS
*    None
*
* RETURNS
*    kal_uint16 : The current sensor gain value.
*
*************************************************************************/
void OV7695PIPYUV_set_AE_mode(kal_bool AE_enable)
{
    kal_uint8 AeTemp;
    AeTemp = OV7695PIPYUV_read_cmos_sensor(0x3503);
    if (AE_enable == KAL_TRUE)
    {
        OV7695PIPYUV_write_cmos_sensor(0x3503,(AeTemp&(~0x03)));
    }
    else
    {
        OV7695PIPYUV_write_cmos_sensor(0x3503,(AeTemp|0x03));
    }
}
/*************************************************************************
* FUNCTION
*    OV7695PIPYUV_set_AWB_mode
*
* DESCRIPTION
*    This function read current sensor gain for calculate the exposure.
*
* PARAMETERS
*    None
*
* RETURNS
*    kal_uint16 : The current sensor gain value.
*
*************************************************************************/
void OV7695PIPYUV_set_AWB_mode(kal_bool AWB_enable)
{
    kal_uint8 AwbTemp;
    AwbTemp = OV7695PIPYUV_read_cmos_sensor(0x5000);
    if (AWB_enable == KAL_TRUE)
    {
        OV7695PIPYUV_write_cmos_sensor(0x5000,AwbTemp|0x20);
        OV7695PIPYUV_write_cmos_sensor(0x5200,0x00);
    }
    else
    {
        OV7695PIPYUV_write_cmos_sensor(0x5000,AwbTemp&0xDF);
        OV7695PIPYUV_write_cmos_sensor(0x5200,0x20);
    }
}

/*************************************************************************
* FUNCTION
*    OV7695PIPYUV_night_mode
*
* DESCRIPTION
*    This function night mode of OV7695PIPYUV.
*
* PARAMETERS
*    none
*
* RETURNS
*    None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void OV7695PIPYUV_night_mode(kal_bool enable)
{
    if (enable)
    {
        OV7695PIPYUV_write_cmos_sensor(0x3a00, 0x78);
        OV7695PIPYUV_write_cmos_sensor(0x3a02, 0x02);
        OV7695PIPYUV_write_cmos_sensor(0x3a03, 0x18);
        OV7695PIPYUV_write_cmos_sensor(0x3a14, 0x02);
        OV7695PIPYUV_write_cmos_sensor(0x3a15, 0x18);
    }
    else
    {
        OV7695PIPYUV_write_cmos_sensor(0x3a00, 0x78);
        OV7695PIPYUV_write_cmos_sensor(0x3a02, 0x02);
        OV7695PIPYUV_write_cmos_sensor(0x3a03, 0x18);
        OV7695PIPYUV_write_cmos_sensor(0x3a14, 0x02);
        OV7695PIPYUV_write_cmos_sensor(0x3a15, 0x18);
    }
}    /* OV7695PIPYUV_night_mode */
/*************************************************************************
* FUNCTION
*    OV7695PIPYUV_GetSensorID
*
* DESCRIPTION
*    This function get the sensor ID
*
* PARAMETERS
*    None
*
* RETURNS
*    None
*
* GLOBALS AFFECTED
*
*************************************************************************/
kal_uint32 OV7695PIPYUV_GetSensorID(kal_uint32 *sensorID)
{
    volatile signed char i;
    kal_uint32 sensor_id=0;

    OV7695PIPYUV_write_cmos_sensor(0x0103,0x01);// Reset sensor
    mDELAY(10);

    for(i=0;i<3;i++)
    {
        sensor_id = (OV7695PIPYUV_read_cmos_sensor(0x300A) << 8) | OV7695PIPYUV_read_cmos_sensor(0x300B);
        OV7695PIPYUVSENSORDB("%s OV7695PIPYUV READ ID: %x\n", __func__, sensor_id);
        if(sensor_id != OV7695PIPYUV_SENSOR_ID)
        {
            *sensorID =0xffffffff;
        }
        else
        {
            *sensorID =OV7695PIP_SENSOR_ID;
            break;
        }
    }
    return ERROR_NONE;
}

#ifdef DEBUG_SENSOR

#define OV7695_OP_CODE_INI		0x00		/* Initial value. */
#define OV7695_OP_CODE_REG		0x01		/* Register */
#define OV7695_OP_CODE_DLY		0x02		/* Delay */
#define OV7695_OP_CODE_END		0x03		/* End of initial setting. */

typedef struct
{
	u16 init_reg;
	u16 init_val;	/* Save the register value and delay tick */
	u8 op_code;		/* 0 - Initial value, 1 - Register, 2 - Delay, 3 - End of setting. */
} OV7695_initial_set_struct;

 static OV7695_initial_set_struct OV7695_Init_Reg[1000];

 static u32 strtol(const char *nptr, u8 base)
{
	u32 ret;
	if(!nptr || (base!=16 && base!=10 && base!=8))
	{
		printk("%s(): NULL pointer input\n", __FUNCTION__);
		return -1;
	}
	for(ret=0; *nptr; nptr++)
	{
		if((base==16 && *nptr>='A' && *nptr<='F') || 
			(base==16 && *nptr>='a' && *nptr<='f') || 
			(base>=10 && *nptr>='0' && *nptr<='9') ||
			(base>=8 && *nptr>='0' && *nptr<='7') )
		{
			ret *= base;
			if(base==16 && *nptr>='A' && *nptr<='F')
				ret += *nptr-'A'+10;
			else if(base==16 && *nptr>='a' && *nptr<='f')
				ret += *nptr-'a'+10;
			else if(base>=10 && *nptr>='0' && *nptr<='9')
				ret += *nptr-'0';
			else if(base>=8 && *nptr>='0' && *nptr<='7')
				ret += *nptr-'0';
		}
		else
			return ret;
	}
	return ret;
}

int OV7695PIP_Initialize_from_T_Flash(void)
{
	u8 *curr_ptr = NULL;
	u32 file_size = 0;
	u32 i = 0, j = 0;
	u8 func_ind[4] = {0};	/* REG or DLY */

	struct file *fp; 
	mm_segment_t fs; 
	loff_t pos = 0; 
	static u8 data_buff[10*1024] ;
 
	fp = filp_open("/storage/sdcard0/ov7695pip_sd.dat", O_RDONLY , 0); 
	if (IS_ERR(fp)) { 
		printk("create file error\n"); 
		return 2;//-1; 
	} 
	else
		printk("OV7695_Initialize_from_T_Flash Open File Success\n");
	
	fs = get_fs(); 
	set_fs(KERNEL_DS); 

	file_size = vfs_llseek(fp, 0, SEEK_END);
	vfs_read(fp, data_buff, file_size, &pos); 
	filp_close(fp, NULL); 
	set_fs(fs);

printk("1\n");

	/* Start parse the setting witch read from t-flash. */
	curr_ptr = data_buff;
	while (curr_ptr < (data_buff + file_size))
	{
		while ((*curr_ptr == ' ') || (*curr_ptr == '\t'))/* Skip the Space & TAB */
			curr_ptr++;				

		if (((*curr_ptr) == '/') && ((*(curr_ptr + 1)) == '*'))
		{
			while (!(((*curr_ptr) == '*') && ((*(curr_ptr + 1)) == '/')))
			{
				curr_ptr++;		/* Skip block comment code. */
			}

			while (!((*curr_ptr == 0x0D) && (*(curr_ptr+1) == 0x0A)))
			{
				curr_ptr++;
			}

			curr_ptr += 2;						/* Skip the enter line */
			
			continue ;
		}
		
		if (((*curr_ptr) == '/') || ((*curr_ptr) == '{') || ((*curr_ptr) == '}'))		/* Comment line, skip it. */
		{
			while (!((*curr_ptr == 0x0D) && (*(curr_ptr+1) == 0x0A)))
			{
				curr_ptr++;
			}

			curr_ptr += 2;						/* Skip the enter line */

			continue ;
		}
		/* This just content one enter line. */
		if (((*curr_ptr) == 0x0D) && ((*(curr_ptr + 1)) == 0x0A))
		{
			curr_ptr += 2;
			continue ;
		}
		memcpy(func_ind, curr_ptr, 3);
	
		if (strcmp((const char *)func_ind, "REG") == 0)		/* REG */
		{
			curr_ptr += 6;				/* Skip "REG(0x" or "DLY(" */
			OV7695_Init_Reg[i].op_code = OV7695_OP_CODE_REG;
			
			OV7695_Init_Reg[i].init_reg = strtol((const char *)curr_ptr, 16);
			curr_ptr += 7;	/* Skip "0000,0x" */
		
			OV7695_Init_Reg[i].init_val = strtol((const char *)curr_ptr, 16);
			curr_ptr += 4;	/* Skip "00);" */
		
		}
		else									/* DLY */
		{
			/* Need add delay for this setting. */
			curr_ptr += 4;	
			OV7695_Init_Reg[i].op_code = OV7695_OP_CODE_DLY;
			
			OV7695_Init_Reg[i].init_reg = 0xFF;
			OV7695_Init_Reg[i].init_val = strtol((const char *)curr_ptr,  10);	/* Get the delay ticks, the delay should less then 50 */
		}
		i++;

		/* Skip to next line directly. */
		while (!((*curr_ptr == 0x0D) && (*(curr_ptr+1) == 0x0A)))
		{
			curr_ptr++;
		}
		curr_ptr += 2;
	}
printk("2\n");
	/* (0xFFFF, 0xFFFF) means the end of initial setting. */
	OV7695_Init_Reg[i].op_code = OV7695_OP_CODE_END;
	OV7695_Init_Reg[i].init_reg = 0xFF;
	OV7695_Init_Reg[i].init_val = 0xFF;
	i++;
	for (j=0; j<i; j++)
		printk(" 0x%04x  ==  0x%02x\n",OV7695_Init_Reg[j].init_reg, OV7695_Init_Reg[j].init_val);

	/* Start apply the initial setting to sensor. */
	#if 1
	for (j=0; j<i; j++)
	{
		if (OV7695_Init_Reg[j].op_code == OV7695_OP_CODE_END)	/* End of the setting. */
		{
			break ;
		}
		else if (OV7695_Init_Reg[j].op_code == OV7695_OP_CODE_DLY)
		{
			msleep(OV7695_Init_Reg[j].init_val);		/* Delay */
		}
		else if (OV7695_Init_Reg[j].op_code == OV7695_OP_CODE_REG)
		{
		
			OV7695PIPYUV_write_cmos_sensor(OV7695_Init_Reg[j].init_reg, OV7695_Init_Reg[j].init_val);
		}
		else
		{
			printk("REG ERROR!\n");
		}
	}
#endif

printk("3\n");
	return 1;	
}

#endif

/*************************************************************************
* FUNCTION
*    OV7695PIPYUVInitialSetting
*
* DESCRIPTION
*    This function initialize the registers of CMOS sensor.
*
* PARAMETERS
*    None
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
void OV7695PIPYUVInitialSetting(void)
{
    //;OV7695PIPYUV 2lane SPI QVGA
    OV7695PIPYUV_write_cmos_sensor(0x0103, 0x01);//    ; software reset
    mDELAY(5);
    OV7695PIPYUV_write_cmos_sensor(0x3620, 0x2f);//
    OV7695PIPYUV_write_cmos_sensor(0x3621, 0x77);//
    OV7695PIPYUV_write_cmos_sensor(0x3623, 0x12);//
    OV7695PIPYUV_write_cmos_sensor(0x3718, 0x88);//
    OV7695PIPYUV_write_cmos_sensor(0x3631, 0x44);//
    OV7695PIPYUV_write_cmos_sensor(0x3632, 0x05);//
    OV7695PIPYUV_write_cmos_sensor(0x3013, 0xd0);//
    OV7695PIPYUV_write_cmos_sensor(0x3713, 0x0e);//
    OV7695PIPYUV_write_cmos_sensor(0x3717, 0x19); //
    OV7695PIPYUV_write_cmos_sensor(0x0340, 0x02);//    ; VTS = 740
    OV7695PIPYUV_write_cmos_sensor(0x0341, 0xe4);//    ; VTS
    OV7695PIPYUV_write_cmos_sensor(0x0342, 0x02);//    ; HTS = 540
    OV7695PIPYUV_write_cmos_sensor(0x0343, 0x1c);//    ; HTS
    OV7695PIPYUV_write_cmos_sensor(0x0344, 0x00);//    ; x start = 0
    OV7695PIPYUV_write_cmos_sensor(0x0345, 0x00);//    ; x start
    OV7695PIPYUV_write_cmos_sensor(0x0348, 0x02);//    ; x end = 479
    OV7695PIPYUV_write_cmos_sensor(0x0349, 0x7f);//    ; x end
    OV7695PIPYUV_write_cmos_sensor(0x034c, 0x01);//    ; x output size = 320
    OV7695PIPYUV_write_cmos_sensor(0x034d, 0x40);//    ; x output size
    OV7695PIPYUV_write_cmos_sensor(0x034e, 0x00);//    ; y output size = 240
    OV7695PIPYUV_write_cmos_sensor(0x034f, 0xf0);//    ; y output size
    OV7695PIPYUV_write_cmos_sensor(0x0383, 0x03);//    ; x odd inc
    OV7695PIPYUV_write_cmos_sensor(0x4500, 0x47);//    ; x sub control
    OV7695PIPYUV_write_cmos_sensor(0x0387, 0x03);//    ; y odd inc
    OV7695PIPYUV_write_cmos_sensor(0x3821, 0x01);//    ; H bin on
    OV7695PIPYUV_write_cmos_sensor(0x4803, 0x08);//    ; HS prepare
    OV7695PIPYUV_write_cmos_sensor(0x370a, 0x23);//
    OV7695PIPYUV_write_cmos_sensor(0x4008, 0x01);//    ; HS prepare
    OV7695PIPYUV_write_cmos_sensor(0x4009, 0x04);//    ; blc end
    OV7695PIPYUV_write_cmos_sensor(0x4300, 0x30);//    ; output YUV 422
    OV7695PIPYUV_write_cmos_sensor(0x3820, 0x94);//    ; V bin on, bypass MIPI
    OV7695PIPYUV_write_cmos_sensor(0x4f05, 0x01);//    ; SPI on
    OV7695PIPYUV_write_cmos_sensor(0x4f06, 0x02);//    ; SPI on
    OV7695PIPYUV_write_cmos_sensor(0x3012, 0x00);//
    OV7695PIPYUV_write_cmos_sensor(0x3005, 0x0f);//
    OV7695PIPYUV_write_cmos_sensor(0x3001, 0x15);//    ; SPI drive 1.75x, FSIN drive 0.75x
    OV7695PIPYUV_write_cmos_sensor(0x3002, 0x08);//    ; FSIN input
    OV7695PIPYUV_write_cmos_sensor(0x3823, 0x30);//
    OV7695PIPYUV_write_cmos_sensor(0x4303, 0xee);//    ; Y max
    OV7695PIPYUV_write_cmos_sensor(0x4307, 0xee);//    ; U max
    OV7695PIPYUV_write_cmos_sensor(0x430b, 0xee);//    ; V max
    OV7695PIPYUV_write_cmos_sensor(0x3014, 0x00);//
    OV7695PIPYUV_write_cmos_sensor(0x301a, 0xf0);//
    OV7695PIPYUV_write_cmos_sensor(0x3024, 0x08);//
    OV7695PIPYUV_write_cmos_sensor(0x3a02, 0x02);//    ; max exp 60 = 738
    OV7695PIPYUV_write_cmos_sensor(0x3a03, 0xe2);//    ; max exp 60
    OV7695PIPYUV_write_cmos_sensor(0x3a08, 0x00);//    ; B50 = 222
    OV7695PIPYUV_write_cmos_sensor(0x3a09, 0xde);//    ; B50
    OV7695PIPYUV_write_cmos_sensor(0x3a0a, 0x00);//    ; B60 = 185
    OV7695PIPYUV_write_cmos_sensor(0x3a0b, 0xb9);//    ; B60
    OV7695PIPYUV_write_cmos_sensor(0x3a0d, 0x04);//    ; Max 60
    OV7695PIPYUV_write_cmos_sensor(0x3a0e, 0x03);//    ; Max 50
    OV7695PIPYUV_write_cmos_sensor(0x3a14, 0x02);//    ; max exp 50 = 738
    OV7695PIPYUV_write_cmos_sensor(0x3a15, 0xe2);//    ; max exp 50
    OV7695PIPYUV_write_cmos_sensor(0x3826, 0x00);//
    OV7695PIPYUV_write_cmos_sensor(0x3827, 0x00);//
    OV7695PIPYUV_write_cmos_sensor(0x0101, 0x00);//     ; mirror_off
    OV7695PIPYUV_write_cmos_sensor(0x5002, 0x4a);//     ; [7:6] Y source select, 50Hz
    OV7695PIPYUV_write_cmos_sensor(0x5910, 0x00);//     ; Y formula
    OV7695PIPYUV_write_cmos_sensor(0x3a0f, 0x58);//    ; AEC H in
    OV7695PIPYUV_write_cmos_sensor(0x3a10, 0x50);//     ; AEC L in
    OV7695PIPYUV_write_cmos_sensor(0x3a1b, 0x5a);//     ; AEC H out
    OV7695PIPYUV_write_cmos_sensor(0x3a1e, 0x4e);//     ; AEC L out
    OV7695PIPYUV_write_cmos_sensor(0x3a11, 0xa0);//     ; fast zone H
    OV7695PIPYUV_write_cmos_sensor(0x3a1f, 0x28);//     ; fase zone L
    OV7695PIPYUV_write_cmos_sensor(0x3a18, 0x00);//     ; max gain
    OV7695PIPYUV_write_cmos_sensor(0x3a19, 0xf8);//     ; max gain 15.5x
    OV7695PIPYUV_write_cmos_sensor(0x3503, 0x00);//     ; aec/agc on
    OV7695PIPYUV_write_cmos_sensor(0x5200, 0x20);//     ; AWBOFF //0x20
    OV7695PIPYUV_write_cmos_sensor(0x5204, 0x05);//  580   ;
    OV7695PIPYUV_write_cmos_sensor(0x5205, 0x80);//     ;
    OV7695PIPYUV_write_cmos_sensor(0x5206, 0x04);//     ;
    OV7695PIPYUV_write_cmos_sensor(0x5207, 0x00);//     ;
    OV7695PIPYUV_write_cmos_sensor(0x5208, 0x06);// 770    ;
    OV7695PIPYUV_write_cmos_sensor(0x5209, 0x00);//     ;
    OV7695PIPYUV_write_cmos_sensor(0x5000, 0xff);//     ; lcd,gma,awb,awbg,bc,wc,lenc,isp
    OV7695PIPYUV_write_cmos_sensor(0x5001, 0x3f);//     ; avg, blc,sde,uv_avg,cmx, cip
    OV7695PIPYUV_write_cmos_sensor(0x5100, 0x01);//    ; r x0    //;lens
    OV7695PIPYUV_write_cmos_sensor(0x5101, 0xc0);//    ; r x0
    OV7695PIPYUV_write_cmos_sensor(0x5102, 0x01);// ; r y0
    OV7695PIPYUV_write_cmos_sensor(0x5103, 0x00);// ; r y0
    OV7695PIPYUV_write_cmos_sensor(0x5104, 0x4b);// ; red_a1
    OV7695PIPYUV_write_cmos_sensor(0x5105, 0x04);// ; red_a2
    OV7695PIPYUV_write_cmos_sensor(0x5106, 0xc2);//    ; red_b1
    OV7695PIPYUV_write_cmos_sensor(0x5107, 0x08);//    ; red_b2
    OV7695PIPYUV_write_cmos_sensor(0x5108, 0x01);//    ; g x0
    OV7695PIPYUV_write_cmos_sensor(0x5109, 0xd0);//    ; g x0
    OV7695PIPYUV_write_cmos_sensor(0x510a, 0x01);//    ; g x0
    OV7695PIPYUV_write_cmos_sensor(0x510b, 0x00);//    ; g x0
    OV7695PIPYUV_write_cmos_sensor(0x510c, 0x30);//    ; g_a1
    OV7695PIPYUV_write_cmos_sensor(0x510d, 0x04);//    ; g_a2
    OV7695PIPYUV_write_cmos_sensor(0x510e, 0xc2);//    ; g_b1
    OV7695PIPYUV_write_cmos_sensor(0x510f, 0x08);//    ; g_b2
    OV7695PIPYUV_write_cmos_sensor(0x5110, 0x01);//    ; b x0
    OV7695PIPYUV_write_cmos_sensor(0x5111, 0xd0);//    ; b x0
    OV7695PIPYUV_write_cmos_sensor(0x5112, 0x01);//    ; b y0
    OV7695PIPYUV_write_cmos_sensor(0x5113, 0x00);//    ; b y0
    OV7695PIPYUV_write_cmos_sensor(0x5114, 0x26);//    ; b_a1
    OV7695PIPYUV_write_cmos_sensor(0x5115, 0x04);//    ; b_a2
    OV7695PIPYUV_write_cmos_sensor(0x5116, 0xc2);//    ; b_b1
    OV7695PIPYUV_write_cmos_sensor(0x5117, 0x08);//    ; b_b2
    OV7695PIPYUV_write_cmos_sensor(0x520a, 0x74);//    ; r range from 0x400 to 0x7ff ;AWB
    OV7695PIPYUV_write_cmos_sensor(0x520b, 0x64);//    ; g range from 0x400 to 0x6ff
    OV7695PIPYUV_write_cmos_sensor(0x520c, 0xd4);//    ; b range from 0x400 to 0xcff
    OV7695PIPYUV_write_cmos_sensor(0x5301, 0x05);//Gamma
    OV7695PIPYUV_write_cmos_sensor(0x5302, 0x0c);//
    OV7695PIPYUV_write_cmos_sensor(0x5303, 0x1c);//
    OV7695PIPYUV_write_cmos_sensor(0x5304, 0x2a);//
    OV7695PIPYUV_write_cmos_sensor(0x5305, 0x39);//
    OV7695PIPYUV_write_cmos_sensor(0x5306, 0x45);//
    OV7695PIPYUV_write_cmos_sensor(0x5307, 0x53);//
    OV7695PIPYUV_write_cmos_sensor(0x5308, 0x5d);//
    OV7695PIPYUV_write_cmos_sensor(0x5309, 0x68);//
    OV7695PIPYUV_write_cmos_sensor(0x530a, 0x7f);//
    OV7695PIPYUV_write_cmos_sensor(0x530b, 0x91);//
    OV7695PIPYUV_write_cmos_sensor(0x530c, 0xa5);//
    OV7695PIPYUV_write_cmos_sensor(0x530d, 0xc6);//
    OV7695PIPYUV_write_cmos_sensor(0x530e, 0xde);//
    OV7695PIPYUV_write_cmos_sensor(0x530f, 0xef);//
    OV7695PIPYUV_write_cmos_sensor(0x5310, 0x16);//
    OV7695PIPYUV_write_cmos_sensor(0x5003, 0x80);//sharpen/denoise ;enable bit7 to avoid even/odd pattern at bright area
    OV7695PIPYUV_write_cmos_sensor(0x5500, 0x08);//     ;sharp th1 8x
    OV7695PIPYUV_write_cmos_sensor(0x5501, 0x48);//     ;sharp th2 8x
    OV7695PIPYUV_write_cmos_sensor(0x5502, 0x20);//     ;sharp mt offset1 10
    OV7695PIPYUV_write_cmos_sensor(0x5503, 0x08);//     ;sharp mt offset2 0
    OV7695PIPYUV_write_cmos_sensor(0x5504, 0x08);//     ;dns th1 8x
    OV7695PIPYUV_write_cmos_sensor(0x5505, 0x48);//     ;dns th2 8x
    OV7695PIPYUV_write_cmos_sensor(0x5506, 0x0c);//     ;dns offset1 02
    OV7695PIPYUV_write_cmos_sensor(0x5507, 0x16);//     ;dns offset2
    OV7695PIPYUV_write_cmos_sensor(0x5508, 0xad);//     ;[6]:sharp_man [4]:dns_man  2d
    OV7695PIPYUV_write_cmos_sensor(0x5509, 0x08);//     ;sharpth th1 8x
    OV7695PIPYUV_write_cmos_sensor(0x550a, 0x48);//     ;sharpth th2 8x
    OV7695PIPYUV_write_cmos_sensor(0x550b, 0x06);//     ;sharpth offset1
    OV7695PIPYUV_write_cmos_sensor(0x550c, 0x04);//     ;sharpth offset2
    OV7695PIPYUV_write_cmos_sensor(0x550d, 0x01);//     ;recursive_en
    OV7695PIPYUV_write_cmos_sensor(0x5800, 0x02);//    ; saturation on
    OV7695PIPYUV_write_cmos_sensor(0x5803, 0x40);//    ; sat 2
    OV7695PIPYUV_write_cmos_sensor(0x5804, 0x34);//    ; sat 1
    OV7695PIPYUV_write_cmos_sensor(0x5600, 0x00);////;@@ CMX
    OV7695PIPYUV_write_cmos_sensor(0x5601, 0x2c);//
    OV7695PIPYUV_write_cmos_sensor(0x5602, 0x5a);//
    OV7695PIPYUV_write_cmos_sensor(0x5603, 0x06);//
    OV7695PIPYUV_write_cmos_sensor(0x5604, 0x1c);//
    OV7695PIPYUV_write_cmos_sensor(0x5605, 0x65);//
    OV7695PIPYUV_write_cmos_sensor(0x5606, 0x81);//
    OV7695PIPYUV_write_cmos_sensor(0x5607, 0x9f);//
    OV7695PIPYUV_write_cmos_sensor(0x5608, 0x8a);//
    OV7695PIPYUV_write_cmos_sensor(0x5609, 0x15);//
    OV7695PIPYUV_write_cmos_sensor(0x560a, 0x01);//
    OV7695PIPYUV_write_cmos_sensor(0x560b, 0x9c);//
    OV7695PIPYUV_write_cmos_sensor(0x3630, 0x69);//

#if 0
    OV7695_write_cmos_sensor(0x5908,0x00); // ;
    OV7695_write_cmos_sensor(0x5909,0x00); // ;
    OV7695_write_cmos_sensor(0x590a,0x10); // ;
    OV7695_write_cmos_sensor(0x590b,0x01); // ;
    OV7695_write_cmos_sensor(0x590c,0x10); // ;
    OV7695_write_cmos_sensor(0x580d,0x01); // ;
    OV7695_write_cmos_sensor(0x580e,0x00); // ;
    OV7695_write_cmos_sensor(0x580f,0x00); // ;
#endif

    //ken update, begin
    OV7695PIPYUV_write_cmos_sensor(0x520a,0x55); // ;
    OV7695PIPYUV_write_cmos_sensor(0x520b,0x44); // ;
    OV7695PIPYUV_write_cmos_sensor(0x520c,0x55); // ;
    mDELAY(40);
    OV7695PIPYUV_write_cmos_sensor(0x520a,0x83); // ;
    OV7695PIPYUV_write_cmos_sensor(0x520b,0x44); // ;
    OV7695PIPYUV_write_cmos_sensor(0x520c,0x83); // ;

    OV7695PIPYUV_write_cmos_sensor(0x3a0f,0x58); // ;60
    OV7695PIPYUV_write_cmos_sensor(0x3a10,0x50); // ;50
    OV7695PIPYUV_write_cmos_sensor(0x3a1b,0x60); // ;
    OV7695PIPYUV_write_cmos_sensor(0x3a1e,0x50); // ;
    OV7695PIPYUV_write_cmos_sensor(0x3a11,0x90); // ;
    OV7695PIPYUV_write_cmos_sensor(0x3a1f,0x30); // ;
    OV7695PIPYUV_write_cmos_sensor(0x3a08,0x00); // ;
    OV7695PIPYUV_write_cmos_sensor(0x3a09,0x50); // ;
    OV7695PIPYUV_write_cmos_sensor(0x3a0a,0x00); // ;
    OV7695PIPYUV_write_cmos_sensor(0x3a0b,0x43); // ;
    OV7695PIPYUV_write_cmos_sensor(0x3a0d,0x0f); // ;
    OV7695PIPYUV_write_cmos_sensor(0x3a0e,0x0d); // ;
    OV7695PIPYUV_write_cmos_sensor(0x3a02,0x04); // ;
    OV7695PIPYUV_write_cmos_sensor(0x3a03,0x30); // ;
    OV7695PIPYUV_write_cmos_sensor(0x3a14,0x04); // ;
    OV7695PIPYUV_write_cmos_sensor(0x3a15,0x30); // ;
    //ken update, end
}

void OV7695PIPYUVInitialSettingVar(void)
{
    spin_lock(&ov7695pipyuv_drv_lock);
    OV7695PIPYUVSensor.IsPVmode= 1;
    OV7695PIPYUVSensor.PreviewDummyPixels= 0;
    OV7695PIPYUVSensor.PreviewDummyLines= 0;
    OV7695PIPYUVSensor.PreviewPclk= 120;
    OV7695PIPYUVSensor.CapturePclk= 120;
    OV7695PIPYUVSensor.PreviewShutter=0x0000; //0265
    OV7695PIPYUVSensor.SensorGain=0x10;
    spin_unlock(&ov7695pipyuv_drv_lock);
}
/*****************************************************************
* FUNCTION
*    OV7695PIPYUVPreviewSetting
*
* DESCRIPTION
*    This function config Preview setting related registers of CMOS sensor.
*
* PARAMETERS
*    None
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
void OV7695PIPYUVPreviewSetting_QVGA(void)
{
    //QVGA for preview
#if defined (OV7695_15FPS)
    OV7695PIPYUV_write_cmos_sensor(0x0340, 0x05);//    ; VTS = 740
    OV7695PIPYUV_write_cmos_sensor(0x0341, 0xc8);//    ; VTS
#else
    OV7695PIPYUV_write_cmos_sensor(0x0340, 0x02);//    ; VTS = 740
    OV7695PIPYUV_write_cmos_sensor(0x0341, 0xe4);//    ; VTS
#endif
    OV7695PIPYUV_write_cmos_sensor(0x0342, 0x02);//    ; HTS = 540
    OV7695PIPYUV_write_cmos_sensor(0x0343, 0x1c);//    ; HTS
    OV7695PIPYUV_write_cmos_sensor(0x0344, 0x00);//    ; x start = 0
    OV7695PIPYUV_write_cmos_sensor(0x0345, 0x00);//    ; x start
    OV7695PIPYUV_write_cmos_sensor(0x0348, 0x02);//    ; x end = 639
    OV7695PIPYUV_write_cmos_sensor(0x0349, 0x7f);//    ; x end
    OV7695PIPYUV_write_cmos_sensor(0x034a, 0x01);//    ; y end = 479
    OV7695PIPYUV_write_cmos_sensor(0x034b, 0xdf);//    ; y end
    OV7695PIPYUV_write_cmos_sensor(0x034c, 0x01);//    ; x output size = 320
    OV7695PIPYUV_write_cmos_sensor(0x034d, 0x40);//    ; x output size
    OV7695PIPYUV_write_cmos_sensor(0x034e, 0x00);//    ; y output size = 240
    OV7695PIPYUV_write_cmos_sensor(0x034f, 0xf0);//    ; y output size
    OV7695PIPYUV_write_cmos_sensor(0x0383, 0x03);//    ; x odd inc
    OV7695PIPYUV_write_cmos_sensor(0x4500, 0x47);//    ; x sub control
    OV7695PIPYUV_write_cmos_sensor(0x0387, 0x03);//    ; y odd inc
    OV7695PIPYUV_write_cmos_sensor(0x3821, 0x01);//    ; H bin on
    OV7695PIPYUV_write_cmos_sensor(0x4008, 0x01);//    ; blc start
    OV7695PIPYUV_write_cmos_sensor(0x4009, 0x04);//    ; blc end
    OV7695PIPYUV_write_cmos_sensor(0x3012, 0x00);//
    OV7695PIPYUV_write_cmos_sensor(0x3001, 0x15);//    ; SPI drive 1.75x, FSIN drive 0.75x
    OV7695PIPYUV_write_cmos_sensor(0x3002, 0x08);//
    OV7695PIPYUV_write_cmos_sensor(0x3014, 0x00);//
    OV7695PIPYUV_write_cmos_sensor(0x301a, 0xf0);//
    OV7695PIPYUV_write_cmos_sensor(0x3024, 0x08);//
#ifdef OV7695_15FPS
    OV7695PIPYUV_write_cmos_sensor(0x3a02, 0x05);//    ; max exp 60 = 738
    OV7695PIPYUV_write_cmos_sensor(0x3a03, 0xc8);//    ; max exp 60
    OV7695PIPYUV_write_cmos_sensor(0x3a09, 0xde);//    ; B50 = 222
    OV7695PIPYUV_write_cmos_sensor(0x3a0b, 0xb9);//    ; B60 = 185
    OV7695PIPYUV_write_cmos_sensor(0x3a0d, 0x08);//    ; Max 60
    OV7695PIPYUV_write_cmos_sensor(0x3a0e, 0x06);//    ; Max 50
    OV7695PIPYUV_write_cmos_sensor(0x3a14, 0x05);//    ; max exp 50 = 738
    OV7695PIPYUV_write_cmos_sensor(0x3a15, 0xc8);//    ; max exp 50
#else
    OV7695PIPYUV_write_cmos_sensor(0x3a02, 0x02);//    ; max exp 60 = 738
    OV7695PIPYUV_write_cmos_sensor(0x3a03, 0xe2);//    ; max exp 60
    OV7695PIPYUV_write_cmos_sensor(0x3a09, 0xde);//    ; B50 = 222
    OV7695PIPYUV_write_cmos_sensor(0x3a0b, 0xb9);//    ; B60 = 185
    OV7695PIPYUV_write_cmos_sensor(0x3a0d, 0x04);//    ; Max 60
    OV7695PIPYUV_write_cmos_sensor(0x3a0e, 0x03);//    ; Max 50
    OV7695PIPYUV_write_cmos_sensor(0x3a14, 0x02);//    ; max exp 50 = 738
    OV7695PIPYUV_write_cmos_sensor(0x3a15, 0xe2);//    ; max exp 50
#endif
    OV7695PIPYUV_write_cmos_sensor(0x5002, 0x42);//    ; 50Hz
    OV7695PIPYUV_write_cmos_sensor(0x5200, 0x00);//     ; AWBOFF //0x20
    OV7695PIPYUV_write_cmos_sensor(0x5204, 0x04);//  580   ;
    OV7695PIPYUV_write_cmos_sensor(0x5205, 0x00);//     ;
    OV7695PIPYUV_write_cmos_sensor(0x5206, 0x04);//     ;
    OV7695PIPYUV_write_cmos_sensor(0x5207, 0x00);//     ;
    OV7695PIPYUV_write_cmos_sensor(0x5208, 0x04);// 770    ;
    OV7695PIPYUV_write_cmos_sensor(0x5209, 0x00);//     ;

    //ken update, begin
    OV7695PIPYUV_write_cmos_sensor(0x520a,0x55); // ;
    OV7695PIPYUV_write_cmos_sensor(0x520b,0x44); // ;
    OV7695PIPYUV_write_cmos_sensor(0x520c,0x55); // ;
    mDELAY(40);
    OV7695PIPYUV_write_cmos_sensor(0x520a,0x83); // ;
    OV7695PIPYUV_write_cmos_sensor(0x520b,0x44); // ;
    OV7695PIPYUV_write_cmos_sensor(0x520c,0x83); // ;
    //ken update, end

    spin_lock(&ov7695pipyuv_drv_lock);
    OV7695PIPYUVSensor.IsPVmode = KAL_TRUE;
    OV7695PIPYUVSensor.PreviewPclk= 120;
    OV7695PIPYUVSensor.SensorMode= OV7695PIPYUV_SENSOR_MODE_PREVIEW;
    spin_unlock(&ov7695pipyuv_drv_lock);
}

void OV7695PIPYUVVideoSetting_QVGA(void)
{
    //QVGA for preview
#if defined (OV7695_15FPS)
    OV7695PIPYUV_write_cmos_sensor(0x0340, 0x05);//    ; VTS = 740
    OV7695PIPYUV_write_cmos_sensor(0x0341, 0xc8);//    ; VTS
#else
    OV7695PIPYUV_write_cmos_sensor(0x0340, 0x02);//    ; VTS = 740
    OV7695PIPYUV_write_cmos_sensor(0x0341, 0xe4);//    ; VTS
#endif
    OV7695PIPYUV_write_cmos_sensor(0x0342, 0x02);//    ; HTS = 540
    OV7695PIPYUV_write_cmos_sensor(0x0343, 0x1c);//    ; HTS
    OV7695PIPYUV_write_cmos_sensor(0x0344, 0x00);//    ; x start = 0
    OV7695PIPYUV_write_cmos_sensor(0x0345, 0x00);//    ; x start
    OV7695PIPYUV_write_cmos_sensor(0x0348, 0x02);//    ; x end = 639
    OV7695PIPYUV_write_cmos_sensor(0x0349, 0x7f);//    ; x end
    OV7695PIPYUV_write_cmos_sensor(0x034a, 0x01);//    ; y end = 479
    OV7695PIPYUV_write_cmos_sensor(0x034b, 0xdf);//    ; y end
    OV7695PIPYUV_write_cmos_sensor(0x034c, 0x01);//    ; x output size = 320
    OV7695PIPYUV_write_cmos_sensor(0x034d, 0x40);//    ; x output size
    OV7695PIPYUV_write_cmos_sensor(0x034e, 0x00);//    ; y output size = 240
    OV7695PIPYUV_write_cmos_sensor(0x034f, 0xf0);//    ; y output size
    OV7695PIPYUV_write_cmos_sensor(0x0383, 0x03);//    ; x odd inc
    OV7695PIPYUV_write_cmos_sensor(0x4500, 0x47);//    ; x sub control
    OV7695PIPYUV_write_cmos_sensor(0x0387, 0x03);//    ; y odd inc
    OV7695PIPYUV_write_cmos_sensor(0x3821, 0x01);//    ; H bin on
    OV7695PIPYUV_write_cmos_sensor(0x4008, 0x01);//    ; blc start
    OV7695PIPYUV_write_cmos_sensor(0x4009, 0x04);//    ; blc end
    OV7695PIPYUV_write_cmos_sensor(0x3012, 0x00);//
    OV7695PIPYUV_write_cmos_sensor(0x3001, 0x15);//    ; SPI drive 1.75x, FSIN drive 0.75x
    OV7695PIPYUV_write_cmos_sensor(0x3002, 0x08);//
    OV7695PIPYUV_write_cmos_sensor(0x3014, 0x00);//
    OV7695PIPYUV_write_cmos_sensor(0x301a, 0xf0);//
    OV7695PIPYUV_write_cmos_sensor(0x3024, 0x08);//
#ifdef OV7695_15FPS
    OV7695PIPYUV_write_cmos_sensor(0x3a02, 0x05);//    ; max exp 60 = 738
    OV7695PIPYUV_write_cmos_sensor(0x3a03, 0xc8);//    ; max exp 60
    OV7695PIPYUV_write_cmos_sensor(0x3a09, 0xde);//    ; B50 = 222
    OV7695PIPYUV_write_cmos_sensor(0x3a0b, 0xb9);//    ; B60 = 185
    OV7695PIPYUV_write_cmos_sensor(0x3a0d, 0x08);//    ; Max 60
    OV7695PIPYUV_write_cmos_sensor(0x3a0e, 0x06);//    ; Max 50
    OV7695PIPYUV_write_cmos_sensor(0x3a14, 0x05);//    ; max exp 50 = 738
    OV7695PIPYUV_write_cmos_sensor(0x3a15, 0xc8);//    ; max exp 50
#else
    OV7695PIPYUV_write_cmos_sensor(0x3a02, 0x02);//    ; max exp 60 = 738
    OV7695PIPYUV_write_cmos_sensor(0x3a03, 0xe2);//    ; max exp 60
    OV7695PIPYUV_write_cmos_sensor(0x3a09, 0xde);//    ; B50 = 222
    OV7695PIPYUV_write_cmos_sensor(0x3a0b, 0xb9);//    ; B60 = 185
    OV7695PIPYUV_write_cmos_sensor(0x3a0d, 0x04);//    ; Max 60
    OV7695PIPYUV_write_cmos_sensor(0x3a0e, 0x03);//    ; Max 50
    OV7695PIPYUV_write_cmos_sensor(0x3a14, 0x02);//    ; max exp 50 = 738
    OV7695PIPYUV_write_cmos_sensor(0x3a15, 0xe2);//    ; max exp 50
#endif
    OV7695PIPYUV_write_cmos_sensor(0x5002, 0x42);//    ; 50Hz
    OV7695PIPYUV_write_cmos_sensor(0x5200, 0x00);//     ; AWBOFF //0x20
    OV7695PIPYUV_write_cmos_sensor(0x5204, 0x04);//  580   ;
    OV7695PIPYUV_write_cmos_sensor(0x5205, 0x00);//     ;
    OV7695PIPYUV_write_cmos_sensor(0x5206, 0x04);//     ;
    OV7695PIPYUV_write_cmos_sensor(0x5207, 0x00);//     ;
    OV7695PIPYUV_write_cmos_sensor(0x5208, 0x04);// 770    ;
    OV7695PIPYUV_write_cmos_sensor(0x5209, 0x00);//     ;

    spin_lock(&ov7695pipyuv_drv_lock);
    OV7695PIPYUVSensor.IsPVmode = KAL_TRUE;
    OV7695PIPYUVSensor.PreviewPclk= 120;
    OV7695PIPYUVSensor.SensorMode= OV7695PIPYUV_SENSOR_MODE_PREVIEW;
    spin_unlock(&ov7695pipyuv_drv_lock);
}

void OV7695PIPYUVVideoSetting_480(void)
{
    //480x480 for preview
    OV7695PIPYUV_write_cmos_sensor(0x0340, 0x02);//    ; VTS = 740
    OV7695PIPYUV_write_cmos_sensor(0x0341, 0xe4);//    ; VTS
    OV7695PIPYUV_write_cmos_sensor(0x0342, 0x02);//    ; HTS = 540
    OV7695PIPYUV_write_cmos_sensor(0x0343, 0x1c);//    ; HTS
    OV7695PIPYUV_write_cmos_sensor(0x0344, 0x00);//    ; x start = 80
    OV7695PIPYUV_write_cmos_sensor(0x0345, 0x50);//    ; x start
    OV7695PIPYUV_write_cmos_sensor(0x0348, 0x02);//    ; x end = 559
    OV7695PIPYUV_write_cmos_sensor(0x0349, 0x2f);//    ; x end
    OV7695PIPYUV_write_cmos_sensor(0x034a, 0x01);//    ; y end = 479
    OV7695PIPYUV_write_cmos_sensor(0x034b, 0xdf);//    ; y end
    OV7695PIPYUV_write_cmos_sensor(0x034c, 0x01);//    ; x output size = 480
    OV7695PIPYUV_write_cmos_sensor(0x034d, 0xe0);//    ; x output size
    OV7695PIPYUV_write_cmos_sensor(0x034e, 0x01);//    ; y output size = 480
    OV7695PIPYUV_write_cmos_sensor(0x034f, 0xe0);//    ; y output size
    OV7695PIPYUV_write_cmos_sensor(0x0383, 0x01);//    ; x odd inc
    OV7695PIPYUV_write_cmos_sensor(0x4500, 0x25);//    ; x sub control
    OV7695PIPYUV_write_cmos_sensor(0x0387, 0x01);//    ; y odd inc
    OV7695PIPYUV_write_cmos_sensor(0x3821, 0x00);//    ; H bin on
    OV7695PIPYUV_write_cmos_sensor(0x4008, 0x02);//    ; blc start
    OV7695PIPYUV_write_cmos_sensor(0x4009, 0x09);//    ; blc end
    OV7695PIPYUV_write_cmos_sensor(0x3012, 0x02);//
    OV7695PIPYUV_write_cmos_sensor(0x3001, 0x1f);//    ; SPI drive 2.75x, FSIN drive 2.75x
    OV7695PIPYUV_write_cmos_sensor(0x3002, 0x00);//
    OV7695PIPYUV_write_cmos_sensor(0x3014, 0x30);//
    OV7695PIPYUV_write_cmos_sensor(0x301a, 0x44);//
    OV7695PIPYUV_write_cmos_sensor(0x3024, 0x00);//
    OV7695PIPYUV_write_cmos_sensor(0x3a02, 0x02);//    ; max exp 60 = 738
    OV7695PIPYUV_write_cmos_sensor(0x3a03, 0xe2);//    ; max exp 60
    OV7695PIPYUV_write_cmos_sensor(0x3a09, 0xde);//    ; B50 = 222
    OV7695PIPYUV_write_cmos_sensor(0x3a0b, 0xb9);//    ; B60 = 185
    OV7695PIPYUV_write_cmos_sensor(0x3a0d, 0x04);//    ; Max 60
    OV7695PIPYUV_write_cmos_sensor(0x3a0e, 0x03);//    ; Max 50
    OV7695PIPYUV_write_cmos_sensor(0x3a14, 0x02);//    ; max exp 50 = 738
    OV7695PIPYUV_write_cmos_sensor(0x3a15, 0xe2);//    ; max exp 50
    OV7695PIPYUV_write_cmos_sensor(0x5002, 0x42);//    ; 50Hz

    spin_lock(&ov7695pipyuv_drv_lock);
    OV7695PIPYUVSensor.IsPVmode = KAL_TRUE;
    OV7695PIPYUVSensor.PreviewPclk= 120;
    OV7695PIPYUVSensor.SensorMode= OV7695PIPYUV_SENSOR_MODE_PREVIEW;
    spin_unlock(&ov7695pipyuv_drv_lock);
}

/*************************************************************************
* FUNCTION
*     OV7695PIPYUVFullSizeCaptureSetting
*
* DESCRIPTION
*    This function config full size capture setting related registers of CMOS sensor.
*
* PARAMETERS
*    None
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
void OV7695PIPYUVFullSizeCaptureSetting(void)
{
    //VGA for capture.
#ifdef OV7695_15FPS
    OV7695PIPYUV_write_cmos_sensor(0x0340, 0x0b);//    ; VTS = 1464
    OV7695PIPYUV_write_cmos_sensor(0x0341, 0x70);//    ; VTS
#else
    OV7695PIPYUV_write_cmos_sensor(0x0340, 0x05);//    ; VTS = 1464
    OV7695PIPYUV_write_cmos_sensor(0x0341, 0xb8);//    ; VTS
#endif
    OV7695PIPYUV_write_cmos_sensor(0x0342, 0x02);//    ; HTS = 746
    OV7695PIPYUV_write_cmos_sensor(0x0343, 0xea);//    ; HTS 763 fb
    OV7695PIPYUV_write_cmos_sensor(0x0344, 0x00);//    ; x start = 00
    OV7695PIPYUV_write_cmos_sensor(0x0345, 0x00);//    ; x start
    OV7695PIPYUV_write_cmos_sensor(0x0348, 0x02);//    ; x end = 639
    OV7695PIPYUV_write_cmos_sensor(0x0349, 0x7f);//    ; x end
    OV7695PIPYUV_write_cmos_sensor(0x034a, 0x01);//    ; y end = 479
    OV7695PIPYUV_write_cmos_sensor(0x034b, 0xdf);//    ; y end
    OV7695PIPYUV_write_cmos_sensor(0x034c, 0x02);//    ; x output size = 640
    OV7695PIPYUV_write_cmos_sensor(0x034d, 0x80);//    ; x output size
    OV7695PIPYUV_write_cmos_sensor(0x034e, 0x01);//    ; y output size = 480
    OV7695PIPYUV_write_cmos_sensor(0x034f, 0xe0);//    ; y output size
    OV7695PIPYUV_write_cmos_sensor(0x0383, 0x01);//    ; x odd inc
    OV7695PIPYUV_write_cmos_sensor(0x4500, 0x25);//    ; x sub control
    OV7695PIPYUV_write_cmos_sensor(0x0387, 0x01);//    ; y odd inc
    OV7695PIPYUV_write_cmos_sensor(0x3821, 0x00);//    ; H bin off
    OV7695PIPYUV_write_cmos_sensor(0x4008, 0x02);//    ; blc start
    OV7695PIPYUV_write_cmos_sensor(0x4009, 0x09);//    ; blc end
    OV7695PIPYUV_write_cmos_sensor(0x3012, 0x00);//
    OV7695PIPYUV_write_cmos_sensor(0x3001, 0x1f);//    ; SPI drive 2.75x, FSIN drive 1.75x
    OV7695PIPYUV_write_cmos_sensor(0x3002, 0x08);//
    OV7695PIPYUV_write_cmos_sensor(0x3014, 0x30);//    ; MIPI
    OV7695PIPYUV_write_cmos_sensor(0x301a, 0x34);//
    OV7695PIPYUV_write_cmos_sensor(0x3024, 0x00);//
#ifdef OV7695_15FPS
    OV7695PIPYUV_write_cmos_sensor(0x3a02, 0x0b);//    ; max exp 60 = 1968
    OV7695PIPYUV_write_cmos_sensor(0x3a03, 0x70);//    ; max exp 60
    OV7695PIPYUV_write_cmos_sensor(0x3a09, 0xa0);//    ; B50 = 160 9d
    OV7695PIPYUV_write_cmos_sensor(0x3a0b, 0x86);//    ; B60 = 134 83
    OV7695PIPYUV_write_cmos_sensor(0x3a0d, 0x16);//    ; max exp 60 b
    OV7695PIPYUV_write_cmos_sensor(0x3a0e, 0x12);//    ; Max 50
    OV7695PIPYUV_write_cmos_sensor(0x3a14, 0x0b);//    ; max exp 50 = 1968
    OV7695PIPYUV_write_cmos_sensor(0x3a15, 0x70);//    ; max exp 50
#else
    OV7695PIPYUV_write_cmos_sensor(0x3a02, 0x05);//    ; max exp 60 = 1968
    OV7695PIPYUV_write_cmos_sensor(0x3a03, 0xb8);//    ; max exp 60
    OV7695PIPYUV_write_cmos_sensor(0x3a09, 0xa0);//    ; B50 = 160 9d
    OV7695PIPYUV_write_cmos_sensor(0x3a0b, 0x86);//    ; B60 = 134 83
    OV7695PIPYUV_write_cmos_sensor(0x3a0d, 0x0a);//    ; max exp 60 b
    OV7695PIPYUV_write_cmos_sensor(0x3a0e, 0x09);//    ; Max 50
    OV7695PIPYUV_write_cmos_sensor(0x3a14, 0x05);//    ; max exp 50 = 1968
    OV7695PIPYUV_write_cmos_sensor(0x3a15, 0xb8);//    ; max exp 50
#endif
    OV7695PIPYUV_write_cmos_sensor(0x5002, 0x42);//    ;// 50Hz
    OV7695PIPYUV_write_cmos_sensor(0x5200, 0x20);//     ; AWBOFF //0x20
    OV7695PIPYUV_write_cmos_sensor(0x5204, 0x05);//  580   ;
    OV7695PIPYUV_write_cmos_sensor(0x5205, 0x80);//     ;
    OV7695PIPYUV_write_cmos_sensor(0x5206, 0x04);//     ;
    OV7695PIPYUV_write_cmos_sensor(0x5207, 0x00);//     ;
    OV7695PIPYUV_write_cmos_sensor(0x5208, 0x06);// 770    ;
    OV7695PIPYUV_write_cmos_sensor(0x5209, 0x00);//     ;

    OV7695PIPYUV_write_cmos_sensor(0x3503,0x03);
#if 0
    //ken update, begin
    OV7695PIPYUV_write_cmos_sensor(0x520a,0x55); // ;
    OV7695PIPYUV_write_cmos_sensor(0x520b,0x44); // ;
    OV7695PIPYUV_write_cmos_sensor(0x520c,0x55); // ;
    mDELAY(40);
    OV7695PIPYUV_write_cmos_sensor(0x520a,0x83); // ;
    OV7695PIPYUV_write_cmos_sensor(0x520b,0x44); // ;
    OV7695PIPYUV_write_cmos_sensor(0x520c,0x83); // ;
    //ken update, end
#endif
    spin_lock(&ov7695pipyuv_drv_lock);
    OV7695PIPYUVSensor.IsPVmode = KAL_FALSE;
    OV7695PIPYUVSensor.CapturePclk= 120;
    OV7695PIPYUVSensor.SensorMode= OV7695PIPYUV_SENSOR_MODE_CAPTURE;
    spin_unlock(&ov7695pipyuv_drv_lock);
}
#if 0
/*************************************************************************
* FUNCTION
*    OV7695PIPYUVSetHVMirror
*
* DESCRIPTION
*    This function set sensor Mirror
*
* PARAMETERS
*    Mirror
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void OV7695PIPYUVSetHVMirror(kal_uint8 Mirror)
{

}
#endif
void OV7695PIPYUV_ON100(void)
{
    OV7695PIPYUV_write_cmos_sensor(0x0100,0x01);
}

void OV7695PIPYUV_OFF100(void)
{
    OV7695PIPYUV_write_cmos_sensor(0x0100,0x00);
}

void OV7695PIPYUV_init_awb(void)
{
	OV7695PIPYUVSensor.awb[0] = 0x04;
	OV7695PIPYUVSensor.awb[1] = 0x00;
	OV7695PIPYUVSensor.awb[2] = 0x04;
	OV7695PIPYUVSensor.awb[3] = 0x00;
	OV7695PIPYUVSensor.awb[4] = 0x04;
	OV7695PIPYUVSensor.awb[5] = 0x00;
}

void OV7695PIPYUV_save_awb(void)
{
	OV7695PIPYUVSensor.awb[0] = OV7695PIPYUV_read_cmos_sensor(0x5204);
	OV7695PIPYUVSensor.awb[1] = OV7695PIPYUV_read_cmos_sensor(0x5205);
	OV7695PIPYUVSensor.awb[2] = OV7695PIPYUV_read_cmos_sensor(0x5206);
	OV7695PIPYUVSensor.awb[3] = OV7695PIPYUV_read_cmos_sensor(0x5207);
	OV7695PIPYUVSensor.awb[4] = OV7695PIPYUV_read_cmos_sensor(0x5208);
	OV7695PIPYUVSensor.awb[5] = OV7695PIPYUV_read_cmos_sensor(0x5209);
/*	OV7695PIPYUVSENSORDB("0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
		OV7695PIPYUVSensor.awb[0],
		OV7695PIPYUVSensor.awb[1],
		OV7695PIPYUVSensor.awb[2],
		OV7695PIPYUVSensor.awb[3],
		OV7695PIPYUVSensor.awb[4],
		OV7695PIPYUVSensor.awb[5]);*/
}

void OV7695PIPYUV_retore_awb(void)
{
        OV7695PIPYUV_write_cmos_sensor(0x5204,OV7695PIPYUVSensor.awb[0]);
        OV7695PIPYUV_write_cmos_sensor(0x5205,OV7695PIPYUVSensor.awb[1]);
        OV7695PIPYUV_write_cmos_sensor(0x5206,OV7695PIPYUVSensor.awb[2]);
        OV7695PIPYUV_write_cmos_sensor(0x5207,OV7695PIPYUVSensor.awb[3]);
        OV7695PIPYUV_write_cmos_sensor(0x5208,OV7695PIPYUVSensor.awb[4]);
        OV7695PIPYUV_write_cmos_sensor(0x5209,OV7695PIPYUVSensor.awb[5]);
}


/*************************************************************************
* FUNCTION
*    OV7695PIPYUVOpen
*
* DESCRIPTION
*    This function initialize the registers of CMOS sensor
*
* PARAMETERS
*    None
*
* RETURNS
*    None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV7695PIPYUVOpen(void)
{
	volatile signed int i;
	kal_uint16 sensor_id = 0;

	OV7695PIPYUVSENSORDB("enter :\n ");
	OV7695PIPYUV_GetSensorID(&sensor_id);
	if(sensor_id != OV7695PIPYUV_SENSOR_ID)
	{
		return ERROR_SENSOR_CONNECT_FAIL;
	}

	OV7695PIPYUVInitialVariable();

	OV7695PIPYUVInitialSetting();
	OV7695PIPYUVInitialSettingVar();
	return ERROR_NONE;
}    /* OV7695PIPYUVOpen() */

/*************************************************************************
* FUNCTION
*    OV7695PIPYUVClose
*
* DESCRIPTION
*    This function is to turn off sensor module power.
*
* PARAMETERS
*    None
*
* RETURNS
*    None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV7695PIPYUVClose(void)
{
  //CISModulePowerOn(FALSE);
    return ERROR_NONE;
}    /* OV7695PIPYUVClose() */
/*************************************************************************
* FUNCTION
*    OV7695PIPYUVPreview
*
* DESCRIPTION
*    This function start the sensor preview.
*
* PARAMETERS
*    *image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*    None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV7695PIPYUVPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                      MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
#if 1 //defined(MT6577)
    switch(CurrentScenarioId)
    {
    case MSDK_SCENARIO_ID_CAMERA_ZSD:
        OV7695PIPYUVFullSizeCaptureSetting();
        break;
    default:
        OV7695PIPYUVPreviewSetting_QVGA();
        break;
    }
#else
    OV7695PIPYUVPreviewSetting_QVGA();
#endif

    OV7695PIPYUV_set_AE_mode(KAL_TRUE);
    OV7695PIPYUV_set_AWB_mode(KAL_TRUE);
    mDELAY(30);
    return TRUE ;//ERROR_NONE;
}    /* OV7695PIPYUVPreview() */

UINT32 OV7695PIPYUVCapture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    kal_uint32 shutter = 0; //, temp_reg = 0;
    //kal_uint32 prev_line_len = 0;
    //kal_uint32 cap_line_len = 0;

    OV7695PIPYUVSENSORDB("[OV7695PIPYUV]OV7695PIPYUVCapture enter :\n ");
    if(OV7695PIPYUV_SENSOR_MODE_PREVIEW == OV7695PIPYUVSensor.SensorMode )
    {
        shutter=OV7695PIPYUVReadShutter();
        spin_lock(&ov7695pipyuv_drv_lock);
      OV7695PIPYUVSensor.SensorGain=OV7695PIPYUVReadSensorGain();
        spin_unlock(&ov7695pipyuv_drv_lock);
        OV7695PIPYUV_set_AE_mode(KAL_FALSE);
        OV7695PIPYUVFullSizeCaptureSetting();
        spin_lock(&ov7695pipyuv_drv_lock);
        OV7695PIPYUVSensor.CaptureDummyPixels = 0;
        OV7695PIPYUVSensor.CaptureDummyLines = 0;
        spin_unlock(&ov7695pipyuv_drv_lock);
      shutter=shutter*71/100;
        OV7695PIPYUVSetDummy(OV7695PIPYUVSensor.CaptureDummyPixels, OV7695PIPYUVSensor.CaptureDummyLines, 0);
        OV7695PIPYUVWriteShutter(shutter,0);
    mDELAY(40);
    }
    OV7695PIPYUVSENSORDB("[OV7695PIPYUV]OV7695PIPYUVCapture exit :\n ");
    return ERROR_NONE;
}/* OV7695PIPYUVCapture() */

UINT32 OV7695PIPYUVGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
    pSensorResolution->SensorPreviewWidth= OV7695PIPYUV_IMAGE_SENSOR_QVGA_WIDTH - 2 * OV7695PIPYUV_PV_GRAB_START_X;
    pSensorResolution->SensorPreviewHeight= OV7695PIPYUV_IMAGE_SENSOR_QVGA_HEIGHT - 2 * OV7695PIPYUV_PV_GRAB_START_Y;
    pSensorResolution->SensorFullWidth= OV7695PIPYUV_IMAGE_SENSOR_VGA_WITDH- 2 * OV7695PIPYUV_FULL_GRAB_START_X;
    pSensorResolution->SensorFullHeight= OV7695PIPYUV_IMAGE_SENSOR_VGA_HEIGHT- 2 * OV7695PIPYUV_FULL_GRAB_START_X;
    return ERROR_NONE;
}    /* OV7695PIPYUVGetResolution() */

UINT32 OV7695PIPYUVGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,MSDK_SENSOR_INFO_STRUCT *pSensorInfo,MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
#if 1 //defined(MT6577)
    switch (ScenarioId)
    {
    case MSDK_SCENARIO_ID_CAMERA_ZSD:
        pSensorInfo->SensorPreviewResolutionX=OV7695PIPYUV_IMAGE_SENSOR_VGA_WITDH - 2*OV7695PIPYUV_PV_GRAB_START_X;
        pSensorInfo->SensorPreviewResolutionY=OV7695PIPYUV_IMAGE_SENSOR_VGA_HEIGHT - 2*OV7695PIPYUV_PV_GRAB_START_Y;
        break;
    default:
        pSensorInfo->SensorPreviewResolutionX=OV7695PIPYUV_IMAGE_SENSOR_QVGA_WIDTH - 2*OV7695PIPYUV_PV_GRAB_START_X;
        pSensorInfo->SensorPreviewResolutionY=OV7695PIPYUV_IMAGE_SENSOR_QVGA_HEIGHT - 2*OV7695PIPYUV_PV_GRAB_START_Y;
        break;
    }
#else
    pSensorInfo->SensorPreviewResolutionX= OV7695PIPYUV_IMAGE_SENSOR_QVGA_WIDTH - 2*OV7695PIPYUV_PV_GRAB_START_X;
    pSensorInfo->SensorPreviewResolutionY= OV7695PIPYUV_IMAGE_SENSOR_QVGA_HEIGHT - 2*OV7695PIPYUV_PV_GRAB_START_Y;
#endif
    pSensorInfo->SensorFullResolutionX= OV7695PIPYUV_IMAGE_SENSOR_VGA_WITDH - 2*OV7695PIPYUV_FULL_GRAB_START_X;
    pSensorInfo->SensorFullResolutionY= OV7695PIPYUV_IMAGE_SENSOR_VGA_HEIGHT - 2*OV7695PIPYUV_FULL_GRAB_START_X;
    pSensorInfo->SensorCameraPreviewFrameRate=30;
    pSensorInfo->SensorVideoFrameRate=30;
    pSensorInfo->SensorStillCaptureFrameRate=11;
    pSensorInfo->SensorWebCamCaptureFrameRate=15;
    pSensorInfo->SensorResetActiveHigh=FALSE;
    pSensorInfo->SensorResetDelayCount=1;
    pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_VYUY; //SENSOR_OUTPUT_FORMAT_YUYV;
    pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW;    /*??? */
    pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorInterruptDelayLines = 1;
    pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_PARALLEL;//???
    pSensorInfo->CaptureDelayFrame = 2;
    pSensorInfo->PreviewDelayFrame = 2;
    pSensorInfo->VideoDelayFrame = 4;
    pSensorInfo->SensorMasterClockSwitch = 0;
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_8MA;
    switch (ScenarioId)
    {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
        //case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
        pSensorInfo->SensorClockFreq=24;//26
        pSensorInfo->SensorClockDividCount=    3;
        pSensorInfo->SensorClockRisingCount= 0;
        pSensorInfo->SensorClockFallingCount= 2;
        pSensorInfo->SensorPixelClockCount= 3;
        pSensorInfo->SensorDataLatchCount= 2;
        pSensorInfo->SensorGrabStartX = OV7695PIPYUV_PV_GRAB_START_X;
        pSensorInfo->SensorGrabStartY = OV7695PIPYUV_PV_GRAB_START_Y;
        break;
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
    //case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
#if 1 //defined(MT6577)
    case MSDK_SCENARIO_ID_CAMERA_ZSD:
#endif
        pSensorInfo->SensorClockFreq=24;//26;
        pSensorInfo->SensorClockDividCount=    3;
        pSensorInfo->SensorClockRisingCount= 0;
        pSensorInfo->SensorClockFallingCount= 2;
        pSensorInfo->SensorPixelClockCount= 3;
        pSensorInfo->SensorDataLatchCount= 2;
        pSensorInfo->SensorGrabStartX = OV7695PIPYUV_FULL_GRAB_START_X;
        pSensorInfo->SensorGrabStartY = OV7695PIPYUV_FULL_GRAB_START_Y;
        break;
    default:
        pSensorInfo->SensorClockFreq=24;//26;
        pSensorInfo->SensorClockDividCount=3;
        pSensorInfo->SensorClockRisingCount=0;
        pSensorInfo->SensorClockFallingCount=2;
        pSensorInfo->SensorPixelClockCount=3;
        pSensorInfo->SensorDataLatchCount=2;
        pSensorInfo->SensorGrabStartX = OV7695PIPYUV_PV_GRAB_START_X;
        pSensorInfo->SensorGrabStartY = OV7695PIPYUV_PV_GRAB_START_Y;
        break;
    }
    memcpy(pSensorConfigData, &OV7695PIPYUVSensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
}    /* OV7695PIPYUVGetInfo() */

UINT32 OV7695PIPYUVControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    OV7695PIPYUVSENSORDB("[OV7695PIPYUV]OV7695PIPYUVControl enter :\n ");
    CurrentScenarioId = ScenarioId;
    switch (ScenarioId)
    {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
    //case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
#if 1 //defined(MT6577)
    case MSDK_SCENARIO_ID_CAMERA_ZSD:
#endif
        OV7695PIPYUVPreview(pImageWindow, pSensorConfigData);
        break;
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        //case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
        OV7695PIPYUVCapture(pImageWindow, pSensorConfigData);
        break;
    default:
        break;
    }
    OV7695PIPYUVSENSORDB("[OV7695PIPYUV]OV7695PIPYUVControl exit :\n ");
    return ERROR_NONE;
}    /* OV7695PIPYUVControl() */

/* [TC] YUV sensor */

BOOL OV7695PIPYUV_set_param_wb(UINT16 para)
{
    switch (para)
    {
    case AWB_MODE_OFF:
        spin_lock(&ov7695pipyuv_drv_lock);
        OV7695PIPYUV_AWB_ENABLE = KAL_FALSE;
        spin_unlock(&ov7695pipyuv_drv_lock);
        OV7695PIPYUV_set_AWB_mode(OV7695PIPYUV_AWB_ENABLE);
        break;
    case AWB_MODE_AUTO:
        spin_lock(&ov7695pipyuv_drv_lock);
        OV7695PIPYUV_AWB_ENABLE = KAL_TRUE;
        spin_unlock(&ov7695pipyuv_drv_lock);
        OV7695PIPYUV_set_AWB_mode(OV7695PIPYUV_AWB_ENABLE);
        break;
    case AWB_MODE_CLOUDY_DAYLIGHT: //cloudy
        OV7695PIPYUV_set_AWB_mode(KAL_FALSE);
        OV7695PIPYUV_write_cmos_sensor(0x5204,0x06);
        OV7695PIPYUV_write_cmos_sensor(0x5205,0x30);
        OV7695PIPYUV_write_cmos_sensor(0x5206,0x04);
        OV7695PIPYUV_write_cmos_sensor(0x5207,0x00);
        OV7695PIPYUV_write_cmos_sensor(0x5208,0x04);
        OV7695PIPYUV_write_cmos_sensor(0x5209,0x30);
        break;
    case AWB_MODE_DAYLIGHT: //sunny
        OV7695PIPYUV_set_AWB_mode(KAL_FALSE);
        OV7695PIPYUV_write_cmos_sensor(0x5204,0x06);
        OV7695PIPYUV_write_cmos_sensor(0x5205,0x10);
        OV7695PIPYUV_write_cmos_sensor(0x5206,0x04);
        OV7695PIPYUV_write_cmos_sensor(0x5207,0x00);
        OV7695PIPYUV_write_cmos_sensor(0x5208,0x04);
        OV7695PIPYUV_write_cmos_sensor(0x5209,0x48);
        break;
    case AWB_MODE_INCANDESCENT: //office
        OV7695PIPYUV_set_AWB_mode(KAL_FALSE);
        OV7695PIPYUV_write_cmos_sensor(0x5204,0x04);
        OV7695PIPYUV_write_cmos_sensor(0x5205,0xe0);
        OV7695PIPYUV_write_cmos_sensor(0x5206,0x04);
        OV7695PIPYUV_write_cmos_sensor(0x5207,0x00);
        OV7695PIPYUV_write_cmos_sensor(0x5208,0x05);
        OV7695PIPYUV_write_cmos_sensor(0x5209,0xa0);
        break;
    case AWB_MODE_TUNGSTEN:
        OV7695PIPYUV_set_AWB_mode(KAL_FALSE);
        OV7695PIPYUV_write_cmos_sensor(0x5204,0x05);
        OV7695PIPYUV_write_cmos_sensor(0x5205,0x48);
        OV7695PIPYUV_write_cmos_sensor(0x5206,0x04);
        OV7695PIPYUV_write_cmos_sensor(0x5207,0x00);
        OV7695PIPYUV_write_cmos_sensor(0x5208,0x05);
        OV7695PIPYUV_write_cmos_sensor(0x5209,0xe0);
        break;
    case AWB_MODE_FLUORESCENT:
        OV7695PIPYUV_set_AWB_mode(KAL_FALSE);
        OV7695PIPYUV_write_cmos_sensor(0x5204,0x04);
        OV7695PIPYUV_write_cmos_sensor(0x5205,0x00);
        OV7695PIPYUV_write_cmos_sensor(0x5206,0x04);
        OV7695PIPYUV_write_cmos_sensor(0x5207,0x00);
        OV7695PIPYUV_write_cmos_sensor(0x5208,0x06);
        OV7695PIPYUV_write_cmos_sensor(0x5209,0x50);
        break;
    default:
        return FALSE;
    }
    return TRUE;
} /* OV7695PIPYUV_set_param_wb */

BOOL OV7695PIPYUV_set_param_effect(UINT16 para)
{
    switch (para)
    {
    case MEFFECT_OFF:
        OV7695PIPYUV_write_cmos_sensor(0x5001,0x3f);
        OV7695PIPYUV_write_cmos_sensor(0x5800,0x06);
        OV7695PIPYUV_write_cmos_sensor(0x5803,0x28);
        OV7695PIPYUV_write_cmos_sensor(0x5804,0x20);
        break;
    case MEFFECT_SEPIA:
        OV7695PIPYUV_write_cmos_sensor(0x5001,0x3f);
        OV7695PIPYUV_write_cmos_sensor(0x5800,0x1e);
        OV7695PIPYUV_write_cmos_sensor(0x5803,0x40);
        OV7695PIPYUV_write_cmos_sensor(0x5804,0xa0);
        break;
    case MEFFECT_NEGATIVE:
        //OV7695PIPYUV_write_cmos_sensor(0x5001,0x3f);
        //OV7695PIPYUV_write_cmos_sensor(0x5800,0x46);
        break;
    case MEFFECT_SEPIAGREEN:
        OV7695PIPYUV_write_cmos_sensor(0x5001,0x3f);
        OV7695PIPYUV_write_cmos_sensor(0x5800,0x1e);
        OV7695PIPYUV_write_cmos_sensor(0x5803,0x60);
        OV7695PIPYUV_write_cmos_sensor(0x5804,0x60);
        break;
    case MEFFECT_SEPIABLUE:
        OV7695PIPYUV_write_cmos_sensor(0x5001,0x3f);
        OV7695PIPYUV_write_cmos_sensor(0x5800,0x1e);
        OV7695PIPYUV_write_cmos_sensor(0x5803,0xa0);
        OV7695PIPYUV_write_cmos_sensor(0x5804,0x40);
        break;
    case MEFFECT_MONO: //B&W
        OV7695PIPYUV_write_cmos_sensor(0x5001,0x3f);
        OV7695PIPYUV_write_cmos_sensor(0x5800,0x26);
        break;
    default:
        return KAL_FALSE;
    }
    return KAL_FALSE;
} /* OV7695PIPYUV_set_param_effect */

BOOL OV7695PIPYUV_set_param_banding(UINT16 para)
{
    switch (para)
    {
    case AE_FLICKER_MODE_50HZ:
        OV7695PIPYUV_write_cmos_sensor(0x5002,0x42);
        break;
    case AE_FLICKER_MODE_60HZ:
        OV7695PIPYUV_write_cmos_sensor(0x5002,0x40);
        break;
    case AE_FLICKER_MODE_AUTO:
        OV7695PIPYUV_write_cmos_sensor(0x5002,0x42);
        break;
    case AE_FLICKER_MODE_OFF:
        OV7695PIPYUV_write_cmos_sensor(0x5002,0x42);
        break;
    default:
        return FALSE;
    }
    return TRUE;
} /* OV7695PIPYUV_set_param_banding */

BOOL OV7695PIPYUV_set_param_exposure(UINT16 para)
{
    switch (para)
    {
    case AE_EV_COMP_10:
    case AE_EV_COMP_20:
        OV7695PIPYUV_write_cmos_sensor(0x3a0f, 0x78);//    ; AEC in H
        OV7695PIPYUV_write_cmos_sensor(0x3a10, 0x70);//    ; AEC in L
        OV7695PIPYUV_write_cmos_sensor(0x3a1b, 0x78);//    ; AEC out H
        OV7695PIPYUV_write_cmos_sensor(0x3a1e, 0x70);//    ; AEC out L
        OV7695PIPYUV_write_cmos_sensor(0x3a11, 0xf0);//    ; control zone H
        OV7695PIPYUV_write_cmos_sensor(0x3a1f, 0x38);//    ; control zone L
        break;
    case AE_EV_COMP_00:
        OV7695PIPYUV_write_cmos_sensor(0x3a0f, 0x58);//    ; AEC in H 60
	    OV7695PIPYUV_write_cmos_sensor(0x3a10,0x50); // ;50
	    OV7695PIPYUV_write_cmos_sensor(0x3a1b,0x60); // ;
	    OV7695PIPYUV_write_cmos_sensor(0x3a1e,0x50); // ;
	    OV7695PIPYUV_write_cmos_sensor(0x3a11,0x90); // ;
	    OV7695PIPYUV_write_cmos_sensor(0x3a1f,0x30); // ;
        break;
    case AE_EV_COMP_n10:
    case AE_EV_COMP_n20:
        OV7695PIPYUV_write_cmos_sensor(0x3a0f, 0x30);//    ; AEC in H
        OV7695PIPYUV_write_cmos_sensor(0x3a10, 0x28);//    ; AEC in L
        OV7695PIPYUV_write_cmos_sensor(0x3a1b, 0x30);//    ; AEC out H
        OV7695PIPYUV_write_cmos_sensor(0x3a1e, 0x28);//    ; AEC out L
        OV7695PIPYUV_write_cmos_sensor(0x3a11, 0x60);//    ; control zone H
        OV7695PIPYUV_write_cmos_sensor(0x3a1f, 0x14);//    ; control zone L
        break;
    default:
        return FALSE;
    }
    return TRUE;
} /* OV7695PIPYUV_set_param_exposure */

UINT32 OV7695PIPYUVYUVSensorSetting(FEATURE_ID iCmd, UINT32 iPara)
{
    OV7695PIPYUVSENSORDB("OV7695PIPYUVYUVSensorSetting:iCmd=%d,iPara=%d\n",iCmd, iPara);

    switch (iCmd) {
    case FID_SCENE_MODE:
        OV7695PIPYUVSENSORDB("Night Mode:%d\n", iPara);
        if (iPara == SCENE_MODE_OFF)
        {
        OV7695PIPYUV_night_mode(KAL_FALSE);
        }
        else if (iPara == SCENE_MODE_NIGHTSCENE)
        {
        OV7695PIPYUV_night_mode(KAL_TRUE);
        }
        break;
    case FID_AWB_MODE:
        OV7695PIPYUVSENSORDB("Set AWB Mode:%d\n", iPara);
        OV7695PIPYUV_set_param_wb(iPara);
        break;
    case FID_COLOR_EFFECT:
        OV7695PIPYUVSENSORDB("Set Color Effect:%d\n", iPara);
        OV7695PIPYUV_set_param_effect(iPara);
        break;
    case FID_AE_EV:
        OV7695PIPYUVSENSORDB("Set EV:%d\n", iPara);
        OV7695PIPYUV_set_param_exposure(iPara);
        break;
    case FID_AE_FLICKER:
        OV7695PIPYUVSENSORDB("Set Flicker:%d\n", iPara);
        OV7695PIPYUV_set_param_banding(iPara);
        break;
    case FID_AE_SCENE_MODE:
        OV7695PIPYUVSENSORDB("Set AE Mode:%d\n", iPara);
        if (iPara == AE_MODE_OFF) {
        spin_lock(&ov7695pipyuv_drv_lock);
        OV7695PIPYUV_AE_ENABLE = KAL_FALSE;
        spin_unlock(&ov7695pipyuv_drv_lock);
        }
        else {
        spin_lock(&ov7695pipyuv_drv_lock);
        OV7695PIPYUV_AE_ENABLE = KAL_TRUE;
        spin_unlock(&ov7695pipyuv_drv_lock);
        }
        OV7695PIPYUV_set_AE_mode(OV7695PIPYUV_AE_ENABLE);
        break;
    case FID_ZOOM_FACTOR:
        OV7695PIPYUVSENSORDB("FID_ZOOM_FACTOR:%d\n", iPara);
        spin_lock(&ov7695pipyuv_drv_lock);
        zoom_factor = iPara;
        spin_unlock(&ov7695pipyuv_drv_lock);
        break;
    default:
        break;
    }
    return TRUE;
}   /* OV7695PIPYUVYUVSensorSetting */

UINT32 OV7695PIPYUVYUVSetVideoMode(UINT16 u2FrameRate)
{
    if (u2FrameRate == 30)
    {
    }
    else if (u2FrameRate == 15)
    {
    }
    else
    {
    }
    return TRUE;
}
/**************************/
#if 1 //defined(MT6577)
static void OV7695PIPYUVGetEvAwbRef(UINT32 pSensorAEAWBRefStruct)
{
/*
    PSENSOR_AE_AWB_REF_STRUCT Ref = (PSENSOR_AE_AWB_REF_STRUCT)pSensorAEAWBRefStruct;
    Ref->SensorAERef.AeRefLV05Shutter=1503;
    Ref->SensorAERef.AeRefLV05Gain=496*2;
    Ref->SensorAERef.AeRefLV13Shutter=49;
    Ref->SensorAERef.AeRefLV13Gain=64*2;
    Ref->SensorAwbGainRef.AwbRefD65Rgain=188;
    Ref->SensorAwbGainRef.AwbRefD65Bgain=128;
    Ref->SensorAwbGainRef.AwbRefCWFRgain=160;
    Ref->SensorAwbGainRef.AwbRefCWFBgain=164;
*/
}

static void OV7695PIPYUVGetCurAeAwbInfo(UINT32 pSensorAEAWBCurStruct)
{
/*
    PSENSOR_AE_AWB_CUR_STRUCT Info = (PSENSOR_AE_AWB_CUR_STRUCT)pSensorAEAWBCurStruct;
    Info->SensorAECur.AeCurShutter=OV7695PIPYUVReadShutter();
    Info->SensorAECur.AeCurGain=OV7695PIPYUVReadSensorGain() * 2;
    Info->SensorAwbGainCur.AwbCurRgain=OV7695PIPYUV_read_cmos_sensor(0x504c);
    Info->SensorAwbGainCur.AwbCurBgain=OV7695PIPYUV_read_cmos_sensor(0x504e);
*/
}
#endif

UINT32 OV7695PIPYUVFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
    UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
    UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
    UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
    UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
    MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
    MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;
    UINT32 Tony_Temp1 = 0;
    UINT32 Tony_Temp2 = 0;
    Tony_Temp1 = pFeaturePara[0];
    Tony_Temp2 = pFeaturePara[1];

    OV7695PIPYUVSENSORDB("[OV7695PIPYUV][OV7695PIPYUVFeatureControl]feature id=%d \n",FeatureId);

    switch (FeatureId)
    {
    case SENSOR_FEATURE_GET_RESOLUTION:
        //OV7695PIPYUVSENSORDB("OV7695PIPYUVFeatureControl:SENSOR_FEATURE_GET_RESOLUTION \n");
        *pFeatureReturnPara16++=OV7695PIPYUV_IMAGE_SENSOR_VGA_WITDH;
        *pFeatureReturnPara16=OV7695PIPYUV_IMAGE_SENSOR_VGA_HEIGHT;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_GET_PERIOD:
#if 1 //defined(MT6577)
        switch(CurrentScenarioId)
        {
        case MSDK_SCENARIO_ID_CAMERA_ZSD:
            //OV7695PIPYUVSENSORDB("OV7695PIPYUVFeatureControl:SENSOR_FEATURE_GET_PERIOD \n");
            *pFeatureReturnPara16++=OV7695PIPYUV_FULL_PERIOD_PIXEL_NUMS + OV7695PIPYUVSensor.CaptureDummyPixels;
            *pFeatureReturnPara16=OV7695PIPYUV_FULL_PERIOD_LINE_NUMS + OV7695PIPYUVSensor.CaptureDummyLines;
            *pFeatureParaLen=4;
            break;
        default:
            //OV7695PIPYUVSENSORDB("OV7695PIPYUVFeatureControl:SENSOR_FEATURE_GET_PERIOD \n");
            *pFeatureReturnPara16++=OV7695PIPYUV_PV_PERIOD_PIXEL_NUMS + OV7695PIPYUVSensor.PreviewDummyPixels;
            *pFeatureReturnPara16=OV7695PIPYUV_PV_PERIOD_LINE_NUMS + OV7695PIPYUVSensor.PreviewDummyLines;
            *pFeatureParaLen=4;
            break;
        }
#else
        //OV7695PIPYUVSENSORDB("OV7695PIPYUVFeatureControl:SENSOR_FEATURE_GET_PERIOD \n");
        *pFeatureReturnPara16++=OV7695PIPYUV_PV_PERIOD_PIXEL_NUMS + OV7695PIPYUVSensor.PreviewDummyPixels;
        *pFeatureReturnPara16=OV7695PIPYUV_PV_PERIOD_LINE_NUMS + OV7695PIPYUVSensor.PreviewDummyLines;
        *pFeatureParaLen=4;
#endif
        break;
    case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
#if 1 //defined(MT6577)
        switch(CurrentScenarioId)
        {
        case MSDK_SCENARIO_ID_CAMERA_ZSD:
            //OV7695PIPYUVSENSORDB("OV7695PIPYUVFeatureControl:SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ \n");
            *pFeatureReturnPara32 = OV7695PIPYUVSensor.CapturePclk * 1000 *100;     //unit: Hz
            *pFeatureParaLen=4;
            break;
        default:
            //OV7695PIPYUVSENSORDB("OV7695PIPYUVFeatureControl:SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ \n");
            *pFeatureReturnPara32 = OV7695PIPYUVSensor.PreviewPclk * 1000 *100;     //unit: Hz
            *pFeatureParaLen=4;
            break;
        }
#else
        //OV7695PIPYUVSENSORDB("OV7695PIPYUVFeatureControl:SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ \n");
        *pFeatureReturnPara32 = OV7695PIPYUVSensor.PreviewPclk * 1000 *100;     //unit: Hz
        *pFeatureParaLen=4;
#endif
        break;
    case SENSOR_FEATURE_SET_ESHUTTER:
        //OV7695PIPYUVSENSORDB("OV7695PIPYUVFeatureControl:SENSOR_FEATURE_SET_ESHUTTER \n");
        break;
    case SENSOR_FEATURE_SET_NIGHTMODE:
        //OV7695PIPYUVSENSORDB("OV7695PIPYUVFeatureControl:SENSOR_FEATURE_SET_NIGHTMODE \n");
        OV7695PIPYUV_night_mode((BOOL) *pFeatureData16);
        break;
    case SENSOR_FEATURE_SET_GAIN:
        //OV7695PIPYUVSENSORDB("OV7695PIPYUVFeatureControl:SENSOR_FEATURE_SET_GAIN \n");
        break;
    case SENSOR_FEATURE_SET_FLASHLIGHT:
        //OV7695PIPYUVSENSORDB("OV7695PIPYUVFeatureControl:SENSOR_FEATURE_SET_FLASHLIGHT \n");
        break;
    case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
        //OV7695PIPYUVSENSORDB("OV7695PIPYUVFeatureControl:SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ \n");
        break;
    case SENSOR_FEATURE_SET_REGISTER:
        //OV7695PIPYUVSENSORDB("OV7695PIPYUVFeatureControl:SENSOR_FEATURE_SET_REGISTER \n");
        OV7695PIPYUV_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
        break;
    case SENSOR_FEATURE_GET_REGISTER:
        //OV7695PIPYUVSENSORDB("OV7695PIPYUVFeatureControl:SENSOR_FEATURE_GET_REGISTER \n");
        pSensorRegData->RegData = OV7695PIPYUV_read_cmos_sensor(pSensorRegData->RegAddr);
        break;
    case SENSOR_FEATURE_GET_CONFIG_PARA:
        //OV7695PIPYUVSENSORDB("OV7695PIPYUVFeatureControl:SENSOR_FEATURE_GET_CONFIG_PARA \n");
        memcpy(pSensorConfigData, &OV7695PIPYUVSensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
        *pFeatureParaLen=sizeof(MSDK_SENSOR_CONFIG_STRUCT);
        break;
    case SENSOR_FEATURE_SET_CCT_REGISTER:
    case SENSOR_FEATURE_GET_CCT_REGISTER:
    case SENSOR_FEATURE_SET_ENG_REGISTER:
    case SENSOR_FEATURE_GET_ENG_REGISTER:
    case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
    case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
    case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
    case SENSOR_FEATURE_GET_GROUP_INFO:
    case SENSOR_FEATURE_GET_ITEM_INFO:
    case SENSOR_FEATURE_SET_ITEM_INFO:
    case SENSOR_FEATURE_GET_ENG_INFO:
        //OV7695PIPYUVSENSORDB("OV7695PIPYUVFeatureControl:SENSOR_FEATURE_SET/get_CCT_xxxx ect \n");
        break;
    case SENSOR_FEATURE_GET_GROUP_COUNT:
        *pFeatureReturnPara32++=0;
        *pFeatureParaLen=4;
        //OV7695PIPYUVSENSORDB("OV7695PIPYUVFeatureControl:SENSOR_FEATURE_GET_GROUP_COUNT \n");
        break;
    case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
        //OV7695PIPYUVSENSORDB("OV7695PIPYUVFeatureControl:SENSOR_FEATURE_GET_LENS_DRIVER_ID \n");
        // get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
        // if EEPROM does not exist in camera module.
        *pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_CHECK_SENSOR_ID:
        //OV7695PIPYUVSENSORDB("OV7695PIPYUVFeatureControl:SENSOR_FEATURE_CHECK_SENSOR_ID \n");
        OV7695PIPYUV_GetSensorID(pFeatureData32);
        break;
    case SENSOR_FEATURE_SET_YUV_CMD:
        //OV7695PIPYUVSENSORDB("OV7695PIPYUVFeatureControl:SENSOR_FEATURE_SET_YUV_CMD \n");
        OV7695PIPYUVYUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));
        break;
    case SENSOR_FEATURE_SET_VIDEO_MODE:
        //OV7695PIPYUVSENSORDB("OV7695PIPYUVFeatureControl:SENSOR_FEATURE_SET_VIDEO_MODE \n");
        OV7695PIPYUVYUVSetVideoMode(*pFeatureData16);
        break;
#if 1 //defined(MT6577)
    case SENSOR_FEATURE_GET_EV_AWB_REF:
        OV7695PIPYUVGetEvAwbRef(*pFeatureData32);
        break;
    case SENSOR_FEATURE_GET_SHUTTER_GAIN_AWB_GAIN:
        OV7695PIPYUVGetCurAeAwbInfo(*pFeatureData32);
        break;
#endif
    default:
        OV7695PIPYUVSENSORDB("OV7695PIPYUVFeatureControl:default \n");
        break;
    }
    //OV7695PIPYUVSENSORDB("[OV7695PIPYUV]OV7695PIPYUVFeatureControl exit :\n ");
    return ERROR_NONE;
    }    /* OV7695PIPYUVFeatureControl() */


SENSOR_FUNCTION_STRUCT    SensorFuncOV7695PIPYUV=
{
    OV7695PIPYUVOpen,
    OV7695PIPYUVGetInfo,
    OV7695PIPYUVGetResolution,
    OV7695PIPYUVFeatureControl,
    OV7695PIPYUVControl,
    OV7695PIPYUVClose
};

UINT32 OV7695PIPYUV_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
    /* To Do : Check Sensor status here */
    if (pfFunc!=NULL)
        *pfFunc=&SensorFuncOV7695PIPYUV;
    return ERROR_NONE;
}    /* SensorInit() */


