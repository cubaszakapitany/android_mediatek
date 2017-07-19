#ifdef BUILD_LK
  #include <platform/mt_typedefs.h>
  #include <platform/mt_gpio.h>
  #define LCM_PRINT printf
#else
  #include <linux/string.h>
  #include <linux/kernel.h>
  #include <mach/mt_gpio.h>
  #define LCM_PRINT printk
#endif
#include "lcm_drv.h"

#if 1
#define LCM_DBG(fmt, arg...) \
	LCM_PRINT("[LCM-ILI9806H-HALFRAM-DSI] %s (line:%d) :" fmt "\r\n", __func__, __LINE__, ## arg)
#else
#define LCM_DBG(fmt, arg...) do {} while (0)
#endif

#define LCM_DSI_CMD_MODE									1

//TN801031 added 2012.09.05
#define TINNO_INT
static unsigned int lcm_esd_test = 0;      ///only for ESD test
#define TINNO_INT_ESD        1

//TN801031 added 2012.09.05

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(480)
#define FRAME_HEIGHT 										(854)
#define FALSE (0)
#define TRUE (1)

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xFA   // END OF REGISTERS MARKER

#define LCM_ID_ILI9806H		0x9826
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
#define read_reg_v2(cmd,buffer,buffer_size)					lcm_util.dsi_dcs_read_lcm_reg_v2(cmd,buffer,buffer_size)


struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[360];
};
static struct LCM_setting_table lcm_initialization_setting[] = {
#if 1
            {0xFF,3,{0xff,0x98,0x26}},
            {0xDF,3,{0x06,0x22,0x20}},
            {0xCE,4,{0xa0,0x0e,0x01,0x02}},
            {0xB6,1,{0x42}},    //bypass RAM.0x42
            {0xBC,26,{0x21,0x06,0x00,0x00,0x01,0x01,0x80,0x02,0x05,0x00,
                           0x00,0x00,0x01,0x01,0xf0,0xf4,0x00,0x00,0x00,0xc0,0x08,
                           0x00,0x00,0x00,0x00,0x00}},
            {0xBD,8,{0x02,0x13,0x45,0x67,0x01,0x23,0x45,0x67}},
            {0xBE,17,{0x13,0x22,0x22,0x22,0x22,0xbb,0xaa,0xdd,0xcc,0x66,
                           0x77,0x22,0x22,0x22,0x22,0x22,0x22}},
            {0x3A,1,{0x77}},
            //{0x3A,1,{0x66}},

            {0xFA,5,{0x08,0x00,0x00,0x02,0x08}},

#if (TINNO_INT_ESD)
        {0xD6,8,{0xFF,0xA0,0x88,0x14,0x04,0x64,0x28,0x1A}},
#endif

#if TN_DISPLAY
        //FRAME RATE 60.18Hz (TE ON)//调速率
        //{0xB1,3,{0x00,0x58,0x03}},
        //{0xB1,3,{0x00,0x65,0x03}},//ok,no te.25fps
        //{0xB1,3,{0x00,0x65,0x03}},//ok
        {0xB1,3,{0x00,0x62,0x03}},//ok.27.5fps
#else
        {0xB1,3,{0x00,0x58,0x03}},
#endif
#if TN_DISPLAY
        //{0x44,2,{0x01,0x00}},//ok,no te
        {0x44,2,{0x01,0x00}},//切线位置的高8位 ,切线位置的低8位
#endif

        {0xC0,1,{0x0A}},
#if 1   //1017.调试第二批屏效果
        {0xB4,1,{0x00}},//设置翻转模式//0x00列翻转,0x02两点翻转
        {0xC1,3,{0x1A,0x78,0x78}},
        {0xC7,2,{0x2F,0x00}},//
#else   //第一批屏效果
        {0xB4,1,{0x00}},//设置翻转模式//0x00列翻转,0x02两点翻转
        {0xC1,3,{0x16,0x78,0x6A}},
        {0xC7,2,{0x35,0x00}},
#endif

        {0xED,2,{0x7f,0x0f}},

        {0xF2,6,{0x00,0x07,0x00,0x8e,0x0a,0x0a}},
        {0xF7,1,{0x01}},
//tinnohd.0928.色彩饱和度
        {0x55,1,{0xB0}},
//tinnohd.0928.色彩饱和度

#if 1 //gamma 2.2//第一批屏OK
        {0XE0,16,{0x00,0x03,0x06,0x0F,0x0D,0x15,0XCB,0x0A,0x03,0x07,
                       0x03,0x05,0x0A,0x3E,0x31,0x00}},
        {0XE1,16,{0x00,0x01,0x0E,0x0A,0x09,0x17,0x79,0x09,0x01,0x08,
                       0x07,0x02,0x0A,0x1F,0x21,0x00}},
#else //gamma 2.5
        {0XE0,16,{0x00,0x01,0x03,0x08,0x03,0x12,0XCb,0x0A,0x03,0x07,
                       0x06,0x05,0x09,0x3B,0x36,0x00}},
        {0XE1,16,{0x00,0x01,0x05,0x0B,0x06,0x15,0x7A,0x08,0x04,0x09,
                       0x06,0x05,0x09,0x3B,0x36,0x00}},
#endif

