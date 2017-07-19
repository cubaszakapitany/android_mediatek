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
#include "ov5645mipiyuv_Sensor_m.h"
#include "ov7695yuv_Sensor_s.h"
#include "ov5645pipyuv_Sensor.h"
#include "ov5645pipyuv_Camera_Sensor_para.h"
#include "ov5645pipyuv_CameraCustomized.h" 
#define OV5645PIPYUV_DEBUG
#ifdef OV5645PIPYUV_DEBUG
#define OV5645PIPSENSORDB(fmt, arg...)    printk("[OV5645PIP]%s: " fmt, __FUNCTION__ ,##arg)
#else
#define OV5645PIPSENSORDB(x,...)
#endif

#define OV5645_15FPS

//#define OV5645PIP_SWITCH_CAMERA
#define OV5645PIP_OTP_READ_ID
//#define OV5645PIP_TEST_PIP_WIN


#ifdef mDELAY
    #undef mDELAY
#endif
#define mDELAY(ms)  msleep(ms)

#define OV5645_TEST_PATTERN_CHECKSUM (0x7ba87eae)

static DEFINE_SPINLOCK(ov5645pip_drv_lock);
static MSDK_SCENARIO_ID_ENUM CurrentScenarioId = MSDK_SCENARIO_ID_CAMERA_PREVIEW;
extern int iReadReg(u16 a_u2Addr , u8 * a_puBuff , u16 i2cId);
extern int iWriteReg(u16 a_u2Addr , u32 a_u4Data , u32 a_u4Bytes , u16 i2cId);

extern void OV7695PIPYUV_init_awb(void);
extern void OV7695PIPYUV_save_awb(void);
extern void OV7695PIPYUV_retore_awb(void);

extern struct Sensor_para OV5645MIPISensor;
extern bool AF_Power;

static void OV5645PIPUpdateCameraStatus(void)
{
	OV5645_PIP_Set_Window(OV5645MIPISensor.zone_addr);
	return;
}

extern void OV5645_Burst_FOCUS_OVT_AFC_Init(void);

#define OV5645PIP_write_cmos_sensor(addr, para) iWriteReg((u16) addr , (u32) para ,1,OV5645_WRITE_ID)
kal_uint16 OV5645PIP_read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte=0;
	iReadReg((u16) addr ,(u8*)&get_byte,OV5645_WRITE_ID);
	return get_byte;
}

/* Global Valuable */
static kal_uint32 zoom_factor = 0; 
//static kal_int8 OV5645PIP_DELAY_AFTER_PREVIEW = -1;
//static kal_uint8 OV5645PIP_Banding_setting = AE_FLICKER_MODE_50HZ;
//static kal_bool OV5645PIP_AWB_ENABLE = KAL_TRUE;
static kal_bool OV5645PIP_AE_ENABLE = KAL_TRUE; 
MSDK_SENSOR_CONFIG_STRUCT OV5645PIPSensorConfigData;
/*************************************************************************
* FUNCTION
*	OV5645PIP_set_dummy
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
*	3. kal_uint8   : isPVmode
*
* RETURNS
*	None
*
*************************************************************************/
void OV5645PIPSetDummy(kal_uint32 dummy_pixels, kal_uint32 dummy_lines, kal_uint8 isPVmode)
{
	//main dummy pixel should be 4x sub camera 
	if(dummy_pixels<4) 
		return;
    //OV5645SetDummy(dummy_pixels, dummy_lines);
    //OV7695PIPYUVSetDummy((dummy_pixels/4), dummy_lines, isPVmode);
} /* OV5645PIP_set_dummy */

/*************************************************************************
* FUNCTION
*	OV5645PIPWriteShutter
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
void OV5645PIPWriteShutter(kal_uint32 shutter_m, kal_uint32 shutter_s, kal_uint8 isPVmode)
{
    //kal_uint32 extra_exposure_lines = 0;
	if (shutter_m < 1)
	{
		shutter_m = 1;
	}
	if (shutter_s < 1)
	{
		shutter_s = 1;
	}
    OV5645WriteShutter(shutter_m);
    OV7695PIPYUVWriteShutter(shutter_s, isPVmode);
}    /* OV5645PIP_write_shutter */
/*************************************************************************
* FUNCTION
*	OV5645PIPWriteWensorGain
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
static void OV5645PIPWriteSensorGain(kal_uint32 gain_m, kal_uint32 gain_s)
{
	kal_uint16 temp_reg = 0;
		
	if(gain_m > 1024 )  ASSERT(0);
	if(gain_s > 1024 )  ASSERT(0);
	temp_reg = 0;
	temp_reg=gain_m&0x0FF;	
	OV5645WriteSensorGain(temp_reg);
	temp_reg=gain_s&0x0FF;	
    OV7695PIPYUVWriteSensorGain(temp_reg);
}  /* OV5645PIP_write_sensor_gain */
/*************************************************************************
* FUNCTION
*	OV5645PIPReadShutter
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
static kal_uint32 OV5645PIPReadShutter(kal_uint32 *shutter_m, kal_uint32 *shutter_s)
{
	*shutter_m=OV5645ReadShutter();
    *shutter_s=OV7695PIPYUVReadShutter();
	return KAL_TRUE;	
}    /* OV5645PIP_read_shutter */
/*************************************************************************
* FUNCTION
*	OV5645PIPReadSensorGain
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
static kal_uint32 OV5645PIPReadSensorGain(kal_uint32 *gain_m, kal_uint32 *gain_s)
{
	*gain_m=OV5645ReadSensorGain();
    *gain_s=OV7695PIPYUVReadSensorGain();
	return KAL_TRUE;
}  /* OV5645PIPReadSensorGain */

static void OV5645PIP_set_AE_mode(kal_bool AE_enable)
{
	OV5645_set_AE_mode(AE_enable);
	OV7695PIPYUV_set_AE_mode(AE_enable);
}

static void OV5645PIP_set_AWB_mode(kal_bool AWB_enable)
{
	OV5645_set_AWB_mode(AWB_enable);
	OV7695PIPYUV_set_AWB_mode(AWB_enable);
}

/*************************************************************************
* FUNCTION
*	OV5645PIP_night_mode
*
* DESCRIPTION
*	This function night mode of OV5645PIP.
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
void OV5645PIP_night_mode(kal_bool enable)
{
    //OV5645_night_mode(enable);
    //OV7695PIPYUV_night_mode(enable);

}    /* OV5645PIP_night_mode */

void OV5645PIP_GetAEFlashlightInfo(UINT32 infoAddr)
{
    SENSOR_FLASHLIGHT_AE_INFO_STRUCT* pAeInfo = (SENSOR_FLASHLIGHT_AE_INFO_STRUCT*) infoAddr;
#if 1
    pAeInfo->Exposuretime = OV5645ReadShutter();
    pAeInfo->Gain = OV5645ReadSensorGain();
//    OV5645PIPSENSORDB("[OV5645PIP] %s pAeInfo->Gain = %d:\n ", __func__, pAeInfo->Gain);
//    OV5645PIPSENSORDB("[OV5645PIP] %s pAeInfo->Exposuretime = %d:\n ", __func__, pAeInfo->Exposuretime);
#endif
    //pAeInfo->u4Fno = 28;
    //pAeInfo->GAIN_BASE = 50;
}

#ifdef OV5645PIP_OTP_READ_ID
void OV5645_Get_Modules( kal_uint32 *pModule_id, kal_uint32 *pLens_id)
{
	*pModule_id = OV5645MIPISensor.module_id;
	*pLens_id = OV5645MIPISensor.lens_id;
}

