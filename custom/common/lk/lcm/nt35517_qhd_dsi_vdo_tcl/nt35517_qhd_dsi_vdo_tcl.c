#ifdef BUILD_LK
#else
#include <linux/string.h>
#endif

#include "lcm_drv.h"
//yufeng
#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	#include <mach/mt_gpio.h>
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(540)
#define FRAME_HEIGHT 										(960)

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xFD   // END OF REGISTERS MARKER

//#define LCM_DSI_CMD_MODE									1

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

struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] =  {
	//By NTK//
	{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},
	//{0xB1,3,{0x7C,0x00,0x00}},
	{0xB1,3,{0x7C,0x04,0x00}},
	{0xB8,4,{0x01,0x02,0x02,0x02}},
	{0xBB,3,{0x63,0x03,0x63}},
	//{0xC9,6,{0x51,0x06,0x0D,0x1A,0x17,0x00}},
	{0xC9,6,{0x61,0x06,0x0D,0x1A,0x17,0x00}},
	//---------------------------
	// CMD2, Page 1
	//---------------------------
	{0xF0,5,{0x55,0xAA,0x52,0x08,0x01}},
	{0xB0,3,{0x05, 0x05, 0x05}},
	{0xB1,3,{0x05, 0x05, 0x05}},
	{0xB2,3,{0x01, 0x01, 0x01}},
	{0xB3,3,{0x0E, 0x0E, 0x0E}},
	{0xB4,3,{0x0A, 0x0A, 0x0A}},
	{0xB6,3,{0x44,0x44,0x44}},
	{0xB7,3,{0x34,0x34,0x34}},
	{0xB8,3,{0x24, 0x24, 0x24}},//02
	{0xB9,3,{0x26, 0x26, 0x26}},//26
	{0xBA,3,{0x24, 0x24, 0x24}},
	// VCOM
	{0xBC,3,{0x00,0xA0,0x00}},//A0
	{0xBD,3,{0x00, 0xA0, 0x00}},//A0
	{0xBE,1,{0x38}},
	{0xBF,1,{0x38}},//0x3b REVERSE SCAN 92
	{0xC0,2,{0x00, 0x08}},
	{0xCA,1,{0x00}},
	//---------------------------
	// CMD2, Page 1
	//---------------------------
	{0xD0,4,{0x0A,0x10,0x0D,0x0F}},
	
	{0xD1,16,{0x00,0x15,0x00,0x40,0x00,0x6D,0x00,0x8C,0x00,0x9E,0x00,0xCB,0x00,0xEC,0x01,0x1C}},
	{0xD2,16,{0x01,0x40,0x01,0x7B,0x01,0xA8,0x01,0xED,0x02,0x26,0x02,0x28,0x02,0x5F,0x02,0x9B}},
	{0xD3,16,{0x02,0xC2,0x02,0xF7,0x03,0x1D,0x03,0x4F,0x03,0x72,0x03,0x9C,0x03,0xB4,0x03,0xD6}},
	{0xD4,4,{0x03,0xF6,0x03,0xFF}},
	
	{0xD5,16,{0x00,0x15,0x00,0x40,0x00,0x6D,0x00,0x8C,0x00,0x9E,0x00,0xCB,0x00,0xEC,0x01,0x1C}},
	{0xD6,16,{0x01,0x40,0x01,0x7B,0x01,0xA8,0x01,0xED,0x02,0x26,0x02,0x28,0x02,0x5F,0x02,0x9B}},
	{0xD7,16,{0x02,0xC2,0x02,0xF7,0x03,0x1D,0x03,0x4F,0x03,0x72,0x03,0x9C,0x03,0xB4,0x03,0xD6}},
	{0xD8,4,{0x03,0xF6,0x03,0xFF}},
	
	{0xD9,16,{0x00,0x15,0x00,0x40,0x00,0x6D,0x00,0x8C,0x00,0x9E,0x00,0xCB,0x00,0xEC,0x01,0x1C}},
	{0xDD,16,{0x01,0x40,0x01,0x7B,0x01,0xA8,0x01,0xED,0x02,0x26,0x02,0x28,0x02,0x5F,0x02,0x9B}},
	{0xDE,16,{0x02,0xC2,0x02,0xF7,0x03,0x1D,0x03,0x4F,0x03,0x72,0x03,0x9C,0x03,0xB4,0x03,0xD6}},
	{0xDF,4,{0x03,0xF6,0x03,0xFF}},
	
	{0xE0,16,{0x00,0x15,0x00,0x40,0x00,0x6D,0x00,0x8C,0x00,0x9E,0x00,0xCB,0x00,0xEC,0x01,0x1C}},
	{0xE1,16,{0x01,0x40,0x01,0x7B,0x01,0xA8,0x01,0xED,0x02,0x26,0x02,0x28,0x02,0x5F,0x02,0x9B}},
	{0xE2,16,{0x02,0xC2,0x02,0xF7,0x03,0x1D,0x03,0x4F,0x03,0x72,0x03,0x9C,0x03,0xB4,0x03,0xD6}},
	{0xE3,4,{0x03,0xF6,0x03,0xFF}},
	
	{0xE4,16,{0x00,0x15,0x00,0x40,0x00,0x6D,0x00,0x8C,0x00,0x9E,0x00,0xCB,0x00,0xEC,0x01,0x1C}},
	{0xE5,16,{0x01,0x40,0x01,0x7B,0x01,0xA8,0x01,0xED,0x02,0x26,0x02,0x28,0x02,0x5F,0x02,0x9B}},
	{0xE6,16,{0x02,0xC2,0x02,0xF7,0x03,0x1D,0x03,0x4F,0x03,0x72,0x03,0x9C,0x03,0xB4,0x03,0xD6}},
	{0xE7,4,{0x03,0xF6,0x03,0xFF}},
	
	{0xE8,16,{0x00,0x15,0x00,0x40,0x00,0x6D,0x00,0x8C,0x00,0x9E,0x00,0xCB,0x00,0xEC,0x01,0x1C}},
	{0xE9,16,{0x01,0x40,0x01,0x7B,0x01,0xA8,0x01,0xED,0x02,0x26,0x02,0x28,0x02,0x5F,0x02,0x9B}},
	{0xEA,16,{0x02,0xC2,0x02,0xF7,0x03,0x1D,0x03,0x4F,0x03,0x72,0x03,0x9C,0x03,0xB4,0x03,0xD6}},
	{0xEB,4,{0x03,0xF6,0x03,0xFF}},
	
	//NT50198 MODE
	{0xC0,1,{0xC0}},
	{0xC2,1,{0x20}},
	//DelayX1ms(5);
	//---------------------------
	
	//---------------------------
	{0xFF,4,{0xAA,0x55,0x25,0x01}},
	{0x6F,1,{0x0B}},
	{0xF4,4,{0x12,0x12,0x56,0x13}},
	
	
		{0x11, 1, {0x00}},
		{REGFLAG_DELAY,150, {}},
	
		// Display ON
		{0x29, 1, {0x00}},
		{REGFLAG_DELAY, 10, {}},
		{REGFLAG_END_OF_TABLE, 0x00, {}}
	
};


