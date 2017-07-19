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

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   sensor.c
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *   Source code of Sensor driver
 *
 *
 * Author:
 * -------
 *   PC Huang (MTK02204)
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * 10 13 2010 sean.cheng
 * [ALPS00021684] [Need Patch] [Volunteer Patch] CCT new feature
 * .
 *
 * 09 10 2010 jackie.su
 * [ALPS00002279] [Need Patch] [Volunteer Patch] ALPS.Wxx.xx Volunteer patch for
 * .10y dual sensor
 *
 * 09 02 2010 jackie.su
 * [ALPS00002279] [Need Patch] [Volunteer Patch] ALPS.Wxx.xx Volunteer patch for
 * .roll back dual sensor
 *
 * 07 27 2010 sean.cheng
 * [ALPS00003112] [Need Patch] [Volunteer Patch] ISP/Sensor driver modification for Customer support
 * .1. add master clock switcher
 *  2. add master enable/disable
 *  3. add dummy line/pixel for sensor
 *  4. add sensor driving current setting
 *
 * 07 01 2010 sean.cheng
 * [ALPS00121215][Camera] Change color when switch low and high
 * .Add video delay frame.
 *
 * 07 01 2010 sean.cheng
 * [ALPS00002805][Need Patch] [Volunteer Patch]10X Patch for DS269 Video Frame Rate
 * .Change the sensor clock to let frame rate to be 30fps in vidoe mode
 *
 * 06 13 2010 sean.cheng
 * [ALPS00002514][Need Patch] [Volunteer Patch] ALPS.10X.W10.11 Volunteer patch for E1k Camera
 * .
 * 1. Add set zoom factor and capdelay frame for YUV sensor
 * 2. Modify e1k sensor setting
 *
 * 05 25 2010 sean.cheng
 * [ALPS00002250][Need Patch] [Volunteer Patch] ALPS.10X.W10.11 Volunteer patch for YUV video frame rate
 * .
 * Add 15fps option for video mode
 *
 * 05 03 2010 sean.cheng
 * [ALPS00001357][Meta]CameraTool
 * .
 * Fix OV7695 YUV sensor frame rate to 30fps in vidoe mode
 *
 * Mar 4 2010 mtk70508
 * [DUMA00154792] Sensor driver
 *
 *
 * Mar 4 2010 mtk70508
 * [DUMA00154792] Sensor driver
 *
 *
 * Mar 1 2010 mtk01118
 * [DUMA00025869] [Camera][YUV I/F & Query feature] check in camera code
 *
 *
 * Feb 24 2010 mtk01118
 * [DUMA00025869] [Camera][YUV I/F & Query feature] check in camera code
 *
 *
 * Nov 24 2009 mtk02204
 * [DUMA00015869] [Camera Driver] Modifiy camera related drivers for dual/backup sensor/lens drivers.
 *
 *
 * Oct 29 2009 mtk02204
 * [DUMA00015869] [Camera Driver] Modifiy camera related drivers for dual/backup sensor/lens drivers.
 *
 *
 * Oct 27 2009 mtk02204
 * [DUMA00015869] [Camera Driver] Modifiy camera related drivers for dual/backup sensor/lens drivers.
 *
 *
 * Aug 13 2009 mtk01051
 * [DUMA00009217] [Camera Driver] CCAP First Check In
 *
 *
 * Aug 5 2009 mtk01051
 * [DUMA00009217] [Camera Driver] CCAP First Check In
 *
 *
 * Jul 17 2009 mtk01051
 * [DUMA00009217] [Camera Driver] CCAP First Check In
 *
 *
 * Jul 7 2009 mtk01051
 * [DUMA00008051] [Camera Driver] Add drivers for camera high ISO binning mode.
 * Add ISO query info for Sensor
 *
 * May 18 2009 mtk01051
 * [DUMA00005921] [Camera] LED Flashlight first check in
 *
 *
 * May 16 2009 mtk01051
 * [DUMA00005921] [Camera] LED Flashlight first check in
 *
 *
 * May 16 2009 mtk01051
 * [DUMA00005921] [Camera] LED Flashlight first check in
 *
 *
 * Apr 7 2009 mtk02204
 * [DUMA00004012] [Camera] Restructure and rename camera related custom folders and folder name of came
 *
 *
 * Mar 27 2009 mtk02204
 * [DUMA00002977] [CCT] First check in of MT6516 CCT drivers
 *
 *
 * Mar 25 2009 mtk02204
 * [DUMA00111570] [Camera] The system crash after some operations
 *
 *
 * Mar 20 2009 mtk02204
 * [DUMA00002977] [CCT] First check in of MT6516 CCT drivers
 *
 *
 * Mar 2 2009 mtk02204
 * [DUMA00001084] First Check in of MT6516 multimedia drivers
 *
 *
 * Feb 24 2009 mtk02204
 * [DUMA00001084] First Check in of MT6516 multimedia drivers
 *
 *
 * Dec 27 2008 MTK01813
 * DUMA_MBJ CheckIn Files
 * created by clearfsimport
 *
 * Dec 10 2008 mtk02204
 * [DUMA00001084] First Check in of MT6516 multimedia drivers
 *
 *
 * Oct 27 2008 mtk01051
 * [DUMA00000851] Camera related drivers check in
 * Modify Copyright Header
 *
 * Oct 24 2008 mtk02204
 * [DUMA00000851] Camera related drivers check in
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
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
#include "ov7695mipiyuv_Sensor.h"

#define OV7695YUV_DEBUG
#ifdef OV7695YUV_DEBUG
#define OV7695SENSORDB(fmt, arg...)    printk("[OV7695MIPI]%s: " fmt, __FUNCTION__ ,##arg)
#else
#define OV7695SENSORDB(x,...)
#endif
#ifdef SIMULATE_PIP
static void spin_lock(int *lock) {}
static void spin_unlock(int *lock) {}
int ov7695_drv_lock=1;
#else
static DEFINE_SPINLOCK(ov7695_drv_lock);
#endif
static MSDK_SCENARIO_ID_ENUM CurrentScenarioId = MSDK_SCENARIO_ID_CAMERA_PREVIEW;
extern int iReadReg(u16 a_u2Addr , u8 * a_puBuff , u16 i2cId);
extern int iWriteReg(u16 a_u2Addr , u32 a_u4Data , u32 a_u4Bytes , u16 i2cId);
#define OV7695_write_cmos_sensor(addr, para) iWriteReg((u16) addr , (u32) para ,1,OV7695_WRITE_ID)
#define OV5645_write_cmos_sensor(addr, para) iWriteReg((u16) addr , (u32) para ,1,0x78)

kal_uint16 OV7695mipi_read_cmos_sensor(kal_uint32 addr)
{
	  kal_uint16 get_byte=0;
    iReadReg((u16) addr ,(u8*)&get_byte,OV7695_WRITE_ID);
    return get_byte;
}

struct
{
	kal_uint8   Banding;
	kal_bool	  NightMode;
	kal_bool	  VideoMode;
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
	OV7695_SENSOR_MODE SensorMode;
} OV7695MipiSensor;

/* Global Valuable */
static kal_uint32 zoom_factor = 0;
static kal_uint8 OV7695_Banding_setting = AE_FLICKER_MODE_50HZ; 
static kal_bool OV7695_AWB_ENABLE = KAL_TRUE;
static kal_bool OV7695_AE_ENABLE = KAL_TRUE;
MSDK_SENSOR_CONFIG_STRUCT OV7695MipiSensorConfigData;

/*************************************************************************
* FUNCTION
*	OV7695_set_dummy
*
* DESCRIPTION
*	This function set the dummy pixels(Horizontal Blanking) & dummy lines(Vertical Blanking), it can be
*	used to adjust the frame rate or gain more time for back-end process.
*
*	IMPORTANT NOTICE: the base shutter need re-calculate for some sensor, or else flicker may occur.
*
* PARAMETERS
*	1. kal_uint32 : Dummy Pixels (Horizontal Blanking)
*	2. kal_uint32 : Dummy Lines (Vertical Blanking)
*
* RETURNS
*	None
*
*************************************************************************/

