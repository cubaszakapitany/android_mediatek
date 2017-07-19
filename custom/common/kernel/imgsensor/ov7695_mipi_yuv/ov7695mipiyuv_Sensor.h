/*****************************************************************************
 *
 * Filename:
 * ---------
 *   sensor.h
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *   Header file of Sensor driver
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
 * 09 10 2010 jackie.su
 * [ALPS00002279] [Need Patch] [Volunteer Patch] ALPS.Wxx.xx Volunteer patch for
 * .10y dual sensor
 *
 * 09 02 2010 jackie.su
 * [ALPS00002279] [Need Patch] [Volunteer Patch] ALPS.Wxx.xx Volunteer patch for
 * .roll back dual sensor
 *
 * Mar 4 2010 mtk70508
 * [DUMA00154792] Sensor driver
 * 
 *
 * Feb 24 2010 mtk01118
 * [DUMA00025869] [Camera][YUV I/F & Query feature] check in camera code
 * 
 *
 * Aug 5 2009 mtk01051
 * [DUMA00009217] [Camera Driver] CCAP First Check In
 * 
 *
 * Apr 7 2009 mtk02204
 * [DUMA00004012] [Camera] Restructure and rename camera related custom folders and folder name of came
 * 
 *
 * Mar 26 2009 mtk02204
 * [DUMA00003515] [PC_Lint] Remove PC_Lint check warnings of camera related drivers.
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
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
/* SENSOR FULL SIZE */
//#ifndef __SENSOR_H
//#define __SENSOR_H


typedef enum {
    OV7695_SENSOR_MODE_INIT = 0,
    OV7695_SENSOR_MODE_PREVIEW,
    OV7695_SENSOR_MODE_CAPTURE
} OV7695_SENSOR_MODE;

typedef enum _OV7695_OP_TYPE_ {
        OV7695_MODE_NONE,
        OV7695_MODE_PREVIEW,
        OV7695_MODE_CAPTURE,
        OV7695_MODE_QCIF_VIDEO,
        OV7695_MODE_CIF_VIDEO,
        OV7695_MODE_QVGA_VIDEO
    } OV7695_OP_TYPE;

extern OV7695_OP_TYPE OV7695_g_iOV7695_Mode;

#define OV7695_ID_REG                          (0x300A)
#define OV7695_INFO_REG                        (0x300B)
 
/* sensor size */
#define OV7695_IMAGE_SENSOR_QVGA_WIDTH          (320)
#define OV7695_IMAGE_SENSOR_QVGA_HEIGHT         (240)
#define OV7695_IMAGE_SENSOR_CIF_WIDTH          (480)
#define OV7695_IMAGE_SENSOR_CIF_HEIGHT         (480)
#define OV7695_IMAGE_SENSOR_VGA_WITDH        (640) 
#define OV7695_IMAGE_SENSOR_VGA_HEIGHT       (480)

/* Sesnor Pixel/Line Numbers in One Period */	
#define OV7695_PV_PERIOD_PIXEL_NUMS    		(746)  		/* Default preview line length */
#define OV7695_PV_PERIOD_LINE_NUMS     			(536)   	/* Default preview frame length */
#define OV7695_FULL_PERIOD_PIXEL_NUMS  		(746)  	/* Default full size line length */
#define OV7695_FULL_PERIOD_LINE_NUMS   		(536)  	/* Default full size frame length */

/* Sensor Exposure Line Limitation */
#define OV7695_PV_EXPOSURE_LIMITATION      	(536-4)
#define OV7695_FULL_EXPOSURE_LIMITATION    	(536-4)

/* Config the ISP grab start x & start y, Config the ISP grab width & height */
#define OV7695_PV_GRAB_START_X 				(1)
#define OV7695_PV_GRAB_START_Y  			(1)
#define OV7695_PV_GRAB_WIDTH				(OV7695_IMAGE_SENSOR_QVGA_WIDTH - 8)
#define OV7695_PV_GRAB_HEIGHT				(OV7695_IMAGE_SENSOR_QVGA_HEIGHT - 6)

#define OV7695_FULL_GRAB_START_X   			(1)
#define OV7695_FULL_GRAB_START_Y	  		(1)
#define OV7695_FULL_GRAB_WIDTH				(OV7695_IMAGE_SENSOR_VGA_WITDH - 8)
#define OV7695_FULL_GRAB_HEIGHT				(OV7695_IMAGE_SENSOR_VGA_HEIGHT - 6)

/*50Hz,60Hz*/
#define OV7695_NUM_50HZ                        (50 * 2)
#define OV7695_NUM_60HZ                        (60 * 2)

/* FRAME RATE UNIT */
#define OV7695_FRAME_RATE_UNIT                 (10)

/* MAX CAMERA FRAME RATE */
#define OV7695_MAX_CAMERA_FPS                  (OV7695_FRAME_RATE_UNIT * 30)

/* DUMMY NEEDS TO BE INSERTED */
/* SETUP TIME NEED TO BE INSERTED */

/* SENSOR READ/WRITE ID */
#define OV7695_WRITE_ID						    0x42
#define OV7695_READ_ID							    0x43
#define OV7695_SENSOR_ID			0x7695

//export functions
UINT32 OV7695MIPIOpen(void);
UINT32 OV7695MIPIGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution);
UINT32 OV7695MIPIGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_INFO_STRUCT *pSensorInfo, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 OV7695MIPIControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 OV7695MIPIFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId, UINT8 *pFeaturePara,UINT32 *pFeatureParaLen);
UINT32 OV7695MIPIClose(void);
UINT32 OV7695_MIPI_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
void OV7695MIPIWriteShutter(kal_uint32 shutter, kal_uint8 IsPVmode);
void OV7695MIPIWriteSensorGain(kal_uint32 gain);
void OV7695MIPISetDummy(kal_uint32 dummy_pixels, kal_uint32 dummy_lines , kal_uint8 isPVmode);
UINT32 OV7695MIPIReadShutter(void);
UINT32 OV7695MIPIReadSensorGain(void);


//#endif /* __SENSOR_H */

