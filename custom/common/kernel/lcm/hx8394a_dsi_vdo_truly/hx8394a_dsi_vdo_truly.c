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

#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	#include <mach/mt_gpio.h>
#endif

#ifdef BUILD_LK
#define LCM_PRINT printf
#else
#if defined(BUILD_UBOOT)
#define LCM_PRINT printf
#else
#define LCM_PRINT printk
#endif
#endif

#define LCM_DBG(fmt, arg...) \
	LCM_PRINT ("[LCM-HX8394A-DSI-VDO] %s (line:%d) :" fmt "\r\n", __func__, __LINE__, ## arg)


// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (720)
#define FRAME_HEIGHT (1280)

#define LCM_ID_HX8394 (0x94)
#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xDD   // END OF REGISTERS MARKER

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

static unsigned int lcm_esd_test = FALSE;      ///only for ESD test

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)   

#define   LCM_DSI_CMD_MODE							0

struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[255];
};
#if 1
static struct LCM_setting_table lcm_initialization_setting[] = {
	{0xB9,3,{0xFF,0x83,0x94}},
    //{0xBA,11,{0x13,0x82,0x00,0x16,0xC5,0x00,0x10,0xFF,0x0F,0x24,0x05}},    //set mipi four lanes
	//{0x11,0,{}},
	//{REGFLAG_DELAY, 200, {}},
    {0xB1,16,{0x01,0x00,0x47,/*0x89*/0x87,0x01,0x11,0x11,0x2A,0x30,0x3F,0x3F,0x47,0x02,0x01,0xE6,0xE2}},
    {0xB4,22,{0x80,0x06,0x32,0x10,0x03,0x32,0x15,0x08,0x32,0x10,0x08,0x33,0x04,0x43,0x05,0x37,
                     0x04,0x3F,0x06,0x61,0x61,0x06}},
    {0xB2,6,{0x00,0xC8,0x08,0x04,0x00,0x22}},
    {0xD5,32,{0x00,0x00,0x00,0x00,0x0A,0x00,0x01,0x00,0xCC,0x00,0x00,0x00,0x88,0x88,0x88,0x88,
                     0x88,0x88,0x88,0x88,0x88,0x88,0x01,0x67,0x45,0x23,0x01,0x23,0x88,0x88,0x88,0x88}},
    {0xC7,4,{0x00,0x10,0x00,0x10}},                 
    {0xBF,4,{0x06,0x00,0x10,0x04}},
    {0xCC,1,{0x09}},
    {0xE0,42,{0x00,0x04,0x06,0x2B,0x33,0x3F,0x11,0x34,0x0A,0x0E,0x0D,0x11,0x13,0x11,0x13,0x10,
                     0x17,0x00,0x04,0x06,0x2B,0x33,0x3F,0x11,0x34,0x0A,0x0E,0x0D,0x11,0x13,0x11,0x13,
                     0x10,0x17,0x0B,0x17,0x07,0x11,0x0B,0x17,0x07,0x11}},
    {0xC1,127,{0x01,0x00,0x07,0x0E,0x15,0x1D,0x25,0x2D,0x34,0x3C,0x42,0x49,0x51,0x58,0x5F,0x67,0x6F,0x77,0x80,0x87,0x8F,0x98,0x9F,0xA7,0xAF,0xB7,0xC1,0xCB,0xD3,0xDD,0xE6,0xEF,0xF6,0xFF,0x16,0x25,0x7C,0x62,0xCA,0x3A,0xC2,0x1F,0xC0
		      ,0x00,0x07,0x0E,0x15,0x1D,0x25,0x2D,0x34,0x3C,0x42,0x49,0x51,0x58,0x5F,0x67,0x6F,0x77,0x80,0x87,0x8F,0x98,0x9F,0xA7,0xAF,0xB7,0xC1,0xCB,0xD3,0xDD,0xE6,0xEF,0xF6,0xFF,0x16,0x25,0x7C,0x62,0xCA,0x3A,0xC2,0x1F,0xC0
		      ,0x00,0x07,0x0E,0x15,0x1D,0x25,0x2D,0x34,0x3C,0x42,0x49,0x51,0x58,0x5F,0x67,0x6F,0x77,0x80,0x87,0x8F,0x98,0x9F,0xA7,0xAF,0xB7,0xC1,0xCB,0xD3,0xDD,0xE6,0xEF,0xF6,0xFF,0x16,0x25,0x7C,0x62,0xCA,0x3A,0xC2,0x1F,0xC0}},
    //{0xB6,1,{0x0C}},
    {0xB6,1,{0x02}},
    {0xD4,1,{0x32}},
    {0x11,0,{}},
    {REGFLAG_DELAY, 200, {}},
	{0xBA,11,{0x13,0x82,0x00,0x16,0xC5,0x00,0x10,0xFF,0x0F,0x28,0x02}},
    {0x29,0,{}},
    {REGFLAG_DELAY, 50, {}},
    // Note
	// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.

