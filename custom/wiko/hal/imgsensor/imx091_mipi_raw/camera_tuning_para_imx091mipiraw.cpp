#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_imx091mipiraw.h"
#include "camera_info_imx091mipiraw.h"
#include "camera_custom_AEPlinetable.h"
#include "camera_custom_tsf_tbl.h"
const NVRAM_CAMERA_ISP_PARAM_STRUCT CAMERA_ISP_DEFAULT_VALUE =
{{
    //Version
    Version: NVRAM_CAMERA_PARA_FILE_VERSION,
    //SensorId
    SensorId: SENSOR_ID,
    ISPComm:{
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        }
    },
    ISPPca:{
        #include INCLUDE_FILENAME_ISP_PCA_PARAM
    },
    ISPRegs:{
        #include INCLUDE_FILENAME_ISP_REGS_PARAM
        },
    ISPMfbMixer:{{
        {//00: MFB mixer for ISO 100
            0x00000000, 0x00000000
        },
        {//01: MFB mixer for ISO 200
            0x00000000, 0x00000000
        },
        {//02: MFB mixer for ISO 400
            0x00000000, 0x00000000
        },
        {//03: MFB mixer for ISO 800
            0x00000000, 0x00000000
        },
        {//04: MFB mixer for ISO 1600
            0x00000000, 0x00000000
        },
        {//05: MFB mixer for ISO 2400
            0x00000000, 0x00000000
        },
        {//06: MFB mixer for ISO 3200
            0x00000000, 0x00000000
        }
    }},
    ISPCcmPoly22:{
        82025,    // i4R_AVG
        18460,    // i4R_STD
        102600,    // i4B_AVG
        23273,    // i4B_STD
        {  // i4P00[9]
            5807500, -1920000, -1332500, -1182500, 4520000, -777500, -532500, -1835000, 4930000
        },
        {  // i4P10[9]
            1365942, -3216940, 1844703, 12567, -576338, 563771, 129766, -176136, 55289
        },
        {  // i4P01[9]
            983622, -1350192, 364897, -26205, -851071, 877276, -110436, 22, 119067
        },
        {  // i4P20[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {  // i4P11[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {  // i4P02[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        }
    }
}};

