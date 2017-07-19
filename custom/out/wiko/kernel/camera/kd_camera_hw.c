#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/xlog.h>

#include "kd_camera_hw.h"

#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_camera_feature.h"

/******************************************************************************
 * Debug configuration
******************************************************************************/
#define PFX "[kd_camera_hw]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, arg...)    printk(PFX "%s: " fmt, __FUNCTION__ ,##arg)

#define DEBUG_CAMERA_HW_K
#ifdef  DEBUG_CAMERA_HW_K
#define PK_DBG PK_DBG_FUNC
#define PK_ERR(fmt, arg...)         printk(KERN_ERR PFX "%s: " fmt, __FUNCTION__ ,##arg)
#define PK_XLOG_INFO(fmt, args...) \
        do {    \
                    xlog_printk(ANDROID_LOG_INFO, "kd_camera_hw", fmt, ##args); \
        } while(0)
#else
#define PK_DBG(a,...)
#define PK_ERR(a,...)
#define PK_XLOG_INFO(fmt, args...)
#endif

#if defined(TINNO_PIP_SUPPORT)
	extern u32 gSearchSendor;
#endif
static u32 subCamPDNStatus = 0;

#define IDX_PS_CMRST 0
#define IDX_PS_CMPDN 4
#define IDX_PS_XCMPDN 8

#define IDX_PS_MODE 1
#define IDX_PS_ON   2
#define IDX_PS_OFF  3
static u32 pinSet[2][12] = {
      //for main sensor
      {
	GPIO_CAMERA_CMRST_PIN,
		GPIO_CAMERA_CMRST_PIN_M_GPIO,   /* mode */
		GPIO_OUT_ONE,                   /* ON state */
		GPIO_OUT_ZERO,                  /* OFF state */
	GPIO_CAMERA_CMPDN_PIN,
		GPIO_CAMERA_CMPDN_PIN_M_GPIO,
		GPIO_OUT_ONE,
		GPIO_OUT_ZERO,
	GPIO_CAMERA_CMPDN1_PIN,
		GPIO_CAMERA_CMPDN1_PIN_M_GPIO,
		GPIO_OUT_ONE, 
		GPIO_OUT_ZERO,
      },
#if defined(TINNO_PIP_SUPPORT)
      //for sub sensor
      {
	GPIO_CAMERA_CMRST1_PIN,
		GPIO_CAMERA_CMRST1_PIN_M_GPIO,
		GPIO_OUT_ONE,
		GPIO_OUT_ZERO,
	GPIO_CAMERA_CMPDN1_PIN,
		GPIO_CAMERA_CMPDN1_PIN_M_GPIO,
		GPIO_OUT_ONE, 
		GPIO_OUT_ZERO,
      }
#else
      //for sub sensor
      {
	GPIO_CAMERA_CMRST1_PIN,
		GPIO_CAMERA_CMRST1_PIN_M_GPIO,
		//GPIO_OUT_ZERO,
		GPIO_OUT_ONE,
		GPIO_OUT_ZERO,   
	GPIO_CAMERA_CMPDN1_PIN,
		GPIO_CAMERA_CMPDN1_PIN_M_GPIO,
		GPIO_OUT_ZERO,
		GPIO_OUT_ONE, 
		//GPIO_OUT_ZERO,         
      }
#endif
};

void kdGPIOSet(int pinSetIdx, int pinOffset, int pinValue )
{
	if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][pinOffset])
	{
              PK_DBG("[CAMERA kdGPIOSet] pinSetIdx:%d; pinNo=%d; pinValue=%d\n",
			  	pinSetIdx, pinSet[pinSetIdx][pinOffset], pinSet[pinSetIdx][pinOffset+pinValue]);
		if(mt_set_gpio_mode(pinSet[pinSetIdx][pinOffset],
			pinSet[pinSetIdx][pinOffset+IDX_PS_MODE])){
			PK_DBG("[CAMERA kdGPIOSet] set gpio mode failed!! \n");
		}
		if(mt_set_gpio_dir(pinSet[pinSetIdx][pinOffset],
			GPIO_DIR_OUT)){
			PK_DBG("[CAMERA kdGPIOSet] set gpio dir failed!! \n");
		}
		if(mt_set_gpio_out(pinSet[pinSetIdx][pinOffset],
			pinSet[pinSetIdx][pinOffset+pinValue])){
			PK_DBG("[CAMERA kdGPIOSet] set gpio value failed!! \n");
		}
	}
}

