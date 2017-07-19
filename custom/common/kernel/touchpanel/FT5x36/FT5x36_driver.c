 
#include "tpd.h"
#include <linux/interrupt.h>
#include <cust_eint.h>
#include <linux/i2c.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>

#include "tpd_custom_FT5x36.h"

#define TPD_POWER_SOURCE_CUSTOM	MT65XX_POWER_LDO_VIO28
#if defined(MT6572)
#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>
#include <mach/mt_gpio.h>
#elif defined(MT6577)
#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>
#elif defined(MT6575)
#include <mach/mt6575_pm_ldo.h>
#include <mach/mt6575_typedefs.h>
#include <mach/mt6575_boot.h>
#elif defined(CONFIG_ARCH_MT6573)
#include <mach/mt6573_boot.h>
#elif defined(MT6589)
#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>
#endif

#include "cust_gpio_usage.h"

#define MULTI_TOUCH_PROTOCOL_TYPE_A
//#define MULTI_TOUCH_PROTOCOL_TYPE_B
#ifdef MULTI_TOUCH_PROTOCOL_TYPE_B
    #include <linux/input/mt.h>
    #define FTS_SUPPORT_TRACK_ID
#endif

#define FTS_CTL_IIC

#ifdef FTS_CTL_IIC
#include "focaltech_ctl.h"
#endif

extern struct tpd_device *tpd;
 
static DECLARE_WAIT_QUEUE_HEAD(waiter);
static void tpd_eint_interrupt_handler(void);
extern void mt_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
extern void mt_eint_mask(unsigned int line);
extern void mt_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
extern unsigned int mt_eint_set_sens(unsigned int eint_num, unsigned int sens);
extern void mt65xx_eint_registration(unsigned int eint_num, unsigned int is_deb_en,
			  unsigned int pol, void (EINT_FUNC_PTR) (void),
			  unsigned int is_auto_umask);
static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_detect(struct i2c_client *client, struct i2c_board_info *info);
static int __devexit tpd_remove(struct i2c_client *client);
static int touch_event_handler(void *unused);
extern u8 *I2CDMABuf_va;
kal_bool upmu_is_chr_det(void);
extern volatile u32 I2CDMABuf_pa;
int g_bootVGP2 =0;
#define TPD_OK 0

#ifdef TPD_HAVE_BUTTON 

static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local_BYD[TPD_KEY_COUNT][4] = TPD_KEYS_DIM_BYD;
static int tpd_keys_dim_local_NB[TPD_KEY_COUNT][4] = TPD_KEYS_DIM_NB;
static void tinno_update_tp_button_dim(int panel_vendor)
{
	if ( FTS_CTP_VENDOR_NANBO == panel_vendor ){
		tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local_NB);
	}else{
		tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local_BYD);
	}
}
 
#endif

//static tinno_ts_data *g_pts = NULL;
static volatile	int tpd_flag = 0;

#define DRIVER_NAME "FT5x36"

static const struct i2c_device_id FT5x36_tpd_id[] = {{DRIVER_NAME,0},{}};

/* This is not use after Android 4.0
static const struct i2c_device_id tpd_id[] = {{TPD_DEVICE,0},{}};
unsigned short force[] = {TPD_I2C_GROUP_ID, TPD_I2C_SLAVE_ADDR2, I2C_CLIENT_END, I2C_CLIENT_END}; 
static const unsigned short * const forces[] = { force, NULL };
static struct i2c_client_address_data addr_data = { 
	.forces = forces, 
};
*/

#if defined (TINNO_ANDROID_S9091) || defined(TINNO_ANDROID_S9096)
	static struct i2c_board_info __initdata FT5x36_i2c_tpd[]={ 
					{I2C_BOARD_INFO(DRIVER_NAME, TPD_I2C_SLAVE_ADDR1)}
				};
#elif defined (TINNO_ANDROID_S8121) || defined(TINNO_ANDROID_S8122) || defined(TINNO_ANDROID_S7811A)
	static struct i2c_board_info __initdata FT5x36_i2c_tpd[]={ 
                                        // {I2C_BOARD_INFO(DRIVER_NAME, TPD_I2C_SLAVE_ADDR)},
					#ifdef  TINNO_ANDROID_S7810
					{I2C_BOARD_INFO(DRIVER_NAME,   TPD_I2C_SLAVE_ADDR1)},
					#endif
					{I2C_BOARD_INFO(DRIVER_NAME, TPD_I2C_SLAVE_ADDR2)}
				};
#else
  #error "Wrong platform config for FT5X16."
#endif

static struct i2c_driver tpd_i2c_driver = {
	.driver = {
		 .name = DRIVER_NAME,
		 //.owner = THIS_MODULE,
	},
	.probe = tpd_probe,
	.remove = __devexit_p(tpd_remove),
	.id_table = FT5x36_tpd_id,
	.detect = tpd_detect,
	//.address_data = &addr_data,
};
static int pre_charge_status=0;
int tp_charger_detect(int current_charge_status)
{

       int ret =-1;
	  //pre_charge_status = 
	printk("current_charge_status = %d pre_charge_status = %d \n",current_charge_status,pre_charge_status);
	if(current_charge_status&&(pre_charge_status==0))
	{
		ret =fts_write_reg(0x8b,1);
		printk("tp_charger_detect start ret =%d \n",ret);
	}
	else if((pre_charge_status==1)&&(current_charge_status==0))
	{
		ret =fts_write_reg(0x8b,0);
		printk("tp_charger_detect stop ret =%d \n",ret);
	}
	pre_charge_status=current_charge_status;

	printk("tp_charger_detect ret =%d \n",ret);
	return ret;
}