static void OV5645PIP_otp_handle(void)
{
	u16 addr = 0;
	kal_uint32      module_id;
	kal_uint32      lens_id;
	static int iOTP_Handled = 0;
	if ( iOTP_Handled ){
		OV5645PIPSENSORDB("iOTP_Handled=%d\n", iOTP_Handled);
		return;
	}
	iOTP_Handled = 1;

	OV5645PIP_write_cmos_sensor(0x3000, 0x00);
	//OV5645PIP_write_cmos_sensor(0x3004, 0x10);
	OV5645PIP_write_cmos_sensor(0x3004, 0xdf);

	for (addr = 0x3D00; addr <= 0x3D1F; addr++)
		OV5645PIP_write_cmos_sensor(addr, 0x00);

	OV5645PIP_write_cmos_sensor(0x3D21, 0x01);
	mdelay(10);
	module_id = OV5645PIP_read_cmos_sensor(0x3d05);
	lens_id = OV5645PIP_read_cmos_sensor(0x3d06);
	
	OV5645PIP_write_cmos_sensor(0x3D21, 0x00);
	
	spin_lock(&ov5645pip_drv_lock);
	OV5645MIPISensor.module_id = module_id;
	OV5645MIPISensor.lens_id = lens_id;
	spin_unlock(&ov5645pip_drv_lock);
	
	OV5645PIPSENSORDB("mid1 = 0x%x:\n", OV5645MIPISensor.module_id);
	OV5645PIPSENSORDB("lens_id = 0x%x:\n", OV5645MIPISensor.lens_id);

	spin_lock(&ov5645pip_drv_lock);
	if ( OV5645_MID_DARLING == OV5645MIPISensor.module_id ){
		if (OV5645_LENS_ID_EP_D420F00 == OV5645MIPISensor.lens_id) {
			OV5645MIPISensor.is_support_af = KAL_TRUE;
		} else if (OV5645_LENS_ID_UNKNOW_FOR_DARLING_AF == OV5645MIPISensor.lens_id) {
			OV5645MIPISensor.is_support_af = KAL_TRUE;
		} else if (OV5645_LENS_ID_UNKNOW_FOR_DARLING_FF0 == OV5645MIPISensor.lens_id) {
			OV5645MIPISensor.is_support_af = KAL_FALSE;
		} else if (OV5645_LENS_ID_UNKNOW_FOR_DARLING_FF1 == OV5645MIPISensor.lens_id) {
			OV5645MIPISensor.is_support_af = KAL_FALSE;
		} 
	}else if( OV5645_MID_TRULY == OV5645MIPISensor.module_id ){
		if (OV5645_LENS_ID_SHUNYU_3515 == OV5645MIPISensor.lens_id) {
			OV5645MIPISensor.is_support_af = KAL_FALSE;
		} else if ( OV5645_LENS_ID_HOKUANG_GBCA2128201 == OV5645MIPISensor.lens_id ){
			OV5645MIPISensor.is_support_af = KAL_TRUE;
		}
	}
	spin_unlock(&ov5645pip_drv_lock);
}
#endif

/*************************************************************************
* FUNCTION
*	OV5645PIP_GetSensorID
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
static int init_sys_device_name(void);
//static kal_uint32 OV5645PIP_GetSensorID(kal_uint32 *sensorID_m, kal_uint32 *sensorID_s)
static kal_uint32 OV5645PIP_GetSensorID(kal_uint32 *sensorID_m)
{
	kal_uint32 temp1;
	OV5645PIPSENSORDB();

	OV5645_GetSensorID(sensorID_m);
	if( 0xffffffff == *sensorID_m )
		return ERROR_NONE;
	
#ifdef OV5645PIP_SWITCH_CAMERA
	init_sys_device_name();
#endif
	OV7695PIPYUVInitialVariable();
	OV5645initalvariable();

#ifdef OV5645PIP_OTP_READ_ID
	OV5645PIP_otp_handle();
#endif

	return ERROR_NONE;    
}   

static void OV5645PIPInitialSettingForDistinctSensor(void)
{
	if (OV5645_MID_TRULY == OV5645MIPISensor.module_id) {
		switch (OV5645MIPISensor.lens_id) {
		case OV5645_LENS_ID_HOKUANG_GBCA2128201:
			OV5645InitialSetting_truly_af();
			break;
		case OV5645_LENS_ID_SHUNYU_3515:
			OV5645InitialSetting_truly_ff();
			break;
		default:
			break;
		}
	} else if (OV5645_MID_DARLING == OV5645MIPISensor.module_id) {
		switch (OV5645MIPISensor.lens_id) {
			case OV5645_LENS_ID_EP_D420F00:
				OV5645InitialSetting_darling_af();
				break;
			case OV5645_LENS_ID_UNKNOW_FOR_DARLING_AF:
				OV5645InitialSetting_darling_af();
				break;
			default:
				OV5645InitialSetting_darling_ff();
				break;
		}
	} else {
		OV5645InitialSetting_truly_af();
	}


}

/*************************************************************************
* FUNCTION
*    OV5645PIPInitialSetting
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
static void OV5645PIPInitialSetting(void)
{
    OV5645PIPSENSORDB("module_id = %d, lens_id=%d\n",OV5645MIPISensor.module_id,
		OV5645MIPISensor.lens_id);
#ifdef DEBUG_SENSOR
{
	struct file *fp; 
	mm_segment_t fs; 
	loff_t pos = 0; 
	kal_uint8 fromsd = 0;

	printk("ov7695pip_sd Open File Start\n");
	fp = filp_open("/storage/sdcard0/ov7695pip_sd.dat", O_RDONLY , 0); 
	if (IS_ERR(fp)) 
	{ 
		fromsd = 0;   
		printk("open file error\n");
	} 
	else 
	{
		printk("open file success\n");
		fromsd = 1;
		filp_close(fp, NULL); 
	}
	if(fromsd == 1)
		OV7695PIP_Initialize_from_T_Flash();
	else
		OV7695PIPYUVInitialSetting();
}
#else
	OV7695PIPYUVInitialSetting();
#endif

	OV7695PIPYUVInitialSettingVar();
//	mDELAY(10);
#ifdef DEBUG_SENSOR
{
	struct file *fp; 
	mm_segment_t fs; 
	loff_t pos = 0; 
	kal_uint8 fromsd = 0;

	printk("ov5645pip_sd Open File Start\n");
	fp = filp_open("/storage/sdcard0/ov5645pip_sd.dat", O_RDONLY , 0); 
	if (IS_ERR(fp)) 
	{ 
		fromsd = 0;   
		printk("open file error\n");
	} 
	else 
	{
		printk("open file success\n");
		fromsd = 1;
		filp_close(fp, NULL); 
	}
	if(fromsd == 1)
		OV5645PIP_Initialize_from_T_Flash();
	else
		OV5645PIPInitialSettingForDistinctSensor();
}
#else
	OV5645PIPInitialSettingForDistinctSensor();
#endif
//	mDELAY(50);	
} 
/*****************************************************************
* FUNCTION
*    OV5645PIPPreviewSetting
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
static void OV5645PIPPreviewSetting_720p_QVGA(void)
{
	//for PIP QVGA preview
	OV5645_Standby();
	mDELAY(10);
	OV7695PIPYUVPreviewSetting_QVGA();
	mDELAY(10);
	
	OV5645PreviewSetting_720p();
	mDELAY(100);//200
	OV5645_Wakeup();
	mDELAY(2);
}

static void OV5645PIPVideoSetting(void)
{
}
/*****************************************************************
* FUNCTION
*    OV5645PIPVideoSetting_720p_480
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
#if 0
static void OV5645PIPVideoSetting_720p_480(void)
{
    //for PIP  video
    OV5645_Standby();
    mDELAY(10);
    OV7695PIPYUV_OFF100();
    OV7695PIPYUVVideoSetting_480();
    mDELAY(10);
    OV5645VideoSetting_720p();
    mDELAY(100);
    OV5645_Wakeup();
    mDELAY(2);
    OV7695PIPYUV_ON100();
}
#endif

/*************************************************************************
* FUNCTION
*     OV5645PIPFullSizeCaptureSetting
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
static void OV5645PIPFullSizeCaptureSetting( OV5645_SENSOR_MODE eSensorMode )
{
	//PIP for capture, 2560x1440 and VGA 
	OV5645_Standby();
	mDELAY(10);
	OV7695PIPYUVFullSizeCaptureSetting();
	mDELAY(10);
	OV5645FullSizeCaptureSetting(eSensorMode);
	OV5645SetHVMirror(IMAGE_V_MIRROR, SENSOR_MODE_CAPTURE);
	mDELAY(100);//100
	OV5645_Wakeup();
	mDELAY(2);
	OV5645PIPUpdateCameraStatus();
}

#ifdef OV5645PIP_TEST_PIP_WIN
#include <linux/kthread.h>
static struct task_struct *tinno_test_task = NULL;
static DECLARE_WAIT_QUEUE_HEAD(waiter);
static volatile	int wait_flag = 0;

static volatile	int g_sleep_count = 350;

static int tinno_test_kthread(void *data)
{
	static int count = 0;
	struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };
	sched_setscheduler(current, SCHED_RR, &param);
	
	for( ;; ) {

		if (kthread_should_stop())
			break;
		
		set_current_state(TASK_INTERRUPTIBLE); 
		wait_event_interruptible(waiter, wait_flag!=0);
		wait_flag = 0;

		msleep(g_sleep_count);

		if(OV5645MIPISensor.SensorMode != SENSOR_MODE_INIT)
		{
			//OV5645PIPUpdateCameraStatus();
		}
	}

	return 0;
}

static kal_uint32 OV5645PIPTest(void)
{
        tinno_test_task = kthread_create(tinno_test_kthread, NULL, "tinno_test_kthread");
        if (IS_ERR(tinno_test_task)) {
            OV5645PIPSENSORDB("test task create fail\n");
        }
        else {
            wake_up_process(tinno_test_task);
        }
}
#endif
/*************************************************************************
* FUNCTION
*	OV5645PIPOpen
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
UINT32 OV5645PIPOpen(void)
{
	kal_uint32  sensorID;
	OV5645PIPSENSORDB("[enter] (ScenarioId=%d) :\n ", CurrentScenarioId);
	OV5645_GetSensorID(&sensorID);
	if(sensorID!=OV5645PIP_MIPI_SENSOR_ID)
		return ERROR_SENSOR_CONNECT_FAIL;

	OV7695PIPYUV_GetSensorID(&sensorID);
	if(sensorID!=OV7695PIP_SENSOR_ID){
		OV7695PIPYUVSetSeneorExist(KAL_FALSE);
		return ERROR_SENSOR_CONNECT_FAIL;
	}
	
	OV7695PIPYUV_init_awb();
	OV5645PIPInitialSetting();
	OV5645_Wakeup();
	OV7695PIPYUV_ON100();
	OV5645MIPISensor.SensorMode= SENSOR_MODE_INIT;

    if (OV5645MIPISensor.is_support_af) {
	OV5645_FOCUS_OVT_AFC_Init();
        if (false == AF_Power){
            ov5645_mipi_yuv_af_poweron(true);
            spin_lock(&ov5645pip_drv_lock);
            AF_Power = true;
            spin_unlock(&ov5645pip_drv_lock);
        }
        else{
            OV5645PIPSENSORDB(" AF Power has already on.\n");
        }
    }
#ifdef OV5645PIP_TEST_PIP_WIN
	OV5645PIPTest();
#endif
	OV5645PIPSENSORDB(" Exit :\n ");
	return ERROR_NONE;
}	/* OV5645PIPOpen() */
/*************************************************************************
* FUNCTION
*	OV5645PIPClose
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
UINT32 OV5645PIPClose(void)
{
    //flash_mode = 2;    //set default flash-mode as video-mode-flashlight 2
    kal_uint16 lastadc, num, i, now, step = 100;
	OV5645PIPSENSORDB("[enter] (ScenarioId=%d) :\n ", CurrentScenarioId);
	
#ifdef OV5645PIP_TEST_PIP_WIN
	if(tinno_test_task){
		OV5645MIPISensor.SensorMode = SENSOR_MODE_INIT;
		wait_flag = 1;
		wake_up_interruptible(&waiter);
		kthread_stop(tinno_test_task);
	}
#endif
    OV5645PIP_write_cmos_sensor(0x3023,0x01);
    OV5645PIP_write_cmos_sensor(0x3022,0x06);
    lastadc = OV5645read_vcm_adc();
    num = lastadc/step;
	for(i = 1; i <= num; i++){
        OV5645set_vcm_adc(lastadc-i*step);
        mdelay(30);
    }

    OV5645set_vcm_adc(0);
    OV5645PIP_write_cmos_sensor(0x3023,0x01);
    OV5645PIP_write_cmos_sensor(0x3022,0x08);

	if (OV5645MIPISensor.is_support_af) {
		if (true == AF_Power){
			ov5645_mipi_yuv_af_poweron(false);
			spin_lock(&ov5645pip_drv_lock);
			AF_Power = false;
			spin_unlock(&ov5645pip_drv_lock);
		}
		else{
			OV5645PIPSENSORDB(" AF Power is already off.\n");
		}
	}
	return ERROR_NONE;
}	/* OV5645PIPClose() */
/*************************************************************************
* FUNCTION
*	OV5645PIPPreview
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
UINT32 OV5645PIPPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	kal_uint32 zsdshutter = 0;
	OV5645PIPSENSORDB(" Enter :\n ");
	switch(CurrentScenarioId)
	{
	case MSDK_SCENARIO_ID_CAMERA_ZSD:
		OV5645MIPISensor.zsd_flag=1;
		OV5645PIPFullSizeCaptureSetting(SENSOR_MODE_PREVIEW);
		zsdshutter=OV5645MIPISensor.PreviewShutter*2;
		//OV5645PIP_set_AE_mode(KAL_FALSE);
		OV5645WriteExpShutter(zsdshutter);
		break;
	default:
		OV5645PIPPreviewSetting_720p_QVGA();
		OV5645MIPISensor.zsd_flag=0;
		zsdshutter=0;
		//OV5645PIP_set_AE_mode(KAL_FALSE);
		OV5645WriteExpShutter(OV5645MIPISensor.PreviewShutter);
		OV5645SetHVMirror(IMAGE_V_MIRROR, SENSOR_MODE_PREVIEW);
		break;
	}
	mDELAY(5);

	OV5645PIP_set_AE_mode(KAL_TRUE);
	OV7695PIPYUV_retore_awb();
	OV5645PIP_set_AWB_mode(KAL_TRUE);
	mDELAY(20);
	if (OV5645MIPISensor.is_support_af) {
		OV5645_FOCUS_OVT_AFC_Constant_Focus();
	}
	
#ifdef OV5645PIP_TEST_PIP_WIN
	wait_flag = 1;
	wake_up_interruptible(&waiter);
#endif
	OV5645PIPSENSORDB(" Exit :\n ");
	return TRUE ;
}    /* OV5645PIPPreview() */