int kdCamPowerOn( CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorIdx, int iPowerOn, char* mode_name )
{
	PK_DBG("[CAMERA SENSOR] SensorIdx = %d, PowerOn=%d\n", SensorIdx, iPowerOn);
	if ( iPowerOn ){
		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_1800,mode_name)){
			PK_ERR("[CAMERA SENSOR] Fail to enable digital core power\n");
			return -EIO;
		}

		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name)){
			PK_ERR("[CAMERA SENSOR] Fail to enable analog power\n");
			return -EIO;
		}

		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1200,mode_name)){
			PK_ERR("[CAMERA SENSOR] Fail to enable digital power\n");
			return -EIO;
		}
		msleep(5);
	}
	else{
		if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D, mode_name)) {
			PK_ERR("[CAMERA SENSOR] Fail to OFF digital power\n");
			return -EIO;
		}

		if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
			PK_ERR("[CAMERA SENSOR] Fail to OFF analog power\n");
			return -EIO;
		}

		if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D2,mode_name))
		{
			PK_ERR("[CAMERA SENSOR] Fail to enable analog core power\n");
			return -EIO;
		}
	}
	return 0;
}

int kdCISModulePIPFrontSensorPowerOn(int iFrontSensorStatus)
{
	if ( iFrontSensorStatus ){
		kdGPIOSet(0, IDX_PS_XCMPDN, IDX_PS_ON);
	}else{
		kdGPIOSet(0, IDX_PS_XCMPDN, IDX_PS_OFF);
	}
}

int ov5645_mipi_yuv_af_poweron(BOOL On)
{
#if defined(TINNO_PIP_SUPPORT)
	if ( gSearchSendor ){
		PK_ERR("[CAMERA SENSOR AF] gSearchSendor=%d, Abort!\n", gSearchSendor);
		return -ENOSYS;
	}
#endif
	if ( On ){
		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A2, VOL_2800, "OV5645_AF")){
			PK_ERR("[CAMERA SENSOR AF] Fail to enable lens analog power\n");
			return -EIO;
		}
	}else{
		if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A2,"OV5645_AF"))
		{
			PK_ERR("[CAMERA SENSOR AF] Fail to enable analog power\n");
			return -EIO;
		}
	}
	return 0;
}


