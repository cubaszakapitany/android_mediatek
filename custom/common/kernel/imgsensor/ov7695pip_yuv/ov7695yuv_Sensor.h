
/* SENSOR FULL SIZE */
#ifndef __OV7695YUV_SENSOR_H
#define __OV7695YUV_SENSOR_H


typedef enum {
    SENSOR_MODE_INIT = 0,
    SENSOR_MODE_PREVIEW,
    SENSOR_MODE_CAPTURE
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
#define OV7695_IMAGE_SENSOR_QVGA_WIDTH         (320)
#define OV7695_IMAGE_SENSOR_QVGA_HEIGHT        (240)
#define OV7695_IMAGE_SENSOR_CIF_WIDTH          (480)
#define OV7695_IMAGE_SENSOR_CIF_HEIGHT         (480)
#define OV7695_IMAGE_SENSOR_VGA_WITDH          (640)
#define OV7695_IMAGE_SENSOR_VGA_HEIGHT         (480)

/* Sesnor Pixel/Line Numbers in One Period */
#define OV7695_PV_PERIOD_PIXEL_NUMS            (746)       /* Default preview line length */
#define OV7695_PV_PERIOD_LINE_NUMS             (536)       /* Default preview frame length */
#define OV7695_FULL_PERIOD_PIXEL_NUMS          (746)       /* Default full size line length */
#define OV7695_FULL_PERIOD_LINE_NUMS           (536)       /* Default full size frame length */

/* Sensor Exposure Line Limitation */
#define OV7695_PV_EXPOSURE_LIMITATION          (536-4)
#define OV7695_FULL_EXPOSURE_LIMITATION        (536-4)

/* Config the ISP grab start x & start y, Config the ISP grab width & height */
#define OV7695_PV_GRAB_START_X                 (1)
#define OV7695_PV_GRAB_START_Y                 (1)
#define OV7695_PV_GRAB_WIDTH                   (OV7695_IMAGE_SENSOR_QVGA_WIDTH - 8)
#define OV7695_PV_GRAB_HEIGHT                  (OV7695_IMAGE_SENSOR_QVGA_HEIGHT - 6)

#define OV7695_FULL_GRAB_START_X               (1)
#define OV7695_FULL_GRAB_START_Y               (1)
#define OV7695_FULL_GRAB_WIDTH                 (OV7695_IMAGE_SENSOR_VGA_WITDH - 8)
#define OV7695_FULL_GRAB_HEIGHT                (OV7695_IMAGE_SENSOR_VGA_HEIGHT - 6)

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
#define OV7695_WRITE_ID                         0x42
#define OV7695_READ_ID                          0x43
#define OV7695_SENSOR_ID                        0x7695

//export functions
UINT32 OV7695Open(void);
UINT32 OV7695GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution);
UINT32 OV7695GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_INFO_STRUCT *pSensorInfo, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 OV7695Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 OV7695FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId, UINT8 *pFeaturePara,UINT32 *pFeatureParaLen);
UINT32 OV7695Close(void);
UINT32 OV7695_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);


#endif /* __OV7695YUV_SENSOR_H */

