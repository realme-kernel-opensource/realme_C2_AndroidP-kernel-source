/* Himax Android Driver Sample Code for common functions
*
* Copyright (C) 2017 Himax Corporation.
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#ifndef HIMAX_COMMON_H
#define HIMAX_COMMON_H

#include <asm/segment.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/async.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/input/mt.h>
#include <linux/firmware.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/pm_wakeup.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2018/12/26 add for factory and meta*/
#include <mt-plat/mtk_boot.h>
#include "himax_platform.h"

#if defined(CONFIG_FB_1)
#include <linux/notifier.h>
#include <linux/fb.h>
#elif defined(CONFIG_HAS_EARLYSUSPEND)
#include <linux/earlysuspend.h>
#endif

#ifdef CONFIG_OF
#include <linux/of_gpio.h>
#endif
#define HIMAX_DRIVER_VER "0.2.17.0_Huaqin_oppo_05"

#define FLASH_DUMP_FILE "/sdcard/HX_Flash_Dump.bin"
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/26 modified for oppo fw update*/
#define HX_FW_FILE "tp/18540/Himax_firmware.bin"
#define HX_FW_FILE_SIGNED "tp/18540/Himax_firmware_signed.bin"

#if defined(CONFIG_TOUCHSCREEN_HIMAX_DEBUG)

#define HX_TP_PROC_DIAG
#define HX_TP_PROC_REGISTER
#define HX_TP_PROC_DEBUG
#define HX_TP_PROC_FLASH_DUMP
#define HX_TP_PROC_SELF_TEST
#define HX_TP_PROC_RESET
#define HX_TP_PROC_SENSE_ON_OFF
#define HX_TP_PROC_2T2R
//#define HX_TP_PROC_GUEST_INFO
extern uint32_t g_oppo_debug_level;
/*zhangyin@ODM_HQ.BSP.TP.Function, 2019/02/12 add for oppo headset state*/
extern u32 g_oppo_headset_state_flag;

#ifdef HX_TP_PROC_SELF_TEST
#define HX_TP_SELF_TEST_DRIVER //if enable, selftest works in driver
//#define HX_ORG_SELFTEST
#endif

int himax_touch_proc_init(void);
void himax_touch_proc_deinit(void);
#endif
//===========Himax Option function=============
#define HX_RST_PIN_FUNC
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2018/12/18 modified for hx8527 resume work
  Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 modified for hx8527*/
#define HX_RESUME_SEND_CMD
#define HX_ESD_RECOVERY
#define HX_AUTO_UPDATE_FW
//#define HX_CHIP_STATUS_MONITOR		/*for ESD 2nd solution,it does not support incell,default off*/
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 modified for hx8527*/
#define HX_SMART_WAKEUP
#define HX_GESTURE_TRACK
//#define HX_HIGH_SENSE
//#define HX_PALM_REPORT
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/17 add for hx8527*/
#define HX_USB_DETECT_GLOBAL
//#define HX_USB_DETECT_CALLBACK
//#define HX_PROTOCOL_A					/* for MTK special platform.If turning on,it will report to system by using specific format. */
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2018/12/18 modified for hx8527 resume work*/
#define HX_RESUME_HW_RESET
#define HX_PROTOCOL_B_3PA
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/29 modified for hx8527 fix touch info*/
#define HX_FIX_TOUCH_INFO 	/* if open, you need to change the touch info in the fix_touch_info*/

//#define HX_EN_SEL_BUTTON		       	/* Support Self Virtual key		,default is close*/
//#define HX_EN_MUT_BUTTON		       	/* Support Mutual Virtual Key	,default is close*/
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 add for hx8527
 *Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/14 modified for ldo28*/
//#define HX_OPPO_3V3_SLEEP_OFF
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/17 add for hx8527*/
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/28 modified for enable oppo fw update*/
#define OPPO_HX_UPDATE_FW
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/28 modified for smart gesture*/
#define HX_SMART_GESTURE_THRESHOLD 0x0B
#define HX_SMART_GESTURE_LOW_VALUE 0x05

