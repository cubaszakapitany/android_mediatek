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
#define OV5645YUV_DEBUG
#ifdef OV5645YUV_DEBUG
#define OV5645SENSORDB(fmt, arg...)    printk("[OV5645PIP_M]%s: " fmt, __FUNCTION__ ,##arg)
#else
#define OV5645SENSORDB(x,...)
#endif

//#define OV5645_20FPS
#define OV5645_15FPS

static DEFINE_SPINLOCK(ov5645_drv_lock);
bool AF_Power = false;
//static MSDK_SCENARIO_ID_ENUM CurrentScenarioId = MSDK_SCENARIO_ID_CAMERA_PREVIEW;
extern int iReadReg(u16 a_u2Addr , u8 * a_puBuff , u16 i2cId);
extern int iWriteReg(u16 a_u2Addr , u32 a_u4Data , u32 a_u4Bytes , u16 i2cId);
extern int iBurstWriteReg(u8 *pData, u32 bytes, u16 i2cId) ;
#define OV5645_write_cmos_sensor(addr, para) iWriteReg((u16) addr , (u32) para ,1,OV5645_WRITE_ID)

#ifdef mDELAY
    #undef mDELAY
#endif
#define mDELAY(ms)  msleep(ms)
//#define mDELAY(ms)  mdelay(ms)

//kal_uint8 OV5645_sensor_socket = DUAL_CAMERA_NONE_SENSOR;
typedef enum
{
    PRV_W=1280,
    PRV_H=720 //960
}PREVIEW_VIEW_SIZE;
kal_uint16 OV5645_read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte=0;
	iReadReg((u16) addr ,(u8*)&get_byte,OV5645_WRITE_ID);
	return get_byte;
}
#define OV5645_MAX_AXD_GAIN (32) //max gain = 32
#define OV5645_MAX_EXPOSURE_TIME (1968) // preview:984,capture 984*2
struct Sensor_para OV5645MIPISensor;
/* Global Valuable */
//static kal_uint32 zoom_factor = 0;
//static kal_int8 OV5645_DELAY_AFTER_PREVIEW = -1;
static kal_uint8 OV5645_Banding_setting = AE_FLICKER_MODE_50HZ;
static kal_bool OV5645_AWB_ENABLE = KAL_TRUE;
//static kal_bool OV5645_AE_ENABLE = KAL_TRUE;
//MSDK_SENSOR_CONFIG_STRUCT OV5645MIPISensorConfigData;
#define OV5645_TEST_PATTERN_CHECKSUM (0x7ba87eae)
static kal_bool run_test_potten=0;

typedef enum
{
    AE_SECTION_INDEX_BEGIN=0,
    AE_SECTION_INDEX_1=AE_SECTION_INDEX_BEGIN,
    AE_SECTION_INDEX_2,
    AE_SECTION_INDEX_3,
    AE_SECTION_INDEX_4,
    AE_SECTION_INDEX_5,
    AE_SECTION_INDEX_6,
    AE_SECTION_INDEX_7,
    AE_SECTION_INDEX_8,
    AE_SECTION_INDEX_9,
    AE_SECTION_INDEX_10,
    AE_SECTION_INDEX_11,
    AE_SECTION_INDEX_12,
    AE_SECTION_INDEX_13,
    AE_SECTION_INDEX_14,
    AE_SECTION_INDEX_15,
    AE_SECTION_INDEX_16,
    AE_SECTION_INDEX_MAX
}AE_SECTION_INDEX;
typedef enum
{
    AE_VERTICAL_BLOCKS=4,
    AE_VERTICAL_BLOCKS_MAX,
    AE_HORIZONTAL_BLOCKS=4,
    AE_HORIZONTAL_BLOCKS_MAX
}AE_VERTICAL_HORIZONTAL_BLOCKS;
static UINT32 line_coordinate[AE_VERTICAL_BLOCKS_MAX] = {0};//line[0]=0      line[1]=160     line[2]=320     line[3]=480     line[4]=640
static UINT32 row_coordinate[AE_HORIZONTAL_BLOCKS_MAX] = {0};//line[0]=0       line[1]=120     line[2]=240     line[3]=360     line[4]=480
static BOOL AE_1_ARRAY[AE_SECTION_INDEX_MAX] = {FALSE};
static BOOL AE_2_ARRAY[AE_HORIZONTAL_BLOCKS][AE_VERTICAL_BLOCKS] = {{FALSE},{FALSE},{FALSE},{FALSE}};//how to ....
//=====================touch AE begin==========================//
static void writeAEReg(void)
{
    UINT8 temp;
    //write 1280X720
    OV5645_write_cmos_sensor(0x501d,0x10);
    OV5645_write_cmos_sensor(0x5680,0x00);
    OV5645_write_cmos_sensor(0x5681,0x00);
    OV5645_write_cmos_sensor(0x5682,0x00);
    OV5645_write_cmos_sensor(0x5683,0x00);
    OV5645_write_cmos_sensor(0x5684,0x05); //width=1280
    OV5645_write_cmos_sensor(0x5685,0x00);
    //OV5645_write_cmos_sensor(0x5686,0x03); //heght=256
    //OV5645_write_cmos_sensor(0x5687,0xc0);
    OV5645_write_cmos_sensor(0x5686,0x02); //heght=720
    OV5645_write_cmos_sensor(0x5687,0xd0);
    temp=0x11;
    if(AE_1_ARRAY[AE_SECTION_INDEX_1]==TRUE)    { temp=temp|0x0F;}
    if(AE_1_ARRAY[AE_SECTION_INDEX_2]==TRUE)    { temp=temp|0xF0;}
    //write 0x5688
    OV5645_write_cmos_sensor(0x5688,temp);

    temp=0x11;
    if(AE_1_ARRAY[AE_SECTION_INDEX_3]==TRUE)    { temp=temp|0x0F;}
    if(AE_1_ARRAY[AE_SECTION_INDEX_4]==TRUE)    { temp=temp|0xF0;}
    //write 0x5689
    OV5645_write_cmos_sensor(0x5689,temp);

    temp=0x11;
    if(AE_1_ARRAY[AE_SECTION_INDEX_5]==TRUE)    { temp=temp|0x0F;}
    if(AE_1_ARRAY[AE_SECTION_INDEX_6]==TRUE)    { temp=temp|0xF0;}
    //write 0x568A
    OV5645_write_cmos_sensor(0x568A,temp);

    temp=0x11;
    if(AE_1_ARRAY[AE_SECTION_INDEX_7]==TRUE)    { temp=temp|0x0F;}
    if(AE_1_ARRAY[AE_SECTION_INDEX_8]==TRUE)    { temp=temp|0xF0;}
    //write 0x568B
    OV5645_write_cmos_sensor(0x568B,temp);

    temp=0x11;
    if(AE_1_ARRAY[AE_SECTION_INDEX_9]==TRUE)    { temp=temp|0x0F;}
    if(AE_1_ARRAY[AE_SECTION_INDEX_10]==TRUE)  { temp=temp|0xF0;}
    //write 0x568C
    OV5645_write_cmos_sensor(0x568C,temp);

    temp=0x11;
    if(AE_1_ARRAY[AE_SECTION_INDEX_11]==TRUE)    { temp=temp|0x0F;}
    if(AE_1_ARRAY[AE_SECTION_INDEX_12]==TRUE)    { temp=temp|0xF0;}
    //write 0x568D
    OV5645_write_cmos_sensor(0x568D,temp);

    temp=0x11;
    if(AE_1_ARRAY[AE_SECTION_INDEX_13]==TRUE)    { temp=temp|0x0F;}
    if(AE_1_ARRAY[AE_SECTION_INDEX_14]==TRUE)    { temp=temp|0xF0;}
    //write 0x568E
    OV5645_write_cmos_sensor(0x568E,temp);

    temp=0x11;
    if(AE_1_ARRAY[AE_SECTION_INDEX_15]==TRUE)    { temp=temp|0x0F;}
    if(AE_1_ARRAY[AE_SECTION_INDEX_16]==TRUE)    { temp=temp|0xF0;}
    //write 0x568F
    OV5645_write_cmos_sensor(0x568F,temp);
}

static void printAE_1_ARRAY(void)
{
    UINT32 i;
    for(i=0; i<AE_SECTION_INDEX_MAX; i++)
    {
        OV5645SENSORDB("AE_1_ARRAY[%2d]=%d\n", i, AE_1_ARRAY[i]);
    }
}

static void printAE_2_ARRAY(void)
{
    UINT32 i, j;
    OV5645SENSORDB("\t\t");
    for(i=0; i<AE_VERTICAL_BLOCKS; i++)
    {
        OV5645SENSORDB("      line[%2d]", i);
    }
    printk("\n");
    for(j=0; j<AE_HORIZONTAL_BLOCKS; j++)
    {
        OV5645SENSORDB("\trow[%2d]", j);
        for(i=0; i<AE_VERTICAL_BLOCKS; i++)
        {
            //SENSORDB("AE_2_ARRAY[%2d][%2d]=%d\n", j,i,AE_2_ARRAY[j][i]);
            OV5645SENSORDB("  %7d", AE_2_ARRAY[j][i]);
        }
        OV5645SENSORDB("\n");
    }
}

static void clearAE_2_ARRAY(void)
{
    UINT32 i, j;
    for(j=0; j<AE_HORIZONTAL_BLOCKS; j++)
    {
        for(i=0; i<AE_VERTICAL_BLOCKS; i++)
        {AE_2_ARRAY[j][i]=FALSE;}
    }
}

static void mapAE_2_ARRAY_To_AE_1_ARRAY(void)
{
    UINT32 i, j;
    for(j=0; j<AE_HORIZONTAL_BLOCKS; j++)
    {
        for(i=0; i<AE_VERTICAL_BLOCKS; i++)
        { AE_1_ARRAY[j*AE_VERTICAL_BLOCKS+i] = AE_2_ARRAY[j][i];}
    }
}

static void mapMiddlewaresizePointToPreviewsizePoint(
    UINT32 mx,
    UINT32 my,
    UINT32 mw,
    UINT32 mh,
    UINT32 * pvx,
    UINT32 * pvy,
    UINT32 pvw,
    UINT32 pvh
)
{
    *pvx = pvw * mx / mw;
    *pvy = pvh * my / mh;
//    OV5645SENSORDB("mapping middlware x[%d],y[%d], [%d X %d]\n\t\tto x[%d],y[%d],[%d X %d]\n ",
//        mx, my, mw, mh, *pvx, *pvy, pvw, pvh);
}


static void calcLine(void)
{//line[5]
    UINT32 i;
    UINT32 step = PRV_W / AE_VERTICAL_BLOCKS;
    for(i=0; i<=AE_VERTICAL_BLOCKS; i++)
    {
        *(&line_coordinate[0]+i) = step*i;
        //OV5645SENSORDB("line[%d]=%d\t",i, *(&line_coordinate[0]+i));
    }
    //OV5645SENSORDB("\n");
}

static void calcRow(void)
{//row[5]
    UINT32 i;
    UINT32 step = PRV_H / AE_HORIZONTAL_BLOCKS;
    for(i=0; i<=AE_HORIZONTAL_BLOCKS; i++)
    {
        *(&row_coordinate[0]+i) = step*i;
        //OV5645SENSORDB("row[%d]=%d\t",i,*(&row_coordinate[0]+i));
    }
    //OV5645SENSORDB("\n");
}

static void calcPointsAELineRowCoordinate(UINT32 x, UINT32 y, UINT32 * linenum, UINT32 * rownum)
{
    UINT32 i;
    i = 1;
    while(i<=AE_VERTICAL_BLOCKS)
    {
        if(x<line_coordinate[i])
        {
            *linenum = i;
            break;
        }
        *linenum = i++;
    }

    i = 1;
    while(i<=AE_HORIZONTAL_BLOCKS)
    {
        if(y<row_coordinate[i])
        {
            *rownum = i;
            break;
        }
        *rownum = i++;
    }
    //OV5645SENSORDB("PV point [%d, %d] to section line coordinate[%d] row[%d]\n",x,y,*linenum,*rownum);
}



static MINT32 clampSection(UINT32 x, UINT32 min, UINT32 max)
{
    if (x > max) return max;
    if (x < min) return min;
    return x;
}

static void mapCoordinate(UINT32 linenum, UINT32 rownum, UINT32 * sectionlinenum, UINT32 * sectionrownum)
{
    *sectionlinenum = clampSection(linenum-1,0,AE_VERTICAL_BLOCKS-1);
    *sectionrownum = clampSection(rownum-1,0,AE_HORIZONTAL_BLOCKS-1);
//    OV5645SENSORDB("mapCoordinate from[%d][%d] to[%d][%d]\n",
//        linenum, rownum,*sectionlinenum,*sectionrownum);
}

static void mapRectToAE_2_ARRAY(UINT32 x0, UINT32 y0, UINT32 x1, UINT32 y1)
{
    UINT32 i, j;
//    OV5645SENSORDB("([%d][%d]),([%d][%d])\n", x0,y0,x1,y1);
    clearAE_2_ARRAY();
    x0=clampSection(x0,0,AE_VERTICAL_BLOCKS-1);
    y0=clampSection(y0,0,AE_HORIZONTAL_BLOCKS-1);
    x1=clampSection(x1,0,AE_VERTICAL_BLOCKS-1);
    y1=clampSection(y1,0,AE_HORIZONTAL_BLOCKS-1);

    for(j=y0; j<=y1; j++)
    {
        for(i=x0; i<=x1; i++)
        {
            AE_2_ARRAY[j][i]=TRUE;
        }
    }
}

static void resetPVAE_2_ARRAY(void)
{
    mapRectToAE_2_ARRAY(1,1,2,2);
}

//update ae window
//@input zone[] addr
void OV5645PIP_FOCUS_Set_AE_Window(UINT32 zone_addr)
{//update global zone
    //input:
    UINT32 FD_XS;
    UINT32 FD_YS;
    UINT32 x0, y0, x1, y1;
    UINT32 pvx0, pvy0, pvx1, pvy1;
    UINT32 linenum, rownum;
    UINT32 rightbottomlinenum,rightbottomrownum;
    UINT32 leftuplinenum,leftuprownum;
    UINT32* zone = (UINT32*)zone_addr;
    x0 = *zone;
    y0 = *(zone + 1);
    x1 = *(zone + 2);
    y1 = *(zone + 3);
    FD_XS = *(zone + 4);
    FD_YS = *(zone + 5);

//    OV5645SENSORDB("[OV5645]enter OV5645_FOCUS_Set_AE_Window function\n");
//    OV5645SENSORDB("ov5645 AE_wind AE x0=%d,y0=%d,x1=%d,y1=%d,FD_XS=%d,FD_YS=%d\n",
//        x0, y0, x1, y1, FD_XS, FD_YS);

	if ( 0 == FD_XS || 0 == FD_YS ){
		OV5645SENSORDB("exit function--ERROR PARAM:\n ");
		return;
	}
    //print_sensor_ae_section();
    //print_AE_section();

    //1.transfer points to preview size
    //UINT32 pvx0, pvy0, pvx1, pvy1;
    mapMiddlewaresizePointToPreviewsizePoint(x0,y0,FD_XS,FD_YS,&pvx0, &pvy0, PRV_W, PRV_H);
    mapMiddlewaresizePointToPreviewsizePoint(x1,y1,FD_XS,FD_YS,&pvx1, &pvy1, PRV_W, PRV_H);

    //2.sensor AE line and row coordinate
    calcLine();
    calcRow();

    //3.calc left up point to section
    //UINT32 linenum, rownum;
    calcPointsAELineRowCoordinate(pvx0,pvy0,&linenum,&rownum);
    //UINT32 leftuplinenum,leftuprownum;
    mapCoordinate(linenum, rownum, &leftuplinenum, &leftuprownum);
    //SENSORDB("leftuplinenum=%d,leftuprownum=%d\n",leftuplinenum,leftuprownum);

    //4.calc right bottom point to section
    calcPointsAELineRowCoordinate(pvx1,pvy1,&linenum,&rownum);
    //UINT32 rightbottomlinenum,rightbottomrownum;
    mapCoordinate(linenum, rownum, &rightbottomlinenum, &rightbottomrownum);
    //SENSORDB("rightbottomlinenum=%d,rightbottomrownum=%d\n",rightbottomlinenum,rightbottomrownum);

    //5.update global section array
    mapRectToAE_2_ARRAY(leftuplinenum, leftuprownum, rightbottomlinenum, rightbottomrownum);
    //print_AE_section();

    //6.write to reg
    mapAE_2_ARRAY_To_AE_1_ARRAY();
    //printAE_1_ARRAY();
    //printAE_2_ARRAY();
    writeAEReg();
}

/*************************************************************************
* FUNCTION
*	OV5645_set_dummy
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
void OV5645initalvariable(void)
{
	static int is_initialized = 0;
	if ( is_initialized ){
		OV5645SENSORDB("initialized\n");
		return;
	}
	is_initialized = 1;
    spin_lock(&ov5645_drv_lock);
    OV5645MIPISensor.VideoMode = KAL_FALSE;
    OV5645MIPISensor.NightMode = KAL_FALSE;
    OV5645MIPISensor.Fps = 100;
    OV5645MIPISensor.ShutterStep= 0xde;
    OV5645MIPISensor.CaptureDummyPixels = 0;
    OV5645MIPISensor.CaptureDummyLines = 0;
    OV5645MIPISensor.PreviewDummyPixels = 0;
    OV5645MIPISensor.PreviewDummyLines = 0;
    OV5645MIPISensor.SensorMode= SENSOR_MODE_INIT;
    OV5645MIPISensor.IsPVmode= KAL_TRUE;
    OV5645MIPISensor.PreviewPclk= 480;//560;
    OV5645MIPISensor.CapturePclk= 480;//900;
    OV5645MIPISensor.ZsdturePclk= 480;//900;
    OV5645MIPISensor.PreviewShutter=0x0375; //0375
    OV5645MIPISensor.PreviewExtraShutter=0x00;
    OV5645MIPISensor.SensorGain=0x10;
    OV5645MIPISensor.manualAEStart=0;
    OV5645MIPISensor.isoSpeed=AE_ISO_100;
    OV5645MIPISensor.userAskAeLock=KAL_FALSE;
    OV5645MIPISensor.userAskAwbLock=KAL_FALSE;
    OV5645MIPISensor.currentExposureTime=0;
    OV5645MIPISensor.currentShutter=0;
    OV5645MIPISensor.zsd_flag=0;
    OV5645MIPISensor.currentextshutter=0;
    OV5645MIPISensor.AF_window_x=0;
    OV5645MIPISensor.AF_window_y=0;
    OV5645MIPISensor.awbMode = AWB_MODE_AUTO;
    OV5645MIPISensor.iWB=AWB_MODE_AUTO;
    OV5645MIPISensor.is_support_af = KAL_FALSE;
    spin_unlock(&ov5645_drv_lock);
}

kal_bool OV5645_Is_Support_AF(void)
{
	return OV5645MIPISensor.is_support_af;
}

void OV5645GetExifInfo(UINT32 exifAddr)
{
    SENSOR_EXIF_INFO_STRUCT* pExifInfo = (SENSOR_EXIF_INFO_STRUCT*)exifAddr;
    pExifInfo->FNumber = 28;
    pExifInfo->AEISOSpeed = OV5645MIPISensor.isoSpeed;
    pExifInfo->AWBMode = OV5645MIPISensor.awbMode;
    pExifInfo->FlashLightTimeus = 0;
    pExifInfo->RealISOValue = OV5645MIPISensor.isoSpeed;
}
void OV5645SetDummy(kal_uint32 dummy_pixels, kal_uint32 dummy_lines)
{
//    OV5645SENSORDB("[OV5645]enter OV5645SetDummy function:dummy_pixels=%d,dummy_lines=%d\n",dummy_pixels,dummy_lines);
    if (OV5645MIPISensor.IsPVmode)
        {
            dummy_pixels = dummy_pixels+OV5645_PV_PERIOD_PIXEL_NUMS;
            OV5645_write_cmos_sensor(0x380D,( dummy_pixels&0xFF));
            OV5645_write_cmos_sensor(0x380C,(( dummy_pixels&0xFF00)>>8));

            dummy_lines= dummy_lines+OV5645_PV_PERIOD_LINE_NUMS;
            OV5645_write_cmos_sensor(0x380F,(dummy_lines&0xFF));
            OV5645_write_cmos_sensor(0x380E,((dummy_lines&0xFF00)>>8));
        }
        else
        {
            dummy_pixels = dummy_pixels+OV5645_FULL_PERIOD_PIXEL_NUMS;
            OV5645_write_cmos_sensor(0x380D,( dummy_pixels&0xFF));
            OV5645_write_cmos_sensor(0x380C,(( dummy_pixels&0xFF00)>>8));

            dummy_lines= dummy_lines+OV5645_FULL_PERIOD_LINE_NUMS;
            OV5645_write_cmos_sensor(0x380F,(dummy_lines&0xFF));
            OV5645_write_cmos_sensor(0x380E,((dummy_lines&0xFF00)>>8));
        }
}    /* OV5645_set_dummy */

/*************************************************************************
* FUNCTION
*	OV5645WriteShutter
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
void OV5645WriteShutter(kal_uint32 shutter)
{
    kal_uint32 extra_exposure_vts = 0;
//    OV5645SENSORDB("[OV5645]enter OV5645WriteShutter function:shutter=%d\n ",shutter);
	if (shutter < 1)
	{
		shutter = 1;
	}
#if 0
    if (shutter > OV5645_FULL_EXPOSURE_LIMITATION)
	{
        extra_exposure_vts =shutter+4;
        OV5645_write_cmos_sensor(0x380f, extra_exposure_vts & 0xFF);          // EXVTS[b7~b0]
        OV5645_write_cmos_sensor(0x380e, (extra_exposure_vts & 0xFF00) >> 8); // EXVTS[b15~b8]
        OV5645_write_cmos_sensor(0x350D,0x00);
        OV5645_write_cmos_sensor(0x350C,0x00);
		}
#endif
    shutter*=16;
    OV5645_write_cmos_sensor(0x3502, (shutter & 0x00FF));           //AEC[7:0]
    OV5645_write_cmos_sensor(0x3501, ((shutter & 0x0FF00) >>8));  //AEC[15:8]
    OV5645_write_cmos_sensor(0x3500, ((shutter & 0xFF0000) >> 16));
}    /* OV5645_write_shutter */
/*************************************************************************
* FUNCTION
*    OV5645ExpWriteShutter
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
void OV5645WriteExpShutter(kal_uint32 shutter)
		{
	shutter*=16;
  //  OV5645SENSORDB("[OV5645]enter OV5645WriteExpShutter function:shutter=%d\n ",shutter);
	OV5645_write_cmos_sensor(0x3502, (shutter & 0x00FF));           //AEC[7:0]
	OV5645_write_cmos_sensor(0x3501, ((shutter & 0x0FF00) >>8));  //AEC[15:8]
	OV5645_write_cmos_sensor(0x3500, ((shutter & 0xFF0000) >> 16));
}    /* OV5645_write_shutter */
/*************************************************************************
* FUNCTION
*    OV5645ExtraWriteShutter
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
#if 0
static void OV5645WriteExtraShutter(kal_uint32 shutter)
{
    OV5645SENSORDB("[OV5645]enter OV5645WriteExtraShutter function:shutter=%d\n ",shutter);
    OV5645_write_cmos_sensor(0x350D, shutter & 0xFF);          // EXVTS[b7~b0]
    OV5645_write_cmos_sensor(0x350C, (shutter & 0xFF00) >> 8); // EXVTS[b15~b8]
    OV5645SENSORDB("[OV5645]exit OV5645WriteExtraShutter function:\n ");
}    /* OV5645_write_shutter */
#endif
/*************************************************************************
* FUNCTION
*    OV5645WriteSensorGain
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
void OV5645WriteSensorGain(kal_uint32 gain)
{
	kal_uint16 temp_reg = 0;
	if(gain > 1024)  ASSERT(0);
	temp_reg = 0;
	temp_reg=gain&0x0FF;
	OV5645_write_cmos_sensor(0x350B,temp_reg);
}  /* OV5645_write_sensor_gain */

/*************************************************************************
* FUNCTION
*	OV5645ReadShutter
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
kal_uint32 OV5645ReadShutter(void)
{
	kal_uint16 temp_reg1, temp_reg2 ,temp_reg3;
	temp_reg1 = OV5645_read_cmos_sensor(0x3500);    // AEC[b19~b16]
	temp_reg2 = OV5645_read_cmos_sensor(0x3501);    // AEC[b15~b8]
	temp_reg3 = OV5645_read_cmos_sensor(0x3502);    // AEC[b7~b0]

	spin_lock(&ov5645_drv_lock);
    OV5645MIPISensor.PreviewShutter  = (temp_reg1 <<12)| (temp_reg2<<4)|(temp_reg3>>4);
    //OV5645MIPISensor.PreviewShutter  = (temp_reg1 <<16)| (temp_reg2<<8)|(temp_reg3);
	spin_unlock(&ov5645_drv_lock);
    return OV5645MIPISensor.PreviewShutter;
}    /* OV5645_read_shutter */

/*************************************************************************
* FUNCTION
*    OV5645ReadExtraShutter
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
#if 0
kal_uint32 OV5645ReadExtraShutter(void)
{
	kal_uint16 temp_reg1, temp_reg2 ;
    OV5645SENSORDB("[OV5645]enter OV5645ReadExtraShutter function:\n ");
	temp_reg1 = OV5645_read_cmos_sensor(0x350c);    // AEC[b15~b8]
	temp_reg2 = OV5645_read_cmos_sensor(0x350d);    // AEC[b7~b0]
	spin_lock(&ov5645_drv_lock);
    OV5645MIPISensor.PreviewExtraShutter  = ((temp_reg1<<8)| temp_reg2);
	spin_unlock(&ov5645_drv_lock);
    OV5645SENSORDB("[OV5645]exit OV5645ReadExtraShutter function:\n ");
    return OV5645MIPISensor.PreviewExtraShutter;
} /* OV5645_read_shutter */
#endif
/*************************************************************************
* FUNCTION
*	OV5645ReadSensorGain
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
kal_uint32 OV5645ReadSensorGain(void)
{
	kal_uint32 sensor_gain = 0;
	sensor_gain=(OV5645_read_cmos_sensor(0x350B)&0xFF);
	return sensor_gain;
}  /* OV5645ReadSensorGain */

