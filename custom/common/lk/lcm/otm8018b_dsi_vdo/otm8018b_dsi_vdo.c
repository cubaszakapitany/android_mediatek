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

#define FRAME_WIDTH  										(480)
#define FRAME_HEIGHT 										(854)

#define LCM_ID_OTM8018B	0x8009

#define LCM_DSI_CMD_MODE									0

#ifndef TRUE
    #define   TRUE     1
#endif
 
#ifndef FALSE
    #define   FALSE    0
#endif

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
#define read_reg_v2(cmd, buffer, buffer_size)				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

static unsigned char lcm_initialization_setting[] = {
/*	cmd,		count,	params*/
#if 0//PANEL V1
	0x00,	1,		0x00,
	0xFF,	3,		0x80, 0x09, 0x01,                    
	0x00,	1,		0x80,
	0xFF,	2,		0x80, 0x09,
	     	  		
	0x00,	1,		0x03,
	0xFF,	1,		0x01,
	     	  		
	0x00,	1,		0x90,
	0xB3,	1,		0x02,
	     	  		
	0x00,	1,		0x92,
	0xB3,	1,		0x45,
	     	  		
	0x00,	1,		0xA6,
	0xB3,	2,		0x20, 0x01,
	     	  		
	0x00,	1,		0xA3,
	0xC0,	1,		0x1B,
	     	  		
	0x00,	1,		0xB4,
	0xC0,	1,		0x50,
	     	  		
	0x00,	1,		0x81,
	0xC4,	1,		0x04,
	     	  		
	0x00,	1,		0x80,
	0xC5,	1,		0x03,
	     	  		
	0x00,	1,		0x90,
	0xC0,	6,		0x00,0x44,0x00,0x00,0x00,0x03,
	     	  		
	0x00,	1,		0xA6,
	0xC1,	3,		0x01,00,00,
	     	  		
	0x00,	1,		0xA0,
	0xC1,	1,		0xEA,
	     	  		
	0x00,	1,		0x8B,
	0xB0,	1,		0x40,
	     	  		
	0x00,	1,		0xC6,//ADD
	0xB0,	1,		0x03,//ADD
	     	  		
	0x00,	1,		0x82,
	0xC5,	1,		0x03,
	     	  		
	0x00,	1,		0x90,
	0xC5,	5,		0x96,0x2B,0x04,0x7B,0x33,
	     	  		
	0x00,	1,		0x00,
	0xD8,	2,		0x70, 0x70,
	     	  		
	0x00,	1,		0x00,
	0xD9,	1,		0x38,//0x3A
	     	  		
	0x00,	1,		0x81,
	0xC1,	1,		0x66,

	0x00,	1,		0x00,                            
	0xE1,	16,		0x08,0x13,0x19,0x0d,0x06,       
					0x0d,0x0a,0x08,0x05,0x08,             
					0x0e,0x09,0x0f,0x0D,0x07,             
					0x03,                                 
	                                              
	0x00,	1,		0x00,                            
	0xE2,	16,		0x08,0x13,0x19,0x0d,0x06,       
					0x0d,0x0a,0x08,0x05,0x08,             
					0x0e,0x09,0x0f,0x0D,0x07,             
					0x03,                                 
	                                              
	0x00,	1,		0x80,                            
	0xCE,	12,		0x86,0x01,0x00,0x85,0x01,       
					0x00,0x00,0x00,0x00,0x00,             
					0x00,0x00,                            
	                                              
	0x00,	1,		0xA0,                            
	0xCE,	14,		0x18,0x03,0x03,0x5A,0x00,       
					0x00,0x00,0x18,0x02,0x03,             
					0x5B,0x00,0x00,0x00,                  
	                                              
	0x00,	1,		0xB0,                            
	0xCE,	14,		0x18,0x05,0x03,0x5C,0x00,       
					0x00,0x00,0x18,0x04,0x03,             
					0x5D,0x00,0x00,0x00,                  
	                                              
	0x00,	1,		0xC0,                            
	0xCE,	14,		0x18,0x05,0x03,0x5A,0x00,       
					0x00,0x00,0x18,0x04,0x03,             
					0x5B,0x00,0x00,0x00,                  
	                                              
	0x00,	1,		0xD0,                            
	0xCE,	14,		0x18,0x03,0x03,0x5C,0x00,       
					0x00,0x00,0x18,0x02,0x03,             
					0x5D,0x00,0x00,0x00,                  
	                                              
	0x00,	1,		0xC0,                            
	0xCF,	10,		0x01,0x01,0x20,0x20,0x00,         
					0x00,0x01,0x02,0x00,0x00,             
	                                              
	0x00,	1,		0xD0,                            
	0xCF,	1,		0x00,                            
	                                              
	0x00,	1,		0x80,                            
	0xCB,	10,		0x00,0x00,0x00,0x00,0x00,         
					0x00,0x00,0x00,0x00,0x00,             
	                                              
	0x00,	1,		0x90,                            
	0xCB,	15,		0x00,0x00,0x00,0x00,0x00,         
					0x00,0x00,0x00,0x00,0x00,             
					0x00,0x00,0x00,0x00,0x00,             
	                                              
	0x00,	1,		0xA0,                            
	0xCB,	15,		0x00,0x00,0x00,0x00,0x00,         
					0x00,0x00,0x00,0x00,0x00,             
					0x00,0x00,0x00,0x00,0x00,             
	                                              
	0x00,	1,		0xB0,                            
	0xCB,	10,		0x00,0x00,0x00,0x00,0x00,         
					0x00,0x00,0x00,0x00,0x00,             
	                                              
	0x00,	1,		0xC0,                            
	0xCB,	15,		0x00,0x04,0x04,0x04,0x04,         
					0x04,0x00,0x00,0x00,0x00,             
					0x00,0x00,0x00,0x00,0x00,             
	                                              
	0x00,	1,		0xD0,                            
	0xCB,	15,		0x00,0x00,0x00,0x00,0x00,         
					0x00,0x04,0x04,0x04,0x04,             
					0x04,0x00,0x00,0x00,0x00,//GOA        
	                                              
	0x00,	1,   0xE0,                            
	0xCB,	10,		0x00,0x00,0x00,0x00,0x00,         
					0x00,0x00,0x00,0x00,0x00,             
	                                              
	0x00,	1,		0xF0,                            
	0xCB,	10,		0xFF,0xFF,0xFF,0xFF,0xFF,         
					0xFF,0xFF,0xFF,0xFF,0xFF,             
	                                              
	0x00,	1,		0x80,                            
	0xCC,	10,		0x00,0x26,0x09,0x0B,0x01,         
					0x25,0x00,0x00,0x00,0x00,             
	                                              
	0x00,	1,		0x90,                            
	0xCC,	15,		0x00,0x00,0x00,0x00,0x00,         
					0x00,0x00,0x00,0x00,0x00,             
					0x00,0x26,0x0A,0x0C,0x02,             
	                                              
	0x00,	1,		0xA0,                            
	0xCC,	15, 		0x25,0x00,0x00,0x00,0x00,         
					0x00,0x00,0x00,0x00,0x00,             
					0x00,0x00,0x00,0x00,0x00,             
	                                              
	0x00,	1,		0xB0,                            
	0xCC,	10,		0x00,0x25,0x10,0x0E,0x02,       
					0x26,0x00,0x00,0x00,0x00,             
	                                              
	0x00,	1,		0xC0,                            
	0xCC,	15,		0x00,0x00,0x00,0x00,0x00,         
					0x00,0x00,0x00,0x00,0x00,             
					0x00,0x25,0x0F,0x0D,0x01,             
	                                              
	                                              
	0x00,	1,		0xD0,                            
	0xCC,	15,		0x26,0x00,0x00,0x00,0x00,         
					0x00,0x00,0x00,0x00,0x00,             
					0x00,0x00,0x00,0x00,0x00,             

	0x00,	1,		0x80,
	0xD6,	1,		0x00,//CE enable：E8-high; 78-middle; 38-low

	0x00,	1,		0x00,
	0xFF,	3,		0xFF,0xFF,0xFF,//disable CMD2
#else//PANEL V2
	0X00,	1,		0X00,         //偏移量数据// Shift Function偏移量命令
	0XFF,	3,		0X80,0X09,0X01,//Enable Access Command2
	0X00,	1,		0X80,        //enable orise mode
	0XFF,	2,		0X80,0X09,
	0X00,	1,		0X90,
	0XB3,	3,		0X02,0X00,0X45,
	0X00,	1,		0X03,
	0XFF,	1,		0X01,
	0X00,	1,		0X90,
	0XC5,	5,		0X96,0X37,0X04,0X7B,0X44,// Power control setting
	0X00,	1,		0X82,
	0XC5,	1,		0X03,// Power control setting
	0X00,	1,		0XB2,
	0XF5,	4,		0X15,0X00,0X15,0X00,
	0X00,	1,		0XB4,
	0XC0,	1,		0X50,// Panel driving mode//00/1 dot invert,50/column invert
//	0X00,	1,		0XB5,
//	0XC0,	1,		0X18,
	0X00,	1,		0X81,
	0XC4,	1,		0X04,
	0X00,	1,		0X00,
	0XD8,	2,		0X45,0x45,  // GVDD/NGVDD Setting    
	0X00,	1,		0XB1,
	0XC5,	1,		0X29,
	0X00,	1,		0X81,
	0XC1,	1,		0X77,// Oscillator adjust for idle/normal mode
	0X00,	1,		0X00,
	0XD9,	1,		0X2F,// VCOM Voltage setting
	0X00,	1,		0XA0,
	0XC1,	1,		0XEA,
	0X00,	1,		0XA6,
	0XB3,	2,		0X20,0X01,
	0X00,	1,		0X00,
	0XE1,	16,		0X00,0X09,0X0E,0X0D,0X06,0X0E,0X0B,0X0A,0X04,0X07,0X0D,0X09,0X0F,0X15,0X10,0X00,// Gamma 2.2+
	0X00,	1,		0X00,
	0XE2,	16,		0X00,0X08,0X0E,0X0D,0X06,0X0E,0X0B,0X0A,0X04,0X08,0X0D,0X09,0X0F,0X15,0X10,0X00,// Gamma 2.2-
	0X00,	1,		0X80,
	0XCE,	12,		0X86,0X01,0X00,0X85,0X01,0X00,0X00,0X00,0X00,0X00,0X00,0X00,// GOA VST Setting
	0X00,	1,		0XA0,
	0XCE,	14,		0X18,0X05,0X83,0X5A,0X00,0X00,0X00,0X18,0X04,0X83,0X5B,0X88,0X00,0X00,// GOA CLKA0 Setting
	0X00,	1,		0XB0,
	0XCE,	14,		0X18,0X03,0X83,0X5C,0X86,0X00,0X00,0X18,0X02,0X83,0X5D,0X88,0X00,0X00,// GOA CLKA3 Setting
	0X00,	1,		0XC0,
	0XCF,	10,		0X01,0X01,0X20,0X20,0X00,0X00,0X01,0X01,0X00,0X00,//GOA ECLK Setting
	0X00,	1,		0XD0,
	0XCF,	1,		0X00,
	0X00,	1,		0X80,
	0XCB,	10,		0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
	0X00,	1,		0X90,
	0XCB,	15,		0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
	0X00,	1,		0XA0,
	0XCB,	15,		0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
	0X00,	1,		0XB0,
	0XCB,	10,		0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
	0X00,	1,		0XC0,            
	0XCB,	15,		0X00,0X04,0X04,0X04,0X04,0X04,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
	0X00,	1,		0XD0,            
	0XCB,	15,		0X00,0X00,0X00,0X00,0X00,0X00,0X04,0X04,0X04,0X04,0X04,0X00,0X00,0X00,0X00,
	0X00,	1,		0XE0,
	0XCB,	10,		0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
	0X00,	1,		0XF0,
	0XCB,	10,		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
	0X00,	1,		0X80,
	0XCC,	10,		0X00,0X26,0X09,0X0B,0X01,0X25,0X00,0X00,0X00,0X00,
	0X00,	1,		0X90,
	0XC0,	6,		0X00,0X56,0X00,0X00,0X00,0X03,
	0X00,	1,		0X90,
	0XCC,	15,		0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X26,0X0A,0X0C,0X02,
	0X00,	1,		0XA0,
	0XCC,	15,		0X25,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
	0X00,	1,		0XB0,
	0XCC,	10,		0X00,0X25,0X0A,0X0C,0X02,0X26,0X00,0X00,0X00,0X00,
	0X00,	1,		0XC0,
	0XCC,	15,		0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X25,0X09,0X0B,0X01,
	0X00,	1,		0XD0,
	0XCC,	15,		0X26,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
	0X00,	1,		0X8B,
	0XB0,	1,		0X40,
	0X00,	1,		0XA3,
	0XC0,	1,		0X18,                 // Source driver precharge
	0X00,	1,		0X80,
	0XC5,	1,		0X03,
	0X00,	1,		0X80,
	0XC0,	9,		0X00,0X58,0X00,0X14,0X16,0X00,0X58,0X14,0X16,
	0X00,	1,		0XA1,
	0XC1,	1,		0X08,
	0X00,	1,		0X88,
	0XC4,	1,		0X80,
	0X00,	1,		0X87,
	0XC4,	1,		0X00,
	0X00,	1,		0X89,
	0XC4,	1,		0X00,
	0X00,	1,		0XA6,
	0XC1,	3,		0X01,0X00,0X00,
#endif
	REGFLAG_DELAY, 10,
	// Sleep Out
	0x11, 0, 
	REGFLAG_DELAY, 150,

	// Display ON
	0x29, 0,
	0x2C,      0,
	REGFLAG_DELAY, 20,
	
	REGFLAG_END_OF_TABLE
};

