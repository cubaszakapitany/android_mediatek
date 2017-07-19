#ifdef BUILD_LK
#else
    #include <linux/string.h>
    #if defined(BUILD_UBOOT)
        #include <asm/arch/mt_gpio.h>
    #else
        #include <mach/mt_gpio.h>
    #endif
#endif
#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#if defined(BUILD_LK)
	#define LCM_DBG printf
#else
	#define LCM_DBG printk
#endif


#define FRAME_WIDTH  (480)
#define FRAME_HEIGHT (800)
#define FALSE (0)
#define TRUE (1)

#define ESD_CHECK_SWITCH   (0)
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))
#define LCD_ILI9806E_CMD(v)       (lcm_util.send_cmd((v)))
#define LCD_ILI9806E_INDEX(v)    (lcm_util.send_data((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))
#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0x00   // END OF REGISTERS MARKER


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)									lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)				lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)      

//#define LCM_DSI_CMD_MODE
struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};


#define LCM_INIT_TYPE (1)

#if LCM_INIT_TYPE
static void init_lcm_registers(void)
{
    unsigned int data_array[16];

    //*************Enable CMD2 Page1  *******************//
    data_array[0]=0x00063902;
    data_array[1]=0x0698ffff;
    data_array[2]=0x00000104;
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00001008;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000121;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000230;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000231;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00001a40;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00006641;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000242;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000943;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00008644;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00007850;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00007851;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00005052;
    dsi_set_cmdq(data_array, 2, 1);  

    data_array[0]=0x00023902;
    data_array[1]=0x00005053;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000760;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000461;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000662;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000263;
    dsi_set_cmdq(data_array, 2, 1);

    //*************gamma  *******************//
    data_array[0]=0x00063902;
    data_array[1]=0x0698ffff;
    data_array[2]=0x00000104;
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x000000A0;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000AA1;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x000009A2;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x000017A3;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x000007A4;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00001DA5;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000AA6;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000BA7;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x000003A8;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000EA9;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x000005AA;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x000001AB;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x000014AC;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x000030AD;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x000033AE;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x000000AF;
    dsi_set_cmdq(data_array, 2, 1);

    //*************gamma  NAGITIVE *******************//

    data_array[0]=0x00023902;
    data_array[1]=0x000000C0;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000CC1;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x000025C2;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000FC3;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000FC4;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x000014C5;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x000007C6;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x000003C7;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x000005C8;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x000003C9;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x000007CA;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x000006CB;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x000005CC;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00001FCD;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x000016CE;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x000000CF;
    dsi_set_cmdq(data_array, 2, 1);

    //*************PAGE 6  *******************//
    data_array[0]=0x00063902;
    data_array[1]=0x0698ffff;
    data_array[2]=0x00000604;
    dsi_set_cmdq(data_array, 3, 1);


    data_array[0]=0x00023902;
    data_array[1]=0x00002000;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000601;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00002002;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000203;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000104;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000105;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00009806;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000407;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000508;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000009;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000000A;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000000B;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000010C;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000010D;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000000E;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000000F;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000FF10;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000F011;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000012;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000013;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000014;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000C015;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000816;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000017;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000018;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000019;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000001A;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000001B;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000001C;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000001D;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000120;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00002321;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00004522;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00006723;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000124;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00002325;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00004526;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00006727;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00001230;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00002231;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00002232;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00002233;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00008734;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00009635;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000BA36;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000AB37;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000DC38;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000CD39;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000783A;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000693B;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000223C;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000223D;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000223E;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x0000223F;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00002240;
    dsi_set_cmdq(data_array, 2, 1);

    //*************PAGE 7 *******************//
    data_array[0]=0x00063902;
    data_array[1]=0x0698ffff;
    data_array[2]=0x00000704;
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00001D18;
    dsi_set_cmdq(data_array, 2, 1);
	
    data_array[0]=0x00023902;
    data_array[1]=0x00001217;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00002240;
    dsi_set_cmdq(data_array, 2, 1);

    //*************PAGE 0 *******************//

    data_array[0]=0x00063902;
    data_array[1]=0x0698ffff;
    data_array[2]=0x00000004;
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00110500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(150);
    
    data_array[0]= 0x00290500;
    dsi_set_cmdq(data_array, 1, 1);
    
}
#endif