void OV5645_set_AE_mode(kal_bool AE_enable)
{
	kal_uint8 AeTemp;
	AeTemp = OV5645_read_cmos_sensor(0x3503);
	if (AE_enable == KAL_TRUE)
	{
		OV5645_write_cmos_sensor(0x3503,(AeTemp&(~0x07)));
		//OV5645_write_cmos_sensor(0x3503,(AeTemp&(~0x03)));
	}
	else
	{
		OV5645_write_cmos_sensor(0x3503,(AeTemp|0x07));
		//OV5645_write_cmos_sensor(0x3503,(AeTemp|0x03));
	}
}

void OV5645_set_AWB_mode(kal_bool AWB_enable)
{
	kal_uint8 AwbTemp;
	AwbTemp = OV5645_read_cmos_sensor(0x3406);

	if (AWB_enable == KAL_TRUE)
	{
		OV5645_write_cmos_sensor(0x3406,AwbTemp&0xFE);
	}
	else
	{
		OV5645_write_cmos_sensor(0x3406,AwbTemp|0x01);
	}
}
static void OV5645_set_AWB_mode_UNLOCK(void)
{
	OV5645_set_AWB_mode(KAL_TRUE);
	if (!((SCENE_MODE_OFF == OV5645MIPISensor.sceneMode) || (SCENE_MODE_NORMAL ==
		OV5645MIPISensor.sceneMode) || (SCENE_MODE_HDR == OV5645MIPISensor.sceneMode)))
	{
		OV5645_set_scene_mode(OV5645MIPISensor.sceneMode);
	}
	if (!((AWB_MODE_OFF == OV5645MIPISensor.iWB) || (AWB_MODE_AUTO == OV5645MIPISensor.iWB)))
	{
		OV5645SENSORDB("[OV5645]enter OV5645_set_AWB_mode_UNLOCK function:iWB=%d\n ",OV5645MIPISensor.iWB);
		OV5645_set_param_wb(OV5645MIPISensor.iWB);
	}
	return;
  }
/*************************************************************************
* FUNCTION
*	OV5645_GetSensorID
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
kal_uint32 OV5645_GetSensorID(kal_uint32 *sensorID)
{
	volatile signed char i;
	kal_uint32 sensor_id=0;
	OV5645_write_cmos_sensor(0x3008,0x82);// Reset sensor
	mDELAY(10);
	for(i=0;i<3;i++){
		sensor_id = (OV5645_read_cmos_sensor(0x300A) << 8) | OV5645_read_cmos_sensor(0x300B);
		OV5645SENSORDB("OV5645 READ ID: %x\n",sensor_id);
		if(sensor_id != OV5645MIPI_SENSOR_ID){
			*sensorID =0xffffffff;
		}
		else{
			*sensorID = OV5645PIP_MIPI_SENSOR_ID;
			break;
		}
	}

	return ERROR_NONE;
}

UINT32 OV5645PIPSetTestPatternMode(kal_bool bEnable)
{
    OV5645SENSORDB("test pattern bEnable:=%d\n",bEnable);
    if(bEnable)
    {
        OV5645_write_cmos_sensor(0x503d,0x80);
        run_test_potten=1;
    }
    else
    {
        OV5645_write_cmos_sensor(0x503d,0x00);
        run_test_potten=0;
    }
    return ERROR_NONE;
}

#ifdef DEBUG_SENSOR

#define OV5645PIP_OP_CODE_INI		0x00		/* Initial value. */
#define OV5645PIP_OP_CODE_REG		0x01		/* Register */
#define OV5645PIP_OP_CODE_DLY		0x02		/* Delay */
#define OV5645PIP_OP_CODE_END		0x03		/* End of initial setting. */

typedef struct
{
	u16 init_reg;
	u16 init_val;	/* Save the register value and delay tick */
	u8 op_code;		/* 0 - Initial value, 1 - Register, 2 - Delay, 3 - End of setting. */
} OV5645PIP_initial_set_struct;

 static OV5645PIP_initial_set_struct OV5645PIP_Init_Reg[1000];

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

