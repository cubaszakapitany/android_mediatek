/*
 * st_fts.h - Platform data for st-series touch driver
 *
 * Copyright (C) 2013 TCL Inc.
 * Author: xiaoyang <xiaoyang.zhang@tcl.com>
 *
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#ifndef _LINUX_ST_TOUCH_H
#define _LINUX_ST_TOUCH_H
#include <mach/mt_typedefs.h>

#define FTS_TS_DRV_NAME                     "st_fts"
#define FTS_TS_DRV_VERSION                  "1108"
#define FTS_ID0                             0x39
#define FTS_ID1                             0x80

#define FTS_FIFO_MAX                        32
#define FTS_EVENT_SIZE                      8
#define READ_EVENT_SIZE			    3


#define PRESSURE_MIN                        0
#define PRESSURE_MAX                        127

#define FINGER_MAX                          10
#define STYLUS_MAX                          1
#define TOUCH_ID_MAX                        (FINGER_MAX + STYLUS_MAX)

#define AREA_MIN                            PRESSURE_MIN
#define AREA_MAX                            PRESSURE_MAX

/*
 * Firmware
 */
#define FTS_FW_NAME		"fts.fw"
#define FTS_FW_SIZE		0x10000

/* Delay to be wait for flash command completion */
#define FTS_FLASH_COMMAND_DELAY    3000

/*
 * Events ID
 */
#define EVENTID_NO_EVENT                    0x00
#define EVENTID_ENTER_POINTER               0x03
#define EVENTID_LEAVE_POINTER               0x04
#define EVENTID_MOTION_POINTER              0x05
#define EVENTID_HOVER_ENTER_POINTER         0x07
#define EVENTID_HOVER_MOTION_POINTER        0x08
#define EVENTID_HOVER_LEAVE_POINTER         0x09
#define EVENTID_PROXIMITY_ENTER             0x0B
#define EVENTID_PROXIMITY_LEAVE             0x0C
#define EVENTID_BUTTON_STATUS               0x0E
#define EVENTID_ERROR                       0x0F
#define EVENTID_CONTROLLER_READY            0x10
#define EVENTID_SLEEPOUT_CONTROLLER_READY   0x11
#define EVENTID_STATUS                      0x16
#define EVENTID_GESTURE                     0x20
#define EVENTID_PEN_ENTER                   0x23
#define EVENTID_PEN_LEAVE                   0x24
#define EVENTID_PEN_MOTION                  0x25

#define EVENTID_LAST                        (EVENTID_PEN_MOTION+1)


/*
 * Commands
 */
#define INT_ENABLE                          0x41
#define INT_DISABLE                         0x00
#define READ_STATUS                         0x84
#define READ_ONE_EVENT                      0x85
#define READ_ALL_EVENT                      0x86
#define SLEEPIN                             0x90
#define SLEEPOUT                            0x91
#define SENSEOFF                            0x92
#define SENSEON                             0x93
#define SELF_SENSEON                        0x95
#define PROXIMITY_ON                        0x97
#define GESTURE_ON                          0x9D
#define FLUSHBUFFER                         0xA1
#define FORCECALIBRATION                    0xA2
#define CX_TUNNING                          0xA3
#define SELF_TUNING                         0xA4
#define HOVER_ON                            0x95
#define BUTTON_ON                           0x9b
#define CHARGER_PLUGGED                     0xA7
#define TUNING_BACKUP                       0xFC

/* Flash programming */
#define FLASH_LOAD_FIRMWARE                 0xF0
#define FLASH_PROGRAM                       0xF2
#define FLASH_ERASE                         0xF3
#define FLASH_READ_STATUS                   0xF4
#define FLASH_UNLOCK                        0xF7
#define FLASH_WRITE_INFO_BLOCK              0xF8
#define FLASH_ERASE_INFO_BLOCK              0xF9
#define FLASH_PROGRAM_INFO_BLOCK            0xFA

#define FLASH_UNLOCK_CODE_0                 0x74
#define FLASH_UNLOCK_CODE_1                 0x45

#define FLASH_STATUS_UNKNOWN                (-1)
#define FLASH_STATUS_READY                  (0)
#define FLASH_STATUS_BUSY                   (1)

/*
#define FLASH_LOAD_CHUNK_SIZE               (2048)
#define FLASH_LOAD_COMMAND_SIZE             (FLASH_LOAD_CHUNK_SIZE + 3)
*/
#define FLASH_LOAD_FIRMWARE_OFFSET          0x0000
#define FLASH_LOAD_CHUNK_SIZE               (128)
#define FLASH_LOAD_COMMAND_SIZE             (FLASH_LOAD_CHUNK_SIZE + 3)

#define ISC_XFER_LEN			128
#define ISC_BLOCK_NUM			(FLASH_LOAD_CHUNK_SIZE / ISC_XFER_LEN)

#define I2C_MASTER_CLOCK_ST              400

//#define ST_FIRMWARE_SIZE		64*1024
#define ST_FIRMWARE_SIZE		(64*1024+32)

/*
 * Gesture direction
 */
#define GESTURE_RPT_LEFT                    1
#define GESTURE_RPT_RIGHT                   2
#define GESTURE_RPT_UP                      3
#define GESTURE_RPT_DOWN                    4


/*
 * Configuration mode
 */
#define MODE_NORMAL                         0
#define MODE_HOVER                          1
#define MODE_PROXIMITY                      2
#define MODE_HOVER_N_PROXIMITY              3
#define MODE_GESTURE                        4
#define MODE_GESTURE_N_PROXIMITY            5
#define MODE_GESTURE_N_PROXIMITY_N_HOVER    6

/*
 * Status Event Field:
 *     id of command that triggered the event
 */
#define FTS_STATUS_MUTUAL_TUNE              0x01
#define FTS_STATUS_SELF_TUNE                0x02
#define FTS_FLASH_WRITE_CONFIG              0x03
#define FTS_FLASH_WRITE_COMP_MEMORY         0x04
#define FTS_FORCE_CAL_SELF_MUTUAL           0x05
#define FTS_FORCE_CAL_SELF                  0x06
#define FTS_WATER_MODE_ON                   0x07
#define FTS_WATER_MODE_OFF                  0x08


#endif /* _LINUX_ST_TOUCH_H */