//extern int g_charger_in_flag;

static  void tpd_down(tinno_ts_data *ts, int x, int y, int pressure, int trackID) 
{
	int iPressure = pressure*pressure/112;
	if ( iPressure < 1 ){
		iPressure = 1;
	}
	CTP_DBG("x=%03d, y=%03d, pressure=%03d, ID=%03d", x, y, pressure, trackID);
#if defined(MULTI_TOUCH_PROTOCOL_TYPE_A)
	input_report_abs(tpd->dev, ABS_PRESSURE, iPressure);
	input_report_abs(tpd->dev, ABS_MT_PRESSURE, iPressure);
	input_report_key(tpd->dev, BTN_TOUCH, 1);
	if(1)
	{	
	input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	}
	else
	{
	 tpd_button(x, y, 1); 
	 CTP_DBG("tpd_buttonx=%03d, y=%03d, ID=%03d", x, y, trackID);
	}
	input_report_abs(tpd->dev, ABS_MT_WIDTH_MAJOR, iPressure);
	input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, iPressure);
	input_mt_sync(tpd->dev);
#elif defined(MULTI_TOUCH_PROTOCOL_TYPE_B)
	input_mt_slot(tpd->dev, trackID);
	input_mt_report_slot_state(tpd->dev, MT_TOOL_FINGER, 1);
	input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	input_report_abs(tpd->dev, ABS_MT_PRESSURE, iPressure);
	input_report_abs(tpd->dev, ABS_MT_WIDTH_MAJOR, iPressure);
	input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, iPressure);
	input_mt_report_pointer_emulation(tpd->dev, true);
#endif
	__set_bit(trackID, &ts->fingers_flag);
	ts->touch_point_pre[trackID].x=x;
	ts->touch_point_pre[trackID].y=y;
	if (FACTORY_BOOT == get_boot_mode() || RECOVERY_BOOT == get_boot_mode()) {   
		tpd_button(x, y, 1);  
	}	 
	TPD_DOWN_DEBUG_TRACK(x,y);

	tp_charger_detect(upmu_is_chr_det());
 }
 
static  int tpd_up(tinno_ts_data *ts, int x, int y, int pressure, int trackID) 
{
	CTP_DBG("x=%03d, y=%03d, ID=%03d", x, y, trackID);
#if defined(MULTI_TOUCH_PROTOCOL_TYPE_A)
	if (FACTORY_BOOT == get_boot_mode() || RECOVERY_BOOT == get_boot_mode()) {   
		CTP_DBG("x=%03d, y=%03d, ID=%03d", x, y, trackID);
		input_report_abs(tpd->dev, ABS_PRESSURE, 0);
		input_report_abs(tpd->dev, ABS_MT_PRESSURE, 0);
		input_report_key(tpd->dev, BTN_TOUCH, 0);
		if(1)
		{		
		input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
		input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
		}
		else
		{
			tpd_button(x, y, 0); 
			CTP_DBG("tpd_buttonx=%03d, y=%03d, ID=%03d", x, y, trackID);
		}
		input_report_abs(tpd->dev, ABS_MT_WIDTH_MAJOR, 0);
		input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);// This must be placed at the last one.
		input_mt_sync(tpd->dev);
	}else{
		/*
		* Android 4.0 and latter don't need to report these up events.
		* If the driver reports one of BTN_TOUCH or ABS_PRESSURE in addition to the ABS_MT 
		* events, the last SYN_MT_REPORT event may be omitted. Otherwise, the last 
		* SYN_REPORT will be dropped by the input core, resulting in nozero-contact event 
		* reaching userland.                                         --Jieve Liu
		*/
		int i, have_down_cnt = 0;
		for ( i=0; i < TINNO_TOUCH_TRACK_IDS; i++ ){
			if ( test_bit(i, &ts->fingers_flag) ){
				++have_down_cnt;
			}
		}
		if ( have_down_cnt < 2 ){
			input_mt_sync(tpd->dev);
		}
	}
#elif defined(MULTI_TOUCH_PROTOCOL_TYPE_B)
		input_mt_slot(tpd->dev, trackID);
		input_mt_report_slot_state(tpd->dev, MT_TOOL_FINGER, 0);
		input_mt_report_pointer_emulation(tpd->dev, true);