	// Setting ending by predefined flag
    {REGFLAG_END_OF_TABLE, 0x00, {}}

};

#endif
#if 0
static struct LCM_setting_table lcm_initialization_setting[] = {
	{0xB9,3,{0xFF,0x83,0x94}},
    //{0xBA,16,{0x13,0x82,0x00,0x16,0xC5,0x00,0x10,0xFF,0x0F,0x24,0x03,0x21,0x24,0x25,0x20,0x08}},    //set mipi four lanes
	//{0x11,0,{}},
	//{REGFLAG_DELAY, 200, {}},
    {0xB1,16,{0x01,0x00,0x04,0x89,0x01,0x11,0x11,0x2A,0x34,0x3F,0x3F,0x47,0x12,0x01,0xE6,0xE6}},
    {0xB4,22,{0x80,0x06,0x32,0x10,0x03,0x32,0x15,0x08,0x32,0x10,0x08,0x33,0x04,0x43,0x05,0x37,
                     0x04,0x43,0x06,0x61,0x61,0x06}},
    {0xB2,6,{0x00,0xC8,0x08,0x04,0x00,0x22}},
    {0xD5,32,{0x00,0x00,0x00,0x00,0x0A,0x00,0x01,0x00,0xCC,0x00,0x00,0x00,0x88,0x88,0x88,0x88,
                     0x88,0x88,0x88,0x88,0x88,0x88,0x01,0x67,0x45,0x23,0x01,0x23,0x88,0x88,0x88,0x88}},
    {0xC7,4,{0x00,0x10,0x00,0x10}},                 
    {0xBF,4,{0x06,0x00,0x10,0x04}},
    {0xCC,1,{0x09}},
    {0xE0,42,{0x00,0x04,0x06,0x2B,0x33,0x3F,0x11,0x34,0x0A,0x0E,0x0D,0x11,0x13,0x11,0x13,0x10,
                     0x17,0x00,0x04,0x06,0x2B,0x33,0x3F,0x11,0x34,0x0A,0x0E,0x0D,0x11,0x13,0x11,0x13,
                     0x10,0x17,0x0B,0x17,0x07,0x11,0x0B,0x17,0x07,0x11}},
    {0xC1,127,   {0x01,0x00,0x07,0x0E,0x15,0x1D,0x25,0x2D,0x34,0x3C,0x42,0x49,0x51,0x58,0x5F,0x67,0x6F,0x77,0x80,0x87,0x8F,0x98,0x9F,0xA7,0xAF,0xB7,0xC1,0xCB,0xD3,0xDD,0xE6,0xEF,0xF6,0xFF,0x16,0x25,0x7C,0x62,0xCA,0x3A,0xC2,0x1F,0xC0
		      ,0x00,0x07,0x0E,0x15,0x1D,0x25,0x2D,0x34,0x3C,0x42,0x49,0x51,0x58,0x5F,0x67,0x6F,0x77,0x80,0x87,0x8F,0x98,0x9F,0xA7,0xAF,0xB7,0xC1,0xCB,0xD3,0xDD,0xE6,0xEF,0xF6,0xFF,0x16,0x25,0x7C,0x62,0xCA,0x3A,0xC2,0x1F,0xC0
		      ,0x00,0x07,0x0E,0x15,0x1D,0x25,0x2D,0x34,0x3C,0x42,0x49,0x51,0x58,0x5F,0x67,0x6F,0x77,0x80,0x87,0x8F,0x98,0x9F,0xA7,0xAF,0xB7,0xC1,0xCB,0xD3,0xDD,0xE6,0xEF,0xF6,0xFF,0x16,0x25,0x7C,0x62,0xCA,0x3A,0xC2,0x1F,0xC0}},
    {0xB6,1,{0x03}},
    {0xD4,1,{0x32}},
    {0x11,0,{}},
    {REGFLAG_DELAY, 200, {}},
	{0xBA,10,{0x13,0x82,0x00,0x16,0xC5,0x00,0x10,0xFF,0x0F,0x28}},
    {0x29,0,{}},
    {REGFLAG_DELAY, 50, {}},
    // Note
	// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.