const NVRAM_CAMERA_3A_STRUCT CAMERA_3A_NVRAM_DEFAULT_VALUE =
{
    NVRAM_CAMERA_3A_FILE_VERSION, // u4Version
    SENSOR_ID, // SensorId

    // AE NVRAM
    {
        // rDevicesInfo
        {
            1144,    // u4MinGain, 1024 base = 1x
            8192,    // u4MaxGain, 16x
            118,    // u4MiniISOGain, ISOxx  
            128,    // u4GainStepUnit, 1x/8 
            20478,    // u4PreExpUnit 
            30,    // u4PreMaxFrameRate
            20478,    // u4VideoExpUnit  
            30,    // u4VideoMaxFrameRate 
            1024,    // u4Video2PreRatio, 1024 base = 1x 
            20910,    // u4CapExpUnit 
            15,    // u4CapMaxFrameRate
            1024,    // u4Cap2PreRatio, 1024 base = 1x
            24,    // u4LensFno, Fno = 2.8
            350    // u4FocusLength_100x
        },
        // rHistConfig
        {
            4,    // u4HistHighThres
            40,    // u4HistLowThres
            2,    // u4MostBrightRatio
            1,    // u4MostDarkRatio
            160,    // u4CentralHighBound
            20,    // u4CentralLowBound
            {240, 230, 220, 210, 200},    // u4OverExpThres[AE_CCT_STRENGTH_NUM] 
            {82, 108, 140, 148, 170},    // u4HistStretchThres[AE_CCT_STRENGTH_NUM] 
            {18, 22, 46, 30, 34}    // u4BlackLightThres[AE_CCT_STRENGTH_NUM] 
        },
        // rCCTConfig
        {
            TRUE,    // bEnableBlackLight
            TRUE,    // bEnableHistStretch
            FALSE,    // bEnableAntiOverExposure
            TRUE,    // bEnableTimeLPF
            FALSE,    // bEnableCaptureThres
            FALSE,    // bEnableVideoThres
            FALSE,    // bEnableStrobeThres
            49,    // u4AETarget
            0,    // u4StrobeAETarget
            50,    // u4InitIndex
            4,    // u4BackLightWeight
            32,    // u4HistStretchWeight
            4,    // u4AntiOverExpWeight
            2,    // u4BlackLightStrengthIndex
            2,    // u4HistStretchStrengthIndex
            2,    // u4AntiOverExpStrengthIndex
            2,    // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8},    // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM] 
            90,    // u4InDoorEV = 9.0, 10 base 
            -19,    // i4BVOffset delta BV = value/10 
            42,    // u4PreviewFlareOffset
            42,    // u4CaptureFlareOffset
            4,    // u4CaptureFlareThres
            40,    // u4VideoFlareOffset
            4,    // u4VideoFlareThres
            32,    // u4StrobeFlareOffset
            2,    // u4StrobeFlareThres
            160,    // u4PrvMaxFlareThres
            0,    // u4PrvMinFlareThres
            160,    // u4VideoMaxFlareThres
            0,    // u4VideoMinFlareThres
            18,    // u4FlatnessThres    // 10 base for flatness condition.
            75    // u4FlatnessStrength
        }
    },
    // AWB NVRAM
    {
        // AWB calibration data
        {
            // rUnitGain (unit gain: 1.0 = 512)
            {
                0,    // i4R
                0,    // i4G
                0    // i4B
            },
            // rGoldenGain (golden sample gain: 1.0 = 512)
            {
                0,    // i4R
                0,    // i4G
                0    // i4B
            },
            // rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
            {
                0,    // i4R
                0,    // i4G
                0    // i4B
            },
            // rD65Gain (D65 WB gain: 1.0 = 512)
            {
                1038,    // i4R
                512,    // i4G
                693    // i4B
            }
        },
        // Original XY coordinate of AWB light source
        {
           // Strobe
            {
                104,    // i4X
                -420    // i4Y
            },
            // Horizon
            {
                -389,    // i4X
                -370    // i4Y
            },
            // A
            {
                -266,    // i4X
                -386    // i4Y
            },
            // TL84
            {
                -97,    // i4X
                -430    // i4Y
            },
            // CWF
            {
                -62,    // i4X
                -457    // i4Y
            },
            // DNP
            {
                -3,    // i4X
                -419    // i4Y
            },
            // D65
            {
                149,    // i4X
                -373    // i4Y
            },
            // DF
            {
                115,    // i4X
                -456    // i4Y
            }
        },
        // Rotated XY coordinate of AWB light source
        {
            // Strobe
            {
                89,    // i4X
                -424    // i4Y
            },
            // Horizon
            {
                -402,    // i4X
                -356    // i4Y
            },
            // A
            {
                -280,    // i4X
                -377    // i4Y
            },
            // TL84
            {
                -112,    // i4X
                -427    // i4Y
            },
            // CWF
            {
                -78,    // i4X
                -455    // i4Y
            },
            // DNP
            {
                -18,    // i4X
                -419    // i4Y
            },
            // D65
            {
                136,    // i4X
                -378    // i4Y
            },
            // DF
            {
                99,    // i4X
                -460    // i4Y
            }
        },
        // AWB gain of AWB light source
        {
            // Strobe 
            {
                1040,    // i4R
                512,    // i4G
                785    // i4B
            },
            // Horizon 
            {
                512,    // i4R
                525,    // i4G
                1466    // i4B
            },
            // A 
            {
                603,    // i4R
                512,    // i4G
                1238    // i4B
            },
            // TL84 
            {
                804,    // i4R
                512,    // i4G
                1046    // i4B
            },
            // CWF 
            {
                874,    // i4R
                512,    // i4G
                1034    // i4B
            },
            // DNP 
            {
                899,    // i4R
                512,    // i4G
                907    // i4B
            },
            // D65 
            {
                1038,    // i4R
                512,    // i4G
                693    // i4B
            },
            // DF 
            {
                1109,    // i4R
                512,    // i4G
                813    // i4B
            }
        },
        // Rotation matrix parameter
        {
            2,    // i4RotationAngle
            256,    // i4Cos
            9    // i4Sin
        },
        // Daylight locus parameter
        {
            -136,    // i4SlopeNumerator
            128    // i4SlopeDenominator
        },
        // AWB light area
        {
            // Strobe:FIXME
            {
            0,    // i4RightBound
            0,    // i4LeftBound
            0,    // i4UpperBound
            0    // i4LowerBound
            },
            // Tungsten
            {
            -162,    // i4RightBound
            -812,    // i4LeftBound
            -316,    // i4UpperBound
            -416    // i4LowerBound
            },
            // Warm fluorescent
            {
            -162,    // i4RightBound
            -812,    // i4LeftBound
            -416,    // i4UpperBound
            -536    // i4LowerBound
            },
            // Fluorescent
            {
            -68,    // i4RightBound
            -162,    // i4LeftBound
            -307,    // i4UpperBound
            -441    // i4LowerBound
            },
            // CWF
            {
            -68,    // i4RightBound
            -162,    // i4LeftBound
            -441,    // i4UpperBound
            -505    // i4LowerBound
            },
            // Daylight
            {
            161,    // i4RightBound
            -68,    // i4LeftBound
            -298,    // i4UpperBound
            -458    // i4LowerBound
            },
            // Shade
            {
            521,    // i4RightBound
            161,    // i4LeftBound
            -298,    // i4UpperBound
            -458    // i4LowerBound
            },
            // Daylight Fluorescent
            {
            161,    // i4RightBound
            -68,    // i4LeftBound
            -458,    // i4UpperBound
            -505    // i4LowerBound
            }
        },
        // PWB light area
        {
            // Reference area
            {
            521,    // i4RightBound
            -812,    // i4LeftBound
            0,    // i4UpperBound
            -536    // i4LowerBound
            },
            // Daylight
            {
            186,    // i4RightBound
            -68,    // i4LeftBound
            -298,    // i4UpperBound
            -458    // i4LowerBound
            },
            // Cloudy daylight
            {
            286,    // i4RightBound
            111,    // i4LeftBound
            -298,    // i4UpperBound
            -458    // i4LowerBound
            },
            // Shade
            {
            386,    // i4RightBound
            111,    // i4LeftBound
            -298,    // i4UpperBound
            -458    // i4LowerBound
            },
            // Twilight
            {
            -68,    // i4RightBound
            -228,    // i4LeftBound
            -298,    // i4UpperBound
            -458    // i4LowerBound
            },
            // Fluorescent
            {
            186,    // i4RightBound
            -212,    // i4LeftBound
            -328,    // i4UpperBound
            -505    // i4LowerBound
            },
            // Warm fluorescent
            {
            -180,    // i4RightBound
            -380,    // i4LeftBound
            -328,    // i4UpperBound
            -505    // i4LowerBound
            },
            // Incandescent
            {
            -180,    // i4RightBound
            -380,    // i4LeftBound
            -298,    // i4UpperBound
            -458    // i4LowerBound
            },
            // Gray World
            {
            5000,    // i4RightBound
            -5000,    // i4LeftBound
            5000,    // i4UpperBound
            -5000    // i4LowerBound
            }
        },
        // PWB default gain	
        {
            // Daylight
            {
            939,    // i4R
            512,    // i4G
            772    // i4B
            },
            // Cloudy daylight
            {
            1126,    // i4R
            512,    // i4G
            635    // i4B
            },
            // Shade
            {
            1202,    // i4R
            512,    // i4G
            592    // i4B
            },
            // Twilight
            {
            716,    // i4R
            512,    // i4G
            1031    // i4B
            },
            // Fluorescent
            {
            902,    // i4R
            512,    // i4G
            898    // i4B
            },
            // Warm fluorescent
            {
            637,    // i4R
            512,    // i4G
            1305    // i4B
            },
            // Incandescent
            {
            603,    // i4R
            512,    // i4G
            1241    // i4B
            },
            // Gray World
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            }
        },
        // AWB preference color	
        {
            // Tungsten
            {
            0,    // i4SliderValue
            6743    // i4OffsetThr
            },
            // Warm fluorescent	
            {
            0,    // i4SliderValue
            5062    // i4OffsetThr
            },
            // Shade
            {
            50,    // i4SliderValue
            341    // i4OffsetThr
            },
            // Daylight WB gain
            {
            930,    // i4R
            512,    // i4G
            860    // i4B
            },
            // Preference gain: strobe
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: tungsten
            {
            530,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: warm fluorescent
            {
            530,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: fluorescent
            {
            520,    // i4R
            512,    // i4G
            508    // i4B
            },
            // Preference gain: CWF
            {
            520,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: daylight
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: shade
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: daylight fluorescent
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            }
        },
        {// CCT estimation
            {// CCT
                2300,    // i4CCT[0]
                2850,    // i4CCT[1]
                4100,    // i4CCT[2]
                5100,    // i4CCT[3]
                6500    // i4CCT[4]
            },
            {// Rotated X coordinate
                -538,    // i4RotatedXCoordinate[0]
                -416,    // i4RotatedXCoordinate[1]
                -248,    // i4RotatedXCoordinate[2]
                -154,    // i4RotatedXCoordinate[3]
                0    // i4RotatedXCoordinate[4]
            }
        }
    },
    {0}
};

#include INCLUDE_FILENAME_ISP_LSC_PARAM
//};  //  namespace

const CAMERA_TSF_TBL_STRUCT CAMERA_TSF_DEFAULT_VALUE =
{
    #include INCLUDE_FILENAME_TSF_PARA
    #include INCLUDE_FILENAME_TSF_DATA
};


typedef NSFeature::RAWSensorInfo<SENSOR_ID> SensorInfoSingleton_T;


namespace NSFeature {
template <>
UINT32
SensorInfoSingleton_T::
impGetDefaultData(CAMERA_DATA_TYPE_ENUM const CameraDataType, VOID*const pDataBuf, UINT32 const size) const
{
    UINT32 dataSize[CAMERA_DATA_TYPE_NUM] = {sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT),
                                             sizeof(NVRAM_CAMERA_3A_STRUCT),
                                             sizeof(NVRAM_CAMERA_SHADING_STRUCT),
                                             sizeof(NVRAM_LENS_PARA_STRUCT),
                                             sizeof(AE_PLINETABLE_T),
                                             0,
                                             sizeof(CAMERA_TSF_TBL_STRUCT)};

    if (CameraDataType > CAMERA_DATA_TSF_TABLE || NULL == pDataBuf || (size < dataSize[CameraDataType]))
    {
        return 1;
    }

    switch(CameraDataType)
    {
        case CAMERA_NVRAM_DATA_ISP:
            memcpy(pDataBuf,&CAMERA_ISP_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));
            break;
        case CAMERA_NVRAM_DATA_3A:
            memcpy(pDataBuf,&CAMERA_3A_NVRAM_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_3A_STRUCT));
            break;
        case CAMERA_NVRAM_DATA_SHADING:
            memcpy(pDataBuf,&CAMERA_SHADING_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_SHADING_STRUCT));
            break;
        case CAMERA_DATA_AE_PLINETABLE:
            memcpy(pDataBuf,&g_PlineTableMapping,sizeof(AE_PLINETABLE_T));
            break;
        case CAMERA_DATA_TSF_TABLE:
            memcpy(pDataBuf,&CAMERA_TSF_DEFAULT_VALUE,sizeof(CAMERA_TSF_TBL_STRUCT));
            break;
        default:
            break;
    }
    return 0;
}}; // NSFeature


