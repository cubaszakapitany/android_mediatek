/*
 * drivers/leds/leds-mt65xx.c
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 * mt65xx leds driver
 *
 */
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/leds.h>
#include <linux/leds-mt65xx.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>
#include <linux/slab.h>

#include <cust_leds.h>

#if defined (CONFIG_ARCH_MT6577) || defined (CONFIG_ARCH_MT6575) || defined (CONFIG_ARCH_MT6575T)
#include <mach/mt_pwm.h>
#include <mach/mt_gpio.h>
#include <mach/pmic_mt6329_hw_bank1.h> 
#include <mach/pmic_mt6329_sw_bank1.h> 
#include <mach/pmic_mt6329_hw.h>
#include <mach/pmic_mt6329_sw.h>
#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>
#include <mach/mt_pmic_feature_api.h>
#include <mach/mt_boot.h>

#elif defined (CONFIG_ARCH_MT6589)
#include <mach/mt_pwm.h>
//#include <mach/mt_gpio.h>
#include <mach/pmic_mt6329_hw_bank1.h> 
#include <mach/pmic_mt6329_sw_bank1.h> 
#include <mach/pmic_mt6329_hw.h>
#include <mach/pmic_mt6329_sw.h>
#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>
//#include <mach/mt_pmic_feature_api.h>
//#include <mach/mt_boot.h>

#elif defined (CONFIG_ARCH_MT6582)
#include <mach/mt_pwm.h>
//#include <mach/mt_gpio.h>
#include <mach/pmic_mt6329_hw_bank1.h> 
#include <mach/pmic_mt6329_sw_bank1.h> 
#include <mach/pmic_mt6329_hw.h>
#include <mach/pmic_mt6329_sw.h>
#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>
//#include <mach/mt_pmic_feature_api.h>
//#include <mach/mt_boot.h>

#elif defined (CONFIG_ARCH_MT6572) || defined (CONFIG_ARCH_MT6571)
#include <mach/mt_pwm.h>
#include <mach/mt_leds.h>
#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>
//#include <mach/mt_pmic_feature_api.h>
//#include <mach/mt_boot.h>

#endif

#ifndef CONTROL_BL_TEMPERATURE
#define CONTROL_BL_TEMPERATURE
#endif
//add for sync led work schedule
static DEFINE_MUTEX(leds_mutex);

//#define ISINK_CHOP_CLK

/****************************************************************************
 * LED Variable Settings
 ***************************************************************************/
#define NLED_OFF 0
#define NLED_ON 1
#define NLED_BLINK 2
#define MIN_FRE_OLD_PWM 32 // the min frequence when use old mode pwm by kHz
#define PWM_DIV_NUM 8
#define ERROR_BL_LEVEL 0xFFFFFFFF
struct nled_setting
{
	u8 nled_mode; //0, off; 1, on; 2, blink;
	u32 blink_on_time ;
	u32 blink_off_time;
};
 
struct cust_mt65xx_led* bl_setting = NULL;
unsigned int bl_brightness = 102;
unsigned int bl_duty = 21;
unsigned int bl_div = CLK_DIV1;
unsigned int bl_frequency = 32000;
#if defined CONFIG_ARCH_MT6589 || defined (CONFIG_ARCH_MT6582) || defined (CONFIG_ARCH_MT6572) || defined (CONFIG_ARCH_MT6571)
typedef enum{  
    PMIC_PWM_0 = 0,  
    PMIC_PWM_1 = 1,  
    PMIC_PWM_2 = 2
} MT65XX_PMIC_PWM_NUMBER;
#endif

#if defined (CONFIG_ARCH_MT6572) || defined (CONFIG_ARCH_MT6571)
u32 isink_step[] =
{
		ISINK_STEP_04MA,	ISINK_STEP_08MA,        ISINK_STEP_12MA,   ISINK_STEP_16MA,   ISINK_STEP_20MA,   ISINK_STEP_24MA,
		ISINK_STEP_32MA,        ISINK_STEP_40MA,        ISINK_STEP_48MA
};
#define ISINK_STEP_SIZE         (sizeof(isink_step)/sizeof(isink_step[0]))
#endif

#ifdef LED_INCREASE_LED_LEVEL_MTKPATCH
#if defined (CONFIG_ARCH_MT6577) || defined (CONFIG_ARCH_MT6575) || defined (CONFIG_ARCH_MT6575T) || defined (CONFIG_ARCH_MT6589)
#define LED_INTERNAL_LEVEL_BIT_CNT 8
#endif

#if defined (CONFIG_ARCH_MT6582) || defined (CONFIG_ARCH_MT6572) || defined (CONFIG_ARCH_MT6571)
#define LED_INTERNAL_LEVEL_BIT_CNT 10
#endif
#endif

/*****************PWM *************************************************/
int time_array[PWM_DIV_NUM]={256,512,1024,2048,4096,8192,16384,32768};
u8 div_array[PWM_DIV_NUM] = {1,2,4,8,16,32,64,128};
unsigned int backlight_PWM_div = CLK_DIV1;// this para come from cust_leds.

/****************************************************************************
 * DEBUG MACROS
 ***************************************************************************/
static int debug_enable = 1;
#define LEDS_DEBUG(format, args...) do{ \
	if(debug_enable) \
	{\
		printk(KERN_EMERG format,##args);\
	}\
}while(0)

static int detail_debug_enable = 0;
#define LEDS_DETAIL_DEBUG(format, args...) do{ \
	if(detail_debug_enable) \
	{\
		printk(KERN_EMERG format,##args);\
	}\
}while(0)

/****************************************************************************
 * structures
 ***************************************************************************/
struct mt65xx_led_data {
	struct led_classdev cdev;
	struct cust_mt65xx_led cust;
	struct work_struct work;
	int level;
	int delay_on;
	int delay_off;
};


/****************************************************************************
 * function prototypes
 ***************************************************************************/

#if defined (CONFIG_ARCH_MT6575) || defined (CONFIG_ARCH_MT6575T)|| defined (CONFIG_ARCH_MT6577)
extern void mt_pwm_power_off (U32 pwm_no);
extern S32 mt_set_pwm_disable ( U32 pwm_no ) ;
#elif defined CONFIG_ARCH_MT6589 || defined (CONFIG_ARCH_MT6582) || defined (CONFIG_ARCH_MT6572) || defined (CONFIG_ARCH_MT6571) 
extern void mt_pwm_disable(U32 pwm_no, BOOL pmic_pad);
#endif
extern unsigned int brightness_mapping(unsigned int level);
extern int mtkfb_get_backlight_pwm(unsigned int divider, unsigned int *freq);

/* export functions */
int mt65xx_leds_brightness_set(enum mt65xx_led_type type, enum led_brightness level);

static int brightness_set_pmic(enum mt65xx_led_pmic pmic_type, u32 level, u32 div);
//static int brightness_set_gpio(int gpio_num, enum led_brightness level);
static int mt65xx_led_set_cust(struct cust_mt65xx_led *cust, int level);
static void mt65xx_led_work(struct work_struct *work);
static void mt65xx_led_set(struct led_classdev *led_cdev, enum led_brightness level);
static int  mt65xx_blink_set(struct led_classdev *led_cdev,
			     unsigned long *delay_on,
			     unsigned long *delay_off);

#ifdef CONTROL_BL_TEMPERATURE
int setMaxbrightness(int max_level, int enable);

#endif
#if defined(CONFIG_ARCH_MT6572) || defined (CONFIG_ARCH_MT6571)
extern S32 pwrap_read( U32  adr, U32 *rdata );
static void PMIC_Led_Control(enum mt65xx_led_pmic pmic_type, struct PMIC_ISINK_CONFIG *config_data);
static void PMIC_Backlight_Control(enum mt65xx_led_pmic pmic_type, PMIC_ISINK_BACKLIGHT_CONTROL control, struct PMIC_ISINK_CONFIG *config_data);
static void PMIC_ISINK_Enable(PMIC_ISINK_CHANNEL channel);
static void PMIC_ISINK_Disable(PMIC_ISINK_CHANNEL channel);
static void PMIC_ISINK_Easy_Config(PMIC_ISINK_CHANNEL channel, struct PMIC_ISINK_CONFIG *config_data);
static void PMIC_ISINK_Current_Config(PMIC_ISINK_CHANNEL channel);
static void PMIC_ISINK_Brightness_Config(PMIC_ISINK_CHANNEL channel, struct PMIC_ISINK_CONFIG *config_data);
static void PMIC_Led_Brightness_Control(enum mt65xx_led_pmic pmic_type, struct PMIC_ISINK_CONFIG *config_data, u32 level);
static unsigned int Value_to_RegisterIdx(u32 val, u32 *table);
static unsigned int PMIC_ISINK_CHANNEL_LOOKUP_BY_GROUP(enum mt65xx_led_pmic pmic_type);
static void PMIC_ISINK_GROUP_CONTROL(enum mt65xx_led_pmic pmic_type, PMIC_ISINK_CONTROL_API handler, struct PMIC_ISINK_CONFIG *config_data);
static unsigned int led_pmic_pwrap_read(U32 addr);
static void led_register_dump(void);
#endif


/****************************************************************************
 * global variables
 ***************************************************************************/
static struct mt65xx_led_data *g_leds_data[MT65XX_LED_TYPE_TOTAL];
struct wake_lock leds_suspend_lock;

#if defined(CONFIG_ARCH_MT6572) || defined (CONFIG_ARCH_MT6571)
static struct PMIC_CUST_ISINK *g_isink_data[ISINK_NUM];
static unsigned int isink_indicator_en = 0;
static unsigned int isink_backlight_en = 0;

#endif

/****************************************************************************
 * add API for temperature control
 ***************************************************************************/

#ifdef CONTROL_BL_TEMPERATURE

//define int limit for brightness limitation
static unsigned  int limit = 255;
static unsigned  int limit_flag = 0;  
static unsigned  int last_level = 0; 
static unsigned  int current_level = 0; 
static DEFINE_MUTEX(bl_level_limit_mutex);


//this API add for control the power and temperature
//if enabe=1, the value of brightness will smaller  than max_level, whatever lightservice transfers to driver
//max_level range from 0~255
int setMaxbrightness(int max_level, int enable)
{

	struct cust_mt65xx_led *cust_led_list = get_cust_led_list();
	mutex_lock(&bl_level_limit_mutex);

	if (1 == enable)
	{
		limit_flag = 1;
#ifdef LED_INCREASE_LED_LEVEL_MTKPATCH
		if(MT65XX_LED_MODE_CUST_BLS_PWM == cust_led_list[MT65XX_LED_TYPE_LCD].mode)
		{
			limit = (255 == max_level) ? ((1 << LED_INTERNAL_LEVEL_BIT_CNT) - 1) : (max_level << (LED_INTERNAL_LEVEL_BIT_CNT - 8));
		}
		else
		{
			limit = max_level;
		}
#else
		limit = max_level;
#endif
		mutex_unlock(&bl_level_limit_mutex);
		//LEDS_DEBUG("[LED] setMaxbrightness limit happen and release lock!!\n");
		printk("setMaxbrightness enable:last_level=%d, current_level=%d\n", last_level, current_level);
		//if (limit < last_level){
		if (0 != current_level){
			if(limit < last_level){
				//printk("mt65xx_leds_set_cust in setMaxbrightness:value control start! limit=%d\n", limit);
				mt65xx_led_set_cust(&cust_led_list[MT65XX_LED_TYPE_LCD], limit);
			}else{
				//printk("mt65xx_leds_set_cust in setMaxbrightness:value control start! last_level=%d\n", last_level);
				mt65xx_led_set_cust(&cust_led_list[MT65XX_LED_TYPE_LCD], last_level);
			}
		}
	}
	else
	{
		limit_flag = 0;
#ifdef LED_INCREASE_LED_LEVEL_MTKPATCH
		if(MT65XX_LED_MODE_CUST_BLS_PWM == cust_led_list[MT65XX_LED_TYPE_LCD].mode)
		{
			limit = ((1 << LED_INTERNAL_LEVEL_BIT_CNT) - 1);
		}
		else
		{
			limit = 255;
		}
#else
		limit = 255;
#endif
		mutex_unlock(&bl_level_limit_mutex);
		//LEDS_DEBUG("[LED] setMaxbrightness limit closed and and release lock!!\n");
		printk("setMaxbrightness disable:last_level=%d, current_level=%d\n", last_level, current_level);
		
		//if (last_level != 0){
		if (0 != current_level){
		        //printk("control temperature close:limit=%d\n", limit);
		        mt65xx_led_set_cust(&cust_led_list[MT65XX_LED_TYPE_LCD], last_level);
		
		        //printk("mt65xx_leds_set_cust in setMaxbrightness:value control close!\n");
		}
	}
 	
	LEDS_DEBUG("[LED] setMaxbrightness limit_flag = %d, limit=%d, current_level=%d\n",limit_flag, limit, current_level);
	
	return 0;
	
}
#endif
/****************************************************************************
 * internal functions
 ***************************************************************************/
static int brightness_mapto64(int level)
{
        if (level < 30)
                return (level >> 1) + 7;
        else if (level <= 120)
                return (level >> 2) + 14;
        else if (level <= 160)
                return level / 5 + 20;
        else
                return (level >> 3) + 33;
}

int find_time_index(int time)
{	
	int index = 0;	
	while(index < 8)	
	{		
		if(time<time_array[index])			
			return index;		
		else
			index++;
	}	
	return PWM_DIV_NUM-1;
}
#if defined CONFIG_ARCH_MT6582
static int led_set_pwm(int pwm_num, struct nled_setting* led)
{
	

	return 0;
	
}
#endif

#if defined CONFIG_ARCH_MT6572 // MT6571 is different with MT6572 here
static int led_set_pwm(int pwm_num, struct nled_setting* led)
{
	struct pwm_spec_config pwm_setting;
	int time_index = 0;
	pwm_setting.pwm_no = pwm_num;
        pwm_setting.mode = PWM_MODE_OLD;
    
        LEDS_DEBUG("[LED]led_set_pwm: mode = %d, pwm_no = %d\n", led->nled_mode, pwm_num);  
	// Only old PWM mode with 32kHz clock source can work in the system sleep mode
	pwm_setting.clk_src = PWM_CLK_OLD_MODE_32K;

	switch (led->nled_mode)
	{
		case NLED_OFF :
			pwm_setting.PWM_MODE_OLD_REGS.THRESH = 0;
			pwm_setting.clk_div = CLK_DIV1;
			pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH = 100;
			break;
            
		case NLED_ON :
			pwm_setting.PWM_MODE_OLD_REGS.THRESH = 30;
			pwm_setting.clk_div = CLK_DIV1;			
			pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH = 100;
			break;
            
		case NLED_BLINK :
			LEDS_DEBUG("[LED]LED blink on time = %d, offtime = %d\n", led->blink_on_time, led->blink_off_time);
			time_index = find_time_index(led->blink_on_time + led->blink_off_time);
			LEDS_DEBUG("[LED]LED div is %d\n", time_index);
			pwm_setting.clk_div = time_index;
			pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH = (led->blink_on_time + led->blink_off_time) * MIN_FRE_OLD_PWM / div_array[time_index];
			pwm_setting.PWM_MODE_OLD_REGS.THRESH = (led->blink_on_time * 100) / (led->blink_on_time + led->blink_off_time);
	}

	pwm_setting.PWM_MODE_FIFO_REGS.IDLE_VALUE = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.GDURATION = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.WAVE_NUM = 0;
	pwm_set_spec_config(&pwm_setting);

	return 0;
}
#endif // End of #if defined CONFIG_ARCH_MT6572 || defined CONFIG_ARCH_MT6571

#if defined CONFIG_ARCH_MT6571
static int led_set_pwm(int pwm_num, struct nled_setting* led)
{
	struct pwm_spec_config pwm_setting;
	int time_index = 0;
	pwm_setting.pwm_no = pwm_num;
    pwm_setting.mode = PWM_MODE_OLD;
    
    LEDS_DEBUG("[LED]led_set_pwm: mode = %d, pwm_no = %d\n", led->nled_mode, pwm_num);  
	// In 6571, we won't choose 32K to be the clock src of old mode because of system performance.
	// The setting here will be clock src = 26MHz, CLKSEL = 26M/1625 (i.e. 16K)
	pwm_setting.clk_src = PWM_CLK_OLD_MODE_32K;

	switch (led->nled_mode)
	{
		case NLED_OFF : // Actually, the setting still can not to turn off NLED. We should disable PWM to turn off NLED.
			pwm_setting.PWM_MODE_OLD_REGS.THRESH = 0;
			pwm_setting.clk_div = CLK_DIV1;
			pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH = 100;
			break;
            
		case NLED_ON :
			pwm_setting.PWM_MODE_OLD_REGS.THRESH = 30/2;
			pwm_setting.clk_div = CLK_DIV1;			
			pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH = 100/2;
			break;
            
		case NLED_BLINK :
			LEDS_DEBUG("[LED]LED blink on time = %d, offtime = %d\n", led->blink_on_time, led->blink_off_time);
			time_index = find_time_index(led->blink_on_time + led->blink_off_time);
			LEDS_DEBUG("[LED]LED div is %d\n", time_index);
			pwm_setting.clk_div = time_index;
			pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH = (led->blink_on_time + led->blink_off_time) * MIN_FRE_OLD_PWM / div_array[time_index];
			pwm_setting.PWM_MODE_OLD_REGS.THRESH = (led->blink_on_time * 100) / (led->blink_on_time + led->blink_off_time);
	}

	pwm_setting.PWM_MODE_FIFO_REGS.IDLE_VALUE = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.GDURATION = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.WAVE_NUM = 0;
	pwm_set_spec_config(&pwm_setting);

	return 0;
}
#endif // End of #if defined CONFIG_ARCH_MT6572 || defined CONFIG_ARCH_MT6571