#if defined(HX_EN_SEL_BUTTON) || defined(HX_EN_MUT_BUTTON)
//#define HX_PLATFOME_DEFINE_KEY		/* for specfic platform to set key(button) */
#endif

#define HX_KEY_MAX_COUNT             4
#define DEFAULT_RETRY_CNT            3

#define HX_85XX_A_SERIES_PWON		1
#define HX_85XX_B_SERIES_PWON		2
#define HX_85XX_C_SERIES_PWON		3
#define HX_85XX_D_SERIES_PWON		4
#define HX_85XX_E_SERIES_PWON		5
#define HX_85XX_ES_SERIES_PWON		6
#define HX_85XX_F_SERIES_PWON		7
#define HX_85XX_G_SERIES_PWON		8
#define HX_85XX_H_SERIES_PWON		9
#define HX_83100A_SERIES_PWON		10
#define HX_83102A_SERIES_PWON		11
#define HX_83102B_SERIES_PWON		12
#define HX_83103A_SERIES_PWON		13
#define HX_83110A_SERIES_PWON		14
#define HX_83110B_SERIES_PWON		15
#define HX_83111B_SERIES_PWON		16
#define HX_83112A_SERIES_PWON		17
#define HX_83112B_SERIES_PWON		18
#define HX_83191_SERIES_PWON		19

#define HX_TP_BIN_CHECKSUM_SW		1
#define HX_TP_BIN_CHECKSUM_HW		2
#define HX_TP_BIN_CHECKSUM_CRC		3

#define SHIFTBITS 5

#define  FW_SIZE_32k 	32768
#define  FW_SIZE_60k 	61440
#define  FW_SIZE_64k 	65536
#define  FW_SIZE_124k 	126976
#define  FW_SIZE_128k 	131072

#define NO_ERR 0
#define READY_TO_SERVE 1
#define WORK_OUT	2
#define I2C_FAIL -1
#define MEM_ALLOC_FAIL -2
#define CHECKSUM_FAIL -3
#define GESTURE_DETECT_FAIL -4
#define INPUT_REGISTER_FAIL -5
#define FW_NOT_READY -6
#define LENGTH_FAIL -7
#define OPEN_FILE_FAIL -8
#define ERR_WORK_OUT	-10

#define HX_FINGER_ON	1
#define HX_FINGER_LEAVE	2

#define HX_REPORT_COORD	1
#define HX_REPORT_SMWP_EVENT	2

/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/29 modified for hx8527 fix touch info*/
#ifdef HX_FIX_TOUCH_INFO
enum fix_touch_info
{
	FIX_HX_RX_NUM = 14,
	FIX_HX_TX_NUM = 29,
	FIX_HX_BT_NUM = 0,
	FIX_HX_X_RES = 720,
	FIX_HX_Y_RES = 1560,
	FIX_HX_MAX_PT = 10,
	FIX_HX_XY_REVERSE = false,
	FIX_HX_INT_IS_EDGE = true,
#ifdef HX_TP_PROC_2T2R
	FIX_HX_RX_NUM_2 = 14,
	FIX_HX_TX_NUM_2 = 29
#endif
};
#endif

#ifdef HX_ZERO_FLASH
#define HX_0F_DEBUG
#endif

struct himax_ic_data
{
    int vendor_fw_ver;
    int vendor_config_ver;
    int vendor_touch_cfg_ver;
    int vendor_display_cfg_ver;
    int vendor_cid_maj_ver;
    int vendor_cid_min_ver;
    int vendor_panel_ver;
    int vendor_sensor_id;
    int		HX_RX_NUM;
    int		HX_TX_NUM;
    int		HX_BT_NUM;
    int		HX_X_RES;
    int		HX_Y_RES;
    int		HX_MAX_PT;
    bool	HX_XY_REVERSE;
    bool	HX_INT_IS_EDGE;
#ifdef HX_TP_PROC_2T2R
    int HX_RX_NUM_2;
    int HX_TX_NUM_2;
#endif
};