UINT32 OV5645PIPYUVSetVideoMode(UINT16 u2FrameRate)
{
//    OV5645PIPPreviewSetting_720p_QVGA();
//	OV5645PIPVideoSetting();
//	OV5645SetHVMirror(IMAGE_V_MIRROR, SENSOR_MODE_PREVIEW);
	return TRUE;
}

UINT32 OV5645PIPCapture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    kal_uint32 shutter_m = 0, shutter_s=0, gain_m =0, gain_s =0;//, temp_reg = 0;
    kal_uint32 extshutter = 0;
    kal_uint32 color_r_gain = 0;
    kal_uint32 color_b_gain = 0;
	OV5645PIPSENSORDB(" Enter :\n ");
    if(SENSOR_MODE_PREVIEW == OV5645MIPISensor.SensorMode )
    {
        OV5645PIPReadShutter(&shutter_m,&shutter_s);
        //extshutter=OV5645ReadExtraShutter();

        OV5645PIPReadSensorGain(&gain_m, &gain_s);
/*        OV5645PIPSENSORDB("matrix befor shutter_s=%d:\n",shutter_s);
        OV5645PIPSENSORDB("matrix befor gain_s=%d:\n",gain_s);
        OV5645PIPSENSORDB("matrix befor shutter_m=%d:\n",shutter_m);
        OV5645PIPSENSORDB("matrix befor gain_m=%d:\n",gain_m);
*/
        spin_lock(&ov5645pip_drv_lock);
        OV5645MIPISensor.PreviewShutter=shutter_m;
        OV5645MIPISensor.PreviewExtraShutter=extshutter;
        OV5645MIPISensor.SensorGain=gain_m;
        spin_unlock(&ov5645pip_drv_lock);

	OV5645_FOCUS_OVT_AFC_Pause();
	OV5645PIP_set_AE_mode(KAL_FALSE);
	OV5645PIP_set_AWB_mode(KAL_FALSE);
	OV7695PIPYUV_save_awb();
        color_r_gain=((OV5645PIP_read_cmos_sensor(0x3401)&0xFF)+((OV5645PIP_read_cmos_sensor(0x3400)&0xFF)*256));
        color_b_gain=((OV5645PIP_read_cmos_sensor(0x3405)&0xFF)+((OV5645PIP_read_cmos_sensor(0x3404)&0xFF)*256));
        OV5645PIPSENSORDB("REG befor reg0x3501=%x£¬"
			"reg0x3502=%x£¬reg0x350B=%x£¬reg0x380C=%x£¬reg0x380D=%x, "
			"reg0x3602=%x£¬reg0x3603=%x:\n",
			OV5645PIP_read_cmos_sensor(0x3501),
			OV5645PIP_read_cmos_sensor(0x3502),
			OV5645PIP_read_cmos_sensor(0x350B),
			OV5645PIP_read_cmos_sensor(0x380C),
			OV5645PIP_read_cmos_sensor(0x380D),
			OV5645PIP_read_cmos_sensor(0x3602),
			OV5645PIP_read_cmos_sensor(0x3603));

        OV5645PIPFullSizeCaptureSetting(SENSOR_MODE_CAPTURE);
#if 1
        //OV5645WBcalibattion(color_r_gain,color_b_gain);
        //OV5645PIPSENSORDB("[OV5645]Before shutter=%d:\n",shutter);
        if(OV5645MIPISensor.zsd_flag==0)
        {
            //shutter_m = shutter_m*2;
            shutter_m=shutter_m*16/11;
            shutter_s=shutter_s*8/11;
        }
        if (SCENE_MODE_HDR == OV5645MIPISensor.sceneMode)
        {
            spin_lock(&ov5645pip_drv_lock);
            OV5645MIPISensor.currentExposureTime=shutter_m;
            OV5645MIPISensor.currentextshutter=extshutter;
            OV5645MIPISensor.currentAxDGain=gain_m;
            spin_unlock(&ov5645pip_drv_lock);
        }
        else
        {
            OV5645PIPWriteSensorGain(OV5645MIPISensor.SensorGain, gain_s);
            OV5645PIPWriteShutter(shutter_m,shutter_s,0);
        }
		
       mDELAY(100);
/*	   
        OV5645PIPReadShutter(&shutter_m,&shutter_s);
        //extshutter=OV5645ReadExtraShutter();

        OV5645PIPReadSensorGain(&gain_m, &gain_s);
        OV5645PIPSENSORDB("matrix after shutter=%d:\n",shutter_s);
        OV5645PIPSENSORDB("matrix after gain_s=%d:\n",gain_s);
        OV5645PIPSENSORDB("matrix after shutter_m=%d:\n",shutter_m);
        OV5645PIPSENSORDB("matrix after gain_m=%d:\n",gain_m);
        OV5645PIPSENSORDB("REG after reg0x3503=%d£¬"
			"reg0x380c=%d£¬reg0x380d=%d£¬reg0x380e=%d£¬reg0x380f=%d:\n",
			OV5645PIP_read_cmos_sensor(0x3503),
			OV5645PIP_read_cmos_sensor(0x380c),
			OV5645PIP_read_cmos_sensor(0x380D),
			OV5645PIP_read_cmos_sensor(0x380E),
			OV5645PIP_read_cmos_sensor(0x380F)	);
*/
/*
        OV5645PIPSENSORDB("REG after reg0x3501=%x£¬"
			"reg0x3502=%x£¬reg0x350B=%x£¬reg0x380C=%x£¬reg0x380D=%x, "
			"reg0x3602=%x£¬reg0x3603=%x:\n",
			OV5645PIP_read_cmos_sensor(0x3501),
			OV5645PIP_read_cmos_sensor(0x3502),
			OV5645PIP_read_cmos_sensor(0x350B),
			OV5645PIP_read_cmos_sensor(0x380C),
			OV5645PIP_read_cmos_sensor(0x380D),
			OV5645PIP_read_cmos_sensor(0x3602),
			OV5645PIP_read_cmos_sensor(0x3603));
*/
		OV5645PIPSENSORDB("REG after reg0x3500=%x£¬"
	            "reg0x3501=%x£¬reg0x3502=%x£¬reg0x3503=%x£¬reg0x350a=%x, "
	            "reg0x350b=%x£¬reg0x3406=%x:\n",
	            OV5645PIP_read_cmos_sensor(0x3500),
	            OV5645PIP_read_cmos_sensor(0x3501),
	            OV5645PIP_read_cmos_sensor(0x3502),
	            OV5645PIP_read_cmos_sensor(0x3503),
	            OV5645PIP_read_cmos_sensor(0x350a),
	            OV5645PIP_read_cmos_sensor(0x350b),
	            OV5645PIP_read_cmos_sensor(0x3406));

#else
        spin_lock(&ov5645pip_drv_lock);
        OV5645MIPISensor.CaptureDummyPixels = 0;
        OV5645MIPISensor.CaptureDummyLines = 0;
        spin_unlock(&ov5645pip_drv_lock);
        shutter_m=2*shutter_m*8/11;
        shutter_s=shutter_s*8/11;
        OV5645WBcalibattion(color_r_gain,color_b_gain);
        OV5645PIPSetDummy(OV5645MIPISensor.CaptureDummyPixels, OV5645MIPISensor.CaptureDummyLines, 0);
        OV5645PIPWriteShutter(shutter_m,shutter_s,0);
        mDELAY(100);
#endif
    }
	OV5645PIPSENSORDB(" Exit :\n ");
    return ERROR_NONE;
}/* OV5645PIPCapture() */