#if defined CONFIG_ARCH_MT6589
static int led_set_pwm(int pwm_num, struct nled_setting* led)
{
	//struct pwm_easy_config pwm_setting;
	struct pwm_spec_config pwm_setting;
	int time_index = 0;
	pwm_setting.pwm_no = pwm_num;
        pwm_setting.mode = PWM_MODE_OLD;
    
        LEDS_DEBUG("[LED]led_set_pwm: mode=%d,pwm_no=%d\n", led->nled_mode, pwm_num);  
	//if((pwm_num != PWM3 && pwm_num != PWM4 && pwm_num != PWM5))//AP PWM all support OLD mode in MT6589
		pwm_setting.clk_src = PWM_CLK_OLD_MODE_32K;
	//else
		//pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625;
    
	switch (led->nled_mode)
	{
		case NLED_OFF :
			pwm_setting.PWM_MODE_OLD_REGS.THRESH = 0;
			pwm_setting.clk_div = CLK_DIV1;
			pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH = 100;
			break;
            
		case NLED_ON :
			pwm_setting.PWM_MODE_OLD_REGS.THRESH = 30;
			pwm_setting.clk_div = CLK_DIV1;			
			pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH = 100;
			break;
            
		case NLED_BLINK :
			LEDS_DEBUG("[LED]LED blink on time = %d offtime = %d\n",led->blink_on_time,led->blink_off_time);
			time_index = find_time_index(led->blink_on_time + led->blink_off_time);
			LEDS_DEBUG("[LED]LED div is %d\n",time_index);
			pwm_setting.clk_div = time_index;
			pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH = (led->blink_on_time + led->blink_off_time) * MIN_FRE_OLD_PWM / div_array[time_index];
			pwm_setting.PWM_MODE_OLD_REGS.THRESH = (led->blink_on_time*100) / (led->blink_on_time + led->blink_off_time);
	}
	
	pwm_setting.PWM_MODE_FIFO_REGS.IDLE_VALUE = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.GDURATION = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.WAVE_NUM = 0;
	pwm_set_spec_config(&pwm_setting);

	return 0;
	
}
#endif
#if defined (CONFIG_ARCH_MT6577) || defined (CONFIG_ARCH_MT6575) || defined (CONFIG_ARCH_MT6575T)
static int led_set_pwm(int pwm_num, struct nled_setting* led)
{
	struct pwm_easy_config pwm_setting;
	int time_index = 0;
	pwm_setting.pwm_no = pwm_num;
    
        LEDS_DEBUG("[LED]led_set_pwm: mode=%d,pwm_no=%d\n", led->nled_mode, pwm_num);  
	if((pwm_num != PWM3 && pwm_num != PWM4 && pwm_num != PWM5))
		pwm_setting.clk_src = PWM_CLK_OLD_MODE_32K;
	else
		pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625;
    
	switch (led->nled_mode)
	{
		case NLED_OFF :
			pwm_setting.duty = 0;
			pwm_setting.clk_div = CLK_DIV1;
			pwm_setting.duration = 100;
			break;
            
		case NLED_ON :
			pwm_setting.duty = 30;
			pwm_setting.clk_div = CLK_DIV1;			
			pwm_setting.duration = 100;
			break;
            
		case NLED_BLINK :
			LEDS_DEBUG("[LED]LED blink on time = %d offtime = %d\n",led->blink_on_time,led->blink_off_time);
			time_index = find_time_index(led->blink_on_time + led->blink_off_time);
			LEDS_DEBUG("[LED]LED div is %d\n",time_index);
			pwm_setting.clk_div = time_index;
			pwm_setting.duration = (led->blink_on_time + led->blink_off_time) * MIN_FRE_OLD_PWM / div_array[time_index];
			pwm_setting.duty = (led->blink_on_time*100) / (led->blink_on_time + led->blink_off_time);
	}
	pwm_set_easy_config(&pwm_setting);

	return 0;
	
}

#endif

//***********************MT6589 led breath funcion*******************************//
#if defined CONFIG_ARCH_MT6589
#if 0
static int led_breath_pmic(enum mt65xx_led_pmic pmic_type, struct nled_setting* led)
{
	//int time_index = 0;
	//int duty = 0;
	LEDS_DEBUG("[LED]led_blink_pmic: pmic_type=%d\n", pmic_type);  
	
	if((pmic_type != MT65XX_LED_PMIC_NLED_ISINK0 && pmic_type!= MT65XX_LED_PMIC_NLED_ISINK1) || led->nled_mode != NLED_BLINK) {
		return -1;
	}
				
	switch(pmic_type){
		case MT65XX_LED_PMIC_NLED_ISINK0://keypad center 4mA
			upmu_set_isinks_ch0_mode(PMIC_PWM_0);// 6320 spec 
			upmu_set_isinks_ch0_step(0x0);//4mA
			upmu_set_isinks_breath0_trf_sel(0x04);
			upmu_set_isinks_breath0_ton_sel(0x02);
			upmu_set_isinks_breath0_toff_sel(0x03);
			upmu_set_isink_dim1_duty(15);
			upmu_set_isink_dim1_fsel(11);//6320 0.25KHz
			upmu_set_rg_spk_div_pdn(0x01);
			upmu_set_rg_spk_pwm_div_pdn(0x01);
			upmu_set_isinks_ch0_en(0x01);
			break;
		case MT65XX_LED_PMIC_NLED_ISINK1://keypad LR  16mA
			upmu_set_isinks_ch1_mode(PMIC_PWM_1);// 6320 spec 
			upmu_set_isinks_ch1_step(0x3);//16mA
			upmu_set_isinks_breath0_trf_sel(0x04);
			upmu_set_isinks_breath0_ton_sel(0x02);
			upmu_set_isinks_breath0_toff_sel(0x03);
			upmu_set_isink_dim1_duty(15);
			upmu_set_isink_dim1_fsel(11);//6320 0.25KHz
			upmu_set_rg_spk_div_pdn(0x01);
			upmu_set_rg_spk_pwm_div_pdn(0x01);
			upmu_set_isinks_ch1_en(0x01);
			break;	
		default:
		break;
	}
	return 0;

}

#endif
#endif

#if defined CONFIG_ARCH_MT6572 || defined CONFIG_ARCH_MT6571
#if 0 // For Build Warning
static int led_breath_pmic(enum mt65xx_led_pmic pmic_type, struct nled_setting* led)
{
        struct PMIC_ISINK_CONFIG isink_config;
	LEDS_DEBUG("[LED]led_blink_pmic: pmic_type = %d\n", pmic_type);  
	
	if((pmic_type != MT65XX_LED_PMIC_NLED_ISINK0 && pmic_type!= MT65XX_LED_PMIC_NLED_ISINK1 \
        && pmic_type != MT65XX_LED_PMIC_NLED_ISINK2 && pmic_type != MT65XX_LED_PMIC_NLED_ISINK3 \
        && pmic_type != MT65XX_LED_PMIC_NLED_ISINK_GROUP0 && pmic_type != MT65XX_LED_PMIC_NLED_ISINK_GROUP1 \
        && pmic_type != MT65XX_LED_PMIC_NLED_ISINK_GROUP2 && pmic_type != MT65XX_LED_PMIC_NLED_ISINK_GROUP3) \
        || led->nled_mode != NLED_BLINK) {
		return -1;
	}

        upmu_set_rg_drv_2m_ck_pdn(0x0); // Disable power down (Indicator no need?)
        upmu_set_rg_drv_32k_ck_pdn(0x0); // Disable power down
        upmu_set_isink_phase_dly_tc(0x0); // TC = 0.5us
    
        isink_config.mode = ISINK_BREATH_MODE;
        isink_config.duty = 31; // useless for breath mode
        isink_config.dim_fsel = 0; // 1K = 32000 / (0 + 1) / 32 (useless for breath mode)
        isink_config.sfstr_tc = 0; // 0.5us
        isink_config.sfstr_en = 0; // Disable soft start
        isink_config.breath_trf_sel = 0x04; // 0.926s
        isink_config.breath_ton_sel = 0x02; // 0.523s
        isink_config.breath_toff_sel = 0x03; // 1.417s
        PMIC_Led_Control(pmic_type, &isink_config);
	return 0;

}
#endif
#endif // End of #if defined CONFIG_ARCH_MT6572 || defined CONFIG_ARCH_MT6571

#if defined CONFIG_ARCH_MT6589

#define PMIC_PERIOD_NUM 13
//int pmic_period_array[] = {250,500,1000,1250,1666,2000,2500,3333,4000,5000,6666,8000,10000,13333,16000};
int pmic_period_array[] = {250,500,1000,1250,1666,2000,2500,3333,4000,5000,6666,8000,10000};
u8 pmic_clksel_array[] = {0,0,0,0,0,0,1,1,1,2,2,2,3,3,3};
//u8 pmic_freqsel_array[] = {24,26,28,29,30,31,29,30,31,29,30,31,29,30,31};
//u8 pmic_freqsel_array[] = {21,22,23,24,25,26,24,25,26,24,25,26,24,25,26};
u8 pmic_freqsel_array[] = {21,22,23,24,24,24,25,26,26,26,28,28,28};


int find_time_index_pmic(int time_ms) {
	int i;
	for(i=0;i<PMIC_PERIOD_NUM;i++) {
		if(time_ms<=pmic_period_array[i]) {
			return i;
		} else {
			continue;
		}
	}
	return PMIC_PERIOD_NUM-1;
}

static int led_blink_pmic(enum mt65xx_led_pmic pmic_type, struct nled_setting* led) {
	int time_index = 0;
	int duty = 0;
	LEDS_DEBUG("[LED]led_blink_pmic: pmic_type=%d\n", pmic_type);  
	
	if((pmic_type != MT65XX_LED_PMIC_NLED_ISINK0 && pmic_type!= MT65XX_LED_PMIC_NLED_ISINK1 && pmic_type!= MT65XX_LED_PMIC_NLED_ISINK2) || led->nled_mode != NLED_BLINK) {
		return -1;
	}
				
	LEDS_DEBUG("[LED]LED blink on time = %d offtime = %d\n",led->blink_on_time,led->blink_off_time);
	time_index = find_time_index_pmic(led->blink_on_time + led->blink_off_time);
	LEDS_DEBUG("[LED]LED index is %d clksel=%d freqsel=%d\n", time_index, pmic_clksel_array[time_index], pmic_freqsel_array[time_index]);
	duty=32*led->blink_on_time/(led->blink_on_time + led->blink_off_time);
	switch(pmic_type){
		case MT65XX_LED_PMIC_NLED_ISINK0://keypad center 4mA
			upmu_set_rg_bst_drv_1m_ck_pdn(0x0);
			upmu_set_isinks_ch0_mode(PMIC_PWM_0);// 6320 spec 
			upmu_set_isinks_ch0_step(0x0);//4mA
			upmu_set_isink_dim0_duty(duty);
			upmu_set_isink_dim0_fsel(pmic_freqsel_array[time_index]);
			//upmu_set_rg_spk_pwm_div_pdn(0x01);
			//upmu_set_rg_bst_drv_1m_ck_pdn(0x0);
			#ifdef ISINK_CHOP_CLK
				//upmu_set_isinks0_chop_en(0x1);
			#endif
			upmu_set_isinks_breath0_trf_sel(0x0);
			upmu_set_isinks_breath0_ton_sel(0x02);
			upmu_set_isinks_breath0_toff_sel(0x05);
			
			upmu_set_isinks_ch0_en(0x01);
			break;
		case MT65XX_LED_PMIC_NLED_ISINK1://keypad LR  16mA
			upmu_set_rg_bst_drv_1m_ck_pdn(0x0);
			upmu_set_isinks_ch1_mode(PMIC_PWM_0);// 6320 spec 
			upmu_set_isinks_ch1_step(0x3);//16mA
			upmu_set_isink_dim1_duty(duty);
			upmu_set_isink_dim1_fsel(pmic_freqsel_array[time_index]);
			upmu_set_isinks_breath1_trf_sel(0x0);
			upmu_set_isinks_breath1_ton_sel(0x02);
			upmu_set_isinks_breath1_toff_sel(0x05);
			//upmu_set_rg_spk_pwm_div_pdn(0x01);
			
			#ifdef ISINK_CHOP_CLK
				//upmu_set_isinks1_chop_en(0x1);
			#endif
			upmu_set_isinks_ch1_en(0x01);
			break;	
		case MT65XX_LED_PMIC_NLED_ISINK2://keypad LR  16mA
			upmu_set_rg_bst_drv_1m_ck_pdn(0x0);
			upmu_set_isinks_ch2_mode(PMIC_PWM_0);// 6320 spec 
			upmu_set_isinks_ch2_step(0x3);//16mA
			upmu_set_isink_dim2_duty(duty);
			upmu_set_isink_dim2_fsel(pmic_freqsel_array[time_index]);
			//upmu_set_rg_spk_pwm_div_pdn(0x01);
			upmu_set_isinks_breath2_trf_sel(0x0);
			upmu_set_isinks_breath2_ton_sel(0x02);
			upmu_set_isinks_breath2_toff_sel(0x05);
			#ifdef ISINK_CHOP_CLK
				//upmu_set_isinks2_chop_en(0x1);
			#endif
			upmu_set_isinks_ch2_en(0x01);
			break;	
		default:
		break;
	}
	return 0;
}

#elif defined CONFIG_ARCH_MT6582

#define PMIC_PERIOD_NUM 13
//int pmic_period_array[] = {250,500,1000,1250,1666,2000,2500,3333,4000,5000,6666,8000,10000,13333,16000};
int pmic_period_array[] = {250,500,1000,1250,1666,2000,2500,3333,4000,5000,6666,8000,10000};
u8 pmic_clksel_array[] = {0,0,0,0,0,0,1,1,1,2,2,2,3,3,3};
//u8 pmic_freqsel_array[] = {24,26,28,29,30,31,29,30,31,29,30,31,29,30,31};
//u8 pmic_freqsel_array[] = {21,22,23,24,25,26,24,25,26,24,25,26,24,25,26};
u8 pmic_freqsel_array[] = {21,22,23,24,24,24,25,26,26,26,28,28,28};


int find_time_index_pmic(int time_ms) {
	int i;
	for(i=0;i<PMIC_PERIOD_NUM;i++) {
		if(time_ms<=pmic_period_array[i]) {
			return i;
		} else {
			continue;
		}
	}
	return PMIC_PERIOD_NUM-1;
}

static int led_blink_pmic(enum mt65xx_led_pmic pmic_type, struct nled_setting* led) {
	
	return 0;
}

#elif defined CONFIG_ARCH_MT6572 || defined CONFIG_ARCH_MT6571

#define PMIC_PERIOD_NUM 9
// 0.01 Hz -> 1 / 0.01 * 1000 = 100000 ms
int pmic_period_array[PMIC_PERIOD_NUM] = {1, 5, 200, 500, 1000, 2000, 5000, 10000, 100000};
int pmic_freqsel_array[PMIC_PERIOD_NUM] = {0, 4, 199, 499, 999, 1999, 4999, 9999, 99999};

int find_time_index_pmic(int time_ms) {
	int i;
    time_ms = time_ms / 100;
	for(i = 0; i < PMIC_PERIOD_NUM; i++) {
		if(time_ms <= pmic_period_array[i]) {
			return i;
		} else {
			continue;
		}
	}
	return PMIC_PERIOD_NUM - 1;
}