	// Setting ending by predefined flag
    {REGFLAG_END_OF_TABLE, 0x00, {}}

};
#endif
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
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
       	}
    }
	
}

static void init_lcm_registers(void)
{

    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type   = LCM_TYPE_DSI;
	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;
	params->dsi.mode   					= SYNC_PULSE_VDO_MODE;   //BURST_VDO_MODE;  // SYNC_PULSE_VDO_MODE
	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				= LCM_FOUR_LANE;//LCM_FOUR_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	// Highly depends on LCD driver capability.
	// Not support in MT6573
	params->dsi.packet_size=256;
	// Video mode setting
	params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.word_count=720*3;

	params->dsi.vertical_sync_active	= 2;
	params->dsi.vertical_backporch		= 8;
	params->dsi.vertical_frontporch		= 6;
	params->dsi.vertical_active_line	= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active	= 50;
	params->dsi.horizontal_backporch	= 80;
	params->dsi.horizontal_frontporch	= 86;
	params->dsi.horizontal_active_pixel	= FRAME_WIDTH;

	// Bit rate calculation
	//params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
	//params->dsi.pll_div2=1;		// div2=0,1,2,3;div1_real=1,2,4,4
	//params->dsi.fbk_div =30;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
      //params->dsi.cont_clock = TRUE; 
      params->dsi.noncont_clock_period = 2; // Unit : frames

      params->dsi.PLL_CLOCK =213; //200;//195; //240;//260;//255;  // 195 -  265
}
static unsigned int lcm_compare_id(void);
static void lcm_init(void)
{
	LCM_DBG();
	lcm_compare_id();
	
	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(10);
	
	SET_RESET_PIN(1);
	MDELAY(120);      

	init_lcm_registers();

}



static void lcm_suspend(void)
{
	unsigned int data_array[16];

	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(120);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120); 
}


static void lcm_resume(void)
{
   //1 do lcm init again to solve some display issue

    SET_RESET_PIN(1);
    MDELAY(5);
    SET_RESET_PIN(0);
    MDELAY(10);

    SET_RESET_PIN(1);
    MDELAY(120);      
    //init_esd_registers();
    init_lcm_registers();

}
         
#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	/*data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(&data_array, 3, 1);
	
	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(&data_array, 3, 1);

	data_array[0]= 0x00290508; //HW bug, so need send one HS packet
	dsi_set_cmdq(&data_array, 1, 1);
	
	data_array[0]= 0x002c3909;
	dsi_set_cmdq(&data_array, 1, 0);
*/
}
#endif