void OV7695MIPISetDummy(kal_uint32 dummy_pixels, kal_uint32 dummy_lines, kal_uint8 IsPVmode)
{
        if (/*OV7695MipiSensor.*/ IsPVmode)
        {
            dummy_pixels = dummy_pixels+OV7695_PV_PERIOD_PIXEL_NUMS;
            OV7695_write_cmos_sensor(0x343,( dummy_pixels&0xFF));
            OV7695_write_cmos_sensor(0x342,(( dummy_pixels&0xFF00)>>8));

            dummy_lines= dummy_lines+OV7695_PV_PERIOD_LINE_NUMS;
            OV7695_write_cmos_sensor(0x341,(dummy_lines&0xFF));
            OV7695_write_cmos_sensor(0x340,((dummy_lines&0xFF00)>>8));
        }
        else
        {
            dummy_pixels = dummy_pixels+OV7695_FULL_PERIOD_PIXEL_NUMS;
            OV7695_write_cmos_sensor(0x343,( dummy_pixels&0xFF));
            OV7695_write_cmos_sensor(0x342,(( dummy_pixels&0xFF00)>>8));

            dummy_lines= dummy_lines+OV7695_FULL_PERIOD_LINE_NUMS;
            OV7695_write_cmos_sensor(0x341,(dummy_lines&0xFF));
            OV7695_write_cmos_sensor(0x340,((dummy_lines&0xFF00)>>8));
        }
}    /* OV7695_set_dummy */

/*************************************************************************
* FUNCTION
*	OV7695WriteShutter
*
* DESCRIPTION
*	This function used to write the shutter.
*
* PARAMETERS
*	1. kal_uint32 : The shutter want to apply to sensor.
*
* RETURNS
*	None
*
*************************************************************************/
void OV7695MIPIWriteShutter(kal_uint32 shutter, kal_uint8 IsPVmode)
{
	kal_uint32 extra_exposure_lines = 0;
	if (shutter < 1)
	{
		shutter = 1;
	}

	if (IsPVmode)
	{
		if (shutter <= OV7695_PV_EXPOSURE_LIMITATION)
		{
			extra_exposure_lines = 0;
		}
		else
		{
			extra_exposure_lines=shutter - OV7695_PV_EXPOSURE_LIMITATION;
		}

	}
	else
	{
		if (shutter <= OV7695_FULL_EXPOSURE_LIMITATION)
		{
			extra_exposure_lines = 0;
		}
		else
		{
			extra_exposure_lines = shutter - OV7695_FULL_EXPOSURE_LIMITATION;
		}

	}
	shutter*=16;
	OV7695_write_cmos_sensor(0x3502, (shutter & 0x00FF));           //AEC[7:0]
	OV7695_write_cmos_sensor(0x3501, ((shutter & 0x0FF00) >>8));  //AEC[15:8]
	OV7695_write_cmos_sensor(0x3500, ((shutter & 0xFF0000) >> 16));
}    /* OV7695_write_shutter */

/*************************************************************************
* FUNCTION
*	OV7695WriteWensorGain
*
* DESCRIPTION
*	This function used to write the sensor gain.
*
* PARAMETERS
*	1. kal_uint32 : The sensor gain want to apply to sensor.
*
* RETURNS
*	None
*
*************************************************************************/
void OV7695MIPIWriteSensorGain(kal_uint32 gain)
{
	kal_uint16 temp_reg = 0;
	if(gain > 1024)  ASSERT(0);
	temp_reg = 0;
	temp_reg=gain&0x0FF;
	OV7695_write_cmos_sensor(0x350B,temp_reg);
}  /* OV7695_write_sensor_gain */

/*************************************************************************
* FUNCTION
*	OV7695ReadShutter
*
* DESCRIPTION
*	This function read current shutter for calculate the exposure.
*
* PARAMETERS
*	None
*
* RETURNS
*	kal_uint16 : The current shutter value.
*
*************************************************************************/
kal_uint32 OV7695MIPIReadShutter(void)
{
	kal_uint16 temp_reg1, temp_reg2 ,temp_reg3;
	
	temp_reg1 = OV7695mipi_read_cmos_sensor(0x3500);    // AEC[b19~b16]
	temp_reg2 = OV7695mipi_read_cmos_sensor(0x3501);    // AEC[b15~b8]
	temp_reg3 = OV7695mipi_read_cmos_sensor(0x3502);    // AEC[b7~b0]
	spin_lock(&ov7695_drv_lock);
	OV7695MipiSensor.PreviewShutter  = (temp_reg1 <<12)| (temp_reg2<<4)|(temp_reg3>>4);
	spin_unlock(&ov7695_drv_lock);
	return OV7695MipiSensor.PreviewShutter;
}    /* OV7695_read_shutter */

/*************************************************************************
* FUNCTION
*	OV7695ReadSensorGain
*
* DESCRIPTION
*	This function read current sensor gain for calculate the exposure.
*
* PARAMETERS
*	None
*
* RETURNS
*	kal_uint16 : The current sensor gain value.
*
*************************************************************************/
kal_uint32 OV7695MIPIReadSensorGain(void)
{
	kal_uint32 sensor_gain = 0;
	sensor_gain=(OV7695mipi_read_cmos_sensor(0x350B)&0xFF);
	return sensor_gain;
}  /* OV7695ReadSensorGain */

/*************************************************************************
* FUNCTION
*	OV7695mipi_set_AE_mode
*
* DESCRIPTION
*	This function read current sensor gain for calculate the exposure.
*
* PARAMETERS
*	None
*
* RETURNS
*	kal_uint16 : The current sensor gain value.
*
*************************************************************************/
void OV7695mipi_set_AE_mode(kal_bool AE_enable)
{
    kal_uint8 AeTemp;
	  AeTemp = OV7695mipi_read_cmos_sensor(0x3503);
    if (AE_enable == KAL_TRUE)
    {
        OV7695_write_cmos_sensor(0x3503,(AeTemp&(~0x03)));
    }
    else
    {
      OV7695_write_cmos_sensor(0x3503,(AeTemp|0x03));
    }
}
/*************************************************************************
* FUNCTION
*	OV7695mipi_set_AWB_mode
*
* DESCRIPTION
*	This function read current sensor gain for calculate the exposure.
*
* PARAMETERS
*	None
*
* RETURNS
*	kal_uint16 : The current sensor gain value.
*
*************************************************************************/
void OV7695mipi_set_AWB_mode(kal_bool AWB_enable)
{
    kal_uint8 AwbTemp;
	  AwbTemp = OV7695mipi_read_cmos_sensor(0x5000);
    if (AWB_enable == KAL_TRUE)
    {
		OV7695_write_cmos_sensor(0x5000,AwbTemp|0x20);  
		OV7695_write_cmos_sensor(0x5200,0x00);
    }
    else
    {
		OV7695_write_cmos_sensor(0x5000,AwbTemp&0xDF);
		OV7695_write_cmos_sensor(0x5200,0x20);
    }
}