        {0x35,1,{0x00}},////tear effect on

        {0x21,0,{}},////

        {0x11,0,{}},
        {REGFLAG_DELAY, 120, {}},
        {0x29,0,{}},
        {REGFLAG_DELAY, 50, {}},

#else //old code
            {0xFF,3,{0xff,0x98,0x26}},
            {0xDF,3,{0x06,0x22,0x20}},
            {0xB6,1,{0x42}},    //bypass RAM.0x42
            {0xBC,26,{0x21,0x06,0x00,0x00,0x01,0x01,0x80,0x02,0x05,0x00,
                           0x00,0x00,0x01,0x01,0xf0,0xf4,0x00,0x00,0x00,0xc0,0x08,
                           0x00,0x00,0x00,0x00,0x00}},
            {0xBD,8,{0x02,0x13,0x45,0x67,0x01,0x23,0x45,0x67}},
            {0xBE,17,{0x13,0x22,0x22,0x22,0x22,0xbb,0xaa,0xdd,0xcc,0x66,
                           0x77,0x22,0x22,0x22,0x22,0x22,0x22}},

            {0x3A,1,{0x77}},
            //{0x3A,1,{0x66}},

            {0xFA,5,{0x08,0x00,0x00,0x02,0x08}},

#if (TINNO_INT_ESD)
        {0xD6,8,{0xFF,0xA0,0x88,0x14,0x04,0x64,0x28,0x1A}},
#endif

#if TN_DISPLAY
        //FRAME RATE 60.18Hz (TE ON)//调速率
        //{0xB1,3,{0x00,0x58,0x03}},
        //{0xB1,3,{0x00,0x65,0x03}},//ok,no te.25fps
        //{0xB1,3,{0x00,0x65,0x03}},//ok
        {0xB1,3,{0x00,0x62,0x03}},//ok.27.5fps
#else
        {0xB1,3,{0x00,0x58,0x03}},
#endif
#if TN_DISPLAY
        //{0x44,2,{0x01,0x00}},//ok,no te
        {0x44,2,{0x01,0x00}},//切线位置的高8位 ,切线位置的低8位
#endif

        {0xB4,1,{0x02}},
        {0xC1,3,{0x15,0x78,0x6A}},
        {0xC7,2,{0x2f,0x00}},

        {0xED,2,{0x7f,0x0f}},

        {0xF2,6,{0x00,0x07,0x00,0x8e,0x0a,0x0a}},
        {0xF7,1,{0x01}},

//tinnohd.add by tcp.display effect
#if TN_DISPLAY
        {0XE0,16,{0x00,0x03,0x0b,0x0f,0x12,0x17,0XCb,0x0b,0x02,0x07,
                       0x06,0x0b,0x0a,0x34,0x30,0x00}},
        {0XE1,16,{0x00,0x03,0x0a,0x0e,0x12,0x16,0x7A,0x07,0x03,0x07,
                       0x07,0x0b,0x0c,0x27,0x22,0x00}},
#else
        {0XE0,16,{0x00,0x06,0x15,0x11,0x12,0x1C,0XCA,0x08,0x02,0x08,
                       0x02,0x0D,0x0B,0x36,0x31,0x00}},
        {0XE1,16,{0x00,0x05,0x0D,0x10,0x12,0x16,0x79,0x07,0x05,0x09,
                       0x07,0x0C,0x0B,0x21,0x1B,0x00}},
#endif
//tinnohd.add by tcp
        {0x35,1,{0x00}},////tear effect on

        {0x21,0,{}},////

        {0x11,0,{}},
        {REGFLAG_DELAY, 120, {}},
        {0x29,0,{}},
        {REGFLAG_DELAY, 50, {}},
#endif
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 0, {}},
        {REGFLAG_DELAY, 10, {}},
	{0x10, 0, {}},
        {REGFLAG_DELAY, 120, {}},
};


static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 0, {}},
	{REGFLAG_DELAY, 10, {}},
	{0x29, 0, {}},
	{REGFLAG_DELAY, 120, {}},
};

