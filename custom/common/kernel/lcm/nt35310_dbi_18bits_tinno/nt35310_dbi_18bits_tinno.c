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

#define FRAME_WIDTH  (320)
#define FRAME_HEIGHT (480)
#define LCM_ID       (0x5310)

#ifdef BUILD_LK
#define LCM_PRINT printf
#else
#if defined(BUILD_UBOOT)
#define LCM_PRINT printf
#else
#define LCM_PRINT printk
#endif
#endif

#define LCM_PRINT_FUNC()  LCM_PRINT("LCM35310_TCL" "`%s:%d [%s] " "\n", __FILE__, __LINE__, __FUNCTION__)

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

static __inline void send_ctrl_cmd(unsigned int cmd)
{
    lcm_util.send_cmd(cmd);
}


static __inline void send_data_cmd(unsigned int data)
{
    lcm_util.send_data(data&0xff);
}


static __inline unsigned int read_data_cmd()
{
    return lcm_util.read_data();
}


static __inline void set_lcm_register(unsigned int regIndex,
        unsigned int regData) {
    send_ctrl_cmd(regIndex);
    send_data_cmd(regData);
}

static void init_lcm_registers(void)
{
LCM_PRINT_FUNC();
send_ctrl_cmd(0xED);
send_data_cmd(0x01);
send_data_cmd(0xFE);
send_ctrl_cmd(0xEE);
send_data_cmd(0xDE);
send_data_cmd(0x21); 

send_ctrl_cmd(0xF1);
send_data_cmd(0x01); 
send_ctrl_cmd(0xDF);
send_data_cmd(0x10);

//  VCOM voltage //
send_ctrl_cmd(0xC4);
send_data_cmd(0x90); //94//90

send_ctrl_cmd(0xB7);
send_data_cmd(0x00);

//  2 dot inversion  //
send_ctrl_cmd(0xB4);
send_data_cmd(0x0D);

//  GVDDP/N = 5V  //
send_ctrl_cmd(0xC0);
send_data_cmd(0x58);
send_data_cmd(0x58);
send_data_cmd(0x20);
send_data_cmd(0x20);

send_ctrl_cmd(0xC2);
send_data_cmd(0x44);
send_data_cmd(0x44);
send_data_cmd(0x44);
send_ctrl_cmd(0xC6);
send_data_cmd(0x00);
send_data_cmd(0xE2);
send_data_cmd(0xE2);
send_data_cmd(0xE2);
send_ctrl_cmd(0xBF);
send_data_cmd(0xAA); 
send_ctrl_cmd(0xB1);
send_data_cmd(0x80);
send_data_cmd(0x00);
send_data_cmd(0x8B);
send_data_cmd(0x00);
send_data_cmd(0x96);
send_data_cmd(0x00);  
send_ctrl_cmd(0xB2);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_ctrl_cmd(0xB7);
send_data_cmd(0x3E);
send_data_cmd(0x00);
send_data_cmd(0x5E);
send_data_cmd(0x00);
send_data_cmd(0x9E);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x8C);
send_data_cmd(0x00);
send_data_cmd(0xAC);
send_data_cmd(0x00);
send_data_cmd(0xDC);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0xB9);
send_data_cmd(0x00);
send_data_cmd(0xEC);
send_data_cmd(0x00);
send_data_cmd(0xDC);
send_data_cmd(0x00);
send_ctrl_cmd(0xB8);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00); 
send_ctrl_cmd(0xC2);
send_data_cmd(0x0A);
send_data_cmd(0x00);
send_data_cmd(0x04);
send_data_cmd(0x00); 
send_ctrl_cmd(0xC7);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00); 
send_ctrl_cmd(0xC9);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00); 