/*************************************************************************
* FUNCTION
*	OV7695mipi_night_mode
*
* DESCRIPTION
*	This function night mode of OV7695.
*
* PARAMETERS
*	none
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void OV7695mipi_night_mode(kal_bool enable)
{
	if (enable)
	{
		OV7695_write_cmos_sensor(0x3a00, 0x78); 
		OV7695_write_cmos_sensor(0x3a02, 0x02); 
		OV7695_write_cmos_sensor(0x3a03, 0x18); 
		OV7695_write_cmos_sensor(0x3a14, 0x02); 
		OV7695_write_cmos_sensor(0x3a15, 0x18); 
  }
	else
	{      
		 OV7695_write_cmos_sensor(0x3a00, 0x78); 
		 OV7695_write_cmos_sensor(0x3a02, 0x02); 
		 OV7695_write_cmos_sensor(0x3a03, 0x18); 
		 OV7695_write_cmos_sensor(0x3a14, 0x02); 
		 OV7695_write_cmos_sensor(0x3a15, 0x18); 
  }
}	/* OV7695mipi_night_mode */
/*************************************************************************
* FUNCTION
*	OV7695mipi_GetSensorID
*
* DESCRIPTION
*	This function get the sensor ID
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
kal_uint32 OV7695mipi_GetSensorID(kal_uint32 *sensorID)
{
	  volatile signed char i;
		kal_uint32 sensor_id=0;
		OV7695SENSORDB("OV7695 READ ID Enter");
		
		OV7695_write_cmos_sensor(0x0103,0x01);// Reset sensor
		mDELAY(10);
		for(i=0;i<3;i++)
		{
			sensor_id = (OV7695mipi_read_cmos_sensor(0x300A) << 8) | OV7695mipi_read_cmos_sensor(0x300B);
			OV7695SENSORDB("OV7695 READ ID: %x",sensor_id);
			if(sensor_id != OV7695_SENSOR_ID)
			{
				*sensorID =0xffffffff;
			}
			else 
			{
				kdCamADCReadChipID(1, 0, 0);
				//*sensorID =OV7695_SENSOR_ID;
				*sensorID =0xffffffff;
				break;
			}
		}
    return ERROR_NONE;
}
/*************************************************************************
* FUNCTION
*    OV7695mipi_InitialSetting
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
void OV7695mipi_InitialSetting(void)
{
	OV5645_write_cmos_sensor(0x3103, 0x11);
	OV5645_write_cmos_sensor(0x3008, 0x82);
	mDELAY(5); 
	OV5645_write_cmos_sensor(0x3008, 0x42);
	OV5645_write_cmos_sensor(0x3103, 0x3 );
	OV5645_write_cmos_sensor(0x3503, 0x7 );
	OV5645_write_cmos_sensor(0x3002, 0x1c);
	OV5645_write_cmos_sensor(0x3006, 0xc3);
	OV5645_write_cmos_sensor(0x300e, 0x25);
	OV5645_write_cmos_sensor(0x3017, 0x40);
	OV5645_write_cmos_sensor(0x3018, 0x0 );
	OV5645_write_cmos_sensor(0x302e, 0xb );
	OV5645_write_cmos_sensor(0x3037, 0x13);
	OV5645_write_cmos_sensor(0x3108, 0x1 );
	OV5645_write_cmos_sensor(0x3611, 0x6 );
	OV5645_write_cmos_sensor(0x3612, 0xab);
	OV5645_write_cmos_sensor(0x3614, 0x50);
	OV5645_write_cmos_sensor(0x3618, 0x0 );
	OV5645_write_cmos_sensor(0x3034, 0x18);
	OV5645_write_cmos_sensor(0x3035, 0x11);
	OV5645_write_cmos_sensor(0x3036, 0x54);
	OV5645_write_cmos_sensor(0x3500, 0x0 );
	OV5645_write_cmos_sensor(0x3501, 0x1 );
	OV5645_write_cmos_sensor(0x3502, 0x0 );
	OV5645_write_cmos_sensor(0x350a, 0x0 );
	OV5645_write_cmos_sensor(0x350b, 0x3f);
	OV5645_write_cmos_sensor(0x3600, 0x9 );
	OV5645_write_cmos_sensor(0x3601, 0x43);
	OV5645_write_cmos_sensor(0x3620, 0x33);
	OV5645_write_cmos_sensor(0x3621, 0xe0);
	OV5645_write_cmos_sensor(0x3622, 0x1 );
	OV5645_write_cmos_sensor(0x3630, 0x2d);
	OV5645_write_cmos_sensor(0x3631, 0x0 );
	OV5645_write_cmos_sensor(0x3632, 0x32);
	OV5645_write_cmos_sensor(0x3633, 0x52);
	OV5645_write_cmos_sensor(0x3634, 0x70);
	OV5645_write_cmos_sensor(0x3635, 0x13);
	OV5645_write_cmos_sensor(0x3636, 0x3 );
	OV5645_write_cmos_sensor(0x3703, 0x5a);
	OV5645_write_cmos_sensor(0x3704, 0xa0);
	OV5645_write_cmos_sensor(0x3705, 0x1a);
	OV5645_write_cmos_sensor(0x3708, 0x66);
	OV5645_write_cmos_sensor(0x3709, 0x52);
	OV5645_write_cmos_sensor(0x370b, 0x61);
	OV5645_write_cmos_sensor(0x370c, 0xc3);
	OV5645_write_cmos_sensor(0x370f, 0x10);
	OV5645_write_cmos_sensor(0x3715, 0x78);
	OV5645_write_cmos_sensor(0x3717, 0x1 );
	OV5645_write_cmos_sensor(0x371b, 0x20);
	OV5645_write_cmos_sensor(0x3731, 0x12);
	OV5645_write_cmos_sensor(0x3901, 0xa );
	OV5645_write_cmos_sensor(0x3905, 0x2 );
	OV5645_write_cmos_sensor(0x3906, 0x10);
	OV5645_write_cmos_sensor(0x3719, 0x86);
	OV5645_write_cmos_sensor(0x3800, 0x0 );
	OV5645_write_cmos_sensor(0x3801, 0x0 );
	OV5645_write_cmos_sensor(0x3802, 0x0 );
	OV5645_write_cmos_sensor(0x3803, 0xa );
	OV5645_write_cmos_sensor(0x3804, 0xa );
	OV5645_write_cmos_sensor(0x3805, 0x3f);
	OV5645_write_cmos_sensor(0x3806, 0x7 );
	OV5645_write_cmos_sensor(0x3807, 0x99);
	OV5645_write_cmos_sensor(0x3808, 0x2 );
	OV5645_write_cmos_sensor(0x3809, 0x80);
	OV5645_write_cmos_sensor(0x380a, 0x1 );
	OV5645_write_cmos_sensor(0x380b, 0xe0);
	OV5645_write_cmos_sensor(0x380c, 0x7 );
	OV5645_write_cmos_sensor(0x380d, 0x58);
	OV5645_write_cmos_sensor(0x380e, 0x1 );
	OV5645_write_cmos_sensor(0x380f, 0xf0);
	OV5645_write_cmos_sensor(0x3810, 0x0 );
	OV5645_write_cmos_sensor(0x3811, 0x8 );
	OV5645_write_cmos_sensor(0x3812, 0x0 );
	OV5645_write_cmos_sensor(0x3813, 0x2 );
	OV5645_write_cmos_sensor(0x3814, 0x71);
	OV5645_write_cmos_sensor(0x3815, 0x35);
	OV5645_write_cmos_sensor(0x3820, 0x47);
	OV5645_write_cmos_sensor(0x3821, 0x1 );
	OV5645_write_cmos_sensor(0x3824, 0x1 );
	OV5645_write_cmos_sensor(0x3826, 0x3 );
	OV5645_write_cmos_sensor(0x3828, 0x8 );
	OV5645_write_cmos_sensor(0x3a02, 0x1 );
	OV5645_write_cmos_sensor(0x3a03, 0xf0);
	OV5645_write_cmos_sensor(0x3a08, 0x1 );
	OV5645_write_cmos_sensor(0x3a09, 0xbe);
	OV5645_write_cmos_sensor(0x3a0a, 0x1 );
	OV5645_write_cmos_sensor(0x3a0b, 0x74);
	OV5645_write_cmos_sensor(0x3a0e, 0x1 );
	OV5645_write_cmos_sensor(0x3a0d, 0x1 );
	OV5645_write_cmos_sensor(0x3a14, 0x1 );
	OV5645_write_cmos_sensor(0x3a15, 0xf0);
	OV5645_write_cmos_sensor(0x4300, 0x30);
	OV5645_write_cmos_sensor(0x4514, 0x0 );
	OV5645_write_cmos_sensor(0x4520, 0xb0);
	OV5645_write_cmos_sensor(0x460b, 0x37);
	OV5645_write_cmos_sensor(0x460c, 0x20);
	OV5645_write_cmos_sensor(0x4818, 0x1 );
	OV5645_write_cmos_sensor(0x481d, 0xf0);
	OV5645_write_cmos_sensor(0x481f, 0x50);
	OV5645_write_cmos_sensor(0x4837, 0x16);
	OV5645_write_cmos_sensor(0x4823, 0x70);
	OV5645_write_cmos_sensor(0x4831, 0x14);
	OV5645_write_cmos_sensor(0x5000, 0xa7);
	OV5645_write_cmos_sensor(0x5001, 0x83);
	OV5645_write_cmos_sensor(0x501d, 0x0 );
	OV5645_write_cmos_sensor(0x501f, 0x0 );
	OV5645_write_cmos_sensor(0x503d, 0x0 );
	OV5645_write_cmos_sensor(0x505c, 0x30);
	OV5645_write_cmos_sensor(0x5684, 0x10);
	OV5645_write_cmos_sensor(0x5685, 0xa0);
	OV5645_write_cmos_sensor(0x5686, 0xc );
	OV5645_write_cmos_sensor(0x5687, 0x78);
	OV5645_write_cmos_sensor(0x5a00, 0x8 );
	OV5645_write_cmos_sensor(0x5a21, 0x0 );
	OV5645_write_cmos_sensor(0x5a24, 0x0 );
	OV5645_write_cmos_sensor(0x3503, 0x0 );
	OV5645_write_cmos_sensor(0x380c, 0x8 );
	OV5645_write_cmos_sensor(0x380d, 0x70);
	OV5645_write_cmos_sensor(0x3017, 0x40);
	OV5645_write_cmos_sensor(0x3036, 0x60);
	OV5645_write_cmos_sensor(0x3035, 0x11);
	OV5645_write_cmos_sensor(0x3108, 0x6 );
	OV5645_write_cmos_sensor(0x5002, 0x81);
	OV5645_write_cmos_sensor(0x4b00, 0x45);
	OV5645_write_cmos_sensor(0x4b04, 0x12);
	OV5645_write_cmos_sensor(0x4315, 0x90);
	OV5645_write_cmos_sensor(0x5b00, 0x1 );
	OV5645_write_cmos_sensor(0x5b01, 0x40);
	OV5645_write_cmos_sensor(0x5b02, 0x0 );
	OV5645_write_cmos_sensor(0x5b03, 0xf0);
	OV5645_write_cmos_sensor(0x4314, 0xf );
	OV5645_write_cmos_sensor(0x4315, 0x99);
	OV5645_write_cmos_sensor(0x4316, 0x54);
	OV5645_write_cmos_sensor(0x5000, 0xa7);
	OV5645_write_cmos_sensor(0x5001, 0x83);
	OV5645_write_cmos_sensor(0x3622, 0x1 );
	OV5645_write_cmos_sensor(0x3635, 0x1c);
	OV5645_write_cmos_sensor(0x3634, 0x40);
	OV5645_write_cmos_sensor(0x5025, 0x0 );
	OV5645_write_cmos_sensor(0x380c, 0x5 );
	OV5645_write_cmos_sensor(0x380d, 0xd4);
	OV5645_write_cmos_sensor(0x3703, 0xa );
	OV5645_write_cmos_sensor(0x3035, 0x22);
	OV5645_write_cmos_sensor(0x380e, 0x2 );
	OV5645_write_cmos_sensor(0x380f, 0x18);
	OV5645_write_cmos_sensor(0x5b00, 0x2 );
	OV5645_write_cmos_sensor(0x5b01, 0x80);
	OV5645_write_cmos_sensor(0x5b02, 0x1 );
	OV5645_write_cmos_sensor(0x5b03, 0xe0);
	OV5645_write_cmos_sensor(0x5b07, 0x0 );
	OV5645_write_cmos_sensor(0x5b08, 0x0 );
	OV5645_write_cmos_sensor(0x503d, 0xc0);//power save
	OV5645_write_cmos_sensor(0x5000, 0x00);
	OV5645_write_cmos_sensor(0x5001, 0x00);
	OV5645_write_cmos_sensor(0x5002, 0x01);
	OV5645_write_cmos_sensor(0x5003, 0x00);
	OV5645_write_cmos_sensor(0x3811, 0x00);
	OV5645_write_cmos_sensor(0x3813, 0x00);
	OV5645_write_cmos_sensor(0x3032, 0x01);
	OV5645_write_cmos_sensor(0x3031, 0x30);
	OV5645_write_cmos_sensor(0x4000, 0x00);
	OV5645_write_cmos_sensor(0x3000, 0x30);
	OV5645_write_cmos_sensor(0x4b02, 0x02);//PIP
	OV5645_write_cmos_sensor(0x4b03, 0x5c);
	OV5645_write_cmos_sensor(0x4b00, 0x55);
	OV5645_write_cmos_sensor(0x4314, 0x17);
	OV5645_write_cmos_sensor(0x4315, 0xf8);
	OV5645_write_cmos_sensor(0x4316, 0x1a);
	OV5645_write_cmos_sensor(0x3008, 0x02);
}
/*************************************************************************
* FUNCTION
*    OV7695InitialSetting
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
static void OV7695InitialSetting(void)
{
	//OV7695 2lane SPI VGA
	OV7695_write_cmos_sensor(0x0103, 0x01); //	; software reset
	mDELAY(5);
	OV7695_write_cmos_sensor(0x3620, 0x28);
  OV7695_write_cmos_sensor(0x3623, 0x12);
  OV7695_write_cmos_sensor(0x3632, 0x5 );
  OV7695_write_cmos_sensor(0x3718, 0x88);
  OV7695_write_cmos_sensor(0x3717, 0x19);
  OV7695_write_cmos_sensor(0x3013, 0xd0);
  OV7695_write_cmos_sensor(0x4803, 0x8 );
  OV7695_write_cmos_sensor(0x0101, 0x3 );
  OV7695_write_cmos_sensor(0x4008, 0x2 );
  OV7695_write_cmos_sensor(0x4009, 0x9 );
  OV7695_write_cmos_sensor(0x4300, 0x30);
  OV7695_write_cmos_sensor(0x3820, 0x94);
  OV7695_write_cmos_sensor(0x4f05, 0x1 );
  OV7695_write_cmos_sensor(0x4f06, 0x2 );
  OV7695_write_cmos_sensor(0x3012, 0x0 );
  OV7695_write_cmos_sensor(0x3005, 0xf );
  OV7695_write_cmos_sensor(0x3001, 0x1f);
  OV7695_write_cmos_sensor(0x3823, 0x30);
  OV7695_write_cmos_sensor(0x4303, 0xee);
  OV7695_write_cmos_sensor(0x4307, 0xee);
  OV7695_write_cmos_sensor(0x430b, 0xee);
  OV7695_write_cmos_sensor(0x3620, 0x2f);
  OV7695_write_cmos_sensor(0x3621, 0x77);
  OV7695_write_cmos_sensor(0x5000, 0xbf);
  OV7695_write_cmos_sensor(0x3826, 0x0 );
  OV7695_write_cmos_sensor(0x3827, 0x0 );
  OV7695_write_cmos_sensor(0x0100, 0x1 );
  OV7695_write_cmos_sensor(0x0340, 0x2 );
  OV7695_write_cmos_sensor(0x0341, 0x18);
  OV7695_write_cmos_sensor(0x0342, 0x2 );
  OV7695_write_cmos_sensor(0x0343, 0xea);
  OV7695_write_cmos_sensor(0x3014, 0x30);
  OV7695_write_cmos_sensor(0x301a, 0x34);
  OV7695_write_cmos_sensor(0x3a02, 0x2 );      
  OV7695_write_cmos_sensor(0x3a03, 0x18);      
  OV7695_write_cmos_sensor(0x3a14, 0x2 );      
  OV7695_write_cmos_sensor(0x3a15, 0x18);      
  OV7695_write_cmos_sensor(0x3a08, 0x0 );      
  OV7695_write_cmos_sensor(0x3a09, 0xa1);      
  OV7695_write_cmos_sensor(0x3a0a, 0x00);      
  OV7695_write_cmos_sensor(0x3a0b, 0x86);      
  OV7695_write_cmos_sensor(0x3a0d, 0x04);      
  OV7695_write_cmos_sensor(0x3a0e, 0x03);      
  OV7695_write_cmos_sensor(0x0101, 0x01); //mirror_on
  OV7695_write_cmos_sensor(0x5002, 0x42); //[7:6] Y source select
  OV7695_write_cmos_sensor(0x5910, 0x00); //Y formula
  OV7695_write_cmos_sensor(0x3a0f, 0x58); 
  OV7695_write_cmos_sensor(0x3a10, 0x50); //AEC target 
  OV7695_write_cmos_sensor(0x3a1b, 0x5a);  
  OV7695_write_cmos_sensor(0x3a1e, 0x4e);  
  OV7695_write_cmos_sensor(0x3a11, 0xb0);  
  OV7695_write_cmos_sensor(0x3a1f, 0x28);  
  OV7695_write_cmos_sensor(0x3a18, 0x00);
  OV7695_write_cmos_sensor(0x3a19, 0xf8); //max gain 15.5x
  OV7695_write_cmos_sensor(0x3503, 0x00); //aec/agc
  OV7695_write_cmos_sensor(0x5200, 0x20);
  OV7695_write_cmos_sensor(0x5204, 0x05);
  OV7695_write_cmos_sensor(0x5205, 0x80);
  OV7695_write_cmos_sensor(0x5206, 0x04);
  OV7695_write_cmos_sensor(0x5207, 0x00);
  OV7695_write_cmos_sensor(0x5208, 0x06);
  OV7695_write_cmos_sensor(0x5209, 0xc0);
  OV7695_write_cmos_sensor(0x5000, 0xff); //lcd,gma,awb,awbg,bc,wc,lenc,isp
  OV7695_write_cmos_sensor(0x5001, 0x3f); //avg, blc,sde,uv_avg,cmx, cip
  OV7695_write_cmos_sensor(0x5100, 0x01); //lens 
  OV7695_write_cmos_sensor(0x5101, 0xc0); 
  OV7695_write_cmos_sensor(0x5102, 0x01); 
  OV7695_write_cmos_sensor(0x5103, 0x00); 
  OV7695_write_cmos_sensor(0x5104, 0x4b); 
  OV7695_write_cmos_sensor(0x5105, 0x04); 
  OV7695_write_cmos_sensor(0x5106, 0xc2);
  OV7695_write_cmos_sensor(0x5107, 0x08);
  OV7695_write_cmos_sensor(0x5108, 0x01);
  OV7695_write_cmos_sensor(0x5109, 0xd0); 
  OV7695_write_cmos_sensor(0x510a, 0x01); 
  OV7695_write_cmos_sensor(0x510b, 0x00); 
  OV7695_write_cmos_sensor(0x510c, 0x30); 
  OV7695_write_cmos_sensor(0x510d, 0x04); 
  OV7695_write_cmos_sensor(0x510e, 0xc2);
  OV7695_write_cmos_sensor(0x510f, 0x08);
  OV7695_write_cmos_sensor(0x5110, 0x01);
  OV7695_write_cmos_sensor(0x5111, 0xd0); 
  OV7695_write_cmos_sensor(0x5112, 0x01); 
  OV7695_write_cmos_sensor(0x5113, 0x00); 
  OV7695_write_cmos_sensor(0x5114, 0x26); 
  OV7695_write_cmos_sensor(0x5115, 0x04); 
  OV7695_write_cmos_sensor(0x5116, 0xc2);
  OV7695_write_cmos_sensor(0x5117, 0x08);
  OV7695_write_cmos_sensor(0x520a, 0x74);//AWB
  OV7695_write_cmos_sensor(0x520b, 0x64);
  OV7695_write_cmos_sensor(0x520c, 0xd4);
  OV7695_write_cmos_sensor(0x5301, 0x05);//gamma
  OV7695_write_cmos_sensor(0x5302, 0x0c);
  OV7695_write_cmos_sensor(0x5303, 0x1c);
  OV7695_write_cmos_sensor(0x5304, 0x2a);
  OV7695_write_cmos_sensor(0x5305, 0x39);
  OV7695_write_cmos_sensor(0x5306, 0x45);
  OV7695_write_cmos_sensor(0x5307, 0x53);
  OV7695_write_cmos_sensor(0x5308, 0x5d);
  OV7695_write_cmos_sensor(0x5309, 0x68);
  OV7695_write_cmos_sensor(0x530a, 0x7f);
  OV7695_write_cmos_sensor(0x530b, 0x91);
  OV7695_write_cmos_sensor(0x530c, 0xa5);
  OV7695_write_cmos_sensor(0x530d, 0xc6);
  OV7695_write_cmos_sensor(0x530e, 0xde);
  OV7695_write_cmos_sensor(0x530f, 0xef);
  OV7695_write_cmos_sensor(0x5310, 0x16);
  OV7695_write_cmos_sensor(0x5500, 0x08); //sharp th1 8x
  OV7695_write_cmos_sensor(0x5501, 0x48); //sharp th2 8x
  OV7695_write_cmos_sensor(0x5502, 0x18); //sharp mt offset1
  OV7695_write_cmos_sensor(0x5503, 0x04); //sharp mt offset2
  OV7695_write_cmos_sensor(0x5504, 0x08); //dns th1 8x
  OV7695_write_cmos_sensor(0x5505, 0x48); //dns th2 8x
  OV7695_write_cmos_sensor(0x5506, 0x02); //dns offset1
  OV7695_write_cmos_sensor(0x5507, 0x16); //dns offset2
  OV7695_write_cmos_sensor(0x5508, 0xad); //[6]:sharp_man [4]:dns_man
  OV7695_write_cmos_sensor(0x5509, 0x08); //sharpth th1 8x
  OV7695_write_cmos_sensor(0x550a, 0x48); //sharpth th2 8x
  OV7695_write_cmos_sensor(0x550b, 0x06); //sharpth offset1
  OV7695_write_cmos_sensor(0x550c, 0x04); //sharpth offset2
  OV7695_write_cmos_sensor(0x550d, 0x01); //recursive_en
  OV7695_write_cmos_sensor(0x5800, 0x02);//SDE
  OV7695_write_cmos_sensor(0x5803, 0x40);
  OV7695_write_cmos_sensor(0x5804, 0x20);
  OV7695_write_cmos_sensor(0x5600, 0x00);//ccm
  OV7695_write_cmos_sensor(0x5601, 0x2c);
  OV7695_write_cmos_sensor(0x5602, 0x5a);
  OV7695_write_cmos_sensor(0x5603, 0x06);
  OV7695_write_cmos_sensor(0x5604, 0x1c);
  OV7695_write_cmos_sensor(0x5605, 0x65);
  OV7695_write_cmos_sensor(0x5606, 0x81);
  OV7695_write_cmos_sensor(0x5607, 0x9f);
  OV7695_write_cmos_sensor(0x5608, 0x8a);
  OV7695_write_cmos_sensor(0x5609, 0x15);
  OV7695_write_cmos_sensor(0x560a, 0x01);
  OV7695_write_cmos_sensor(0x560b, 0x9c);

	spin_lock(&ov7695_drv_lock);
	OV7695MipiSensor.IsPVmode= 1;
	OV7695MipiSensor.PreviewDummyPixels= 0;
	OV7695MipiSensor.PreviewDummyLines= 0;
	OV7695MipiSensor.PreviewPclk= 120;
	OV7695MipiSensor.CapturePclk= 120;
	OV7695MipiSensor.PreviewShutter=0x0000; 
	OV7695MipiSensor.SensorGain=0x10;
	spin_unlock(&ov7695_drv_lock);
}
/*************************************************************************
* FUNCTION
*    OV7695SetHVMirror
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
static void OV7695SetHVMirror(kal_uint8 Mirror)
{
  kal_uint8 mirror= 0, flip=0;

  flip = OV7695mipi_read_cmos_sensor(0x3820);
	mirror=OV7695mipi_read_cmos_sensor(0x3821);

	switch (Mirror)
	{
	case IMAGE_NORMAL:
		OV7695_write_cmos_sensor(0x3820, flip&0xf9);
		OV7695_write_cmos_sensor(0x3821, mirror&0xf9);
		break;
	case IMAGE_H_MIRROR:
		OV7695_write_cmos_sensor(0x3820, flip&0xf9);
		OV7695_write_cmos_sensor(0x3821, mirror|0x06);
		break;
	case IMAGE_V_MIRROR:
		OV7695_write_cmos_sensor(0x3820, flip|0x06);
		OV7695_write_cmos_sensor(0x3821, mirror&0xf9);
		break;
	case IMAGE_HV_MIRROR:
		OV7695_write_cmos_sensor(0x3820, flip|0x06);
		OV7695_write_cmos_sensor(0x3821, mirror|0x06);
		break;
	default:
		ASSERT(0);
	}
}

/*************************************************************************
* FUNCTION
*	OV7695Open
*
* DESCRIPTION
*	This function initialize the registers of CMOS sensor
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV7695MIPIOpen(void)
{
	volatile signed int i;
	kal_uint16 sensor_id = 0;

	OV7695SENSORDB("enter :CurrentScenarioId=%d :\n ", CurrentScenarioId);
	OV7695mipi_GetSensorID(&sensor_id);
	if(sensor_id!=OV7695_SENSOR_ID)
		return ERROR_SENSOR_CONNECT_FAIL;
	if ( kdCamADCReadChipID(1, 450, 1350) ){
		/*PART NO:     N705-H70000-020, 
		 * PART NAME: 5M FF -270-OV5645-8.5*8.5*4.85-TRULY
		 * V-ID:          1.8V
		 * ADC:           900
		 */
		return ERROR_SENSOR_CONNECT_FAIL;
	}else{
		return ERROR_SENSOR_CONNECT_FAIL;
	}

	spin_lock(&ov7695_drv_lock);
	OV7695MipiSensor.VideoMode = KAL_FALSE;
	OV7695MipiSensor.NightMode = KAL_FALSE;
	OV7695MipiSensor.Fps = 100;
	OV7695MipiSensor.ShutterStep= 0xde;
	OV7695MipiSensor.CaptureDummyPixels = 0;
	OV7695MipiSensor.CaptureDummyLines = 0;
	OV7695MipiSensor.PreviewDummyPixels = 0;
	OV7695MipiSensor.PreviewDummyLines = 0;
	OV7695MipiSensor.SensorMode= OV7695_SENSOR_MODE_INIT;
	spin_unlock(&ov7695_drv_lock);
	
	OV7695mipi_InitialSetting();
	mDELAY(20);
	OV7695InitialSetting();
	
	OV7695SENSORDB("exit :\n ");
	return ERROR_NONE;
}	/* OV7695Open() */