static unsigned char lcm_sleep_out_setting[] = {
	// Sleep Out
	0x11, 0, 
	REGFLAG_DELAY, 150,

	// Display ON
	0x29, 0,
	0x2C,      0,
	REGFLAG_DELAY, 20,
	
	REGFLAG_END_OF_TABLE
};

static unsigned char lcm_sleep_mode_in_setting[] = {
	// Display off sequence
	0x28, 0,
	REGFLAG_DELAY, 50,

	// Sleep Mode On
	0x10, 0, 
	REGFLAG_DELAY, 20,

	REGFLAG_END_OF_TABLE
};


static int push_table(unsigned char table[])
{
	unsigned int i, bExit = 0;
	unsigned char *p = (unsigned char *)table;
	LCM_SETTING_ITEM *pSetting_item;

	while(!bExit) {
		pSetting_item = (LCM_SETTING_ITEM *)p;

		switch (pSetting_item->cmd) {
			
		case REGFLAG_DELAY :
			MDELAY(pSetting_item->count);
			p += 2;
		break;

		case REGFLAG_END_OF_TABLE :
			p += 2;
			bExit = 1;
		break;

		default:
			dsi_set_cmdq_V2(pSetting_item->cmd, 
							pSetting_item->count, pSetting_item->params, 1);
			//MDELAY(2);
			p += pSetting_item->count + 2;
		break;
		}
	}
	return p - table; //return the size of  settings array.
}

