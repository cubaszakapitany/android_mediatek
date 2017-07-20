#ifndef __CUST_KEY_H__
#define __CUST_KEY_H__

#include<cust_kpd.h>

#define MT65XX_META_KEY		42	/* KEY_2 */
#ifndef MT65XX_FACTORY_KEY
#define MT65XX_FACTORY_KEY	11 //11 by zhangxian for S8811AE/*invaild key*/
#define MT65XX_PMIC_RST_KEY	MT65XX_FACTORY_KEY
#define MT6582_FCHR_ENB 1
#else
#define MT65XX_PMIC_RST_KEY	MT65XX_FACTORY_KEY //11 /*invaild key*/
#endif
#define MT_CAMERA_KEY 		10

#define MT65XX_BOOT_MENU_KEY       MT65XX_RECOVERY_KEY   /* KEY_VOLUMEUP */
#define MT65XX_MENU_SELECT_KEY     MT65XX_BOOT_MENU_KEY   
#define MT65XX_MENU_OK_KEY         MT65XX_FACTORY_KEY /* KEY_VOLUMEDOWN */

#endif /* __CUST_KEY_H__ */