send_ctrl_cmd(0xE0);
send_data_cmd(0x01);
send_data_cmd(0x00);
send_data_cmd(0x0A);
send_data_cmd(0x00);
send_data_cmd(0x22);
send_data_cmd(0x00);
send_data_cmd(0x43);
send_data_cmd(0x00);
send_data_cmd(0x56);
send_data_cmd(0x00);
send_data_cmd(0x66);
send_data_cmd(0x00);
send_data_cmd(0x7A);
send_data_cmd(0x00);
send_data_cmd(0x90);
send_data_cmd(0x00);
send_data_cmd(0xA1);
send_data_cmd(0x00);
send_data_cmd(0xAE);
send_data_cmd(0x00);
send_data_cmd(0xBA);
send_data_cmd(0x00);
send_data_cmd(0xC8);
send_data_cmd(0x00);
send_data_cmd(0xD0);
send_data_cmd(0x00);
send_data_cmd(0xD5);
send_data_cmd(0x00);
send_data_cmd(0xDB);
send_data_cmd(0x00);
send_data_cmd(0xE0);
send_data_cmd(0x00);
send_data_cmd(0xE7);
send_data_cmd(0x00);
send_data_cmd(0xF3);
send_data_cmd(0x00); 
send_ctrl_cmd(0xE1);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x0A);
send_data_cmd(0x00);
send_data_cmd(0x22);
send_data_cmd(0x00);
send_data_cmd(0x43);
send_data_cmd(0x00);
send_data_cmd(0x56);
send_data_cmd(0x00);
send_data_cmd(0x66);
send_data_cmd(0x00);
send_data_cmd(0x7A);
send_data_cmd(0x00);
send_data_cmd(0x90);
send_data_cmd(0x00);
send_data_cmd(0xA1);
send_data_cmd(0x00);
send_data_cmd(0xAE);
send_data_cmd(0x00);
send_data_cmd(0xBA);
send_data_cmd(0x00);
send_data_cmd(0xC8);
send_data_cmd(0x00);
send_data_cmd(0xD0);
send_data_cmd(0x00);
send_data_cmd(0xD5);
send_data_cmd(0x00);
send_data_cmd(0xDB);
send_data_cmd(0x00);
send_data_cmd(0xE0);
send_data_cmd(0x00);
send_data_cmd(0xE7);
send_data_cmd(0x00);
send_data_cmd(0xF3);
send_data_cmd(0x00);  

send_ctrl_cmd(0xE2);
send_data_cmd(0x01);
send_data_cmd(0x00);
send_data_cmd(0x0A);
send_data_cmd(0x00);
send_data_cmd(0x22);
send_data_cmd(0x00);
send_data_cmd(0x43);
send_data_cmd(0x00);
send_data_cmd(0x56);
send_data_cmd(0x00);
send_data_cmd(0x66);
send_data_cmd(0x00);
send_data_cmd(0x79);
send_data_cmd(0x00);
send_data_cmd(0x90);
send_data_cmd(0x00);
send_data_cmd(0xA1);
send_data_cmd(0x00);
send_data_cmd(0xAE);
send_data_cmd(0x00);
send_data_cmd(0xBA);
send_data_cmd(0x00);
send_data_cmd(0xC8);
send_data_cmd(0x00);
send_data_cmd(0xD0);
send_data_cmd(0x00);
send_data_cmd(0xD5);
send_data_cmd(0x00);
send_data_cmd(0xDB);
send_data_cmd(0x00);
send_data_cmd(0xE0);
send_data_cmd(0x00);
send_data_cmd(0xE7);
send_data_cmd(0x00);
send_data_cmd(0xF3);
send_data_cmd(0x00); 
send_ctrl_cmd(0xE3);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x0A);
send_data_cmd(0x00);
send_data_cmd(0x22);
send_data_cmd(0x00);
send_data_cmd(0x43);
send_data_cmd(0x00);
send_data_cmd(0x56);
send_data_cmd(0x00);
send_data_cmd(0x66);
send_data_cmd(0x00);
send_data_cmd(0x79);
send_data_cmd(0x00);
send_data_cmd(0x90);
send_data_cmd(0x00);
send_data_cmd(0xA1);
send_data_cmd(0x00);
send_data_cmd(0xAE);
send_data_cmd(0x00);
send_data_cmd(0xBA);
send_data_cmd(0x00);
send_data_cmd(0xC8);
send_data_cmd(0x00);
send_data_cmd(0xD0);
send_data_cmd(0x00);
send_data_cmd(0xD5);
send_data_cmd(0x00);
send_data_cmd(0xDB);
send_data_cmd(0x00);
send_data_cmd(0xE0);
send_data_cmd(0x00);
send_data_cmd(0xE7);
send_data_cmd(0x00);
send_data_cmd(0xF3);
send_data_cmd(0x00);  