#if !defined(BUILD_UBOOT) && !defined(BUILD_LK)
static int get_initialization_settings(unsigned char table[])
{
	memcpy(table, lcm_initialization_setting, sizeof(lcm_initialization_setting));
	return sizeof(lcm_initialization_setting);
}

static void lcm_init(void);

static int set_initialization_settings(const unsigned char table[], const int count)
{
	if ( count > LCM_INIT_TABLE_SIZE_MAX ){
		return -EIO;
	}
	memset(lcm_initialization_setting, REGFLAG_END_OF_TABLE, sizeof(lcm_initialization_setting));
	memcpy(lcm_initialization_setting, table, count);

	lcm_init();

	return count;
}
#endif

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
	//params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
	params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
	//params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if (LCM_DSI_CMD_MODE)
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
	params->dsi.force_dcs_packet=0;
	params->dsi.isFixIli9806cESD=0;

	// Video mode setting		
	params->dsi.intermediat_buffer_num = 2;

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.word_count=480*3;
#if 1
	params->dsi.vertical_sync_active				= 4;
	params->dsi.vertical_backporch					= 21;
	params->dsi.vertical_frontporch					= 21;
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 

	params->dsi.horizontal_sync_active				= 6;
	params->dsi.horizontal_backporch				= 44;
	params->dsi.horizontal_frontporch				= 46;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