#endif

	__clear_bit(trackID, &ts->fingers_flag);
	TPD_UP_DEBUG_TRACK(x,y);
	if (FACTORY_BOOT == get_boot_mode() || RECOVERY_BOOT == get_boot_mode()) {   
		tpd_button(x, y, 0); 
	}   		 
	return 0;
 }

 static void tpd_dump_touchinfo(tinno_ts_data *ts)
 {
 	uint8_t *pdata = ts->buffer;
	CTP_DBG("0x%02x 0x%02x 0x%02x"
		"   0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x"
		"   0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x"
		"   0x%02x 0x%02x  0x%02x 0x%02x 0x%02x 0x%02x"
		"   0x%02x 0x%02x  0x%02x 0x%02x 0x%02x 0x%02x"
              , 
	      pdata[0],   pdata[1],  pdata[2],   
	      pdata[3],   pdata[4],  pdata[5],   pdata[6],  pdata[7], pdata[8],   
	      pdata[9],  pdata[10], pdata[11], pdata[12], pdata[13], pdata[15], 
	      pdata[15], pdata[16], pdata[17], pdata[18], pdata[19], pdata[20], 
	      pdata[21], pdata[22], pdata[23], pdata[24], pdata[25], pdata[26]); 
 }

 static void ft_map_coordinate(int *pX, int *pY)
 {
 	int x = *pX, y = *pY;
	*pX = x * 540 / 760;
	*pY = y * 960 / 1280;
 }
 
 static int tpd_touchinfo(tinno_ts_data *ts, tinno_ts_point *touch_point)
 {
	int i = 0;
	int iInvalidTrackIDs = 0;
	int iTouchID, iSearchDeep;
	fts_report_data_t *pReportData = (fts_report_data_t *)ts->buffer;

	if ( tpd_read_touchinfo(ts) ){
		CTP_DBG("Read touch information error. \n");
		return -EAGAIN; 
	}
	
//	tpd_dump_touchinfo( ts );
	
	if ( 0 != pReportData->device_mode ){
		CTP_DBG("device mode is %d\n", pReportData->device_mode);
		return -EPERM; 
	}
	
	//We need only valid points...
	if ( pReportData->fingers > TINNO_TOUCH_TRACK_IDS ){
		CTP_DBG("fingers is %d\n", pReportData->fingers);
		return -EAGAIN; 
	}

	// For processing gestures.
	if (pReportData->gesture >= 0xF0 && pReportData->gesture <= 0xF3) {
		//fts_5x36_parase_keys(ts, pReportData);
	}
	
	iSearchDeep = 0;
#ifdef FTS_SUPPORT_TRACK_ID
	for ( i = 0; i < TINNO_TOUCH_TRACK_IDS; i++ ){
		iSearchDeep += ((pReportData->xy_data[i].event_flag != FTS_EF_RESERVED)?1:0);
	}
#else
	if (pReportData->fingers >= ts->last_fingers ){
		iSearchDeep = pReportData->fingers;
	}else{
		iSearchDeep = ts->last_fingers;
	}
	ts->last_fingers = pReportData->fingers;
#endif

	if ( iSearchDeep ) {
#ifdef FTS_SUPPORT_TRACK_ID
		for ( i=0; i < TINNO_TOUCH_TRACK_IDS; i++ ){
#else
		for ( i=0; i < iSearchDeep; i++ ){
#endif
			if (pReportData->xy_data[i].event_flag != FTS_EF_RESERVED) {
#ifdef FTS_SUPPORT_TRACK_ID
				iTouchID = pReportData->xy_data[i].touch_id;
				if ( iTouchID >= TINNO_TOUCH_TRACK_IDS )
				{
					CTP_DBG("i: Invalied Track ID(%d)\n!", i, iTouchID);
					iInvalidTrackIDs++;
					continue;
				}
#else
				iTouchID = i;
#endif
				touch_point[iTouchID].flag = pReportData->xy_data[i].event_flag;
				touch_point[iTouchID].x = pReportData->xy_data[i].x_h << 8 | pReportData->xy_data[i].x_l;
				touch_point[iTouchID].y = pReportData->xy_data[i].y_h << 8 | pReportData->xy_data[i].y_l;
				touch_point[iTouchID].pressure = pReportData->xy_data[i].pressure;
#ifdef TPD_FIRST_FIRWARE
				ft_map_coordinate(&(touch_point[iTouchID].x), &(touch_point[iTouchID].y));
#endif
			}else{
				//CTP_DBG("We got a invalied point, we take it the same as a up event!");
				//CTP_DBG("As it has no valid track ID, we assume it's order is the same as it's layout in the memory!");
				//touch_point[i].flag = FTS_EF_RESERVED;
			}
		}
		if ( TINNO_TOUCH_TRACK_IDS == iInvalidTrackIDs ){
			CTP_DBG("All points are Invalied, Ignore the interrupt!\n");
			return -EAGAIN; 
		}
	}
/*	
	CTP_DBG("p0_flag=0x%x x0=0x%03x y0=0x%03x pressure0=0x%03x "
	              "p1_flag=0x%x x1=0x%03x y1=0x%03x pressure1=0x%03x "
	              "gesture = 0x%x fingers=0x%x", 
	       touch_point[0].flag, touch_point[0].x, touch_point[0].y, touch_point[0].pressure,
	       touch_point[1].flag, touch_point[1].x, touch_point[1].y, touch_point[1].pressure,
	       pReportData->gesture, pReportData->fingers); 
*/	  
	 return 0;

 }

 static int touch_event_handler(void *para)
 {
 	int i;
	tinno_ts_point touch_point[TINNO_TOUCH_TRACK_IDS];
	struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };
	tinno_ts_data *ts = (tinno_ts_data *)para;
	sched_setscheduler(current, SCHED_RR, &param);
	
	do {
		set_current_state(TASK_INTERRUPTIBLE); 
		wait_event_interruptible(waiter, tpd_flag!=0);
		tpd_flag = 0;
		memset(touch_point, FTS_INVALID_DATA, sizeof(touch_point));
		set_current_state(TASK_RUNNING);
		if (!tpd_touchinfo(ts, &touch_point)) {
			//report muti point then
			for ( i=0; i < TINNO_TOUCH_TRACK_IDS; i++ ){
				if ( FTS_INVALID_DATA != touch_point[i].x ){
					if ( FTS_EF_UP == touch_point[i].flag ){
						if( test_bit(i, &ts->fingers_flag) ){
							tpd_up(ts, ts->touch_point_pre[i].x, ts->touch_point_pre[i].y, 
								touch_point[i].pressure, i);
						}else{
							CTP_DBG("This is a invalid up event.(%d)", i);
						}
					}else{//FTS_EF_CONTACT or FTS_EF_DOWN
						if ( test_bit(i, &ts->fingers_flag) 
							&& (FTS_EF_DOWN == touch_point[i].flag) ){
							CTP_DBG("Ignore a invalid down event.(%d)", i);
							continue;
						}
						tpd_down(ts, touch_point[i].x, touch_point[i].y, 
							touch_point[i].pressure, i);
					}
				}else if (  test_bit(i, &ts->fingers_flag) ){
					CTP_DBG("Complete a invalid down or move event.(%d)", i);
					tpd_up(ts, ts->touch_point_pre[i].x, ts->touch_point_pre[i].y, 
						touch_point[i].pressure, i);
				}
			}
			input_sync(tpd->dev);
		}
		
		mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM); 
	}while(!kthread_should_stop());
	mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM); 
	return 0;
 }
 