//tinnohd
static void push_table_tkc(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
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
//tinnohd

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
       params->dbi.te_mode                 = LCM_DBI_TE_MODE_VSYNC_ONLY;//LCM_DBI_TE_MODE_DISABLED;//LCM_DBI_TE_MODE_VSYNC_ONLY; 
       params->dbi.te_edge_polarity        = LCM_POLARITY_RISING;//LCM_POLARITY_FALLING;//LCM_POLARITY_RISING   TINNO_INT test

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
	params->dsi.force_dcs_packet=1;
	params->dsi.isFixIli9806cESD=1;

	// Video mode setting		
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.word_count=480*3;//480*3;	
	params->dsi.vertical_sync_active=3;
	params->dsi.vertical_backporch=3;
	params->dsi.vertical_frontporch=3;
	params->dsi.vertical_active_line=FRAME_HEIGHT;//800

	params->dsi.line_byte=2180;		// 2256 = 752*3
	params->dsi.horizontal_sync_active_byte=32;
	params->dsi.horizontal_backporch_byte=32;
	params->dsi.horizontal_frontporch_byte=32;	
	params->dsi.rgb_byte=(480*3+6);	

	params->dsi.horizontal_sync_active_word_count=20;	
	params->dsi.horizontal_backporch_word_count=200;
	params->dsi.horizontal_frontporch_word_count=200;

	//edit by Magnum 2012-7-4 try to solve lcd not updating....
        params->dsi.HS_TRAIL=0x08; 
        params->dsi.HS_ZERO=0x07; 
        params->dsi.HS_PRPR=0x03; 
        params->dsi.LPX=0x04; 

        params->dsi.TA_SACK=0x01; 
        params->dsi.TA_GET=0x14; 
        params->dsi.TA_SURE=0x06;         
        params->dsi.TA_GO=0x10; 

        params->dsi.CLK_TRAIL=0x04; 
        params->dsi.CLK_ZERO=0x0f;         
        params->dsi.LPX_WAIT=0x01; 
        params->dsi.CONT_DET=0x00; 

        params->dsi.CLK_HS_PRPR=0x04; 

	// Bit rate calculation 416Mhz 
	params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
	params->dsi.pll_div2=1;		// div2=0,1,2,3;div1_real=1,2,4,4	
	params->dsi.fbk_div =13;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
}

static void lcm_reset(void)
{
    //************* Reset LCD Driver ****************// 
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
    push_table_tkc(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
    push_table_tkc(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_resume(void)
{
    push_table_tkc(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
  //  LCM_DBG("UPDATE-UPDATE-UPDATE");
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

#if 0
	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	data_array[3]= 0x00053902;
	data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[5]= (y1_LSB);
	data_array[6]= 0x002c3909;
	dsi_set_cmdq(&data_array, 7, 0);
#else
	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);
#endif
//LCM_DBG("tcp_test:lcm_update\n");
}

#if 0
static unsigned int check_esd(void)
{
    unsigned char buffer_1[2];
    unsigned char buffer_2[2];
    unsigned char buffer_3[2];
    unsigned char buffer_4[2];
    unsigned int array[16];
    unsigned int ret=1;
/*
    array[0] = 0x00023700;// set return byte number
    dsi_set_cmdq(array, 1, 1);
*/
//Ｒ０９Ｈ是显示状态寄存器，不同平台不同接口有不同的值，以你读出来的值为准。
//0x09跟扫描方向有关
    array[0] = 0x00023700;// set return byte number
    dsi_set_cmdq(array, 1, 1);
    read_reg_V2(0x09, &buffer_1, 1);//work status
    if(buffer_1[0] == 0x80){//0x80,0x63,0x50
	array[0] = 0x00023700;// set return byte number
	dsi_set_cmdq(array, 1, 1);
        read_reg_V2(0x0A, &buffer_2, 1);//voltage track
        if(buffer_2[0] == 0x9c){
            ret = 0;
        }
    }
LCM_DBG("tcp_test_esd:esddddddd=%x,%x\n",buffer_1[0],buffer_2[0]);
    return ret;
    //return (buffer_1[0] == 0x80)?0:1;
}
#endif

#if !defined(BUILD_UBOOT) && !defined(BUILD_LK)
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
    lcm_resume();
	return TRUE;
}
#endif

// ---------------------------------------------------------------------------
//  Get LCM ID Information
// ---------------------------------------------------------------------------
extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);

static unsigned int lcm_compare_id()
{
#if 0//ndef TINNO_ANDROID_S7810
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

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER ili9806h_fwvga_dsi_vdo_txd_drv = 
{
	.name			= "ili9806h_dsi_txd",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id		= lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
	.update         = lcm_update,
#endif
#if !defined(BUILD_UBOOT) && !defined(BUILD_LK)
//	.esd_check   = lcm_esd_check,
//  .esd_recover   = lcm_esd_recover,
#endif
};