/*************************************************************************
* FUNCTION
*	OV7695Close
*
* DESCRIPTION
*	This function is to turn off sensor module power.
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV7695MIPIClose(void)
{
	return ERROR_NONE;
}	/* OV7695Close() */
/*************************************************************************
* FUNCTION
*	OV7695Preview
*
* DESCRIPTION
*	This function start the sensor preview.
*
* PARAMETERS
*	*image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static UINT32 OV7695Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	OV7695SENSORDB("enter :\n ");
	mDELAY(100);
	OV7695mipi_set_AE_mode(KAL_TRUE);
	OV7695mipi_set_AWB_mode(KAL_TRUE);
  return TRUE ;//ERROR_NONE;
}	/* OV7695Preview() */

static UINT32 OV7695Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	OV7695SENSORDB("enter :\n ");
	OV7695SENSORDB("exit :\n ");
	return ERROR_NONE;
}/* OV7695Capture() */


UINT32 OV7695MIPIGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
	OV7695SENSORDB("[enter] :\n ");
	pSensorResolution->SensorPreviewWidth= OV7695_IMAGE_SENSOR_VGA_WITDH - 2 * OV7695_PV_GRAB_START_X;
	pSensorResolution->SensorPreviewHeight= OV7695_IMAGE_SENSOR_VGA_HEIGHT - 2 * OV7695_PV_GRAB_START_Y;
	pSensorResolution->SensorFullWidth= OV7695_IMAGE_SENSOR_VGA_WITDH- 2 * OV7695_FULL_GRAB_START_X; 
	pSensorResolution->SensorFullHeight= OV7695_IMAGE_SENSOR_VGA_HEIGHT- 2 * OV7695_FULL_GRAB_START_X;
	OV7695SENSORDB("[exit] :\n ");
	return ERROR_NONE;
}	/* OV7695GetResolution() */