static int led_blink_pmic(enum mt65xx_led_pmic pmic_type, struct nled_setting* led) {
	int time_index = 0;
	int duty = 0;
        struct PMIC_ISINK_CONFIG isink_config;
	LEDS_DEBUG("[LED]led_blink_pmic: pmic_type = %d\n", pmic_type);  
	
	if((pmic_type != MT65XX_LED_PMIC_NLED_ISINK0 && pmic_type != MT65XX_LED_PMIC_NLED_ISINK1 && \
            pmic_type != MT65XX_LED_PMIC_NLED_ISINK2 && pmic_type != MT65XX_LED_PMIC_NLED_ISINK3) || \
            led->nled_mode != NLED_BLINK) {
		return -1;
	}
				
	LEDS_DEBUG("[LED]LED blink on time = %d, off time = %d\n", led->blink_on_time, led->blink_off_time);
	time_index = (led->blink_on_time + led->blink_off_time) - 1;
	// LEDS_DEBUG("[LED]LED index is %d, freqsel = %d\n", time_index, pmic_freqsel_array[time_index]);
	duty = 32 * led->blink_on_time / (led->blink_on_time + led->blink_off_time);
        isink_config.mode = ISINK_PWM_MODE;
        isink_config.duty = duty;
        isink_config.dim_fsel = pmic_freqsel_array[time_index];
        isink_config.sfstr_tc = 0; // 0.5us
        isink_config.sfstr_en = 0; // Disable soft start
        isink_config.breath_trf_sel = 0x00; // 0.123s
        isink_config.breath_ton_sel = 0x00; // 0.123s
        isink_config.breath_toff_sel = 0x00; // 0.246s
        PMIC_Led_Control(pmic_type, &isink_config);
	return 0;
}

#elif defined (CONFIG_ARCH_MT6575) || defined (CONFIG_ARCH_MT6575T) || defined (CONFIG_ARCH_MT6577)
#define PMIC_PERIOD_NUM 15
int pmic_period_array[] = {250,500,1000,1250,1666,2000,2500,3333,4000,5000,6666,8000,10000,13333,16000};
u8 pmic_clksel_array[] = {0,0,0,0,0,0,1,1,1,2,2,2,3,3,3};
u8 pmic_freqsel_array[] = {24,26,28,29,30,31,29,30,31,29,30,31,29,30,31};

int find_time_index_pmic(int time_ms) {
	int i;
	for(i=0;i<PMIC_PERIOD_NUM;i++) {
		if(time_ms<=pmic_period_array[i]) {
			return i;
		} else {
			continue;
		}
	}
	return PMIC_PERIOD_NUM-1;
}

static int led_blink_pmic(enum mt65xx_led_pmic pmic_type, struct nled_setting* led) {
	int time_index = 0;
	int duty = 0;
	LEDS_DEBUG("[LED]led_blink_pmic: pmic_type=%d\n", pmic_type);  
	
	if((pmic_type != MT65XX_LED_PMIC_NLED_ISINK4 && pmic_type!= MT65XX_LED_PMIC_NLED_ISINK5) || led->nled_mode != NLED_BLINK) {
		return -1;
	}
				
	LEDS_DEBUG("[LED]LED blink on time = %d offtime = %d\n",led->blink_on_time,led->blink_off_time);
	time_index = find_time_index_pmic(led->blink_on_time + led->blink_off_time);
	LEDS_DEBUG("[LED]LED index is %d clksel=%d freqsel=%d\n", time_index, pmic_clksel_array[time_index], pmic_freqsel_array[time_index]);
	duty=32*led->blink_on_time/(led->blink_on_time + led->blink_off_time);
	switch(pmic_type){
		case MT65XX_LED_PMIC_NLED_ISINK4:
			upmu_isinks_ch4_mode(PMIC_PWM_1);
			upmu_isinks_ch4_step(0x3);
			upmu_isinks_ch4_cabc_en(0);
			upmu_isinks_dim1_duty(duty);
			upmu_isinks_dim1_fsel(pmic_freqsel_array[time_index]);
			pmic_bank1_config_interface(0x2e, pmic_clksel_array[time_index], 0x3, 0x6);
			upmu_top2_bst_drv_ck_pdn(0x0);
			hwBacklightISINKTurnOn(MT65XX_LED_PMIC_NLED_ISINK4);
		break;
		case MT65XX_LED_PMIC_NLED_ISINK5:
			upmu_isinks_ch5_mode(PMIC_PWM_2);
			upmu_isinks_ch5_step(0x3);
			upmu_isinks_ch5_cabc_en(0);
			upmu_isinks_dim2_duty(duty);
			upmu_isinks_dim2_fsel(pmic_freqsel_array[time_index]);
			pmic_bank1_config_interface(0x30, pmic_clksel_array[time_index], 0x3, 0x6);
			upmu_top2_bst_drv_ck_pdn(0x0);
			hwBacklightISINKTurnOn(MT65XX_LED_PMIC_NLED_ISINK5);
		break;
		default:
		break;
	}
			
	return 0;
}
#endif

//#if 0
#if defined CONFIG_ARCH_MT6589
static int backlight_set_pwm(int pwm_num, u32 level, u32 div, struct PWM_config *config_data)
{
	struct pwm_spec_config pwm_setting;
	pwm_setting.pwm_no = pwm_num;
	pwm_setting.mode = PWM_MODE_FIFO; //new mode fifo and periodical mode

	pwm_setting.pmic_pad = config_data->pmic_pad;
		
	if(config_data->div)
	{
		pwm_setting.clk_div = config_data->div;
		backlight_PWM_div = config_data->div;
	}
	else
		pwm_setting.clk_div = div;
	if(config_data->clock_source)
		pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK;
	else
		pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625;
	
	if(config_data->High_duration && config_data->low_duration)
		{
			pwm_setting.PWM_MODE_FIFO_REGS.HDURATION = config_data->High_duration;
			pwm_setting.PWM_MODE_FIFO_REGS.LDURATION = pwm_setting.PWM_MODE_FIFO_REGS.HDURATION;
		}
	else
		{
			pwm_setting.PWM_MODE_FIFO_REGS.HDURATION = 4;
			pwm_setting.PWM_MODE_FIFO_REGS.LDURATION = 4;
		}
	pwm_setting.PWM_MODE_FIFO_REGS.IDLE_VALUE = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.STOP_BITPOS_VALUE = 31;
	pwm_setting.PWM_MODE_FIFO_REGS.GDURATION = (pwm_setting.PWM_MODE_FIFO_REGS.HDURATION+1)*32 - 1;
	pwm_setting.PWM_MODE_FIFO_REGS.WAVE_NUM = 0;
		
	LEDS_DEBUG("[LED]backlight_set_pwm:duty is %d\n", level);
	LEDS_DEBUG("[LED]backlight_set_pwm:clk_src/div/high/low is %d%d%d%d\n", pwm_setting.clk_src,pwm_setting.clk_div,pwm_setting.PWM_MODE_FIFO_REGS.HDURATION,pwm_setting.PWM_MODE_FIFO_REGS.LDURATION);
	if(level>0 && level <= 32)
	{
		pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = 0;
		pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA0 =  (1 << level) - 1 ;
		//pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA1 = 0 ;
		pwm_set_spec_config(&pwm_setting);
	}else if(level>32 && level <=64)
	{
		pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = 1;
		level -= 32;
		pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA0 = (1 << level) - 1 ;
		//pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA0 =  0xFFFFFFFF ;
		//pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA1 = (1 << level) - 1;
		pwm_set_spec_config(&pwm_setting);
	}else
	{
		LEDS_DEBUG("[LED]Error level in backlight\n");
		//mt_set_pwm_disable(pwm_setting.pwm_no);
		//mt_pwm_power_off(pwm_setting.pwm_no);
		mt_pwm_disable(pwm_setting.pwm_no, config_data->pmic_pad);
	}
		//printk("[LED]PWM con register is %x \n", INREG32(PWM_BASE + 0x0150));
	return 0;
}
#endif

#if defined CONFIG_ARCH_MT6582
static int backlight_set_pwm(int pwm_num, u32 level, u32 div, struct PWM_config *config_data)
{
	
	return 0;
}
#endif

#if defined CONFIG_ARCH_MT6572 || defined CONFIG_ARCH_MT6571
static int backlight_set_pwm(int pwm_num, u32 level, u32 div, struct PWM_config *config_data)
{
	struct pwm_spec_config pwm_setting;

	pwm_setting.pwm_no = pwm_num;
	pwm_setting.mode = PWM_MODE_FIFO; // New mode fifo and periodical mode

	pwm_setting.pmic_pad = config_data->pmic_pad;

	if(config_data->div)
	{
		pwm_setting.clk_div = config_data->div;
		backlight_PWM_div = config_data->div;
	}
	else
	{
		pwm_setting.clk_div = div;
	}

	if(config_data->clock_source)
	{
		pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK;
	}
	else
	{
		pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625;
	}

	if(config_data->High_duration && config_data->low_duration)
	{
		pwm_setting.PWM_MODE_FIFO_REGS.HDURATION = config_data->High_duration;
		pwm_setting.PWM_MODE_FIFO_REGS.LDURATION = pwm_setting.PWM_MODE_FIFO_REGS.HDURATION;
	}
	else
	{
		pwm_setting.PWM_MODE_FIFO_REGS.HDURATION = 4;
		pwm_setting.PWM_MODE_FIFO_REGS.LDURATION = 4;
	}

	pwm_setting.PWM_MODE_FIFO_REGS.IDLE_VALUE = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.STOP_BITPOS_VALUE = 31;
	pwm_setting.PWM_MODE_FIFO_REGS.GDURATION = (pwm_setting.PWM_MODE_FIFO_REGS.HDURATION + 1) * 32 - 1;
	pwm_setting.PWM_MODE_FIFO_REGS.WAVE_NUM = 0;

	LEDS_DEBUG("[LEDS]backlight_set_pwm:duty is %d\n", level);
	LEDS_DEBUG("[LEDS]backlight_set_pwm:clk_src/div/high/low is %d%d%d%d\n", pwm_setting.clk_src, pwm_setting.clk_div, pwm_setting.PWM_MODE_FIFO_REGS.HDURATION, pwm_setting.PWM_MODE_FIFO_REGS.LDURATION);
	if(level > 0 && level <= 32)
	{
		pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = 0;
		pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA0 = (1 << level) - 1;
		pwm_set_spec_config(&pwm_setting);
	}else if(level > 32 && level <= 64)
	{
		pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = 1;
		level -= 32;
		pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA0 = (1 << level) - 1 ;
		pwm_set_spec_config(&pwm_setting);
	}else
	{
		LEDS_DEBUG("[LEDS]Error level in backlight\n");
		mt_pwm_disable(pwm_setting.pwm_no, config_data->pmic_pad);
	}

	return 0;
}
#endif


#if defined (CONFIG_ARCH_MT6577) || defined (CONFIG_ARCH_MT6575) || defined (CONFIG_ARCH_MT6575T)
static int backlight_set_pwm(int pwm_num, u32 level, u32 div, struct PWM_config *config_data)
{
	struct pwm_spec_config pwm_setting;
	pwm_setting.pwm_no = pwm_num;
	pwm_setting.mode = PWM_MODE_FIFO; //new mode fifo and periodical mode
		
	#ifdef CONTROL_BL_TEMPERATURE
	mutex_lock(&bl_level_limit_mutex);
			current_level = level;
			//printk("brightness_set_pwm:current_level=%d\n", current_level);
			if(0 == limit_flag){
				last_level = level;
				//printk("brightness_set_pwm:last_level=%d\n", last_level);
			}else {
					if(limit < current_level){
						level = limit;
					//	printk("backlight_set_pwm: control level=%d\n", level);
					}
			}	
	mutex_unlock(&bl_level_limit_mutex);
	#endif
	if(config_data->div)
	{
		pwm_setting.clk_div = config_data->div;
		backlight_PWM_div = config_data->div;
	}
	else
		pwm_setting.clk_div = div;
	if(config_data->clock_source)
		pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK;
	else
		pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625;
	
	if(config_data->High_duration && config_data->low_duration)
		{
			pwm_setting.PWM_MODE_FIFO_REGS.HDURATION = config_data->High_duration;
			pwm_setting.PWM_MODE_FIFO_REGS.LDURATION = config_data->low_duration;
		}
	else
		{
			pwm_setting.PWM_MODE_FIFO_REGS.HDURATION = 4;
			pwm_setting.PWM_MODE_FIFO_REGS.LDURATION = 4;
		}
	pwm_setting.PWM_MODE_FIFO_REGS.IDLE_VALUE = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.STOP_BITPOS_VALUE = 63;
	pwm_setting.PWM_MODE_FIFO_REGS.GDURATION = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.WAVE_NUM = 0;
		
	LEDS_DEBUG("[LED]backlight_set_pwm:duty is %d\n", level);
	LEDS_DEBUG("[LED]backlight_set_pwm:clk_src/div/high/low is %d%d%d%d\n", pwm_setting.clk_src,pwm_setting.clk_div,pwm_setting.PWM_MODE_FIFO_REGS.HDURATION,pwm_setting.PWM_MODE_FIFO_REGS.LDURATION);
	if(level>0 && level <= 32)
	{
		pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA0 =  (1 << level) - 1 ;
		pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA1 = 0 ;
		pwm_set_spec_config(&pwm_setting);
	}else if(level>32 && level <=64)
	{
		level -= 32;
		pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA0 =  0xFFFFFFFF ;
		pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA1 = (1 << level) - 1;
		pwm_set_spec_config(&pwm_setting);
	}else
	{
		LEDS_DEBUG("[LED]Error level in backlight\n");
		mt_set_pwm_disable(pwm_setting.pwm_no);
		mt_pwm_power_off(pwm_setting.pwm_no);
	}
		//printk("[LED]PWM con register is %x \n", INREG32(PWM_BASE + 0x0150));
	return 0;
}

