#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>

static struct alsps_hw cust_alsps_hw = {
    .i2c_num    = 2,
	.polling_mode_ps =0,
	.polling_mode_als =1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    .i2c_addr   = {0x0C, 0x48, 0x78, 0x00},
    .als_level  = { 1,  2,  10,   40,  64,  100,  200, 300, 400,  500,  6000, 10000, 14000, 18000, 20000},
    .als_value  = {30, 30, 40,  280, 280, 280,  2500,  3500,  4500,  10240,  10240,  10240,  10240, 10240,  10240, 10240},
    /* MTK: modified to support AAL */
    //.als_level  = { 1,  2,  48, 200,  400, 1024,  65535,  65535,  65535, 65535, 65535, 65535, 65535, 65535, 65535},
    //.als_value  = { 30,  30,  280,  2500,  3500,  4500,  10243,  10243, 10243, 10243, 10243, 10243, 10243, 10243, 10243},
    //.ps_threshold = 2,	//3,
    .ps_threshold_high = 0x147, // 0x11c,
    .ps_threshold_low = 0x97, // 0x85,
    .als_threshold_high = 0xFFFF,
    .als_threshold_low = 0,
};
struct alsps_hw *get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}