#else
	params->dsi.vertical_sync_active				= 2;
	params->dsi.vertical_backporch					= 16;
	params->dsi.vertical_frontporch					= 16;
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 

	params->dsi.horizontal_sync_active				= 4;
	params->dsi.horizontal_backporch				= 32;
	params->dsi.horizontal_frontporch				= 32;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
#endif
	params->dsi.compatibility_for_nvk = 0;	
	// Bit rate calculation
	//		params->dsi.pll_div1=29;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
	//		params->dsi.pll_div2=1; 		// div2=0~15: fout=fvo/(2*div2)


	/* ESD or noise interference recovery For video mode LCM only. */ // Send TE packet to LCM in a period of n frames and check the response. 
	/*		params->dsi.lcm_int_te_monitor = FALSE; 
	params->dsi.lcm_int_te_period = 1; // Unit : frames 

	// Need longer FP for more opportunity to do int. TE monitor applicably. 
	if(params->dsi.lcm_int_te_monitor) 
		params->dsi.vertical_frontporch *= 2; 

	// Monitor external TE (or named VSYNC) from LCM once per 2 sec. (LCM VSYNC must be wired to baseband TE pin.) 
	params->dsi.lcm_ext_te_monitor = TRUE; 
	// Non-continuous clock 
	params->dsi.noncont_clock = FALSE; 
	params->dsi.noncont_clock_period = 2; // Unit : frames
	*/

	params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
	params->dsi.pll_div2=2;		// div2=0,1,2,3;div1_real=1,2,4,4	
	params->dsi.fbk_div =26;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	

}

static void lcm_init(void)
{
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(20);

	push_table(lcm_initialization_setting);
}