UINT32 OV5645PIPGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
	OV5645PIPSENSORDB("[enter] (ScenarioId=%d) :\n ", CurrentScenarioId);

	pSensorResolution->SensorPreviewWidth= OV5645PIP_IMAGE_SENSOR_720P_WIDTH - 2 * OV5645PIP_PV_GRAB_START_X;
	pSensorResolution->SensorPreviewHeight= OV5645PIP_IMAGE_SENSOR_720P_HEIGHT - 2 * OV5645PIP_PV_GRAB_START_Y;
	pSensorResolution->SensorFullWidth= OV5645PIP_IMAGE_SENSOR_QSXGA_WITDH- 2 * OV5645PIP_FULL_GRAB_START_X; 
	pSensorResolution->SensorFullHeight= OV5645PIP_IMAGE_SENSOR_QSXGA_HEIGHT- 2 * OV5645PIP_FULL_GRAB_START_X;
	pSensorResolution->SensorVideoWidth= OV5645PIP_IMAGE_SENSOR_720P_WIDTH - 2 * OV5645PIP_PV_GRAB_START_X;
	pSensorResolution->SensorVideoHeight= OV5645PIP_IMAGE_SENSOR_720P_HEIGHT - 2 * OV5645PIP_PV_GRAB_START_Y;
#ifdef TINNO_PIP_SUPPORT	
	pSensorResolution->SensorPIPPreviewWidth= OV5645PIP_IMAGE_SENSOR_720P_WIDTH - 2 * OV5645PIP_PV_GRAB_START_X;
	pSensorResolution->SensorPIPPreviewHeight= OV5645PIP_IMAGE_SENSOR_720P_HEIGHT - 2 * OV5645PIP_PV_GRAB_START_Y;
	pSensorResolution->SensorPIPFullWidth= OV5645PIP_IMAGE_SENSOR_QSXGA_WITDH- 2 * OV5645PIP_FULL_GRAB_START_X; 
	pSensorResolution->SensorPIPFullHeight= OV5645PIP_IMAGE_SENSOR_QSXGA_HEIGHT- 2 * OV5645PIP_FULL_GRAB_START_X;
	pSensorResolution->SensorPIPVideoWidth= OV5645PIP_IMAGE_SENSOR_720P_WIDTH - 2 * OV5645PIP_PV_GRAB_START_X;
	pSensorResolution->SensorPIPVideoHeight= OV5645PIP_IMAGE_SENSOR_720P_HEIGHT - 2 * OV5645PIP_PV_GRAB_START_Y;
#endif
	return ERROR_NONE;
}	/* OV5645PIPGetResolution() */

