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

#define FRAME_WIDTH  (480)
#define FRAME_HEIGHT (854)
#define FALSE (0)
#define TRUE (1)
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
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)									lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)				lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)      

//static kal_uint16 Vcom=0x98;
static unsigned char lcm_initialization_setting[] = {
/*	cmd,		count,	params*/
	0xFF,	3,		0xff,0x98,0x16,					// EXTC Command Set enable register 
	0xBA,	1,		0x60,  							//SPI Interface setting
	0xB0,	1,		0x01,							// Interface Mode Control 
	0xBC,	18,		0x03,0x0D,0x03,0x61,0x01,
					0x01,0x1B,0x11,0x6E,0x00,
					0x00,0x00,0x01,0x01,0x16,
					0x00,0xFF,0xF2,					// GIP 1 
	0xBD,	8,		0x02,0x13,0x45,0x67,0x45,
					0x67,0x01,0x23,					// GIP 2
	0xBE,	17,		0x03,0x22,0x22,0x22,0x22,
					0xDD,0xCC,0xBB,0xAA,0x66,
					0x77,0x22,0x22,0x22,0x22,
					0x22,0x22,						// GIP 3
	0xED,	2,		0x7F,0x0F,						// en_volt_reg measure VGMP
	0xF3,	1,		0x70,
	0xB4,	1,		0x02,     							// Display Inversion Control    
	0xC0,	3,		0x0f,0x0B,0x0A,					// Power Control 1 
	0xC1,	4,		0x17,0x85,0x56,0x20,				// Power Control 2 
	0xD8,	1,		0x50,							// VGLO Selection 
	0xFC,	1,		0x07,							// VGLO Selection 
	0xE0,	16,		0x00,0x05,0x14,0x14,0x12,
					0x1A,0xCC,0x0C,0x03,0x07,
					0x03,0x0F,0x11,0x2C,0x26,
					0x00,							// Positive Gamma Control 
	0xE1,	16,		0x00,0x01,0x02,0x05,0x0E,
					0x10,0x75,0x03,0x06,0x0B,
					0x0A,0x0C,0x0B,0x33,0x2C,
					0x00,							// Negative Gamma Control 
	0xD5,	8,		0x0A,0x08,0x07,0x08,0xCB,
					0xA5,0x01,0x04,					// Source Timing Adjust 
	0xD6,   8,		0xFF,0xA0,0x88,0x14,0x04,
					0x64,0x28,0x1A,
	0xF7,	1,		0x89,							// Resolution 
	0xC7,	1,		0x57,							// Vcom 
	0x21,	0,

	REGFLAG_DELAY, 10,
	// Sleep Out
	0x11,	0, 
	REGFLAG_DELAY, 150,
	0xEE,   9,		0x0A,0x1B,0x5F,0x40,0x00,
					0x00,0x10,0x00,0x58,
	// Display ON
	0x29,	0,
	0x2C,	0,
	REGFLAG_DELAY, 20,
	
	REGFLAG_END_OF_TABLE
};

static unsigned char lcm_oFF[] = {
        0x28,	0,
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


static unsigned char lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	0x28,	0,
	REGFLAG_DELAY, 50,

	// Sleep Mode On
	0x10,	0, 
	REGFLAG_DELAY, 20,

	REGFLAG_END_OF_TABLE
};


static unsigned char lcm_compare_id_setting[] = {
	// Display off sequence
	0xB9,	3,	0xFF, 0x83, 0x69,
	REGFLAG_DELAY, 10,

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
	params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;

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
	params->dsi.force_dcs_packet=1;
	params->dsi.isFixIli9806cESD=1;

	// Video mode setting		
	params->dsi.intermediat_buffer_num = 2;

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active				= 4; // 3 2 18      4 6 6
	params->dsi.vertical_backporch					= 16; // 7 6 24
	params->dsi.vertical_frontporch					= 18; // 6, 6 24
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 

	params->dsi.horizontal_sync_active				= 10; // 12 12 32 32
	params->dsi.horizontal_backporch				= 60; //132 92 52 60
	params->dsi.horizontal_frontporch				= 60; //132 92 60 60  
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	// Bit rate calculation 416Mhz 
	params->dsi.pll_div1=1;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
	params->dsi.pll_div2=1;		// div2=0,1,2,3;div1_real=1,2,4,4	
	params->dsi.fbk_div =31;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
}

static void lcm_reset(void)
{
	SET_RESET_PIN(1);
	MDELAY(1);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(20);
}

static void lcm_init(void)
{
  
    lcm_reset();
    push_table(lcm_initialization_setting);
}

static void lcm_suspend(void)
{
//	push_table(lcm_deep_sleep_mode_in_setting);
	lcm_reset();
}

//unsigned int vcomf=0xa0;
static void lcm_resume(void)
{
	lcm_init();
      // push_table(lcm_oFF);
      
}