int OV5645PIP_Initialize_from_T_Flash(void)
{
	u8 *curr_ptr = NULL;
	u32 file_size = 0;
	u32 i = 0, j = 0;
	u8 func_ind[4] = {0};	/* REG or DLY */

	struct file *fp; 
	mm_segment_t fs; 
	loff_t pos = 0; 
	static u8 data_buff[10*1024] ;
 
	fp = filp_open("/storage/sdcard0/ov5645pip_sd.dat", O_RDONLY , 0); 
	if (IS_ERR(fp)) { 
		printk("create file error\n"); 
		return 2;//-1; 
	} 
	else
		printk("OV5645PIP_Initialize_from_T_Flash Open File Success\n");
	
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
			OV5645PIP_Init_Reg[i].op_code = OV5645PIP_OP_CODE_REG;
			
			OV5645PIP_Init_Reg[i].init_reg = strtol((const char *)curr_ptr, 16);
			curr_ptr += 7;	/* Skip "0000,0x" */
		
			OV5645PIP_Init_Reg[i].init_val = strtol((const char *)curr_ptr, 16);
			curr_ptr += 4;	/* Skip "00);" */
		
		}
		else									/* DLY */
		{
			/* Need add delay for this setting. */
			curr_ptr += 4;	
			OV5645PIP_Init_Reg[i].op_code = OV5645PIP_OP_CODE_DLY;
			
			OV5645PIP_Init_Reg[i].init_reg = 0xFF;
			OV5645PIP_Init_Reg[i].init_val = strtol((const char *)curr_ptr,  10);	/* Get the delay ticks, the delay should less then 50 */
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
	OV5645PIP_Init_Reg[i].op_code = OV5645PIP_OP_CODE_END;
	OV5645PIP_Init_Reg[i].init_reg = 0xFF;
	OV5645PIP_Init_Reg[i].init_val = 0xFF;
	i++;
	for (j=0; j<i; j++)
		printk(" 0x%04x  ==  0x%02x\n",OV5645PIP_Init_Reg[j].init_reg, OV5645PIP_Init_Reg[j].init_val);

	/* Start apply the initial setting to sensor. */
	#if 1
	for (j=0; j<i; j++)
	{
		if (OV5645PIP_Init_Reg[j].op_code == OV5645PIP_OP_CODE_END)	/* End of the setting. */
		{
			break ;
		}
		else if (OV5645PIP_Init_Reg[j].op_code == OV5645PIP_OP_CODE_DLY)
		{
			msleep(OV5645PIP_Init_Reg[j].init_val);		/* Delay */
		}
		else if (OV5645PIP_Init_Reg[j].op_code == OV5645PIP_OP_CODE_REG)
		{
		
			OV5645_write_cmos_sensor(OV5645PIP_Init_Reg[j].init_reg, OV5645PIP_Init_Reg[j].init_val);
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

static void OV5645InitialSetting_common(void)
{
	OV5645_write_cmos_sensor(0x3103, 0x11);//	; PLL clock selection
	OV5645_write_cmos_sensor(0x3008, 0x82);//	; software reset
	mDELAY(5);   
	OV5645_write_cmos_sensor(0x3008, 0x42);//	; software power down
	OV5645_write_cmos_sensor(0x3103, 0x03);//	; clock from PLL
	OV5645_write_cmos_sensor(0x3503, 0x07);//	; AGC manual, AEC manual
	OV5645_write_cmos_sensor(0x3000, 0x30); 
	OV5645_write_cmos_sensor(0x3004, 0xef);//	; clock enable
	OV5645_write_cmos_sensor(0x3002, 0x1c);//	; system reset
	OV5645_write_cmos_sensor(0x3006, 0xc3);//	; clock enable
	OV5645_write_cmos_sensor(0x300e, 0x45);//	; MIPI 2 lane
	OV5645_write_cmos_sensor(0x3017, 0x40);//	; Frex, CSK input, Vsync output
	OV5645_write_cmos_sensor(0x3018, 0x00);//	; GPIO input
	OV5645_write_cmos_sensor(0x302c, 0x42);//	; GPIO input
	OV5645_write_cmos_sensor(0x302e, 0x0b);//
	OV5645_write_cmos_sensor(0x3031, 0x00);// ;regulater
	OV5645_write_cmos_sensor(0x3611, 0x06);//
	OV5645_write_cmos_sensor(0x3612, 0xab);//
	OV5645_write_cmos_sensor(0x3614, 0x50);//
	OV5645_write_cmos_sensor(0x3618, 0x00);//
	OV5645_write_cmos_sensor(0x4800, 0x04);//chage mipi data free/gate
	OV5645_write_cmos_sensor(0x3034, 0x18);//PLL, MIPI 8-bit mode
	OV5645_write_cmos_sensor(0x3035, 0x21);//PLL
	OV5645_write_cmos_sensor(0x3036, 0x60);//PLL
	OV5645_write_cmos_sensor(0x3037, 0x13);//PLL
	OV5645_write_cmos_sensor(0x3108, 0x01);//PLL
	OV5645_write_cmos_sensor(0x3824, 0x01);//PLL
	OV5645_write_cmos_sensor(0x460c, 0x22);//PLL
	OV5645_write_cmos_sensor(0x3500, 0x00);//	; exposure = 0x100
	OV5645_write_cmos_sensor(0x3501, 0x01);//	; exposure
	OV5645_write_cmos_sensor(0x3502, 0x00);//	; exposure
	OV5645_write_cmos_sensor(0x350a, 0x00);//	; gain = 0x3f
	OV5645_write_cmos_sensor(0x350b, 0x3f);//	; gain
	OV5645_write_cmos_sensor(0x3600, 0x09);//
	OV5645_write_cmos_sensor(0x3601, 0x43);//
	OV5645_write_cmos_sensor(0x3620, 0x33);//
	OV5645_write_cmos_sensor(0x3621, 0xe0);//
	OV5645_write_cmos_sensor(0x3622, 0x01);//
	OV5645_write_cmos_sensor(0x3630, 0x2d);//
	OV5645_write_cmos_sensor(0x3631, 0x00);//
	OV5645_write_cmos_sensor(0x3632, 0x32);//
	OV5645_write_cmos_sensor(0x3633, 0x52);//
	OV5645_write_cmos_sensor(0x3634, 0x70);//
	OV5645_write_cmos_sensor(0x3635, 0x13);//
	OV5645_write_cmos_sensor(0x3636, 0x03);//
	OV5645_write_cmos_sensor(0x3702, 0x6e);//
	OV5645_write_cmos_sensor(0x3703, 0x52);//
	OV5645_write_cmos_sensor(0x3704, 0xa0);//
	OV5645_write_cmos_sensor(0x3705, 0x33);//
	OV5645_write_cmos_sensor(0x3708, 0x66);//
	OV5645_write_cmos_sensor(0x3709, 0x12);//
	OV5645_write_cmos_sensor(0x370b, 0x61);//
	OV5645_write_cmos_sensor(0x370c, 0xc3);//
	OV5645_write_cmos_sensor(0x370f, 0x10);//
	OV5645_write_cmos_sensor(0x3715, 0x08);//
	OV5645_write_cmos_sensor(0x3717, 0x01);//
	OV5645_write_cmos_sensor(0x371b, 0x20);//
	OV5645_write_cmos_sensor(0x3731, 0x12);//
	OV5645_write_cmos_sensor(0x3739, 0x70);//
	OV5645_write_cmos_sensor(0x3901, 0x0a);//
	OV5645_write_cmos_sensor(0x3905, 0x02);//
	OV5645_write_cmos_sensor(0x3906, 0x10);//
	OV5645_write_cmos_sensor(0x3719, 0x86);//
	OV5645_write_cmos_sensor(0x3800, 0x00);//	; HS = 0
	OV5645_write_cmos_sensor(0x3801, 0x10);//0x00	; HS
	OV5645_write_cmos_sensor(0x3802, 0x00);//	; VS = 250
	OV5645_write_cmos_sensor(0x3803, 0xfa);//	; VS
	OV5645_write_cmos_sensor(0x3804, 0x0a);//	; HW = 2623
	OV5645_write_cmos_sensor(0x3805, 0x2f);//0x3f	; HW
	OV5645_write_cmos_sensor(0x3806, 0x06);//	; VH = 1705
	OV5645_write_cmos_sensor(0x3807, 0xa9);//	; VH
	OV5645_write_cmos_sensor(0x3808, 0x05);//	; DVPHO = 1280
	OV5645_write_cmos_sensor(0x3809, 0x00);//	; DVPHO
	OV5645_write_cmos_sensor(0x380a, 0x02);//    ; DVPHO
	OV5645_write_cmos_sensor(0x380b, 0xd0);//	; DVPVO
	OV5645_write_cmos_sensor(0x380c, 0x08);//	; HTS = 2160
	OV5645_write_cmos_sensor(0x380d, 0x70);//	; HTS
	OV5645_write_cmos_sensor(0x380e, 0x02);//	; VTS = 740
	OV5645_write_cmos_sensor(0x380f, 0xe4);//	; VTS
	OV5645_write_cmos_sensor(0x3810, 0x00);//	; H OFF = 16
	OV5645_write_cmos_sensor(0x3811, 0x08);//0x10	; H OFF
	OV5645_write_cmos_sensor(0x3812, 0x00);//	; V OFF = 4
	OV5645_write_cmos_sensor(0x3813, 0x04);//	; V OFF
	OV5645_write_cmos_sensor(0x3814, 0x31);//	; X INC
	OV5645_write_cmos_sensor(0x3815, 0x31);//	; Y INC
	OV5645_write_cmos_sensor(0x3820, 0x41);//	; flip off, V bin on
	OV5645_write_cmos_sensor(0x3821, 0x01);//	; mirror on, H bin on
	OV5645_write_cmos_sensor(0x3826, 0x03);//
	OV5645_write_cmos_sensor(0x3828, 0x08);//
	OV5645_write_cmos_sensor(0x3a02, 0x02);//	; max exp 60 = 740
	OV5645_write_cmos_sensor(0x3a03, 0xe4);//	; max exp 60
	OV5645_write_cmos_sensor(0x3a08, 0x00);//	; B50 = 222
	OV5645_write_cmos_sensor(0x3a09, 0xde);//	; B50
	OV5645_write_cmos_sensor(0x3a0a, 0x00);//	; B60 = 185
	OV5645_write_cmos_sensor(0x3a0b, 0xb9);//	; B60
	OV5645_write_cmos_sensor(0x3a0e, 0x03);//	; max 50
	OV5645_write_cmos_sensor(0x3a0d, 0x04);//	; max 60
	OV5645_write_cmos_sensor(0x3a14, 0x02);//	; max exp 50 = 740
	OV5645_write_cmos_sensor(0x3a15, 0xe4);//	; max exp 50
	OV5645_write_cmos_sensor(0x3a18, 0x00);//	; gain ceiling = 15.5x
	OV5645_write_cmos_sensor(0x3a19, 0xf8);//	; gain ceiling
	OV5645_write_cmos_sensor(0x3a05, 0x30);//    ; enable band insert, ken,
	OV5645_write_cmos_sensor(0x3c01, 0xb4); // ;manual banding mode
	OV5645_write_cmos_sensor(0x3c00, 0x04); // ;50 Banding mode
	OV5645_write_cmos_sensor(0x3c04, 0x28);//
	OV5645_write_cmos_sensor(0x3c05, 0x98);//
	OV5645_write_cmos_sensor(0x3c07, 0x07);//
	OV5645_write_cmos_sensor(0x3c08, 0x01);//
	OV5645_write_cmos_sensor(0x3c09, 0xc2);//
	OV5645_write_cmos_sensor(0x3c0a, 0x9c);//
	OV5645_write_cmos_sensor(0x3c0b, 0x40);//
	OV5645_write_cmos_sensor(0x4001, 0x02);//	; BLC start line
	OV5645_write_cmos_sensor(0x4004, 0x02);//	; BLC line number
	OV5645_write_cmos_sensor(0x4005, 0x18);//	; BLC update triggered by gain change
	OV5645_write_cmos_sensor(0x4050, 0x6e);//	; blc
	OV5645_write_cmos_sensor(0x4051, 0x8f);//
	OV5645_write_cmos_sensor(0x4300, 0x31);//    ; YUV 422, YUYV
	OV5645_write_cmos_sensor(0x4514, 0x00);//
	OV5645_write_cmos_sensor(0x4520, 0xb0);//
	OV5645_write_cmos_sensor(0x460b, 0x37);//
	OV5645_write_cmos_sensor(0x4818, 0x01);//	//; MIPI timing
	OV5645_write_cmos_sensor(0x481d, 0xf0);//
	OV5645_write_cmos_sensor(0x481f, 0x50);//
	OV5645_write_cmos_sensor(0x4823, 0x70);//
	OV5645_write_cmos_sensor(0x4831, 0x14);//
	OV5645_write_cmos_sensor(0x4837, 0x14);//
	OV5645_write_cmos_sensor(0x4b00, 0x45);//	; SPI receiver control
	OV5645_write_cmos_sensor(0x4b02, 0x01);//	; line start manual = 320
	OV5645_write_cmos_sensor(0x4b03, 0x40);//	; line start manual
	OV5645_write_cmos_sensor(0x4b04, 0x12);//	; SPI receiver conrol
	OV5645_write_cmos_sensor(0x4310, 0x01);//	; vsync width
	OV5645_write_cmos_sensor(0x5000, 0xa7);//	; Lenc on, raw gamma on, BPC on, WPC on, color interpolation on
	OV5645_write_cmos_sensor(0x5001, 0xa3);//	; SDE on, scale off, UV adjust off, color matrix on, AWB on
	OV5645_write_cmos_sensor(0x5002, 0x81);//
	OV5645_write_cmos_sensor(0x501d, 0x00);//
	OV5645_write_cmos_sensor(0x501f, 0x00);//	; select ISP YUV 422
	OV5645_write_cmos_sensor(0x503d, 0x00);//
	OV5645_write_cmos_sensor(0x505c, 0x30);//
	OV5645_write_cmos_sensor(0x5684, 0x10);//
	OV5645_write_cmos_sensor(0x5685, 0xa0);//
	OV5645_write_cmos_sensor(0x5686, 0x0c);//
	OV5645_write_cmos_sensor(0x5687, 0x78);//
	OV5645_write_cmos_sensor(0x5a00, 0x08);//
	OV5645_write_cmos_sensor(0x5a21, 0x00);//
	OV5645_write_cmos_sensor(0x5a24, 0x00);//
	OV5645_write_cmos_sensor(0x3008, 0x02);//	; wake up from software standby
	OV5645_write_cmos_sensor(0x3503, 0x00);//	; AGC on, AEC on
	OV5645_write_cmos_sensor(0x5180, 0xff);//awb
	OV5645_write_cmos_sensor(0x5181, 0xf3);
	OV5645_write_cmos_sensor(0x5182, 0x0 );
	OV5645_write_cmos_sensor(0x5183, 0x14);
	OV5645_write_cmos_sensor(0x5184, 0x25);
	OV5645_write_cmos_sensor(0x5185, 0x24);
	OV5645_write_cmos_sensor(0x5186, 0xe );
	OV5645_write_cmos_sensor(0x5187, 0x10);
	OV5645_write_cmos_sensor(0x5188, 0xb );
	OV5645_write_cmos_sensor(0x5189, 0x74);
	OV5645_write_cmos_sensor(0x518a, 0x54);
	OV5645_write_cmos_sensor(0x518b, 0xeb);
	OV5645_write_cmos_sensor(0x518c, 0xa8);
	OV5645_write_cmos_sensor(0x518d, 0x36);
	OV5645_write_cmos_sensor(0x518e, 0x2d);
	OV5645_write_cmos_sensor(0x518f, 0x51);
	OV5645_write_cmos_sensor(0x5190, 0x40);
	OV5645_write_cmos_sensor(0x5191, 0xf8);
	OV5645_write_cmos_sensor(0x5192, 0x4 );
	OV5645_write_cmos_sensor(0x5193, 0x70);
	OV5645_write_cmos_sensor(0x5194, 0xf0);
	OV5645_write_cmos_sensor(0x5195, 0xf0);
	OV5645_write_cmos_sensor(0x5196, 0x3 );
	OV5645_write_cmos_sensor(0x5197, 0x1 );
	OV5645_write_cmos_sensor(0x5198, 0x5 );
	OV5645_write_cmos_sensor(0x5199, 0xe5);
	OV5645_write_cmos_sensor(0x519a, 0x4 );
	OV5645_write_cmos_sensor(0x519b, 0x0 );
	OV5645_write_cmos_sensor(0x519c, 0x4 );
	OV5645_write_cmos_sensor(0x519d, 0x8f);
	OV5645_write_cmos_sensor(0x519e, 0x38);
	OV5645_write_cmos_sensor(0x5381, 0x1e);
	OV5645_write_cmos_sensor(0x5382, 0x5b);
	OV5645_write_cmos_sensor(0x5383, 0x08);
	OV5645_write_cmos_sensor(0x5384, 0x0a);
	OV5645_write_cmos_sensor(0x5385, 0x7e);
	OV5645_write_cmos_sensor(0x5386, 0x88);
	OV5645_write_cmos_sensor(0x5387, 0x7c);
	OV5645_write_cmos_sensor(0x5388, 0x6c);
	OV5645_write_cmos_sensor(0x5389, 0x10);
	OV5645_write_cmos_sensor(0x538a, 0x01);
	OV5645_write_cmos_sensor(0x538b, 0x98);
	OV5645_write_cmos_sensor(0x5300, 0x08);
	OV5645_write_cmos_sensor(0x5301, 0x30);
	OV5645_write_cmos_sensor(0x5302, 0x18);
	OV5645_write_cmos_sensor(0x5303, 0x08);
	OV5645_write_cmos_sensor(0x5304, 0x08);
	OV5645_write_cmos_sensor(0x5305, 0x30);
	OV5645_write_cmos_sensor(0x5306, 0x08);
	OV5645_write_cmos_sensor(0x5307, 0x16);
	OV5645_write_cmos_sensor(0x5309, 0x08);
	OV5645_write_cmos_sensor(0x530a, 0x30);
	OV5645_write_cmos_sensor(0x530b, 0x04);
	OV5645_write_cmos_sensor(0x530c, 0x06);
	OV5645_write_cmos_sensor(0x5480, 0x01);
	OV5645_write_cmos_sensor(0x5481, 0x08);
	OV5645_write_cmos_sensor(0x5482, 0x14);
	OV5645_write_cmos_sensor(0x5483, 0x28);
	OV5645_write_cmos_sensor(0x5484, 0x51);
	OV5645_write_cmos_sensor(0x5485, 0x65);
	OV5645_write_cmos_sensor(0x5486, 0x71);
	OV5645_write_cmos_sensor(0x5487, 0x7d);
	OV5645_write_cmos_sensor(0x5488, 0x87);
	OV5645_write_cmos_sensor(0x5489, 0x91);
	OV5645_write_cmos_sensor(0x548a, 0x9a);
	OV5645_write_cmos_sensor(0x548b, 0xaa);
	OV5645_write_cmos_sensor(0x548c, 0xb8);
	OV5645_write_cmos_sensor(0x548d, 0xcd);
	OV5645_write_cmos_sensor(0x548e, 0xdd);
	OV5645_write_cmos_sensor(0x548f, 0xea);
	OV5645_write_cmos_sensor(0x5490, 0x1d);
	OV5645_write_cmos_sensor(0x5580, 0x06);
	OV5645_write_cmos_sensor(0x5583, 0x40);
	OV5645_write_cmos_sensor(0x5584, 0x20);
	OV5645_write_cmos_sensor(0x5589, 0x10);
	OV5645_write_cmos_sensor(0x558a, 0x00);
	OV5645_write_cmos_sensor(0x558b, 0xf8);

	OV5645_write_cmos_sensor(0x5b04 ,0xFF);
	OV5645_write_cmos_sensor(0x5b05 ,0x80);
	OV5645_write_cmos_sensor(0x5b06 ,0x80);

//	OV5645_write_cmos_sensor(0x3a12 ,0x0a);
}

/*************************************************************************
* FUNCTION
*    OV5645InitialSetting_truly_af
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
//static 
void OV5645InitialSetting_truly_af(void)
{
    OV5645InitialSetting_common();

	//1 st for truly modules
	OV5645_write_cmos_sensor(0x5180, 0xff);//awb
	OV5645_write_cmos_sensor(0x5181, 0x50);//
	OV5645_write_cmos_sensor(0x5182, 0x11);//
	OV5645_write_cmos_sensor(0x5183, 0x14);//
	OV5645_write_cmos_sensor(0x5184, 0x25);//
	OV5645_write_cmos_sensor(0x5185, 0x24);//
	OV5645_write_cmos_sensor(0x5186, 0x1b);// 
	OV5645_write_cmos_sensor(0x5187, 0x18);//
	OV5645_write_cmos_sensor(0x5188, 0x18);// 
	OV5645_write_cmos_sensor(0x5189, 0x68);//0x72 60 0x68
	OV5645_write_cmos_sensor(0x518a, 0x55);//0x68 50 0x55
	OV5645_write_cmos_sensor(0x518b, 0xcb);//
	OV5645_write_cmos_sensor(0x518c, 0x87);//
	OV5645_write_cmos_sensor(0x518d, 0x3d);//
	OV5645_write_cmos_sensor(0x518e, 0x36);//
	OV5645_write_cmos_sensor(0x518f, 0x55);//
	OV5645_write_cmos_sensor(0x5190, 0x45);//
	OV5645_write_cmos_sensor(0x5191, 0xf8);//
	OV5645_write_cmos_sensor(0x5192, 0x04);// 
	OV5645_write_cmos_sensor(0x5193, 0x70);//
	OV5645_write_cmos_sensor(0x5194, 0xf0);//
	OV5645_write_cmos_sensor(0x5195, 0xf0);//
	OV5645_write_cmos_sensor(0x5196, 0x03);// 
	OV5645_write_cmos_sensor(0x5197, 0x01);// 
	OV5645_write_cmos_sensor(0x5198, 0x04);// 
	OV5645_write_cmos_sensor(0x5199, 0x00);//
	OV5645_write_cmos_sensor(0x519a, 0x04);//
	OV5645_write_cmos_sensor(0x519b, 0x68);// 
	OV5645_write_cmos_sensor(0x519c, 0x09);//
	OV5645_write_cmos_sensor(0x519d, 0xea);//
	OV5645_write_cmos_sensor(0x519e, 0x38);//

        //CCM
	OV5645_write_cmos_sensor(0x5381, 0x27);//ccm
	OV5645_write_cmos_sensor(0x5382, 0x50);
	OV5645_write_cmos_sensor(0x5383, 0x11);
	OV5645_write_cmos_sensor(0x5384, 0x0a);
	OV5645_write_cmos_sensor(0x5385, 0x66);
	OV5645_write_cmos_sensor(0x5386, 0x71);
	OV5645_write_cmos_sensor(0x5387, 0x7c);
	OV5645_write_cmos_sensor(0x5388, 0x6b);
	OV5645_write_cmos_sensor(0x5389, 0x11);
	OV5645_write_cmos_sensor(0x538a, 0x01);//
	OV5645_write_cmos_sensor(0x538b, 0x98);//

	//From OV Kobe sharpness
	OV5645_write_cmos_sensor(0x5308, 0x35);
	OV5645_write_cmos_sensor(0x5300, 0x08);//	; sharpen MT th1
	OV5645_write_cmos_sensor(0x5301, 0x48);//	; sharpen MT th2
	OV5645_write_cmos_sensor(0x5302, 0x20);//	; sharpen MT off1
	OV5645_write_cmos_sensor(0x5303, 0x00);//	; sharpen MT off2
	OV5645_write_cmos_sensor(0x5304, 0x08);//	; DNS th1
	OV5645_write_cmos_sensor(0x5305, 0x30);//	; DNS th2
	OV5645_write_cmos_sensor(0x5306, 0x10);//	; DNS off1 //0x08
	OV5645_write_cmos_sensor(0x5307, 0x20);//	; DNS off2 //0x16
	OV5645_write_cmos_sensor(0x5309, 0x08);//	; sharpen TH th1
	OV5645_write_cmos_sensor(0x530a, 0x30);//	; sharpen TH th2
	OV5645_write_cmos_sensor(0x530b, 0x04);//	; sharpen TH th2
	OV5645_write_cmos_sensor(0x530c, 0x06);//	; sharpen TH off2

	OV5645_write_cmos_sensor(0x5480, 0x01);//	; bias on
	OV5645_write_cmos_sensor(0x5481, 0x0a);//	; Y yst 00
	OV5645_write_cmos_sensor(0x5482, 0x16);//
	OV5645_write_cmos_sensor(0x5483, 0x2e);//
	OV5645_write_cmos_sensor(0x5484, 0x57);//
	OV5645_write_cmos_sensor(0x5485, 0x69);//
	OV5645_write_cmos_sensor(0x5486, 0x7a);//
	OV5645_write_cmos_sensor(0x5487, 0x88);//
	OV5645_write_cmos_sensor(0x5488, 0x93);//
	OV5645_write_cmos_sensor(0x5489, 0x9f);//
	OV5645_write_cmos_sensor(0x548a, 0xa9);//
	OV5645_write_cmos_sensor(0x548b, 0xb7);//
	OV5645_write_cmos_sensor(0x548c, 0xc0);//
	OV5645_write_cmos_sensor(0x548d, 0xd1);//
	OV5645_write_cmos_sensor(0x548e, 0xdf);//
	OV5645_write_cmos_sensor(0x548f, 0xea);//
	OV5645_write_cmos_sensor(0x5490, 0x1d);//

        //DPC laimingji 20130815
	OV5645_write_cmos_sensor(0x5780, 0xfc);//
	OV5645_write_cmos_sensor(0x5781, 0x13);//
	OV5645_write_cmos_sensor(0x5782, 0x03);//
	OV5645_write_cmos_sensor(0x5786, 0x20);//
	OV5645_write_cmos_sensor(0x5787, 0x40);//
	OV5645_write_cmos_sensor(0x5788, 0x08);//
	OV5645_write_cmos_sensor(0x5789, 0x08);//
	OV5645_write_cmos_sensor(0x578a, 0x02);//
	OV5645_write_cmos_sensor(0x578b, 0x01);//
	OV5645_write_cmos_sensor(0x578c, 0x01);//
	OV5645_write_cmos_sensor(0x578d, 0x0c);//
	OV5645_write_cmos_sensor(0x578e, 0x02);//
	OV5645_write_cmos_sensor(0x578f, 0x01);//
	OV5645_write_cmos_sensor(0x5790, 0x01);//
	OV5645_write_cmos_sensor(0x5588, 0x01);//
	OV5645_write_cmos_sensor(0x5580, 0x06);//uv
	OV5645_write_cmos_sensor(0x5583, 0x38);//0x40
	OV5645_write_cmos_sensor(0x5584, 0x20);//
	OV5645_write_cmos_sensor(0x5589, 0x18);//
	OV5645_write_cmos_sensor(0x558a, 0x00);//
	OV5645_write_cmos_sensor(0x558b, 0x3c);//

	OV5645_write_cmos_sensor(0x5800, 0x23);//dnp lsc
	OV5645_write_cmos_sensor(0x5801, 0x19);//
	OV5645_write_cmos_sensor(0x5802, 0x11);//
	OV5645_write_cmos_sensor(0x5803, 0x10);//
	OV5645_write_cmos_sensor(0x5804, 0x17);//
	OV5645_write_cmos_sensor(0x5805, 0x26);//
	OV5645_write_cmos_sensor(0x5806, 0x10);//                                   
	OV5645_write_cmos_sensor(0x5807, 0x08);//
	OV5645_write_cmos_sensor(0x5808, 0x05);//
	OV5645_write_cmos_sensor(0x5809, 0x05);//
	OV5645_write_cmos_sensor(0x580a, 0x08);//
	OV5645_write_cmos_sensor(0x580b, 0x0f);//
	OV5645_write_cmos_sensor(0x580c, 0x0a);//
	OV5645_write_cmos_sensor(0x580d, 0x03);//
	OV5645_write_cmos_sensor(0x580e, 0x00);//
	OV5645_write_cmos_sensor(0x580f, 0x00);//
	OV5645_write_cmos_sensor(0x5810, 0x03);//
	OV5645_write_cmos_sensor(0x5811, 0x09);//
	OV5645_write_cmos_sensor(0x5812, 0x09);//
	OV5645_write_cmos_sensor(0x5813, 0x04);//
	OV5645_write_cmos_sensor(0x5814, 0x00);//
	OV5645_write_cmos_sensor(0x5815, 0x00);//
	OV5645_write_cmos_sensor(0x5816, 0x03);//
	OV5645_write_cmos_sensor(0x5817, 0x08);//
	OV5645_write_cmos_sensor(0x5818, 0x14);//
	OV5645_write_cmos_sensor(0x5819, 0x0c);//
	OV5645_write_cmos_sensor(0x581a, 0x08);//
	OV5645_write_cmos_sensor(0x581b, 0x07);//
	OV5645_write_cmos_sensor(0x581c, 0x09);//
	OV5645_write_cmos_sensor(0x581d, 0x0f);//
	OV5645_write_cmos_sensor(0x581e, 0x2f);//
	OV5645_write_cmos_sensor(0x581f, 0x1e);//
	OV5645_write_cmos_sensor(0x5820, 0x17);//
	OV5645_write_cmos_sensor(0x5821, 0x17);//
	OV5645_write_cmos_sensor(0x5822, 0x1b);//
	OV5645_write_cmos_sensor(0x5823, 0x26);//
	OV5645_write_cmos_sensor(0x5824, 0x0a);//
	OV5645_write_cmos_sensor(0x5825, 0x26);//
	OV5645_write_cmos_sensor(0x5826, 0x2A);//
	OV5645_write_cmos_sensor(0x5827, 0x26);//
	OV5645_write_cmos_sensor(0x5828, 0x48);//
	OV5645_write_cmos_sensor(0x5829, 0x48);//
	OV5645_write_cmos_sensor(0x582a, 0x26);//
	OV5645_write_cmos_sensor(0x582b, 0x24);//
	OV5645_write_cmos_sensor(0x582c, 0x26);//
	OV5645_write_cmos_sensor(0x582d, 0x08);//
	OV5645_write_cmos_sensor(0x582e, 0x26);//
	OV5645_write_cmos_sensor(0x582f, 0x42);//
	OV5645_write_cmos_sensor(0x5830, 0x40);//
	OV5645_write_cmos_sensor(0x5831, 0x22);//
	OV5645_write_cmos_sensor(0x5832, 0x28);//
	OV5645_write_cmos_sensor(0x5833, 0x28);//
	OV5645_write_cmos_sensor(0x5834, 0x24);//
	OV5645_write_cmos_sensor(0x5835, 0x24);//
	OV5645_write_cmos_sensor(0x5836, 0x26);//
	OV5645_write_cmos_sensor(0x5837, 0x08);//
	OV5645_write_cmos_sensor(0x5838, 0x06);//
	OV5645_write_cmos_sensor(0x5839, 0x28);//
	OV5645_write_cmos_sensor(0x583a, 0x2a);//
	OV5645_write_cmos_sensor(0x583b, 0x28);//
	OV5645_write_cmos_sensor(0x583c, 0x2a);//
	OV5645_write_cmos_sensor(0x583d, 0xce);//

	OV5645_write_cmos_sensor(0x3a00, 0x38);//	; ae mode	
	OV5645_write_cmos_sensor(0x3a0f, 0x29);//	; AEC in H
	OV5645_write_cmos_sensor(0x3a10, 0x23);//	; AEC in L
	OV5645_write_cmos_sensor(0x3a1b, 0x29);//	; AEC out H
	OV5645_write_cmos_sensor(0x3a1e, 0x23);//	; AEC out L
	OV5645_write_cmos_sensor(0x3a11, 0x53);//	; control zone H
	OV5645_write_cmos_sensor(0x3a1f, 0x12);//	; control zone L	
	OV5645_write_cmos_sensor(0x501d, 0x00);//	; control zone L	
}


/*************************************************************************
* FUNCTION
*    OV5645InitialSetting_truly_ff
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
//static 
void OV5645InitialSetting_truly_ff(void)
{
    OV5645InitialSetting_common();

	//1 st for truly modules
	OV5645_write_cmos_sensor(0x5180, 0xff);//awb
	OV5645_write_cmos_sensor(0x5181, 0x50);//
	OV5645_write_cmos_sensor(0x5182, 0x11);//
	OV5645_write_cmos_sensor(0x5183, 0x14);//
	OV5645_write_cmos_sensor(0x5184, 0x25);//
	OV5645_write_cmos_sensor(0x5185, 0x24);//
	OV5645_write_cmos_sensor(0x5186, 0x1b);// 
	OV5645_write_cmos_sensor(0x5187, 0x18);//
	OV5645_write_cmos_sensor(0x5188, 0x18);// 
	OV5645_write_cmos_sensor(0x5189, 0x68);//0x72 60 0x68
	OV5645_write_cmos_sensor(0x518a, 0x55);//0x68 50 0x55
	OV5645_write_cmos_sensor(0x518b, 0xcb);//
	OV5645_write_cmos_sensor(0x518c, 0x87);//
	OV5645_write_cmos_sensor(0x518d, 0x3d);//
	OV5645_write_cmos_sensor(0x518e, 0x36);//
	OV5645_write_cmos_sensor(0x518f, 0x55);//
	OV5645_write_cmos_sensor(0x5190, 0x45);//
	OV5645_write_cmos_sensor(0x5191, 0xf8);//
	OV5645_write_cmos_sensor(0x5192, 0x04);// 
	OV5645_write_cmos_sensor(0x5193, 0x70);//
	OV5645_write_cmos_sensor(0x5194, 0xf0);//
	OV5645_write_cmos_sensor(0x5195, 0xf0);//
	OV5645_write_cmos_sensor(0x5196, 0x03);// 
	OV5645_write_cmos_sensor(0x5197, 0x01);// 
	OV5645_write_cmos_sensor(0x5198, 0x04);// 
	OV5645_write_cmos_sensor(0x5199, 0x00);//
	OV5645_write_cmos_sensor(0x519a, 0x04);//
	OV5645_write_cmos_sensor(0x519b, 0x68);// 
	OV5645_write_cmos_sensor(0x519c, 0x09);//
	OV5645_write_cmos_sensor(0x519d, 0xea);//
	OV5645_write_cmos_sensor(0x519e, 0x38);//

	OV5645_write_cmos_sensor(0x5381, 0x21);//CCM
	OV5645_write_cmos_sensor(0x5382, 0x54);
	OV5645_write_cmos_sensor(0x5383, 0x15);
	OV5645_write_cmos_sensor(0x5384, 0x08);
	OV5645_write_cmos_sensor(0x5385, 0x75);
	OV5645_write_cmos_sensor(0x5386, 0x7D);
	OV5645_write_cmos_sensor(0x5387, 0x81);
	OV5645_write_cmos_sensor(0x5388, 0x74);
	OV5645_write_cmos_sensor(0x5389, 0x0D);
	OV5645_write_cmos_sensor(0x538a, 0x01);//
	OV5645_write_cmos_sensor(0x538b, 0x98);//

	OV5645_write_cmos_sensor(0x5308, 0x35);
	OV5645_write_cmos_sensor(0x5300, 0x08);//	; sharpen MT th1
	OV5645_write_cmos_sensor(0x5301, 0x48);//	; sharpen MT th2
	OV5645_write_cmos_sensor(0x5302, 0x20);//	; sharpen MT off1
	OV5645_write_cmos_sensor(0x5303, 0x00);//	; sharpen MT off2
	OV5645_write_cmos_sensor(0x5304, 0x08);//	; DNS th1
	OV5645_write_cmos_sensor(0x5305, 0x30);//	; DNS th2
	OV5645_write_cmos_sensor(0x5306, 0x10);//	; DNS off1 //0x08
	OV5645_write_cmos_sensor(0x5307, 0x20);//	; DNS off2 //0x16
	OV5645_write_cmos_sensor(0x5309, 0x08);//	; sharpen TH th1
	OV5645_write_cmos_sensor(0x530a, 0x30);//	; sharpen TH th2
	OV5645_write_cmos_sensor(0x530b, 0x04);//	; sharpen TH th2
	OV5645_write_cmos_sensor(0x530c, 0x06);//	; sharpen TH off2

	OV5645_write_cmos_sensor(0x5480, 0x01);//	; bias on
	OV5645_write_cmos_sensor(0x5481, 0x0a);//	; Y yst 00
	OV5645_write_cmos_sensor(0x5482, 0x16);//
	OV5645_write_cmos_sensor(0x5483, 0x2e);//
	OV5645_write_cmos_sensor(0x5484, 0x57);//
	OV5645_write_cmos_sensor(0x5485, 0x69);//
	OV5645_write_cmos_sensor(0x5486, 0x7a);//
	OV5645_write_cmos_sensor(0x5487, 0x88);//
	OV5645_write_cmos_sensor(0x5488, 0x93);//
	OV5645_write_cmos_sensor(0x5489, 0x9f);//
	OV5645_write_cmos_sensor(0x548a, 0xa9);//
	OV5645_write_cmos_sensor(0x548b, 0xb7);//
	OV5645_write_cmos_sensor(0x548c, 0xc0);//
	OV5645_write_cmos_sensor(0x548d, 0xd1);//
	OV5645_write_cmos_sensor(0x548e, 0xdf);//
	OV5645_write_cmos_sensor(0x548f, 0xea);//
	OV5645_write_cmos_sensor(0x5490, 0x1d);//

        //DPC laimingji 20130815
	OV5645_write_cmos_sensor(0x5780, 0xfc);//
	OV5645_write_cmos_sensor(0x5781, 0x13);//
	OV5645_write_cmos_sensor(0x5782, 0x03);//
	OV5645_write_cmos_sensor(0x5786, 0x20);//
	OV5645_write_cmos_sensor(0x5787, 0x40);//
	OV5645_write_cmos_sensor(0x5788, 0x08);//
	OV5645_write_cmos_sensor(0x5789, 0x08);//
	OV5645_write_cmos_sensor(0x578a, 0x02);//
	OV5645_write_cmos_sensor(0x578b, 0x01);//
	OV5645_write_cmos_sensor(0x578c, 0x01);//
	OV5645_write_cmos_sensor(0x578d, 0x0c);//
	OV5645_write_cmos_sensor(0x578e, 0x02);//
	OV5645_write_cmos_sensor(0x578f, 0x01);//
	OV5645_write_cmos_sensor(0x5790, 0x01);//

	OV5645_write_cmos_sensor(0x5580, 0x06);//UV
	OV5645_write_cmos_sensor(0x5588, 0x01);//
	OV5645_write_cmos_sensor(0x5583, 0x40);//
	OV5645_write_cmos_sensor(0x5584, 0x20);//
	OV5645_write_cmos_sensor(0x5589, 0x18);//
	OV5645_write_cmos_sensor(0x558a, 0x00);//
	OV5645_write_cmos_sensor(0x558b, 0x3c);//

	//From OV Kobe DNP
	OV5645_write_cmos_sensor(0x5800, 0x1C);//lsc
	OV5645_write_cmos_sensor(0x5801, 0x17);//
	OV5645_write_cmos_sensor(0x5802, 0x12);//
	OV5645_write_cmos_sensor(0x5803, 0x12);//
	OV5645_write_cmos_sensor(0x5804, 0x16);//
	OV5645_write_cmos_sensor(0x5805, 0x18);//
	OV5645_write_cmos_sensor(0x5806, 0x11);//
	OV5645_write_cmos_sensor(0x5807, 0x0A);//
	OV5645_write_cmos_sensor(0x5808, 0x06);//
	OV5645_write_cmos_sensor(0x5809, 0x06);//
	OV5645_write_cmos_sensor(0x580a, 0x08);//
	OV5645_write_cmos_sensor(0x580b, 0x0F);//
	OV5645_write_cmos_sensor(0x580c, 0x0A);//
	OV5645_write_cmos_sensor(0x580d, 0x05);//
	OV5645_write_cmos_sensor(0x580e, 0x00);//
	OV5645_write_cmos_sensor(0x580f, 0x00);//
	OV5645_write_cmos_sensor(0x5810, 0x03);//
	OV5645_write_cmos_sensor(0x5811, 0x0A);//
	OV5645_write_cmos_sensor(0x5812, 0x0E);//
	OV5645_write_cmos_sensor(0x5813, 0x05);//
	OV5645_write_cmos_sensor(0x5814, 0x01);//
	OV5645_write_cmos_sensor(0x5815, 0x00);//
	OV5645_write_cmos_sensor(0x5816, 0x04);//
	OV5645_write_cmos_sensor(0x5817, 0x0A);//
	OV5645_write_cmos_sensor(0x5818, 0x0C);//
	OV5645_write_cmos_sensor(0x5819, 0x0D);//
	OV5645_write_cmos_sensor(0x581a, 0x08);//
	OV5645_write_cmos_sensor(0x581b, 0x08);//
	OV5645_write_cmos_sensor(0x581c, 0x0B);//
	OV5645_write_cmos_sensor(0x581d, 0x12);//
	OV5645_write_cmos_sensor(0x581e, 0x3E);//
	OV5645_write_cmos_sensor(0x581f, 0x11);//
	OV5645_write_cmos_sensor(0x5820, 0x15);//
	OV5645_write_cmos_sensor(0x5821, 0x12);//
	OV5645_write_cmos_sensor(0x5822, 0x16);//
	OV5645_write_cmos_sensor(0x5823, 0x19);//
	OV5645_write_cmos_sensor(0x5824, 0x46);//
	OV5645_write_cmos_sensor(0x5825, 0x28);//
	OV5645_write_cmos_sensor(0x5826, 0x0A);//
	OV5645_write_cmos_sensor(0x5827, 0x26);//
	OV5645_write_cmos_sensor(0x5828, 0x2A);//
	OV5645_write_cmos_sensor(0x5829, 0x2A);//
	OV5645_write_cmos_sensor(0x582a, 0x26);//
	OV5645_write_cmos_sensor(0x582b, 0x24);//
	OV5645_write_cmos_sensor(0x582c, 0x26);//
	OV5645_write_cmos_sensor(0x582d, 0x26);//
	OV5645_write_cmos_sensor(0x582e, 0x0C);//
	OV5645_write_cmos_sensor(0x582f, 0x22);//
	OV5645_write_cmos_sensor(0x5830, 0x40);//
	OV5645_write_cmos_sensor(0x5831, 0x22);//
	OV5645_write_cmos_sensor(0x5832, 0x08);//
	OV5645_write_cmos_sensor(0x5833, 0x2A);//
	OV5645_write_cmos_sensor(0x5834, 0x28);//
	OV5645_write_cmos_sensor(0x5835, 0x06);//
	OV5645_write_cmos_sensor(0x5836, 0x26);//
	OV5645_write_cmos_sensor(0x5837, 0x26);//
	OV5645_write_cmos_sensor(0x5838, 0x26);//
	OV5645_write_cmos_sensor(0x5839, 0x28);//
	OV5645_write_cmos_sensor(0x583a, 0x2A);//
	OV5645_write_cmos_sensor(0x583b, 0x28);//
	OV5645_write_cmos_sensor(0x583c, 0x44);//
	OV5645_write_cmos_sensor(0x583d, 0xCE);//

	OV5645_write_cmos_sensor(0x583e, 0x10);//
	OV5645_write_cmos_sensor(0x583f, 0x08);//
	OV5645_write_cmos_sensor(0x5840, 0x00);//
	OV5645_write_cmos_sensor(0x5025, 0x00);//
	OV5645_write_cmos_sensor(0x3a00, 0x38);//	; ae mode	
	OV5645_write_cmos_sensor(0x3a0f, 0x29);//	; AEC in H
	OV5645_write_cmos_sensor(0x3a10, 0x23);//	; AEC in L
	OV5645_write_cmos_sensor(0x3a1b, 0x29);//	; AEC out H
	OV5645_write_cmos_sensor(0x3a1e, 0x23);//	; AEC out L
	OV5645_write_cmos_sensor(0x3a11, 0x53);//	; control zone H
	OV5645_write_cmos_sensor(0x3a1f, 0x12);//	; control zone L	
	OV5645_write_cmos_sensor(0x501d, 0x00);//	; control zone L	
}


/*************************************************************************
* FUNCTION
*    OV5645InitialSetting_truly_af
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
//static 
void OV5645InitialSetting_darling_af(void)
{
	OV5645InitialSetting_common();

	//1 st for darling modules
	OV5645_write_cmos_sensor(0x5180, 0xff);//awb
	OV5645_write_cmos_sensor(0x5181, 0x50);//
	OV5645_write_cmos_sensor(0x5182, 0x11);//
	OV5645_write_cmos_sensor(0x5183, 0x14);//
	OV5645_write_cmos_sensor(0x5184, 0x25);//
	OV5645_write_cmos_sensor(0x5185, 0x24);//
	OV5645_write_cmos_sensor(0x5186, 0x1b);// 
	OV5645_write_cmos_sensor(0x5187, 0x18);//
	OV5645_write_cmos_sensor(0x5188, 0x18);// 
	OV5645_write_cmos_sensor(0x5189, 0x68);//0x72 60 0x68
	OV5645_write_cmos_sensor(0x518a, 0x55);//0x68 50 0x55
	OV5645_write_cmos_sensor(0x518b, 0xcb);//
	OV5645_write_cmos_sensor(0x518c, 0x87);//
	OV5645_write_cmos_sensor(0x518d, 0x3d);//
	OV5645_write_cmos_sensor(0x518e, 0x36);//
	OV5645_write_cmos_sensor(0x518f, 0x55);//
	OV5645_write_cmos_sensor(0x5190, 0x45);//
	OV5645_write_cmos_sensor(0x5191, 0xf8);//
	OV5645_write_cmos_sensor(0x5192, 0x04);// 
	OV5645_write_cmos_sensor(0x5193, 0x70);//
	OV5645_write_cmos_sensor(0x5194, 0xf0);//
	OV5645_write_cmos_sensor(0x5195, 0xf0);//
	OV5645_write_cmos_sensor(0x5196, 0x03);// 
	OV5645_write_cmos_sensor(0x5197, 0x01);// 
	OV5645_write_cmos_sensor(0x5198, 0x04);// 
	OV5645_write_cmos_sensor(0x5199, 0x00);//
	OV5645_write_cmos_sensor(0x519a, 0x04);//
	OV5645_write_cmos_sensor(0x519b, 0x68);// 
	OV5645_write_cmos_sensor(0x519c, 0x09);//
	OV5645_write_cmos_sensor(0x519d, 0xea);//
	OV5645_write_cmos_sensor(0x519e, 0x38);//

        //CCM
	OV5645_write_cmos_sensor(0x5381, 0x21);//ccm
	OV5645_write_cmos_sensor(0x5382, 0x54);
	OV5645_write_cmos_sensor(0x5383, 0x15);
    OV5645_write_cmos_sensor(0x5384,0x08);
	OV5645_write_cmos_sensor(0x5385, 0x75);
	OV5645_write_cmos_sensor(0x5386, 0x7D);
	OV5645_write_cmos_sensor(0x5387, 0x81);
	OV5645_write_cmos_sensor(0x5388, 0x74);
	OV5645_write_cmos_sensor(0x5389, 0x0D);
	OV5645_write_cmos_sensor(0x538a, 0x01);//
	OV5645_write_cmos_sensor(0x538b, 0x98);//
  
	//From OV Kobe sharpness
	OV5645_write_cmos_sensor(0x5308, 0x35);
	OV5645_write_cmos_sensor(0x5300, 0x08);//	; sharpen MT th1
	OV5645_write_cmos_sensor(0x5301, 0x48);//	; sharpen MT th2
	OV5645_write_cmos_sensor(0x5302, 0x20);//	; sharpen MT off1
	OV5645_write_cmos_sensor(0x5303, 0x00);//	; sharpen MT off2
	OV5645_write_cmos_sensor(0x5304, 0x08);//	; DNS th1
	OV5645_write_cmos_sensor(0x5305, 0x30);//	; DNS th2
	OV5645_write_cmos_sensor(0x5306, 0x10);//	; DNS off1 //0x08
	OV5645_write_cmos_sensor(0x5307, 0x20);//	; DNS off2 //0x16
	OV5645_write_cmos_sensor(0x5309, 0x08);//	; sharpen TH th1
	OV5645_write_cmos_sensor(0x530a, 0x30);//	; sharpen TH th2
	OV5645_write_cmos_sensor(0x530b, 0x04);//	; sharpen TH th2
	OV5645_write_cmos_sensor(0x530c, 0x06);//	; sharpen TH off2

	OV5645_write_cmos_sensor(0x5480, 0x01);//	; bias on
	OV5645_write_cmos_sensor(0x5481, 0x08);//	; Y yst 00
	OV5645_write_cmos_sensor(0x5482, 0x14);//
	OV5645_write_cmos_sensor(0x5483, 0x28);//
	OV5645_write_cmos_sensor(0x5484, 0x51);//
	OV5645_write_cmos_sensor(0x5485, 0x65);//
	OV5645_write_cmos_sensor(0x5486, 0x71);//
	OV5645_write_cmos_sensor(0x5487, 0x7d);//
	OV5645_write_cmos_sensor(0x5488, 0x87);//
	OV5645_write_cmos_sensor(0x5489, 0x91);//
	OV5645_write_cmos_sensor(0x548a, 0x9a);//
	OV5645_write_cmos_sensor(0x548b, 0xaa);//
	OV5645_write_cmos_sensor(0x548c, 0xb8);//
	OV5645_write_cmos_sensor(0x548d, 0xcd);//
	OV5645_write_cmos_sensor(0x548e, 0xdd);//
	OV5645_write_cmos_sensor(0x548f, 0xea);//
	OV5645_write_cmos_sensor(0x5490, 0x1d);//

        //DPC laimingji 20130815
	OV5645_write_cmos_sensor(0x5780, 0xfc);//
	OV5645_write_cmos_sensor(0x5781, 0x13);//
	OV5645_write_cmos_sensor(0x5782, 0x03);//
	OV5645_write_cmos_sensor(0x5786, 0x20);//
	OV5645_write_cmos_sensor(0x5787, 0x40);//
	OV5645_write_cmos_sensor(0x5788, 0x08);//
	OV5645_write_cmos_sensor(0x5789, 0x08);//
	OV5645_write_cmos_sensor(0x578a, 0x02);//
	OV5645_write_cmos_sensor(0x578b, 0x01);//
	OV5645_write_cmos_sensor(0x578c, 0x01);//
	OV5645_write_cmos_sensor(0x578d, 0x0c);//
	OV5645_write_cmos_sensor(0x578e, 0x02);//
	OV5645_write_cmos_sensor(0x578f, 0x01);//
	OV5645_write_cmos_sensor(0x5790, 0x01);//
	OV5645_write_cmos_sensor(0x5580, 0x06);//uv
	OV5645_write_cmos_sensor(0x5588, 0x01);//
	OV5645_write_cmos_sensor(0x5583, 0x40);//
	OV5645_write_cmos_sensor(0x5584, 0x20);//
	OV5645_write_cmos_sensor(0x5589, 0x18);//
	OV5645_write_cmos_sensor(0x558a, 0x00);//
	OV5645_write_cmos_sensor(0x558b, 0x3c);//

	//From OV Kobe DNP.
	OV5645_write_cmos_sensor(0x5800, 0x23);//lsc
	OV5645_write_cmos_sensor(0x5801, 0x16);//
	OV5645_write_cmos_sensor(0x5802, 0x0f);//
	OV5645_write_cmos_sensor(0x5803, 0x0f);//
	OV5645_write_cmos_sensor(0x5804, 0x16);//
	OV5645_write_cmos_sensor(0x5805, 0x29);//
	OV5645_write_cmos_sensor(0x5806, 0x0c);//
	OV5645_write_cmos_sensor(0x5807, 0x08);//
	OV5645_write_cmos_sensor(0x5808, 0x05);//
	OV5645_write_cmos_sensor(0x5809, 0x06);//
	OV5645_write_cmos_sensor(0x580a, 0x08);//
	OV5645_write_cmos_sensor(0x580b, 0x0F);//
	OV5645_write_cmos_sensor(0x580c, 0x08);//
	OV5645_write_cmos_sensor(0x580d, 0x03);//
	OV5645_write_cmos_sensor(0x580e, 0x00);//
	OV5645_write_cmos_sensor(0x580f, 0x00);//
	OV5645_write_cmos_sensor(0x5810, 0x04);//
	OV5645_write_cmos_sensor(0x5811, 0x09);//
	OV5645_write_cmos_sensor(0x5812, 0x07);//
	OV5645_write_cmos_sensor(0x5813, 0x04);//
	OV5645_write_cmos_sensor(0x5814, 0x00);//
	OV5645_write_cmos_sensor(0x5815, 0x01);//
	OV5645_write_cmos_sensor(0x5816, 0x05);//
	OV5645_write_cmos_sensor(0x5817, 0x09);//
	OV5645_write_cmos_sensor(0x5818, 0x10);//
	OV5645_write_cmos_sensor(0x5819, 0x09);//
	OV5645_write_cmos_sensor(0x581a, 0x06);//
	OV5645_write_cmos_sensor(0x581b, 0x07);//
	OV5645_write_cmos_sensor(0x581c, 0x0a);//
	OV5645_write_cmos_sensor(0x581d, 0x13);//
	OV5645_write_cmos_sensor(0x581e, 0x26);//
	OV5645_write_cmos_sensor(0x581f, 0x1c);//
	OV5645_write_cmos_sensor(0x5820, 0x14);//
	OV5645_write_cmos_sensor(0x5821, 0x14);//
	OV5645_write_cmos_sensor(0x5822, 0x1f);//
	OV5645_write_cmos_sensor(0x5823, 0x29);//
	OV5645_write_cmos_sensor(0x5824, 0x2a);//
	OV5645_write_cmos_sensor(0x5825, 0x2a);//
	OV5645_write_cmos_sensor(0x5826, 0x0c);//
	OV5645_write_cmos_sensor(0x5827, 0x0a);//
	OV5645_write_cmos_sensor(0x5828, 0x28);//
	OV5645_write_cmos_sensor(0x5829, 0x0c);//
	OV5645_write_cmos_sensor(0x582a, 0x26);//
	OV5645_write_cmos_sensor(0x582b, 0x24);//
	OV5645_write_cmos_sensor(0x582c, 0x26);//
	OV5645_write_cmos_sensor(0x582d, 0x2a);//
	OV5645_write_cmos_sensor(0x582e, 0x0C);//
	OV5645_write_cmos_sensor(0x582f, 0x42);//
	OV5645_write_cmos_sensor(0x5830, 0x40);//
	OV5645_write_cmos_sensor(0x5831, 0x22);//
	OV5645_write_cmos_sensor(0x5832, 0x0a);//
	OV5645_write_cmos_sensor(0x5833, 0x2A);//
	OV5645_write_cmos_sensor(0x5834, 0x26);//
	OV5645_write_cmos_sensor(0x5835, 0x26);//
	OV5645_write_cmos_sensor(0x5836, 0x26);//
	OV5645_write_cmos_sensor(0x5837, 0x2a);//
	OV5645_write_cmos_sensor(0x5838, 0x44);//
	OV5645_write_cmos_sensor(0x5839, 0x28);//
	OV5645_write_cmos_sensor(0x583a, 0x2A);//
	OV5645_write_cmos_sensor(0x583b, 0x48);//
	OV5645_write_cmos_sensor(0x583c, 0x42);//
	OV5645_write_cmos_sensor(0x583d, 0xCE);//

	OV5645_write_cmos_sensor(0x583e, 0x10);//
	OV5645_write_cmos_sensor(0x583f, 0x08);//
	OV5645_write_cmos_sensor(0x5840, 0x00);//
	OV5645_write_cmos_sensor(0x5025, 0x00);//
    OV5645_write_cmos_sensor(0x3a00, 0x38);//    ; ae mode
	OV5645_write_cmos_sensor(0x3a0f, 0x29);//	; AEC in H
	OV5645_write_cmos_sensor(0x3a10, 0x23);//	; AEC in L
	OV5645_write_cmos_sensor(0x3a1b, 0x29);//	; AEC out H
	OV5645_write_cmos_sensor(0x3a1e, 0x23);//	; AEC out L
	OV5645_write_cmos_sensor(0x3a11, 0x53);//	; control zone H
	OV5645_write_cmos_sensor(0x3a1f, 0x12);//	; control zone L	
	OV5645_write_cmos_sensor(0x501d, 0x00);//	; control zone L	
}


/*************************************************************************
* FUNCTION
*    OV5645InitialSetting_truly_af
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
//static 
void OV5645InitialSetting_darling_ff(void)
{
	OV5645InitialSetting_common();

	//1 st for darling modules
	OV5645_write_cmos_sensor(0x5180, 0xff);//awb
	OV5645_write_cmos_sensor(0x5181, 0xf2);//
	OV5645_write_cmos_sensor(0x5182, 0x00);//
	OV5645_write_cmos_sensor(0x5183, 0x14);//
	OV5645_write_cmos_sensor(0x5184, 0x25);//
	OV5645_write_cmos_sensor(0x5185, 0x24);//
	OV5645_write_cmos_sensor(0x5186, 0x0e);//
	OV5645_write_cmos_sensor(0x5187, 0x16);//
	OV5645_write_cmos_sensor(0x5188, 0x0d);//
	OV5645_write_cmos_sensor(0x5189, 0x74);//0x75
	OV5645_write_cmos_sensor(0x518a, 0x54);//
	OV5645_write_cmos_sensor(0x518b, 0xd8);//0xcc
	OV5645_write_cmos_sensor(0x518c, 0x84);//
	OV5645_write_cmos_sensor(0x518d, 0x3b);//
	OV5645_write_cmos_sensor(0x518e, 0x31);//
	OV5645_write_cmos_sensor(0x518f, 0x52);//
	OV5645_write_cmos_sensor(0x5190, 0x3d);//
	OV5645_write_cmos_sensor(0x5191, 0xf8);//
	OV5645_write_cmos_sensor(0x5192, 0x04);//
	OV5645_write_cmos_sensor(0x5193, 0x70);//
	OV5645_write_cmos_sensor(0x5194, 0xf0);//
	OV5645_write_cmos_sensor(0x5195, 0xf0);//
	OV5645_write_cmos_sensor(0x5196, 0x03);//
	OV5645_write_cmos_sensor(0x5197, 0x01);//
	OV5645_write_cmos_sensor(0x5198, 0x05);//
	OV5645_write_cmos_sensor(0x5199, 0x18);//
	OV5645_write_cmos_sensor(0x519a, 0x04);//
	OV5645_write_cmos_sensor(0x519b, 0x00);//
	OV5645_write_cmos_sensor(0x519c, 0x07);//
	OV5645_write_cmos_sensor(0x519d, 0x33);//
	OV5645_write_cmos_sensor(0x519e, 0x38);//

        //CCM
	OV5645_write_cmos_sensor(0x5381, 0x21);//ccm
	OV5645_write_cmos_sensor(0x5382, 0x54);
	OV5645_write_cmos_sensor(0x5383, 0x15);
	OV5645_write_cmos_sensor(0x5384, 0x08);
	OV5645_write_cmos_sensor(0x5385, 0x75);
	OV5645_write_cmos_sensor(0x5386, 0x7D);
	OV5645_write_cmos_sensor(0x5387, 0x81);
	OV5645_write_cmos_sensor(0x5388, 0x74);
	OV5645_write_cmos_sensor(0x5389, 0x0D);
	OV5645_write_cmos_sensor(0x538a, 0x01);//
	OV5645_write_cmos_sensor(0x538b, 0x98);//

	//From OV Kobe sharpness
	OV5645_write_cmos_sensor(0x5308, 0x35);
	OV5645_write_cmos_sensor(0x5300, 0x08);//	; sharpen MT th1
	OV5645_write_cmos_sensor(0x5301, 0x48);//	; sharpen MT th2
	OV5645_write_cmos_sensor(0x5302, 0x20);//	; sharpen MT off1
	OV5645_write_cmos_sensor(0x5303, 0x00);//	; sharpen MT off2
	OV5645_write_cmos_sensor(0x5304, 0x08);//	; DNS th1
	OV5645_write_cmos_sensor(0x5305, 0x30);//	; DNS th2
	OV5645_write_cmos_sensor(0x5306, 0x10);//	; DNS off1 //0x08 0x10
	OV5645_write_cmos_sensor(0x5307, 0x16);//	; DNS off2 //0x16 0x20
	OV5645_write_cmos_sensor(0x5309, 0x08);//	; sharpen TH th1
	OV5645_write_cmos_sensor(0x530a, 0x30);//	; sharpen TH th2
	OV5645_write_cmos_sensor(0x530b, 0x04);//	; sharpen TH th2
	OV5645_write_cmos_sensor(0x530c, 0x06);//	; sharpen TH off2

	OV5645_write_cmos_sensor(0x5480, 0x01);//	; bias on
	OV5645_write_cmos_sensor(0x5481, 0x08);//	; Y yst 00
	OV5645_write_cmos_sensor(0x5482, 0x14);//
	OV5645_write_cmos_sensor(0x5483, 0x28);//
	OV5645_write_cmos_sensor(0x5484, 0x51);//
	OV5645_write_cmos_sensor(0x5485, 0x65);//
	OV5645_write_cmos_sensor(0x5486, 0x71);//
	OV5645_write_cmos_sensor(0x5487, 0x7d);//
	OV5645_write_cmos_sensor(0x5488, 0x87);//
	OV5645_write_cmos_sensor(0x5489, 0x91);//
	OV5645_write_cmos_sensor(0x548a, 0x9a);//
	OV5645_write_cmos_sensor(0x548b, 0xaa);//
	OV5645_write_cmos_sensor(0x548c, 0xb8);//
	OV5645_write_cmos_sensor(0x548d, 0xcd);//
	OV5645_write_cmos_sensor(0x548e, 0xdd);//
	OV5645_write_cmos_sensor(0x548f, 0xea);//
	OV5645_write_cmos_sensor(0x5490, 0x1d);//

        //DPC laimingji 20130815
	OV5645_write_cmos_sensor(0x5780, 0xfc);//
	OV5645_write_cmos_sensor(0x5781, 0x13);//
	OV5645_write_cmos_sensor(0x5782, 0x03);//
	OV5645_write_cmos_sensor(0x5786, 0x20);//
	OV5645_write_cmos_sensor(0x5787, 0x40);//
	OV5645_write_cmos_sensor(0x5788, 0x08);//
	OV5645_write_cmos_sensor(0x5789, 0x08);//
	OV5645_write_cmos_sensor(0x578a, 0x02);//
	OV5645_write_cmos_sensor(0x578b, 0x01);//
	OV5645_write_cmos_sensor(0x578c, 0x01);//
	OV5645_write_cmos_sensor(0x578d, 0x0c);//
	OV5645_write_cmos_sensor(0x578e, 0x02);//
	OV5645_write_cmos_sensor(0x578f, 0x01);//
	OV5645_write_cmos_sensor(0x5790, 0x01);//
	OV5645_write_cmos_sensor(0x5580, 0x06);//uv
	OV5645_write_cmos_sensor(0x5588, 0x01);//
	OV5645_write_cmos_sensor(0x5583, 0x40);//
	OV5645_write_cmos_sensor(0x5584, 0x20);//
	OV5645_write_cmos_sensor(0x5589, 0x18);//
	OV5645_write_cmos_sensor(0x558a, 0x00);//
	OV5645_write_cmos_sensor(0x558b, 0x3c);//

	//From OV Kobe DNP.
	OV5645_write_cmos_sensor(0x5800, 0x1C);//lsc
	OV5645_write_cmos_sensor(0x5801, 0x17);//
	OV5645_write_cmos_sensor(0x5802, 0x12);//
	OV5645_write_cmos_sensor(0x5803, 0x12);//
	OV5645_write_cmos_sensor(0x5804, 0x16);//
	OV5645_write_cmos_sensor(0x5805, 0x18);//
	OV5645_write_cmos_sensor(0x5806, 0x11);//
	OV5645_write_cmos_sensor(0x5807, 0x0A);//
	OV5645_write_cmos_sensor(0x5808, 0x06);//
	OV5645_write_cmos_sensor(0x5809, 0x06);//
	OV5645_write_cmos_sensor(0x580a, 0x08);//
	OV5645_write_cmos_sensor(0x580b, 0x0F);//
	OV5645_write_cmos_sensor(0x580c, 0x0A);//
	OV5645_write_cmos_sensor(0x580d, 0x05);//
	OV5645_write_cmos_sensor(0x580e, 0x00);//
	OV5645_write_cmos_sensor(0x580f, 0x00);//
	OV5645_write_cmos_sensor(0x5810, 0x03);//
	OV5645_write_cmos_sensor(0x5811, 0x0A);//
	OV5645_write_cmos_sensor(0x5812, 0x0E);//
	OV5645_write_cmos_sensor(0x5813, 0x05);//
	OV5645_write_cmos_sensor(0x5814, 0x01);//
	OV5645_write_cmos_sensor(0x5815, 0x00);//
	OV5645_write_cmos_sensor(0x5816, 0x04);//
	OV5645_write_cmos_sensor(0x5817, 0x0A);//
	OV5645_write_cmos_sensor(0x5818, 0x0C);//
	OV5645_write_cmos_sensor(0x5819, 0x0D);//
	OV5645_write_cmos_sensor(0x581a, 0x08);//
	OV5645_write_cmos_sensor(0x581b, 0x08);//
	OV5645_write_cmos_sensor(0x581c, 0x0B);//
	OV5645_write_cmos_sensor(0x581d, 0x12);//
	OV5645_write_cmos_sensor(0x581e, 0x3E);//
	OV5645_write_cmos_sensor(0x581f, 0x11);//
	OV5645_write_cmos_sensor(0x5820, 0x15);//
	OV5645_write_cmos_sensor(0x5821, 0x12);//
	OV5645_write_cmos_sensor(0x5822, 0x16);//
	OV5645_write_cmos_sensor(0x5823, 0x19);//
	OV5645_write_cmos_sensor(0x5824, 0x46);//
	OV5645_write_cmos_sensor(0x5825, 0x28);//
	OV5645_write_cmos_sensor(0x5826, 0x0A);//
	OV5645_write_cmos_sensor(0x5827, 0x26);//
	OV5645_write_cmos_sensor(0x5828, 0x2A);//
	OV5645_write_cmos_sensor(0x5829, 0x2A);//
	OV5645_write_cmos_sensor(0x582a, 0x26);//
	OV5645_write_cmos_sensor(0x582b, 0x24);//
	OV5645_write_cmos_sensor(0x582c, 0x26);//
	OV5645_write_cmos_sensor(0x582d, 0x26);//
	OV5645_write_cmos_sensor(0x582e, 0x0C);//
	OV5645_write_cmos_sensor(0x582f, 0x22);//
	OV5645_write_cmos_sensor(0x5830, 0x40);//
	OV5645_write_cmos_sensor(0x5831, 0x22);//
	OV5645_write_cmos_sensor(0x5832, 0x08);//
	OV5645_write_cmos_sensor(0x5833, 0x2A);//
	OV5645_write_cmos_sensor(0x5834, 0x28);//
	OV5645_write_cmos_sensor(0x5835, 0x06);//
	OV5645_write_cmos_sensor(0x5836, 0x26);//
	OV5645_write_cmos_sensor(0x5837, 0x26);//
	OV5645_write_cmos_sensor(0x5838, 0x26);//
	OV5645_write_cmos_sensor(0x5839, 0x28);//
	OV5645_write_cmos_sensor(0x583a, 0x2A);//
	OV5645_write_cmos_sensor(0x583b, 0x28);//
	OV5645_write_cmos_sensor(0x583c, 0x44);//
	OV5645_write_cmos_sensor(0x583d, 0xCE);//

	OV5645_write_cmos_sensor(0x583e, 0x10);//
	OV5645_write_cmos_sensor(0x583f, 0x08);//
	OV5645_write_cmos_sensor(0x5840, 0x00);//
	OV5645_write_cmos_sensor(0x5025, 0x00);//
	OV5645_write_cmos_sensor(0x3a00, 0x38);//	; ae mode	
	OV5645_write_cmos_sensor(0x3a0f, 0x29);//	; AEC in H
	OV5645_write_cmos_sensor(0x3a10, 0x23);//	; AEC in L
	OV5645_write_cmos_sensor(0x3a1b, 0x29);//	; AEC out H
	OV5645_write_cmos_sensor(0x3a1e, 0x23);//	; AEC out L
	OV5645_write_cmos_sensor(0x3a11, 0x53);//	; control zone H
	OV5645_write_cmos_sensor(0x3a1f, 0x12);//	; control zone L	
	OV5645_write_cmos_sensor(0x501d, 0x00);//	; control zone L
}

/*****************************************************************
* FUNCTION
*    OV5645PreviewSetting
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
void OV5645PreviewSetting_SVGA(void)
{
    //;OV5645 1280x960,30fps
    //56Mhz, 224Mbps/Lane, 2Lane.
    OV5645SENSORDB("[OV5645]enter OV5645PreviewSetting_SVGA function:\n ");
    OV5645_write_cmos_sensor(0x4202, 0x0f);//    ; stop mipi stream
    OV5645_write_cmos_sensor(0x300e, 0x45);//    ; MIPI 2 lane
    OV5645_write_cmos_sensor(0x3034, 0x18);// PLL, MIPI 8-bit mode
    OV5645_write_cmos_sensor(0x3035, 0x21);// PLL
    OV5645_write_cmos_sensor(0x3036, 0x70);// PLL
    OV5645_write_cmos_sensor(0x3037, 0x13);// PLL
    OV5645_write_cmos_sensor(0x3108, 0x01);// PLL
    OV5645_write_cmos_sensor(0x3824, 0x01);// PLL
    OV5645_write_cmos_sensor(0x460c, 0x20);// PLL
    OV5645_write_cmos_sensor(0x3618, 0x00);//
    OV5645_write_cmos_sensor(0x3600, 0x09);//
    OV5645_write_cmos_sensor(0x3601, 0x43);//
    OV5645_write_cmos_sensor(0x3708, 0x66);//
    OV5645_write_cmos_sensor(0x3709, 0x12);//
    OV5645_write_cmos_sensor(0x370c, 0xc3);//
    OV5645_write_cmos_sensor(0x3800, 0x00); // HS = 0
    OV5645_write_cmos_sensor(0x3801, 0x00); // HS
    OV5645_write_cmos_sensor(0x3802, 0x00); // VS = 250
    OV5645_write_cmos_sensor(0x3803, 0x06); // VS
    OV5645_write_cmos_sensor(0x3804, 0x0a); // HW = 2623
    OV5645_write_cmos_sensor(0x3805, 0x3f);//    ; HW
    OV5645_write_cmos_sensor(0x3806, 0x07);//    ; VH =
    OV5645_write_cmos_sensor(0x3807, 0x9d);//    ; VH
    OV5645_write_cmos_sensor(0x3808, 0x05);//    ; DVPHO = 1280
    OV5645_write_cmos_sensor(0x3809, 0x00);//    ; DVPHO
    OV5645_write_cmos_sensor(0x380a, 0x03);//    ; DVPVO = 960
    OV5645_write_cmos_sensor(0x380b, 0xc0);//    ; DVPVO
    OV5645_write_cmos_sensor(0x380c, 0x07);//    ; HTS = 2160
    OV5645_write_cmos_sensor(0x380d, 0x68);//    ; HTS
    OV5645_write_cmos_sensor(0x380e, 0x03);//    ; VTS = 740
    OV5645_write_cmos_sensor(0x380f, 0xd8);//    ; VTS
    OV5645_write_cmos_sensor(0x3810, 0x00); // H OFF = 16
    OV5645_write_cmos_sensor(0x3811, 0x10); // H OFF
    OV5645_write_cmos_sensor(0x3812, 0x00); // V OFF = 4
    OV5645_write_cmos_sensor(0x3813, 0x06);//    ; V OFF
    OV5645_write_cmos_sensor(0x3814, 0x31);//    ; X INC
    OV5645_write_cmos_sensor(0x3815, 0x31);//    ; Y INC
    OV5645_write_cmos_sensor(0x3820, 0x41);//    ; flip off, V bin on
    OV5645_write_cmos_sensor(0x3821, 0x01);//    ; mirror on, H bin on
    OV5645_write_cmos_sensor(0x4514, 0x00);
    OV5645_write_cmos_sensor(0x3a02, 0x03);//    ; max exp 60 = 740
    OV5645_write_cmos_sensor(0x3a03, 0xd8);//    ; max exp 60
    OV5645_write_cmos_sensor(0x3a08, 0x01);//    ; B50 = 222
    OV5645_write_cmos_sensor(0x3a09, 0x27);//    ; B50
    OV5645_write_cmos_sensor(0x3a0a, 0x00);//    ; B60 = 185
    OV5645_write_cmos_sensor(0x3a0b, 0xf6);//    ; B60
    OV5645_write_cmos_sensor(0x3a0e, 0x03);//    ; max 50
    OV5645_write_cmos_sensor(0x3a0d, 0x04);//    ; max 60
    OV5645_write_cmos_sensor(0x3a14, 0x03);//    ; max exp 50 = 740
    OV5645_write_cmos_sensor(0x3a15, 0xd8);//    ; max exp 50
    OV5645_write_cmos_sensor(0x3c07, 0x07);//    ; 50/60 auto detect
    OV5645_write_cmos_sensor(0x3c08, 0x01);//    ; 50/60 auto detect
    OV5645_write_cmos_sensor(0x3c09, 0xc2);//    ; 50/60 auto detect
    OV5645_write_cmos_sensor(0x4004, 0x02);//    ; BLC line number
    OV5645_write_cmos_sensor(0x4005, 0x18);//    ; BLC triggered by gain change
    OV5645_write_cmos_sensor(0x4837, 0x11); // MIPI global timing 16
    OV5645_write_cmos_sensor(0x503d, 0x00);//
    OV5645_write_cmos_sensor(0x5000, 0xa7);//
    OV5645_write_cmos_sensor(0x5001, 0xa3);//
    OV5645_write_cmos_sensor(0x5002, 0x80);//
    OV5645_write_cmos_sensor(0x5003, 0x08);//
    OV5645_write_cmos_sensor(0x3032, 0x00);//
    OV5645_write_cmos_sensor(0x4000, 0x89);//
    OV5645_write_cmos_sensor(0x3a00, 0x38);//    ; ae mode
    if(run_test_potten)
    {
        run_test_potten=0;
        OV5645PIPSetTestPatternMode(1);
    }
    OV5645_write_cmos_sensor(0x4202, 0x00);//    ; open mipi stream
//    OV5645WriteExtraShutter(OV5645MIPISensor.PreviewExtraShutter);
	spin_lock(&ov5645_drv_lock);
    OV5645MIPISensor.SensorMode= SENSOR_MODE_PREVIEW;
    OV5645MIPISensor.IsPVmode = KAL_TRUE;
    OV5645MIPISensor.PreviewPclk= 560;
	spin_unlock(&ov5645_drv_lock);
    OV5645SENSORDB("[OV5645]exit OV5645PreviewSetting_SVGA function:\n ");
}
/*****************************************************************
* FUNCTION
*    OV5645PreviewSetting
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
//static
void OV5645PreviewSetting_720p(void)
{
	//for PIP QVGA preview
	OV5645_write_cmos_sensor(0x300e, 0x45);//	; MIPI 2 lane
	OV5645_write_cmos_sensor(0x3034, 0x18);//PLL, MIPI 8-bit mode
	OV5645_write_cmos_sensor(0x3035, 0x21);//PLL
	OV5645_write_cmos_sensor(0x3036, 0x60);//PLL
	OV5645_write_cmos_sensor(0x3037, 0x13);//PLL
	OV5645_write_cmos_sensor(0x3108, 0x01);//PLL
	OV5645_write_cmos_sensor(0x3824, 0x01);//PLL
	OV5645_write_cmos_sensor(0x460c, 0x22);//PLL
	OV5645_write_cmos_sensor(0x3618, 0x00);//
	OV5645_write_cmos_sensor(0x3600, 0x09);//
	OV5645_write_cmos_sensor(0x3601, 0x43);//
	OV5645_write_cmos_sensor(0x3708, 0x66);//
	OV5645_write_cmos_sensor(0x3709, 0x12);//
	OV5645_write_cmos_sensor(0x370c, 0xc3);//
	OV5645_write_cmos_sensor(0x3801, 0x10);//0x00	; HS = 0
	OV5645_write_cmos_sensor(0x3803, 0xfa);//	; VS = 250
	OV5645_write_cmos_sensor(0x3805, 0x2f);//0x3f	; HW
	OV5645_write_cmos_sensor(0x3806, 0x06);//	; VH = 1705
	OV5645_write_cmos_sensor(0x3807, 0xa9);//	; VH
	OV5645_write_cmos_sensor(0x3808, 0x05);//	; DVPHO = 1280
	OV5645_write_cmos_sensor(0x3809, 0x00);//	; DVPHO
	OV5645_write_cmos_sensor(0x380a, 0x02);//	; DVPVO = 720
	OV5645_write_cmos_sensor(0x380b, 0xd0);//	; DVPVO
	OV5645_write_cmos_sensor(0x380c, 0x08);//	; HTS = 2160
	OV5645_write_cmos_sensor(0x380d, 0x70);//	; HTS
#if defined (OV5645_15FPS)
    OV5645_write_cmos_sensor(0x380e, 0x05);//    ; VTS = 740
    OV5645_write_cmos_sensor(0x380f, 0xc8);//    ; VTS
#else
	OV5645_write_cmos_sensor(0x380e, 0x02);//	; VTS = 740
	OV5645_write_cmos_sensor(0x380f, 0xe4);//	; VTS
#endif
	OV5645_write_cmos_sensor(0x3811, 0x08);//0x10	; H OFF
	OV5645_write_cmos_sensor(0x3813, 0x04);//	; V OFF
	OV5645_write_cmos_sensor(0x3814, 0x31);//	; X INC
	OV5645_write_cmos_sensor(0x3815, 0x31);//	; Y INC
	OV5645_write_cmos_sensor(0x3820, 0x41);//	; flip off, V bin on
	OV5645_write_cmos_sensor(0x3821, 0x01);//	; mirror on, H bin on
#if defined (OV5645_15FPS)
    OV5645_write_cmos_sensor(0x3a02, 0x05);//    ; max exp 60 = 740
    OV5645_write_cmos_sensor(0x3a03, 0xc8);//    ; max exp 60
    OV5645_write_cmos_sensor(0x3a14, 0x05);//    ; max exp 50 = 740
    OV5645_write_cmos_sensor(0x3a15, 0xc8);//    ; max exp 50
    OV5645_write_cmos_sensor(0x3a08, 0x00);//    ; B50 = 222
    OV5645_write_cmos_sensor(0x3a09, 0xde);//    ; B50
    OV5645_write_cmos_sensor(0x3a0a, 0x00);//    ; B60 = 185
    OV5645_write_cmos_sensor(0x3a0b, 0xb9);//    ; B60
    OV5645_write_cmos_sensor(0x3a0e, 0x06);//    ; max 50
    OV5645_write_cmos_sensor(0x3a0d, 0x08);//    ; max 60
#else
	OV5645_write_cmos_sensor(0x3a02, 0x02);//	; max exp 60 = 740
	OV5645_write_cmos_sensor(0x3a03, 0xe4);//	; max exp 60
	OV5645_write_cmos_sensor(0x3a14, 0x02);//	; max exp 50 = 740
	OV5645_write_cmos_sensor(0x3a15, 0xe4);//	; max exp 50
	OV5645_write_cmos_sensor(0x3a08, 0x00);//	; B50 = 222
	OV5645_write_cmos_sensor(0x3a09, 0xde);//	; B50
	OV5645_write_cmos_sensor(0x3a0a, 0x00);//	; B60 = 185
	OV5645_write_cmos_sensor(0x3a0b, 0xb9);//	; B60
	OV5645_write_cmos_sensor(0x3a0e, 0x03);//	; max 50
	OV5645_write_cmos_sensor(0x3a0d, 0x04);//	; max 60
#endif
	OV5645_write_cmos_sensor(0x3c07, 0x08);//	; 50/60 auto detect
	OV5645_write_cmos_sensor(0x3c08, 0x00);//	; 50/60 auto detect
	OV5645_write_cmos_sensor(0x3c09, 0x1c);//	; 50/60 auto detect
	OV5645_write_cmos_sensor(0x4004, 0x02);//	; BLC line number
	OV5645_write_cmos_sensor(0x4005, 0x18);//	; BLC triggered by gain change
	OV5645_write_cmos_sensor(0x4837, 0x14);//	; MIPI global timing
	OV5645_write_cmos_sensor(0x4b00, 0x45);//	; SPI receiver control
	OV5645_write_cmos_sensor(0x4b02, 0x01);//	; line start manual = 320
	OV5645_write_cmos_sensor(0x4b03, 0x40);//	; line start manual
	OV5645_write_cmos_sensor(0x4310, 0x01);//	; Vsync width
	OV5645_write_cmos_sensor(0x4314, 0x0f);//	; Vsync delay
	OV5645_write_cmos_sensor(0x4315, 0x95);//	; Vsync delay
	OV5645_write_cmos_sensor(0x4316, 0x9f);//	; Vsync delay
	OV5645_write_cmos_sensor(0x5b00, 0x01);//
	OV5645_write_cmos_sensor(0x5b01, 0x40);//
	OV5645_write_cmos_sensor(0x5b02, 0x00);//
	OV5645_write_cmos_sensor(0x5b03, 0xf0);//
	OV5645_write_cmos_sensor(0x5b07, 0x04);//
	OV5645_write_cmos_sensor(0x5b08, 0x04);//
	OV5645_write_cmos_sensor(0x503d, 0x00);//
	OV5645_write_cmos_sensor(0x5000, 0xa7);//
	OV5645_write_cmos_sensor(0x5001, 0xa3);//
	OV5645_write_cmos_sensor(0x5002, 0x81);//
	OV5645_write_cmos_sensor(0x5003, 0x08);//
    OV5645_write_cmos_sensor(0x5302, 0x20); //sharpness
    OV5645_write_cmos_sensor(0x5303, 0x10); //sharpness
	OV5645_write_cmos_sensor(0x3032, 0x00);//
	OV5645_write_cmos_sensor(0x4000, 0x89);//
    //OV5645_write_cmos_sensor(0x3000, 0x30);//
    OV5645_write_cmos_sensor(0x3503, 0x00);//AEC
		
//    OV5645WriteExtraShutter(OV5645MIPISensor.PreviewExtraShutter);
	spin_lock(&ov5645_drv_lock);
    OV5645MIPISensor.IsPVmode = KAL_TRUE;
    OV5645MIPISensor.PreviewPclk= 480;
    OV5645MIPISensor.SensorMode= SENSOR_MODE_PREVIEW;
    spin_unlock(&ov5645_drv_lock);
}
/*****************************************************************
* FUNCTION
*    OV5645VideoSetting
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
//static
void OV5645VideoSetting_720p(void)
{
    //for PIP QVGA preview
    OV5645_write_cmos_sensor(0x300e, 0x45);//    ; MIPI 2 lane
    OV5645_write_cmos_sensor(0x3034, 0x18);//PLL, MIPI 8-bit mode
    OV5645_write_cmos_sensor(0x3035, 0x21);//PLL
    OV5645_write_cmos_sensor(0x3036, 0x60);//PLL
    OV5645_write_cmos_sensor(0x3037, 0x13);//PLL
    OV5645_write_cmos_sensor(0x3108, 0x01);//PLL
    OV5645_write_cmos_sensor(0x3824, 0x01);//PLL
    OV5645_write_cmos_sensor(0x460c, 0x22);//PLL
    OV5645_write_cmos_sensor(0x3618, 0x00);//
    OV5645_write_cmos_sensor(0x3600, 0x09);//
    OV5645_write_cmos_sensor(0x3601, 0x43);//
    OV5645_write_cmos_sensor(0x3708, 0x66);//
    OV5645_write_cmos_sensor(0x3709, 0x12);//
    OV5645_write_cmos_sensor(0x370c, 0xc3);//
    OV5645_write_cmos_sensor(0x3801, 0x00);//    ; HS = 0
    OV5645_write_cmos_sensor(0x3803, 0xfa);//    ; VS = 250
    OV5645_write_cmos_sensor(0x3805, 0x3f);//    ; HW
    OV5645_write_cmos_sensor(0x3806, 0x06);//    ; VH = 1705
    OV5645_write_cmos_sensor(0x3807, 0xa9);//    ; VH
    OV5645_write_cmos_sensor(0x3808, 0x05);//    ; DVPHO = 1280
    OV5645_write_cmos_sensor(0x3809, 0x00);//    ; DVPHO
    OV5645_write_cmos_sensor(0x380a, 0x02);//    ; DVPVO = 720
    OV5645_write_cmos_sensor(0x380b, 0xd0);//    ; DVPVO
    OV5645_write_cmos_sensor(0x380c, 0x08);//    ; HTS = 2160
    OV5645_write_cmos_sensor(0x380d, 0x70);//    ; HTS
#if defined (OV5645_15FPS)
    OV5645_write_cmos_sensor(0x380e, 0x05);//    ; VTS = 740
    OV5645_write_cmos_sensor(0x380f, 0xc8);//    ; VTS
#else
    OV5645_write_cmos_sensor(0x380e, 0x02);//    ; VTS = 740
    OV5645_write_cmos_sensor(0x380f, 0xe4);//    ; VTS
#endif
    OV5645_write_cmos_sensor(0x3811, 0x10);//    ; H OFF
    OV5645_write_cmos_sensor(0x3813, 0x04);//    ; V OFF
    OV5645_write_cmos_sensor(0x3814, 0x31);//    ; X INC
    OV5645_write_cmos_sensor(0x3815, 0x31);//    ; Y INC
    OV5645_write_cmos_sensor(0x3820, 0x41);//    ; flip off, V bin on
    OV5645_write_cmos_sensor(0x3821, 0x01);//    ; mirror on, H bin on
#if defined (OV5645_15FPS)
    OV5645_write_cmos_sensor(0x3a02, 0x05);//    ; max exp 60 = 740
    OV5645_write_cmos_sensor(0x3a03, 0xc8);//    ; max exp 60
    OV5645_write_cmos_sensor(0x3a14, 0x05);//    ; max exp 50 = 740
    OV5645_write_cmos_sensor(0x3a15, 0xc8);//    ; max exp 50
    OV5645_write_cmos_sensor(0x3a08, 0x00);//    ; B50 = 222
    OV5645_write_cmos_sensor(0x3a09, 0xde);//    ; B50
    OV5645_write_cmos_sensor(0x3a0a, 0x00);//    ; B60 = 185
    OV5645_write_cmos_sensor(0x3a0b, 0xb9);//    ; B60
    OV5645_write_cmos_sensor(0x3a0e, 0x06);//    ; max 50
    OV5645_write_cmos_sensor(0x3a0d, 0x08);//    ; max 60
#else
    OV5645_write_cmos_sensor(0x3a02, 0x02);//    ; max exp 60 = 740
    OV5645_write_cmos_sensor(0x3a03, 0xe4);//    ; max exp 60
    OV5645_write_cmos_sensor(0x3a14, 0x02);//    ; max exp 50 = 740
    OV5645_write_cmos_sensor(0x3a15, 0xe4);//    ; max exp 50
    OV5645_write_cmos_sensor(0x3a08, 0x00);//    ; B50 = 222
    OV5645_write_cmos_sensor(0x3a09, 0xde);//    ; B50
    OV5645_write_cmos_sensor(0x3a0a, 0x00);//    ; B60 = 185
    OV5645_write_cmos_sensor(0x3a0b, 0xb9);//    ; B60
    OV5645_write_cmos_sensor(0x3a0e, 0x03);//    ; max 50
    OV5645_write_cmos_sensor(0x3a0d, 0x04);//    ; max 60
#endif
    OV5645_write_cmos_sensor(0x3c07, 0x08);//    ; 50/60 auto detect
    OV5645_write_cmos_sensor(0x3c08, 0x00);//    ; 50/60 auto detect
    OV5645_write_cmos_sensor(0x3c09, 0x1c);//    ; 50/60 auto detect
    OV5645_write_cmos_sensor(0x4004, 0x02);//    ; BLC line number
    OV5645_write_cmos_sensor(0x4005, 0x18);//    ; BLC triggered by gain change
    OV5645_write_cmos_sensor(0x4837, 0x14);//    ; MIPI global timing
    OV5645_write_cmos_sensor(0x4b00, 0x45);//    ; SPI receiver control
    OV5645_write_cmos_sensor(0x4b02, 0x01);//    ; line start manual = 320
    OV5645_write_cmos_sensor(0x4b03, 0x40);//    ; line start manual
    OV5645_write_cmos_sensor(0x4310, 0x01);//    ; Vsync width
    OV5645_write_cmos_sensor(0x4314, 0x0f);//    ; Vsync delay
    OV5645_write_cmos_sensor(0x4315, 0x95);//    ; Vsync delay
    OV5645_write_cmos_sensor(0x4316, 0x9f);//    ; Vsync delay
    OV5645_write_cmos_sensor(0x5b00, 0x01);//
    OV5645_write_cmos_sensor(0x5b01, 0x40);//
    OV5645_write_cmos_sensor(0x5b02, 0x00);//
    OV5645_write_cmos_sensor(0x5b03, 0xf0);//
    OV5645_write_cmos_sensor(0x5b07, 0x04);//
    OV5645_write_cmos_sensor(0x5b08, 0x04);//
    OV5645_write_cmos_sensor(0x503d, 0x00);//
    OV5645_write_cmos_sensor(0x5000, 0xa7);//
    OV5645_write_cmos_sensor(0x5001, 0xa3);//
    OV5645_write_cmos_sensor(0x5002, 0x81);//
    OV5645_write_cmos_sensor(0x5003, 0x08);//
    OV5645_write_cmos_sensor(0x3032, 0x00);//
    OV5645_write_cmos_sensor(0x4000, 0x89);//
    //OV5645_write_cmos_sensor(0x3000, 0x30);//
    OV5645_write_cmos_sensor(0x3503, 0x00);//AEC

    spin_lock(&ov5645_drv_lock);
    OV5645MIPISensor.IsPVmode = KAL_TRUE;
    OV5645MIPISensor.PreviewPclk= 480;
    OV5645MIPISensor.SensorMode= SENSOR_MODE_PREVIEW;
	spin_unlock(&ov5645_drv_lock);
}

static void OV5645MIPI_Capture_Sharpness(void)
{
/*
    if (OV5645_MID_OFLIM == OV5645MIPISensor.module_id) {
        switch (OV5645MIPISensor.lens_id) {
        case OV5645_LENS_ID_LARGAN9467A8:
            OV5645_write_cmos_sensor(0x5302, 0x30);//    ; sharpen MT
            OV5645_write_cmos_sensor(0x5303, 0x18);//    ; sharpen MT
            break;
        case OV5645_LENS_ID_T1516:
            //OV5645_write_cmos_sensor(0x5302, 0x20);//    ; sharpen MT
            //OV5645_write_cmos_sensor(0x5303, 0x18);//    ; sharpen MT
            break;
        default:
            break;
        }
    } else if (OV5645_MID_A_KERR == OV5645MIPISensor.module_id) {
        switch (OV5645MIPISensor.lens_id) {
        case OV5645_LENS_ID_HOKUANG_GBCA2085201:
            OV5645_write_cmos_sensor(0x5302, 0x30);//    ; sharpen MT
            OV5645_write_cmos_sensor(0x5303, 0x18);//    ; sharpen MT
            break;
        case OV5645_LENS_ID_T1516:
            //OV5645_write_cmos_sensor(0x5302, 0x20);//    ; sharpen MT
            //OV5645_write_cmos_sensor(0x5303, 0x18);//    ; sharpen MT
            break;
        default:
            break;
        }
    } else {
        //OV5645_write_cmos_sensor(0x5302, 0x20);//    ; sharpen MT
        //OV5645_write_cmos_sensor(0x5303, 0x18);//    ; sharpen MT
    }
    */
}
/*************************************************************************
* FUNCTION
*     OV5645FullSizeCaptureSetting
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
//static
void OV5645FullSizeCaptureSetting(OV5645_SENSOR_MODE eSensorMode)
{
	//2560x1440
	OV5645_write_cmos_sensor(0x300e, 0x45);//	; MIPI 2 lane
	OV5645_write_cmos_sensor(0x3034, 0x18);//PLL, MIPI 8-bit mode
	OV5645_write_cmos_sensor(0x3035, 0x21);//PLL
	OV5645_write_cmos_sensor(0x3036, 0x60);//PLL
	OV5645_write_cmos_sensor(0x3037, 0x13);//PLL
	OV5645_write_cmos_sensor(0x3108, 0x01);//PLL
	OV5645_write_cmos_sensor(0x3824, 0x01);//PLL
	OV5645_write_cmos_sensor(0x460c, 0x22);//PLL
	OV5645_write_cmos_sensor(0x3618, 0x04);//
	OV5645_write_cmos_sensor(0x3600, 0x08);//
	OV5645_write_cmos_sensor(0x3601, 0x33);//
	OV5645_write_cmos_sensor(0x3708, 0x63);//
	OV5645_write_cmos_sensor(0x3709, 0x12);//
	OV5645_write_cmos_sensor(0x370c, 0xc0);//
	OV5645_write_cmos_sensor(0x3801, 0x10);//	; HS = 16
	OV5645_write_cmos_sensor(0x3803, 0xfa);//0xfc	; VS = 252
	OV5645_write_cmos_sensor(0x3805, 0x2f);//	; HW
	OV5645_write_cmos_sensor(0x3806, 0x06);//	; VH = 1705
	OV5645_write_cmos_sensor(0x3807, 0xa9);//0xa3	; VH
	OV5645_write_cmos_sensor(0x3808, 0x0a);//	; DVPHO = 2560
	OV5645_write_cmos_sensor(0x3809, 0x00);//	; DVPHO
	OV5645_write_cmos_sensor(0x380a, 0x05);//	; DVPVO = 1440
	OV5645_write_cmos_sensor(0x380b, 0xa0);//	; DVPVO
	OV5645_write_cmos_sensor(0x380c, 0x0b);//	; HTS = 2984
	OV5645_write_cmos_sensor(0x380d, 0xa8);//    ; HTS e8
#if defined (OV5645_15FPS)
	OV5645_write_cmos_sensor(0x380e, 0x0b);//    ; VTS = 1464
	OV5645_write_cmos_sensor(0x380f, 0x70);//    ; VTS
#else
	OV5645_write_cmos_sensor(0x380e, 0x05);//	; VTS = 1464
	OV5645_write_cmos_sensor(0x380f, 0xb8);//	; VTS
#endif
	OV5645_write_cmos_sensor(0x3811, 0x10);//	; H OFF
	OV5645_write_cmos_sensor(0x3813, 0x08);//0x06	; V OFF
	OV5645_write_cmos_sensor(0x3814, 0x11);//	; X INC
	OV5645_write_cmos_sensor(0x3815, 0x11);//	; Y INC
	OV5645_write_cmos_sensor(0x3820, 0x40);//	; flip off, V bin off
	OV5645_write_cmos_sensor(0x3821, 0x00);//	; mirror on, H bin off
#if defined (OV5645_15FPS)
	OV5645_write_cmos_sensor(0x3a02, 0x0b);//    ; max exp 60 = 1968
	OV5645_write_cmos_sensor(0x3a03, 0x70);//    ; max exp 60
	OV5645_write_cmos_sensor(0x3a14, 0x0b);//    ; max exp 50 = 1968
	OV5645_write_cmos_sensor(0x3a15, 0x70);//    ; max exp 50
	OV5645_write_cmos_sensor(0x3a08, 0x00);//    ; B50 = 160
	OV5645_write_cmos_sensor(0x3a09, 0xa0);//    ; B50 9d
	OV5645_write_cmos_sensor(0x3a0a, 0x00);//    ; B60 = 134
	OV5645_write_cmos_sensor(0x3a0b, 0x86);//    ; B60 83
	OV5645_write_cmos_sensor(0x3a0e, 0x12);//    ; max 50
	OV5645_write_cmos_sensor(0x3a0d, 0x16);//    ; max 60 b
#else
	OV5645_write_cmos_sensor(0x3a02, 0x05);//	; max exp 60 = 1968
	OV5645_write_cmos_sensor(0x3a03, 0xb8);//	; max exp 60
	OV5645_write_cmos_sensor(0x3a14, 0x05);//	; max exp 50 = 1968
	OV5645_write_cmos_sensor(0x3a15, 0xb8);//	; max exp 50
	OV5645_write_cmos_sensor(0x3a08, 0x00);//	; B50 = 160
	OV5645_write_cmos_sensor(0x3a09, 0xa0);//    ; B50 9d
	OV5645_write_cmos_sensor(0x3a0a, 0x00);//	; B60 = 134
	OV5645_write_cmos_sensor(0x3a0b, 0x86);//    ; B60 83
	OV5645_write_cmos_sensor(0x3a0e, 0x09);//	; max 50
	OV5645_write_cmos_sensor(0x3a0d, 0x0a);//    ; max 60 b
#endif
	OV5645_write_cmos_sensor(0x3c07, 0x07);//	; 50/60 auto detect
	OV5645_write_cmos_sensor(0x3c08, 0x01);//	; 50/60 auto detect
	OV5645_write_cmos_sensor(0x3c09, 0xc2);//	; 50/60 auto detect
	OV5645_write_cmos_sensor(0x4004, 0x06);//	; BLC line number
	OV5645_write_cmos_sensor(0x4837, 0x14);//	; MIPI global timing
	OV5645_write_cmos_sensor(0x4b00, 0x45);//	; MIPI global timing
	OV5645_write_cmos_sensor(0x4b02, 0x01);//	; line start manual = 320
	OV5645_write_cmos_sensor(0x4b03, 0x40);//	; line start manual
	OV5645_write_cmos_sensor(0x4310, 0x00);//	; Vsync width
	OV5645_write_cmos_sensor(0x4314, 0x2a);//    ; Vsync delay 2b
	OV5645_write_cmos_sensor(0x4315, 0xdd);//    ; Vsync delay 82
	OV5645_write_cmos_sensor(0x4316, 0xb4);//    ; Vsync delay 24
	OV5645_write_cmos_sensor(0x5b00, 0x02);//
	OV5645_write_cmos_sensor(0x5b01, 0x80);//
	OV5645_write_cmos_sensor(0x5b02, 0x01);//
	OV5645_write_cmos_sensor(0x5b03, 0xe0);//
	OV5645_write_cmos_sensor(0x5b07, 0x04);//
	OV5645_write_cmos_sensor(0x5b08, 0x04);//
	OV5645_write_cmos_sensor(0x503d, 0x00);//
	OV5645_write_cmos_sensor(0x5000, 0xa7);//
	OV5645_write_cmos_sensor(0x5001, 0x83);//
	OV5645_write_cmos_sensor(0x5002, 0x81);
	OV5645_write_cmos_sensor(0x5003, 0x08);

    OV5645MIPI_Capture_Sharpness();

	OV5645_write_cmos_sensor(0x3032, 0x00);
	OV5645_write_cmos_sensor(0x4000, 0x89);
//	OV5645_write_cmos_sensor(0x3000, 0x30);
	spin_lock(&ov5645_drv_lock);
	OV5645MIPISensor.IsPVmode = KAL_FALSE;
	OV5645MIPISensor.CapturePclk= 480;
	OV5645MIPISensor.SensorMode= eSensorMode;
	spin_unlock(&ov5645_drv_lock);
}

UINT32 OV5645read_vcm_adc(void)
{
    kal_uint16 high, low, adcvalue;
    high= OV5645_read_cmos_sensor(0x3603);
    low= OV5645_read_cmos_sensor(0x3602);
    adcvalue= ((high&0x3f)<<4) + ((low&0xf0)>>4);
    return adcvalue;
}

UINT32 OV5645set_vcm_adc(adcvalue)
{
    kal_uint16 high, low;
    high = adcvalue>>4;
    low = (adcvalue&0x0f)<<4;
    OV5645_write_cmos_sensor(0x3603,(OV5645_read_cmos_sensor(0x3603)&0xC0)|high);
    OV5645_write_cmos_sensor(0x3602,(OV5645_read_cmos_sensor(0x3602)&0x0F)|low);
    return ERROR_NONE;
}

/*************************************************************************
* FUNCTION
*    OV5645SetHVMirror
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
 void OV5645SetHVMirror(kal_uint8 Mirror,kal_uint8 Mode)
{
  	kal_uint8 mirror= 0, flip=0;
		flip = OV5645_read_cmos_sensor(0x3820);
		mirror=OV5645_read_cmos_sensor(0x3821);
	
    if (Mode==SENSOR_MODE_PREVIEW)
	{
		switch (Mirror)
		{
		case IMAGE_NORMAL:
			OV5645_write_cmos_sensor(0x3820, flip&0xf9);     
			OV5645_write_cmos_sensor(0x3821, mirror&0xf9);
			OV5645_write_cmos_sensor(0x4514, 0x00);
			break;
		case IMAGE_H_MIRROR:
			OV5645_write_cmos_sensor(0x3820, flip&0xf9);     
			OV5645_write_cmos_sensor(0x3821, mirror|0x06);
			OV5645_write_cmos_sensor(0x4514, 0x00);
			break;
		case IMAGE_V_MIRROR: 
			OV5645_write_cmos_sensor(0x3820, flip|0x06);     
			OV5645_write_cmos_sensor(0x3821, mirror&0xf9);
			OV5645_write_cmos_sensor(0x4514, 0x00);
			break;		
		case IMAGE_HV_MIRROR:
			OV5645_write_cmos_sensor(0x3820, flip|0x06);     
			OV5645_write_cmos_sensor(0x3821, mirror|0x06);
			OV5645_write_cmos_sensor(0x4514, 0x00);
			break; 		
		default:
			ASSERT(0);
		}
	}
    else if (Mode== SENSOR_MODE_CAPTURE)
	{
		switch (Mirror)
		{
		case IMAGE_NORMAL:
			OV5645_write_cmos_sensor(0x3820, flip&0xf9);     
			OV5645_write_cmos_sensor(0x3821, mirror&0xf9);
			OV5645_write_cmos_sensor(0x4514, 0x00);
			break;
		case IMAGE_H_MIRROR:
			OV5645_write_cmos_sensor(0x3820, flip&0xf9);     
			OV5645_write_cmos_sensor(0x3821, mirror|0x06);
			OV5645_write_cmos_sensor(0x4514, 0x00);
			break;
		case IMAGE_V_MIRROR: 
			OV5645_write_cmos_sensor(0x3820, flip|0x06);     
			OV5645_write_cmos_sensor(0x3821, mirror&0xf9);
			OV5645_write_cmos_sensor(0x4514, 0x88);
			break;		
		case IMAGE_HV_MIRROR:
			OV5645_write_cmos_sensor(0x3820, flip|0x06);     
			OV5645_write_cmos_sensor(0x3821, mirror|0x06);
			OV5645_write_cmos_sensor(0x4514, 0xbb);
			break; 		
		default:
			ASSERT(0);
	}
	}
}

void update_PIP_position(int position, int Cam_Mode) 
{
    if(Cam_Mode==SENSOR_MODE_PREVIEW) {
		 // preview
		 switch(position) {
			 case LL:
					OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3002, 0x3c);
					OV5645_write_cmos_sensor(0x3003, 0x01);
					OV5645_write_cmos_sensor(0x3002, 0x1c);
					OV5645_write_cmos_sensor(0x4314, 0x0f);
					OV5645_write_cmos_sensor(0x4315, 0x95);
					OV5645_write_cmos_sensor(0x4316, 0x9f);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
					mDELAY(100);  
					OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3003, 0x00);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
				   break;

			 case LR:
				  OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3002, 0x3c);
					OV5645_write_cmos_sensor(0x3003, 0x01);
					OV5645_write_cmos_sensor(0x3002, 0x1c);
					OV5645_write_cmos_sensor(0x4314, 0x0f);
					OV5645_write_cmos_sensor(0x4315, 0xa1);
					OV5645_write_cmos_sensor(0x4316, 0xcc);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
					mDELAY(100);  
					OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3003, 0x00);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
				   break;

			 case UL:
			 		OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3002, 0x3c);
					OV5645_write_cmos_sensor(0x3003, 0x01);
					OV5645_write_cmos_sensor(0x3002, 0x1c);
					OV5645_write_cmos_sensor(0x4314, 0x00);
					OV5645_write_cmos_sensor(0x4315, 0x07);
					OV5645_write_cmos_sensor(0x4316, 0x24);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
					mDELAY(100);  
					OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3003, 0x00);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
				   break;

			 case UR:
			 		OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3002, 0x3c);
					OV5645_write_cmos_sensor(0x3003, 0x01);
					OV5645_write_cmos_sensor(0x3002, 0x1c);
					OV5645_write_cmos_sensor(0x4314, 0x00);
					OV5645_write_cmos_sensor(0x4315, 0x02);
					OV5645_write_cmos_sensor(0x4316, 0x68);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
					mDELAY(100);  
					OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3003, 0x00);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
				   break;

			 default:
				   break;
		 }
	 }
    else if (Cam_Mode==SENSOR_MODE_VIDEO) {
		 // video
		 switch(position) {
			 case LL:
					OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3002, 0x3c);
					OV5645_write_cmos_sensor(0x3003, 0x01);
					OV5645_write_cmos_sensor(0x3002, 0x1c);
					OV5645_write_cmos_sensor(0x4314, 0x0f);
					OV5645_write_cmos_sensor(0x4315, 0x96);
					OV5645_write_cmos_sensor(0x4316, 0x34);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
					mDELAY(100);  
					OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3003, 0x00);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
				   break;

			 case LR:
				  OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3002, 0x3c);
					OV5645_write_cmos_sensor(0x3003, 0x01);
					OV5645_write_cmos_sensor(0x3002, 0x1c);
					OV5645_write_cmos_sensor(0x4314, 0x0f);
					OV5645_write_cmos_sensor(0x4315, 0xa1);
					OV5645_write_cmos_sensor(0x4316, 0xcc);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
					mDELAY(100);  
					OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3003, 0x00);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
				   break;

			 case UL:
			 		OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3002, 0x3c);
					OV5645_write_cmos_sensor(0x3003, 0x01);
					OV5645_write_cmos_sensor(0x3002, 0x1c);
					OV5645_write_cmos_sensor(0x4314, 0x00);
					OV5645_write_cmos_sensor(0x4315, 0x07);
					OV5645_write_cmos_sensor(0x4316, 0x24);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
					mDELAY(100);  
					OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3003, 0x00);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
				   break;

			 case UR:
			 		OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3002, 0x3c);
					OV5645_write_cmos_sensor(0x3003, 0x01);
					OV5645_write_cmos_sensor(0x3002, 0x1c);
					OV5645_write_cmos_sensor(0x4314, 0x00);
					OV5645_write_cmos_sensor(0x4315, 0x02);
					OV5645_write_cmos_sensor(0x4316, 0x68);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
					mDELAY(100);  
					OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3003, 0x00);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
				   break;

			 default:
				   break;
		 }
	 }
	 else {
		 // capture
		 switch(position) {
			 case LL:
			 		OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3002, 0x3c);
					OV5645_write_cmos_sensor(0x3003, 0x01);
					OV5645_write_cmos_sensor(0x3002, 0x1c);
            OV5645_write_cmos_sensor(0x4314, 0x2a);
            OV5645_write_cmos_sensor(0x4315, 0xdd);
            OV5645_write_cmos_sensor(0x4316, 0xb4);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
					mDELAY(100);  
					OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3003, 0x00);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
				   break;

			 case LR:
			 	  OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3002, 0x3c);
					OV5645_write_cmos_sensor(0x3003, 0x01);
					OV5645_write_cmos_sensor(0x3002, 0x1c);
					OV5645_write_cmos_sensor(0x4314, 0x2a);
					OV5645_write_cmos_sensor(0x4315, 0xe5);
					OV5645_write_cmos_sensor(0x4316, 0x2c);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
					mDELAY(100);  
					OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3003, 0x00);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
				   break;

			 case UL:
			 	  OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3002, 0x3c);
					OV5645_write_cmos_sensor(0x3003, 0x01);
					OV5645_write_cmos_sensor(0x3002, 0x1c);
					OV5645_write_cmos_sensor(0x4314, 0x00);
					OV5645_write_cmos_sensor(0x4315, 0x05);
					OV5645_write_cmos_sensor(0x4316, 0x38);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
					mDELAY(100);  
					OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3003, 0x00);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
				   break;

			 case UR:
			 		OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3002, 0x3c);
					OV5645_write_cmos_sensor(0x3003, 0x01);
					OV5645_write_cmos_sensor(0x3002, 0x1c);
					OV5645_write_cmos_sensor(0x4314, 0x00);
					OV5645_write_cmos_sensor(0x4315, 0x00);
					OV5645_write_cmos_sensor(0x4316, 0xf8);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
					mDELAY(100);  
					OV5645_write_cmos_sensor(0x3212, 0x00);
					OV5645_write_cmos_sensor(0x3003, 0x00);
					OV5645_write_cmos_sensor(0x3212, 0x10);
					OV5645_write_cmos_sensor(0x3212, 0xa0);
				   break;

			 default:
				   break;
		 }
	 }
}

void OV5645_Standby(void)
{
	OV5645_write_cmos_sensor(0x3008,0x42);
}

void OV5645_Wakeup(void)
{
	OV5645_write_cmos_sensor(0x3008,0x02);
}

/*************************************************************************
* FUNCTION
*   OV5645_FOCUS_OVT_AFC_Init
* DESCRIPTION
*   This function is to load micro code for AF function
* PARAMETERS
*   None
* RETURNS
*   None
* GLOBALS AFFECTED
*************************************************************************/
static kal_uint8 gtemp[255];

void OV5645_FOCUS_OVT_AFC_Init(void)
{
	int i, length, address;//, AF_status;
	OV5645_write_cmos_sensor(0x3000, 0x20);        // reset MCU

	length = sizeof(firmware);
	address = 0x8000;
#if 0
	for(i=0;i<length;i++) {
		OV5645_write_cmos_sensor(address, firmware[i]);
		address++;
	}
#else
{
#define PACK_MAX_LEN (252)
	int pack_cnt = length / PACK_MAX_LEN;
	int last_pack_cnt = length % PACK_MAX_LEN;
	kal_uint8 *pfirmware = (kal_uint8 *)firmware;
	for(i=0;i<pack_cnt;i++){
		gtemp[0] = (address>>8) & 0x00FF;
		gtemp[1] =address & 0x00FF ;
		memcpy(&gtemp[2], pfirmware, PACK_MAX_LEN);
		iBurstWriteReg(gtemp, PACK_MAX_LEN+2, OV5645_WRITE_ID);
		address += PACK_MAX_LEN;
		pfirmware += PACK_MAX_LEN;
	}
	if ( last_pack_cnt ){
		gtemp[0] = (address>>8) & 0x00FF;
		gtemp[1] =address & 0x00FF ;
		memcpy(&gtemp[2], pfirmware, last_pack_cnt);
		iBurstWriteReg(gtemp, last_pack_cnt+2, OV5645_WRITE_ID);
	}
}
#endif
	OV5645_write_cmos_sensor(0x3022,0x00);
	OV5645_write_cmos_sensor(0x3023,0x00);
	OV5645_write_cmos_sensor(0x3024,0x00);
	OV5645_write_cmos_sensor(0x3025,0x00);
	OV5645_write_cmos_sensor(0x3026,0x00);
	OV5645_write_cmos_sensor(0x3027,0x00);
	OV5645_write_cmos_sensor(0x3028,0x00);
	OV5645_write_cmos_sensor(0x3029, 0x7f);
	OV5645_write_cmos_sensor(0x3000, 0x00);        // Enable MCU
	if (false == AF_Power)
	{
		OV5645_write_cmos_sensor(0x3602,0x00);
		OV5645_write_cmos_sensor(0x3603,0x00);
	}
}
/*************************************************************************
* FUNCTION
*   OV5640_FOCUS_OVT_AFC_Constant_Focus
* DESCRIPTION
*   GET af stauts
* PARAMETERS
*   None
* RETURNS
*   None
* GLOBALS AFFECTED
*************************************************************************/	
void OV5645_FOCUS_OVT_AFC_Constant_Focus(void)
{
//	OV5645SENSORDB("[OV5645]enter OV5645_FOCUS_OVT_AFC_Constant_Focus function:\n ");
	OV5645_write_cmos_sensor(0x3023,0x01);
	OV5645_write_cmos_sensor(0x3022,0x04);
//	OV5645SENSORDB("[OV5645]exit OV5645_FOCUS_OVT_AFC_Constant_Focus function:\n ");
}   
/*************************************************************************
* FUNCTION
*   OV5640_FOCUS_OVT_AFC_Single_Focus
* DESCRIPTION
*   GET af stauts
* PARAMETERS
*   None
* RETURNS
*   None
* GLOBALS AFFECTED
*************************************************************************/	
void OV5645_FOCUS_OVT_AFC_Single_Focus(void)
{
//	OV5645SENSORDB("[OV5645]enter OV5645_FOCUS_OVT_AFC_Single_Focus function:\n ");
	OV5645_write_cmos_sensor(0x3023,0x01);
	OV5645_write_cmos_sensor(0x3022,0x81);//0x81
	mDELAY(20);
	OV5645_write_cmos_sensor(0x3022,0x03);
//	OV5645SENSORDB("[OV5645]exit OV5645_FOCUS_OVT_AFC_Single_Focus function:\n ");
}
/*************************************************************************
* FUNCTION
*   OV5640_FOCUS_OVT_AFC_Pause_Focus
* DESCRIPTION
*   GET af stauts
* PARAMETERS
*   None
* RETURNS
*   None
* GLOBALS AFFECTED
*************************************************************************/	
void OV5645_FOCUS_OVT_AFC_Pause(void)
{
	OV5645_write_cmos_sensor(0x3023,0x01);
	OV5645_write_cmos_sensor(0x3022,0x06);
}

void OV5645_FOCUS_OVT_AFC_Realse(void)
{
	OV5645_write_cmos_sensor(0x3023,0x01);
	OV5645_write_cmos_sensor(0x3022,0x08);
}

void OV5645_FOCUS_Get_AF_Max_Num_Focus_Areas(UINT32 *pFeatureReturnPara32)
{ 	  
    *pFeatureReturnPara32 = 1;    

//    OV5645SENSORDB(" *pFeatureReturnPara32 = %d\n",  *pFeatureReturnPara32);	
}

void OV5645_FOCUS_Get_AE_Max_Num_Metering_Areas(UINT32 *pFeatureReturnPara32)
{ 	
	*pFeatureReturnPara32 = 1;    
//	OV5645SENSORDB(" *pFeatureReturnPara32 = %d\n",  *pFeatureReturnPara32);
}

void OV5645_FOCUS_OVT_AFC_Touch_AF(INT32 x, INT32 y)
{
	int x_view,y_view;
#if 0
{
#define OV5645_TAF_TOLERANCE_X (10)
#define OV5645_TAF_TOLERANCE_Y (7)

	//check if we really need to update AF window? the tolerance shall less than 10 pixels
	//on the both x/y direction for inner & ouuter window
	if ((abs(x-OV5645MIPISensor.AF_window_x) < OV5645_TAF_TOLERANCE_X) &&
		(abs(y - OV5645MIPISensor.AF_window_y) < OV5645_TAF_TOLERANCE_Y))
	{
//		OV5645SENSORDB("AF window is very near the previous'...(%d, %d)-->(%d, %d)\n", 
//			OV5645MIPISensor.AF_window_x, OV5645MIPISensor.AF_window_y, x, y);
		return ERROR_NONE;
	}
	OV5645MIPISensor.AF_window_x = x;
	OV5645MIPISensor.AF_window_y = y ;
}
#endif
	//OV5645SENSORDB("X=%d, Y=%d\n", x, y);
	
	if(x<1){
		x_view=1;
	}
	else if(x>79){
		x_view=79;
	}
	else	{
		x_view= x;
	}

	if(y<1)	{
		y_view=1;
	}
	else if(y>44){ //59
		y_view=44; //59
	}
	else	{
		y_view= y;
	}
	OV5645_write_cmos_sensor(0x3024,x_view);
	OV5645_write_cmos_sensor(0x3025,y_view);   
}

void OV5645_FOCUS_Set_AF_Window(UINT32 zone_addr)
{//update global zone
	UINT32 FD_XS;
	UINT32 FD_YS;   
	UINT32 x0, y0, x1, y1;
	UINT32 pvx0, pvy0, pvx1, pvy1;
	//UINT32 linenum, rownum;
	INT32 AF_pvx, AF_pvy;
	UINT32* zone = (UINT32*)zone_addr;
	x0 = *zone;
	y0 = *(zone + 1);
	x1 = *(zone + 2);
	y1 = *(zone + 3);   
	FD_XS = *(zone + 4);
	FD_YS = *(zone + 5);
	if ( 0 == FD_XS || 0 == FD_YS ){
		OV5645SENSORDB("exit function--ERROR PARAM:\n ");
		return;
	}
	mapMiddlewaresizePointToPreviewsizePoint(x0,y0,FD_XS,FD_YS,&pvx0, &pvy0, PRV_W, PRV_H);
	mapMiddlewaresizePointToPreviewsizePoint(x1,y1,FD_XS,FD_YS,&pvx1, &pvy1, PRV_W, PRV_H);  
	AF_pvx =(pvx0+pvx1)/32;
	AF_pvy =(pvy0+pvy1)/32;
	OV5645_FOCUS_OVT_AFC_Touch_AF(AF_pvx ,AF_pvy);
}
void OV5645_FOCUS_Get_AF_Macro(UINT32 *pFeatureReturnPara32)
{
    *pFeatureReturnPara32 = 0;
}
void OV5645_FOCUS_Get_AF_Inf(UINT32 * pFeatureReturnPara32)
{
    *pFeatureReturnPara32 = 0;
}

/*************************************************************************
//此函数变量未定义,请根据编译定义数据变量.
//prview 1280*960 
//16的整数倍 ; n*16*80/1280
//16的整数倍 ; n*16*60/960
//touch_x  为对应preview的横坐标[0-1280]
//touch_y  为对应preview的纵坐标[0-960]
*************************************************************************/ 
UINT32 OV5645_FOCUS_Move_to(UINT32 a_u2MovePosition)//??how many bits for ov3640??
{
    return 0;
}
/*************************************************************************
* FUNCTION
*   OV5640_FOCUS_OVT_AFC_Get_AF_Status
* DESCRIPTION
*   GET af stauts
* PARAMETERS
*   None
* RETURNS
*   None
* GLOBALS AFFECTED
*************************************************************************/                        
void OV5645_FOCUS_OVT_AFC_Get_AF_Status(UINT32 *pFeatureReturnPara32)
{
	UINT32 state_3028=0;
	UINT32 state_3029=0;
	*pFeatureReturnPara32 = SENSOR_AF_IDLE;
	state_3028 = OV5645_read_cmos_sensor(0x3028);
	state_3029 = OV5645_read_cmos_sensor(0x3029);
	mDELAY(1);
	//OV5645SENSORDB("enter:state_3028=%d,state_3029=%d\n",state_3028,state_3029);
	if (state_3028==0)
	{
		*pFeatureReturnPara32 = SENSOR_AF_ERROR;    
	}
	else if (state_3028==1)
	{
		switch (state_3029)
		{
			case 0x70:
				*pFeatureReturnPara32 = SENSOR_AF_IDLE;
				break;
			case 0x00:
				*pFeatureReturnPara32 = SENSOR_AF_FOCUSING;
				break;
			case 0x10:
				*pFeatureReturnPara32 = SENSOR_AF_FOCUSED;
				break;
			case 0x20:
				*pFeatureReturnPara32 = SENSOR_AF_FOCUSED;
				break;
			default:
				*pFeatureReturnPara32 = SENSOR_AF_SCENE_DETECTING; 
				break;
		}                                  
	}    
//	OV5645SENSORDB("[OV5645]exit OV5645_FOCUS_OVT_AFC_Get_AF_Status function:state_3028=%d,state_3029=%d\n",state_3028,state_3029);
}

/*************************************************************************
* FUNCTION
*   OV5640_FOCUS_OVT_AFC_Cancel_Focus
* DESCRIPTION
*   cancel af 
* PARAMETERS
*   None
* RETURNS
*   None
* GLOBALS AFFECTED
*************************************************************************/     
void OV5645_FOCUS_OVT_AFC_Cancel_Focus(void)
{
    //OV5645_write_cmos_sensor(0x3023,0x01);
    //OV5645_write_cmos_sensor(0x3022,0x08);
}

/*************************************************************************
* FUNCTION
*   OV5645WBcalibattion
* DESCRIPTION
*   color calibration
* PARAMETERS
*   None
* RETURNS
*   None
* GLOBALS AFFECTED
*************************************************************************/	
void OV5645WBcalibattion(kal_uint32 color_r_gain,kal_uint32 color_b_gain)
{
	kal_uint32 color_r_gain_w = 0;
	kal_uint32 color_b_gain_w = 0;
	kal_uint8 temp = OV5645_read_cmos_sensor(0x350b); 
//	OV5645SENSORDB("[OV5645]enter OV5645WBcalibattion function:\n ");
	
	if(temp>=0xb0)
	{	
		color_r_gain_w=color_r_gain*98/100;																																														
		color_b_gain_w=color_b_gain*99/100;  
	}
	else if (temp>=0x70)
	{
		color_r_gain_w=color_r_gain *99/100;																																														
		color_b_gain_w=color_b_gain*100/100;
	}
	else if (temp>=0x30)
	{
		color_r_gain_w=color_r_gain;																																														
		color_b_gain_w=color_b_gain;
	}
	else
	{
		if(color_b_gain>0x730)
		{
			color_r_gain_w=color_r_gain;																																														
		color_b_gain_w=color_b_gain*99/100; 
        }
        else
		{
			color_r_gain_w=color_r_gain;																																														
			color_b_gain_w=color_b_gain;
		}
	}																																																																						
	OV5645_write_cmos_sensor(0x3400,(color_r_gain_w & 0xff00)>>8);																																														
	OV5645_write_cmos_sensor(0x3401,color_r_gain_w & 0xff); 			
	OV5645_write_cmos_sensor(0x3404,(color_b_gain_w & 0xff00)>>8);																																														
	OV5645_write_cmos_sensor(0x3405,color_b_gain_w & 0xff); 
//	OV5645SENSORDB("[OV5645]exit OV5645WBcalibattion function:\n ");
}	
BOOL OV5645_set_param_exposure_for_HDR(UINT16 para)
{
    kal_uint32 totalGain = 0, exposureTime = 0;
    OV5645SENSORDB("[OV5645]enter para=%d,manualAEStart%d\n",para,OV5645MIPISensor.manualAEStart);
    if (0 == OV5645MIPISensor.manualAEStart)
	{
        OV5645_set_AE_mode(KAL_FALSE);//Manual AE enable
		spin_lock(&ov5645_drv_lock);
        OV5645MIPISensor.manualAEStart = 1;
		spin_unlock(&ov5645_drv_lock);
	}
    totalGain = OV5645MIPISensor.currentAxDGain;
    exposureTime = OV5645MIPISensor.currentExposureTime;
    switch (para)
	{
    case AE_EV_COMP_20:    //+2 EV
    case AE_EV_COMP_10:    // +1 EV
        totalGain = totalGain<<1;
        exposureTime = exposureTime<<1;
        OV5645SENSORDB("[4EC] HDR AE_EV_COMP_20\n");
			break;
    case AE_EV_COMP_00:    // +0 EV
        OV5645SENSORDB("[4EC] HDR AE_EV_COMP_00\n");
			   break;
    case AE_EV_COMP_n10:  // -1 EV
    case AE_EV_COMP_n20:  // -2 EV
        totalGain = totalGain >> 1;
        exposureTime = exposureTime >> 1;
        OV5645SENSORDB("[4EC] HDR AE_EV_COMP_n20\n");
	  	   break;
		default:
        break;//return FALSE;
	}
    totalGain = (totalGain > OV5645_MAX_AXD_GAIN) ? OV5645_MAX_AXD_GAIN : totalGain;
    exposureTime = (exposureTime > OV5645_MAX_EXPOSURE_TIME) ? OV5645_MAX_EXPOSURE_TIME : exposureTime;
    OV5645WriteSensorGain(totalGain);
    OV5645WriteShutter(exposureTime);
	return TRUE;
}

/* [TC] YUV sensor */

BOOL OV5645_set_param_wb(UINT16 para)
{
    OV5645SENSORDB("enter: para = %d\n ", para);
    spin_lock(&ov5645_drv_lock);
    OV5645MIPISensor.awbMode = para;
    spin_unlock(&ov5645_drv_lock);

    switch (para)
    {
    case AWB_MODE_OFF:
        spin_lock(&ov5645_drv_lock);
        OV5645_AWB_ENABLE = KAL_FALSE;
        spin_unlock(&ov5645_drv_lock);
        OV5645_set_AWB_mode(OV5645_AWB_ENABLE);
        break;
    case AWB_MODE_AUTO:
        spin_lock(&ov5645_drv_lock);
        OV5645_AWB_ENABLE = KAL_TRUE;
        spin_unlock(&ov5645_drv_lock);
        OV5645_set_AWB_mode(OV5645_AWB_ENABLE);
        break;
    case AWB_MODE_CLOUDY_DAYLIGHT: //cloudy
        OV5645_write_cmos_sensor(0x3212,0x03);
        OV5645_set_AWB_mode(KAL_FALSE);
        OV5645_write_cmos_sensor(0x3400,0x06);
        OV5645_write_cmos_sensor(0x3401,0x40);
        OV5645_write_cmos_sensor(0x3402,0x04);
        OV5645_write_cmos_sensor(0x3403,0x00);
        OV5645_write_cmos_sensor(0x3404,0x04);
        OV5645_write_cmos_sensor(0x3405,0x10);
        OV5645_write_cmos_sensor(0x3212,0x13);
        OV5645_write_cmos_sensor(0x3212,0xa3);
        break;
    case AWB_MODE_DAYLIGHT: //sunny
        OV5645_write_cmos_sensor(0x3212,0x03);
        OV5645_set_AWB_mode(KAL_FALSE);
        OV5645_write_cmos_sensor(0x3400,0x06);
        OV5645_write_cmos_sensor(0x3401,0x00);
        OV5645_write_cmos_sensor(0x3402,0x04);
        OV5645_write_cmos_sensor(0x3403,0x00);
        OV5645_write_cmos_sensor(0x3404,0x04);
        OV5645_write_cmos_sensor(0x3405,0x98);
        OV5645_write_cmos_sensor(0x3212,0x13);
        OV5645_write_cmos_sensor(0x3212,0xa3);
        break;
    case AWB_MODE_INCANDESCENT: //office
        OV5645_write_cmos_sensor(0x3212,0x03);
        OV5645_set_AWB_mode(KAL_FALSE);
        OV5645_write_cmos_sensor(0x3400,0x04);
        OV5645_write_cmos_sensor(0x3401,0xe0);
        OV5645_write_cmos_sensor(0x3402,0x04);
        OV5645_write_cmos_sensor(0x3403,0x00);
        OV5645_write_cmos_sensor(0x3404,0x05);
        OV5645_write_cmos_sensor(0x3405,0xa0);
        OV5645_write_cmos_sensor(0x3212,0x13);
        OV5645_write_cmos_sensor(0x3212,0xa3);
        break;
    case AWB_MODE_TUNGSTEN:
        OV5645_write_cmos_sensor(0x3212,0x03);
        OV5645_set_AWB_mode(KAL_FALSE);
        OV5645_write_cmos_sensor(0x3400,0x05);
        OV5645_write_cmos_sensor(0x3401,0x48);
        OV5645_write_cmos_sensor(0x3402,0x04);
        OV5645_write_cmos_sensor(0x3403,0x00);
        OV5645_write_cmos_sensor(0x3404,0x05);
        OV5645_write_cmos_sensor(0x3405,0xe0);
        OV5645_write_cmos_sensor(0x3212,0x13);
        OV5645_write_cmos_sensor(0x3212,0xa3);
        break;
    case AWB_MODE_FLUORESCENT:
        OV5645_write_cmos_sensor(0x3212,0x03);
        OV5645_set_AWB_mode(KAL_FALSE);
        OV5645_write_cmos_sensor(0x3400,0x04);
        OV5645_write_cmos_sensor(0x3401,0x00);
        OV5645_write_cmos_sensor(0x3402,0x04);
        OV5645_write_cmos_sensor(0x3403,0x00);
        OV5645_write_cmos_sensor(0x3404,0x06);
        OV5645_write_cmos_sensor(0x3405,0x50);
        OV5645_write_cmos_sensor(0x3212,0x13);
        OV5645_write_cmos_sensor(0x3212,0xa3);
        break;
    default:
        return FALSE;
    }
    spin_lock(&ov5645_drv_lock);
    OV5645MIPISensor.iWB = para;
    spin_unlock(&ov5645_drv_lock);
       return TRUE;
} /* OV5645_set_param_wb */

void OV5645_set_contrast(UINT16 para)
{
    OV5645SENSORDB("enter:\n ");
    switch (para)
    {
    case ISP_CONTRAST_LOW:
         OV5645_write_cmos_sensor(0x3212,0x03);
         OV5645_write_cmos_sensor(0x5586,0x14);
         OV5645_write_cmos_sensor(0x5585,0x14);
         OV5645_write_cmos_sensor(0x3212,0x13);
         OV5645_write_cmos_sensor(0x3212,0xa3);
         break;
    case ISP_CONTRAST_HIGH:
         OV5645_write_cmos_sensor(0x3212,0x03);
         OV5645_write_cmos_sensor(0x5586,0x2c);
         OV5645_write_cmos_sensor(0x5585,0x1c);
         OV5645_write_cmos_sensor(0x3212,0x13);
         OV5645_write_cmos_sensor(0x3212,0xa3);
         break;
    case ISP_CONTRAST_MIDDLE:
         OV5645_write_cmos_sensor(0x3212,0x03);
         OV5645_write_cmos_sensor(0x5586,0x20);
         OV5645_write_cmos_sensor(0x5585,0x00);
         OV5645_write_cmos_sensor(0x3212,0x13);
         OV5645_write_cmos_sensor(0x3212,0xa3);
         break;
    default:
         break;
    }
    return;
}
void OV5645_set_brightness(UINT16 para)
{
    OV5645SENSORDB("enter:\n ");
    switch (para)
    {
        case ISP_BRIGHT_LOW:
             OV5645_write_cmos_sensor(0x3212,0x03);
             OV5645_write_cmos_sensor(0x5587,0x10);
             OV5645_write_cmos_sensor(0x5588,0x09);
             OV5645_write_cmos_sensor(0x3212,0x13);
             OV5645_write_cmos_sensor(0x3212,0xa3);
             break;
        case ISP_BRIGHT_HIGH:
             OV5645_write_cmos_sensor(0x3212,0x03);
             OV5645_write_cmos_sensor(0x5587,0x10);
             OV5645_write_cmos_sensor(0x5588,0x01);
             OV5645_write_cmos_sensor(0x3212,0x13);
             OV5645_write_cmos_sensor(0x3212,0xa3);
             break;
        case ISP_BRIGHT_MIDDLE:
             OV5645_write_cmos_sensor(0x3212,0x03);
             OV5645_write_cmos_sensor(0x5587,0x00);
             OV5645_write_cmos_sensor(0x5588,0x01);
             OV5645_write_cmos_sensor(0x3212,0x13);
             OV5645_write_cmos_sensor(0x3212,0xa3);
             break;
        default:
             return;
             break;
    }
    return;
}
void OV5645_set_saturation(UINT16 para)
{
    OV5645SENSORDB("enter:\n ");
    switch (para)
    {
    case ISP_SAT_HIGH:
        OV5645_write_cmos_sensor(0x3212,0x03);
	OV5645_write_cmos_sensor(0x5381,0x22);//ccm
	OV5645_write_cmos_sensor(0x5382,0x55);//
	OV5645_write_cmos_sensor(0x5383,0x12);//
	OV5645_write_cmos_sensor(0x5384,0x04);//
	OV5645_write_cmos_sensor(0x5385,0x94);//
	OV5645_write_cmos_sensor(0x5386,0x98);//
	OV5645_write_cmos_sensor(0x5387,0xa9);//
	OV5645_write_cmos_sensor(0x5388,0x9a);//
	OV5645_write_cmos_sensor(0x5389,0x0e);//    
	OV5645_write_cmos_sensor(0x538a,0x01);//      
	OV5645_write_cmos_sensor(0x538b,0x98);//      			                                           
        OV5645_write_cmos_sensor(0x3212,0x13);
        OV5645_write_cmos_sensor(0x3212,0xa3);
        break;
    case ISP_SAT_LOW:
        OV5645_write_cmos_sensor(0x3212,0x03);
	OV5645_write_cmos_sensor(0x5381,0x22);//ccm
	OV5645_write_cmos_sensor(0x5382,0x55);//
	OV5645_write_cmos_sensor(0x5383,0x12);//
	OV5645_write_cmos_sensor(0x5384,0x02);//
	OV5645_write_cmos_sensor(0x5385,0x62);//
	OV5645_write_cmos_sensor(0x5386,0x66);//
	OV5645_write_cmos_sensor(0x5387,0x71);//
	OV5645_write_cmos_sensor(0x5388,0x66);//
	OV5645_write_cmos_sensor(0x5389,0x0a);//    
	OV5645_write_cmos_sensor(0x538a,0x01);//      
	OV5645_write_cmos_sensor(0x538b,0x98);//      			                                           
        OV5645_write_cmos_sensor(0x3212,0x13);
        OV5645_write_cmos_sensor(0x3212,0xa3);
        break;
    case ISP_SAT_MIDDLE:
	OV5645_write_cmos_sensor(0x3212,0x03);
	OV5645_write_cmos_sensor(0x5381,0x24);//ccm //0x22 28
	OV5645_write_cmos_sensor(0x5382,0x5b);// //0x55 66
	OV5645_write_cmos_sensor(0x5383,0x13);// //0x12 15
	OV5645_write_cmos_sensor(0x5384,0x03);//
	OV5645_write_cmos_sensor(0x5385,0x7f);//
	OV5645_write_cmos_sensor(0x5386,0x83);//
	OV5645_write_cmos_sensor(0x5387,0x91);//
	OV5645_write_cmos_sensor(0x5388,0x84);//
	OV5645_write_cmos_sensor(0x5389,0x0d);//
	OV5645_write_cmos_sensor(0x538a, 0x01);//
	OV5645_write_cmos_sensor(0x538b, 0x98);//
	OV5645_write_cmos_sensor(0x3212,0x13);
	OV5645_write_cmos_sensor(0x3212,0xa3);
	break;
    default:
        break;
    }
    mDELAY(50);
    return;
}
void OV5645_scene_mode_PORTRAIT(void)
{
    OV5645SENSORDB("enter:\n ");
    OV5645_write_cmos_sensor(0x3212,0x03);
    /*FRAME rate*/
#if 1
    OV5645_write_cmos_sensor(0x3A00, 0x38); //10-30
    OV5645_write_cmos_sensor(0x3a02, 0x02);//    ; max exp 60 = 740
    OV5645_write_cmos_sensor(0x3a03, 0xe4);//    ; max exp 60
    OV5645_write_cmos_sensor(0x3a14, 0x02);//    ; max exp 50 = 740
    OV5645_write_cmos_sensor(0x3a15, 0xe4);//    ; max exp 50
#else
    OV5645_write_cmos_sensor(0x3A00,0x3c); //fix30
    OV5645_write_cmos_sensor(0x3a02,0x0b);
    OV5645_write_cmos_sensor(0x3a03,0x88);
    OV5645_write_cmos_sensor(0x3a14,0x0b);
    OV5645_write_cmos_sensor(0x3a15,0x88);
#endif
    OV5645_write_cmos_sensor(0x350b,0x00);
    OV5645_write_cmos_sensor(0x350c,0x00);
    OV5645_write_cmos_sensor(0x350d,0x00);

    /*AE Weight - Average*/
    OV5645_write_cmos_sensor(0x501d,0x00);
    OV5645_write_cmos_sensor(0x5688,0x11);
    OV5645_write_cmos_sensor(0x5689,0x11);
	OV5645_write_cmos_sensor(0x568a,0x21);
	OV5645_write_cmos_sensor(0x568b,0x12);
	OV5645_write_cmos_sensor(0x568c,0x12);
	OV5645_write_cmos_sensor(0x568d,0x21);
    OV5645_write_cmos_sensor(0x568e,0x11);
    OV5645_write_cmos_sensor(0x568f,0x11);
    OV5645_write_cmos_sensor(0x3212,0x13);
    OV5645_write_cmos_sensor(0x3212,0xa3);
}

void OV5645_scene_mode_LANDSCAPE(void)
{
    OV5645SENSORDB("enter:\n ");
    OV5645_write_cmos_sensor(0x3212,0x03);
    /*FRAME rate*/
#if 1
    OV5645_write_cmos_sensor(0x3A00, 0x38); //10-30
    OV5645_write_cmos_sensor(0x3a02, 0x02);//    ; max exp 60 = 740
    OV5645_write_cmos_sensor(0x3a03, 0xe4);//    ; max exp 60
    OV5645_write_cmos_sensor(0x3a14, 0x02);//    ; max exp 50 = 740
    OV5645_write_cmos_sensor(0x3a15, 0xe4);//    ; max exp 50
#else
    OV5645_write_cmos_sensor(0x3A00,0x3c); //10-30
    OV5645_write_cmos_sensor(0x3a02,0x0b);
    OV5645_write_cmos_sensor(0x3a03,0x88);
    OV5645_write_cmos_sensor(0x3a14,0x0b);
    OV5645_write_cmos_sensor(0x3a15,0x88);
#endif
    OV5645_write_cmos_sensor(0x350b,0x00);
    OV5645_write_cmos_sensor(0x350c,0x00);
    OV5645_write_cmos_sensor(0x350d,0x00);

    /*AE Weight - Average*/
    OV5645_write_cmos_sensor(0x501d,0x00);
	OV5645_write_cmos_sensor(0x5688,0x11);
	OV5645_write_cmos_sensor(0x5689,0x11);
	OV5645_write_cmos_sensor(0x568a,0x11);
	OV5645_write_cmos_sensor(0x568b,0x11);
	OV5645_write_cmos_sensor(0x568c,0x11);
	OV5645_write_cmos_sensor(0x568d,0x11);
	OV5645_write_cmos_sensor(0x568e,0x11);
	OV5645_write_cmos_sensor(0x568f,0x11);
    OV5645_write_cmos_sensor(0x3212,0x13);
    OV5645_write_cmos_sensor(0x3212,0xa3);
}
void OV5645_scene_mode_SUNSET(void)
{
    OV5645SENSORDB("enter:\n ");
    OV5645_write_cmos_sensor(0x3212,0x03);
#if 1
    OV5645_write_cmos_sensor(0x3A00, 0x38); //10-30
    OV5645_write_cmos_sensor(0x3a02, 0x02);//    ; max exp 60 = 740
    OV5645_write_cmos_sensor(0x3a03, 0xe4);//    ; max exp 60
    OV5645_write_cmos_sensor(0x3a14, 0x02);//    ; max exp 50 = 740
    OV5645_write_cmos_sensor(0x3a15, 0xe4);//    ; max exp 50
#else
    OV5645_write_cmos_sensor(0x3A00,0x3c); //10-30
    OV5645_write_cmos_sensor(0x3a02,0x0b);
    OV5645_write_cmos_sensor(0x3a03,0x88);
    OV5645_write_cmos_sensor(0x3a14,0x0b);
    OV5645_write_cmos_sensor(0x3a15,0x88);
#endif
    OV5645_write_cmos_sensor(0x350b,0x00);
    OV5645_write_cmos_sensor(0x350c,0x00);
    OV5645_write_cmos_sensor(0x350d,0x00);

    /*AE Weight - Average*/
    OV5645_write_cmos_sensor(0x501d,0x00);
    OV5645_write_cmos_sensor(0x5688,0x11);
    OV5645_write_cmos_sensor(0x5689,0x11);
    OV5645_write_cmos_sensor(0x568a,0x11);
    OV5645_write_cmos_sensor(0x568b,0x11);
    OV5645_write_cmos_sensor(0x568c,0x11);
    OV5645_write_cmos_sensor(0x568d,0x11);
    OV5645_write_cmos_sensor(0x568e,0x11);
    OV5645_write_cmos_sensor(0x568f,0x11);
    OV5645_write_cmos_sensor(0x3212,0x13);
    OV5645_write_cmos_sensor(0x3212,0xa3);

}
void OV5645_scene_mode_SPORTS(void)
{
    OV5645SENSORDB("enter:\n ");
    OV5645_write_cmos_sensor(0x3212,0x03);
    /*FRAME rate*/
    OV5645_write_cmos_sensor(0x3A00, 0x38); //10-30
    OV5645_write_cmos_sensor(0x3a02, 0x02);//    ; max exp 60 = 740
    OV5645_write_cmos_sensor(0x3a03, 0xe4);//    ; max exp 60
    OV5645_write_cmos_sensor(0x3a14, 0x02);//    ; max exp 50 = 740
    OV5645_write_cmos_sensor(0x3a15, 0xe4);//    ; max exp 50
    OV5645_write_cmos_sensor(0x350b,0x00);
    OV5645_write_cmos_sensor(0x350c,0x00);
    OV5645_write_cmos_sensor(0x350d,0x00);

    /*AE Weight - Average*/
    OV5645_write_cmos_sensor(0x501d,0x00);
    OV5645_write_cmos_sensor(0x5688,0x11);
    OV5645_write_cmos_sensor(0x5689,0x11);
    OV5645_write_cmos_sensor(0x568a,0x11);
    OV5645_write_cmos_sensor(0x568b,0x11);
    OV5645_write_cmos_sensor(0x568c,0x11);
    OV5645_write_cmos_sensor(0x568d,0x11);
    OV5645_write_cmos_sensor(0x568e,0x11);
    OV5645_write_cmos_sensor(0x568f,0x11);
    OV5645_write_cmos_sensor(0x3212,0x13);
    OV5645_write_cmos_sensor(0x3212,0xa3);

}
void OV5645_scene_mode_night(void)
{
    OV5645SENSORDB("enter:\n ");
    OV5645_write_cmos_sensor(0x3212,0x03);
    /*FRAME rate*/
    if(OV5645MIPISensor.NightMode)
    {
        OV5645_write_cmos_sensor(0x3A00,0x38); //5-30
        spin_lock(&ov5645_drv_lock);
        OV5645MIPISensor.NightMode=KAL_FALSE;
        spin_unlock(&ov5645_drv_lock);
    }
    else
    OV5645_write_cmos_sensor(0x3A00,0x3c);
    OV5645_write_cmos_sensor(0x3a02,0x17);
    OV5645_write_cmos_sensor(0x3a03,0x10);
    OV5645_write_cmos_sensor(0x3a14,0x17);
    OV5645_write_cmos_sensor(0x3a15,0x88);
    OV5645_write_cmos_sensor(0x350b,0x00);
    OV5645_write_cmos_sensor(0x350c,0x00);
    OV5645_write_cmos_sensor(0x350d,0x00);

    /*AE Weight - Average*/
    OV5645_write_cmos_sensor(0x501d,0x00);
    OV5645_write_cmos_sensor(0x5688,0x11);
    OV5645_write_cmos_sensor(0x5689,0x11);
    OV5645_write_cmos_sensor(0x568a,0x11);
    OV5645_write_cmos_sensor(0x568b,0x11);
    OV5645_write_cmos_sensor(0x568c,0x11);
    OV5645_write_cmos_sensor(0x568d,0x11);
    OV5645_write_cmos_sensor(0x568e,0x11);
    OV5645_write_cmos_sensor(0x568f,0x11);
    OV5645_write_cmos_sensor(0x3212,0x13);
    OV5645_write_cmos_sensor(0x3212,0xa3);
}
void OV5645_scene_mode_OFF(void)
{
    OV5645SENSORDB("enter:\n ");
    OV5645_write_cmos_sensor(0x3212,0x03);
        /*FRAME rate*/
#if 1
    OV5645_write_cmos_sensor(0x3A00, 0x38); //10-30
    OV5645_write_cmos_sensor(0x3a02, 0x02);//    ; max exp 60 = 740
    OV5645_write_cmos_sensor(0x3a03, 0xe4);//    ; max exp 60
    OV5645_write_cmos_sensor(0x3a14, 0x02);//    ; max exp 50 = 740
    OV5645_write_cmos_sensor(0x3a15, 0xe4);//    ; max exp 50
#else
    OV5645_write_cmos_sensor(0x3A00,0x3c); //10-30
    OV5645_write_cmos_sensor(0x3a02,0x0b);
    OV5645_write_cmos_sensor(0x3a03,0x88);
    OV5645_write_cmos_sensor(0x3a14,0x0b);
    OV5645_write_cmos_sensor(0x3a15,0x88);
#endif
    OV5645_write_cmos_sensor(0x350b,0x00);
    OV5645_write_cmos_sensor(0x350c,0x00);
    OV5645_write_cmos_sensor(0x350d,0x00);

    /*AE Weight - Average*/
    OV5645_write_cmos_sensor(0x501d,0x00);
    OV5645_write_cmos_sensor(0x5688,0x11);
    OV5645_write_cmos_sensor(0x5689,0x11);
    OV5645_write_cmos_sensor(0x568a,0x11);
    OV5645_write_cmos_sensor(0x568b,0x11);
    OV5645_write_cmos_sensor(0x568c,0x11);
    OV5645_write_cmos_sensor(0x568d,0x11);
    OV5645_write_cmos_sensor(0x568e,0x11);
    OV5645_write_cmos_sensor(0x568f,0x11);
    OV5645_write_cmos_sensor(0x3212,0x13);
    OV5645_write_cmos_sensor(0x3212,0xa3);
}

void OV5645_set_scene_mode(UINT16 para)
{
    OV5645SENSORDB("mode=%d",para);

    spin_lock(&ov5645_drv_lock);
    OV5645MIPISensor.sceneMode=para;
    spin_unlock(&ov5645_drv_lock);
    switch (para)
    {
    case SCENE_MODE_NIGHTSCENE:
        OV5645_scene_mode_night();
        break;
    case SCENE_MODE_PORTRAIT:
        OV5645_scene_mode_PORTRAIT();
         break;
    case SCENE_MODE_LANDSCAPE:
        OV5645_scene_mode_LANDSCAPE();
         break;
    case SCENE_MODE_SUNSET:
        OV5645_scene_mode_SUNSET();
        break;
    case SCENE_MODE_SPORTS:
        OV5645_scene_mode_SPORTS();
        break;
    case SCENE_MODE_HDR:
        if (1 == OV5645MIPISensor.manualAEStart)
        {
            OV5645_set_AE_mode(KAL_TRUE);//Manual AE disable
            spin_lock(&ov5645_drv_lock);
            OV5645MIPISensor.manualAEStart = 0;
            OV5645MIPISensor.currentExposureTime = 0;
            OV5645MIPISensor.currentAxDGain = 0;
            spin_unlock(&ov5645_drv_lock);
        }
        break;
    case SCENE_MODE_OFF:
        OV5645SENSORDB("[OV5645]set SCENE_MODE_OFF :\n ");
    default:
        OV5645_scene_mode_OFF();
        OV5645SENSORDB("[OV5645]set default mode :\n ");
        break;
    }
    return;
}
void OV5645_set_iso(UINT16 para)
{
    spin_lock(&ov5645_drv_lock);
    OV5645MIPISensor.isoSpeed = para;
    spin_unlock(&ov5645_drv_lock);
    switch (para)
    {
    case AE_ISO_100:
    case AE_ISO_AUTO:
        //ISO 100
        OV5645_write_cmos_sensor(0x3a18, 0x00);
        OV5645_write_cmos_sensor(0x3a19, 0x60);
        break;
    case AE_ISO_200:
        //ISO 200
        OV5645_write_cmos_sensor(0x3a18, 0x00);
        OV5645_write_cmos_sensor(0x3a19, 0x90);
        break;
    case AE_ISO_400:
        //ISO 400
        OV5645_write_cmos_sensor(0x3a18, 0x00);
        OV5645_write_cmos_sensor(0x3a19, 0xc0);
        break;
    default:
        break;
    }
    return;
}

BOOL OV5645_set_param_effect(UINT16 para)
{
	OV5645SENSORDB("enter: para=%d\n ", para);
	switch (para)
	{
	case MEFFECT_OFF:  
		OV5645_write_cmos_sensor(0x3212,0x03);
		OV5645_write_cmos_sensor(0x5580,0x06); 
		if ( OV5645MIPISensor.is_support_af ){
			OV5645_write_cmos_sensor(0x5583,0x38); 
		}
		else{
			OV5645_write_cmos_sensor(0x5583,0x40); 
		}
		OV5645_write_cmos_sensor(0x5584,0x20);
		OV5645_write_cmos_sensor(0x3212,0x13); 
		OV5645_write_cmos_sensor(0x3212,0xa3);
		break;
	case MEFFECT_SEPIA: 
		OV5645_write_cmos_sensor(0x3212,0x03);
		OV5645_write_cmos_sensor(0x5580,0x1e);
		OV5645_write_cmos_sensor(0x5583,0x40); 
		OV5645_write_cmos_sensor(0x5584,0xa0);
		OV5645_write_cmos_sensor(0x3212,0x13); 
		OV5645_write_cmos_sensor(0x3212,0xa3);
		break;
	case MEFFECT_NEGATIVE:
		OV5645_write_cmos_sensor(0x3212,0x03);
		OV5645_write_cmos_sensor(0x5580,0x46);
		OV5645_write_cmos_sensor(0x5583,0x40); 
             OV5645_write_cmos_sensor(0x5584,0x20);
		OV5645_write_cmos_sensor(0x3212,0x13); 
		OV5645_write_cmos_sensor(0x3212,0xa3);
		break;
	case MEFFECT_SEPIAGREEN:
		OV5645_write_cmos_sensor(0x3212,0x03);
		OV5645_write_cmos_sensor(0x5580,0x1e);			 
		OV5645_write_cmos_sensor(0x5583,0x60); 
		OV5645_write_cmos_sensor(0x5584,0x60);
		OV5645_write_cmos_sensor(0x3212,0x13); 
		OV5645_write_cmos_sensor(0x3212,0xa3);
		break;
	case MEFFECT_SEPIABLUE:
		OV5645_write_cmos_sensor(0x3212,0x03);
		OV5645_write_cmos_sensor(0x5580,0x1e);             
		OV5645_write_cmos_sensor(0x5583,0xa0); 
		OV5645_write_cmos_sensor(0x5584,0x40);
		OV5645_write_cmos_sensor(0x3212,0x13); 
		OV5645_write_cmos_sensor(0x3212,0xa3);
		break;
	case MEFFECT_MONO: //B&W
		OV5645_write_cmos_sensor(0x3212,0x03);
		OV5645_write_cmos_sensor(0x5580,0x1e);      		
		OV5645_write_cmos_sensor(0x5583,0x80); 
		OV5645_write_cmos_sensor(0x5584,0x80);
		OV5645_write_cmos_sensor(0x3212,0x13); 
		OV5645_write_cmos_sensor(0x3212,0xa3);
		break;
	default:
		return KAL_FALSE;
	}    
	return KAL_FALSE;
} /* OV5645_set_param_effect */

BOOL OV5645_set_param_banding(UINT16 para)
{
	switch (para)
    {
        case AE_FLICKER_MODE_50HZ:
						spin_lock(&ov5645_drv_lock);
						OV5645_Banding_setting = AE_FLICKER_MODE_50HZ;
						spin_unlock(&ov5645_drv_lock);
						OV5645_write_cmos_sensor(0x3c00,0x04);
						OV5645_write_cmos_sensor(0x3c01,0x80);
            break;
        case AE_FLICKER_MODE_60HZ:			
						spin_lock(&ov5645_drv_lock);
						OV5645_Banding_setting = AE_FLICKER_MODE_60HZ;
						spin_unlock(&ov5645_drv_lock);
						OV5645_write_cmos_sensor(0x3c00,0x00);
						OV5645_write_cmos_sensor(0x3c01,0x80);
            break;
    case AE_FLICKER_MODE_AUTO:
        OV5645_write_cmos_sensor(0x3c00,0x04);
        OV5645_write_cmos_sensor(0x3c01,0x80);
        break;
    case AE_FLICKER_MODE_OFF:
        OV5645_write_cmos_sensor(0x3c00,0x04);
        OV5645_write_cmos_sensor(0x3c01,0x80);
        break;
            default:
                 return FALSE;
    }
        return TRUE;
} /* OV5645_set_param_banding */


BOOL OV5645_set_param_exposure(UINT16 para)
{
    OV5645SENSORDB("para=%d:\n",para);
    //spin_lock(&ov5645_drv_lock);
    if (SCENE_MODE_HDR == OV5645MIPISensor.sceneMode &&
        SENSOR_MODE_CAPTURE == OV5645MIPISensor.SensorMode)
    {
        //spin_unlock(&ov5645_drv_lock);
        OV5645_set_param_exposure_for_HDR(para);
        return TRUE;
    }
    //spin_unlock(&ov5645_drv_lock);
    switch (para)
    {
    case AE_EV_COMP_20:
        OV5645_write_cmos_sensor(0x3a0f, 0x50);//    ; AEC in H
        OV5645_write_cmos_sensor(0x3a10, 0x48);//    ; AEC in L
        OV5645_write_cmos_sensor(0x3a11, 0x90);//    ; AEC out H
        OV5645_write_cmos_sensor(0x3a1b, 0x50);//    ; AEC out L
        OV5645_write_cmos_sensor(0x3a1e, 0x48);//    ; control zone H
        OV5645_write_cmos_sensor(0x3a1f, 0x24);//    ; control zone L
        break;
    case AE_EV_COMP_10:
        OV5645_write_cmos_sensor(0x3a0f, 0x40);//    ; AEC in H
        OV5645_write_cmos_sensor(0x3a10, 0x38);//    ; AEC in L
        OV5645_write_cmos_sensor(0x3a11, 0x80);//    ; AEC out H
        OV5645_write_cmos_sensor(0x3a1b, 0x40);//    ; AEC out L
        OV5645_write_cmos_sensor(0x3a1e, 0x38);//    ; control zone H
        OV5645_write_cmos_sensor(0x3a1f, 0x1c);//    ; control zone L
        break;
    case AE_EV_COMP_00:
			OV5645_write_cmos_sensor(0x3a0f, 0x29);//	; AEC in H
			OV5645_write_cmos_sensor(0x3a10, 0x23);//	; AEC in L
			OV5645_write_cmos_sensor(0x3a11, 0x53);//	; AEC out H
			OV5645_write_cmos_sensor(0x3a1b, 0x29);//	; AEC out L
			OV5645_write_cmos_sensor(0x3a1e, 0x23);//	; control zone H
			OV5645_write_cmos_sensor(0x3a1f, 0x12);//	; control zone L  
        break;
    case AE_EV_COMP_n10:
			OV5645_write_cmos_sensor(0x3a0f, 0x22);//	; AEC in H
			OV5645_write_cmos_sensor(0x3a10, 0x1e);//	; AEC in L
			OV5645_write_cmos_sensor(0x3a11, 0x45);//	; AEC out H
			OV5645_write_cmos_sensor(0x3a1b, 0x22);//	; AEC out L
			OV5645_write_cmos_sensor(0x3a1e, 0x1e);//	; control zone H
        OV5645_write_cmos_sensor(0x3a1f, 0x10);//    ; control zone L
        break;
    case AE_EV_COMP_n20:  // -2 EV
			OV5645_write_cmos_sensor(0x3a0f, 0x1e);//	; AEC in H
			OV5645_write_cmos_sensor(0x3a10, 0x18);//	; AEC in L
			OV5645_write_cmos_sensor(0x3a11, 0x3c);//	; AEC out H
			OV5645_write_cmos_sensor(0x3a1b, 0x1e);//	; AEC out L
			OV5645_write_cmos_sensor(0x3a1e, 0x18);//	; control zone H
        OV5645_write_cmos_sensor(0x3a1f, 0x0c);//    ; control zone L
        break;
    default:
        return FALSE;
    }
    return TRUE;
} /* OV5645_set_param_exposure */

UINT32 OV5645YUVSetVideoMode(UINT16 u2FrameRate)
{
	OV5645SENSORDB("[OV5645]enter OV5645YUVSetVideoMode function:\n ");
	spin_lock(&ov5645_drv_lock);
	OV5645MIPISensor.NightMode=KAL_TRUE;
	spin_unlock(&ov5645_drv_lock);
	if (u2FrameRate == 30)
	{
		//;OV5645 1280x960,30fps
		//56Mhz, 224Mbps/Lane, 2Lane.
		OV5645SENSORDB("[OV5645]OV5645YUVSetVideoMode enter u2FrameRate == 30 setting  :\n ");
		OV5645_write_cmos_sensor(0x3A00, 0x38); //fix30
		OV5645_write_cmos_sensor(0x300e, 0x45);//	; MIPI 2 lane
		OV5645_write_cmos_sensor(0x3034, 0x18); // PLL, MIPI 8-bit mode
		OV5645_write_cmos_sensor(0x3035, 0x21); // PLL
		OV5645_write_cmos_sensor(0x3036, 0x70); // PLL
		OV5645_write_cmos_sensor(0x3037, 0x13); // PLL
		OV5645_write_cmos_sensor(0x3108, 0x01); // PLL
		OV5645_write_cmos_sensor(0x3824, 0x01); // PLL
		OV5645_write_cmos_sensor(0x460c, 0x20); // PLL
		OV5645_write_cmos_sensor(0x3618, 0x00);//
		OV5645_write_cmos_sensor(0x3600, 0x09);//
		OV5645_write_cmos_sensor(0x3601, 0x43);//
		OV5645_write_cmos_sensor(0x3708, 0x66);//
		OV5645_write_cmos_sensor(0x3709, 0x12);//
		OV5645_write_cmos_sensor(0x370c, 0xc3);//
		OV5645_write_cmos_sensor(0x3800, 0x00); // HS = 0
		OV5645_write_cmos_sensor(0x3801, 0x00); // HS
		OV5645_write_cmos_sensor(0x3802, 0x00); // VS = 250
		OV5645_write_cmos_sensor(0x3803, 0x06); // VS
		OV5645_write_cmos_sensor(0x3804, 0x0a); // HW = 2623
		OV5645_write_cmos_sensor(0x3805, 0x3f);//	; HW
		OV5645_write_cmos_sensor(0x3806, 0x07);//	; VH = 
		OV5645_write_cmos_sensor(0x3807, 0x9d);//	; VH
		OV5645_write_cmos_sensor(0x3808, 0x05);//	; DVPHO = 1280
		OV5645_write_cmos_sensor(0x3809, 0x00);//	; DVPHO
		OV5645_write_cmos_sensor(0x380a, 0x03);//	; DVPVO = 960
		OV5645_write_cmos_sensor(0x380b, 0xc0);//	; DVPVO
		OV5645_write_cmos_sensor(0x380c, 0x07);//	; HTS = 2160
		OV5645_write_cmos_sensor(0x380d, 0x68);//	; HTS
		OV5645_write_cmos_sensor(0x380e, 0x03);//	; VTS = 740
		OV5645_write_cmos_sensor(0x380f, 0xd8);//	; VTS
		OV5645_write_cmos_sensor(0x3810, 0x00); // H OFF = 16
		OV5645_write_cmos_sensor(0x3811, 0x10); // H OFF
		OV5645_write_cmos_sensor(0x3812, 0x00); // V OFF = 4
		OV5645_write_cmos_sensor(0x3813, 0x06);//	; V OFF
		OV5645_write_cmos_sensor(0x3814, 0x31);//	; X INC
		OV5645_write_cmos_sensor(0x3815, 0x31);//	; Y INC
		OV5645_write_cmos_sensor(0x3820, 0x41);//	; flip off, V bin on
		OV5645_write_cmos_sensor(0x3821, 0x07);//	; mirror on, H bin on
		OV5645_write_cmos_sensor(0x4514, 0x00);
		OV5645_write_cmos_sensor(0x3a00, 0x38);//	; ae mode	
		OV5645_write_cmos_sensor(0x3a02, 0x03);//	; max exp 60 = 740
		OV5645_write_cmos_sensor(0x3a03, 0xd8);//	; max exp 60
		OV5645_write_cmos_sensor(0x3a08, 0x01);//	; B50 = 222
		OV5645_write_cmos_sensor(0x3a09, 0x27);//	; B50
		OV5645_write_cmos_sensor(0x3a0a, 0x00);//	; B60 = 185
		OV5645_write_cmos_sensor(0x3a0b, 0xf6);//	; B60
		OV5645_write_cmos_sensor(0x3a0e, 0x03);//	; max 50
		OV5645_write_cmos_sensor(0x3a0d, 0x04);//	; max 60
		OV5645_write_cmos_sensor(0x3a14, 0x03);//	; max exp 50 = 740
		OV5645_write_cmos_sensor(0x3a15, 0xd8);//	; max exp 50
		OV5645_write_cmos_sensor(0x3c07, 0x07);//	; 50/60 auto detect
		OV5645_write_cmos_sensor(0x3c08, 0x01);//	; 50/60 auto detect
		OV5645_write_cmos_sensor(0x3c09, 0xc2);//	; 50/60 auto detect
		OV5645_write_cmos_sensor(0x4004, 0x02);//	; BLC line number
		OV5645_write_cmos_sensor(0x4005, 0x18);//	; BLC triggered by gain change
		OV5645_write_cmos_sensor(0x4837, 0x11); // MIPI global timing 16           
		OV5645_write_cmos_sensor(0x503d, 0x00);//
		OV5645_write_cmos_sensor(0x5000, 0xa7);//
		OV5645_write_cmos_sensor(0x5001, 0xa3);//
		OV5645_write_cmos_sensor(0x5002, 0x80);//
		OV5645_write_cmos_sensor(0x5003, 0x08);//
		OV5645_write_cmos_sensor(0x3032, 0x00);//
		OV5645_write_cmos_sensor(0x4000, 0x89);//
		OV5645_write_cmos_sensor(0x350c, 0x00);//
		OV5645_write_cmos_sensor(0x350d, 0x00);//
		OV5645SENSORDB("[OV5645]OV5645YUVSetVideoMode exit u2FrameRate == 30 setting  :\n ");
	}
	else if (u2FrameRate == 15)   
	{
		//;OV5645 1280x960,15fps
		//28Mhz, 112Mbps/Lane, 2Lane.
		OV5645SENSORDB("[OV5645]OV5645YUVSetVideoMode enter u2FrameRate == 15 setting  :\n ");
		OV5645_write_cmos_sensor(0x3A00, 0x38); //fix15
		OV5645_write_cmos_sensor(0x300e, 0x45);//	; MIPI 2 lane
		OV5645_write_cmos_sensor(0x3034, 0x18); // PLL, MIPI 8-bit mode
		OV5645_write_cmos_sensor(0x3035, 0x21); // PLL
		OV5645_write_cmos_sensor(0x3036, 0x38); // PLL
		OV5645_write_cmos_sensor(0x3037, 0x13); // PLL
		OV5645_write_cmos_sensor(0x3108, 0x01); // PLL
		OV5645_write_cmos_sensor(0x3824, 0x01); // PLL
		OV5645_write_cmos_sensor(0x460c, 0x20); // PLL
		OV5645_write_cmos_sensor(0x3618, 0x00);//
		OV5645_write_cmos_sensor(0x3600, 0x09);//
		OV5645_write_cmos_sensor(0x3601, 0x43);//
		OV5645_write_cmos_sensor(0x3708, 0x66);//
		OV5645_write_cmos_sensor(0x3709, 0x12);//
		OV5645_write_cmos_sensor(0x370c, 0xc3);//
		OV5645_write_cmos_sensor(0x3800, 0x00); // HS = 0
		OV5645_write_cmos_sensor(0x3801, 0x00); // HS
		OV5645_write_cmos_sensor(0x3802, 0x00); // VS = 250
		OV5645_write_cmos_sensor(0x3803, 0x06); // VS
		OV5645_write_cmos_sensor(0x3804, 0x0a); // HW = 2623
		OV5645_write_cmos_sensor(0x3805, 0x3f);//	; HW
		OV5645_write_cmos_sensor(0x3806, 0x07);//	; VH = 
		OV5645_write_cmos_sensor(0x3807, 0x9d);//	; VH
		OV5645_write_cmos_sensor(0x3808, 0x05);//	; DVPHO = 1280
		OV5645_write_cmos_sensor(0x3809, 0x00);//	; DVPHO
		OV5645_write_cmos_sensor(0x380a, 0x03);//	; DVPVO = 960
		OV5645_write_cmos_sensor(0x380b, 0xc0);//	; DVPVO
		OV5645_write_cmos_sensor(0x380c, 0x07);//	; HTS = 2160
		OV5645_write_cmos_sensor(0x380d, 0x68);//	; HTS
		OV5645_write_cmos_sensor(0x380e, 0x03);//	; VTS = 740
		OV5645_write_cmos_sensor(0x380f, 0xd8);//	; VTS
		OV5645_write_cmos_sensor(0x3810, 0x00); // H OFF = 16
		OV5645_write_cmos_sensor(0x3811, 0x10); // H OFF
		OV5645_write_cmos_sensor(0x3812, 0x00); // V OFF = 4
		OV5645_write_cmos_sensor(0x3813, 0x06);//	; V OFF
		OV5645_write_cmos_sensor(0x3814, 0x31);//	; X INC
		OV5645_write_cmos_sensor(0x3815, 0x31);//	; Y INC
		OV5645_write_cmos_sensor(0x3820, 0x41);//	; flip off, V bin on
		OV5645_write_cmos_sensor(0x3821, 0x07);//	; mirror on, H bin on
		OV5645_write_cmos_sensor(0x4514, 0x00);
		OV5645_write_cmos_sensor(0x3a00, 0x38);//	; ae mode	
		OV5645_write_cmos_sensor(0x3a02, 0x03);//	; max exp 60 = 740
		OV5645_write_cmos_sensor(0x3a03, 0xd8);//	; max exp 60
		OV5645_write_cmos_sensor(0x3a08, 0x00);//	; B50 = 222
		OV5645_write_cmos_sensor(0x3a09, 0x94);//	; B50
		OV5645_write_cmos_sensor(0x3a0a, 0x00);//	; B60 = 185
		OV5645_write_cmos_sensor(0x3a0b, 0x7b);//	; B60
		OV5645_write_cmos_sensor(0x3a0e, 0x06);//	; max 50
		OV5645_write_cmos_sensor(0x3a0d, 0x07);//	; max 60
		OV5645_write_cmos_sensor(0x3a14, 0x03);//	; max exp 50 = 740
		OV5645_write_cmos_sensor(0x3a15, 0xd8);//	; max exp 50
		OV5645_write_cmos_sensor(0x3c07, 0x08);//	; 50/60 auto detect
		OV5645_write_cmos_sensor(0x3c08, 0x00);//	; 50/60 auto detect
		OV5645_write_cmos_sensor(0x3c09, 0x1c);//	; 50/60 auto detect
		OV5645_write_cmos_sensor(0x4004, 0x02);//	; BLC line number
		OV5645_write_cmos_sensor(0x4005, 0x18);//	; BLC triggered by gain change
		OV5645_write_cmos_sensor(0x4837, 0x11); // MIPI global timing 16           
		OV5645_write_cmos_sensor(0x503d, 0x00);//
		OV5645_write_cmos_sensor(0x5000, 0xa7);//
		OV5645_write_cmos_sensor(0x5001, 0xa3);//
		OV5645_write_cmos_sensor(0x5002, 0x80);//
		OV5645_write_cmos_sensor(0x5003, 0x08);//
		OV5645_write_cmos_sensor(0x3032, 0x00);//
		OV5645_write_cmos_sensor(0x4000, 0x89);//
		OV5645_write_cmos_sensor(0x350c, 0x00);//
		OV5645_write_cmos_sensor(0x350d, 0x00);//
		OV5645_write_cmos_sensor(0x3A00, 0x38); //fix30
		OV5645SENSORDB("[OV5645]OV5645YUVSetVideoMode exit u2FrameRate == 15 setting  :\n ");
	}   
	else 
	{
		printk("Wrong frame rate setting \n");
	} 
	OV5645SENSORDB("[OV5645]exit OV5645YUVSetVideoMode function:\n ");
	return TRUE; 
}
/**************************/
void OV5645GetEvAwbRef(UINT32 pSensorAEAWBRefStruct)
{
	PSENSOR_AE_AWB_REF_STRUCT Ref = (PSENSOR_AE_AWB_REF_STRUCT)pSensorAEAWBRefStruct;
    OV5645SENSORDB("enter:\n ");
    Ref->SensorAERef.AeRefLV05Shutter=0x170c;
    Ref->SensorAERef.AeRefLV05Gain=0x30;
    Ref->SensorAERef.AeRefLV13Shutter=0x24e;
    Ref->SensorAERef.AeRefLV13Gain=0x10;
    Ref->SensorAwbGainRef.AwbRefD65Rgain=0x610;
    Ref->SensorAwbGainRef.AwbRefD65Bgain=0x448;
    Ref->SensorAwbGainRef.AwbRefCWFRgain=0x4e0;
    Ref->SensorAwbGainRef.AwbRefCWFBgain=0x5a0;
}

void OV5645GetCurAeAwbInfo(UINT32 pSensorAEAWBCurStruct)
{
	PSENSOR_AE_AWB_CUR_STRUCT Info = (PSENSOR_AE_AWB_CUR_STRUCT)pSensorAEAWBCurStruct;
    OV5645SENSORDB("enter:\n ");
	Info->SensorAECur.AeCurShutter=OV5645ReadShutter();
    Info->SensorAECur.AeCurGain=OV5645ReadSensorGain() ;
    Info->SensorAwbGainCur.AwbCurRgain=((OV5645_read_cmos_sensor(0x3401)&&0xff)+((OV5645_read_cmos_sensor(0x3400)&&0xff)*256));
    Info->SensorAwbGainCur.AwbCurBgain=((OV5645_read_cmos_sensor(0x3405)&&0xff)+((OV5645_read_cmos_sensor(0x3404)&&0xff)*256));
}

UINT32 OV5645GetDefaultFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 *pframeRate)
{
    OV5645SENSORDB("enter:\n ");
    switch (scenarioId) {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
        *pframeRate = 300;
        break;
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
    case MSDK_SCENARIO_ID_CAMERA_ZSD:
        *pframeRate = 150;
        break;
    case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: //added
    case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
    case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: //added
        *pframeRate = 300;
        break;
    default:
        break;
    }
    return ERROR_NONE;
}
void OV5645_get_AEAWB_lock(UINT32 *pAElockRet32, UINT32 *pAWBlockRet32)
{
   // OV5645SENSORDB("[OV5645]enter OV5645_get_AEAWB_lock function:\n ");
    *pAElockRet32 =1;
    *pAWBlockRet32=1;
    //OV5645SENSORDB("[OV5645]OV5645_get_AEAWB_lock,AE=%d,AWB=%d\n",*pAElockRet32,*pAWBlockRet32);
    //OV5645SENSORDB("[OV5645]exit OV5645_get_AEAWB_lock function:\n ");
}
void OV5645_GetDelayInfo(UINT32 delayAddr)
{
    SENSOR_DELAY_INFO_STRUCT *pDelayInfo=(SENSOR_DELAY_INFO_STRUCT*)delayAddr;
    pDelayInfo->InitDelay=0;
    pDelayInfo->EffectDelay=0;
    pDelayInfo->AwbDelay=0;
    pDelayInfo->AFSwitchDelayFrame=50;
}
void OV5645_3ACtrl(ACDK_SENSOR_3A_LOCK_ENUM action)
{
//    OV5645SENSORDB("enter action=%d\n",action);
    switch (action)
    {
    case SENSOR_3A_AE_LOCK:
        spin_lock(&ov5645_drv_lock);
        OV5645MIPISensor.userAskAeLock = TRUE;
        spin_unlock(&ov5645_drv_lock);
        OV5645_set_AE_mode(KAL_FALSE);
        break;
    case SENSOR_3A_AE_UNLOCK:
        spin_lock(&ov5645_drv_lock);
        OV5645MIPISensor.userAskAeLock = FALSE;
        spin_unlock(&ov5645_drv_lock);
        OV5645_set_AE_mode(KAL_TRUE);
        break;

    case SENSOR_3A_AWB_LOCK:
        spin_lock(&ov5645_drv_lock);
        OV5645MIPISensor.userAskAwbLock = TRUE;
        spin_unlock(&ov5645_drv_lock);
        OV5645_set_AWB_mode(KAL_FALSE);
        break;

    case SENSOR_3A_AWB_UNLOCK:
        spin_lock(&ov5645_drv_lock);
        OV5645MIPISensor.userAskAwbLock = FALSE;
        spin_unlock(&ov5645_drv_lock);
        OV5645_set_AWB_mode_UNLOCK();
        break;
    default:
        break;
    }
    return;
}
#define FLASH_BV_THRESHOLD 0x25
void OV5645_FlashTriggerCheck(unsigned int *pFeatureReturnPara32)
{
    unsigned int NormBr;
    NormBr = OV5645_read_cmos_sensor(0x56A1);
    if (NormBr > FLASH_BV_THRESHOLD)
    {
       *pFeatureReturnPara32 = FALSE;
        return;
    }
    *pFeatureReturnPara32 = TRUE;
    return;
}

#define RIGHT_LIMIT_PREVIEW 960   //upper_right_position preview
#define LOWER_LIMIT_PREVIEW 470   //lower_right_position preview
#define RIGHT_LIMIT_CAPTURE 1920  //upper_right_position capture
#define LOWER_LIMIT_CAPTURE 940   //lower_right_position capture
#define START_OFFSET_PREVIEW 1813 //upper_left_position preview
#define START_OFFSET_CAPTURE 1308 //upper_left_position capture

//**************** defined position (x,y)
//**************** START_OFFSET + X + Y* HTS
void update_pip_motion_position(unsigned int StartX, unsigned int StartY, OV5645_SENSOR_MODE MODE)
{
	unsigned long POSITION=0,temp;	
	unsigned long HTS;	

	HTS=OV5645_read_cmos_sensor(0x380c)<<8 | OV5645_read_cmos_sensor(0x380d);
	if ( OV5645MIPISensor.zsd_flag && MODE == SENSOR_MODE_PREVIEW ){
		MODE = SENSOR_MODE_CAPTURE;
	}
	
	if(MODE == SENSOR_MODE_PREVIEW
		||MODE == SENSOR_MODE_VIDEO)
	{
		if( StartX >  RIGHT_LIMIT_PREVIEW) StartX = RIGHT_LIMIT_PREVIEW;
		else if( StartY > LOWER_LIMIT_PREVIEW) StartY = LOWER_LIMIT_PREVIEW;
		POSITION = START_OFFSET_PREVIEW + StartX + StartY*HTS;	
	}
	else if(MODE == SENSOR_MODE_CAPTURE)
	{
		#define CAPTURE_OFFSET_X (20)
		
		unsigned int temp_cord;
		StartY *= 2;
		
		temp_cord = StartX * 2;
		if ( temp_cord < CAPTURE_OFFSET_X )
			temp_cord=CAPTURE_OFFSET_X;
		StartX = temp_cord-CAPTURE_OFFSET_X;
		
		if( StartX >  RIGHT_LIMIT_CAPTURE) StartX = RIGHT_LIMIT_CAPTURE;
		else if( StartY > LOWER_LIMIT_CAPTURE) StartY = LOWER_LIMIT_CAPTURE;
		POSITION = START_OFFSET_CAPTURE + StartX + StartY*HTS;	
	}else{
		OV5645SENSORDB("Set PIP mode in wrong mode(%d).\n", MODE);
		return;
	}

//	OV5645SENSORDB("MODE=%d, POSITION=%d StartX=%d, StartY=%d\n", MODE, POSITION, StartX, StartY);
	
	OV5645_write_cmos_sensor(0x4202,0x0F);
	
	OV5645_write_cmos_sensor(0x3212, 0x00); 
	OV5645_write_cmos_sensor(0x3003, 0x01);

	POSITION = POSITION & 0xffffff;

	temp = POSITION & 0xff;
	OV5645_write_cmos_sensor(0x4316, temp);
	temp = POSITION & 0xffff;
	temp = temp>>8;
	OV5645_write_cmos_sensor(0x4315, temp);
	temp = POSITION>>16;
	OV5645_write_cmos_sensor(0x4314, temp);

	OV5645_write_cmos_sensor(0x3212, 0x10);  
	OV5645_write_cmos_sensor(0x3212, 0xa0);  
	if(MODE == SENSOR_MODE_CAPTURE){
		mDELAY(200);
	}else{
		mDELAY(90);//90
	}
	OV5645_write_cmos_sensor(0x3212, 0x00);  
	OV5645_write_cmos_sensor(0x3003, 0x00);  
	OV5645_write_cmos_sensor(0x3212, 0x10);  
	OV5645_write_cmos_sensor(0x3212, 0xa0);  
	
	OV5645_write_cmos_sensor(0x4202,0x00);
}

/*
Y'= 0.299*R' + 0.587*G' + 0.114*B'
U'= -0.147*R' - 0.289*G' + 0.436*B' = 0.492*(B'- Y')
V'= 0.615*R' - 0.515*G' - 0.100*B' = 0.877*(R'- Y')
*/

void OV5645_PIP_Set_Frame_Color(kal_uint8  uR, kal_uint8 uG, kal_uint8 uB)
{
	kal_int32  iY, iU, iV;
	kal_int32 iR=uR, iG=uG, iB=uB;
	iY = (299*iR + 587*iG + 114*iB)/1000;
	iU = 492*(iB-iY)/1000;
	iV = 877*(iR-iY)/1000;
	OV5645_write_cmos_sensor(0x5b04 ,iY);
	OV5645_write_cmos_sensor(0x5b05 ,iU);
	OV5645_write_cmos_sensor(0x5b06 ,iV);
}

extern void OV7695PIPYUV_save_awb(void);
void OV5645_PIP_Set_Window(UINT32 zone_addr)
{
	UINT32 FD_XS;
	UINT32 FD_YS;   
	UINT32 x0, y0, x1, y1;
	UINT32 pvx0, pvy0, pvx1, pvy1;
	UINT32 AF_pvx, AF_pvy;
	UINT32* zone = (UINT32*)zone_addr;
	x0 = *zone;
	y0 = *(zone + 1);
	x1 = *(zone + 2);
	y1 = *(zone + 3);   
	FD_XS = *(zone + 4);
	FD_YS = *(zone + 5);

	if ( 0 == FD_XS || 0 == FD_YS ){
		OV5645SENSORDB("exit function--ERROR PARAM:\n ");
		return;
	}

	memcpy((void *)&OV5645MIPISensor.zone_addr, zone, sizeof(OV5645MIPISensor.zone_addr));

	x0 = FD_XS - x0 - 160;//160=240/1.5, 1.5=720/480
	mapMiddlewaresizePointToPreviewsizePoint(y0,x0,FD_YS,FD_XS, &pvx0, &pvy0, PRV_W, PRV_H);  
	AF_pvx =pvx0+15;
	AF_pvy =pvy0-15;
	update_pip_motion_position(AF_pvx ,AF_pvy, OV5645MIPISensor.SensorMode);
//	OV7695PIPYUV_save_awb();
}