struct himax_virtual_key
{
    int index;
    int keycode;
    int x_range_min;
    int x_range_max;
    int y_range_min;
    int y_range_max;
};

struct himax_report_data
{
    int touch_all_size;
    int raw_cnt_max;
    int raw_cnt_rmd;
    int touch_info_size;
    uint8_t	finger_num;
    uint8_t	finger_on;
    uint8_t *hx_coord_buf;
    uint8_t hx_state_info[2];
#if defined(HX_SMART_WAKEUP)
    int event_size;
    uint8_t *hx_event_buf;
#endif
#if defined(HX_TP_PROC_DIAG)
    int rawdata_size;
    uint8_t diag_cmd;
    uint8_t *hx_rawdata_buf;
    uint8_t rawdata_frame_size;
#endif
};

struct himax_ts_data
{
    bool suspended;
    atomic_t suspend_mode;
    uint8_t x_channel;
    uint8_t y_channel;
    uint8_t useScreenRes;
    uint8_t diag_command;

    uint8_t protocol_type;
    uint8_t first_pressed[10];
    uint8_t coord_data_size;
    uint8_t area_data_size;
    uint8_t coordInfoSize;
    uint8_t raw_data_frame_size;
    uint8_t raw_data_nframes;
    uint8_t nFinger_support;
    uint8_t irq_enabled;
    uint8_t diag_self[50];
    uint8_t psensor_stus;

    uint16_t finger_pressed;
    uint16_t last_slot;
    uint16_t pre_finger_mask;

    uint32_t debug_log_level;
    uint32_t widthFactor;
    uint32_t heightFactor;
    uint32_t tw_x_min;
    uint32_t tw_x_max;
    uint32_t tw_y_min;
    uint32_t tw_y_max;
    uint32_t pl_x_min;
    uint32_t pl_x_max;
    uint32_t pl_y_min;
    uint32_t pl_y_max;

    int rst_gpio;
    int use_irq;
    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2018/12/29 add for factory and meta*/
    int boot_mode;
    int firmware_update_type;    /*firmware_update_type: 0=check firmware version 1=force update; 2=for FAE debug*/
    int (*power)(int on);
    int pre_finger_data[10][2];

    struct device *dev;
    struct workqueue_struct *himax_wq;
    struct work_struct work;
    struct input_dev *input_dev;
    struct hrtimer timer;
    struct i2c_client *client;
    struct himax_i2c_platform_data *pdata;
    struct himax_virtual_key *button;
    struct mutex rw_lock;
    struct mutex himax_mutex;
    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/04/24 modified for hx8527*/
    struct mutex coord_mutex;

#if defined(CONFIG_FB_1)
    struct notifier_block fb_notif;
    struct workqueue_struct *himax_att_wq;
    struct delayed_work work_att;
#elif defined(CONFIG_HAS_EARLYSUSPEND)
    struct early_suspend early_suspend;
#endif
#ifdef HX_CHIP_STATUS_MONITOR
    struct workqueue_struct *himax_chip_monitor_wq;
    struct delayed_work himax_chip_monitor;
#endif
#ifdef HX_TP_PROC_FLASH_DUMP
    struct workqueue_struct 			*flash_wq;
    struct work_struct 					flash_work;
#endif
#ifdef HX_TP_PROC_GUEST_INFO
    struct workqueue_struct 			*guest_info_wq;
    struct work_struct 					guest_info_work;
#endif

#ifdef HX_AUTO_UPDATE_FW
    struct workqueue_struct *himax_update_wq;
    struct delayed_work work_update;
#endif

#ifdef HX_ZERO_FLASH
	struct workqueue_struct *himax_0f_update_wq;
	struct delayed_work work_0f_update;
#endif

#ifdef HX_TP_PROC_DIAG
    struct workqueue_struct *himax_diag_wq;
    struct delayed_work himax_diag_delay_wrok;
#endif

#ifdef HX_SMART_WAKEUP
    uint8_t SMWP_enable;
    uint8_t gesture_cust_en[16];
    struct wakeup_source ts_SMWP_wake_lock;
#endif

#ifdef HX_HIGH_SENSE
    uint8_t HSEN_enable;
#endif

#if defined(HX_USB_DETECT_CALLBACK)||defined(HX_USB_DETECT_GLOBAL)
    uint8_t usb_connected;
    uint8_t *cable_config;
#endif
    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/02/12 add for headset state*/
    uint8_t headset_connected;

#if defined(CONFIG_TOUCHSCREEN_HIMAX_ITO_TEST)
    struct workqueue_struct *ito_test_wq;
    struct work_struct ito_test_work;
#endif
    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 add for hx8527*/
	uint32_t oppo_swmp_control;
	uint8_t oppo_sp_reg_state;
	uint8_t oppo_sp_en;
};
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 add for hx8527*/
#define OPPO_EN_PORTRAIT				(0x01u << 2)
#define OPPO_EN_LANSCAPE_FRONT_LEFT		(0x01u << 1)
#define OPPO_EN_LANSCAPE_FRONT_RIGHT	(0x01u << 0)
#define OPPO_EN_LANSCAPE_MASK			(OPPO_EN_LANSCAPE_FRONT_LEFT|\
	OPPO_EN_LANSCAPE_FRONT_RIGHT)