//edit by Magnum 2012-12-18
static void dsi_send_cmdq_tinno(unsigned cmd, unsigned char count, unsigned char *para_list, unsigned char force_update)
{
	unsigned int item[16];
	unsigned char dsi_cmd = (unsigned char)cmd;
	unsigned char dc;
	int index = 0, length = 0;
	
	memset(item,0,sizeof(item));
	if(count+1 > 60)
	{
		//LCM_DBG("//Exceed 16 entry\n");
		return;
	}
/*
	Data ID will depends on the following rule.
	
		count of parameters > 1	=> Data ID = 0x39
		count of parameters = 1	=> Data ID = 0x15
		count of parameters = 0	=> Data ID = 0x05
*/	
	if(count == 0)
	{
		item[0] = 0x0500 | (dsi_cmd<<16);
		length = 1;
	}
	else if(count == 1)
	{
		item[0] = 0x1500 | (dsi_cmd<<16) | (para_list[0]<<24);
		length = 1;
	}
	else
	{
		item[0] = 0x3902 | ((count+1)<<16);//Count include command.
		++length;
		while(1)
		{
			if (index == count+1)
				break;
			if ( 0 == index ){
				dc = cmd;
			}else{
				dc = para_list[index-1];
			}
			// an item make up of 4data. 
			item[index/4+1] |= (dc<<(8*(index%4)));  
			if ( index%4 == 0 ) ++length;
			++index;
		}
	}
	
	dsi_set_cmdq(&item, length, force_update);

}


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
				dsi_send_cmdq_tinno(cmd, table[i].count, table[i].para_list, force_update);
		//	dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
       	}
    }
	
}