#endif
#if defined CONFIG_ARCH_MT6589
static int brightness_set_pmic(enum mt65xx_led_pmic pmic_type, u32 level, u32 div)
{
		//int tmp_level = level;
		//static bool backlight_init_flag[2] = {false, false};
		static bool led_init_flag[3] = {false, false, false};
		static bool first_time = true;
		
		LEDS_DEBUG("[LED]PMIC#%d:%d\n", pmic_type, level);
	
		if (pmic_type == MT65XX_LED_PMIC_LCD_ISINK)
		{
		#if 0
			if(backlight_init_flag[0] == false)
			{
				hwBacklightISINKTuning(1, PMIC_PWM_0, 0x3, 0);
				hwBacklightISINKTuning(2, PMIC_PWM_0, 0x3, 0);
				hwBacklightISINKTuning(3, PMIC_PWM_0, 0x3, 0);
				backlight_init_flag[0] = true;
			}
			
			if (level) 
			{
				level = brightness_mapping(tmp_level);
				if(level == ERROR_BL_LEVEL)
					level = tmp_level/17;
				hwPWMsetting(PMIC_PWM_0, level, div);
				upmu_top2_bst_drv_ck_pdn(0x0);
				hwBacklightISINKTurnOn(1);
				hwBacklightISINKTurnOn(2);
				hwBacklightISINKTurnOn(3);
				bl_duty = level;	
			}
			else 
			{
				hwBacklightISINKTurnOff(1);
				hwBacklightISINKTurnOff(2);
				hwBacklightISINKTurnOff(3);
				bl_duty = level;	
			}
			return 0;
		}
		else if(pmic_type == MT65XX_LED_PMIC_LCD_BOOST)
		{
			/*
			if(backlight_init_flag[1] == false)
			{
				hwBacklightBoostTuning(PMIC_PWM_0, 0xC, 0);
				backlight_init_flag[1] = true;
			}
			*/		
			if (level) 
			{
				level = brightness_mapping(tmp_level);
				if(level == ERROR_BL_LEVEL)
					level = tmp_level/42;
		
				upmu_boost_isink_hw_sel(0x1);
				upmu_boost_mode(3);
				upmu_boost_cabc_en(0);
	
				switch(level)
				{
					case 0: 			
						upmu_boost_vrsel(0x0);
						//hwPWMsetting(PMIC_PWM_0, 0, 0x15);
						break;
					case 1:
						upmu_boost_vrsel(0x1);
						//hwPWMsetting(PMIC_PWM_0, 4, 0x15);
						break;
					case 2:
						upmu_boost_vrsel(0x2);
						//hwPWMsetting(PMIC_PWM_0, 5, 0x15);
						break;
					case 3:
						upmu_boost_vrsel(0x3);
						//hwPWMsetting(PMIC_PWM_0, 6, 0x15);
						break;
					case 4:
						upmu_boost_vrsel(0x5);
						//hwPWMsetting(PMIC_PWM_0, 7, 0x15);
						break;
					case 5:
						upmu_boost_vrsel(0x8);
						//hwPWMsetting(PMIC_PWM_0, 8, 0x15);
						break;
					case 6:
						upmu_boost_vrsel(0xB);
						//hwPWMsetting(PMIC_PWM_0, 9, 0x15);
						break;
					default:
						printk("[LED] invalid brightness level %d->%d\n", tmp_level, level);
						break;
				}
				
				upmu_top2_bst_drv_ck_pdn(0x0);
				upmu_boost_en(0x1);
				bl_duty = level;	
			}
			else 
			{
				upmu_boost_en(0x0);
				bl_duty = level;	
				//upmu_top2_bst_drv_ck_pdn(0x1);
			}
			return 0;
		#endif
		}
		
		else if (pmic_type == MT65XX_LED_PMIC_BUTTON) 
		{
			
			if (level) 
			{
				upmu_set_kpled_dim_duty(0x9);
				upmu_set_kpled_en(0x1);
			}
			else 
			{
				upmu_set_kpled_en(0x0);
			}
			return 0;
			
		}
		else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK0)
		{
			if(first_time == true)
			{
				upmu_set_isinks_ch1_en(0x0);  //sw workround for sync leds status 
				upmu_set_isinks_ch2_en(0x0); 
				first_time = false;
			}
	
					//hwBacklightISINKTuning(0x0, 0x3, 0x0, 0);  //register mode, ch0_step=4ma ,disable CABC
					// hwBacklightISINKTurnOn(0x0);  //turn on ISINK0 77 uses ISINK4&5
					
	
			//if(led_init_flag[0] == false)
			{
				//hwBacklightISINKTuning(MT65XX_LED_PMIC_NLED_ISINK4, PMIC_PWM_1, 0x0, 0);
				upmu_set_isinks_ch0_mode(PMIC_PWM_0);
				upmu_set_isinks_ch0_step(0x0);//4mA
				//hwPWMsetting(PMIC_PWM_1, 15, 8);
				upmu_set_isink_dim0_duty(15);
				upmu_set_isink_dim0_fsel(11);//6320 0.25KHz
				led_init_flag[0] = true;
			}
			
			if (level) 
			{
				//upmu_top2_bst_drv_ck_pdn(0x0);
				//upmu_set_rg_spk_div_pdn(0x01);
				//upmu_set_rg_spk_pwm_div_pdn(0x01);
				
				upmu_set_rg_bst_drv_1m_ck_pdn(0x0);
				#ifdef ISINK_CHOP_CLK
				//upmu_set_isinks0_chop_en(0x1);
				#endif
				upmu_set_isinks_ch0_en(0x01);
				
			}
			else 
			{
				upmu_set_isinks_ch0_en(0x00);
				//upmu_set_rg_bst_drv_1m_ck_pdn(0x1);
				//upmu_top2_bst_drv_ck_pdn(0x1);
			}
			return 0;
		}
		else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK1)
		{
			if(first_time == true)
			{
				upmu_set_isinks_ch0_en(0);  //sw workround for sync leds status
				upmu_set_isinks_ch2_en(0); 
				first_time = false;
			}
	
					//hwBacklightISINKTuning(0x0, 0x3, 0x0, 0);  //register mode, ch0_step=4ma ,disable CABC
					//hwBacklightISINKTurnOn(0x0);  //turn on ISINK0
	
			//if(led_init_flag[1] == false)
			{
				//hwBacklightISINKTuning(MT65XX_LED_PMIC_NLED_ISINK5, PMIC_PWM_2, 0x0, 0);
				upmu_set_isinks_ch1_mode(PMIC_PWM_0);
				upmu_set_isinks_ch1_step(0x3);//4mA
				//hwPWMsetting(PMIC_PWM_2, 15, 8);
				upmu_set_isink_dim1_duty(15);
				upmu_set_isink_dim1_fsel(11);//6320 0.25KHz
				led_init_flag[1] = true;
			}
			if (level) 
			{
				//upmu_top2_bst_drv_ck_pdn(0x0);
				
				//upmu_set_rg_spk_div_pdn(0x01);
				//upmu_set_rg_spk_pwm_div_pdn(0x01);
				upmu_set_rg_bst_drv_1m_ck_pdn(0x0);
				#ifdef ISINK_CHOP_CLK
				//upmu_set_isinks1_chop_en(0x1);
				#endif
				upmu_set_isinks_ch1_en(0x01);
			}
			else 
			{
				upmu_set_isinks_ch1_en(0x00);
				//upmu_set_rg_bst_drv_1m_ck_pdn(0x1);
				//upmu_top2_bst_drv_ck_pdn(0x1);
			}
			return 0;
		}
		else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK2)
		{
		
			if(first_time == true)
			{
				upmu_set_isinks_ch0_en(0);  //sw workround for sync leds status
				upmu_set_isinks_ch1_en(0);
				first_time = false;
			}
	
					//hwBacklightISINKTuning(0x0, 0x3, 0x0, 0);  //register mode, ch0_step=4ma ,disable CABC
					//hwBacklightISINKTurnOn(0x0);  //turn on ISINK0
	
			//if(led_init_flag[1] == false)
			{
				//hwBacklightISINKTuning(MT65XX_LED_PMIC_NLED_ISINK5, PMIC_PWM_2, 0x0, 0);
				upmu_set_isinks_ch2_mode(PMIC_PWM_0);
				upmu_set_isinks_ch2_step(0x3);//16mA
				//hwPWMsetting(PMIC_PWM_2, 15, 8);
				upmu_set_isink_dim2_duty(15);
				upmu_set_isink_dim2_fsel(11);//6320 0.25KHz
				led_init_flag[2] = true;
			}
			if (level) 
			{
				//upmu_top2_bst_drv_ck_pdn(0x0);
				
				//upmu_set_rg_spk_div_pdn(0x01);
				//upmu_set_rg_spk_pwm_div_pdn(0x01);
				upmu_set_rg_bst_drv_1m_ck_pdn(0x0);
				#ifdef ISINK_CHOP_CLK
				//upmu_set_isinks2_chop_en(0x1);
				#endif
				upmu_set_isinks_ch2_en(0x01);
			}
			else 
			{
				upmu_set_isinks_ch2_en(0x00);
				//upmu_set_rg_bst_drv_1m_ck_pdn(0x1);
				//upmu_top2_bst_drv_ck_pdn(0x1);
			}
			return 0;
		}
		else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK01)
		{
	
			//hwBacklightISINKTuning(0x0, 0x3, 0x0, 0);  //register mode, ch0_step=4ma ,disable CABC
			//hwBacklightISINKTurnOn(0x0);  //turn on ISINK0
	
			//if(led_init_flag[1] == false)
			{

				upmu_set_isinks_ch0_mode(PMIC_PWM_0);
				upmu_set_isinks_ch0_step(0x0);//4mA
				upmu_set_isink_dim0_duty(1);
				upmu_set_isink_dim0_fsel(1);//6320 1.5KHz
				//hwBacklightISINKTuning(MT65XX_LED_PMIC_NLED_ISINK5, PMIC_PWM_2, 0x0, 0);
				upmu_set_isinks_ch1_mode(PMIC_PWM_0);
				upmu_set_isinks_ch1_step(0x3);//4mA
				//hwPWMsetting(PMIC_PWM_2, 15, 8);
				upmu_set_isink_dim1_duty(15);
				upmu_set_isink_dim1_fsel(11);//6320 0.25KHz
				led_init_flag[0] = true;
				led_init_flag[1] = true;
			}
			if (level) 
			{
				//upmu_top2_bst_drv_ck_pdn(0x0);
				
				//upmu_set_rg_spk_div_pdn(0x01);
				//upmu_set_rg_spk_pwm_div_pdn(0x01);
				upmu_set_rg_bst_drv_1m_ck_pdn(0x0);
				#ifdef ISINK_CHOP_CLK
				//upmu_set_isinks0_chop_en(0x1);
				//upmu_set_isinks1_chop_en(0x1);
				#endif
				upmu_set_isinks_ch0_en(0x01);
				upmu_set_isinks_ch1_en(0x01);
			}
			else 
			{
				upmu_set_isinks_ch0_en(0x00);
				upmu_set_isinks_ch1_en(0x00);
				//upmu_set_rg_bst_drv_1m_ck_pdn(0x1);
				//upmu_top2_bst_drv_ck_pdn(0x1);
			}
			return 0;
		}
		return -1;

}

#elif defined CONFIG_ARCH_MT6582
static int brightness_set_pmic(enum mt65xx_led_pmic pmic_type, u32 level, u32 div)
{
		
			
		return -1;

}

#elif defined CONFIG_ARCH_MT6572 || defined CONFIG_ARCH_MT6571
static int brightness_set_pmic(enum mt65xx_led_pmic pmic_type, u32 level, u32 div)
{
#define PMIC_ISINK_BACKLIGHT_LEVEL    80
	
	u32 tmp_level = level;
	static bool backlight_init_flag = false;
//	static bool first_time = true;
        struct PMIC_ISINK_CONFIG isink_config;
        static unsigned char duty_mapping[PMIC_ISINK_BACKLIGHT_LEVEL] = {
                0,	1,	2,	3,	4,	5,	6,	7,	8,	9,	10,	11,	
                12,     13,     14,     15,     16,     17,     18,     19,     20,     21,     22,     23,     
                24,     25,     26,     27,     28,     29,     30,     31,     16,     17,     18,     19,     
                20,     21,     22,     23,     24,     25,     26,     27,     28,     29,     30,     31,     
                21,     22,     23,     24,     25,     26,     27,     28,     29,     30,     31,     24,     
                25,     26,     27,     28,     29,     30,     31,     25,     26,     27,     28,     29,     
                30,     31,     26,     27,     28,     29,     30,     31,

    };
        static unsigned char current_mapping[PMIC_ISINK_BACKLIGHT_LEVEL] = {
                0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
                0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      
                0,      0,      0,      0,      0,      0,      0,      0,      1,      1,      1,      1,      
                1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      
                2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      3,      
                3,      3,      3,      3,      3,      3,      3,      4,      4,      4,      4,      4,      
                4,      4,      5,      5,      5,      5,      5,      5,
    };
	
	LEDS_DEBUG("[LED] PMIC Type: %d, Level: %d\n", pmic_type, level);
        led_register_dump();

        if (pmic_type == MT65XX_LED_PMIC_LCD_ISINK || \
            pmic_type == MT65XX_LED_PMIC_LCD_ISINK_GROUP0 || pmic_type == MT65XX_LED_PMIC_LCD_ISINK_GROUP1 || \
            pmic_type == MT65XX_LED_PMIC_LCD_ISINK_GROUP2 || pmic_type == MT65XX_LED_PMIC_LCD_ISINK_GROUP3)
	{
                LEDS_DETAIL_DEBUG("[LED] Backlight Configuration");
                // For backlight: Current: 24mA, PWM frequency: 20K, Soft start: off, Phase shift: on                
                isink_config.mode = ISINK_PWM_MODE;
                isink_config.dim_fsel = 0x2; // 20KHz
                isink_config.sfstr_tc = 0; // 0.5us
                isink_config.sfstr_en = 0; // Disable soft start
                isink_config.breath_trf_sel = 0x00; // 0.123s
                isink_config.breath_ton_sel = 0x00; // 0.123s
                isink_config.breath_toff_sel = 0x00; // 0.246s                
                
		if(backlight_init_flag == false)
		{
			upmu_set_isink_phase_dly_tc(0x0); // TC = 0.5us
                        PMIC_Backlight_Control(pmic_type, ISINK_BACKLIGHT_INIT, &isink_config);     
			backlight_init_flag = true;
		}
		
		if (level) 
		{
			level = brightness_mapping(tmp_level);
			if(level == ERROR_BL_LEVEL)
				level = limit;
#if 0            
            if(((level << 5) / limit) < 1)
            {
                level = 0;
            }
            else
            {
                level = ((level << 5) / limit) - 1;
            }
#endif      

#ifdef CONTROL_BL_TEMPERATURE
            if(level >= limit && limit_flag != 0)
            {
                level = limit; // Backlight cooler will limit max level
            }
#endif
            if(level == 255)
            {
                                level = PMIC_ISINK_BACKLIGHT_LEVEL;
            }
            else
            {
                                level = ((level * PMIC_ISINK_BACKLIGHT_LEVEL) / 255) + 1;
            }

            LEDS_DEBUG("[LED]Level Mapping = %d \n", level);
            LEDS_DEBUG("[LED]ISINK DIM Duty = %d \n", duty_mapping[level-1]);
            LEDS_DEBUG("[LED]ISINK Current = %d \n", current_mapping[level-1]);
                        isink_config.duty = duty_mapping[level - 1];
                        isink_config.step = current_mapping[level - 1];
                        LEDS_DEBUG("[LED] isink_config.duty = %d \n", isink_config.duty);
                        LEDS_DEBUG("[LED] isink_config.step = %d \n", isink_config.step);                        
                        PMIC_Backlight_Control(pmic_type, ISINK_BACKLIGHT_ADJUST, &isink_config);
			bl_duty = level;	
		}
		else 
		{
                        PMIC_Backlight_Control(pmic_type, ISINK_BACKLIGHT_DISABLE, &isink_config);
			bl_duty = level;	
		}
        
		return 0;
	}
	else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK0 || pmic_type == MT65XX_LED_PMIC_NLED_ISINK1 || \
                pmic_type == MT65XX_LED_PMIC_NLED_ISINK2 || pmic_type == MT65XX_LED_PMIC_NLED_ISINK3 || \
                pmic_type == MT65XX_LED_PMIC_NLED_ISINK_GROUP0 || pmic_type == MT65XX_LED_PMIC_NLED_ISINK_GROUP1 || \
                pmic_type == MT65XX_LED_PMIC_NLED_ISINK_GROUP2 || pmic_type == MT65XX_LED_PMIC_NLED_ISINK_GROUP3)
	{
                LEDS_DETAIL_DEBUG("[LED] LED Configuration");
                isink_config.mode = ISINK_PWM_MODE;
                isink_config.duty = 15; // 16 / 32
                isink_config.dim_fsel = 0; // 1KHz
                isink_config.sfstr_tc = 0; // 0.5us
                isink_config.sfstr_en = 0; // Disable soft start
                isink_config.breath_trf_sel = 0x00; // 0.123s
                isink_config.breath_ton_sel = 0x00; // 0.123s
                isink_config.breath_toff_sel = 0x00; // 0.246s                
                PMIC_Led_Brightness_Control(pmic_type, &isink_config, level);
		return 0;
	}

	return -1;
}

#elif  defined (CONFIG_ARCH_MT6575) || defined (CONFIG_ARCH_MT6575T)|| defined (CONFIG_ARCH_MT6577)
static int brightness_set_pmic(enum mt65xx_led_pmic pmic_type, u32 level, u32 div)
{
	int tmp_level = level;
	static bool backlight_init_flag[2] = {false, false};
	static bool led_init_flag[2] = {false, false};
	static bool first_time = true;
	
	LEDS_DEBUG("[LED]PMIC#%d:%d\n", pmic_type, level);

	if (pmic_type == MT65XX_LED_PMIC_LCD_ISINK)
	{
		if(backlight_init_flag[0] == false)
		{
			hwBacklightISINKTuning(1, PMIC_PWM_0, 0x3, 0);
			hwBacklightISINKTuning(2, PMIC_PWM_0, 0x3, 0);
			hwBacklightISINKTuning(3, PMIC_PWM_0, 0x3, 0);
			backlight_init_flag[0] = true;
		}
		
		if (level) 
		{
			level = brightness_mapping(tmp_level);
			if(level == ERROR_BL_LEVEL)
				level = tmp_level/17;
			hwPWMsetting(PMIC_PWM_0, level, div);
			upmu_top2_bst_drv_ck_pdn(0x0);
			hwBacklightISINKTurnOn(1);
			hwBacklightISINKTurnOn(2);
			hwBacklightISINKTurnOn(3);
			bl_duty = level;	
		}
		else 
		{
			hwBacklightISINKTurnOff(1);
			hwBacklightISINKTurnOff(2);
			hwBacklightISINKTurnOff(3);
			bl_duty = level;	
		}
		return 0;
	}
	else if(pmic_type == MT65XX_LED_PMIC_LCD_BOOST)
	{
		/*
		if(backlight_init_flag[1] == false)
		{
			hwBacklightBoostTuning(PMIC_PWM_0, 0xC, 0);
			backlight_init_flag[1] = true;
		}
		*/		
		if (level) 
		{
			level = brightness_mapping(tmp_level);
			if(level == ERROR_BL_LEVEL)
				level = tmp_level/42;
	
			upmu_boost_isink_hw_sel(0x1);
			upmu_boost_mode(3);
			upmu_boost_cabc_en(0);

			switch(level)
			{
				case 0:				
					upmu_boost_vrsel(0x0);
					//hwPWMsetting(PMIC_PWM_0, 0, 0x15);
					break;
				case 1:
					upmu_boost_vrsel(0x1);
					//hwPWMsetting(PMIC_PWM_0, 4, 0x15);
					break;
				case 2:
					upmu_boost_vrsel(0x2);
					//hwPWMsetting(PMIC_PWM_0, 5, 0x15);
					break;
				case 3:
					upmu_boost_vrsel(0x3);
					//hwPWMsetting(PMIC_PWM_0, 6, 0x15);
					break;
				case 4:
					upmu_boost_vrsel(0x5);
					//hwPWMsetting(PMIC_PWM_0, 7, 0x15);
					break;
				case 5:
					upmu_boost_vrsel(0x8);
					//hwPWMsetting(PMIC_PWM_0, 8, 0x15);
					break;
				case 6:
					upmu_boost_vrsel(0xB);
					//hwPWMsetting(PMIC_PWM_0, 9, 0x15);
					break;
				default:
					printk("[LED] invalid brightness level %d->%d\n", tmp_level, level);
					break;
			}
			
			upmu_top2_bst_drv_ck_pdn(0x0);
			upmu_boost_en(0x1);
			bl_duty = level;	
		}
		else 
		{
			upmu_boost_en(0x0);
			bl_duty = level;	
			//upmu_top2_bst_drv_ck_pdn(0x1);
		}
		return 0;
	}
	else if (pmic_type == MT65XX_LED_PMIC_BUTTON) 
	{
		if (level) 
		{
			hwBacklightKPLEDTuning(0x9, 0x0);
			hwBacklightKPLEDTurnOn();
		}
		else 
		{
			hwBacklightKPLEDTurnOff();
		}
		return 0;
	}
	else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK4)
	{
		if(first_time == true)
		{
			hwBacklightISINKTurnOff(MT65XX_LED_PMIC_NLED_ISINK5);  //sw workround for sync leds status 
			first_time = false;
		}

                hwBacklightISINKTuning(0x0, 0x3, 0x0, 0);  //register mode, ch0_step=4ma ,disable CABC
                hwBacklightISINKTurnOn(0x0);  //turn on ISINK0

		//if(led_init_flag[0] == false)
		{
			hwBacklightISINKTuning(MT65XX_LED_PMIC_NLED_ISINK4, PMIC_PWM_1, 0x0, 0);
			hwPWMsetting(PMIC_PWM_1, 15, 8);
			led_init_flag[0] = true;
		}
		
		if (level) 
		{
			upmu_top2_bst_drv_ck_pdn(0x0);
			hwBacklightISINKTurnOn(MT65XX_LED_PMIC_NLED_ISINK4);
		}
		else 
		{
			hwBacklightISINKTurnOff(MT65XX_LED_PMIC_NLED_ISINK4);
			//upmu_top2_bst_drv_ck_pdn(0x1);
		}
		return 0;
	}
	else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK5)
	{
		if(first_time == true)
		{
			hwBacklightISINKTurnOff(MT65XX_LED_PMIC_NLED_ISINK4);  //sw workround for sync leds status
			first_time = false;
		}

                hwBacklightISINKTuning(0x0, 0x3, 0x0, 0);  //register mode, ch0_step=4ma ,disable CABC
                hwBacklightISINKTurnOn(0x0);  //turn on ISINK0

		//if(led_init_flag[1] == false)
		{
			hwBacklightISINKTuning(MT65XX_LED_PMIC_NLED_ISINK5, PMIC_PWM_2, 0x0, 0);
			hwPWMsetting(PMIC_PWM_2, 15, 8);
			led_init_flag[1] = true;
		}
		if (level) 
		{
			upmu_top2_bst_drv_ck_pdn(0x0);
			hwBacklightISINKTurnOn(MT65XX_LED_PMIC_NLED_ISINK5);
		}
		else 
		{
			hwBacklightISINKTurnOff(MT65XX_LED_PMIC_NLED_ISINK5);
			//upmu_top2_bst_drv_ck_pdn(0x1);
		}
		return 0;
	}
	
	return -1;
}