#if defined(TINNO_PIP_SUPPORT)
extern int g_i4PipMode;
#endif
int kdCISModulePowerOn(CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorIdx, char *currSensorName, BOOL On, char* mode_name)
{
	u32 pinSetIdx = 0;//default main sensor

#if defined(TINNO_PIP_SUPPORT)
	PK_DBG("<Jieve>kdCISModulePowerOn: SensorIdx=%d, g_i4PipMode=%d, gSearchSendor=%d\n", 
		SensorIdx, g_i4PipMode, gSearchSendor);
#else
	PK_DBG("<Jieve>kdCISModulePowerOn: SensorIdx=%d\n", 
		SensorIdx);
#endif
	if (DUAL_CAMERA_MAIN_SENSOR == SensorIdx){
		pinSetIdx = 0;
	}
	else if (DUAL_CAMERA_SUB_SENSOR == SensorIdx) {
		pinSetIdx = 1;
	}


    //power ON
	if (On) {
		PK_DBG("kdCISModulePowerOn -on:currSensorName=%s;\n",currSensorName);

		if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV5645_MIPI_YUV,currSensorName)
			                           ||0 == strcmp(SENSOR_DRVNAME_OV5645PIP_MIPI_YUV,currSensorName) )){
			PK_DBG("[CAMERA SENSOR] kdCISModulePowerOn get in---"
			"sensorIdx:%d; pinSetIdx=%d\n",SensorIdx, pinSetIdx);

			kdGPIOSet(pinSetIdx, IDX_PS_CMPDN, IDX_PS_OFF);
			kdGPIOSet(pinSetIdx, IDX_PS_CMRST, IDX_PS_OFF);
			if ( kdCamPowerOn(SensorIdx, 1, mode_name) ){
				PK_ERR("kdCamPowerOn -on: failed;\n");
				goto _kdCISModulePowerOn_exit_;
			}

			//PDN pin
			kdGPIOSet(pinSetIdx, IDX_PS_CMPDN, IDX_PS_ON);
			msleep(3);

			//RST pin
			kdGPIOSet(pinSetIdx, IDX_PS_CMRST, IDX_PS_ON);
			msleep(3);
#if defined(TINNO_PIP_SUPPORT)
			if ( 0 == gSearchSendor ){//Normal Power on
				kdGPIOSet(pinSetIdx, IDX_PS_XCMPDN, IDX_PS_ON);
			}
#endif
		}
		else if (currSensorName && 0 == strcmp(SENSOR_DRVNAME_OV7695PIP_YUV,currSensorName)){  
			PK_DBG("[CAMERA SENSOR] kdCISModulePowerOn get in---sensorIdx:%d; pinSetIdx=%d\n",SensorIdx, pinSetIdx);
#if defined(TINNO_PIP_SUPPORT)
			if ( 0 == gSearchSendor ){//Normal Power on
				//Power On Main Camera also...
				kdGPIOSet(0, IDX_PS_CMPDN, IDX_PS_OFF);
				kdGPIOSet(0, IDX_PS_CMRST, IDX_PS_OFF);
			}
#endif
			
			if ( kdCamPowerOn(SensorIdx, 1, mode_name) ){
				PK_ERR("kdCamPowerOn -on: failed;\n");
				goto _kdCISModulePowerOn_exit_;
			}
#if defined(TINNO_PIP_SUPPORT)
			if ( 0 == gSearchSendor ){//Normal Power on
				//Power On Main Camera also...
				//PDN pin
				kdGPIOSet(0, IDX_PS_CMPDN, IDX_PS_ON);
				msleep(3);
				//RST pin
				kdGPIOSet(0, IDX_PS_CMRST, IDX_PS_ON);
				msleep(3);
			}
#endif
			//=================================================
			//=================================================
			//Power On Sub Camera ...
			//PDN/STBY pin
			kdGPIOSet(pinSetIdx, IDX_PS_CMPDN, IDX_PS_OFF);
			msleep(10);
			kdGPIOSet(pinSetIdx, IDX_PS_CMPDN, IDX_PS_ON);
			msleep(5);

			kdGPIOSet(pinSetIdx, IDX_PS_CMRST, IDX_PS_OFF);
			msleep(10);
			kdGPIOSet(pinSetIdx, IDX_PS_CMRST, IDX_PS_ON);
			msleep(5);
		}
		else if (currSensorName && 0 == strcmp(SENSOR_DRVNAME_OV7695_MIPI_YUV,currSensorName)){  
			PK_DBG("[CAMERA SENSOR] kdCISModulePowerOn get in---sensorIdx:%d; pinSetIdx=%d\n",SensorIdx, pinSetIdx);

			if ( kdCamPowerOn(SensorIdx, 1, mode_name) ){
				PK_ERR("kdCamPowerOn -on: failed;\n");
				goto _kdCISModulePowerOn_exit_;
			}

			//PDN/STBY pin
			kdGPIOSet(pinSetIdx, IDX_PS_CMPDN, IDX_PS_OFF);
			msleep(10);
			kdGPIOSet(pinSetIdx, IDX_PS_CMPDN, IDX_PS_ON);
			msleep(5);

			kdGPIOSet(pinSetIdx, IDX_PS_CMRST, IDX_PS_OFF);
			msleep(10);
			kdGPIOSet(pinSetIdx, IDX_PS_CMRST, IDX_PS_ON);
			msleep(5);
		}
		else if ((currSensorName && (0 == strcmp(SENSOR_DRVNAME_HI704_YUV,currSensorName)))
			||(currSensorName && (0 == strcmp(SENSOR_DRVNAME_SP0A19_YUV,currSensorName))))
       {  
          PK_DBG("[CAMERA SENSOR] kdCISModulePowerOn get in---HI704_YUV sensorIdx:%d; pinSetIdx=%d\n",SensorIdx, pinSetIdx);
       
          if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_1800/*VOL_2800*/,mode_name))
          {
             PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
             //return -EIO;
             goto _kdCISModulePowerOn_exit_;
          }
       
          if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
          {
             PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
             //return -EIO;
             goto _kdCISModulePowerOn_exit_;
          }
       
          if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1500,mode_name)) //VOL_1200
          {
             PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
             //return -EIO;
             goto _kdCISModulePowerOn_exit_;
          }
