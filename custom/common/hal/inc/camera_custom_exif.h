#ifndef _CAMERA_CUSTOM_EXIF_
#define _CAMERA_CUSTOM_EXIF_
//
#include "camera_custom_types.h"

//
namespace NSCamCustom
{
/*******************************************************************************
* Custom EXIF: Imgsensor-related.
*******************************************************************************/
typedef struct SensorExifInfo_S
{
    MUINT32 uFLengthNum;
    MUINT32 uFLengthDenom;
    
} SensorExifInfo_T;

SensorExifInfo_T const&
getParamSensorExif()
{
    static SensorExifInfo_T inst = { 
        uFLengthNum     : 35, // Numerator of Focal Length. Default is 35.
        uFLengthDenom   : 10, // Denominator of Focal Length, it should not be 0.  Default is 10.
    };
    return inst;
}


/*******************************************************************************
* Custom EXIF
******************************************************************************/
#define EN_CUSTOM_EXIF_INFO
#define SET_EXIF_TAG_STRING(tag,str) \
    if (strlen((const char*)str) <= 32) { \
        strcpy((char *)pexifApp1Info->tag, (const char*)str); }
        
typedef struct customExifInfo_s {
    unsigned char strMake[32];
    unsigned char strModel[32];
    unsigned char strSoftware[32];
} customExifInfo_t;

MINT32 custom_SetExif(void **ppCustomExifTag)
{
#ifdef EN_CUSTOM_EXIF_INFO
#if 0
#define CUSTOM_EXIF_STRING_MAKE  "custom make"
#define CUSTOM_EXIF_STRING_MODEL "custom model"
#define CUSTOM_EXIF_STRING_SOFTWARE "custom software"
static customExifInfo_t exifTag = {CUSTOM_EXIF_STRING_MAKE,CUSTOM_EXIF_STRING_MODEL,CUSTOM_EXIF_STRING_SOFTWARE};
    if (0 != ppCustomExifTag) {
        *ppCustomExifTag = (void*)&exifTag;
    }
    return 0;
#else
#if defined(CUSTOM_WIKO_VERSION)
#define CUSTOM_EXIF_STRING_MAKE  "WIKO" //custom make
#define CUSTOM_EXIF_STRING_MODEL "GOA" //custom model
#define CUSTOM_EXIF_STRING_SOFTWARE "Camera Application" //custom software
#else
#define CUSTOM_EXIF_STRING_MAKE  "TINNO"
#define CUSTOM_EXIF_STRING_MODEL "SmartPhone"
#define CUSTOM_EXIF_STRING_SOFTWARE "Camera Application"
#endif
static customExifInfo_t exifTag = {CUSTOM_EXIF_STRING_MAKE,CUSTOM_EXIF_STRING_MODEL,CUSTOM_EXIF_STRING_SOFTWARE};
    if (0 != ppCustomExifTag) {
        char valueMAKE[256] = {'\0'};
        char valueMODEL[256] = {'\0'};
        property_get("ro.product.brand", valueMAKE, CUSTOM_EXIF_STRING_MAKE);
        property_get("ro.product.model", valueMODEL, CUSTOM_EXIF_STRING_MODEL);
        memcpy(exifTag.strMake, valueMAKE, 31);
        memcpy(exifTag.strModel, valueMODEL, 31);
        *ppCustomExifTag = (void*)&exifTag;
    }
    return 0;
#endif
#else
    return -1;
#endif
}


/*******************************************************************************
* Custom EXIF: Exposure Program
******************************************************************************/
typedef struct customExif_s
{
    MBOOL   bEnCustom;
    MUINT32 u4ExpProgram;
    
} customExif_t;

customExif_t const&
getCustomExif()
{
    static customExif_t inst = {
        bEnCustom       :   false,  // default value: false.
        u4ExpProgram    :   0,      // default value: 0.    '0' means not defined, '1' manual control, '2' program normal
    };
    return inst;
}


};  //NSCamCustom
#endif  //  _CAMERA_CUSTOM_EXIF_