#endif


#if 0
static int brightness_set_gpio(int gpio_num, enum led_brightness level)
{
	LEDS_DEBUG("[LED]GPIO#%d:%d\n", gpio_num, level);
	mt_set_gpio_mode(gpio_num, GPIO_MODE_GPIO);
	mt_set_gpio_dir(gpio_num, GPIO_DIR_OUT);

	if (level)
		mt_set_gpio_out(gpio_num, GPIO_OUT_ONE);
	else
		mt_set_gpio_out(gpio_num, GPIO_OUT_ZERO);

	return 0;
}
#endif

#if defined CONFIG_ARCH_MT6589
static int mt65xx_led_set_cust(struct cust_mt65xx_led *cust, int level)
{
	struct nled_setting led_tmp_setting = {0,0,0};
	int tmp_level = level;

//Mark out since the level is already cliped before sending in
/*
	if (level > LED_FULL)
		level = LED_FULL;
	else if (level < 0)
		level = 0;
*/

    printk("mt65xx_leds_set_cust: set brightness, name:%s, mode:%d, level:%d\n", 
		cust->name, cust->mode, level);
	switch (cust->mode) {
		
		case MT65XX_LED_MODE_PWM:
			if(strcmp(cust->name,"lcd-backlight") == 0)
			{
				bl_brightness = level;
				if(level == 0)
				{
					//mt_set_pwm_disable(cust->data);
					//mt_pwm_power_off (cust->data);
					mt_pwm_disable(cust->data, cust->config_data.pmic_pad);
					
				}else
				{
					level = brightness_mapping(tmp_level);
					if(level == ERROR_BL_LEVEL)
						level = brightness_mapto64(tmp_level);
						
					backlight_set_pwm(cust->data, level, bl_div,&cust->config_data);
				}
                bl_duty = level;	
				
			}else
			{
				if(level == 0)
				{
					led_tmp_setting.nled_mode = NLED_OFF;
				}else
				{
					led_tmp_setting.nled_mode = NLED_ON;
				}
				led_set_pwm(cust->data,&led_tmp_setting);
			}
			return 1;
          
		case MT65XX_LED_MODE_GPIO:
			//printk("brightness_set_cust:go GPIO mode!!!!!\n");
			#ifdef CONTROL_BL_TEMPERATURE
			mutex_lock(&bl_level_limit_mutex);
			current_level = level;
			//printk("brightness_set_cust:current_level=%d\n", current_level);
			if(0 == limit_flag){
				last_level = level;
				//printk("brightness_set_cust:last_level=%d\n", last_level);
			}else {
					if(limit < current_level){
						level = limit;
						printk("backlight_set_cust: control level=%d\n", level);
					}
			}	
			mutex_unlock(&bl_level_limit_mutex);
			#endif
			return ((cust_set_brightness)(cust->data))(level);
              
		case MT65XX_LED_MODE_PMIC:
			return brightness_set_pmic(cust->data, level, bl_div);
            
		case MT65XX_LED_MODE_CUST_LCM:
            if(strcmp(cust->name,"lcd-backlight") == 0)
			{
			    bl_brightness = level;
            }
			#ifdef CONTROL_BL_TEMPERATURE
			mutex_lock(&bl_level_limit_mutex);
			current_level = level;
			//printk("brightness_set_cust:current_level=%d\n", current_level);
			if(0 == limit_flag){
				last_level = level;
				//printk("brightness_set_cust:last_level=%d\n", last_level);
			}else {
					if(limit < current_level){
						level = limit;
						printk("backlight_set_cust: control level=%d\n", level);
					}
			}	
			mutex_unlock(&bl_level_limit_mutex);
			#endif
			//printk("brightness_set_cust:backlight control by LCM\n");
			return ((cust_brightness_set)(cust->data))(level, bl_div);

		
		case MT65XX_LED_MODE_CUST_BLS_PWM:
					if(strcmp(cust->name,"lcd-backlight") == 0)
					{
						bl_brightness = level;
					}
			#ifdef CONTROL_BL_TEMPERATURE
					mutex_lock(&bl_level_limit_mutex);
					current_level = level;
					//printk("brightness_set_cust:current_level=%d\n", current_level);
					if(0 == limit_flag){
						last_level = level;
						//printk("brightness_set_cust:last_level=%d\n", last_level);
					}else {
							if(limit < current_level){
								level = limit;
								printk("backlight_set_cust: control level=%d\n", level);
							}
					}	
					mutex_unlock(&bl_level_limit_mutex);
			#endif
					//printk("brightness_set_cust:backlight control by BLS_PWM!!\n");
			//#if !defined (MTK_AAL_SUPPORT)
			return ((cust_set_brightness)(cust->data))(level);
			printk("brightness_set_cust:backlight control by BLS_PWM done!!\n");
			//#endif
            
		case MT65XX_LED_MODE_NONE:
		default:
			break;
	}
	return -1;
}

#elif defined CONFIG_ARCH_MT6582
static int mt65xx_led_set_cust(struct cust_mt65xx_led *cust, int level)
{
	
			
			
	return -1;
}

#elif defined CONFIG_ARCH_MT6572 || defined CONFIG_ARCH_MT6571 
static int mt65xx_led_set_cust(struct cust_mt65xx_led *cust, int level)
{
	struct nled_setting led_tmp_setting = {0, 0, 0};
	int tmp_level = level;

//Mark out since the level is already cliped before sending in
/*
	if (level > LED_FULL)
		level = LED_FULL;
	else if (level < 0)
		level = 0;
*/

    printk("mt65xx_leds_set_cust: set brightness, name:%s, mode:%d, level:%d\n", 
		cust->name, cust->mode, level);


	#ifdef CONTROL_BL_TEMPERATURE
	if(strcmp(cust->name, "lcd-backlight") == 0)
	{
		mutex_lock(&bl_level_limit_mutex);
		current_level = level;
		//printk("lcd-backlight: brightness_set_cust:current_level=%d\n", current_level);
		if(0 == limit_flag){
			last_level = level;
			//printk("lcd-backlight: brightness_set_cust:last_level=%d\n", last_level);
		}else {
			if(limit < current_level){
				level = limit;
				//printk("lcd-backlight: backlight_set_cust: control level = %d\n", level);
			}
		}	
		mutex_unlock(&bl_level_limit_mutex);
	}
	#endif
	
	switch (cust->mode) {
		
		case MT65XX_LED_MODE_PWM:
			if(strcmp(cust->name, "lcd-backlight") == 0)
			{
				bl_brightness = level;
				if(level == 0)
				{
					mt_pwm_disable(cust->data, cust->config_data.pmic_pad);
				}else
				{
					level = brightness_mapping(tmp_level);
					if(level == ERROR_BL_LEVEL)
						level = brightness_mapto64(tmp_level);						
					backlight_set_pwm(cust->data, level, bl_div,&cust->config_data);
				}
                bl_duty = level;		
			}else
			{
				if(level == 0)
				{
					led_tmp_setting.nled_mode = NLED_OFF;
					led_set_pwm(cust->data,&led_tmp_setting);
					mt_pwm_disable(cust->data, cust->config_data.pmic_pad);
				}else
				{
					led_tmp_setting.nled_mode = NLED_ON;
					led_set_pwm(cust->data,&led_tmp_setting);
				}
			}
			return 1;
          
		case MT65XX_LED_MODE_GPIO:
			return ((cust_set_brightness)(cust->data))(level);
              
		case MT65XX_LED_MODE_PMIC:
			return brightness_set_pmic(cust->data, level, bl_div);
            
		case MT65XX_LED_MODE_CUST_LCM:
            if(strcmp(cust->name, "lcd-backlight") == 0)
			{
			    bl_brightness = level;
            }
			//printk("brightness_set_cust:backlight control by LCM\n");
			return ((cust_brightness_set)(cust->data))(level, bl_div);
		
        case MT65XX_LED_MODE_CUST_BLS_PWM:
			if(strcmp(cust->name, "lcd-backlight") == 0)
			{
				bl_brightness = level;
			}
			//printk("brightness_set_cust:backlight control by BLS_PWM!!\n");
			//#if !defined (MTK_AAL_SUPPORT)
    		printk("brightness_set_cust:backlight control by BLS_PWM done!!\n");
			return ((cust_set_brightness)(cust->data))(level);
			//#endif
            
		case MT65XX_LED_MODE_NONE:
            break;
            
		default:
			break;
	}
	return -1;	
}


#elif defined (CONFIG_ARCH_MT6575) || defined (CONFIG_ARCH_MT6575T)|| defined (CONFIG_ARCH_MT6577)
	static int brightness_set_gpio(int gpio_num, enum led_brightness level)
	{
		LEDS_DEBUG("[LED]GPIO#%d:%d\n", gpio_num, level);
		mt_set_gpio_mode(gpio_num, GPIO_MODE_GPIO);
		mt_set_gpio_dir(gpio_num, GPIO_DIR_OUT);
	
		if (level)
			mt_set_gpio_out(gpio_num, GPIO_OUT_ONE);
		else
			mt_set_gpio_out(gpio_num, GPIO_OUT_ZERO);
	
		return 0;
	}

	static int mt65xx_led_set_cust(struct cust_mt65xx_led *cust, int level)
	{
		struct nled_setting led_tmp_setting = {0,0,0};
		int tmp_level;
	
/*
	#ifdef CONTROL_BL_TEMPERATURE
		level = control_backlight_brightness(level);
		
       #endif
*/
		tmp_level = level;
		if (level > LED_FULL)
			level = LED_FULL;
		else if (level < 0)
			level = 0;
	
		printk("mt65xx_leds_set_cust: set brightness, name:%s, mode:%d, level:%d\n", 
			cust->name, cust->mode, level);
		switch (cust->mode) {
			case MT65XX_LED_MODE_PWM:
				if(strcmp(cust->name,"lcd-backlight") == 0)
				{
					bl_brightness = level;
					if(level == 0)
					{
						mt_set_pwm_disable(cust->data);
						mt_pwm_power_off (cust->data);
					}else
					{
						level = brightness_mapping(tmp_level);
						if(level == ERROR_BL_LEVEL)
							level = brightness_mapto64(tmp_level);
							
						backlight_set_pwm(cust->data, level, bl_div,&cust->config_data);
					}
					bl_duty = level;	
					
				}else
				{
					if(level == 0)
					{
						led_tmp_setting.nled_mode = NLED_OFF;
					}else
					{
						led_tmp_setting.nled_mode = NLED_ON;
					}
					led_set_pwm(cust->data,&led_tmp_setting);
				}
				return 1;
				
			case MT65XX_LED_MODE_GPIO:
				return brightness_set_gpio(cust->data, level);
				
			case MT65XX_LED_MODE_PMIC:
				return brightness_set_pmic(cust->data, level, bl_div);
				
			case MT65XX_LED_MODE_CUST:
				if(strcmp(cust->name,"lcd-backlight") == 0)
				{
					bl_brightness = level;
				}
			#ifdef CONTROL_BL_TEMPERATURE
				mutex_lock(&bl_level_limit_mutex);
				current_level = level;
				//printk("brightness_set_cust:current_level=%d\n", current_level);
				if(0 == limit_flag){
					last_level = level;
					//printk("brightness_set_cust:last_level=%d\n", last_level);
				}else {
						if(limit < current_level){
							level = limit;
							printk("backlight_set_cust: control level=%d\n", level);
						}
				}	
				mutex_unlock(&bl_level_limit_mutex);
			#endif
				return ((cust_brightness_set)(cust->data))(level, bl_div);
				
			case MT65XX_LED_MODE_NONE:
			default:
				break;
		}
		return -1;
	}

#endif

static void mt65xx_led_work(struct work_struct *work)
{
	struct mt65xx_led_data *led_data =
		container_of(work, struct mt65xx_led_data, work);

	LEDS_DEBUG("[LED]%s:%d\n", led_data->cust.name, led_data->level);
	mutex_lock(&leds_mutex);
	mt65xx_led_set_cust(&led_data->cust, led_data->level);
	mutex_unlock(&leds_mutex);
}

static void mt65xx_led_set(struct led_classdev *led_cdev, enum led_brightness level)
{
	struct mt65xx_led_data *led_data =
		container_of(led_cdev, struct mt65xx_led_data, cdev);

#ifdef LED_INCREASE_LED_LEVEL_MTKPATCH

    if(level >> LED_RESERVEBIT_SHIFT)
    {
        if(LED_RESERVEBIT_PATTERN != (level >> LED_RESERVEBIT_SHIFT))
        {
            //sanity check for hidden code
            printk("incorrect input : %d,%d\n" , level , (level >> LED_RESERVEBIT_SHIFT));
            return;
        }

        if(MT65XX_LED_MODE_CUST_BLS_PWM != led_data->cust.mode)
        {
            //only BLS PWM support expand bit
            printk("Not BLS PWM %d \n" , led_data->cust.mode);
            return;
        }

        level &= ((1 << LED_RESERVEBIT_SHIFT) - 1);

        if((level +1) > (1 << LED_INTERNAL_LEVEL_BIT_CNT))
        {
            //clip to max value
            level = (1 << LED_INTERNAL_LEVEL_BIT_CNT) - 1;
        }

        led_cdev->brightness = ((level + (1 << (LED_INTERNAL_LEVEL_BIT_CNT - 9))) >> (LED_INTERNAL_LEVEL_BIT_CNT - 8));//brightness is 8 bit level
        if(led_cdev->brightness > led_cdev->max_brightness)
        {
            led_cdev->brightness = led_cdev->max_brightness;
        }

        if(led_data->level != level)
        {
            led_data->level = level;
            mt65xx_led_set_cust(&led_data->cust, led_data->level);
        }
    }
    else
    {
        if(led_data->level != level)
        {
            led_data->level = level;
            if(strcmp(led_data->cust.name,"lcd-backlight"))
            {
                if(MT65XX_LED_MODE_CUST_BLS_PWM == led_data->cust.mode)
                {
                    mt65xx_led_set_cust(&led_data->cust, ((((1 << LED_INTERNAL_LEVEL_BIT_CNT) - 1)*level + 127)/255));
                }
                else
                {
                    schedule_work(&led_data->work);
                }
            }
            else
            {
                LEDS_DEBUG("[LED]Set Backlight directly %d at time %lu\n",led_data->level,jiffies);
                mt65xx_led_set_cust(&led_data->cust, led_data->level);	
            }
        }
    }

#else

	// do something only when level is changed
	if (led_data->level != level) {
		led_data->level = level;
		if(strcmp(led_data->cust.name,"lcd-backlight"))
		{
				schedule_work(&led_data->work);
		}else
		{
				LEDS_DEBUG("[LED]Set Backlight directly %d at time %lu\n",led_data->level,jiffies);
				mt65xx_led_set_cust(&led_data->cust, led_data->level);	
		}
	}

#endif
}

