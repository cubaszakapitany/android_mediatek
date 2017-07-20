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
#define FRAME_HEIGHT (800)
#define FALSE (0)
#define TRUE (1)
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

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

//static kal_uint16 Vcom=0x98;
static struct LCM_setting_table lcm_initialization_setting[] = {
	
        {0xFF,3,{0xff,0x98,0x16}},
        {0xBA,1,{0x60}},  //SPI Interface setting
        {0xB0,1,{0x01}},
        {0xB6,1,{0x22}},
        {0xBC,23,{0x03,0x0D,0x63,0x31,0x01,0x01,0x1B,0x10,0x37,0x13,
                       0x10,0x37,0x32,0x32,0x19,0x00,0xFF,0xE2,0x01,0x05,
                       0x05,/*0x10,0x10,*/0x43,0x0B}},
        {0xBD,8,{0x01,0x23,0x45,0x67,0x01,0x23,0x45,0x67}},
        {0xBE,17,{0x00,0x22,0x22,0x22,0x97,0x86,0xCA,0xDB,0xAC,0xBD,
                       0x68,0x79,0x22,0x22,0x22,0x22,0x22}},
        {0xED,2,{0x7F,0x0F}},
        {0xF3,1,{0x70}},
        {0xB4,1,{0x02}},
        {0xC0,3,{0x9f,0x0B,0x0A}},
        {0xC1,4,{0x17,0x98,0x92,0x20}},
        {0xD8,1,{0x50}},
        {0xFC,1,{0x08}},
		{0xE0,16,{0x00,0x0C,0x24,0x13,0x15,0x1F,0xCB,0x08,0x04,0x08,
					   0x03,0x0F,0x13,0x23,0x17,0x00}},
        {0xE1,16,{0x00,0x09,0x16,0x11,0x13,0x17,0x79,0x08,0x03,0x08,
                       0x06,0x0B,0x08,0x24,0x14,0x00}},
 //       {0xE0,16,{0x00,0x0C,0x23,0x13,0x15,0x1F,0xCB,0x08,0x04,0x08,
 //                      0x03,0x0F,0x13,0x26,0x24,0x00}},
 //       {0xE1,16,{0x00,0x09,0x15,0x11,0x13,0x17,0x79,0x08,0x03,0x08,
 //                      0x06,0x0B,0x08,0x27,0x21,0x00}},
        {0xD5,8,{0x0D,0x0A,0x0A,0x0A,0xCB,0xA5,0x01,0x04}},
        {0xF7,1,{0x8A}},
        {0xC7,1,{0x87}},
        //{0x3A,1,{0x77}},
        //{0x36,1,{0x08}},
        //{0x21,0,{}},
        {0xD6,8,{0xff,0xA0,0x88,0x14,0x04,0x64,0x28,0x3a}},  //0x1A 0x3a(esd enhance IC)
        {0x2A,4,{0x00, 0x00, 0x01,0xDF}},
	 {0x2B,4,{0x00, 0x00, 0x03,0x1F}},
	 {0x2C,0,{}},
	 
