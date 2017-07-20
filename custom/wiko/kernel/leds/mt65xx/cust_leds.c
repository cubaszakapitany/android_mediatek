#include <cust_leds.h>
#include <mach/mt_pwm.h>

#include <linux/kernel.h>
#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>
#include <mach/mt_leds.h>

//extern int mtkfb_set_backlight_level(unsigned int level);
//extern int mtkfb_set_backlight_pwm(int div);
extern int disp_bls_set_backlight(unsigned int level);
/*
#define ERROR_BL_LEVEL 0xFFFFFFFF

unsigned int brightness_mapping(unsigned int level)
{  
	return ERROR_BL_LEVEL;
}
*/

unsigned int brightness_mapping(unsigned int level)
{
    unsigned int mapped_level;
    
    mapped_level = level;
       
	return mapped_level;
}

unsigned int Cust_SetBacklight(int level, int div)
{
    //mtkfb_set_backlight_pwm(div);
    //mtkfb_set_backlight_level(brightness_mapping(level));
    disp_bls_set_backlight(brightness_mapping(level));
    return 0;
}

/*
 * To explain How to set these para for cust_led_list[] of led/backlight
 * "name" para: led or backlight
 * "mode" para:which mode for led/backlight
 *	such as:
 *			MT65XX_LED_MODE_NONE,	
 *			MT65XX_LED_MODE_PWM,	
 *			MT65XX_LED_MODE_GPIO,	
 *			MT65XX_LED_MODE_PMIC,	
 *			MT65XX_LED_MODE_CUST_LCM,	
 *			MT65XX_LED_MODE_CUST_BLS_PWM
 *
 *"data" para: control methord for led/backlight
 *   such as:
 *			MT65XX_LED_PMIC_LCD_ISINK=0,	
 *			MT65XX_LED_PMIC_NLED_ISINK0,
 *			MT65XX_LED_PMIC_NLED_ISINK1,
 *			MT65XX_LED_PMIC_NLED_ISINK2,
 *			MT65XX_LED_PMIC_NLED_ISINK3
 * 
 *"PWM_config" para:PWM(AP side Or BLS module), by default setting{0,0,0,0,0} Or {0}
 *struct PWM_config {	
 *	int clock_source;
 *	int div; 
 *	int low_duration;
 *	int High_duration;
 *	BOOL pmic_pad;//AP side PWM pin in PMIC chip (only 89 needs confirm); 1:yes 0:no(default)
 *};
 *   for BLS PWM setting as follow:
 *1.	PWM config data
 *	clock_source: clock source frequency, can be 0/1/2/3
 *  	div: clock division, can be any value within 0~1023
 *	 low_duration: non-use
 *	High_duration: non-use
 *	pmic_pad: non-use
 *
*2.	PWM freq.= clock source / (div + 1) /1024
*Clock source: 
*	0: 26 MHz
*	1: 104 MHz
*	2: 124.8 MHz
*	3: 156 MHz
*Div: 0~1023
*
*By default, clock_source = 0 and div = 0 => PWM freq. = 26 KHz 
*/

static struct PMIC_CUST_ISINK cust_isink_config[ISINK_NUM] = {
        {/* ISINK0 */   ISINK_AS_BACKLIGHT,     ISINK_STEP_FOR_BACK_LIGHT,        MT65XX_LED_PMIC_LCD_ISINK},
        {/* ISINK1 */   ISINK_AS_BACKLIGHT,     ISINK_STEP_FOR_BACK_LIGHT,        MT65XX_LED_PMIC_LCD_ISINK},
        {/* ISINK2 */   ISINK_AS_BACKLIGHT,     ISINK_STEP_FOR_BACK_LIGHT,        MT65XX_LED_PMIC_LCD_ISINK},
        {/* ISINK3 */   ISINK_AS_INDICATOR,     ISINK_STEP_24MA,        MT65XX_LED_PMIC_NLED_ISINK3},
};

static struct cust_mt65xx_led cust_led_list[MT65XX_LED_TYPE_TOTAL] = {
	{"red",               MT65XX_LED_MODE_NONE, -1,{0,0,0,0,0}},
	{"green",             MT65XX_LED_MODE_NONE, -1,{0,0,0,0,0}},
	{"blue",              MT65XX_LED_MODE_NONE, -1,{0,0,0,0,0}},
	{"jogball-backlight", MT65XX_LED_MODE_NONE, -1,{0,0,0,0,0}},
	{"keyboard-backlight",MT65XX_LED_MODE_NONE, -1,{0,0,0,0,0}},
	{"button-backlight",  MT65XX_LED_MODE_NONE, -1,{0,0,0,0,0}},
	{"lcd-backlight",     MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_LCD_ISINK,{0,0,0,0,0}},
};

struct cust_mt65xx_led *get_cust_led_list(void)
{
	return cust_led_list;
}

struct PMIC_CUST_ISINK *get_cust_isink_config(void)
{
	return cust_isink_config;
}