static unsigned int lcm_esd_check(void)
{

  #ifndef BUILD_LK
    unsigned char buffer[1];
    unsigned int array[16];
    unsigned char print_buffer[4];

    static int once_time=0;

    unsigned int data_array[16];
  #if 1
/*
    data_array[0]= 0x00323902;
    data_array[1]= 0x000000D5;
    data_array[2]= 0x01000A00;
    data_array[3]= 0x0000CC00;
    data_array[4]= 0x88888800;
    data_array[5]= 0x88888888;
    data_array[6]= 0x01888888;
    data_array[7]= 0x01234567;
    data_array[8]= 0x88888823;
    data_array[9]= 0x88;
    dsi_set_cmdq(&data_array, 10, 1);
*/
    array[0] = 0x00013700;// read id return two byte,version and id
    dsi_set_cmdq(array, 1, 1);
    read_reg_v2(0x0A, buffer, 1);
    #if defined(BUILD_LK)
    //printf("lcm_esd_check  0x0A = %x\n",buffer[0]);
    #else
    print_buffer[0] = buffer[0];
    //printk("lcm_esd_check  0x0A = %x\n",buffer[0]);
    #endif
    /*
    if(buffer[0] != 0x1C)  
    {
        return TRUE;
    }
    */
    array[0] = 0x00013700;// read id return two byte,version and id
    dsi_set_cmdq(array, 1, 1);
    //read_reg_v2(0x0B, buffer, 1);
    read_reg_v2(0x0D, buffer, 1);
    #if defined(BUILD_LK)
    //printf("lcm_esd_check  0x0B = %x\n",buffer[0]);
    #else
    print_buffer[1] = buffer[0];
    //printk("lcm_esd_check  0x0B = %x\n",buffer[0]);
    #endif
    /*
    if(buffer[0] != 0x00)  
    {
        return TRUE;
    }*/
	//array[0] = 0x00013700;// read id return two byte,version and id
    //dsi_set_cmdq(array, 1, 1);
    //read_reg_v2(0x0C, buffer, 1);
    #if defined(BUILD_LK)
    //printf("lcm_esd_check  0x0B = %x\n",buffer[0]);
    #else
    //print_buffer[2] = buffer[0];
    //printk("lcm_esd_check  0x0C = %x\n",buffer[0]);
    #endif
	//array[0] = 0x00013700;// read id return two byte,version and id
    //dsi_set_cmdq(array, 1, 1);
    //read_reg_v2(0x0D, buffer, 1);
    #if defined(BUILD_LK)
    //printf("lcm_esd_check  0x0B = %x\n",buffer[0]);
    #else
    /*
    if(0==once_time)
    {
        once_time=1;
        //UDELAY(3); // 2 3
    }
    */
    //print_buffer[3] = buffer[0];
    //printk("lcm_esd_check:%x,%x,%x,%x\n",print_buffer[0],print_buffer[1],print_buffer[2],print_buffer[3]);
    #endif

    //if(0x9C==print_buffer[0]&&0x00==print_buffer[1]&&0x70==print_buffer[2]&&0x00==print_buffer[3])
    if(0x1C==print_buffer[0]&&0x00==print_buffer[1])
       return FALSE;
    else
        return TRUE;
  #endif    
#endif


}
void lcm_reset(void)
{
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);
	SET_RESET_PIN(1);
	MDELAY(120); 
}

static unsigned int lcm_esd_recover(void)
{
    unsigned char para = 0;
    #ifdef BUILD_LK
        printf("%s, LINE = %d\n", __func__,__LINE__); 
    #else
        printk("%s, LINE = %d\n", __func__,__LINE__);  
    #endif
        
    lcm_reset();
    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
    //MDELAY(10);
    //push_table_tkc(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
    
    //MDELAY(10);
    //dsi_set_cmdq_V2(0x35, 1, &para, 1);     ///enable TE
    //MDELAY(10);
    
    return TRUE;
        
}



static unsigned int lcm_compare_id(void)
{
	unsigned int id0,id1,id=0;
	unsigned char buffer[2];
	unsigned int array[16];  
        LCM_DBG();
	 
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);
	SET_RESET_PIN(1);
	MDELAY(20); 

	array[0] = 0x00043902;                          
	array[1] = 0x9483ffb9;                 
	dsi_set_cmdq(array, 2, 1);

	  
	MDELAY(20);
	memset(buffer, 0, sizeof(buffer));
	array[0] = 0x00023700;// return byte number
	dsi_set_cmdq(&array, 1, 1);
	//MDELAY(5);

	read_reg_v2(0xf4, buffer, 1);
	id=buffer[0];

	LCM_DBG("id0 = 0x%08x\n", id0);
	LCM_DBG("id1 = 0x%08x\n" , id1);
	LCM_DBG("id  = 0x%08x\n", id);
	
	return (LCM_ID_HX8394 == id)?1:0;
}

LCM_DRIVER hx8394a_dsi_vdo_lcm_drv_truly = 
{
       .name	         = "hx8394a_dsi_vdo_truly",
	.set_util_funcs  = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init                = lcm_init,
	.suspend         = lcm_suspend,
	.resume          = lcm_resume,
	.compare_id    = lcm_compare_id,
	.esd_check = lcm_esd_check,
	.esd_recover = lcm_esd_recover,
    #if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
    #endif
    };