#define OPPO_EN_DIRECTION_MASK			(OPPO_EN_PORTRAIT|\
	OPPO_EN_LANSCAPE_FRONT_LEFT|OPPO_EN_LANSCAPE_FRONT_RIGHT)
#define OPPO_EN_GAME_MODE				(0x01u << 3)
#define OPPO_SET_GAME_SWITCH(en, cur_state)		({ \
	en?(OPPO_EN_GAME_MODE|cur_state) \
	:((~OPPO_EN_GAME_MODE)&cur_state); \
})

#define OPPO_GET_GAME_SWITCH(cur_state)	({ \
	(cur_state >> 3)&(0x01); \
})

#define OPPO_SET_PORTRAIT(en, cur_state)		({ \
	(((~OPPO_EN_DIRECTION_MASK)&cur_state)|OPPO_EN_PORTRAIT); \
})

#define OPPO_GET_PORTRAIT(cur_state)	({ \
	(cur_state >> 2)&(0x01); \
})

#define OPPO_SET_LANSCAPE_FRONT_LEFT(en, cur_state)		({ \
	(((~OPPO_EN_DIRECTION_MASK)&cur_state)|OPPO_EN_LANSCAPE_FRONT_LEFT); \
})

#define OPPO_GET_LANSCAPE_FRONT_LEFT(cur_state)	({ \
	(cur_state >> 1)&(0x01); \
})

#define OPPO_SET_LANSCAPE_FRONT_RIGHT(en, cur_state)		({ \
	(((~OPPO_EN_DIRECTION_MASK)&cur_state)|OPPO_EN_LANSCAPE_FRONT_RIGHT); \
})

#define OPPO_GET_LANSCAPE_FRONT_RIGHT(cur_state)	({ \
	(cur_state >> 0)&(0x01); \
})

#define OPPO_SET_TP_LIMIT(en, cur_state, req_state)	({\
	en?req_state:((~OPPO_EN_DIRECTION_MASK)&cur_state); \
})

#ifdef HX_CHIP_STATUS_MONITOR
struct chip_monitor_data
{
    int HX_CHIP_POLLING_COUNT;
    int HX_POLLING_TIMER;//unit:sec
    int HX_POLLING_TIMES;//ex:5(timer)x2(times)=10sec(polling time)
    int HX_ON_HAND_SHAKING;//
    int HX_CHIP_MONITOR_EN;
};
#endif