static int  mt65xx_blink_set(struct led_classdev *led_cdev,
			     unsigned long *delay_on,
			     unsigned long *delay_off)
{
	struct mt65xx_led_data *led_data =
		container_of(led_cdev, struct mt65xx_led_data, cdev);
	static int got_wake_lock = 0;
	struct nled_setting nled_tmp_setting = {0,0,0};
	

	// only allow software blink when delay_on or delay_off changed
	if (*delay_on != led_data->delay_on || *delay_off != led_data->delay_off) {
		led_data->delay_on = *delay_on;
		led_data->delay_off = *delay_off;
		if (led_data->delay_on && led_data->delay_off) { // enable blink
			led_data->level = 255; // when enable blink  then to set the level  (255)
			//if(led_data->cust.mode == MT65XX_LED_MODE_PWM && 
			//(led_data->cust.data != PWM3 && led_data->cust.data != PWM4 && led_data->cust.data != PWM5))

			//AP PWM all support OLD mode in MT6589
			
			if(led_data->cust.mode == MT65XX_LED_MODE_PWM)
			{
				nled_tmp_setting.nled_mode = NLED_BLINK;
				nled_tmp_setting.blink_off_time = led_data->delay_off;
				nled_tmp_setting.blink_on_time = led_data->delay_on;
				led_set_pwm(led_data->cust.data,&nled_tmp_setting);
				return 0;
			}

			#if defined CONFIG_ARCH_MT6589
			else if((led_data->cust.mode == MT65XX_LED_MODE_PMIC) && (led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK0
				|| led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK1 || led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK2))
			{
				//if(get_chip_eco_ver() == CHIP_E2) {
				if(1){
					
					nled_tmp_setting.nled_mode = NLED_BLINK;
					nled_tmp_setting.blink_off_time = led_data->delay_off;
					nled_tmp_setting.blink_on_time = led_data->delay_on;
					led_blink_pmic(led_data->cust.data, &nled_tmp_setting);
					return 0;
				} else {
					wake_lock(&leds_suspend_lock);
				}
			}
			#endif
            
			#if defined CONFIG_ARCH_MT6572 || defined CONFIG_ARCH_MT6571
			else if((led_data->cust.mode == MT65XX_LED_MODE_PMIC) && \
                                (led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK0 || led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK1 || \
                                 led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK2 || led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK3 || \
                                 led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK_GROUP0 || led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK_GROUP1 || \
                                 led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK_GROUP2 || led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK_GROUP3))
			{			
				nled_tmp_setting.nled_mode = NLED_BLINK;
				nled_tmp_setting.blink_off_time = led_data->delay_off;
				nled_tmp_setting.blink_on_time = led_data->delay_on;
				led_blink_pmic(led_data->cust.data, &nled_tmp_setting);
				return 0;
			}
			#endif		
						
			#if defined (CONFIG_ARCH_MT6575) || defined (CONFIG_ARCH_MT6575T)|| defined (CONFIG_ARCH_MT6577)
			else if((led_data->cust.mode == MT65XX_LED_MODE_PMIC) && (led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK4
				|| led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK5))
			{
				if(get_chip_eco_ver() == CHIP_E2) {
					nled_tmp_setting.nled_mode = NLED_BLINK;
					nled_tmp_setting.blink_off_time = led_data->delay_off;
					nled_tmp_setting.blink_on_time = led_data->delay_on;
					led_blink_pmic(led_data->cust.data, &nled_tmp_setting);
					return 0;
				} else {
					wake_lock(&leds_suspend_lock);
				}
			}
			#endif		
			
			else if (!got_wake_lock) {
				wake_lock(&leds_suspend_lock);
				got_wake_lock = 1;
			}
		}
		else if (!led_data->delay_on && !led_data->delay_off) { // disable blink
			//if(led_data->cust.mode == MT65XX_LED_MODE_PWM && 
			//(led_data->cust.data != PWM3 && led_data->cust.data != PWM4 && led_data->cust.data != PWM5))

			//AP PWM all support OLD mode in MT6589
			
			if(led_data->cust.mode == MT65XX_LED_MODE_PWM)
			{
				nled_tmp_setting.nled_mode = NLED_OFF;
				led_set_pwm(led_data->cust.data,&nled_tmp_setting);
				return 0;
			}

			#if defined CONFIG_ARCH_MT6589
			else if((led_data->cust.mode == MT65XX_LED_MODE_PMIC) && (led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK0
				|| led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK1 || led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK2))
			{
				//if(get_chip_eco_ver() == CHIP_E2) {
				if(1){
					brightness_set_pmic(led_data->cust.data, 0, 0);
					return 0;
				} else {
					wake_unlock(&leds_suspend_lock);
				}
			}
			#endif	
			
			#if defined CONFIG_ARCH_MT6572 || defined CONFIG_ARCH_MT6571
			else if((led_data->cust.mode == MT65XX_LED_MODE_PMIC) && \
                                (led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK0 || led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK1 || \
                                 led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK2 || led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK3 || \
                                 led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK_GROUP0 || led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK_GROUP1 || \
                                 led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK_GROUP2 || led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK_GROUP3))				
			{
				brightness_set_pmic(led_data->cust.data, 0, 0);
				return 0;
			} 
			#endif	
						
			#if defined (CONFIG_ARCH_MT6575) || defined (CONFIG_ARCH_MT6575T)|| defined (CONFIG_ARCH_MT6577)
			else if((led_data->cust.mode == MT65XX_LED_MODE_PMIC) && (led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK4
				|| led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK5))
			{
				if(get_chip_eco_ver() == CHIP_E2) {
					brightness_set_pmic(led_data->cust.data, 0, 0);
					return 0;
				} else {
					wake_unlock(&leds_suspend_lock);
				}
			}
			#endif	
			
			else if (got_wake_lock) {
				wake_unlock(&leds_suspend_lock);
				got_wake_lock = 0;
			}
		}
		return -1;
	}

	// delay_on and delay_off are not changed
	return 0;
}
#if defined(CONFIG_ARCH_MT6572) || defined CONFIG_ARCH_MT6571
static void PMIC_ISINK_Enable(PMIC_ISINK_CHANNEL channel)
{
        LEDS_DETAIL_DEBUG("[LED] %s->ISINK%d: Enable\n", __func__, channel);

        switch (channel) {
	        case ISINK0:
                        upmu_set_rg_isink0_ck_pdn(0x0); // Disable power down                        
                        upmu_set_isink_ch0_en(0x1); // Turn on ISINK Channel 0 
                        break;
                case ISINK1:
                        upmu_set_rg_isink1_ck_pdn(0x0); // Disable power down                        
                        upmu_set_isink_ch1_en(0x1); // Turn on ISINK Channel 1
                        break;                        
                case ISINK2:
                        upmu_set_rg_isink2_ck_pdn(0x0); // Disable power down                        
                        upmu_set_isink_ch2_en(0x1); // Turn on ISINK Channel 2
                        break;     
                case ISINK3:
                        upmu_set_rg_isink3_ck_pdn(0x0); // Disable power down                        
                        upmu_set_isink_ch3_en(0x1); // Turn on ISINK Channel 3
                        break;                                                     
		default:
			break;                        
        }
        
        if(g_isink_data[channel]->usage == ISINK_AS_INDICATOR)
        {
                isink_indicator_en |= (1 << channel);
                upmu_set_rg_drv_32k_ck_pdn(0x0); // Disable power down                    
        }
        else if(g_isink_data[channel]->usage == ISINK_AS_BACKLIGHT)
        {
                isink_backlight_en |= (1 << channel);
                upmu_set_rg_drv_2m_ck_pdn(0x0); // Disable power down
        }
        LEDS_DETAIL_DEBUG("[LED] ISINK Usage: Indicator->%x, Backlight->%x\n", isink_indicator_en, isink_backlight_en);        
}

static void PMIC_ISINK_Disable(PMIC_ISINK_CHANNEL channel)
{
        LEDS_DETAIL_DEBUG("[LED] %s->ISINK%d: Disable\n", __func__, channel);
        
        switch (channel) {
	        case ISINK0:
                        upmu_set_isink_ch0_en(0x0); // Turn off ISINK Channel 0
                        mdelay(1);
                        upmu_set_rg_isink0_ck_pdn(0x1); // Enable power down
                        break;
                case ISINK1:
                        upmu_set_isink_ch1_en(0x0); // Turn off ISINK Channel 1
                        mdelay(1);
                        upmu_set_rg_isink1_ck_pdn(0x1); // Enable power down                        
                        break;                        
                case ISINK2:
                        upmu_set_isink_ch2_en(0x0); // Turn off ISINK Channel 2
                        mdelay(1);
                        upmu_set_rg_isink2_ck_pdn(0x1); // Enable power down                        
                        break;     
                case ISINK3:
                        upmu_set_isink_ch3_en(0x0); // Turn off ISINK Channel 3
                        mdelay(1);                        
                        upmu_set_rg_isink3_ck_pdn(0x1); // Enable power down                        
                        break;                                                     
		default:
			break;                        
        } 
        if(g_isink_data[channel]->usage == ISINK_AS_INDICATOR)
        {
                isink_indicator_en &= ~(1 << channel);
        }
        else if(g_isink_data[channel]->usage == ISINK_AS_BACKLIGHT)
        {
                isink_backlight_en &= ~(1 << channel);
        }
        LEDS_DEBUG("[LED] ISINK Usage: Indicator->%x, Backlight->%x\n", isink_indicator_en, isink_backlight_en);             
        if(0 == isink_indicator_en)
        {
                upmu_set_rg_drv_32k_ck_pdn(0x1); // Enable power down
        }
        else if(0 == isink_backlight_en)
        {
                upmu_set_rg_drv_2m_ck_pdn(0x1); // Enable power down
        }
}

static void PMIC_ISINK_Easy_Config(PMIC_ISINK_CHANNEL channel, struct PMIC_ISINK_CONFIG *config_data)
{
        PMIC_ISINK_USAGE usage = g_isink_data[channel]->usage;
        LEDS_DETAIL_DEBUG("[LED] PMIC_ISINK_Easy_Config->ISINK%d: usage = %d\n", channel, usage);
        LEDS_DETAIL_DEBUG("[LED] config_data->mode %d\n", config_data->mode);
        LEDS_DETAIL_DEBUG("[LED] config_data->duty %d\n", config_data->duty);
        LEDS_DETAIL_DEBUG("[LED] config_data->dim_fsel %d\n", config_data->dim_fsel);
        LEDS_DETAIL_DEBUG("[LED] config_data->sfstr_tc %d\n", config_data->sfstr_tc);
        LEDS_DETAIL_DEBUG("[LED] config_data->sfstr_en %d\n", config_data->sfstr_en);
        LEDS_DETAIL_DEBUG("[LED] config_data->breath_trf_sel %d\n", config_data->breath_trf_sel);        
        LEDS_DETAIL_DEBUG("[LED] config_data->breath_ton_sel %d\n", config_data->breath_ton_sel); 
        LEDS_DETAIL_DEBUG("[LED] config_data->breath_toff_sel %d\n", config_data->breath_toff_sel);        
	switch (channel) {
		case ISINK0:
                        // upmu_set_rg_isink0_ck_pdn(0x0); // Disable power down    
                        
                        if(usage == ISINK_AS_INDICATOR)
                        {
                                upmu_set_rg_isink0_ck_sel(0x0); // Freq = 32KHz for Indicator
                                upmu_set_isink_dim0_duty(config_data->duty);
                                upmu_set_isink_ch0_mode(config_data->mode);
                                upmu_set_isink_dim0_fsel(config_data->dim_fsel);
                                upmu_set_isink_sfstr0_tc(config_data->sfstr_tc);
                                upmu_set_isink_sfstr0_en(config_data->sfstr_en);
                                upmu_set_isink_breath0_trf_sel(config_data->breath_trf_sel);
                                upmu_set_isink_breath0_ton_sel(config_data->breath_ton_sel);
                                upmu_set_isink_breath0_toff_sel(config_data->breath_toff_sel);
                                upmu_set_isink_phase0_dly_en(0x0); // Disable phase delay
                                upmu_set_isink_chop0_en(0x0); // Disable CHOP clk                                   
                        }
                        else
                        {
                                upmu_set_rg_isink0_ck_sel(0x1); // Freq = 1Mhz for Backlight
                                upmu_set_isink_ch0_mode(config_data->mode);
                                upmu_set_isink_dim0_fsel(config_data->dim_fsel);
                                upmu_set_isink_sfstr0_tc(config_data->sfstr_tc);
                                upmu_set_isink_sfstr0_en(config_data->sfstr_en);
                                upmu_set_isink_breath0_trf_sel(config_data->breath_trf_sel);
                                upmu_set_isink_breath0_ton_sel(config_data->breath_ton_sel);
                                upmu_set_isink_breath0_toff_sel(config_data->breath_toff_sel);
                                upmu_set_isink_phase0_dly_en(0x1); // Enable phase delay
                                upmu_set_isink_chop0_en(0x1); // Enable CHOP clk                                   
                        }
                        break;
                        
		case ISINK1:
                        // upmu_set_rg_isink1_ck_pdn(0x0); // Disable power down    
                        
                        if(usage == ISINK_AS_INDICATOR)
                        {
                                upmu_set_rg_isink1_ck_sel(0x0); // Freq = 32KHz for Indicator            
                                upmu_set_isink_dim1_duty(config_data->duty);
                                upmu_set_isink_ch1_mode(config_data->mode);
                                upmu_set_isink_dim1_fsel(config_data->dim_fsel);
                                upmu_set_isink_sfstr1_tc(config_data->sfstr_tc);
                                upmu_set_isink_sfstr1_en(config_data->sfstr_en);
                                upmu_set_isink_breath0_trf_sel(config_data->breath_trf_sel);
                                upmu_set_isink_breath0_ton_sel(config_data->breath_ton_sel);
                                upmu_set_isink_breath0_toff_sel(config_data->breath_toff_sel);                                
        			upmu_set_isink_phase1_dly_en(0x0); // Disable phase delay
                                upmu_set_isink_chop1_en(0x0); // Disable CHOP clk                                   
                        }
                        else
                        {
                                upmu_set_rg_isink1_ck_sel(0x1); // Freq = 1Mhz for Backlight
                                upmu_set_isink_ch1_mode(config_data->mode);
                                upmu_set_isink_dim1_fsel(config_data->dim_fsel);
                                upmu_set_isink_sfstr1_tc(config_data->sfstr_tc);
                                upmu_set_isink_sfstr1_en(config_data->sfstr_en);
                                upmu_set_isink_breath1_trf_sel(config_data->breath_trf_sel);
                                upmu_set_isink_breath1_ton_sel(config_data->breath_ton_sel);
                                upmu_set_isink_breath1_toff_sel(config_data->breath_toff_sel);
                                upmu_set_isink_phase1_dly_en(0x1); // Enable phase delay
                                upmu_set_isink_chop1_en(0x1); // Enable CHOP clk      
                        }
                        break;
                        
		case ISINK2:
                        // upmu_set_rg_isink2_ck_pdn(0x0); // Disable power down    
                        
                        if(usage == ISINK_AS_INDICATOR)
                        {
                                upmu_set_rg_isink2_ck_sel(0x0); // Freq = 32KHz for Indicator            
                                upmu_set_isink_dim1_duty(config_data->duty);                                
                                upmu_set_isink_ch2_mode(config_data->mode);
                                upmu_set_isink_dim2_fsel(config_data->dim_fsel);
                                upmu_set_isink_sfstr2_tc(config_data->sfstr_tc);
                                upmu_set_isink_sfstr2_en(config_data->sfstr_en);
                                upmu_set_isink_breath2_trf_sel(config_data->breath_trf_sel);
                                upmu_set_isink_breath2_ton_sel(config_data->breath_ton_sel);
                                upmu_set_isink_breath2_toff_sel(config_data->breath_toff_sel);                                
        			upmu_set_isink_phase2_dly_en(0x0); // Disable phase delay
                                upmu_set_isink_chop2_en(0x0); // Disable CHOP clk                                   
                        }
                        else
                        {
                                upmu_set_rg_isink2_ck_sel(0x1); // Freq = 1Mhz for Backlight
                                upmu_set_isink_ch2_mode(config_data->mode);
                                upmu_set_isink_dim2_fsel(config_data->dim_fsel);
                                upmu_set_isink_sfstr2_tc(config_data->sfstr_tc);
                                upmu_set_isink_sfstr2_en(config_data->sfstr_en);
                                upmu_set_isink_breath2_trf_sel(config_data->breath_trf_sel);
                                upmu_set_isink_breath2_ton_sel(config_data->breath_ton_sel);
                                upmu_set_isink_breath2_toff_sel(config_data->breath_toff_sel);
                                upmu_set_isink_phase2_dly_en(0x1); // Enable phase delay
                                upmu_set_isink_chop2_en(0x1); // Enable CHOP clk    
                        }
                        break;
                        
		case ISINK3:
                        // upmu_set_rg_isink3_ck_pdn(0x0); // Disable power down    
                        
                        if(usage == ISINK_AS_INDICATOR)
                        {
                                upmu_set_rg_isink3_ck_sel(0x0); // Freq = 32KHz for Indicator            
                                upmu_set_isink_dim3_duty(config_data->duty);                                 
                                upmu_set_isink_ch3_mode(config_data->mode);
                                upmu_set_isink_dim3_fsel(config_data->dim_fsel);
                                upmu_set_isink_sfstr3_tc(config_data->sfstr_tc);
                                upmu_set_isink_sfstr3_en(config_data->sfstr_en);
                                upmu_set_isink_breath3_trf_sel(config_data->breath_trf_sel);
                                upmu_set_isink_breath3_ton_sel(config_data->breath_ton_sel);
                                upmu_set_isink_breath3_toff_sel(config_data->breath_toff_sel);                                
        			upmu_set_isink_phase3_dly_en(0x0); // Disable phase delay
                                upmu_set_isink_chop3_en(0x0); // Disable CHOP clk                                   
                        }
                        else
                        {
                                upmu_set_rg_isink3_ck_sel(0x1); // Freq = 1Mhz for Backlight
                                upmu_set_isink_ch3_mode(config_data->mode);
                                upmu_set_isink_dim3_fsel(config_data->dim_fsel);
                                upmu_set_isink_sfstr3_tc(config_data->sfstr_tc);
                                upmu_set_isink_sfstr3_en(config_data->sfstr_en);
                                upmu_set_isink_breath3_trf_sel(config_data->breath_trf_sel);
                                upmu_set_isink_breath3_ton_sel(config_data->breath_ton_sel);
                                upmu_set_isink_breath3_toff_sel(config_data->breath_toff_sel);
                                upmu_set_isink_phase3_dly_en(0x1); // Enable phase delay
                                upmu_set_isink_chop3_en(0x1); // Enable CHOP clk    
                        }
                        break;                             
 		default:
			break;                        

	}
        
}