UINT32 OV7695MIPIGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,MSDK_SENSOR_INFO_STRUCT *pSensorInfo,MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	OV7695SENSORDB("enter :\n ");
	#if defined(MT6577)
	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			pSensorInfo->SensorPreviewResolutionX=OV7695_IMAGE_SENSOR_VGA_WITDH - 2*OV7695_PV_GRAB_START_X;
			pSensorInfo->SensorPreviewResolutionY=OV7695_IMAGE_SENSOR_VGA_HEIGHT - 2*OV7695_PV_GRAB_START_Y;
			break;
		default:
			pSensorInfo->SensorPreviewResolutionX=OV7695_IMAGE_SENSOR_QVGA_WIDTH - 2*OV7695_PV_GRAB_START_X;
			pSensorInfo->SensorPreviewResolutionY=OV7695_IMAGE_SENSOR_QVGA_HEIGHT - 2*OV7695_PV_GRAB_START_Y;
			break;
	}
	#else
		  pSensorInfo->SensorPreviewResolutionX= OV7695_IMAGE_SENSOR_QVGA_WIDTH - 2*OV7695_PV_GRAB_START_X;
		  pSensorInfo->SensorPreviewResolutionY= OV7695_IMAGE_SENSOR_QVGA_HEIGHT - 2*OV7695_PV_GRAB_START_Y;
	#endif
	pSensorInfo->SensorFullResolutionX= OV7695_IMAGE_SENSOR_VGA_WITDH - 2*OV7695_FULL_GRAB_START_X;
	pSensorInfo->SensorFullResolutionY= OV7695_IMAGE_SENSOR_VGA_HEIGHT - 2*OV7695_FULL_GRAB_START_X;
	pSensorInfo->SensorCameraPreviewFrameRate=30;
	pSensorInfo->SensorVideoFrameRate=30;
	pSensorInfo->SensorStillCaptureFrameRate=30;
	pSensorInfo->SensorWebCamCaptureFrameRate=30;
	pSensorInfo->SensorResetActiveHigh=FALSE;
	pSensorInfo->SensorResetDelayCount=1;
	pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_YUYV;
	pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorInterruptDelayLines = 1;
	pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_MIPI;
	pSensorInfo->CaptureDelayFrame = 2;
	pSensorInfo->PreviewDelayFrame = 2;
	pSensorInfo->VideoDelayFrame = 4;
	pSensorInfo->SensorMasterClockSwitch = 0;
  pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_8MA;
	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
					pSensorInfo->SensorClockFreq=24;
					pSensorInfo->SensorClockDividCount=	3;
					pSensorInfo->SensorClockRisingCount= 0;
					pSensorInfo->SensorClockFallingCount= 2;
					pSensorInfo->SensorPixelClockCount= 3;
					pSensorInfo->SensorDataLatchCount= 2;
					pSensorInfo->SensorGrabStartX = OV7695_PV_GRAB_START_X;
					pSensorInfo->SensorGrabStartY = OV7695_PV_GRAB_START_Y;
					pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;			
					pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
					pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
					pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
					pSensorInfo->SensorWidthSampling = 0; 
					pSensorInfo->SensorHightSampling = 0;
			   break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		#if defined(MT6577)
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
		#endif
					pSensorInfo->SensorClockFreq=24;
					pSensorInfo->SensorClockDividCount=	3;
					pSensorInfo->SensorClockRisingCount= 0;
					pSensorInfo->SensorClockFallingCount= 2;
					pSensorInfo->SensorPixelClockCount= 3;
					pSensorInfo->SensorDataLatchCount= 2;
					pSensorInfo->SensorGrabStartX = OV7695_FULL_GRAB_START_X;
					pSensorInfo->SensorGrabStartY = OV7695_FULL_GRAB_START_Y;
					pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;			
					pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
					pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
					pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0; 
					pSensorInfo->SensorWidthSampling = 0; 
					pSensorInfo->SensorHightSampling = 0;
				 break;
		default:
					pSensorInfo->SensorClockFreq=24;
					pSensorInfo->SensorClockDividCount=3;
					pSensorInfo->SensorClockRisingCount=0;
					pSensorInfo->SensorClockFallingCount=2;
					pSensorInfo->SensorPixelClockCount=3;
					pSensorInfo->SensorDataLatchCount=2;
					pSensorInfo->SensorGrabStartX = OV7695_PV_GRAB_START_X;
					pSensorInfo->SensorGrabStartY = OV7695_PV_GRAB_START_Y;
					pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;			
					pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
					pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
					pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
					pSensorInfo->SensorWidthSampling = 0;
					pSensorInfo->SensorHightSampling = 0;
					
		     break;
	}
	memcpy(pSensorConfigData, &OV7695MipiSensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	OV7695SENSORDB("exit :\n ");
	return ERROR_NONE;
}	/* OV7695GetInfo() */