static int tpd_detect (struct i2c_client *client, struct i2c_board_info *info) 
{
	strcpy(info->type, TPD_DEVICE);	
	return 0;
}
 
static void tpd_eint_interrupt_handler(void)
{
	if ( 0 == tpd_load_status ){
		return;
	}
	tpd_flag = 1;
	wake_up_interruptible(&waiter);
}

void fts_5x36_hw_reset(void)
{
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	mdelay(10);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
	mdelay(10);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	mdelay(60);
}
static bool fts_5x36_Is_Power28V(void)
{
	CTP_DBG("======fts_5x36_Is_Power28V=======start\n");
	int iRetry = 5;
	int ret = 0;
	tinno_ts_data *ts28v;
	int panel_version = 0;
	int panel_vendor = 0;

		ts28v = kzalloc(sizeof(*ts28v), GFP_KERNEL);
	if (ts28v == NULL) {
		ret = -ENOMEM;
		return  -1;
	}
	memset(ts28v, 0, sizeof(*ts28v));
	CTP_DBG("======FT5x36_get_fw_version=======start\n");	
	while (iRetry) 
	{
	    CTP_DBG("==========fts_5x36_Is_Power28V===========while");
		ret = FT5x36_get_vendor_version(g_pts, &panel_vendor, &panel_version);
		if ( panel_version < 0 || panel_vendor < 0 || ret < 0 )
		{
			CTP_DBG("Product version is %d\n", panel_version);
			fts_5x36_hw_reset();
		}
		else
		{
			CTP_DBG("==========fts_5x36_Is_Power28V===========0");
			return 1;
		}
		iRetry--;
		msleep(15);  
	} 
	if ( panel_version < 0 || panel_vendor < 0 || ret < 0 )
	{
		CTP_DBG("==========fts_5x36_Is_Power28V===========0");
		g_bootVGP2 = 1;
		return 0;
	}
}
static void fts_5x36_hw_init2(void)
{
	int ret = 0;
	 hwPowerDown(TPD_POWER_SOURCE_CUSTOM, "touch");
	//make sure the WakeUp is high before  it enter power on mode, 
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
	msleep(10);
	
	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_EINT_PIN, GPIO_OUT_ONE);	
	msleep(100);

	//Power On CTP
#if defined(MT6589)
    #if defined(TINNO_ANDROID_S9096)
	hwPowerOn(MT65XX_POWER_LDO_VGP4, VOL_2800, "touch"); 
    #elif defined (TINNO_ANDROID_S8122)
	hwPowerOn(MT65XX_POWER_LDO_VIO28, VOL_2800, "touch"); 
    #endif
#elif defined(MT6577)
	hwPowerOn(MT65XX_POWER_LDO_VGP, VOL_2800, "touch"); 
#elif defined (MT6572)
hwPowerOn(MT6323_POWER_LDO_VGP2, VOL_2800, "touch"); 
#else
	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_EINT_PIN, GPIO_OUT_ONE);
#endif
	msleep(10); 
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(200);
}
static void fts_5x36_hw_init(void)
{
	int ret = 0;
	CTP_DBG("=============static void fts_5x36_hw_init(void)====start=======\n");
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
	msleep(10);	
	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_EINT_PIN, GPIO_OUT_ONE);	
	msleep(100);
#if defined(MT6589)
    #if defined(TINNO_ANDROID_S9096)
	hwPowerOn(MT65XX_POWER_LDO_VGP4, VOL_2800, "touch"); 
    #elif defined (TINNO_ANDROID_S8122)
	hwPowerOn(MT65XX_POWER_LDO_VIO28, VOL_2800, "touch"); 
    #endif
