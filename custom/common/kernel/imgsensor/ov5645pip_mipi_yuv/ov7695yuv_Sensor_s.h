/* SENSOR FULL SIZE */
#ifndef __OV7695PIPYUV_SENSOR_S_H
#define __OV7695PIPYUV_SENSOR_S_H


typedef enum {
    OV7695PIPYUV_SENSOR_MODE_INIT = 0,
    OV7695PIPYUV_SENSOR_MODE_PREVIEW,
    OV7695PIPYUV_SENSOR_MODE_CAPTURE
} OV7695PIPYUV_SENSOR_MODE;

typedef enum _OV7695PIPYUV_OP_TYPE_ {
    OV7695PIPYUV_MODE_NONE,
    OV7695PIPYUV_MODE_PREVIEW,
    OV7695PIPYUV_MODE_CAPTURE,
    OV7695PIPYUV_MODE_QCIF_VIDEO,
    OV7695PIPYUV_MODE_CIF_VIDEO,
    OV7695PIPYUV_MODE_QVGA_VIDEO
} OV7695PIPYUV_OP_TYPE;

extern OV7695PIPYUV_OP_TYPE OV7695PIPYUV_g_iOV7695PIPYUV_Mode;

#define OV7695PIPYUV_ID_REG                          (0x300A)
#define OV7695PIPYUV_INFO_REG                        (0x300B)

/* sensor size */
#define OV7695PIPYUV_IMAGE_SENSOR_QVGA_WIDTH          (320)
#define OV7695PIPYUV_IMAGE_SENSOR_QVGA_HEIGHT         (240)
#define OV7695PIPYUV_IMAGE_SENSOR_CIF_WIDTH          (480)
#define OV7695PIPYUV_IMAGE_SENSOR_CIF_HEIGHT         (480)
#define OV7695PIPYUV_IMAGE_SENSOR_VGA_WITDH        (640)
#define OV7695PIPYUV_IMAGE_SENSOR_VGA_HEIGHT       (480)

/* Sesnor Pixel/Line Numbers in One Period */
#define OV7695PIPYUV_PV_PERIOD_PIXEL_NUMS            (540)          /* Default preview line length */
#define OV7695PIPYUV_PV_PERIOD_LINE_NUMS                 (740)       /* Default preview frame length */
#define OV7695PIPYUV_FULL_PERIOD_PIXEL_NUMS          (746)      /* Default full size line length */
#define OV7695PIPYUV_FULL_PERIOD_LINE_NUMS           (1464)      /* Default full size frame length */

/* Sensor Exposure Line Limitation */
#define OV7695PIPYUV_PV_EXPOSURE_LIMITATION          (740-4)
#define OV7695PIPYUV_FULL_EXPOSURE_LIMITATION        (1464-4)

/* Config the ISP grab start x & start y, Config the ISP grab width & height */
#define OV7695PIPYUV_PV_GRAB_START_X                 (1)
#define OV7695PIPYUV_PV_GRAB_START_Y              (1)
#define OV7695PIPYUV_PV_GRAB_WIDTH                (OV7695PIPYUV_IMAGE_SENSOR_QVGA_WIDTH - 8)
#define OV7695PIPYUV_PV_GRAB_HEIGHT                (OV7695PIPYUV_IMAGE_SENSOR_QVGA_HEIGHT - 6)

#define OV7695PIPYUV_FULL_GRAB_START_X               (1)
#define OV7695PIPYUV_FULL_GRAB_START_Y              (1)
#define OV7695PIPYUV_FULL_GRAB_WIDTH                (OV7695PIPYUV_IMAGE_SENSOR_VGA_WITDH - 16)
#define OV7695PIPYUV_FULL_GRAB_HEIGHT                (OV7695PIPYUV_IMAGE_SENSOR_VGA_HEIGHT - 12)

/*50Hz,60Hz*/
#define OV7695PIPYUV_NUM_50HZ                        (50 * 2)
#define OV7695PIPYUV_NUM_60HZ                        (60 * 2)

/* FRAME RATE UNIT */
#define OV7695PIPYUV_FRAME_RATE_UNIT                 (10)

/* MAX CAMERA FRAME RATE */
#define OV7695PIPYUV_MAX_CAMERA_FPS                  (OV7695PIPYUV_FRAME_RATE_UNIT * 30)


/* DUMMY NEEDS TO BE INSERTED */
/* SETUP TIME NEED TO BE INSERTED */


/* SENSOR READ/WRITE ID */
#define OV7695PIPYUV_WRITE_ID                            0x42
#define OV7695PIPYUV_READ_ID                             0x43

#define OV7695PIPYUV_SENSOR_ID            0x7695


//export functions
UINT32 OV7695PIPYUVOpen(void);
UINT32 OV7695PIPYUVGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution);
UINT32 OV7695PIPYUVGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_INFO_STRUCT *pSensorInfo, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 OV7695PIPYUVControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 OV7695PIPYUVFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId, UINT8 *pFeaturePara,UINT32 *pFeatureParaLen);
UINT32 OV7695PIPYUVClose(void);
void OV7695PIPYUVPreviewSetting_480x360(void);

UINT32 OV7695PIPYUV_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);

extern void OV7695PIPYUVWriteShutter(kal_uint32 shutter, kal_uint8 IsPVmode);
extern void OV7695PIPYUVWriteSensorGain(kal_uint32 gain);
extern void OV7695PIPYUVSetDummy(kal_uint32 dummy_pixels, kal_uint32 dummy_lines , kal_uint8 isPVmode);
extern UINT32 OV7695PIPYUVReadShutter(void);
extern UINT32 OV7695PIPYUVReadSensorGain(void);

extern BOOL OV7695PIPYUV_set_param_wb(UINT16 para);
extern BOOL OV7695PIPYUV_set_param_effect(UINT16 para);
extern BOOL OV7695PIPYUV_set_param_banding(UINT16 para);
extern BOOL OV7695PIPYUV_set_param_exposure(UINT16 para);
extern void OV7695PIPYUV_night_mode(kal_bool enable);
extern void OV7695PIPYUV_set_AE_mode(kal_bool AE_enable);
extern void OV7695PIPYUV_set_AWB_mode(kal_bool AWB_enable);
extern void OV7695PIPYUV_night_mode(kal_bool enable);
extern kal_uint32 OV7695PIPYUV_GetSensorID(kal_uint32 *sensorID);
extern void OV7695PIPYUVInitialSetting(void);
extern void OV7695PIPYUVInitialSettingVar(void);
extern void OV7695PIPYUVPreviewSetting_QVGA(void);
extern void OV7695PIPYUVVideoSetting_QVGA(void);
extern void OV7695PIPYUVVideoSetting_480(void);
extern void OV7695PIPYUVFullSizeCaptureSetting(void);
extern void OV7695PIPYUV_ON100(void);
extern void OV7695PIPYUV_OFF100(void);
extern void OV7695PIPYUVSetSeneorExist(kal_bool bSeneorExist);
extern void OV7695PIPYUVInitialVariable(void);
extern int OV7695PIP_Initialize_from_T_Flash(void);
#ifndef DEBUG_SENSOR
#define DEBUG_SENSOR
#endif

#endif /* __OV7695PIPYUVYUV_SENSOR_S_H*/