static void lcm_suspend(void)
{
	push_table(lcm_sleep_mode_in_setting);
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(20);	
}

static void lcm_resume(void)
{
	lcm_init();
}

static unsigned char lcm_compare_id_setting[] = {

	0x00,	1,		0x00,
	0xFF,	3,		0x80,0x09,0x01,
	REGFLAG_DELAY, 10,
	
	0x00,	1,		0x80,
	0xFF,	3,		0x80,0x09,0x01,
	REGFLAG_DELAY, 10,

	0x00,	1,		0x02,

	REGFLAG_END_OF_TABLE
};

static unsigned int lcm_compare_id(void)
{
	int array[4];
	char buffer[5];
	char id_high=0;
	char id_low=0;
	int id=0;

#ifdef TINNO_ANDROID_S7811A
	extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);
	int Channel=1;
	int data[4];
	int adcVol=0;

	int res=IMM_GetOneChannelValue(Channel,data,0);
	adcVol=data[0]*1000+data[1]*10;


#ifdef BUILD_LK
	printf("LINE=%d %s, res=%d,adcVol = %d \n", __LINE__,__func__,res,adcVol); 
#else
	printk("LINE=%d %s, id1 = 0x%08x\n",__LINE__,__func__, id);   
#endif

	if(adcVol <= 750 && adcVol >= 300){ //550
		return 1;
	}
	else	{
		return 0;
	}	
#else
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(20);
/*
	array[0] = 0x00053700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xa1, buffer, 5);

	id_high = buffer[2];
	id_low = buffer[3];
	id = (id_high<<8) | id_low;
*/
	push_table(lcm_compare_id_setting);

	array[0] = 0x00023700;// set return byte number
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xD2, &buffer, 2);

	id = buffer[0]<<8 |buffer[1]; 

	#if defined(BUILD_UBOOT)||defined(BUILD_LK)
		//printf("OTM8018B uboot %s \n", __func__);
		//printf("%s id = 0x%08x \n", __func__, id);
		printf("OTM8018B  id = 0x%08x \n", __func__, id);
	#else
		//printk("OTM8018B kernel %s \n", __func__);
		//printk("%s id = 0x%08x \n", __func__, id);
		printk("OTM8018B 0x%x , 0x%x , 0x%x \n",buffer[0],buffer[1],id);
	#endif

	return 1;//(id == LCM_ID_OTM8018B)?1:0;
#endif/*TINNO_ANDROID_S7810*/
}

#if !defined(BUILD_UBOOT) && !defined(BUILD_LK)
extern void Tinno_Open_Mipi_HS_Read();
extern void Tinno_Close_Mipi_HS_Read();
static unsigned int lcm_esd_check(void)
{
	unsigned char esd_tag1 = 0, esd_tag2 = 0, esd_tag3 = 0, esd_tag4 = 0;
	unsigned int data_array[16];
	unsigned int max_return_size = 1;
#if 0
	static unsigned int lcm_esd_test = 1;      ///only for ESD test
	if(lcm_esd_test++%2 == 0)
	{
		return 1;
	}
#endif
	Tinno_Open_Mipi_HS_Read();
	data_array[0]= 0x00003700 | (max_return_size << 16);    
	dsi_set_cmdq(&data_array, 1, 1);
		
	read_reg_v2(0x0A, &esd_tag1, 1);
	read_reg_v2(0x0B, &esd_tag2, 1);
	read_reg_v2(0x0C, &esd_tag3, 1);
	read_reg_v2(0x0D, &esd_tag4, 1);
	Tinno_Close_Mipi_HS_Read();

	if( esd_tag1 == 0x9C 
		&& esd_tag2 == 0x00 
		   && esd_tag3 == 0x07 
		   && esd_tag4 == 0x00)
	{
		return 0;
	}
	else
	{            
		printk("lcm_esd_check esd_tag=0x%x,0x%x,0x%x,x%x\n", esd_tag1, esd_tag2, esd_tag3, esd_tag4);     
		return 1;
	}
}

static unsigned int lcm_esd_recover(void)
{
	printk("lcm_esd_recover ...\n");       
	lcm_init();
	return 1;
}
#endif

LCM_DRIVER otm8018b_dsi_vdo_lcm_drv = 
{
    .name			= "otm8018b_vdo_boe",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,	
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
#if !defined(BUILD_UBOOT) && !defined(BUILD_LK)
	.esd_check   = lcm_esd_check,
	.esd_recover   = lcm_esd_recover,
	.get_initialization_settings = get_initialization_settings,
	.set_initialization_settings = set_initialization_settings,
#endif
};