#elif defined(MT6577)
	hwPowerOn(MT65XX_POWER_LDO_VGP, VOL_2800, "touch"); 
#elif defined (MT6572)
	hwPowerOn(TPD_POWER_SOURCE_CUSTOM, VOL_2800, "touch"); 
	hwPowerOn(MT6323_POWER_LDO_VGP2, VOL_2800, "touch"); 
#else
	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_EINT_PIN, GPIO_OUT_ONE);
#endif
	msleep(10); 
	//Reset CTP
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(200);
}

char *fts_get_vendor_name(int vendor_id)
{
	switch(vendor_id){
		case FTS_CTP_VENDOR_BYD:		return "BYD";			 break;
		case FTS_CTP_VENDOR_TRULY:		return "TRULY";		     break;
		case FTS_CTP_VENDOR_NANBO:		return "NANBO";		     break;
		case FTS_CTP_VENDOR_BAOMING:	return "BAOMING";	     break;
		case FTS_CTP_VENDOR_JIEMIAN:	return "JIEMIAN";		 break;
		case FTS_CTP_VENDOR_YEJI:		return "YEJI";			 break;
		case FTS_CTP_VENDOR_LianChuang:		return "lianchuang"; break;		
		default:							return "UNKNOWN";	 break;
	}
	return "UNKNOWN";
}
 EXPORT_SYMBOL(fts_get_vendor_name);

 int fts_ctpm_get_factory_id(void)
 { 
 	CTP_DBG("[FTS]========= fts_ctpm_get_factory_id============\n");
		  unsigned char uc_i2c_addr;			 //I2C slave address (8 bit address)
		  unsigned char uc_io_voltage;			 //IO Voltage 0---3.3v;	 1 ----1.8v
		  unsigned char uc_panel_factory_id;	 //TP panel factory ID
 		  int i =0;
		  unsigned char buf[6];
		  FTS_BYTE reg_val[2] = {0};
		  FTS_BYTE	auc_i2c_write_buf[10];
		  //FTS_BYTE  packet_buf[FTS_SETTING_BUF_LEN + 6];
		  //FTS_DWRD i = 0;
		  int	   i_ret;
 
		  uc_i2c_addr = TPD_I2C_SLAVE_ADDR2;	//CTP_I2C_SLAVEADDR = 0x70;
		  uc_io_voltage = 0x0;
		  uc_panel_factory_id = 0x5a;
 
		  /*********Step 1:Reset  CTPM *****/
		  /*write 0xaa to register 0xfc*/
		  fts_write_reg(0xfc,0xaa);
		  mdelay(50);
		   /*write 0x55 to register 0xfc*/
		  fts_write_reg(0xfc,0x55);
		  CTP_DBG("[FTS] Step 1: Reset CTPM test\n");
 
		  mdelay(30);	 
 
		  /*********Step 2:Enter upgrade mode *****/
		  auc_i2c_write_buf[0] = 0x55;
		  auc_i2c_write_buf[1] = 0xaa;
		  
		  do
		  {
				   i ++;
				   //i_ret = ft5x0x_i2c_txdata(auc_i2c_write_buf, 2);
				   i_ret = fts_write_reg(auc_i2c_write_buf, 2);
				   mdelay(5);
		  }while(i_ret <= 0 && i < 5 );
 
		  /*********Step 3:check READ-ID***********************/
		  auc_i2c_write_buf[0] = 0x90;
		  auc_i2c_write_buf[1] = auc_i2c_write_buf[2] = auc_i2c_write_buf[3] = 
 0x00;
		  fts_i2c_Read(g_pts->client,auc_i2c_write_buf, 4, reg_val, 2);
		  if (reg_val[0] == 0x79 && reg_val[1] == 0x11)
		  {
				   CTP_DBG("[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",
 reg_val[0],reg_val[1]);
		  }
		  else
		  {
				   return ERR_READID;
		  }
		  auc_i2c_write_buf[0] = 0xcd;
		  fts_i2c_Read(g_pts->client,auc_i2c_write_buf, 1, reg_val, 1);
		  CTP_DBG("bootloader version = 0x%x\n", reg_val[0]);
 
 
		  /* --------- read current project setting  ---------- */
		  //set read start address
		  buf[0] = 0x3;
		  buf[1] = 0x0;
		  buf[2] = 0x78;
		  buf[3] = 0x0;
		  //ft5x0x_i2c_Write(buf, 4);
		  //byte_read(buf, FTS_SETTING_BUF_LEN);
		   fts_i2c_Read(g_pts->client,buf, 4, buf, 6);
 
		  CTP_DBG("[FTS] old setting: uc_i2c_addr = 0x%x, uc_io_voltage = %d,uc_panel_factory_id = 0x%x\n",buf[0],  buf[2], buf[4]);
 
 
		  /********* reset the new FW***********************/
		  auc_i2c_write_buf[0] = 0x07;
		  fts_i2c_Write(g_pts->client,auc_i2c_write_buf, 1);
		  msleep(200);		  
 		  return buf[4];
	 
 }

 static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id)
 {	 
	int retval = TPD_OK;
	int panel_version = 0;
	int panel_vendor = 0;
	int iRetry = 3;
	tinno_ts_data *ts;
	int ret = 0;
	
	if ( tpd_load_status ){
		CTP_DBG("Already probed a TP, needn't to probe any more!");
		return -1;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev,"need I2C_FUNC_I2C");
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}

	ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (ts == NULL) {
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}
	
	CTP_DBG("TPD enter tpd_probe ts=0x%p, TPD_RES_X=%d, TPD_RES_Y=%d, addr=0x%x\n", ts, TPD_RES_X, TPD_RES_Y, client->addr);
	memset(ts, 0, sizeof(*ts));
	g_pts = ts;

	client->timing = I2C_MASTER_CLOCK;
	ts->client = client;
	ts->start_reg = 0x00;
	atomic_set( &ts->ts_sleepState, 0 );
	mutex_init(&ts->mutex);

	i2c_set_clientdata(client, ts);
    CTP_DBG("=================fts_ctpm_get_factory_id===========start");
	//fts_ctpm_get_factory_id();
	CTP_DBG("=================fts_ctpm_get_factory_id===========end");
	fts_iic_init(ts);
	
	{
		 I2CDMABuf_va = (u8 *)dma_alloc_coherent(NULL, FTS_DMA_BUF_SIZE, &I2CDMABuf_pa, GFP_KERNEL);

		if(!I2CDMABuf_va)
		{
			dev_err(&client->dev,"%s Allocate DMA I2C Buffer failed!\n",__func__);
			return -EIO;
		}
		CTP_DBG("FTP: I2CDMABuf_pa=%x,val=%x val2=%x\n",&I2CDMABuf_pa,I2CDMABuf_pa,(unsigned char *)I2CDMABuf_pa);

	}