send_ctrl_cmd(0xE4);
send_data_cmd(0x01);
send_data_cmd(0x00);
send_data_cmd(0x0A);
send_data_cmd(0x00);
send_data_cmd(0x22);
send_data_cmd(0x00);
send_data_cmd(0x43);
send_data_cmd(0x00);
send_data_cmd(0x56);
send_data_cmd(0x00);
send_data_cmd(0x66);
send_data_cmd(0x00);
send_data_cmd(0x7A);
send_data_cmd(0x00);
send_data_cmd(0x90);
send_data_cmd(0x00);
send_data_cmd(0xA1);
send_data_cmd(0x00);
send_data_cmd(0xAE);
send_data_cmd(0x00);
send_data_cmd(0xBA);
send_data_cmd(0x00);
send_data_cmd(0xC8);
send_data_cmd(0x00);
send_data_cmd(0xD0);
send_data_cmd(0x00);
send_data_cmd(0xD5);
send_data_cmd(0x00);
send_data_cmd(0xDB);
send_data_cmd(0x00);
send_data_cmd(0xE0);
send_data_cmd(0x00);
send_data_cmd(0xE7);
send_data_cmd(0x00);
send_data_cmd(0xF3);
send_data_cmd(0x00); 
send_ctrl_cmd(0xE5);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x0A);
send_data_cmd(0x00);
send_data_cmd(0x22);
send_data_cmd(0x00);
send_data_cmd(0x43);
send_data_cmd(0x00);
send_data_cmd(0x56);
send_data_cmd(0x00);
send_data_cmd(0x66);
send_data_cmd(0x00);
send_data_cmd(0x7A);
send_data_cmd(0x00);
send_data_cmd(0x90);
send_data_cmd(0x00);
send_data_cmd(0xA1);
send_data_cmd(0x00);
send_data_cmd(0xAE);
send_data_cmd(0x00);
send_data_cmd(0xBA);
send_data_cmd(0x00);
send_data_cmd(0xC8);
send_data_cmd(0x00);
send_data_cmd(0xD0);
send_data_cmd(0x00);
send_data_cmd(0xD5);
send_data_cmd(0x00);
send_data_cmd(0xDB);
send_data_cmd(0x00);
send_data_cmd(0xE0);
send_data_cmd(0x00);
send_data_cmd(0xE7);
send_data_cmd(0x00);
send_data_cmd(0xF3);
send_data_cmd(0x00); 