UINT32 OV5645PIPGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,MSDK_SENSOR_INFO_STRUCT *pSensorInfo,MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	OV5645PIPSENSORDB(" enter (ScenarioId=%d):\n ", ScenarioId);
#if 1 //defined(MT6577)
	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			pSensorInfo->SensorPreviewResolutionX=OV5645PIP_IMAGE_SENSOR_QSXGA_WITDH - 2*OV5645PIP_PV_GRAB_START_X;
			pSensorInfo->SensorPreviewResolutionY=OV5645PIP_IMAGE_SENSOR_QSXGA_HEIGHT - 2*OV5645PIP_PV_GRAB_START_Y;	
			break;
		default:
			pSensorInfo->SensorPreviewResolutionX=OV5645PIP_IMAGE_SENSOR_720P_WIDTH - 2*OV5645PIP_PV_GRAB_START_X;
			pSensorInfo->SensorPreviewResolutionY=OV5645PIP_IMAGE_SENSOR_720P_HEIGHT - 2*OV5645PIP_PV_GRAB_START_Y;	
			break;
	}
#else
		  pSensorInfo->SensorPreviewResolutionX= OV5645PIP_IMAGE_SENSOR_720P_WIDTH - 2*OV5645PIP_PV_GRAB_START_X;
		  pSensorInfo->SensorPreviewResolutionY= OV5645PIP_IMAGE_SENSOR_720P_HEIGHT - 2*OV5645PIP_PV_GRAB_START_Y;
#endif
	pSensorInfo->SensorFullResolutionX= OV5645PIP_IMAGE_SENSOR_QSXGA_WITDH - 2*OV5645PIP_FULL_GRAB_START_X;
	pSensorInfo->SensorFullResolutionY= OV5645PIP_IMAGE_SENSOR_QSXGA_HEIGHT - 2*OV5645PIP_FULL_GRAB_START_X;
#ifdef OV5645_15FPS
    pSensorInfo->SensorCameraPreviewFrameRate=15; //30;
    pSensorInfo->SensorVideoFrameRate=15;//30;
    pSensorInfo->SensorStillCaptureFrameRate=5;//11;
#else
    pSensorInfo->SensorCameraPreviewFrameRate=30; //15;
	pSensorInfo->SensorVideoFrameRate=30;
	pSensorInfo->SensorStillCaptureFrameRate=11;
#endif
	pSensorInfo->SensorWebCamCaptureFrameRate=15;
	pSensorInfo->SensorResetActiveHigh=FALSE;
	pSensorInfo->SensorResetDelayCount=1;
    pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_VYUY;//SENSOR_OUTPUT_FORMAT_YUYV;
	pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorInterruptDelayLines = 1;
	pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_MIPI;
#ifdef OV5645_15FPS
	pSensorInfo->CaptureDelayFrame = 4;//1//--4
    pSensorInfo->PreviewDelayFrame = 5;
	pSensorInfo->VideoDelayFrame = 4; //  --4
#else
	pSensorInfo->CaptureDelayFrame = 2;//1
    pSensorInfo->PreviewDelayFrame = 3;//5
	pSensorInfo->VideoDelayFrame = 3; ///4	
#endif
	pSensorInfo->SensorMasterClockSwitch = 0; 
	pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_8MA;   		

	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		//case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
					pSensorInfo->SensorClockFreq=24;//26
					pSensorInfo->SensorClockDividCount=	3;
					pSensorInfo->SensorClockRisingCount= 0;
					pSensorInfo->SensorClockFallingCount= 2;
					pSensorInfo->SensorPixelClockCount= 3;
					pSensorInfo->SensorDataLatchCount= 2;
					pSensorInfo->SensorGrabStartX = OV5645PIP_PV_GRAB_START_X; 
					pSensorInfo->SensorGrabStartY = OV5645PIP_PV_GRAB_START_Y;            
					pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;
					pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
					pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14;
					pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
					pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
					pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x
					pSensorInfo->SensorPacketECCOrder = 1;
			   break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		//case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
#if 1//defined(MT6577)
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
		#endif
					pSensorInfo->SensorClockFreq=24;//26;
					pSensorInfo->SensorClockDividCount=	3;
					pSensorInfo->SensorClockRisingCount= 0;
					pSensorInfo->SensorClockFallingCount= 2;
					pSensorInfo->SensorPixelClockCount= 3;
					pSensorInfo->SensorDataLatchCount= 2;
					pSensorInfo->SensorGrabStartX = OV5645PIP_FULL_GRAB_START_X; 
					pSensorInfo->SensorGrabStartY = OV5645PIP_FULL_GRAB_START_Y;             
					pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;
					pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
					pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 0;//14;
					pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
					pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
					pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x
					pSensorInfo->SensorPacketECCOrder = 1;
				 break;
		default:
					pSensorInfo->SensorClockFreq=24;//26;
					pSensorInfo->SensorClockDividCount=3;
					pSensorInfo->SensorClockRisingCount=0;
					pSensorInfo->SensorClockFallingCount=2;
					pSensorInfo->SensorPixelClockCount=3;
					pSensorInfo->SensorDataLatchCount=2;
					pSensorInfo->SensorGrabStartX = OV5645PIP_PV_GRAB_START_X; 
					pSensorInfo->SensorGrabStartY = OV5645PIP_PV_GRAB_START_Y; 			                        
		     break;
	}
	memcpy(pSensorConfigData, &OV5645PIPSensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));	
	return ERROR_NONE;
}	/* OV5645PIPGetInfo() */

extern int kdCISModulePIPFrontSensorPowerOn(int iFrontSensorStatus);

UINT32 OV5645PIPControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	  OV5645PIPSENSORDB("enter (ScenarioId=%d):\n ",ScenarioId);
	  CurrentScenarioId = ScenarioId;
	  switch (ScenarioId)
	  {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		//case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
#if 1//defined(MT6577)
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
#endif
		   	 OV5645PIPPreview(pImageWindow, pSensorConfigData);
			   break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		//case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
			   OV5645PIPCapture(pImageWindow, pSensorConfigData);
	  	   break;
		default:
		     break; 
	}
    return ERROR_NONE;
}	/* OV5645PIPControl() */

/* [TC] YUV sensor */	

BOOL OV5645PIP_set_param_wb(UINT16 para)
{
    OV5645_set_param_wb(para);
    OV7695PIPYUV_set_param_wb(para);
    mDELAY(30);
       return TRUE;
} /* OV5645PIP_set_param_wb */

void OV5645PIP_set_contrast(UINT16 para)
{
    OV5645_set_contrast(para);
    //OV7695PIPYUV_set_contrast(para);
    mDELAY(30);
}

void OV5645PIP_set_brightness(UINT16 para)
{
    OV5645_set_brightness(para);
    //OV9695_set_brightness(para);
    mDELAY(30);
}

void OV5645PIP_set_saturation(UINT16 para)
{
    OV5645_set_saturation(para);
    //OV7695PIPYUV_set_saturation(para);
    mDELAY(30);
}

void OV5645PIP_set_iso(UINT16 para)
{
    OV5645_set_iso(para);
    //OV7695PIPYUV_set_iso(para);
    mDELAY(30);
}

void OV5645PIP_set_scene_mode(UINT16 para)
{
    OV5645_set_scene_mode(para);
    //OV7695PIPYUV_set_scene_mode(para);
    mDELAY(30);
}

BOOL OV5645PIP_set_param_effect(UINT16 para)
{
    OV5645_set_param_effect(para);
    OV7695PIPYUV_set_param_effect(para);
    mDELAY(30);
    return KAL_FALSE;
} /* OV5645PIP_set_param_effect */

BOOL OV5645PIP_set_param_banding(UINT16 para)
{
    OV5645_set_param_banding(para);
    OV7695PIPYUV_set_param_banding(para);
    mDELAY(30);
        return TRUE;
} /* OV5645PIP_set_param_banding */

BOOL OV5645PIP_set_param_exposure(UINT16 para)
{
    OV5645_set_param_exposure(para);
    OV7695PIPYUV_set_param_exposure(para);
    mDELAY(30);
    return TRUE;
} /* OV5645PIP_set_param_exposure */