#if !defined(BUILD_UBOOT) && !defined(BUILD_LK)
static int get_initialization_settings(unsigned char table[])
{
	memcpy(table, lcm_initialization_setting, sizeof(lcm_initialization_setting));
	return sizeof(lcm_initialization_setting);
}

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

static unsigned int lcm_esd_check(void)
{

	unsigned char id0 = 0,id1 = 0,id2 = 0,id3 = 0;
	unsigned char buffer[4] = {0};
	unsigned int array[16];  
	unsigned int max_return_size = 4;
	int rc;
#if 0
	static unsigned int lcm_esd_test = 1;      ///only for ESD test
	if(lcm_esd_test++%2 == 0)
	{
		return 1;
	}
#endif
	max_return_size = 3;
	array[0]= 0x00003700 | (max_return_size << 16);    
	dsi_set_cmdq(&array, 1, 1);
	
	rc = read_reg_v2(0xD3, buffer, 3);
	if ( 0x00==buffer[0] && 0x98==buffer[1] && 0x16==buffer[2] ){
		
		max_return_size = 4;
		memset(buffer, 0, sizeof(buffer));
		array[0]= 0x00003700 | (max_return_size << 16);    
		dsi_set_cmdq(&array, 1, 1);
		
		rc = read_reg_v2(0x09, buffer, 4);
		id0 = buffer[0]; 
		id1 = buffer[1]; 
		id2 = buffer[2]; 
		id3 = buffer[3]; 
		/*0x80,0x73/0xB3,0x24,x00*/
		/*0xF8,0x0C        ,0x06,0xC0*/
		if( ((0x80==(0x80&id0)) && 
			(0x73==(0x73&id1)) && 
			(0x24==(0x24&id2))) /*|| (0xC0==id3)*/ ){
			memset(buffer, 0, sizeof(buffer));
			max_return_size = 1;
			array[0]= 0x00003700 | (max_return_size << 16);    
			dsi_set_cmdq(&array, 1, 1);
			
			rc = read_reg_v2(0x0A, buffer, 1);
			if ( 0x9c == (0x9c&buffer[0]) ){
				return FALSE;
			}
			printk("lcm_esd_check 0x0A=0x%x, rc=%d\n", buffer[0], rc);     
			return TRUE;
		}else{
			printk("lcm_esd_check esd_tag=0x%x,0x%x,0x%x,0x%x, rc=%d\n", id0, id1, id2, id3, rc);  
			return TRUE;
		}
	}else{
		printk("lcm_esd_check dev_id=0x%x,0x%x,0x%x, rc=%d\n", buffer[0], buffer[1], buffer[2], rc);     
		return TRUE;
	}
}

static unsigned int lcm_esd_recover(void)
{
	printk("%s, LINE = %d\n", __func__,__LINE__);  

	lcm_init();

	return TRUE;
}
#endif

// ---------------------------------------------------------------------------
//  Get LCM ID Information
// ---------------------------------------------------------------------------
extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);

static unsigned int lcm_compare_id()
{
#ifdef TINNO_ANDROID_S7812BP
        unsigned int id = 0;
        unsigned char buffer[2];
        unsigned int array[16];

        int Channel=1;
        int data[4];
        int adcVol=0;
        
        int res=IMM_GetOneChannelValue(Channel,data,0);
        adcVol=data[0]*1000+data[1]*10;
        
        /*
        SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
        SET_RESET_PIN(0);
        MDELAY(10);
        SET_RESET_PIN(1);
        MDELAY(10);
    
        push_table(lcm_compare_id_setting, sizeof(lcm_compare_id_setting) / sizeof(unsigned char), 1);
    
        array[0] = 0x00023700;// read id return two byte,version and id
        dsi_set_cmdq(array, 1, 1);
        //id = read_reg(0xF4);
        read_reg_v2(0xF4, buffer, 2);
        id = buffer[0]; //we only need ID
        */
        // Read ADC value
        
        #ifdef BUILD_LK
           printf("LINE=%d %s, res=%d,adcVol = %d \n", __LINE__,__func__,res,adcVol); 
        #else
           printk("LINE=%d %s, id1 = 0x%08x\n",__LINE__,__func__, id);   
        #endif
		
       if(adcVol>=1300)
       {
            return 1;
       }
       else
       {
            return 0;
       }
#else
	return 1;//Default DRV
#endif/*TINNO_ANDROID_S7810*/

}

LCM_DRIVER ili9806c_fwvga_dsi_vdo_txdkj_drv = 
{
	.name			= "ili9806c_dsi_txdkj",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
#if !defined(BUILD_UBOOT) && !defined(BUILD_LK)
	.esd_check   = lcm_esd_check,
	.esd_recover   = lcm_esd_recover,
	.get_initialization_settings = get_initialization_settings,
	.set_initialization_settings = set_initialization_settings,
#endif
};