UINT32 OV7695MIPIControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	  OV7695SENSORDB("enter :\n ");
	  CurrentScenarioId = ScenarioId;
	  switch (ScenarioId)
	  {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		#if defined(MT6577)
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
		#endif
		   	 OV7695Preview(pImageWindow, pSensorConfigData);
			   break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			   OV7695Capture(pImageWindow, pSensorConfigData);
	  	   break;
		default:
		     break;
	}
	OV7695SENSORDB("exit :\n ");
	return TRUE;
}	/* OV7695Control() */

/* [TC] YUV sensor */

BOOL OV7695mipi_set_param_wb(UINT16 para)
{

    switch (para)
    {
        case AWB_MODE_OFF:
							spin_lock(&ov7695_drv_lock);
							OV7695_AWB_ENABLE = KAL_FALSE;
							spin_unlock(&ov7695_drv_lock);
							OV7695mipi_set_AWB_mode(OV7695_AWB_ENABLE);
							break;
        case AWB_MODE_AUTO:
							spin_lock(&ov7695_drv_lock);
							OV7695_AWB_ENABLE = KAL_TRUE;
							spin_unlock(&ov7695_drv_lock);
							OV7695mipi_set_AWB_mode(OV7695_AWB_ENABLE);
			       break;
        case AWB_MODE_CLOUDY_DAYLIGHT: //cloudy
							OV7695mipi_set_AWB_mode(KAL_FALSE);
							OV7695_write_cmos_sensor(0x5204,0x06);
							OV7695_write_cmos_sensor(0x5205,0x30);
							OV7695_write_cmos_sensor(0x5206,0x04);
							OV7695_write_cmos_sensor(0x5207,0x00);
							OV7695_write_cmos_sensor(0x5208,0x04);
							OV7695_write_cmos_sensor(0x5209,0x30);
              break;
        case AWB_MODE_DAYLIGHT: //sunny
              OV7695mipi_set_AWB_mode(KAL_FALSE);
              OV7695_write_cmos_sensor(0x5204,0x06);
              OV7695_write_cmos_sensor(0x5205,0x10);
              OV7695_write_cmos_sensor(0x5206,0x04);
              OV7695_write_cmos_sensor(0x5207,0x00);
              OV7695_write_cmos_sensor(0x5208,0x04);
              OV7695_write_cmos_sensor(0x5209,0x48);
			        break;
        case AWB_MODE_INCANDESCENT: //office
							OV7695mipi_set_AWB_mode(KAL_FALSE);
							OV7695_write_cmos_sensor(0x5204,0x04);
							OV7695_write_cmos_sensor(0x5205,0xe0);
							OV7695_write_cmos_sensor(0x5206,0x04);
							OV7695_write_cmos_sensor(0x5207,0x00);
							OV7695_write_cmos_sensor(0x5208,0x05);
							OV7695_write_cmos_sensor(0x5209,0xa0);
			        break;
			   case AWB_MODE_TUNGSTEN:
              OV7695mipi_set_AWB_mode(KAL_FALSE);
              OV7695_write_cmos_sensor(0x5204,0x05);
              OV7695_write_cmos_sensor(0x5205,0x48);
              OV7695_write_cmos_sensor(0x5206,0x04);
              OV7695_write_cmos_sensor(0x5207,0x00);
              OV7695_write_cmos_sensor(0x5208,0x05);
              OV7695_write_cmos_sensor(0x5209,0xe0);
			        break;
        case AWB_MODE_FLUORESCENT:
              OV7695mipi_set_AWB_mode(KAL_FALSE);
              OV7695_write_cmos_sensor(0x5204,0x04);
              OV7695_write_cmos_sensor(0x5205,0x00);
              OV7695_write_cmos_sensor(0x5206,0x04);
              OV7695_write_cmos_sensor(0x5207,0x00);
              OV7695_write_cmos_sensor(0x5208,0x06);
              OV7695_write_cmos_sensor(0x5209,0x50);
			        break;
        default:
              return FALSE;
    }
       return TRUE;
} /* OV7695mipi_set_param_wb */

