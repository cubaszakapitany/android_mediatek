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
	LCM_PRINT ("[LCM-hx8389b_qhd_dsi_vdo] %s (line:%d) :" fmt "\r\n", __func__, __LINE__, ## arg)
	
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(540)
#define FRAME_HEIGHT 										(960)

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xFF   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE									0

#define LCM_ID_HX8389B 0x89

static unsigned int lcm_esd_test = FALSE;      ///only for ESD test

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   			lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    
       

static struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static struct LCM_setting_table lcm_set_window[] = {
	{0x2A,	4,	{0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
	{0x2B,	4,	{0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void init_lcm_registers(void)
{

	/*
		Note :
	
		Data ID will depends on the following rule.
		
			count of parameters > 1 => Data ID = 0x39
			count of parameters = 1 => Data ID = 0x15
			count of parameters = 0 => Data ID = 0x05
	
		Structure Format :
	
		{DCS command, count of parameters, {parameter list}}
		{REGFLAG_DELAY, milliseconds of time, {}},
	
		...
	
		Setting ending by predefined flag
		
		{REGFLAG_END_OF_TABLE, 0x00, {}}
		*/

    
	unsigned int data_array[16];
	
	//data_array[0] = 0x00110500; // Sleep Out
	//dsi_set_cmdq(data_array, 1, 1);
	//MDELAY(200);

	//PacketHeader[39 04 00 xx] // Set EXTC 
	//Payload[B9 FF 83 94] 
    data_array[0] = 0x00043902;                          
    data_array[1] = 0x8983ffb9;                 
    dsi_set_cmdq(data_array, 2, 1);	

      //PacketHeader[39 11 00 xx] // Set MIPI 
      //Payload[BA 13 82 00 16 C5 00 10 FF 0F 24 03 21 24 25 20 08] 
      
    data_array[0] = 0x00043902;                          
    data_array[1] = 0x105805de;       	  
    dsi_set_cmdq(data_array, 2, 1);	
      
    data_array[0] = 0x000c3902;                          
    data_array[1] = 0x009201ba;
    data_array[2] = 0x1800a416;
    data_array[3] = 0x00230fff;       	  
    dsi_set_cmdq(data_array, 4, 1);	

   
    data_array[0] = 0x00143902;                          
    data_array[1] = 0x040000b1; 
    data_array[2] = 0x111096e3; 
    data_array[3] = 0x3028ef6f; 
    data_array[4] = 0x00422323; 
    data_array[5] = 0x0020f258;                 	 
    dsi_set_cmdq(data_array, 6, 1);	

	
	data_array[0] = 0x00083902;                          
    data_array[1] = 0x780000b2; 
    data_array[2] = 0x80000308;                	 
    dsi_set_cmdq(data_array, 3, 1);	
   
   	data_array[0] = 0x00183902;                          
    data_array[1] = 0x000880b4; 
    data_array[2] = 0x00001032; 
    data_array[3] = 0x00000000;	
    data_array[4] = 0x400a3700;		
    data_array[5] = 0x400a3704;	
    data_array[6] = 0x0a555014;	
    dsi_set_cmdq(data_array, 7, 1);	

   	data_array[0] = 0x001e3902;                          
    data_array[1] = 0x4c0000d5; 
    data_array[2] = 0x00000100; 
    data_array[3] = 0x99006000;	
    data_array[4] = 0x88889988;		
    data_array[5] = 0x88108832;	
    data_array[6] = 0x10548876;	
    data_array[7] = 0x88888832;	
    data_array[8] = 0x00008888;	    
    dsi_set_cmdq(data_array, 9, 1);	
    
   	data_array[0] = 0x00233902;                          
    data_array[1] = 0x181405e0; 
    data_array[2] = 0x203f342d; 
    data_array[3] = 0x0e0e083c;	
    data_array[4] = 0x12101311;		
    data_array[5] = 0x14051c1a;	
    data_array[6] = 0x3f342d18;	
    data_array[7] = 0x0e083c20;	
    data_array[8] = 0x1013110e;	   
    data_array[9] = 0x001c1a12; 
    dsi_set_cmdq(data_array, 10, 1);	   
   
    /*data_array[0] = 0x00803902 ;
    data_array[1] = 0x080201C1 ;
    data_array[2] = 0x2D272017 ;
    data_array[3] = 0x48403832 ;
    data_array[4] = 0x675F574F ;
    data_array[5] = 0x89817970 ;
    data_array[6] = 0xA8A19991 ;
    data_array[7] = 0xC9C2B9B0 ;
    data_array[8] = 0xE9E2D9D0 ;
    data_array[9] = 0xD0FFFAF3 ;
    data_array[10] = 0xA9A9DFAF ;
    data_array[11] = 0xC092793D ;
    data_array[12] = 0x20170802 ;
    data_array[13] = 0x38322D27 ;
    data_array[14] = 0x574F4840 ;
    data_array[15] = 0x7970675F ;
    data_array[16] = 0x99918981 ;
    data_array[17] = 0xB9B0A8A1 ;
    data_array[18] = 0xD9D0C9C2 ;
    data_array[19] = 0xFAF3E9E2 ;
    data_array[20] = 0xDFAFD0FF ;
    data_array[21] = 0x793DA9A9 ;
    data_array[22] = 0x0802C092 ;
    data_array[23] = 0x2D272017 ;
    data_array[24] = 0x48403832 ;
    data_array[25] = 0x675F574F ;
    data_array[26] = 0x89817970 ;
    data_array[27] = 0xA8A19991 ;
    data_array[28] = 0xC9C2B9B0 ;
    data_array[29] = 0xE9E2D9D0 ;
    data_array[30] = 0xD0FFFAF3 ;
    data_array[31] = 0xA9A9DFAF ;
    data_array[32] = 0xC092793D ;     
    dsi_set_cmdq(data_array, 33, 1);*/
   /////////////////////////////DGC
    data_array[0] = 0x00213902;  
    data_array[1] = 0x080201C1 ;
    data_array[2] = 0x2D272017 ;
    data_array[3] = 0x48403832 ;
    data_array[4] = 0x675F574F ;
    data_array[5] = 0x89817970 ;
    data_array[6] = 0xA8A19991 ;
    data_array[7] = 0xC9C2B9B0 ;
    data_array[8] = 0xE9E2D9D0 ;
    data_array[9] = 0x000000F3 ;
    dsi_set_cmdq(data_array, 10, 1);
    
    data_array[0] = 0x00212902;  
    data_array[1] = 0xD0FFFAC1 ;
    data_array[2] = 0xA9A9DFAF ;
    data_array[3] = 0xC092793D ;
    data_array[4] = 0x20170802 ;
    data_array[5] = 0x38322D27 ;
    data_array[6] = 0x574F4840 ;
    data_array[7] = 0x7970675F ;
    data_array[8] = 0x99918981 ;
    data_array[9] = 0x000000A1 ;
    dsi_set_cmdq(data_array, 10, 1);
    
    data_array[0] = 0x00212902;  
    data_array[1] = 0xB9B0A8C1 ;
    data_array[2] = 0xD9D0C9C2 ;
    data_array[3] = 0xFAF3E9E2 ;
    data_array[4] = 0xDFAFD0FF ;
    data_array[5] = 0x793DA9A9 ;
    data_array[6] = 0x0802C092 ;
    data_array[7] = 0x2D272017 ;
    data_array[8] = 0x48403832 ;
    data_array[9] = 0x0000004F ;
    dsi_set_cmdq(data_array, 10, 1);
    
     data_array[0] = 0x00202902;  
    data_array[1] = 0x675F57C1 ;
    data_array[2] = 0x89817970 ;
    data_array[3] = 0xA8A19991 ;
    data_array[4] = 0xC9C2B9B0 ;
    data_array[5] = 0xE9E2D9D0 ;
    data_array[6] = 0xD0FFFAF3 ;
    data_array[7] = 0xA9A9DFAF ;
    data_array[8] = 0xC092793D ;
    dsi_set_cmdq(data_array, 9, 1);
    //////////////////// DGC  
   
    data_array[0] = 0x00023902;                          
    data_array[1] = 0x000002cc; 
    dsi_set_cmdq(data_array, 2, 1);
    
    data_array[0] = 0x00053902;                          
    data_array[1] = 0x00a000b6;
    data_array[2] = 0x000000a0;
    dsi_set_cmdq(data_array, 3, 1);
    
    data_array[0] = 0x00033902;                          
    data_array[1] = 0x000707cb; 
    dsi_set_cmdq(data_array, 2, 1);
    
    data_array[0] = 0x00053902;                          
    data_array[1] = 0xff0000bb;
    data_array[2] = 0x00000080;
    dsi_set_cmdq(data_array, 3, 1);
       
	data_array[0] = 0x00110500; // Sleep Out
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(200);
       //PacketHeader[05 29 00 xx] // Display On 
       //Delay 10ms 
    data_array[0] = 0x00290500; // Sleep Out
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(10);

	  
}

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 0, {0x00}},
	{REGFLAG_DELAY, 10, {}},
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_in_setting[] = {
	// Display off sequence
	//{0x28, 0, {0x00}},

    // Sleep Mode On
	{0x10, 0, {0x00}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

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
		
	    // enable tearing-free
	    params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
	    params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;
	
        #if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
        #else
		params->dsi.mode   = SYNC_PULSE_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE; 
        #endif
	
		// DSI
		/* Command mode setting */
		//1 Three lane or Four lane
		params->dsi.LANE_NUM				= LCM_TWO_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

		params->dsi.vertical_sync_active				= 2;
		params->dsi.vertical_backporch					= 10;
		params->dsi.vertical_frontporch					= 5;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 20;
		params->dsi.horizontal_backporch				= 46;
		params->dsi.horizontal_frontporch				= 21;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		// Bit rate calculation
		//params->dsi.pll_div1=34;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
		//params->dsi.pll_div2=1; 		// div2=0~15: fout=fvo/(2*div2)

		params->dsi.PLL_CLOCK=240;
}


static void lcm_init(void)
{
	LCM_DBG();
    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(1);
    SET_RESET_PIN(1);
    MDELAY(10);//Must over 6 ms,SPEC request
	
	init_lcm_registers();
	//push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
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
	LCM_DBG();
    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(1);
    SET_RESET_PIN(1);
    MDELAY(120);//Must over 6 ms,SPEC request
	
	init_lcm_registers();
}


static unsigned int lcm_esd_check(void)
{
	unsigned int ret=FALSE;
#ifndef BUILD_LK	
	char  buffer[6];
	int   array[4];
	char esd;
	
    if(lcm_esd_test)
    {
        lcm_esd_test = FALSE;
        return TRUE;
    }

	array[0] = 0x00023700;
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0x0A, buffer, 2);
	esd=buffer[0];
    LCM_DBG("lcm_esd_check=0x%x",esd);
	if(esd==0x1C)
	{
		ret=FALSE;
	}
	else
	{			 
		ret=TRUE;
	}
#endif
	return ret;
}

static unsigned int lcm_esd_recover(void)
{
    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(1);
    SET_RESET_PIN(1);
    MDELAY(120);
	lcm_init();
    return TRUE;
}

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

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	data_array[3]= 0x00053902;
	data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[5]= (y1_LSB);
	data_array[6]= 0x002c3909;

	dsi_set_cmdq(&data_array, 7, 0);

}


static unsigned int lcm_compare_id(void)
{
	unsigned int id=0;
	unsigned char buffer[2];
	unsigned int array[16];  

    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(1);
    SET_RESET_PIN(1);
    MDELAY(10);//Must over 6 ms

	array[0]=0x00043902;
	array[1]=0x8983FFB9;// page enable
	dsi_set_cmdq(&array, 2, 1);
	MDELAY(10);

	array[0] = 0x00023700;// return byte number
	dsi_set_cmdq(&array, 1, 1);
	MDELAY(10);

	read_reg_v2(0xF4, buffer, 2);
	id = buffer[0]; 
	
	LCM_DBG("%s, id = 0x%08x\n", __func__, id);

	return (LCM_ID_HX8389B == id)?1:0;

}




LCM_DRIVER hx8389b_qhd_dsi_vdo_drv = 
{
    .name			= "hx8389b_dsi_vdo",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
	.esd_check   = lcm_esd_check,
	.esd_recover   = lcm_esd_recover,
#if (LCM_DSI_CMD_MODE)
	.set_backlight	= lcm_setbacklight,
    .update         = lcm_update,
#endif
};

