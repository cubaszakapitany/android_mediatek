//#include <platform/cust_leds.h>
#include <cust_leds.h>
#include <platform/mt_pwm.h>
#include <platform/mt_leds.h>

//extern int DISP_SetBacklight(int level);

extern int disp_bls_set_backlight(unsigned int level);
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