send_ctrl_cmd(0xE6);
send_data_cmd(0x44);
send_data_cmd(0x00);
send_data_cmd(0x55);
send_data_cmd(0x00);
send_data_cmd(0x66);
send_data_cmd(0x00);
send_data_cmd(0x99);
send_data_cmd(0x00);
send_data_cmd(0x88);
send_data_cmd(0x00);
send_data_cmd(0x77);
send_data_cmd(0x00);
send_data_cmd(0x77);
send_data_cmd(0x00);
send_data_cmd(0x99);
send_data_cmd(0x00);
send_data_cmd(0xAA);
send_data_cmd(0x00);
send_data_cmd(0x88);
send_data_cmd(0x00);
send_data_cmd(0x44);
send_data_cmd(0x00);
send_data_cmd(0x44);
send_data_cmd(0x00);
send_data_cmd(0x44);
send_data_cmd(0x00);
send_data_cmd(0x33);
send_data_cmd(0x00);
send_data_cmd(0x33);
send_data_cmd(0x00);
send_data_cmd(0x22);
send_data_cmd(0x00); 
send_ctrl_cmd(0xE7);
send_data_cmd(0x44);
send_data_cmd(0x00);
send_data_cmd(0x55);
send_data_cmd(0x00);
send_data_cmd(0x66);
send_data_cmd(0x00);
send_data_cmd(0x99);
send_data_cmd(0x00);
send_data_cmd(0x88);
send_data_cmd(0x00);
send_data_cmd(0x77);
send_data_cmd(0x00);
send_data_cmd(0x77);
send_data_cmd(0x00);
send_data_cmd(0x99);
send_data_cmd(0x00);
send_data_cmd(0xAA);
send_data_cmd(0x00);
send_data_cmd(0x88);
send_data_cmd(0x00);
send_data_cmd(0x44);
send_data_cmd(0x00);
send_data_cmd(0x44);
send_data_cmd(0x00);
send_data_cmd(0x44);
send_data_cmd(0x00);
send_data_cmd(0x33);
send_data_cmd(0x00);
send_data_cmd(0x33);
send_data_cmd(0x00);
send_data_cmd(0x22);
send_data_cmd(0x00);  
send_ctrl_cmd(0xE8);
send_data_cmd(0x44);
send_data_cmd(0x00);
send_data_cmd(0x55);
send_data_cmd(0x00);
send_data_cmd(0x66);
send_data_cmd(0x00);
send_data_cmd(0x99);
send_data_cmd(0x00);
send_data_cmd(0x88);
send_data_cmd(0x00);
send_data_cmd(0x77);
send_data_cmd(0x00);
send_data_cmd(0x77);
send_data_cmd(0x00);
send_data_cmd(0x99);
send_data_cmd(0x00);
send_data_cmd(0xAA);
send_data_cmd(0x00);
send_data_cmd(0x88);
send_data_cmd(0x00);
send_data_cmd(0x44);
send_data_cmd(0x00);
send_data_cmd(0x44);
send_data_cmd(0x00);
send_data_cmd(0x44);
send_data_cmd(0x00);
send_data_cmd(0x33);
send_data_cmd(0x00);
send_data_cmd(0x33);
send_data_cmd(0x00);
send_data_cmd(0x22);
send_data_cmd(0x00);  

send_ctrl_cmd(0xE9);
send_data_cmd(0xAA);
send_data_cmd(0x00);
send_data_cmd(0x00);
send_data_cmd(0x00);  

send_ctrl_cmd(0x00);
send_data_cmd(0xAA); 
send_ctrl_cmd(0xF3);
send_data_cmd(0x00);
send_ctrl_cmd(0xF9);
send_data_cmd(0x06);
send_data_cmd(0x10);
send_data_cmd(0x29);
send_data_cmd(0x00); 

send_ctrl_cmd(0x3A);
send_data_cmd(0x66);
send_ctrl_cmd(0xB3);
send_data_cmd(0x20);//mcu mipi
//send_ctrl_cmd(0xB3);
//send_data_cmd(0x21);//rgb interface

send_ctrl_cmd(0x55);
send_data_cmd(0x00);

send_ctrl_cmd(0x11);
MDELAY(120);
send_ctrl_cmd(0x29);
send_ctrl_cmd(0x35);
send_data_cmd(0x00); 

}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    LCM_PRINT_FUNC();    
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
    LCM_PRINT_FUNC();    

    memset(params, 0, sizeof(LCM_PARAMS));

    params->type   = LCM_TYPE_DBI;
    params->ctrl   = LCM_CTRL_PARALLEL_DBI;
    params->width  = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;

    params->io_select_mode = 3; 
    params->dbi.port                    = 0;
    params->dbi.clock_freq              = LCM_DBI_CLOCK_FREQ_104M;
    params->dbi.data_width              = LCM_DBI_DATA_WIDTH_18BITS;
    params->dbi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dbi.data_format.trans_seq   = LCM_DBI_TRANS_SEQ_MSB_FIRST;
    params->dbi.data_format.padding     = LCM_DBI_PADDING_ON_LSB;
    params->dbi.data_format.format      = LCM_DBI_FORMAT_RGB666;
    params->dbi.data_format.width       = LCM_DBI_DATA_WIDTH_18BITS;
    params->dbi.cpu_write_bits          = LCM_DBI_CPU_WRITE_32_BITS;	
    params->dbi.io_driving_current      = LCM_DRIVING_CURRENT_8MA;


    /* params->dbi.parallel.write_setup    = 2; */
    /* params->dbi.parallel.write_hold     = 2; */
    /* params->dbi.parallel.write_wait     = 10; */
    /* params->dbi.parallel.read_setup     = 2; */
    /* params->dbi.parallel.read_hold      = 2; */
    /* params->dbi.parallel.read_latency   = 10; */
    /* params->dbi.parallel.wait_period    = 1; */

    params->dbi.parallel.write_setup    = 1;
    params->dbi.parallel.write_hold     = 1;
    params->dbi.parallel.write_wait     = 3;
    params->dbi.parallel.read_setup     = 4;
    params->dbi.parallel.read_hold      = 0;
    params->dbi.parallel.read_latency   = 18;
    params->dbi.parallel.wait_period    = 1;

	// enable tearing-free
    params->dbi.te_mode                 = LCM_DBI_TE_MODE_VSYNC_ONLY;
    params->dbi.te_edge_polarity        = LCM_POLARITY_FALLING;
}