static void PMIC_ISINK_Current_Config(PMIC_ISINK_CHANNEL channel)
{
        unsigned int isink_current = 0;
        unsigned int double_en = 0;
        unsigned int idx = 0;
        PMIC_ISINK_STEP step = g_isink_data[channel]->step;
        LEDS_DEBUG("[LED] %s\n", __func__);
        if(step == ISINK_STEP_FOR_BACK_LIGHT && g_isink_data[channel]->usage == ISINK_AS_BACKLIGHT)
        {
                LEDS_DETAIL_DEBUG("[LED] ISINK_STEP_48MA\n");
                step = ISINK_STEP_48MA;
        }
        if(step >  ISINK_STEP_24MA)
        {
                LEDS_DETAIL_DEBUG("[LED] Enable Double Bit\n");
                isink_current = step >> 1;
                double_en = 1;
        }
        idx = Value_to_RegisterIdx(isink_current, isink_step);
        if(idx == 0xFFFFFFFF)
        {
                LEDS_DETAIL_DEBUG("[LED] ISINK%d Current not available\n", channel);
        }
        LEDS_DETAIL_DEBUG("[LED] ISINK%d: step = %d, double_en = %d\n", channel, idx, double_en);
        switch (channel) {
		case ISINK0:
                        upmu_set_isink_ch0_step(idx);
                        upmu_set_rg_isink0_double_en(double_en);
                        break;
		case ISINK1:
                        upmu_set_isink_ch1_step(idx);
                        upmu_set_rg_isink1_double_en(double_en);
                        break;
		case ISINK2:
                        upmu_set_isink_ch2_step(idx);
                        upmu_set_rg_isink2_double_en(double_en);
                        break;
		case ISINK3:
                        upmu_set_isink_ch3_step(idx);
                        upmu_set_rg_isink3_double_en(double_en);
                        break;    
                default:
                        break;
        }          
}

static void PMIC_ISINK_Brightness_Config(PMIC_ISINK_CHANNEL channel, struct PMIC_ISINK_CONFIG *config_data)
{
        LEDS_DETAIL_DEBUG("[LED] %s, duty = %d, step = %d\n", __func__, config_data->duty, config_data->step);
        switch (channel) {
		case ISINK0:
                        upmu_set_isink_dim0_duty(config_data->duty);
                        upmu_set_isink_ch0_step(config_data->step);
                        break;
		case ISINK1:
                        upmu_set_isink_dim1_duty(config_data->duty);
                        upmu_set_isink_ch1_step(config_data->step);
                        break;
		case ISINK2:
                        upmu_set_isink_dim2_duty(config_data->duty);
                        upmu_set_isink_ch2_step(config_data->step);
                        break;
		case ISINK3:
                        upmu_set_isink_dim3_duty(config_data->duty);
                        upmu_set_isink_ch3_step(config_data->step);
                        break;                        
                default:
                        break;
        }                  
}

static void PMIC_Led_Control(enum mt65xx_led_pmic pmic_type, struct PMIC_ISINK_CONFIG *config_data)
{
        PMIC_ISINK_CHANNEL isink_channel;
        if(pmic_type >= MT65XX_LED_PMIC_NLED_ISINK_GROUP_BEGIN && pmic_type < MT65XX_LED_PMIC_NLED_ISINK_GROUP_END)
        {
                PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_EASY_CONFIG_API, config_data);
                PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_CURRENT_CONFIG_API, NULL);
                PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_ENABLE_API, NULL);
        }
        else
        {
                switch(pmic_type){
                        case MT65XX_LED_PMIC_NLED_ISINK0:        
                                isink_channel = ISINK0;
                                break;
                        case MT65XX_LED_PMIC_NLED_ISINK1:        
                                isink_channel = ISINK1;    
                                break;
                        case MT65XX_LED_PMIC_NLED_ISINK2:        
                                isink_channel = ISINK2;     
                                break;
                        case MT65XX_LED_PMIC_NLED_ISINK3:        
                                isink_channel = ISINK3;    
                                break;
                        default:
                                break;
                }                  
                PMIC_ISINK_Easy_Config(isink_channel, config_data);
                PMIC_ISINK_Current_Config(isink_channel);
                PMIC_ISINK_Enable(isink_channel);  
        }
}

static void PMIC_Led_Brightness_Control(enum mt65xx_led_pmic pmic_type, struct PMIC_ISINK_CONFIG *config_data, u32 level)
{
//        unsigned int i = 0;
//        unsigned int operation_channel = 0;
        static unsigned int led_first_time[ISINK_NUM] = {true, true, true, true};
        PMIC_ISINK_CHANNEL isink_channel = ISINK_NUM;

        LEDS_DEBUG("[LED] %s\n", __func__);        
        if(pmic_type >= MT65XX_LED_PMIC_NLED_ISINK_GROUP_BEGIN && pmic_type < MT65XX_LED_PMIC_NLED_ISINK_GROUP_END)
        {
                // operation_channel = PMIC_ISINK_CHANNEL_LOOKUP_BY_GROUP(pmic_type);

                PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_EASY_CONFIG_API,  config_data);
                PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_CURRENT_CONFIG_API, NULL);              
		
		if (level) 
		{
                        PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_ENABLE_API, NULL);
		}
		else 
		{
                        PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_DISABLE_API, NULL);
		}      
        }
        else
        {
                switch(pmic_type){
                        case MT65XX_LED_PMIC_NLED_ISINK0:        
                                isink_channel = ISINK0;
                                break;
                        case MT65XX_LED_PMIC_NLED_ISINK1:        
                                isink_channel = ISINK1;    
                                break;
                        case MT65XX_LED_PMIC_NLED_ISINK2:        
                                isink_channel = ISINK2;     
                                break;
                        case MT65XX_LED_PMIC_NLED_ISINK3:        
                                isink_channel = ISINK3;    
                                break;
                        default:
                                break;                                
                }   
                if(led_first_time[isink_channel] == true)
                {
                        PMIC_ISINK_Disable(isink_channel);
                        led_first_time[isink_channel] = false;
                }
                
                PMIC_ISINK_Easy_Config(isink_channel, config_data);
                PMIC_ISINK_Current_Config(isink_channel);
                
                if(level)
                {
                        PMIC_ISINK_Enable(isink_channel);  
                }
                else
                {
                        PMIC_ISINK_Disable(isink_channel);
                }
        }

}

static void PMIC_Backlight_Control(enum mt65xx_led_pmic pmic_type, PMIC_ISINK_BACKLIGHT_CONTROL control, struct PMIC_ISINK_CONFIG *config_data)
{
        unsigned int i;
        LEDS_DEBUG("[LED] %s\n", __func__);
        if(pmic_type >= MT65XX_LED_PMIC_LCD_ISINK_GROUP_BEGIN && pmic_type < MT65XX_LED_PMIC_LCD_ISINK_GROUP_END)
        {
                if(control == ISINK_BACKLIGHT_INIT)
                {
                        LEDS_DETAIL_DEBUG("[LED] Backlight Init\n");
                        PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_EASY_CONFIG_API,  config_data);
                        PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_CURRENT_CONFIG_API, NULL);                       
                }
                else if(control == ISINK_BACKLIGHT_ADJUST)
                {
                        LEDS_DETAIL_DEBUG("[LED] Backlight Adjust\n");
                        PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_BRIGHTNESS_CONFIG_API, config_data);   
                        PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_ENABLE_API, NULL);                            
                }
                else if(control == ISINK_BACKLIGHT_DISABLE)
                {
                        LEDS_DETAIL_DEBUG("[LED] Backlight Disable\n");
                        PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_DISABLE_API, NULL);
                }   
      
        }
        else if(pmic_type == MT65XX_LED_PMIC_LCD_ISINK)
        {       
                for(i = ISINK0; i < ISINK_NUM; i++)
                {
                        if(g_isink_data[i]->usage == ISINK_AS_BACKLIGHT)
                        {
                                if(control == ISINK_BACKLIGHT_INIT)
                                {
                                        PMIC_ISINK_Easy_Config(i, config_data);
                                        PMIC_ISINK_Current_Config(i);                                
                                }
                                else if(control == ISINK_BACKLIGHT_ADJUST)
                                {
                                        PMIC_ISINK_Brightness_Config(i, config_data);
                                        PMIC_ISINK_Enable(i);                                
                                }
                                else if(control == ISINK_BACKLIGHT_DISABLE)
                                {
                                        PMIC_ISINK_Disable(i);                                 
                                }   
                        }
                }
        }
}

static unsigned int PMIC_ISINK_CHANNEL_LOOKUP_BY_GROUP(enum mt65xx_led_pmic pmic_type)
{
        unsigned int i;
        unsigned int start_point = 0, end_point = 0;
        unsigned int operation_channel = 0;

        if(pmic_type >= MT65XX_LED_PMIC_NLED_ISINK_GROUP_BEGIN && pmic_type < MT65XX_LED_PMIC_NLED_ISINK_GROUP_END)
        {
                start_point = MT65XX_LED_PMIC_NLED_ISINK_GROUP_BEGIN;
                end_point = MT65XX_LED_PMIC_NLED_ISINK_GROUP_END;
        }
        else if (pmic_type >= MT65XX_LED_PMIC_LCD_ISINK_GROUP_BEGIN && pmic_type < MT65XX_LED_PMIC_LCD_ISINK_GROUP_END)
        {
                start_point = MT65XX_LED_PMIC_LCD_ISINK_GROUP_BEGIN;
                end_point = MT65XX_LED_PMIC_LCD_ISINK_GROUP_END;
        }         
       
        for(i = start_point; i < end_point; i++)
        {
                if(g_isink_data[i - start_point]->data == pmic_type)
                {
                        operation_channel |= (1 << (i - start_point));
                }
        }

        return operation_channel;                                
}

static void PMIC_ISINK_GROUP_CONTROL(enum mt65xx_led_pmic pmic_type, PMIC_ISINK_CONTROL_API handler, struct PMIC_ISINK_CONFIG *config_data)
{
        unsigned int i = 0, operation_channel = 0;
        operation_channel = PMIC_ISINK_CHANNEL_LOOKUP_BY_GROUP(pmic_type);
        LEDS_DETAIL_DEBUG("[LED] ISINK Group %d Operation: %x Handler = %d\n", pmic_type, operation_channel, handler);

        for(i = 0; i < ISINK_NUM; i++)
        {
                LEDS_DETAIL_DEBUG("[LED] Channel = %d\n", operation_channel & (1 << i));
                if(operation_channel & (1 << i))
                {
                        LEDS_DETAIL_DEBUG("[LED] ISINK Channel %d is configured\n", i);
                        switch (handler) {
                                case ISINK_ENABLE_API:
                                        PMIC_ISINK_Enable(i);
                                        break;
                                case ISINK_DISABLE_API:
                                        PMIC_ISINK_Disable(i);
                                        break;
                                case ISINK_EASY_CONFIG_API:
                                        PMIC_ISINK_Easy_Config(i, config_data);
                                        break;
                                case ISINK_CURRENT_CONFIG_API:
                                        PMIC_ISINK_Current_Config(i);
                                        break;
                                case ISINK_BRIGHTNESS_CONFIG_API:
                                        PMIC_ISINK_Brightness_Config(i, config_data);
                                        break;
                                default:
                                        break;                                        
                        }
                }
        }
}

static unsigned int Value_to_RegisterIdx(u32 val, u32 *table)
{
        unsigned int *index_array = table;
        unsigned int i = 0, idx = 0, size = ISINK_STEP_SIZE;
        LEDS_DETAIL_DEBUG("[LED] Current = %d, size = %d\n", val, size);
        for(i = 0; i < size; i++)
        {
                LEDS_DETAIL_DEBUG("[LED] Current = %d\n", index_array[i]);
       		if (val == index_array[i])
       		{
   		        idx  = i;
                        break;
        	}                
                else
                {
                        idx = 0xFFFFFFFF;
                }
        } 
        return idx;
}

static unsigned int led_pmic_pwrap_read(U32 addr)
{

	U32 val =0;
	pwrap_read(addr, &val);
	return val;
	
}

static void led_register_dump(void)
{  
#define ISINK_REG_OFFSET        0x0330        
#define ISINK_REG_END           0x0356        
        unsigned int i = 0;

        LEDS_DETAIL_DEBUG("[LED] RG_DRV_32K_CK_PDN [11]: %x\n", led_pmic_pwrap_read(0x0102));        
        LEDS_DETAIL_DEBUG("[LED] RG_DRV_2M_CK_PDN [6]: %x\n", led_pmic_pwrap_read(0x0108));
        LEDS_DETAIL_DEBUG("[LED] RG_ISINK#_CK_PDN [3:0]: %x\n", led_pmic_pwrap_read(0x010E));
        LEDS_DETAIL_DEBUG("[LED] RG_ISINK#_CK_SEL [13:10]: %x\n", led_pmic_pwrap_read(0x0126));
        for(i = ISINK_REG_OFFSET; i <= ISINK_REG_END; i += 2)
        {
                LEDS_DETAIL_DEBUG("[LED] ISINK_REG %x = %x\n", i, led_pmic_pwrap_read(i));
        }        
}
#endif // End of #if defined(CONFIG_ARCH_MT6572) || defined CONFIG_ARCH_MT6571
/****************************************************************************
 * external functions
 ***************************************************************************/
int mt65xx_leds_brightness_set(enum mt65xx_led_type type, enum led_brightness level)
{
	struct cust_mt65xx_led *cust_led_list = get_cust_led_list();

	LEDS_DEBUG("[LED]#%d:%d\n", type, level);

	if (type < 0 || type >= MT65XX_LED_TYPE_TOTAL)
		return -1;

	if (level > LED_FULL)
		level = LED_FULL;
	else if (level < 0)
		level = 0;

	return mt65xx_led_set_cust(&cust_led_list[type], level);

}

EXPORT_SYMBOL(mt65xx_leds_brightness_set);