#if 1
static struct LCM_setting_table lcm_initialization_setting[] = {
/*{addr, length, {data}} */

{0xFF,5,{0xff,0x98,0x06,0x04,0x01}},
{0x08,1,{0x10}},  
{0x21,1,{0x01}},
{0x30,1,{0x02}},
{0x31,1,{0x02}},
{0x40,1,{0x16}},
{0x41,1,{0x33}},
{0x42,1,{0x1}},
{0x43,1,{0x9}},
{0x44,1,{0x86}},
{0x50,1,{0x78}},
{0x51,1,{0x78}},
{0x52,1,{0x0}},
{0x53,1,{0x40}},
{0x60,1,{0x7}},
{0x61,1,{0x4}},
{0x62,1,{0x6}},
{0x63,1,{0x2}},

//++++++++++++++++++ Gamma Setting ++++++++++++++++++//
{0xff,5,{0xff,0x98,0x6,0x4,0x1}},
{0xa0,1,{0x0}},
{0xa1,1,{0x9}},
{0xa2,1,{0x1b}},
{0xa3,1,{0x12}},
{0xa4,1,{0x9}},
{0xa5,1,{0x1a}},
{0xa6,1,{0x4}},
{0xa7,1,{0x7}},
{0xa8,1,{0x7}},
{0xa9,1,{0x8}},
{0xaa,1,{0x7}},
{0xab,1,{0x2}},
{0xac,1,{0xa}},
{0xad,1,{0x32}},
{0xae,1,{0x2b}},
{0xaf,1,{0x00}},

///==============Nagitive
{0xc0,1,{0x00}},
{0xc1,1,{0x6}},
{0xc2,1,{0x18}},
{0xc3,1,{0x17}},
{0xc4,1,{0x10}},
{0xc5,1,{0x1c}},
{0xc6,1,{0xa}},
{0xc7,1,{0x5}},
{0xc8,1,{0x4}},
{0xc9,1,{0xb}},
{0xca,1,{0x4}},
{0xcb,1,{0x1a}},
{0xcc,1,{0x16}},
{0xcd,1,{0x1a}},
{0xce,1,{0x16}},
{0xcf,1,{0x00}},

//****************************************************************************//
//****************************** Page 6 Command ******************************//
//****************************************************************************//
{0xff,5,{0xff,0x98,0x6,0x4,0x6}},
{0x0,1,{0x20}},
{0x1,1,{0x06}},
{0x2,1,{0x20}},
{0x3,1,{0x2}},
{0x4,1,{0x1}},
{0x5,1,{0x1}},
{0x6,1,{0x98}},
{0x7,1,{0x4}},
{0x8,1,{0x5}},
{0x9,1,{0x0}},
{0xa,1,{0x0}},
{0xb,1,{0x0}},
{0xc,1,{0x1}},
{0xd,1,{0x1}},
{0xe,1,{0x0}},
{0xf,1,{0x0}},
{0x10,1,{0xff}},
{0x11,1,{0xf0}},
{0x12,1,{0x0}},
{0x13,1,{0x0}},
{0x14,1,{0x0}},
{0x15,1,{0xc0}},
{0x16,1,{0x08}},
{0x17,1,{0x0}},
{0x18,1,{0x0}},
{0x19,1,{0x0}},
{0x1a,1,{0x0}},
{0x1b,1,{0x0}},
{0x1c,1,{0x0}},
{0x1d,1,{0x0}},
{0x20,1,{0x01}},
{0x21,1,{0x23}},
{0x22,1,{0x45}},
{0x23,1,{0x67}},
{0x24,1,{0x01}},
{0x25,1,{0x23}},
{0x26,1,{0x45}},
{0x27,1,{0x67}},
{0x30,1,{0x12}},
{0x31,1,{0x22}},
{0x32,1,{0x22}},
{0x33,1,{0x22}},
{0x34,1,{0x87}},
{0x35,1,{0x96}},
{0x36,1,{0xba}},
{0x37,1,{0xab}},
{0x38,1,{0xdc}},
{0x39,1,{0xcd}},
{0x3a,1,{0x78}},
{0x3b,1,{0x69}},
{0x3c,1,{0x22}},
{0x3d,1,{0x22}},
{0x3e,1,{0x22}},
{0x3f,1,{0x22}},
{0x40,1,{0x22}},

//****************************************************************************//
//****************************** Page 7 Command ******************************//
//****************************************************************************//
{0xff,5,{0xff,0x98,0x6,0x4,0x7}},
{0x18,1,{0x1d}},
{0x17,1,{0x12}},

//****************************************************************************//
{0xff,5,{0xff,0x98,0x6,0x4,0x0}},
{0x11,1,{0x0}},
{0x29,1,{0x0}},

};
#endif