	{0x11,0,{}},
	{REGFLAG_DELAY, 150, {}},
        {0xEE,9,{0x0A,0x1B,0x5F,0x40,0x00,0x00,0x10,0x00,0x58}},
	{0x29,0,{}},
    {REGFLAG_DELAY, 50, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_set_window[] = {
	{0x2A,	4,	{0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
	{0x2B,	4,	{0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


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
  
    lcm_reset();
    push_table_tkc(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{
	lcm_reset();
//	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
	//SET_RESET_PIN(0);
	//MDELAY(1);
	//SET_RESET_PIN(1);
}

//unsigned int vcomf=0xa0;
static void lcm_resume(void)
{
	lcm_init();
/*
	unsigned int data_array[16];

	data_array[0]= 0x00043902;
	data_array[1]= 0x7983ffb9;

	dsi_set_cmdq(&data_array, 2, 1);

    	data_array[0]= 0x00053902;
	data_array[1]= 0x00a000b6; // a0 a7 aa vcomf
	data_array[1]=data_array[1]|(vcomf<<16);
	data_array[2]= 0x000000a0;
	dsi_set_cmdq(&data_array, 3, 1);

      vcomf=vcomf+3;
     */
	//push_table_tkc(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
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

static void lcm_setbacklight(unsigned int level)
{
	// Refresh value of backlight level.
	lcm_backlight_level_setting[0].para_list[0] = level;

	push_table_tkc(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_setpwm(unsigned int divider)
{
	// TBD
}


static unsigned int lcm_getpwm(unsigned int divider)
{
	// ref freq = 15MHz, B0h setting 0x80, so 80.6% * freq is pwm_clk;
	// pwm_clk / 255 / 2(lcm_setpwm() 6th params) = pwm_duration = 23706
	unsigned int pwm_clk = 23706 / (1<<divider);	
	return pwm_clk;
}
static unsigned int lcm_esd_check(void)
{

  #ifndef BUILD_LK
    unsigned char buffer[1];
    unsigned int array[16];
    unsigned char print_buffer[4];

    static int once_time=0;
    
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

    read_reg_v2(0x0B, buffer, 1);
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

    read_reg_v2(0x0C, buffer, 1);
    #if defined(BUILD_LK)
    //printf("lcm_esd_check  0x0B = %x\n",buffer[0]);
    #else
    print_buffer[2] = buffer[0];
    //printk("lcm_esd_check  0x0C = %x\n",buffer[0]);
    #endif
/*
    if(buffer[0] != 0x00)  
    {
        return TRUE;
    }
    */

    read_reg_v2(0x0D, buffer, 1);
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
    print_buffer[3] = buffer[0];
    printk("lcm_esd_check:%x,%x,%x,%x\n",print_buffer[0],print_buffer[1],print_buffer[2],print_buffer[3]);
    #endif

    if(0x9C==print_buffer[0]&&0x00==print_buffer[1]&&0x70==print_buffer[2]&&0x00==print_buffer[3])
       return FALSE;
    else
        return TRUE;
/*
    if(buffer[0] != 0x00)  
    {
        return TRUE;
    }
    return FALSE;
    */
    
/*
   array[0]= 0x00043700;// | (max_return_size << 16);    
        
        dsi_set_cmdq(array, 1, 1);

   read_reg_v2(0x09, buffer, 4);

   #if defined(BUILD_LK)
   printf("lcm_esd_check,buffer[0]=%x\n", buffer[0]);
   printf("lcm_esd_check,buffer[1]=%x\n", buffer[1]);
   printf("lcm_esd_check,buffer[2]=%x\n", buffer[2]);
   printf("lcm_esd_check,buffer[3]=%x\n", buffer[3]);
   #else
   printk("lcm_esd_check,buffer[0]=%x\n", buffer[0]);
   printk("lcm_esd_check,buffer[1]=%x\n", buffer[1]);
   printk("lcm_esd_check,buffer[2]=%x\n", buffer[2]);
   printk("lcm_esd_check,buffer[3]=%x\n", buffer[3]);
   #endif        
*/
   /*if((buffer[0] != 0x80)&&(buffer[1] != 0x73));//&&(buffer[2] != 0x37)&&(buffer[3] != 0x04));
   {
      //return TRUE;
   }*/

    
#endif


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
    push_table_tkc(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
    //MDELAY(10);
    push_table_tkc(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
    
    //MDELAY(10);
    //dsi_set_cmdq_V2(0x35, 1, &para, 1);     ///enable TE
    //MDELAY(10);
    
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
}


LCM_DRIVER ili9806c_wvga_dsi_vdo_drv_tkc = 
{
    .name			= "ili9806c_dsi_tkc",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.set_backlight	= lcm_setbacklight,
	.set_pwm        = lcm_setpwm,
	.get_pwm        = lcm_getpwm,
	.esd_check   = lcm_esd_check,
	.esd_recover   = lcm_esd_recover,
	.compare_id     = lcm_compare_id,
#if defined(LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
    };