static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
};

static void lcm_get_params(LCM_PARAMS *params)
{
// NT35517LG550
	memset(params, 0, sizeof(LCM_PARAMS));
	params->type   = LCM_TYPE_DSI;
	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;
	params->dsi.mode					= SYNC_PULSE_VDO_MODE;
	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				= LCM_TWO_LANE;//LCM_FOUR_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq	= LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding 	= LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format		= LCM_DSI_FORMAT_RGB888;

	// Highly depends on LCD driver capability.
	// Not support in MT6573
	params->dsi.packet_size=256;
	// Video mode setting
	params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.word_count=540*3;

	params->dsi.vertical_sync_active	= 2;//2
	params->dsi.vertical_backporch		= 10;//8
	params->dsi.vertical_frontporch 	= 10;//6
	params->dsi.vertical_active_line	= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active	= 10;//60
	params->dsi.horizontal_backporch	= 90;//60
	params->dsi.horizontal_frontporch	= 90;//107
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;

	// Bit rate calculation
	//params->dsi.pll_div1=0; //0-26-702M // div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
	//params->dsi.pll_div2=1; 	// div2=0,1,2,3;div1_real=1,2,4,4
	//params->dsi.fbk_div =19; //37=494M	 // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)

	params->dsi.PLL_CLOCK = 273;

}



static void lcm_reset(void)
{
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(5);
	SET_RESET_PIN(1);
	MDELAY(120);
}
static void lcm_init(void)
{
	lcm_reset();
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
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
	dsi_set_cmdq(&data_array, 3, 1);
	//MDELAY(1);
	
	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(&data_array, 3, 1);
	//MDELAY(1);
	
	data_array[0]= 0x00290508;
	dsi_set_cmdq(&data_array, 1, 1);
	//MDELAY(1);
	
	data_array[0]= 0x002c3909;
	dsi_set_cmdq(&data_array, 1, 0);
	//MDELAY(1);

}

static void lcm_suspend(void)
{
	unsigned int data_array[16];
	data_array[0]=0x00280500;
	dsi_set_cmdq(&data_array,1,1);
	MDELAY(10);
	data_array[0]=0x00100500;
	dsi_set_cmdq(&data_array,1,1);
	MDELAY(120);
}

static void lcm_resume(void)
{
	unsigned int data_array[16];

	lcm_reset();
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);

}

extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);
static unsigned int lcm_compare_id(void)
{
        unsigned int id = 0;
        unsigned char buffer[2];
        unsigned int array[16];

        int Channel=0;
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
    
        push_table(lcm_compare_id_setting, sizeof(lcm_compare_id_setting) / sizeof(struct LCM_setting_table), 1);
    
        array[0] = 0x00023700;// read id return two byte,version and id
        dsi_set_cmdq(array, 1, 1);
        //id = read_reg(0xF4);
        read_reg_v2(0xF4, buffer, 2);
        id = buffer[0]; //we only need ID
        */
        // Read ADC value
        
        #ifdef BUILD_LK
           printf("LINE=%d %s, res=%d,adcVol = %d ,we are using tcl lcd \n", __LINE__,__func__,res,adcVol); 
        #else
           printk("LINE=%d %s, id1 = 0x%08x\n",__LINE__,__func__, id);   
        #endif
		
       if(adcVol>=450&&adcVol<1500)
       {
           #ifdef BUILD_LK
           printf("we are using tcl lcd \n"); 
        #else
           printk("we are using tcl lcd \n");   
        #endif

            return 1;
       }
       else
       {
            return 0;
       }
}


LCM_DRIVER tcl_nt35517_qhd_dsi_vdo_lcm_drv = 
{
	.name			= "nt35517_qhd_vdo_tcl",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
	.update         = lcm_update,
#endif
};
