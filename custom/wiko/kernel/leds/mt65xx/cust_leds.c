#include <cust_leds.h>
#include <mach/mt_pwm.h>

#include <linux/kernel.h>
#include <mach/pmic_mt6329_hw_bank1.h> 
#include <mach/pmic_mt6329_sw_bank1.h> 
#include <mach/pmic_mt6329_hw.h>
#include <mach/pmic_mt6329_sw.h>
#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>
#include <mach/mt_gpio.h>

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
    
    mapped_level = level/4;
       
	return mapped_level;
}

unsigned int Cust_SetBacklight(int level, int div)
{
    //mtkfb_set_backlight_pwm(div);
    //mtkfb_set_backlight_level(brightness_mapping(level));
    disp_bls_set_backlight(brightness_mapping(level));
    return 0;
}

#if defined (TINNO_ANDROID_S7811A)
// Jake.L, DATE20130922, SSEO-452, DATE20130918-01 START
#define DISP_BACKLIGHT_GPIO GPIO22//GPIO134

static int Cust_SetLcdBacklight(int level)
{
    static current_level = 0;
    int ret = 0;
    
    printk("Cust_SetLcdBacklight: level=%d, current_level=%d\n", level, current_level);
    
    if ((0 == current_level) && (0 != level))
    {
        mt_set_gpio_mode(DISP_BACKLIGHT_GPIO, GPIO_MODE_02);
        mt_set_gpio_dir(DISP_BACKLIGHT_GPIO, GPIO_DIR_OUT);
        printk("Set to PWM_BL mode.\n");
    }
    
    ret = disp_bls_set_backlight(level);

    if (0 == level)
    {
        mt_set_gpio_mode(DISP_BACKLIGHT_GPIO, GPIO_MODE_00);
    	mt_set_gpio_dir(DISP_BACKLIGHT_GPIO, GPIO_DIR_OUT);
    	mt_set_gpio_out(DISP_BACKLIGHT_GPIO, GPIO_OUT_ZERO);
        printk("Turn of LCD backlight.\n");
    }
    current_level = level;

    return ret;
}
// Jake.L, DATE20130922-01 END
#endif  /* TINNO_ANDROID_S7811A */

static struct cust_mt65xx_led cust_led_list[MT65XX_LED_TYPE_TOTAL] = {
	{"red",               MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK1,{0}},
	{"green",             MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK0,{0}},
	{"blue",              MT65XX_LED_MODE_NONE, -1,{0}},
	{"jogball-backlight", MT65XX_LED_MODE_NONE, -1,{0}},
	{"keyboard-backlight",MT65XX_LED_MODE_NONE, -1,{0}},
	{"button-backlight",  MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK2,{0}},
	#if defined (TINNO_ANDROID_S7811A)
	{"lcd-backlight",     MT65XX_LED_MODE_CUST_BLS_PWM, (int)Cust_SetLcdBacklight,{0}},
	#else
	{"lcd-backlight",     MT65XX_LED_MODE_PWM, PWM2,{0}},
	#endif  /* TINNO_ANDROID_S7811A */
};

struct cust_mt65xx_led *get_cust_led_list(void)
{
	return cust_led_list;
}