BOOL OV7695mipi_set_param_effect(UINT16 para)
{
    switch (para)
    {
        case MEFFECT_OFF:
                OV7695_write_cmos_sensor(0x5001,0x3f);
                OV7695_write_cmos_sensor(0x5800,0x06);
                OV7695_write_cmos_sensor(0x5803,0x40);
                OV7695_write_cmos_sensor(0x5804,0x20);
                break;
        case MEFFECT_SEPIA:
                OV7695_write_cmos_sensor(0x5001,0x3f);
                OV7695_write_cmos_sensor(0x5800,0x1e);
                OV7695_write_cmos_sensor(0x5803,0x40);
                OV7695_write_cmos_sensor(0x5804,0xa0);
                break;
        case MEFFECT_NEGATIVE:
                OV7695_write_cmos_sensor(0x5001,0x3f);
                OV7695_write_cmos_sensor(0x5800,0x46);
                break;
        case MEFFECT_SEPIAGREEN:
                OV7695_write_cmos_sensor(0x5001,0x3f);
                OV7695_write_cmos_sensor(0x5800,0x1e);
                OV7695_write_cmos_sensor(0x5803,0x60);
                OV7695_write_cmos_sensor(0x5804,0x60);
                break;
        case MEFFECT_SEPIABLUE:
                OV7695_write_cmos_sensor(0x5001,0x3f);
                OV7695_write_cmos_sensor(0x5800,0x1e);
                OV7695_write_cmos_sensor(0x5803,0xa0);
                OV7695_write_cmos_sensor(0x5804,0x40);
                      break;
		    case MEFFECT_MONO: //B&W
	              OV7695_write_cmos_sensor(0x5001,0x3f);
	              OV7695_write_cmos_sensor(0x5800,0x26);
	              break;
              default:
                   return KAL_FALSE;
    }
    return KAL_FALSE;

} /* OV7695mipi_set_param_effect */

BOOL OV7695mipi_set_param_banding(UINT16 para)
{
		switch (para) 
		{
			case AE_FLICKER_MODE_50HZ:
					OV7695_write_cmos_sensor(0x5002,0x42);
			break;
			case AE_FLICKER_MODE_60HZ:
					OV7695_write_cmos_sensor(0x5002,0x40);
			break;
			default:
					return FALSE;
		}
   return TRUE;
} /* OV7695mipi_set_param_banding */

BOOL OV7695mipi_set_param_exposure(UINT16 para)
{
  switch (para)
    {	
    case AE_EV_COMP_10:	                   
				OV7695_write_cmos_sensor(0x3a0f, 0x78);//	; AEC in H
				OV7695_write_cmos_sensor(0x3a10, 0x70);//	; AEC in L
				OV7695_write_cmos_sensor(0x3a1b, 0x78);//	; AEC out H
				OV7695_write_cmos_sensor(0x3a1e, 0x70);//	; AEC out L
				OV7695_write_cmos_sensor(0x3a11, 0xf0);//	; control zone H
				OV7695_write_cmos_sensor(0x3a1f, 0x38);//	; control zone L   
			  break;
		case AE_EV_COMP_00:
				OV7695_write_cmos_sensor(0x3a0f, 0x58);//	; AEC in H
				OV7695_write_cmos_sensor(0x3a10, 0x50);//	; AEC in L
				OV7695_write_cmos_sensor(0x3a1b, 0x5a);//	; AEC out H
				OV7695_write_cmos_sensor(0x3a1e, 0x4e);//	; AEC out L
				OV7695_write_cmos_sensor(0x3a11, 0xb0);//	; control zone H
				OV7695_write_cmos_sensor(0x3a1f, 0x28);//	; control zone L   
			  break;
    case AE_EV_COMP_n10:
				OV7695_write_cmos_sensor(0x3a0f, 0x30);//	; AEC in H
				OV7695_write_cmos_sensor(0x3a10, 0x28);//	; AEC in L
				OV7695_write_cmos_sensor(0x3a1b, 0x30);//	; AEC out H
				OV7695_write_cmos_sensor(0x3a1e, 0x28);//	; AEC out L
				OV7695_write_cmos_sensor(0x3a11, 0x60);//	; control zone H
				OV7695_write_cmos_sensor(0x3a1f, 0x14);//	; control zone L   
			  break;	
     default:
        return FALSE;
    }          
    return TRUE;
} /* OV7695mipi_set_param_exposure */

UINT32 OV7695MIPIYUVSensorSetting(FEATURE_ID iCmd, UINT32 iPara)
{
	OV7695SENSORDB("iCmd=%d,iPara=%d, %d \n",iCmd, iPara);

	switch (iCmd) {
		case FID_SCENE_MODE:
			OV7695SENSORDB("Night Mode:%d\n", iPara);
	    	if (iPara == SCENE_MODE_OFF)
	    	{
	       		  OV7695mipi_night_mode(KAL_FALSE);
	    	}
	    	else if (iPara == SCENE_MODE_NIGHTSCENE)
	    	{
          		OV7695mipi_night_mode(KAL_TRUE);
	    	}
	    	break;
		case FID_AWB_MODE:
        OV7695mipi_set_param_wb(iPara);
			  break;
		case FID_COLOR_EFFECT:
        OV7695mipi_set_param_effect(iPara);
		 	  break;
		case FID_AE_EV:
        OV7695mipi_set_param_exposure(iPara);
		    break;
		case FID_AE_FLICKER:
        OV7695mipi_set_param_banding(iPara);
		 	  break;
    case FID_AE_SCENE_MODE:
			  OV7695SENSORDB("Set AE Mode:%d\n", iPara);
         	if (iPara == AE_MODE_OFF) {
				spin_lock(&ov7695_drv_lock);
		 		    OV7695_AE_ENABLE = KAL_FALSE;
				spin_unlock(&ov7695_drv_lock);
        }
         	else {
				spin_lock(&ov7695_drv_lock);
		 		OV7695_AE_ENABLE = KAL_TRUE;
				spin_unlock(&ov7695_drv_lock);
	     	}
         OV7695mipi_set_AE_mode(OV7695_AE_ENABLE);
          break;
    case FID_ZOOM_FACTOR:
						OV7695SENSORDB("FID_ZOOM_FACTOR:%d\n", iPara);
						spin_lock(&ov7695_drv_lock);
						zoom_factor = iPara;
						spin_unlock(&ov7695_drv_lock);
            break;
	  default:
		 	      break;
	}
	  return TRUE;
}   /* OV7695MIPIYUVSensorSetting */