#ifdef MULTI_TOUCH_PROTOCOL_TYPE_B
	input_mt_init_slots(tpd->dev, TINNO_TOUCH_TRACK_IDS);
	set_bit(BTN_TOOL_FINGER, tpd->dev->keybit);
	set_bit(BTN_TOOL_DOUBLETAP, tpd->dev->keybit);
	set_bit(BTN_TOOL_TRIPLETAP, tpd->dev->keybit);
	set_bit(BTN_TOOL_QUADTAP, tpd->dev->keybit);
	set_bit(BTN_TOOL_QUINTTAP, tpd->dev->keybit);
	input_set_abs_params(tpd->dev, ABS_MT_TOOL_TYPE,0, MT_TOOL_MAX, 0, 0);
#endif
	if ( fts_5x36_isp_init(ts) ){
		goto err_isp_register;
	}
	fts_5x36_hw_init();
	msleep(120);

	while (iRetry) {
		ret = FT5x36_get_vendor_version(ts, &panel_vendor, &panel_version);
		if ( panel_version < 0 || panel_vendor<0 || ret<0 ){
			CTP_DBG("Product version is %d\n", panel_version);
			fts_5x36_hw_reset();
		}else{
			break;
		}
		iRetry--;
		msleep(15);  
	} 
	if ( panel_version < 0 || panel_vendor<0 || ret<0 ){
#ifdef  TINNO_ANDROID_S7810
		goto err_get_version;
#endif
	}
#ifdef TPD_HAVE_BUTTON 
	tinno_update_tp_button_dim(panel_vendor);
#endif
#ifdef CONFIG_TOUCHSCREEN_FT5X05_DISABLE_KEY_WHEN_SLIDE
	if ( fts_keys_init(ts) ){
		fts_keys_deinit();
		CTP_DBG("===========fts_keys_init===============");
		goto err_get_version;
	}
#endif
	
	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
	mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
	mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);
 
 	mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE);
	mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
	mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_TOUCH_PANEL_POLARITY, tpd_eint_interrupt_handler, 0); 
 
	ts->thread = kthread_run(touch_event_handler, ts, TPD_DEVICE);
	 if (IS_ERR(ts->thread)){ 
		  retval = PTR_ERR(ts->thread);
		  CTP_DBG(TPD_DEVICE " failed to create kernel thread: %d\n", retval);
			goto err_start_touch_kthread;
	}

	tpd_load_status = 1;
	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM); 
	
#ifdef FTS_CTL_IIC
	if (ft_rw_iic_drv_init(client) < 0)
		dev_err(&client->dev, "%s:[FTS] create fts control iic driver failed\n",
				__func__);
#endif
	
	CTP_DBG("Touch Panel Device(%s) Probe PASS\n", fts_get_vendor_name(panel_vendor));

#if 1
{
	extern char tpd_desc[50];
	extern int tpd_fw_version;
  #if defined(TINNO_ANDROID_S9096)
	sprintf(tpd_desc, "%s-S9096-%s-%X", fts_get_vendor_name(panel_vendor),
				"FT5316", panel_version);
  #elif defined (TINNO_ANDROID_T8122)
	sprintf(tpd_desc, "%s-T8122-%s-%X", fts_get_vendor_name(panel_vendor),
				"FT5316", panel_version);
  #elif defined (TINNO_ANDROID_S8122)
	sprintf(tpd_desc, "%s-S8122-%s-%X", fts_get_vendor_name(panel_vendor),
				"FT5316", panel_version);
 /*
 #elif defined (TINNO_ANDROID_S7811A)
    sprintf(tpd_desc, "%s-S7811AE-%s-%X", fts_get_vendor_name(panel_vendor),
				"FT5436", panel_version);
*/
  #endif
	tpd_fw_version = panel_version;
}
#endif  /* #if 0 */

	return 0;
   