enum input_protocol_type
{
    PROTOCOL_TYPE_A	= 0x00,
    PROTOCOL_TYPE_B	= 0x01,
};

#ifdef HX_HIGH_SENSE
void himax_set_HSEN_func(struct i2c_client *client,uint8_t HSEN_enable);
#endif

#ifdef HX_SMART_WAKEUP
#define GEST_PTLG_ID_LEN	(4)
#define GEST_PTLG_HDR_LEN	(4)
#define GEST_PTLG_HDR_ID1	(0xCC)
#define GEST_PTLG_HDR_ID2	(0x44)
#define GEST_PT_MAX_NUM 	(128)

void himax_set_SMWP_func(struct i2c_client *client,uint8_t SMWP_enable);

enum gesture_event_type
{
    EV_GESTURE_01 = 0x01,
    EV_GESTURE_02,
    EV_GESTURE_03,
    EV_GESTURE_04,
    EV_GESTURE_05,
    EV_GESTURE_06,
    EV_GESTURE_07,
    EV_GESTURE_08,
    EV_GESTURE_09,
    EV_GESTURE_10,
    EV_GESTURE_11,
    EV_GESTURE_12,
    EV_GESTURE_13,
    EV_GESTURE_14,
    EV_GESTURE_15,
    EV_GESTURE_PWR = 0x80,
};
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/17 add for hx8527*/
/*
#define KEY_CUST_01 251
#define KEY_CUST_02 252
#define KEY_CUST_03 253
#define KEY_CUST_04 254
#define KEY_CUST_05 255
#define KEY_CUST_06 256
#define KEY_CUST_07 257
#define KEY_CUST_08 258
#define KEY_CUST_09 259
#define KEY_CUST_10 260
#define KEY_CUST_11 261
#define KEY_CUST_12 262
#define KEY_CUST_13 263
#define KEY_CUST_14 264
#define KEY_CUST_15 265
*/

#define KEY_CUST_01 0x01
#define KEY_CUST_02 0x02
#define KEY_CUST_03 0x03
#define KEY_CUST_04 0x04
#define KEY_CUST_05 0x05
#define KEY_CUST_06 0x06
#define KEY_CUST_07 0x07
#define KEY_CUST_08 0x08
#define KEY_CUST_09 0x09
#define KEY_CUST_10 0x0A
#define KEY_CUST_11 0x0B
#define KEY_CUST_12 0x0C
#define KEY_CUST_13 0x0D
#define KEY_CUST_14 0x0E
#define KEY_CUST_15 0x0F
#endif

#if defined(CONFIG_TOUCHSCREEN_HIMAX_ITO_TEST)
extern uint8_t himax_ito_test(void);
#endif

#if defined(HX_SMART_WAKEUP) || defined(HX_INSPECT_LPWUG_TEST)
extern uint32_t FAKE_POWER_KEY_SEND;
#endif

extern int irq_enable_count;

//void himax_HW_reset(uint8_t loadconfig,uint8_t int_off);
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 add for hx8527*/
#ifdef HX_GESTURE_TRACK
extern int gest_pt_cnt;
extern int gest_pt_x[GEST_PT_MAX_NUM];
extern int gest_pt_y[GEST_PT_MAX_NUM];
#if 0
extern int gest_start_x=0,gest_start_y=0,gest_end_x=0,gest_end_y=0;
extern int gest_width=0,gest_height=0,gest_mid_x=0,gest_mid_y=0;
#endif
extern int hx_gesture_coor[];
#endif

#ifdef QCT
irqreturn_t himax_ts_thread(int irq, void *ptr);
int himax_input_register(struct himax_ts_data *ts);
#endif

extern int himax_chip_common_probe(struct i2c_client *client, const struct i2c_device_id *id);
extern int himax_chip_common_remove(struct i2c_client *client);
extern int himax_chip_common_suspend(struct himax_ts_data *ts);
extern int himax_chip_common_resume(struct himax_ts_data *ts);

#endif