UINT32 OV5645PIPYUVSensorSetting(FEATURE_ID iCmd, UINT32 iPara)
{
    switch (iCmd) {
    case FID_SCENE_MODE:
//        OV5645PIPSENSORDB("Set FID_SCENE_MODE:%d\n", iPara);
#if 1
        //OV5645PIP_set_scene_mode(iPara);
#else
        OV5645PIPSENSORDB("Night Mode:%d\n", iPara);
        if (iPara == SCENE_MODE_OFF)
        {
            OV5645PIP_night_mode(KAL_FALSE);
        } else if (iPara == SCENE_MODE_NIGHTSCENE) {
            OV5645PIP_night_mode(KAL_TRUE);
        }
#endif
        break;
    case FID_AWB_MODE:
//        OV5645PIPSENSORDB("Set AWB Mode:%d\n", iPara);
        OV5645PIP_set_param_wb(iPara);
        break;
    case FID_COLOR_EFFECT:
//        OV5645PIPSENSORDB("Set Color Effect:%d\n", iPara);
        OV5645PIP_set_param_effect(iPara);
        break;
    case FID_AE_EV:
 //       OV5645PIPSENSORDB("Set EV:%d\n", iPara);
        OV5645PIP_set_param_exposure(iPara);
        break;
    case FID_AE_FLICKER:
//        OV5645PIPSENSORDB("Set Flicker:%d\n", iPara);
        OV5645PIP_set_param_banding(iPara);
        break;
    case FID_AE_SCENE_MODE:
//        OV5645PIPSENSORDB("Set AE Mode:%d\n", iPara);
        if (iPara == AE_MODE_OFF) {
            spin_lock(&ov5645pip_drv_lock);
            OV5645PIP_AE_ENABLE = KAL_FALSE;
            spin_unlock(&ov5645pip_drv_lock);
        } else {
            spin_lock(&ov5645pip_drv_lock);
            OV5645PIP_AE_ENABLE = KAL_TRUE;
            spin_unlock(&ov5645pip_drv_lock);
        }
        OV5645PIP_set_AE_mode(OV5645PIP_AE_ENABLE);
        break;
    case FID_ISP_CONTRAST:
//        OV5645PIPSENSORDB("Set FID_ISP_CONTRAST:%d\n", iPara);
        OV5645PIP_set_contrast(iPara);
        break;
    case FID_ISP_BRIGHT:
//         OV5645PIPSENSORDB("Set FID_ISP_BRIGHT:%d\n", iPara);
        OV5645PIP_set_brightness(iPara);
        break;
    case FID_ISP_SAT:
//        OV5645PIPSENSORDB("Set FID_ISP_SAT:%d\n", iPara);
        OV5645PIP_set_saturation(iPara);
        break;
    case FID_ZOOM_FACTOR:
//        OV5645PIPSENSORDB("FID_ZOOM_FACTOR:%d\n", iPara);
        spin_lock(&ov5645pip_drv_lock);
        zoom_factor = iPara;
        spin_unlock(&ov5645pip_drv_lock);
        break;
    case FID_AE_ISO:
//        OV5645PIPSENSORDB("Set FID_AE_ISO:%d\n", iPara);
        OV5645PIP_set_iso(iPara);
        break;
    default:
        break;
    }
    return TRUE;
}   /* OV5645PIPYUVSensorSetting */

UINT32 OV5645PIPMaxFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 frameRate)
{
    kal_uint32 pclk;
    kal_int16 dummyLine;
    kal_uint16 lineLength,frameHeight;
    switch (scenarioId) {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        pclk = 56000000;
        lineLength = OV5645PIP_IMAGE_SENSOR_720P_WIDTH; //OV5645PIP_IMAGE_SENSOR_SVGA_WIDTH;
        frameHeight = (10 * pclk)/frameRate/lineLength;
        dummyLine = frameHeight - OV5645PIP_IMAGE_SENSOR_720P_HEIGHT; //OV5645PIP_IMAGE_SENSOR_SVGA_HEIGHT;
        if(dummyLine<0)
            dummyLine = 0;
        spin_lock(&ov5645pip_drv_lock);
        OV5645MIPISensor.SensorMode= SENSOR_MODE_PREVIEW;
        OV5645MIPISensor.PreviewDummyLines = dummyLine;
        spin_unlock(&ov5645pip_drv_lock);
        OV5645PIPSetDummy(OV5645MIPISensor.PreviewDummyPixels, dummyLine, 0);
        break;
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
        pclk = 56000000;
        lineLength = OV5645PIP_IMAGE_SENSOR_VIDEO_WITDH;
        frameHeight = (10 * pclk)/frameRate/lineLength;
        dummyLine = frameHeight - OV5645PIP_IMAGE_SENSOR_VIDEO_HEIGHT;
        if(dummyLine<0)
            dummyLine = 0;
        //spin_lock(&ov5645pip_drv_lock);
        //ov8825.sensorMode = SENSOR_MODE_VIDEO;
        //spin_unlock(&ov5645pip_drv_lock);
        OV5645PIPSetDummy(0, dummyLine, 0);
        break;
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
    case MSDK_SCENARIO_ID_CAMERA_ZSD:
        pclk = 90000000;
        lineLength = OV5645PIP_IMAGE_SENSOR_QSXGA_WITDH;
        frameHeight = (10 * pclk)/frameRate/lineLength;
        dummyLine = frameHeight - OV5645PIP_IMAGE_SENSOR_QSXGA_HEIGHT;
        if(dummyLine<0)
            dummyLine = 0;
        spin_lock(&ov5645pip_drv_lock);
        OV5645MIPISensor.CaptureDummyLines = dummyLine;
        OV5645MIPISensor.SensorMode= SENSOR_MODE_CAPTURE;
        spin_unlock(&ov5645pip_drv_lock);
        OV5645PIPSetDummy(OV5645MIPISensor.CaptureDummyPixels, dummyLine, 0);
        break;
    case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: //added
        break;
    case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
        break;
    case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: //added
        break;
    default:
        break;
    }
    return ERROR_NONE;
}