UINT32 OV7695MIPIYUVSetVideoMode(UINT16 u2FrameRate)
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
#if defined(MT6577)
static void OV7695GetEvAwbRef(UINT32 pSensorAEAWBRefStruct)
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

static void OV7695GetCurAeAwbInfo(UINT32 pSensorAEAWBCurStruct)
{
/*
	PSENSOR_AE_AWB_CUR_STRUCT Info = (PSENSOR_AE_AWB_CUR_STRUCT)pSensorAEAWBCurStruct;
	Info->SensorAECur.AeCurShutter=OV7695MIPIReadShutter();
	Info->SensorAECur.AeCurGain=OV7695MIPIReadSensorGain() * 2;
	Info->SensorAwbGainCur.AwbCurRgain=OV7695mipi_read_cmos_sensor(0x504c);
	Info->SensorAwbGainCur.AwbCurBgain=OV7695mipi_read_cmos_sensor(0x504e);
*/
}
#endif

UINT32 OV7695MIPIFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
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

	OV7695SENSORDB("feature id=%d \n",FeatureId);

	switch (FeatureId)
	{
		case SENSOR_FEATURE_GET_RESOLUTION:
			*pFeatureReturnPara16++=OV7695_IMAGE_SENSOR_VGA_WITDH;
			*pFeatureReturnPara16=OV7695_IMAGE_SENSOR_VGA_HEIGHT;
			*pFeatureParaLen=4;
			break;
		case SENSOR_FEATURE_GET_PERIOD:
			#if defined(MT6577)
			switch(CurrentScenarioId)
			{
				case MSDK_SCENARIO_ID_CAMERA_ZSD:
					*pFeatureReturnPara16++=OV7695_FULL_PERIOD_PIXEL_NUMS + OV7695MipiSensor.CaptureDummyPixels;
					*pFeatureReturnPara16=OV7695_FULL_PERIOD_LINE_NUMS + OV7695MipiSensor.CaptureDummyLines;
					*pFeatureParaLen=4;
					break;
				default:
					//OV7695SENSORDB("OV7695FeatureControl:SENSOR_FEATURE_GET_PERIOD \n");
					*pFeatureReturnPara16++=OV7695_PV_PERIOD_PIXEL_NUMS + OV7695MipiSensor.PreviewDummyPixels;
					*pFeatureReturnPara16=OV7695_PV_PERIOD_LINE_NUMS + OV7695MipiSensor.PreviewDummyLines;
					*pFeatureParaLen=4;
					break;
			}
			#else
				//OV7695SENSORDB("OV7695FeatureControl:SENSOR_FEATURE_GET_PERIOD \n");
				*pFeatureReturnPara16++=OV7695_PV_PERIOD_PIXEL_NUMS + OV7695MipiSensor.PreviewDummyPixels;
				*pFeatureReturnPara16=OV7695_PV_PERIOD_LINE_NUMS + OV7695MipiSensor.PreviewDummyLines;
				*pFeatureParaLen=4;
			#endif
			break;
		case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
			#if defined(MT6577)
			switch(CurrentScenarioId)
			{
				case MSDK_SCENARIO_ID_CAMERA_ZSD:
					//OV7695SENSORDB("OV7695FeatureControl:SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ \n");
					*pFeatureReturnPara32 = OV7695MipiSensor.CapturePclk * 1000 *100;	 //unit: Hz
					*pFeatureParaLen=4;
					break;
				default:
					//OV7695SENSORDB("OV7695FeatureControl:SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ \n");
					*pFeatureReturnPara32 = OV7695MipiSensor.PreviewPclk * 1000 *100;	 //unit: Hz
					*pFeatureParaLen=4;
					break;
				}
			#else
				//OV7695SENSORDB("OV7695FeatureControl:SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ \n");
				*pFeatureReturnPara32 = OV7695MipiSensor.PreviewPclk * 1000 *100;	 //unit: Hz
				*pFeatureParaLen=4;
			#endif
			break;
		case SENSOR_FEATURE_SET_ESHUTTER:
			//OV7695SENSORDB("OV7695FeatureControl:SENSOR_FEATURE_SET_ESHUTTER \n");
			break;
		case SENSOR_FEATURE_SET_NIGHTMODE:
			//OV7695SENSORDB("OV7695FeatureControl:SENSOR_FEATURE_SET_NIGHTMODE \n");
			OV7695mipi_night_mode((BOOL) *pFeatureData16);
			break;
		case SENSOR_FEATURE_SET_GAIN:
			//OV7695SENSORDB("OV7695FeatureControl:SENSOR_FEATURE_SET_GAIN \n");
			break;
		case SENSOR_FEATURE_SET_FLASHLIGHT:
			//OV7695SENSORDB("OV7695FeatureControl:SENSOR_FEATURE_SET_FLASHLIGHT \n");
			break;
		case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
			//OV7695SENSORDB("OV7695FeatureControl:SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ \n");
			break;
		case SENSOR_FEATURE_SET_REGISTER:
			//OV7695SENSORDB("OV7695FeatureControl:SENSOR_FEATURE_SET_REGISTER \n");
			OV7695_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
			break;
		case SENSOR_FEATURE_GET_REGISTER:
			//OV7695SENSORDB("OV7695FeatureControl:SENSOR_FEATURE_GET_REGISTER \n");
			pSensorRegData->RegData = OV7695mipi_read_cmos_sensor(pSensorRegData->RegAddr);
			break;
		case SENSOR_FEATURE_GET_CONFIG_PARA:
			//OV7695SENSORDB("OV7695FeatureControl:SENSOR_FEATURE_GET_CONFIG_PARA \n");
			memcpy(pSensorConfigData, &OV7695MipiSensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
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
			break;
		case SENSOR_FEATURE_GET_GROUP_COUNT:
            *pFeatureReturnPara32++=0;
            *pFeatureParaLen=4;
		    break;
		case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
			//OV7695SENSORDB("OV7695FeatureControl:SENSOR_FEATURE_GET_LENS_DRIVER_ID \n");
			// get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
			// if EEPROM does not exist in camera module.
			*pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
			*pFeatureParaLen=4;
			break;
		case SENSOR_FEATURE_CHECK_SENSOR_ID:
			//OV7695SENSORDB("OV7695FeatureControl:SENSOR_FEATURE_CHECK_SENSOR_ID \n");
			OV7695mipi_GetSensorID(pFeatureData32);
			break;
		case SENSOR_FEATURE_SET_YUV_CMD:
			//OV7695SENSORDB("OV7695FeatureControl:SENSOR_FEATURE_SET_YUV_CMD \n");
			OV7695MIPIYUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));
			break;
		case SENSOR_FEATURE_SET_VIDEO_MODE:
			//OV7695SENSORDB("OV7695FeatureControl:SENSOR_FEATURE_SET_VIDEO_MODE \n");
		    OV7695MIPIYUVSetVideoMode(*pFeatureData16);
		    break;
		#if defined(MT6577)
		case SENSOR_FEATURE_GET_EV_AWB_REF:
			OV7695GetEvAwbRef(*pFeatureData32);
			break;
		case SENSOR_FEATURE_GET_SHUTTER_GAIN_AWB_GAIN:
			OV7695GetCurAeAwbInfo(*pFeatureData32);
			break;
		#endif
		default:
			OV7695SENSORDB(":default \n");
			break;
	}
	return ERROR_NONE;
}	/* OV7695FeatureControl() */


SENSOR_FUNCTION_STRUCT	SensorFuncOV7695MIPI=
{
	OV7695MIPIOpen,
	OV7695MIPIGetInfo,
	OV7695MIPIGetResolution,
	OV7695MIPIFeatureControl,
	OV7695MIPIControl,
	OV7695MIPIClose
};

UINT32 OV7695_MIPI_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	OV7695SENSORDB();
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&SensorFuncOV7695MIPI;
	return ERROR_NONE;
}	/* SensorInit() */