err_start_touch_kthread:
	mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM); 
err_get_version:
err_isp_register:
#ifdef CONFIG_TOUCHSCREEN_POWER_DOWN_WHEN_SLEEP
  #if defined(MT6589)
    #if defined(TINNO_ANDROID_S9096)
	hwPowerDown(MT65XX_POWER_LDO_VGP4, "touch"); 
    #elif defined (TINNO_ANDROID_S8122)
	hwPowerDown(MT65XX_POWER_LDO_VGP5, "touch"); 
    #endif
  #elif defined(MT6577)
	hwPowerDown(MT65XX_POWER_LDO_VGP, "touch"); 
  #elif defined(MT6572)
    hwPowerDown(TPD_POWER_SOURCE_CUSTOM, "touch");
 	hwPowerDown(MT6323_POWER_LDO_VGP2, "touch"); 
  #endif  
#endif	
	fts_5x36_isp_exit();
	mutex_destroy(&ts->mutex);
	g_pts = NULL;
	kfree(ts);
err_alloc_data_failed:
err_check_functionality_failed:
	CTP_DBG("Touch Panel Device Probe FAIL\n");
	return -1;
 }

 static int __devexit tpd_remove(struct i2c_client *client)
{
	CTP_DBG("TPD removed\n");
#ifdef FTS_CTL_IIC
	ft_rw_iic_drv_exit();
#endif
	return 0;
}
 
 static int tpd_local_init(void)
{
	TPD_DMESG("Focaltech FT5x36 I2C Touchscreen Driver (Built %s @ %s)\n", __DATE__, __TIME__);
	if(i2c_add_driver(&tpd_i2c_driver)!=0)
	{
		TPD_DMESG("unable to add i2c driver.\n");
		return -1;
	}
#ifdef TPD_HAVE_BUTTON     
		tinno_update_tp_button_dim(FTS_CTP_VENDOR_NANBO);
#endif   
	TPD_DMESG("end %s, %d\n", __FUNCTION__, __LINE__);  
	tpd_type_cap = 1;
	return 0; 
}

static void  fts_5x36_hw_resume_reset()
{

#ifdef CONFIG_TOUCHSCREEN_POWER_DOWN_WHEN_SLEEP
		fts_5x36_hw_init();
        mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
        mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
        mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
        mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);

#else //!CONFIG_TOUCHSCREEN_POWER_DOWN_WHEN_SLEEP
		mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_CTP_EINT_PIN, GPIO_OUT_ZERO);
		msleep(1);  
		mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_CTP_EINT_PIN, GPIO_OUT_ONE);
#endif//CONFIG_TOUCHSCREEN_POWER_DOWN_WHEN_SLEEP
#ifndef CONFIG_SMP
		mutex_unlock(&g_pts->mutex);//Lock on suspend
#endif
		mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);  

		atomic_set( &g_pts->ts_sleepState, 0 );
}
static void tpd_resume(struct early_suspend *h)
{   
	int iRetry = 5;
	int ret =0;
	int panel_vendor,panel_version;
	panel_vendor=panel_version = 0;
	pre_charge_status = 0;
	//printk("===========pre_charge_status==============");
	if ( g_pts )
		{
		CTP_DBG("TPD wake up\n");
		if (atomic_read(&g_pts->isp_opened))
	   {
			CTP_DBG("isp is already opened.");
			return;
	   }
		fts_5x36_hw_resume_reset();
	while (iRetry) 
	{
		ret = FT5x36_get_vendor_version(g_pts, &panel_vendor, &panel_version);
		if ( panel_version < 0 || panel_vendor<0 || ret<0 )
		{
			TPD_DMESG("Product version is %d\n", panel_version);
			fts_5x36_hw_resume_reset();
		}
		else
		{
			break;
		}
		iRetry--;
		msleep(15); 
	}
	}
 }
 
//Clear the unfinished touch event, simulate a up event if there this a pen down or move event.
void FT5x36_complete_unfinished_event( void )
{
	int i = 0;
	for ( i=0; i < TINNO_TOUCH_TRACK_IDS; i++ ){
		if (  test_bit(i, &g_pts->fingers_flag) ){
			tpd_up(g_pts, g_pts->touch_point_pre[i].x, g_pts->touch_point_pre[i].y, 
				g_pts->touch_point_pre[i].pressure, i);
		}
	}
	input_sync(tpd->dev);
}