static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 1, {0x00}},
    {REGFLAG_DELAY, 130, {}},

    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},
    {REGFLAG_DELAY, 10, {}},
    // Sleep Mode On
	{0x10, 1, {0x00}},
    {REGFLAG_DELAY, 130, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_backlight_level_setting[] = {
	{0x51, 1, {0xFF}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_compare_id_setting[] = {
	// Display off sequence
	{0xB9,	3,	{0xFF, 0x83, 0x69}},
	{REGFLAG_DELAY, 10, {}},

        // Sleep Mode On
        // {0xC3, 1, {0xFF}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};



void push_table_tkc(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
				MDELAY(10);
       	}
    }
	
}


static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
#if 1
    memset(params, 0, sizeof(LCM_PARAMS));
    
    params->type   = LCM_TYPE_DSI;
    
    params->width  = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;
    
    // enable tearing-free
    params->dbi.te_mode				= LCM_DBI_TE_MODE_VSYNC_ONLY;
    params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;
    
    params->dsi.mode   = SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE;
    
    // DSI
    /* Command mode setting */
    params->dsi.LANE_NUM				= LCM_TWO_LANE;
    
    //The following defined the fomat for data coming from LCD engine.
    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dsi.data_format.trans_seq	= LCM_DSI_TRANS_SEQ_MSB_FIRST;
    params->dsi.data_format.padding 	= LCM_DSI_PADDING_ON_LSB;
    params->dsi.data_format.format	  = LCM_DSI_FORMAT_RGB888;
    
    // Video mode setting		
    params->dsi.intermediat_buffer_num = 2;
    
    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
    
    params->dsi.word_count=480*3;	//DSI CMD mode need set these two bellow params, different to 6577
    params->dsi.vertical_active_line=800;

    params->dsi.vertical_sync_active				= 4;
    params->dsi.vertical_backporch					= 16;
    params->dsi.vertical_frontporch					= 20;	
    params->dsi.vertical_active_line				= FRAME_HEIGHT;
    
    params->dsi.horizontal_sync_active				= 10;//10;
    params->dsi.horizontal_backporch				= 40;//50;
    params->dsi.horizontal_frontporch				= 70;//50;
   // params->dsi.horizontal_blanking_pixel				= 60;
    params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
   //params->dsi.compatibility_for_nvk = 0;		// this parameter would be set to 1 if DriverIC is NTK's and when force match DSI clock for NTK's

    // Bit rate calculation
//    params->dsi.pll_div1=1;		// div1=0,1,2,3;div1_real=1,2,4,4
//    params->dsi.pll_div2=0;		// div2=0,1,2,3;div2_real=1,2,4,4
//    params->dsi.fbk_div =16;		// fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)		

//mipi clock 控制改成只需要一個參數
    params->dsi.PLL_CLOCK = 208; //dsi clock customization: should config clock value directly
 
//(建議值請使用 26 的倍數 ex: 208, 234 因為基頻是 26MHz)
//而展頻變成可動態控制  展頻幅度也是可以動態調
    params->dsi.ssc_disable = 0;  // ssc disable control (1: disable, 0: enable, default: 0)
    params->dsi.ssc_range = 8;  // ssc range control (1:min, 8:max, default: 4)


#else
		memset(params, 0, sizeof(LCM_PARAMS));

		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

		// enable tearing-free
		params->dsi.lcm_int_te_monitor       = FALSE;
		params->dsi.lcm_int_te_period         = 1; // Unit : frames
		params->dsi.lcm_ext_te_monitor      = FALSE;//TRUE; //FALSE;
		params->dsi.noncont_clock              =TRUE;
		params->dsi.noncont_clock_period    =2;

		params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if defined(LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		params->dsi.mode   = SYNC_PULSE_VDO_MODE;
#endif
	
		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_TWO_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size=256;

		// Video mode setting		
		params->dsi.intermediat_buffer_num = 2;

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

		params->dsi.vertical_sync_active				= 8; // 3 2 18      4 6 6
		params->dsi.vertical_backporch					= 12; // 7 6 24
		params->dsi.vertical_frontporch					= 19; // 6, 6 24
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 32; // 12 12 32 32
		params->dsi.horizontal_backporch				= 60; //132 92 52 60
		params->dsi.horizontal_frontporch				= 60; //132 92 60 60
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		// Bit rate calculation
		params->dsi.pll_div1=1;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
		params->dsi.pll_div2=1; 		// div2=0~15: fout=fvo/(2*div2)
             params->dsi.fbk_div=29; //30,27 24 26;          // 21(default) 22 henxian 23 baiping
#endif
}

static void lcm_reset(void)
{
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(128);
}

static void lcm_init(void)
{
	LCM_DBG("###ILI9806E_WVGA_DSI_TKC: %s, %d\n", __FUNCTION__,__LINE__);

	lcm_reset();
#if LCM_INIT_TYPE
	init_lcm_registers();
#else
	push_table_tkc(
		lcm_initialization_setting, 
		sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
#endif
}

extern int disp_bls_set_backlight(unsigned int level);
static void lcm_suspend(void)
{
LCM_DBG("###ILI9806E_WVGA_DSI_TKC: %s, %d\n", __FUNCTION__,__LINE__);

#if LCM_INIT_TYPE
    unsigned int data_array[16];

#if 1
    data_array[0]=0x00063902; 
    data_array[1]=0x0698ffff;
    data_array[2]=0x00000004;
    dsi_set_cmdq(data_array, 3, 1);
 
    data_array[0] = 0x00280500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(50);
    data_array[0] = 0x00100500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(20);
 
    data_array[0]=0x00063902; 
    data_array[1]=0x0698ffff;
    data_array[2]=0x00000804;
    dsi_set_cmdq(data_array, 3, 1);
#else
    disp_bls_set_backlight(0);
    data_array[0] = 0x00280500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(20);
    data_array[0] = 0x00100500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(20);
    SET_RESET_PIN(0);  
#endif
#else
	lcm_reset();
	push_table_tkc(
		lcm_deep_sleep_mode_in_setting, 
		sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
#endif
}

static void lcm_resume(void)
{
	LCM_DBG("###ILI9806E_WVGA_DSI_TKC: %s, %d\n", __FUNCTION__,__LINE__);
	lcm_init();
}

static unsigned int lcm_esd_check(void)
{

//return 0;

#ifndef BUILD_LK
	unsigned char buffer[2];
	int array[4];

	unsigned int data_array[16];
	data_array[0]=0x00063902;
	data_array[1]=0x0698ffff;
	data_array[2]=0x00000004;
	dsi_set_cmdq(data_array, 3, 1);

	array[0]=0x00043700;//0x00023700;
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0x0A, buffer, 4);

	LCM_DBG("9806e tkc lcm_esd_check:%x\n",buffer[0]);

	data_array[0]=0x00063902; 
	data_array[1]=0x0698ffff;
	data_array[2]=0x00000804;
	dsi_set_cmdq(data_array, 3, 1);
	
	if((buffer[0] == 0x9C) ||(buffer[0] == 0x0))
	{
	    return FALSE;
	}

	LCM_DBG(" ####ili9806e_tkc-lcm_esd_check:%x\n",buffer[0]);
	return TRUE;
	
#endif
}


static unsigned int lcm_esd_recover(void)
{
	LCM_DBG("###ILI9806E_WVGA_DSI_TKC: %s, %d\n", __FUNCTION__,__LINE__);  
	lcm_init();
	return TRUE;
}

// ---------------------------------------------------------------------------
//  Get LCM ID Information
// ---------------------------------------------------------------------------
extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);
static unsigned int lcm_compare_id()
{
        unsigned int id = 0;
        unsigned char buffer[2];
        unsigned int array[16];

        int Channel=1;
        int data[4];
        int adcVol=0;
        
        int res=IMM_GetOneChannelValue(Channel,data,0);
        adcVol=data[0]*1000+data[1]*10;
        

	LCM_DBG("###ILI9806E_WVGA_DSI_TKC: LINE=%d %s, res=%d,adcVol = %d \n", __LINE__,__func__,res,adcVol);
		
       if(adcVol>=1300) //tkc adcVol is 1380.
       {
		LCM_DBG("###ILI9806E_WVGA_DSI_TKC: Succecss!");
		return 1;
       }
       else
       {
		LCM_DBG("###ILI9806E_WVGA_DSI_TKC: Fail!");
		return 0;
       }
}


LCM_DRIVER ili9806e_wvga_dsi_vdo_tkc_drv = 
{
         .name = "ili9806e_wvga_dsi_vdo_tkc_drv",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
#if ESD_CHECK_SWITCH
	.esd_check   = lcm_esd_check,
	.esd_recover   = lcm_esd_recover,
#endif
	.compare_id     = lcm_compare_id,
    };