static unsigned int lcm_compare_id(void)
{
	LCM_PRINT_FUNC();
    MDELAY(50); 
    SET_RESET_PIN(1);
    MDELAY(1);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(50);
  
    unsigned int id = 0;
    unsigned int id1 = 0;
    unsigned int id2 = 0;
	unsigned int id3 = 0;
    unsigned int id4 = 0;
    send_ctrl_cmd(0xd4);
    id1 = read_data_cmd();
    id2 = read_data_cmd();
    id3 = read_data_cmd();
    id4 = read_data_cmd();
    id = ((id3 << 8) | id4);
    LCM_PRINT("%s, id=0x%x, id1=0x%x, id2=0x%x, id3=0x%x, id4=0x%x\n", __func__, id, id1, id2, id3, id4);

    return (LCM_ID == id)?1:0;
}

static void lcm_init(void)
{
    LCM_PRINT_FUNC();
    MDELAY(50); 
    SET_RESET_PIN(1);
    MDELAY(1);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(50);

    init_lcm_registers();
}

static void lcm_suspend(void) {
    LCM_PRINT_FUNC();	
    send_ctrl_cmd(0x10);
    MDELAY(120);
}

static void lcm_resume(void) {
    LCM_PRINT_FUNC();	
    send_ctrl_cmd(0x11);
    MDELAY(120);
}

static void lcm_update(unsigned int x, unsigned int y,
        unsigned int width, unsigned int height) {
    unsigned short x0, y0, x1, y1;
    unsigned short h_X_start, l_X_start, h_X_end, l_X_end, h_Y_start, l_Y_start, h_Y_end, l_Y_end;

    x0 = (unsigned short) x;
    y0 = (unsigned short) y;
    x1 = (unsigned short) x + width - 1;
    y1 = (unsigned short) y + height - 1;

    h_X_start = ((x0 & 0xFF00) >> 8);
    l_X_start = (x0 & 0x00FF);
    h_X_end = ((x1 & 0xFF00) >> 8);
    l_X_end = (x1 & 0x00FF);

    h_Y_start = ((y0 & 0xFF00) >> 8);
    l_Y_start = (y0 & 0x00FF);
    h_Y_end = ((y1 & 0xFF00) >> 8);
    l_Y_end = (y1 & 0x00FF);

    send_ctrl_cmd(0x2A);
    send_data_cmd(h_X_start);
    send_data_cmd(l_X_start);
    send_data_cmd(h_X_end);
    send_data_cmd(l_X_end);

    send_ctrl_cmd(0x2B);
    send_data_cmd(h_Y_start);
    send_data_cmd(l_Y_start);
    send_data_cmd(h_Y_end);
    send_data_cmd(l_Y_end);

    send_ctrl_cmd(0x29);

    send_ctrl_cmd(0x2C);
}


LCM_DRIVER nt35310_dbi_18bits_tinno_lcm_drv = 
{
    .name			= "nt35310_mcu_6572_tinno",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
    .update         = lcm_update,
    .compare_id     = lcm_compare_id
};