static ssize_t show_duty(struct device *dev,struct device_attribute *attr, char *buf)
{
	LEDS_DEBUG("[LED]get backlight duty value is:%d \n",bl_duty);
	return sprintf(buf, "%u\n", bl_duty);
}
static ssize_t store_duty(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	char *pvalue = NULL;
	unsigned int level = 0;
	size_t count = 0;
	LEDS_DEBUG("set backlight duty start \n");
	level = simple_strtoul(buf,&pvalue,10);
	count = pvalue - buf;
	if (*pvalue && isspace(*pvalue))
		count++;
    
	if(count == size)
	{
	
		if(bl_setting->mode == MT65XX_LED_MODE_PMIC)
		{
			
#if defined (CONFIG_ARCH_MT6575) || defined (CONFIG_ARCH_MT6575T)|| defined (CONFIG_ARCH_MT6577)
			//duty:0-16
			if((level >= 0) && (level <= 15))
			{
				brightness_set_pmic(MT65XX_LED_PMIC_LCD_BOOST, (level*17), bl_div);
			}
			else
			{
				LEDS_DEBUG("duty value is error, please select vaule from [0-15]!\n");
			}
#endif			
		}
		
		else if(bl_setting->mode == MT65XX_LED_MODE_PWM)
		//if(bl_setting->mode == MT65XX_LED_MODE_PWM)
		{
			if(level == 0)
			{
			#if defined (CONFIG_ARCH_MT6575) || defined (CONFIG_ARCH_MT6575T)|| defined (CONFIG_ARCH_MT6577)
				mt_set_pwm_disable(bl_setting->data);
				mt_pwm_power_off (bl_setting->data);
			#elif defined (CONFIG_ARCH_MT6589) || defined (CONFIG_ARCH_MT6572) || defined (CONFIG_ARCH_MT6571)
				mt_pwm_disable(bl_setting->data, bl_setting->config_data.pmic_pad);	
			#endif
			}else if(level <= 64)
			{
				backlight_set_pwm(bl_setting->data,level, bl_div,&bl_setting->config_data);
			}	
		}
            
    
		bl_duty = level;

	}
	
	return size;
}


static DEVICE_ATTR(duty, 0664, show_duty, store_duty);


static ssize_t show_div(struct device *dev,struct device_attribute *attr, char *buf)
{
	LEDS_DEBUG("get backlight div value is:%d \n",bl_div);
	return sprintf(buf, "%u\n", bl_div);
}

static ssize_t store_div(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	char *pvalue = NULL;
	unsigned int div = 0;
	size_t count = 0;
	LEDS_DEBUG("set backlight div start \n");
	div = simple_strtoul(buf,&pvalue,10);
	count = pvalue - buf;
	
	if (*pvalue && isspace(*pvalue))
		count++;
		
	if(count == size)
	{
         if(div < 0 || (div > 7))
		{
            LEDS_DEBUG("set backlight div parameter error: %d[div:0~7]\n", div);
            return 0;
		}
        
		if(bl_setting->mode == MT65XX_LED_MODE_PWM)
		{
            LEDS_DEBUG("set PWM backlight div OK: div=%d, duty=%d\n", div, bl_duty);
            backlight_set_pwm(bl_setting->data, bl_duty, div,&bl_setting->config_data);
		}
		
        else if(bl_setting->mode == MT65XX_LED_MODE_CUST_LCM || bl_setting->mode == MT65XX_LED_MODE_CUST_BLS_PWM)
        {
            LEDS_DEBUG("set cust backlight div OK: div=%d, brightness=%d\n", div, bl_brightness);
	        ((cust_brightness_set)(bl_setting->data))(bl_brightness, div);
        }
        
		bl_div = div;
	}
	
	return size;
}

static DEVICE_ATTR(div, 0664, show_div, store_div);


static ssize_t show_frequency(struct device *dev,struct device_attribute *attr, char *buf)
{
    if(bl_setting->mode == MT65XX_LED_MODE_PWM)
    {
        bl_frequency = 32000/div_array[bl_div];
    }
    else if(bl_setting->mode == MT65XX_LED_MODE_CUST_LCM)
    {
        //mtkfb_get_backlight_pwm(bl_div, &bl_frequency);  		
    }

    LEDS_DEBUG("[LED]get backlight PWM frequency value is:%d \n", bl_frequency);
    
	return sprintf(buf, "%u\n", bl_frequency);
}

static DEVICE_ATTR(frequency, 0664, show_frequency, NULL);



static ssize_t store_pwm_register(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	char *pvalue = NULL;
	unsigned int reg_value = 0;
	unsigned int reg_address = 0;
	if(buf != NULL && size != 0)
	{
		LEDS_DEBUG("store_pwm_register: size:%d,address:0x%s\n", size,buf);
		reg_address = simple_strtoul(buf,&pvalue,16);
	
		if(*pvalue && (*pvalue == '#'))
		{
			reg_value = simple_strtoul((pvalue+1),NULL,16);
			LEDS_DEBUG("set pwm register:[0x%x]= 0x%x\n",reg_address,reg_value);
			OUTREG32(reg_address,reg_value);
			
		}else if(*pvalue && (*pvalue == '@'))
		{
			LEDS_DEBUG("get pwm register:[0x%x]=0x%x\n",reg_address,INREG32(reg_address));
		}	
	}

	return size;
}

static ssize_t show_pwm_register(struct device *dev,struct device_attribute *attr, char *buf)
{
	return 0;
}

static DEVICE_ATTR(pwm_register, 0664, show_pwm_register, store_pwm_register);


/****************************************************************************
 * driver functions
 ***************************************************************************/
static int __init mt65xx_leds_probe(struct platform_device *pdev)
{
	int i;
	int ret, rc;
	struct cust_mt65xx_led *cust_led_list = get_cust_led_list();
        struct PMIC_CUST_ISINK *isink_config = get_cust_isink_config();
	LEDS_DEBUG("[LED]%s\n", __func__);

	for (i = 0; i < MT65XX_LED_TYPE_TOTAL; i++) {
		if (cust_led_list[i].mode == MT65XX_LED_MODE_NONE) {
			g_leds_data[i] = NULL;
			continue;
		}

		g_leds_data[i] = kzalloc(sizeof(struct mt65xx_led_data), GFP_KERNEL);
		if (!g_leds_data[i]) {
			ret = -ENOMEM;
			goto err;
		}

		g_leds_data[i]->cust.mode = cust_led_list[i].mode;
		g_leds_data[i]->cust.data = cust_led_list[i].data;
		g_leds_data[i]->cust.name = cust_led_list[i].name;

		g_leds_data[i]->cdev.name = cust_led_list[i].name;
		g_leds_data[i]->cust.config_data = cust_led_list[i].config_data;//bei add

		g_leds_data[i]->cdev.brightness_set = mt65xx_led_set;
		g_leds_data[i]->cdev.blink_set = mt65xx_blink_set;

		INIT_WORK(&g_leds_data[i]->work, mt65xx_led_work);

		ret = led_classdev_register(&pdev->dev, &g_leds_data[i]->cdev);
        
		if(strcmp(g_leds_data[i]->cdev.name,"lcd-backlight") == 0)
		{
			rc = device_create_file(g_leds_data[i]->cdev.dev, &dev_attr_duty);
            if(rc)
            {
                LEDS_DEBUG("[LED]device_create_file duty fail!\n");
            }
            
            rc = device_create_file(g_leds_data[i]->cdev.dev, &dev_attr_div);
            if(rc)
            {
                LEDS_DEBUG("[LED]device_create_file duty fail!\n");
            }
            
            rc = device_create_file(g_leds_data[i]->cdev.dev, &dev_attr_frequency);
            if(rc)
            {
                LEDS_DEBUG("[LED]device_create_file duty fail!\n");
            }
            
	    rc = device_create_file(g_leds_data[i]->cdev.dev, &dev_attr_pwm_register);
            if(rc)
            {
                LEDS_DEBUG("[LED]device_create_file duty fail!\n");
            }
			bl_setting = &g_leds_data[i]->cust;
		}

		if (ret)
			goto err;
		
	}
#ifdef CONTROL_BL_TEMPERATURE
	
		last_level = 0;  
		limit = 255;
		limit_flag = 0; 
		current_level = 0;
		printk("[LED]led probe last_level = %d, limit = %d, limit_flag = %d, current_level = %d\n",last_level,limit,limit_flag,current_level);
#endif

#if defined(CONFIG_ARCH_MT6572) || defined CONFIG_ARCH_MT6571
        for (i = 0; i < ISINK_NUM; i++) {
                g_isink_data[i] = kzalloc(sizeof(struct PMIC_ISINK_CONFIG), GFP_KERNEL);
		if (!g_isink_data[i]) {
			ret = -ENOMEM;
			goto err_isink;
		}                
                g_isink_data[i]->usage = isink_config[i].usage;
                g_isink_data[i]->step = isink_config[i].step;
                g_isink_data[i]->data = isink_config[i].data;
		if (ret)
			goto err_isink;                
        }
#endif
	return 0;

err:
	if (i) {
		for (i = i-1; i >=0; i--) {
			if (!g_leds_data[i])
				continue;
			led_classdev_unregister(&g_leds_data[i]->cdev);
			cancel_work_sync(&g_leds_data[i]->work);
			kfree(g_leds_data[i]);
			g_leds_data[i] = NULL;
		}
	}

err_isink:
	if (i) {
		for (i = i-1; i >=0; i--) {
			if (!g_isink_data[i])
				continue;
			kfree(g_isink_data[i]);
			g_isink_data[i] = NULL;
		}
	}        
	return ret;
}

static int mt65xx_leds_remove(struct platform_device *pdev)
{
	int i;
	for (i = 0; i < MT65XX_LED_TYPE_TOTAL; i++) {
		if (!g_leds_data[i])
			continue;
		led_classdev_unregister(&g_leds_data[i]->cdev);
		cancel_work_sync(&g_leds_data[i]->work);
		kfree(g_leds_data[i]);
		g_leds_data[i] = NULL;
	}

	for (i = 0; i < ISINK_NUM; i++) {
		if (!g_isink_data[i])
			continue;
		kfree(g_isink_data[i]);
		g_isink_data[i] = NULL;
	}
	return 0;
}

/*
static int mt65xx_leds_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}
*/
#if defined CONFIG_ARCH_MT6589
static void mt65xx_leds_shutdown(struct platform_device *pdev)
{
	int i;
    struct nled_setting led_tmp_setting = {NLED_OFF,0,0};
    
    LEDS_DEBUG("[LED]%s\n", __func__);
    printk("[LED]mt65xx_leds_shutdown: turn off backlight\n");
    
	for (i = 0; i < MT65XX_LED_TYPE_TOTAL; i++) {
		if (!g_leds_data[i])
			continue;
		switch (g_leds_data[i]->cust.mode) {
			
		    case MT65XX_LED_MODE_PWM:
			    if(strcmp(g_leds_data[i]->cust.name,"lcd-backlight") == 0)
			    {
					//mt_set_pwm_disable(g_leds_data[i]->cust.data);
					//mt_pwm_power_off (g_leds_data[i]->cust.data);
					mt_pwm_disable(g_leds_data[i]->cust.data, g_leds_data[i]->cust.config_data.pmic_pad);
			    }else
			    {
				    led_set_pwm(g_leds_data[i]->cust.data,&led_tmp_setting);
			    }
                break;
                
		   // case MT65XX_LED_MODE_GPIO:
			//    brightness_set_gpio(g_leds_data[i]->cust.data, 0);
            //    break;
                
		    case MT65XX_LED_MODE_PMIC:
			    brightness_set_pmic(g_leds_data[i]->cust.data, 0, 0);
                break;
		    case MT65XX_LED_MODE_CUST_LCM:
				printk("[LED]backlight control through LCM!!1\n");
			    ((cust_brightness_set)(g_leds_data[i]->cust.data))(0, bl_div);
                break;
            case MT65XX_LED_MODE_CUST_BLS_PWM:
				printk("[LED]backlight control through BLS!!1\n");
			    ((cust_set_brightness)(g_leds_data[i]->cust.data))(0);
                break;    
		    case MT65XX_LED_MODE_NONE:
		    default:
			    break;
          }
	}

}

#elif defined CONFIG_ARCH_MT6582
static void mt65xx_leds_shutdown(struct platform_device *pdev)
{
	

}

#elif defined CONFIG_ARCH_MT6572 || defined CONFIG_ARCH_MT6571
static void mt65xx_leds_shutdown(struct platform_device *pdev)
{
	int i;
    struct nled_setting led_tmp_setting = {NLED_OFF, 0, 0};
    
    LEDS_DEBUG("[LED]%s\n", __func__);
    printk("[LED]mt65xx_leds_shutdown: turn off backlight\n");
    
	for (i = 0; i < MT65XX_LED_TYPE_TOTAL; i++) {
		if (!g_leds_data[i])
			continue;
		switch (g_leds_data[i]->cust.mode) {
			
		    case MT65XX_LED_MODE_PWM:
			    if(strcmp(g_leds_data[i]->cust.name, "lcd-backlight") == 0)
			    {
					mt_pwm_disable(g_leds_data[i]->cust.data, g_leds_data[i]->cust.config_data.pmic_pad);
			    }else
			    {
				    led_set_pwm(g_leds_data[i]->cust.data, &led_tmp_setting);
			    }
                break;
                
            // case MT65XX_LED_MODE_GPIO:
            //    brightness_set_gpio(g_leds_data[i]->cust.data, 0);
            //    break;
                
		    case MT65XX_LED_MODE_PMIC:
			    brightness_set_pmic(g_leds_data[i]->cust.data, 0, 0);
                break;
		    case MT65XX_LED_MODE_CUST_LCM:
				printk("[LED]backlight control through LCM!!1\n");
			    ((cust_brightness_set)(g_leds_data[i]->cust.data))(0, bl_div);
                break;
            case MT65XX_LED_MODE_CUST_BLS_PWM:
				printk("[LED]backlight control through BLS!!1\n");
			    ((cust_set_brightness)(g_leds_data[i]->cust.data))(0);
                break;    
		    case MT65XX_LED_MODE_NONE:
                break;
		    default:
			    break;
          }
	}  
}

#elif defined (CONFIG_ARCH_MT6575) || defined (CONFIG_ARCH_MT6575T)|| defined (CONFIG_ARCH_MT6577)
static void mt65xx_leds_shutdown(struct platform_device *pdev)
{
	int i;
    struct nled_setting led_tmp_setting = {NLED_OFF,0,0};
    
    LEDS_DEBUG("[LED]%s\n", __func__);
    printk("[LED]mt65xx_leds_shutdown: turn off backlight\n");
    
	for (i = 0; i < MT65XX_LED_TYPE_TOTAL; i++) {
		if (!g_leds_data[i])
			continue;
		switch (g_leds_data[i]->cust.mode) {
		    case MT65XX_LED_MODE_PWM:
			    if(strcmp(g_leds_data[i]->cust.name,"lcd-backlight") == 0)
			    {
					mt_set_pwm_disable(g_leds_data[i]->cust.data);
					mt_pwm_power_off (g_leds_data[i]->cust.data);
			    }else
			    {
				    led_set_pwm(g_leds_data[i]->cust.data,&led_tmp_setting);
			    }
                break;
                
		    case MT65XX_LED_MODE_GPIO:
			    brightness_set_gpio(g_leds_data[i]->cust.data, 0);
                break;
                
		    case MT65XX_LED_MODE_PMIC:
			    brightness_set_pmic(g_leds_data[i]->cust.data, 0, 0);
                break;
                
		    case MT65XX_LED_MODE_CUST:
			    ((cust_brightness_set)(g_leds_data[i]->cust.data))(0, bl_div);
                break;
                
		    case MT65XX_LED_MODE_NONE:
		    default:
			    break;
          }
	}

}
#endif

static struct platform_driver mt65xx_leds_driver = {
	.driver		= {
		.name	= "leds-mt65xx",
		.owner	= THIS_MODULE,
	},
	.probe		= mt65xx_leds_probe,
	.remove		= mt65xx_leds_remove,
	//.suspend	= mt65xx_leds_suspend,
	.shutdown   = mt65xx_leds_shutdown,
};

#if 0
static struct platform_device mt65xx_leds_device = {
	.name = "leds-mt65xx",
	.id = -1
};

#endif

static int __init mt65xx_leds_init(void)
{
	int ret;

	LEDS_DEBUG("[LED]%s\n", __func__);

#if 0
	ret = platform_device_register(&mt65xx_leds_device);
	if (ret)
		printk("[LED]mt65xx_leds_init:dev:E%d\n", ret);
#endif
	ret = platform_driver_register(&mt65xx_leds_driver);

	if (ret)
	{
		printk("[LED]mt65xx_leds_init:drv:E%d\n", ret);
//		platform_device_unregister(&mt65xx_leds_device);
		return ret;
	}

	wake_lock_init(&leds_suspend_lock, WAKE_LOCK_SUSPEND, "leds wakelock");

	return ret;
}

static void __exit mt65xx_leds_exit(void)
{
	platform_driver_unregister(&mt65xx_leds_driver);
//	platform_device_unregister(&mt65xx_leds_device);
}

module_param(debug_enable, int,0644);

module_init(mt65xx_leds_init);
module_exit(mt65xx_leds_exit);

MODULE_AUTHOR("MediaTek Inc.");
MODULE_DESCRIPTION("LED driver for MediaTek MT65xx chip");
MODULE_LICENSE("GPL");
MODULE_ALIAS("leds-mt65xx");