UINT32 OV5645PIPFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
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
	
    switch (FeatureId)
    {
    case SENSOR_FEATURE_GET_RESOLUTION:
        *pFeatureReturnPara16++=OV5645PIP_IMAGE_SENSOR_QSXGA_WITDH;
        *pFeatureReturnPara16=OV5645PIP_IMAGE_SENSOR_QSXGA_HEIGHT;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_GET_PERIOD:
#if 1//defined(MT6577)
        switch(CurrentScenarioId)
        {
        case MSDK_SCENARIO_ID_CAMERA_ZSD:
            //OV5645PIPSENSORDB("OV5645PIPFeatureControl:SENSOR_FEATURE_GET_PERIOD \n");
            *pFeatureReturnPara16++=OV5645PIP_FULL_PERIOD_PIXEL_NUMS + OV5645MIPISensor.CaptureDummyPixels;
            *pFeatureReturnPara16=OV5645PIP_FULL_PERIOD_LINE_NUMS + OV5645MIPISensor.CaptureDummyLines;
            *pFeatureParaLen=4;
            break;
        default:
            //OV5645PIPSENSORDB("OV5645PIPFeatureControl:SENSOR_FEATURE_GET_PERIOD \n");
            *pFeatureReturnPara16++=OV5645PIP_PV_PERIOD_PIXEL_NUMS + OV5645MIPISensor.PreviewDummyPixels;
            *pFeatureReturnPara16=OV5645PIP_PV_PERIOD_LINE_NUMS + OV5645MIPISensor.PreviewDummyLines;
            *pFeatureParaLen=4;
            break;
        }
#else
        //OV5645PIPSENSORDB("OV5645PIPFeatureControl:SENSOR_FEATURE_GET_PERIOD \n");
        *pFeatureReturnPara16++=OV5645PIP_PV_PERIOD_PIXEL_NUMS + OV5645MIPISensor.PreviewDummyPixels;
        *pFeatureReturnPara16=OV5645PIP_PV_PERIOD_LINE_NUMS + OV5645MIPISensor.PreviewDummyLines;
        *pFeatureParaLen=4;
#endif
        break;
    case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
#if 1//defined(MT6577)
        switch(CurrentScenarioId)
        {
        case MSDK_SCENARIO_ID_CAMERA_ZSD:
            //OV5645PIPSENSORDB("OV5645PIPFeatureControl:SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ \n");
            *pFeatureReturnPara32 = OV5645MIPISensor.CapturePclk * 1000 *100;     //unit: Hz
            *pFeatureParaLen=4;
            break;
        default:
            //OV5645PIPSENSORDB("OV5645PIPFeatureControl:SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ \n");
            *pFeatureReturnPara32 = OV5645MIPISensor.PreviewPclk * 1000 *100;     //unit: Hz
            *pFeatureParaLen=4;
            break;
        }
#else
        //OV5645PIPSENSORDB("OV5645PIPFeatureControl:SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ \n");
        *pFeatureReturnPara32 = OV5645MIPISensor.PreviewPclk * 1000 *100;     //unit: Hz
        *pFeatureParaLen=4;
#endif
        break;
    case SENSOR_FEATURE_SET_ESHUTTER:
        //OV5645PIPSENSORDB("OV5645PIPFeatureControl:SENSOR_FEATURE_SET_ESHUTTER \n");
        break;
    case SENSOR_FEATURE_SET_NIGHTMODE:
        //OV5645PIPSENSORDB("OV5645PIPFeatureControl:SENSOR_FEATURE_SET_NIGHTMODE \n");
        //OV5645PIP_night_mode((BOOL) *pFeatureData16);
        break;
    case SENSOR_FEATURE_SET_GAIN:
        //OV5645PIPSENSORDB("OV5645PIPFeatureControl:SENSOR_FEATURE_SET_GAIN \n");
        break;
    case SENSOR_FEATURE_GET_AE_FLASHLIGHT_INFO:
        //OV5645PIPSENSORDB("SENSOR_FEATURE_GET_AE_FLASHLIGHT_INFO \n");
        OV5645PIP_GetAEFlashlightInfo(*pFeatureData32);
        break;
    case SENSOR_FEATURE_SET_FLASHLIGHT:
        //OV5645PIPSENSORDB("OV5645PIPFeatureControl:SENSOR_FEATURE_SET_FLASHLIGHT \n");
        break;
    case SENSOR_FEATURE_GET_TRIGGER_FLASHLIGHT_INFO:
        OV5645_FlashTriggerCheck(pFeatureData32);
        //OV5645PIPSENSORDB("SENSOR_FEATURE_GET_TRIGGER_FLASHLIGHT_INFO \n");
        break;
    case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
        //OV5645PIPSENSORDB("OV5645PIPFeatureControl:SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ \n");
        break;
    case SENSOR_FEATURE_SET_REGISTER:
        //OV5645PIPSENSORDB("OV5645PIPFeatureControl:SENSOR_FEATURE_SET_REGISTER \n");
        OV5645PIP_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
        break;
    case SENSOR_FEATURE_GET_REGISTER:
        //OV5645PIPSENSORDB("OV5645PIPFeatureControl:SENSOR_FEATURE_GET_REGISTER \n");
        pSensorRegData->RegData = OV5645PIP_read_cmos_sensor(pSensorRegData->RegAddr);
        break;
    case SENSOR_FEATURE_GET_CONFIG_PARA:
        //OV5645PIPSENSORDB("OV5645PIPFeatureControl:SENSOR_FEATURE_GET_CONFIG_PARA \n");
        memcpy(pSensorConfigData, &OV5645PIPSensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
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
        //OV5645PIPSENSORDB("OV5645PIPFeatureControl:SENSOR_FEATURE_SET/get_CCT_xxxx ect \n");
        break;
    case SENSOR_FEATURE_GET_GROUP_COUNT:
        *pFeatureReturnPara32++=0;
        *pFeatureParaLen=4;
        //OV5645PIPSENSORDB("OV5645PIPFeatureControl:SENSOR_FEATURE_GET_GROUP_COUNT \n");
        break;
    case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
        //OV5645PIPSENSORDB("OV5645PIPFeatureControl:SENSOR_FEATURE_GET_LENS_DRIVER_ID \n");
        // get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
        // if EEPROM does not exist in camera module.
        *pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_CHECK_SENSOR_ID:
        //OV5645PIPSENSORDB("OV5645PIPFeatureControl:SENSOR_FEATURE_CHECK_SENSOR_ID \n");
        //OV5645PIP_GetSensorID(pFeatureData32, (pFeatureData32+1));
        OV5645PIP_GetSensorID(pFeatureData32);
        break;
    case SENSOR_FEATURE_SET_YUV_CMD:
        //OV5645PIPSENSORDB("SENSOR_FEATURE_SET_YUV_CMD \n");
        OV5645PIPYUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));
        break;
    case SENSOR_FEATURE_SET_YUV_3A_CMD:
        OV5645_3ACtrl((ACDK_SENSOR_3A_LOCK_ENUM)*pFeatureData32);
        break;
    case SENSOR_FEATURE_SET_VIDEO_MODE:
        OV5645PIPSENSORDB("SENSOR_FEATURE_SET_VIDEO_MODE \n");
        OV5645PIPYUVSetVideoMode(*pFeatureData16);
        break;
    case SENSOR_FEATURE_GET_EV_AWB_REF:
        OV5645GetEvAwbRef(*pFeatureData32);
        break;
    case SENSOR_FEATURE_GET_SHUTTER_GAIN_AWB_GAIN:
        OV5645GetCurAeAwbInfo(*pFeatureData32);
        break;
    case SENSOR_FEATURE_GET_EXIF_INFO:
        OV5645GetExifInfo(*pFeatureData32);
        break;
    case SENSOR_FEATURE_GET_DELAY_INFO:
        //OV5645PIPSENSORDB("[OV5645]SENSOR_FEATURE_GET_DELAY_INFO\n");
        OV5645_GetDelayInfo(*pFeatureData32);
        break;
    case SENSOR_FEATURE_SET_SLAVE_I2C_ID:
         //OV5645PIPSENSORDB("[OV5645]SENSOR_FEATURE_SET_SLAVE_I2C_ID\n");
         //OV5645_sensor_socket = *pFeatureData32;
         break;
    case SENSOR_FEATURE_SET_TEST_PATTERN:
        //OV5645PIPSENSORDB("[OV5645]SENSOR_FEATURE_SET_TEST_PATTERN\n");
        OV5645PIPSetTestPatternMode((BOOL)*pFeatureData16);
        break;
    case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE:
        //OV5645PIPSENSORDB("[OV5645]OV5645_TEST_PATTERN_CHECKSUM\n");
        *pFeatureReturnPara32=OV5645_TEST_PATTERN_CHECKSUM;
        *pFeatureParaLen=4;
        break;

    case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