static void tpd_suspend(struct early_suspend *h)
 {
	int ret = 0;
	int iRetry = 5;
	const char data = 0x3;
 /* {
     	unsigned char uc_temp[1];
		fts_read_reg(0x80,uc_temp);
		CTP_DBG("tpd_suspend%x",uc_temp[0]);
		msleep(1);
		fts_write_reg(0x80,0x20);
		msleep(1);
		fts_read_reg(0x80,uc_temp);
		CTP_DBG("tpd_suspend%x",uc_temp[0]);
       //	 if ( ((uc_temp[0]&0x70)>>4) == 0x0)  //return to normal mode,calibration finish
      //  {
     //       break;
     //   }
	}*/
	
	if ( g_pts )
		{
		 CTP_DBG("TPD enter sleep\n");
		if (atomic_read(&g_pts->isp_opened))
		{
			CTP_DBG("isp is already opened.");
			return;
		}
		 mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
#ifndef CONFIG_SMP
		mutex_lock(&g_pts->mutex);//Unlock on resume
#endif		 
#ifdef CONFIG_TOUCHSCREEN_FT5X05_DISABLE_KEY_WHEN_SLIDE
		fts_5x36_key_cancel();
#endif
ret = fts_write_reg(0xa5,0x03);//i2c_smbus_write_i2c_block_data(g_pts->client, 0xA5, 1, &data);  //TP enter sleep mode
TPD_DMESG("Enter sleep i2c_smbus_write_i2c_block_data mode is %d\n", ret);


#ifdef CONFIG_TOUCHSCREEN_POWER_DOWN_WHEN_SLEEP	
	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_EINT_PIN, GPIO_OUT_ZERO);
	msleep(10);
  #if defined(MT6589)
    #if defined(TINNO_ANDROID_S9096)
	hwPowerDown(MT65XX_POWER_LDO_VGP4, "touch"); 
    #elif defined (TINNO_ANDROID_S8122)
	hwPowerDown(MT65XX_POWER_LDO_VGP5, "touch"); 
    #endif
  #elif defined(MT6577)
		hwPowerDown(MT65XX_POWER_LDO_VGP, "touch"); 
  #elif defined(MT6572)
  hwPowerDown(MT6323_POWER_LDO_VGP2, "touch"); 
  hwPowerDown(TPD_POWER_SOURCE_CUSTOM, "touch");
	//hwPowerOn(TPD_POWER_SOURCE_CUSTOM, VOL_2800, "touch");
	//ret = fts_write_reg(0x80,0x20);//i2c_smbus_write_i2c_block_data(g_pts->client, 0xA5, 1, &data);  //TP enter sleep mode
	TPD_DMESG("Enter sleep i2c_smbus_write_i2c_block_data mode is %d\n", ret);
  #else
		mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_CTP_EINT_PIN, GPIO_OUT_ZERO);
  #endif	
#else //!CONFIG_TOUCHSCREEN_POWER_DOWN_WHEN_SLEEP
		//make sure the WakeUp is high before it enter sleep mode, 
		//otherwise the touch can't be resumed.
		//mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_GPIO);
		//mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_OUT);
		//mt_set_gpio_out(GPIO_CTP_EINT_PIN, GPIO_OUT_ONE);
		//msleep(1);  

		while (iRetry) {
			ret = i2c_smbus_write_i2c_block_data(g_pts->client, 0xA5, 1, &data);  //TP enter sleep mode
			if ( ret < 0 ){
				TPD_DMESG("Enter sleep mode is %d\n", ret);
  #if defined(MT6589)
    #if defined(TINNO_ANDROID_S9096)
				hwPowerDown(MT65XX_POWER_LDO_VGP4, "touch"); 
    #elif defined (TINNO_ANDROID_S8122)
				hwPowerDown(MT65XX_POWER_LDO_VGP5, "touch"); 
    #endif
  #elif defined(MT6577)
				hwPowerDown(MT65XX_POWER_LDO_VGP, "touch"); 
  #elif defined(MT6572)
				hwPowerDown(TPD_POWER_SOURCE_CUSTOM, "touch");
  				hwPowerDown(MT6323_POWER_LDO_VGP2, "touch");
  #else
				mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_GPIO);
				mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_OUT);
				mt_set_gpio_out(GPIO_CTP_EINT_PIN, GPIO_OUT_ZERO);
  #endif	
				msleep(2);  
				fts_5x36_hw_init();
			}else{
				break;
			}
			iRetry--;
			msleep(100);  
		} 
#endif//CONFIG_TOUCHSCREEN_POWER_DOWN_WHEN_SLEEP
		FT5x36_complete_unfinished_event();

		atomic_set( &g_pts->ts_sleepState, 1 );
		
	}
 } 


 static struct tpd_driver_t tpd_device_driver = {
	.tpd_device_name = "FT5x36",
	.tpd_local_init = tpd_local_init,
	.suspend = tpd_suspend,
	.resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
	.tpd_have_button = 1,
#else
	.tpd_have_button = 0,
#endif		
 };
 
 /* called when loaded into kernel */
 static int __init tpd_driver_init(void) 
 {
	CTP_DBG("MediaTek FT5x36 touch panel driver init\n");
	i2c_register_board_info(TPD_I2C_GROUP_ID, &FT5x36_i2c_tpd, sizeof(FT5x36_i2c_tpd)/sizeof(FT5x36_i2c_tpd[0]));
	if(tpd_driver_add(&tpd_device_driver) < 0)
		TPD_DMESG("add FT5x36 driver failed\n");
	return 0;
 }
 
 /* should never be called */
 static void __exit tpd_driver_exit(void) 
{
	TPD_DMESG("MediaTek FT5x36 touch panel driver exit\n");
	//input_unregister_device(tpd->dev);
	tpd_driver_remove(&tpd_device_driver);
}
 
 module_init(tpd_driver_init);
 module_exit(tpd_driver_exit);