/*
          if(mainCamAFPowerFlag==1) {
             if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A2,mode_name))
             {
                 PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
                 //return -EIO;
                 goto _kdCISModulePowerOn_exit_;
             }
             mainCamAFPowerFlag = 0;
          }
*/
          /*if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A2, VOL_2800,mode_name))
          {
             PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
             //return -EIO;
             goto _kdCISModulePowerOn_exit_;
          }*/
       
          //PDN/STBY pin
          if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
          {
             if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
             if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
             if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
             msleep(10);
             if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
             msleep(5);
       
             //RST pin
             if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
             if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
             if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
             mdelay(10);
             if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
             mdelay(5);
          }
       
          //disable inactive sensor
          if(pinSetIdx == 0) {//disable sub
             pinSetIdx = 1;
          }
          else{
             pinSetIdx = 0;
          }
          if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
             if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
             if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
             if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
             if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
             if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
             if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
          }          
       }
		else{
			PK_ERR("kdCISModulePowerOn -No Handle! currSensorName=%s\n",currSensorName);
		}
	}
	else {//power OFF
		PK_DBG("kdCISModulePowerOn -off:currSensorName=%s\n",currSensorName);
		if((currSensorName && (0 == strcmp(SENSOR_DRVNAME_HI704_YUV,currSensorName)))
			||(currSensorName && (0 == strcmp(SENSOR_DRVNAME_SP0A19_YUV,currSensorName)))){
		 if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
           		if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
           		if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
           		if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
       
           		if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
           		if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
          		if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
       	}

	       if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D, mode_name)) {
	           PK_DBG("[CAMERA SENSOR] Fail to OFF digital power\n");
	           //return -EIO;
	           goto _kdCISModulePowerOn_exit_;
	       }
       
	       if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
	           PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
	           //return -EIO;
	           goto _kdCISModulePowerOn_exit_;
	       }
       
	       if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D2,mode_name))
	       {
	           PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
	           //return -EIO;
	           goto _kdCISModulePowerOn_exit_;
	       }
		   
	       //if (subCamPDNStatus == 1){
	           mt_set_gpio_mode(GPIO_CAMERA_CMPDN1_PIN,GPIO_MODE_00);
	           mt_set_gpio_dir(GPIO_CAMERA_CMPDN1_PIN,GPIO_DIR_OUT);
	           mt_set_gpio_out(GPIO_CAMERA_CMPDN1_PIN,GPIO_OUT_ZERO);
       	       //}
		   
		}
		else{
			kdGPIOSet(pinSetIdx, IDX_PS_CMRST, IDX_PS_OFF);
			kdGPIOSet(pinSetIdx, IDX_PS_CMPDN, IDX_PS_OFF);
			if ( kdCamPowerOn(SensorIdx, 0, mode_name) ){
				PK_ERR("kdCamPowerOn -off: failed;\n");
				goto _kdCISModulePowerOn_exit_;
			}
		}
		
	}

  return 0;
_kdCISModulePowerOn_exit_:
    return -EIO;
}


EXPORT_SYMBOL(kdCISModulePowerOn);

void SubCameraDigtalPDNCtrl(u32 onoff){
    subCamPDNStatus = onoff;
}
EXPORT_SYMBOL(SubCameraDigtalPDNCtrl);

#include "cust_adc.h"


extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);
extern int IMM_IsAdcInitReady(void);

#ifdef AUXADC_CAM_ID_SUB_CHANNEL
#undef AUXADC_CAM_ID_SUB_CHANNEL
#endif
#define AUXADC_CAM_ID_SUB_CHANNEL (15)

int kdCamADCReadChipID(int camID, int min, int max)
{
	int data[4] = {0, 0, 0, 0};
	int tmp = 0, rc = 0, iVoltage = 0;
	int dwChannel = -1;
	if ( 0 == camID ){
		dwChannel = AUXADC_CAM_ID_MAIN_CHANNEL;
	}else if ( 1 == camID ){
		dwChannel = AUXADC_CAM_ID_SUB_CHANNEL;
	}
	if( IMM_IsAdcInitReady() == 0 )
	{
		PK_ERR("AUXADC is not ready\n");
		return 0;
	}

	rc = IMM_GetOneChannelValue(dwChannel, data, &tmp);
	if(rc < 0) {
		PK_ERR("read CAM_ID vol error--Jieve\n");
		return 0;
	}
	else {
		iVoltage = (data[0]*1000) + (data[1]*10) + (data[2]);
		PK_ERR("read CAM_ID (%d) success, data[0]=%d, data[1]=%d, data[2]=%d, data[3]=%d, iVoltage=%d\n", 
			AUXADC_CAM_ID_SUB_CHANNEL, data[0], data[1], data[2], data[3], iVoltage);
		if(	min < iVoltage &&
			iVoltage <= max)
			return 1;
		else
			return 0;
	}
	return 0;
}



//!--
//