//        OV5645PIPMaxFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32,*(pFeatureData32+1));
        break;
    case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
        OV5645GetDefaultFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32,(MUINT32 *)*(pFeatureData32+1));
        break;
    /**********************below is AF control**********************/
    case SENSOR_FEATURE_INITIALIZE_AF:
    case SENSOR_FEATURE_MOVE_FOCUS_LENS:
    case SENSOR_FEATURE_GET_AF_STATUS:
    case SENSOR_FEATURE_SINGLE_FOCUS_MODE:
    case SENSOR_FEATURE_CONSTANT_AF:
    case SENSOR_FEATURE_CANCEL_AF:
    case SENSOR_FEATURE_GET_AF_INF:
    case SENSOR_FEATURE_GET_AF_MACRO:
    case SENSOR_FEATURE_SET_AF_WINDOW:
    case SENSOR_FEATURE_GET_AF_MAX_NUM_FOCUS_AREAS:
    case SENSOR_FEATURE_GET_AE_AWB_LOCK_INFO:
    case SENSOR_FEATURE_GET_AE_MAX_NUM_METERING_AREAS:
    case SENSOR_FEATURE_SET_AE_WINDOW:
        if (OV5645MIPISensor.is_support_af) {
            switch (FeatureId) {
            case SENSOR_FEATURE_INITIALIZE_AF:
                //OV5645PIPSENSORDB("SENSOR_FEATURE_INITIALIZE_AF\n");
                OV5645_FOCUS_OVT_AFC_Init();
                break;
            case SENSOR_FEATURE_MOVE_FOCUS_LENS:
        //        OV5645PIPSENSORDB("[OV5645]SENSOR_FEATURE_MOVE_FOCUS_LENS\n");
                //OV5645_FOCUS_Move_to(*pFeatureData16);
                break;
            case SENSOR_FEATURE_GET_AF_STATUS:
                //OV5645PIPSENSORDB("SENSOR_FEATURE_GET_AF_STATUS\n");
                OV5645_FOCUS_OVT_AFC_Get_AF_Status(pFeatureReturnPara32);
                *pFeatureParaLen=4;
                break;
            case SENSOR_FEATURE_SINGLE_FOCUS_MODE:
                //OV5645PIPSENSORDB("SENSOR_FEATURE_SINGLE_FOCUS_MODE\n");
                OV5645_FOCUS_OVT_AFC_Single_Focus();
                break;
            case SENSOR_FEATURE_CONSTANT_AF:
                //OV5645PIPSENSORDB("SENSOR_FEATURE_CONSTANT_AF\n");
                OV5645_FOCUS_OVT_AFC_Constant_Focus();
                break;
            case SENSOR_FEATURE_CANCEL_AF:
                //OV5645PIPSENSORDB("SENSOR_FEATURE_CANCEL_AF\n");
                OV5645_FOCUS_OVT_AFC_Cancel_Focus();
                break;
            case SENSOR_FEATURE_GET_AF_INF:
               // OV5645PIPSENSORDB("SENSOR_FEATURE_GET_AF_INF\n");
                OV5645_FOCUS_Get_AF_Inf(pFeatureReturnPara32);
                *pFeatureParaLen=4;
                break;
            case SENSOR_FEATURE_GET_AF_MACRO:
                //OV5645PIPSENSORDB("SENSOR_FEATURE_GET_AF_MACRO\n");
                OV5645_FOCUS_Get_AF_Macro(pFeatureReturnPara32);
                *pFeatureParaLen=4;
                break;
            case SENSOR_FEATURE_SET_AF_WINDOW:
                //OV5645PIPSENSORDB("SENSOR_FEATURE_SET_AF_WINDOW\n");
                OV5645_FOCUS_Set_AF_Window(*pFeatureData32);
                //OV5645_FOCUS_OVT_AFC_Single_Focus();
                break;
            case SENSOR_FEATURE_GET_AF_MAX_NUM_FOCUS_AREAS:
                //OV5645PIPSENSORDB("SENSOR_FEATURE_GET_AF_MAX_NUM_FOCUS_AREAS\n");
                OV5645_FOCUS_Get_AF_Max_Num_Focus_Areas(pFeatureReturnPara32);
                *pFeatureParaLen=4;
                break;
            case SENSOR_FEATURE_GET_AE_AWB_LOCK_INFO:
                //OV5645PIPSENSORDB("SENSOR_FEATURE_GET_AF_STATUS\n");
                OV5645_get_AEAWB_lock(pFeatureData32, (pFeatureData32+1));
                break;
            case SENSOR_FEATURE_GET_AE_MAX_NUM_METERING_AREAS:
                //OV5645PIPSENSORDB("AE zone addr = 0x%x\n",*pFeatureData32);
                OV5645_FOCUS_Get_AE_Max_Num_Metering_Areas(pFeatureReturnPara32);
                *pFeatureParaLen=4;
                break;
            case SENSOR_FEATURE_SET_AE_WINDOW:
                //OV5645PIPSENSORDB("AE zone addr = 0x%x\n",*pFeatureData32);
                OV5645PIP_FOCUS_Set_AE_Window(*pFeatureData32);
                break;
            }
        }
        break;
#if defined(TINNO_PIP_SUPPORT)
	case SENSOR_FEATURE_SET_SENSOR_PIP_FRONT_CAMERA_WINDOW:
		OV5645PIPSENSORDB("[Jieve] Set PIP WINDOW para1=%d\n", *pFeatureData32);
			OV5645_PIP_Set_Window(*pFeatureData32);
		break;
#endif
    case SENSOR_FEATURE_GET_SENSOR_NAME:
		{/*MAX LEN is 31 char*/
			*pFeaturePara = '\0';
			strcat(pFeaturePara, SENSOR_DRVNAME_OV5645PIP_MIPI_YUV );
			if ( OV5645_MID_DARLING == OV5645MIPISensor.module_id ){
				strcat(pFeaturePara, "_daling" );
				if (OV5645_LENS_ID_EP_D420F00 == OV5645MIPISensor.lens_id
					|| OV5645_LENS_ID_UNKNOW_FOR_DARLING_AF == OV5645MIPISensor.lens_id) {
					strcat(pFeaturePara, "_AF" );
				} else /*if (OV5645_LENS_ID_CHT_833C2008 == OV5645MIPISensor.lens_id) */{
					strcat(pFeaturePara, "_FF" );
				} 
			}else if( OV5645_MID_TRULY == OV5645MIPISensor.module_id ){
				strcat(pFeaturePara, "_truly" );
				if (OV5645_LENS_ID_SHUNYU_3515 == OV5645MIPISensor.lens_id) {
					strcat(pFeaturePara, "_FF" );
				} else if ( OV5645_LENS_ID_HOKUANG_GBCA2128201 == OV5645MIPISensor.lens_id ){
					strcat(pFeaturePara, "_AF" );
				}
			}else{
				strcat(pFeaturePara, "_unknown" );
			}

			*pFeatureParaLen = strlen(pFeaturePara);
			break;
		}
	default:
		OV5645PIPSENSORDB("OV5645PIPFeatureControl:default \n");
		break;			
	}
	//OV5645PIPSENSORDB("[OV5645PIP]OV5645PIPFeatureControl exit :\n ");
	return ERROR_NONE;
}	/* OV5645PIPFeatureControl() */

SENSOR_FUNCTION_STRUCT	SensorFuncOV5645PIP=
{
	OV5645PIPOpen,
	OV5645PIPGetInfo,
	OV5645PIPGetResolution,
	OV5645PIPFeatureControl,
	OV5645PIPControl,
	OV5645PIPClose
};

extern int g_i4PipMode;
extern SENSOR_FUNCTION_STRUCT	SensorFuncOV5645MIPI;

UINT32 OV5645PIP_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	/* To Do : Check Sensor status here */
	OV5645PIPSENSORDB("g_i4PipMode=%d\n", g_i4PipMode);
	if (pfFunc!=NULL){
		if ( g_i4PipMode ){
			*pfFunc=&SensorFuncOV5645PIP;
		}else{
			*pfFunc=&SensorFuncOV5645MIPI;
		}
	}
	return ERROR_NONE;
}	/* SensorInit() */

#ifdef OV5645PIP_SWITCH_CAMERA
static struct kobject *ov564pip_sensor_device;

static ssize_t ov5645pip_switch_camera_show(struct device *dev,
                struct device_attribute *attr,
                char *buf)
{
	ssize_t rc = 0;
	OV5645PIPSENSORDB("buf = %s:\n ", buf);
	OV5645PIPUpdateCameraStatus();
	return rc;
}
static ssize_t ov5645pip_switch_camera_store(struct device *dev,
                struct device_attribute *attr,
                const char *buf, size_t size)
{
	int rc = 0;
	unsigned long x, y, z;
	char *p = buf, *q;
	OV5645PIPSENSORDB("buf = %s:\n ", buf);
	q = p;
	p = strstr(p, ",");
	if(p == NULL){
		return rc;
	}
	*p = '\0';
	p = p +1;
	rc = kstrtoul(q, 10, &x);
#ifdef OV5645PIP_TEST_PIP_WIN
	g_sleep_count = x;
#endif
	if (rc){
		OV5645PIPSENSORDB("ERR x: buf = %s:\n ", q);
		return rc;
	}
	q = p;
	p = strstr(p, ",");
	if(p == NULL){
		return rc;
	}
	*p = '\0';
	p = p +1;
	rc = kstrtoul(q, 10, &y);
	if (rc){
		OV5645PIPSENSORDB("ERR y: buf = %s:\n ", q);
		return rc;
	}
	
	rc = kstrtoul(p, 10, &z);
	if (rc){
		OV5645PIPSENSORDB("ERR z: buf = %s:\n ", z);
		return rc;
	}
	
	OV5645PIPSENSORDB("x = %ld, y = %ld, z = %ld\n ", x, y, z);
	OV5645_PIP_Set_Frame_Color(x, y, z);
	return size;
}

static DEVICE_ATTR(ov5645pip, 0666, ov5645pip_switch_camera_show, ov5645pip_switch_camera_store);

static int init_sys_device_name(void)
{
    int rc = 0;
    static int is_first_in = 1;

    if (!is_first_in) {
        return 0;
    }
    is_first_in = 0;
    ov564pip_sensor_device = kobject_create_and_add("camera_pip2", NULL);
    if (ov564pip_sensor_device == NULL) {
        pr_info("%s: subsystem_register failed\n", __func__);
        rc = -ENXIO;
        return rc ;
    }
    rc = sysfs_create_file(ov564pip_sensor_device, &dev_attr_ov5645pip.attr);
    if (rc) {
        pr_info("%s: sysfs_create_file failed\n", __func__);
        kobject_del(ov564pip_sensor_device);
    }

    return rc;
}
#endif

