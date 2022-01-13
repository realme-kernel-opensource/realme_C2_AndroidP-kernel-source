/* drivers/input/touchscreen/mediatek/gt9xx_mtk/gtp_extents.c
 *
 * Copyright  (C)  2010 - 2016 Goodix., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be a reference
 * to you, when you are integrating the GOODiX's CTP IC into your system,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * Version: V2.6.0.4
 */

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/input.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <asm/ioctl.h>
#include "include/tpd_gt9xx_common.h"
#include <mt-plat/mtk_boot.h>

/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2018/02/21 add for proc node*/
extern s32 gtp_read_rawdata(struct i2c_client* client, u16* data);
extern s32 gt9_read_raw_cmd(struct i2c_client* client);
extern s32 gtp_raw_test_init(void);
extern s32 gtp_parse_config(struct i2c_client *client);
extern s32 gt9_read_coor_cmd(struct i2c_client *client);
extern s32 gtp_read_diff_data(struct i2c_client* client,u16* data);
extern struct proc_dir_entry *gtp_android_touch_proc;
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/14 modified for node function */
int  firmwre_input_val = -1;
u8 limit_control = 0;
u8 limit_direction = 0;
u8 game_control = 0;
bool OPPO_GTP_PROC_SEND_FLAG = false;
extern unsigned int oppo_irq;
unsigned int oppo_debug_level = 0;
extern u16 boot_mode;

extern u8  gt9xx_drv_num;
extern u8  gt9xx_sen_num;
extern s32 *test_rslt_buf;
extern struct gt9xx_open_info *touchpad_sum;
extern u16 max_limit_vale_re[];
extern u16 min_limit_vale_re[];
extern u16 accord_limit_vale_re[];
u8 raw_data[42*30*2+20];
u8 diff_data[42*30*2+20];
u8 double_tap_flag=0;

#ifdef CONFIG_GTP_GESTURE_WAKEUP

#define GESTURE_NODE "goodix_gesture"
#define GTP_REG_WAKEUP_GESTURE     		0x814B
#define GTP_REG_WAKEUP_GESTURE_DETAIL	0x9420
#define GTP_REG_GESTURE_START           0x814d
#define GTP_REG_GESTURE_DATA_LEN           32

#define SETBIT(longlong, bit)   (longlong[bit/8] |=  (1 << bit%8))
#define CLEARBIT(longlong, bit) (longlong[bit/8] &=(~(1 << bit%8)))
#define QUERYBIT(longlong, bit) (!!(longlong[bit/8] & (1 << bit%8)))
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2018/02/21 add for proc node*/
#define GESTURE_DOUBLECLICK                     0xCC
#define GESTURE_DOWN_V                          0x5E
#define GESTURE_UP_V                            0x76
#define GESTURE_RIGHT_V                         0x63
#define GESTURE_LEFT_V                          0x3E
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/14 modified for node function */
#define GESTURE_O                               0x5B
#define GESTURE_CLOCKWISE_O                     0x6F
#define GESTURE_DOUBLE_LINE                     0x48
#define GESTURE_LEFT                            0xBB
#define GESTURE_RIGHT                           0xAA
#define GESTURE_DOWN                            0xAB
#define GESTURE_UP                              0xBA
#define GESTURE_M                               0x6D
#define GESTURE_W                               0x77

#define GESTURE_E                               0x33
#define GESTURE_L                               0x44
#define GESTURE_S                               0x46
#define GESTURE_V                               0x54
#define GESTURE_Z                               0x41
#define GESTURE_C                               0x34

#define UnkownGesture       0
#define DouTap              1   // double tap
#define UpVee               2   // V
#define DownVee             3   // ^
#define LeftVee             4   // >
#define RightVee            5   // <
#define Circle              6   // O
#define DouSwip             7   // ||
#define Left2RightSwip      8   // -->
#define Right2LeftSwip      9   // <--
#define Up2DownSwip         10  // |v
#define Down2UpSwip         11  // |^
#define Mgestrue            12  // M
#define Wgestrue            13  // W
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/14 modified for node function */
#define OPPO_GTP_PORTRAIT_MODE 0x00
#define OPPO_GTP_LANDSCAPE_MODE 0x01
#define OPPO_GTP_LANDSCAPE_90_DEGREES 0x01
#define OPPO_GTP_LANDSCAPE_270_DEGREES 0x02
#define OPPO_GTP_DIRECTION   0xC430

static struct proc_dir_entry *gtp_touchpanel_proc1;
static struct proc_dir_entry *oppo_gtp_baseline;
static struct proc_dir_entry *oppo_gtp_datalimt;
static struct proc_dir_entry *oppo_gtp_delta;
static struct proc_dir_entry *oppo_gtp_tap;
static struct proc_dir_entry *oppo_gtp_game;
static struct proc_dir_entry *oppo_gtp_incell;
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/14 modified for node function */
static struct proc_dir_entry *oppo_gtp_fw_update;
static struct proc_dir_entry *oppo_gtp_main_register;
static struct proc_dir_entry *oppo_gtp_tp_direction;
static struct proc_dir_entry *oppo_gtp_tp_limit_enable;
static struct proc_dir_entry *oppo_gtp_irq_depath;
static struct proc_dir_entry *oppo_gtp_debug_level;
static struct proc_dir_entry *oppo_gtp_register_info;

static u8 gestures_flag[32];
struct gesture_data gesture_data;
static struct mutex gesture_data_mutex;
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/14 modified for node function */
static struct mutex register_mutex;
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2018/02/21 add for proc node*/
struct Coordinate {
    int x;
    int y;
};

struct gesture_info {
    uint32_t gesture_type;
    uint32_t clockwise;
    struct Coordinate Point_start;
    struct Coordinate Point_end;
    struct Coordinate Point_mid;
    struct Coordinate Point_1st;
    struct Coordinate Point_2nd;
    struct Coordinate Point_3rd;
    struct Coordinate Point_4th;
};
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/14 modified for node function */
static struct gtp_rwreg_operation_t{
    int reg;             /*  register */
    int len;             /*  read/write length */
    int print_len;  //read length
    int res;
    char *opbuf;         /*  length >= 1, read return value, write: op return */
}gtp_rw_op;

static inline s32 ges_i2c_write_bytes(u16 addr, u8 *buf, s32 len)
{
    return i2c_write_bytes(i2c_client_point, addr, buf, len);
}

static inline s32 ges_i2c_read_bytes(u16 addr, u8 *buf, s32 len)
{
    return i2c_read_bytes(i2c_client_point, addr, buf, len);
}

static ssize_t gtp_gesture_data_read(struct file *file, char __user * page, size_t size, loff_t * ppos)
{
	s32 ret = -1;
	GTP_DEBUG("visit gtp_gesture_data_read. ppos:%d", (int)*ppos);
	if (*ppos) {
		return 0;
	}
	if (size == 4) {
		ret = copy_to_user(((u8 __user *) page), "GT1X", 4);
		return 4;
	}
	ret = simple_read_from_buffer(page, size, ppos, &gesture_data, sizeof(gesture_data));

	GTP_DEBUG("Got the gesture data.");
	return ret;
}

static ssize_t gtp_gesture_data_write(struct file *filp, const char __user * buff, size_t len, loff_t * off)
{
	s32 ret = 0;
	ret = copy_from_user(&gesture_data.enabled, buff, 1);
	if (ret) {
		GTP_ERROR("copy_from_user failed.");
		return -EPERM;
	}

	GTP_DEBUG("gesture enabled:%x, ret:%d", gesture_data.enabled, ret);

	return len;
}

s8 gtp_enter_doze(void)
{
    int ret = -1;
    s8 retry = 0;
    u8 i2c_control_buf[1] = {8};

    GTP_INFO("Entering doze mode.");
    while(retry++ < 5) {
        ret = ges_i2c_write_bytes(0x8046, i2c_control_buf, 1);
        if (ret < 0) {
            GTP_DEBUG("failed to set doze flag into 0x8046, %d", retry);
            continue;
        }

        ret = ges_i2c_write_bytes(0x8040, i2c_control_buf, 1);
        if (!ret){
            gesture_data.doze_status = DOZE_ENABLED;
            GTP_INFO("Gesture mode enabled.");
            return ret;
        }
        msleep(10);
    }
    GTP_ERROR("GTP send doze cmd failed.");
    return ret;
}
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2018/02/21 add for proc node*/
int gesture_coordinate[14];
static void goodix_gesture_report(struct input_dev *dev, u8 gestrue_id, u8 *gesture_buf)
{
	struct gesture_info gesture_info_temp;
    GTP_INFO("gtp gesture_id==0x%x ", gestrue_id);

	memset(&gesture_info_temp, 0, sizeof(struct gesture_info));
    gesture_info_temp.clockwise = 1;//set default clockwise is 1.

    switch (gestrue_id) {
    case GESTURE_LEFT:
        gesture_info_temp.gesture_type = Right2LeftSwip;
        gesture_info_temp.Point_start.x = (gesture_buf[0] & 0xFF) | (gesture_buf[1] & 0xFF) << 8;
        gesture_info_temp.Point_start.y = (gesture_buf[2] & 0xFF) | (gesture_buf[3] & 0xFF) << 8;
        gesture_info_temp.Point_end.x = (gesture_buf[4] & 0xFF) | (gesture_buf[5] & 0xFF) << 8;
        gesture_info_temp.Point_end.y = (gesture_buf[6] & 0xFF) | (gesture_buf[7] & 0xFF) << 8;
        break;
    case GESTURE_RIGHT:
        gesture_info_temp.gesture_type = Left2RightSwip;
        gesture_info_temp.Point_start.x = (gesture_buf[0] & 0xFF) | (gesture_buf[1] & 0xFF) << 8;
        gesture_info_temp.Point_start.y = (gesture_buf[2] & 0xFF) | (gesture_buf[3] & 0xFF) << 8;
        gesture_info_temp.Point_end.x = (gesture_buf[4] & 0xFF) | (gesture_buf[5] & 0xFF) << 8;
        gesture_info_temp.Point_end.y = (gesture_buf[6] & 0xFF) | (gesture_buf[7] & 0xFF) << 8;
        break;
    case GESTURE_UP:
        gesture_info_temp.gesture_type = Down2UpSwip;
        gesture_info_temp.Point_start.x = (gesture_buf[0] & 0xFF) | (gesture_buf[1] & 0xFF) << 8;
        gesture_info_temp.Point_start.y = (gesture_buf[2] & 0xFF) | (gesture_buf[3] & 0xFF) << 8;
        gesture_info_temp.Point_end.x = (gesture_buf[4] & 0xFF) | (gesture_buf[5] & 0xFF) << 8;
        gesture_info_temp.Point_end.y = (gesture_buf[6] & 0xFF) | (gesture_buf[7] & 0xFF) << 8;
        break;
    case GESTURE_DOWN:
        gesture_info_temp.gesture_type = Up2DownSwip;
        gesture_info_temp.Point_start.x = (gesture_buf[0] & 0xFF) | (gesture_buf[1] & 0xFF) << 8;
        gesture_info_temp.Point_start.y = (gesture_buf[2] & 0xFF) | (gesture_buf[3] & 0xFF) << 8;
        gesture_info_temp.Point_end.x = (gesture_buf[4] & 0xFF) | (gesture_buf[5] & 0xFF) << 8;
        gesture_info_temp.Point_end.y = (gesture_buf[6] & 0xFF) | (gesture_buf[7] & 0xFF) << 8;
        break;
    case GESTURE_DOUBLECLICK:
        gesture_info_temp.gesture_type = DouTap;
        gesture_info_temp.Point_start.x = (gesture_buf[0] & 0xFF) | (gesture_buf[1] & 0xFF) << 8;
        gesture_info_temp.Point_start.y = (gesture_buf[2] & 0xFF) | (gesture_buf[3] & 0xFF) << 8;
        gesture_info_temp.Point_end.x = (gesture_buf[4] & 0xFF) | (gesture_buf[5] & 0xFF) << 8;
        gesture_info_temp.Point_end.y = (gesture_buf[6] & 0xFF) | (gesture_buf[7] & 0xFF) << 8;
        break;
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/14 modified for node function */
    case GESTURE_O:
        gesture_info_temp.gesture_type = Circle;
        gesture_info_temp.clockwise = 0;
        gesture_info_temp.Point_start.x = (gesture_buf[0] & 0xFF) | (gesture_buf[1] & 0xFF) << 8;
        gesture_info_temp.Point_start.y = (gesture_buf[2] & 0xFF) | (gesture_buf[3] & 0xFF) << 8;
        gesture_info_temp.Point_end.x = (gesture_buf[4] & 0xFF) | (gesture_buf[5] & 0xFF) << 8;
        gesture_info_temp.Point_end.y = (gesture_buf[6] & 0xFF) | (gesture_buf[7] & 0xFF) << 8;
        gesture_info_temp.Point_mid.x = (gesture_buf[12] & 0xFF) | (gesture_buf[13] & 0xFF) << 8;
        gesture_info_temp.Point_mid.y = (gesture_buf[14] & 0xFF) | (gesture_buf[15] & 0xFF) << 8;
        gesture_info_temp.Point_1st.x = gesture_info_temp.Point_mid.x;
        gesture_info_temp.Point_1st.y = gesture_info_temp.Point_mid.y - ((gesture_buf[10] & 0xFF) | (gesture_buf[11] & 0xFF) << 8)/2;
        gesture_info_temp.Point_2nd.x = gesture_info_temp.Point_mid.x - ((gesture_buf[8] & 0xFF) | (gesture_buf[9] & 0xFF) << 8)/2;
        gesture_info_temp.Point_2nd.y = gesture_info_temp.Point_mid.y;
        gesture_info_temp.Point_3rd.x = gesture_info_temp.Point_mid.x;
        gesture_info_temp.Point_3rd.y = gesture_info_temp.Point_mid.y + ((gesture_buf[10] & 0xFF) | (gesture_buf[11] & 0xFF) << 8)/2;
        gesture_info_temp.Point_4th.x = gesture_info_temp.Point_mid.x + ((gesture_buf[8] & 0xFF) | (gesture_buf[9] & 0xFF) << 8)/2;
        gesture_info_temp.Point_4th.y = gesture_info_temp.Point_mid.y;
        break;
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/14 modified for node function */
    case GESTURE_CLOCKWISE_O:
        gesture_info_temp.gesture_type = Circle;
        gesture_info_temp.clockwise = 1;
        gesture_info_temp.Point_start.x = (gesture_buf[0] & 0xFF) | (gesture_buf[1] & 0xFF) << 8;
        gesture_info_temp.Point_start.y = (gesture_buf[2] & 0xFF) | (gesture_buf[3] & 0xFF) << 8;
        gesture_info_temp.Point_end.x = (gesture_buf[4] & 0xFF) | (gesture_buf[5] & 0xFF) << 8;
        gesture_info_temp.Point_end.y = (gesture_buf[6] & 0xFF) | (gesture_buf[7] & 0xFF) << 8;
	gesture_info_temp.Point_mid.x = (gesture_buf[12] & 0xFF) | (gesture_buf[13] & 0xFF) << 8;
	gesture_info_temp.Point_mid.y = (gesture_buf[14] & 0xFF) | (gesture_buf[15] & 0xFF) << 8;
        gesture_info_temp.Point_1st.x = gesture_info_temp.Point_mid.x;
        gesture_info_temp.Point_1st.y = gesture_info_temp.Point_mid.y - ((gesture_buf[10] & 0xFF) | (gesture_buf[11] & 0xFF) << 8)/2;
        gesture_info_temp.Point_2nd.x = gesture_info_temp.Point_mid.x - ((gesture_buf[8] & 0xFF) | (gesture_buf[9] & 0xFF) << 8)/2;
        gesture_info_temp.Point_2nd.y = gesture_info_temp.Point_mid.y;
        gesture_info_temp.Point_3rd.x = gesture_info_temp.Point_mid.x;
        gesture_info_temp.Point_3rd.y = gesture_info_temp.Point_mid.y + ((gesture_buf[10] & 0xFF) | (gesture_buf[11] & 0xFF) << 8)/2;
        gesture_info_temp.Point_4th.x = gesture_info_temp.Point_mid.x + ((gesture_buf[8] & 0xFF) | (gesture_buf[9] & 0xFF) << 8)/2;
        gesture_info_temp.Point_4th.y = gesture_info_temp.Point_mid.y;
        break;
    case GESTURE_W:
        gesture_info_temp.gesture_type  = Wgestrue;
        gesture_info_temp.Point_start.x = (gesture_buf[0] & 0xFF) | (gesture_buf[1] & 0xFF) << 8;
        gesture_info_temp.Point_start.y = (gesture_buf[2] & 0xFF) | (gesture_buf[3] & 0xFF) << 8;
        gesture_info_temp.Point_end.x = (gesture_buf[4] & 0xFF) | (gesture_buf[5] & 0xFF) << 8;
        gesture_info_temp.Point_end.y = (gesture_buf[6] & 0xFF) | (gesture_buf[7] & 0xFF) << 8;
        gesture_info_temp.Point_mid.x = (gesture_buf[12] & 0xFF) | (gesture_buf[13] & 0xFF) << 8;
        gesture_info_temp.Point_mid.y = (gesture_buf[14] & 0xFF) | (gesture_buf[15] & 0xFF) << 8;
        gesture_info_temp.Point_1st.x = (gesture_info_temp.Point_start.x + gesture_info_temp.Point_mid.x)/2;
        gesture_info_temp.Point_1st.y = 2*gesture_info_temp.Point_mid.y - gesture_info_temp.Point_start.y ;
        gesture_info_temp.Point_2nd.x = gesture_info_temp.Point_mid.x;
        gesture_info_temp.Point_2nd.y = gesture_info_temp.Point_start.y;
        gesture_info_temp.Point_3rd.x = (gesture_info_temp.Point_end.x + gesture_info_temp.Point_mid.x)/2;
        gesture_info_temp.Point_3rd.y = 2*gesture_info_temp.Point_mid.y - gesture_info_temp.Point_start.y;
        gesture_info_temp.Point_4th.x = gesture_info_temp.Point_mid.x;
        gesture_info_temp.Point_4th.y = gesture_info_temp.Point_mid.y;

        break;
    case GESTURE_M:
        gesture_info_temp.gesture_type  = Mgestrue;
        gesture_info_temp.Point_start.x = (gesture_buf[0] & 0xFF) | (gesture_buf[1] & 0xFF) << 8;
        gesture_info_temp.Point_start.y = (gesture_buf[2] & 0xFF) | (gesture_buf[3] & 0xFF) << 8;
        gesture_info_temp.Point_mid.x = (gesture_buf[12] & 0xFF) | (gesture_buf[13] & 0xFF) << 8;
        gesture_info_temp.Point_mid.y = (gesture_buf[14] & 0xFF) | (gesture_buf[15] & 0xFF) << 8;
        gesture_info_temp.Point_1st.x = (gesture_info_temp.Point_start.x + gesture_info_temp.Point_mid.x)/2;
        gesture_info_temp.Point_1st.y = 2*gesture_info_temp.Point_mid.y - gesture_info_temp.Point_start.y;
        gesture_info_temp.Point_2nd.x = gesture_info_temp.Point_mid.x;
        gesture_info_temp.Point_2nd.y = 2*gesture_info_temp.Point_mid.y - gesture_info_temp.Point_1st.y;
        gesture_info_temp.Point_4th.x = (gesture_buf[28] & 0xFF) | (gesture_buf[29] & 0xFF) << 8;
        gesture_info_temp.Point_4th.y = (gesture_buf[30] & 0xFF) | (gesture_buf[31] & 0xFF) << 8;
        gesture_info_temp.Point_end.x = gesture_info_temp.Point_4th.x;
        gesture_info_temp.Point_end.y = gesture_info_temp.Point_4th.y;
        gesture_info_temp.Point_3rd.x = (gesture_info_temp.Point_end.x + gesture_info_temp.Point_mid.x)/2;
        gesture_info_temp.Point_3rd.y = 2*gesture_info_temp.Point_mid.y - gesture_info_temp.Point_start.y;

    gesture_info_temp.Point_4th.x = gesture_info_temp.Point_mid.x;
    gesture_info_temp.Point_4th.y = gesture_info_temp.Point_mid.y;


        break;
    case GESTURE_LEFT_V:
        gesture_info_temp.gesture_type = LeftVee;
        gesture_info_temp.Point_start.x = (gesture_buf[0] & 0xFF) | (gesture_buf[1] & 0xFF) << 8;
        gesture_info_temp.Point_start.y = (gesture_buf[2] & 0xFF) | (gesture_buf[3] & 0xFF) << 8;
        gesture_info_temp.Point_end.x = (gesture_buf[4] & 0xFF) | (gesture_buf[5] & 0xFF) << 8;
        gesture_info_temp.Point_end.y = (gesture_buf[6] & 0xFF) | (gesture_buf[7] & 0xFF) << 8;
        gesture_info_temp.Point_1st.x = (gesture_buf[16] & 0xFF) | (gesture_buf[17] & 0xFF) << 8;
        gesture_info_temp.Point_1st.y = (gesture_buf[18] & 0xFF) | (gesture_buf[19] & 0xFF) << 8;
        gesture_info_temp.Point_2nd.x = (gesture_buf[20] & 0xFF) | (gesture_buf[21] & 0xFF) << 8;
        gesture_info_temp.Point_2nd.y = (gesture_buf[22] & 0xFF) | (gesture_buf[23] & 0xFF) << 8;
        gesture_info_temp.Point_3rd.x = (gesture_buf[24] & 0xFF) | (gesture_buf[25] & 0xFF) << 8;
        gesture_info_temp.Point_3rd.y = (gesture_buf[26] & 0xFF) | (gesture_buf[27] & 0xFF) << 8;
       if(gesture_info_temp.Point_1st.x == gesture_info_temp.Point_2nd.x ){
           gesture_info_temp.Point_1st.x = gesture_info_temp.Point_3rd.x;
           gesture_info_temp.Point_1st.y = gesture_info_temp.Point_3rd.y;
       }else{
           gesture_info_temp.Point_1st.x = gesture_info_temp.Point_2nd.x;
           gesture_info_temp.Point_1st.y = gesture_info_temp.Point_2nd.y;
       }
       gesture_info_temp.Point_2nd.x = 0;
       gesture_info_temp.Point_2nd.y = 0;
       gesture_info_temp.Point_3rd.x = 0;
       gesture_info_temp.Point_3rd.y = 0;

        break;
    case GESTURE_RIGHT_V:
        gesture_info_temp.gesture_type = RightVee;
        gesture_info_temp.Point_start.x = (gesture_buf[0] & 0xFF) | (gesture_buf[1] & 0xFF) << 8;
        gesture_info_temp.Point_start.y = (gesture_buf[2] & 0xFF) | (gesture_buf[3] & 0xFF) << 8;
        gesture_info_temp.Point_end.x = (gesture_buf[4] & 0xFF) | (gesture_buf[5] & 0xFF) << 8;
        gesture_info_temp.Point_end.y = (gesture_buf[6] & 0xFF) | (gesture_buf[7] & 0xFF) << 8;
        gesture_info_temp.Point_1st.x = (gesture_buf[16] & 0xFF) | (gesture_buf[17] & 0xFF) << 8;
        gesture_info_temp.Point_1st.y = (gesture_buf[18] & 0xFF) | (gesture_buf[19] & 0xFF) << 8;
        gesture_info_temp.Point_2nd.x = (gesture_buf[20] & 0xFF) | (gesture_buf[21] & 0xFF) << 8;
        gesture_info_temp.Point_2nd.y = (gesture_buf[22] & 0xFF) | (gesture_buf[23] & 0xFF) << 8;
        gesture_info_temp.Point_3rd.x = (gesture_buf[24] & 0xFF) | (gesture_buf[25] & 0xFF) << 8;
        gesture_info_temp.Point_3rd.y = (gesture_buf[26] & 0xFF) | (gesture_buf[27] & 0xFF) << 8;
       if(gesture_info_temp.Point_1st.x == gesture_info_temp.Point_2nd.x ){
           gesture_info_temp.Point_1st.x = gesture_info_temp.Point_3rd.x;
           gesture_info_temp.Point_1st.y = gesture_info_temp.Point_3rd.y;
       }else{
           gesture_info_temp.Point_1st.x = gesture_info_temp.Point_2nd.x;
           gesture_info_temp.Point_1st.y = gesture_info_temp.Point_2nd.y;
       }
       gesture_info_temp.Point_2nd.x = 0;
       gesture_info_temp.Point_2nd.y = 0;
       gesture_info_temp.Point_3rd.x = 0;
       gesture_info_temp.Point_3rd.y = 0;
        break;
    case GESTURE_UP_V:
        gesture_info_temp.gesture_type  = UpVee;
        gesture_info_temp.Point_start.x = (gesture_buf[0] & 0xFF) | (gesture_buf[1] & 0xFF) << 8;
        gesture_info_temp.Point_start.y = (gesture_buf[2] & 0xFF) | (gesture_buf[3] & 0xFF) << 8;
        gesture_info_temp.Point_end.x = (gesture_buf[4] & 0xFF) | (gesture_buf[5] & 0xFF) << 8;
        gesture_info_temp.Point_end.y = (gesture_buf[6] & 0xFF) | (gesture_buf[7] & 0xFF) << 8;
        gesture_info_temp.Point_1st.x = (gesture_buf[16] & 0xFF) | (gesture_buf[17] & 0xFF) << 8;
        gesture_info_temp.Point_1st.y = (gesture_buf[18] & 0xFF) | (gesture_buf[19] & 0xFF) << 8;
        gesture_info_temp.Point_2nd.x = (gesture_buf[20] & 0xFF) | (gesture_buf[21] & 0xFF) << 8;
        gesture_info_temp.Point_2nd.y = (gesture_buf[22] & 0xFF) | (gesture_buf[23] & 0xFF) << 8;
        gesture_info_temp.Point_3rd.x = (gesture_buf[24] & 0xFF) | (gesture_buf[25] & 0xFF) << 8;
        gesture_info_temp.Point_3rd.y = (gesture_buf[26] & 0xFF) | (gesture_buf[27] & 0xFF) << 8;
       if(gesture_info_temp.Point_1st.x == gesture_info_temp.Point_2nd.x ){
           gesture_info_temp.Point_1st.x = gesture_info_temp.Point_3rd.x;
           gesture_info_temp.Point_1st.y = gesture_info_temp.Point_3rd.y;
       }else{
           gesture_info_temp.Point_1st.x = gesture_info_temp.Point_2nd.x;
           gesture_info_temp.Point_1st.y = gesture_info_temp.Point_2nd.y;
       }
       gesture_info_temp.Point_2nd.x = 0;
       gesture_info_temp.Point_2nd.y = 0;
       gesture_info_temp.Point_3rd.x = 0;
       gesture_info_temp.Point_3rd.y = 0;
        break;
    case GESTURE_DOWN_V:
        gesture_info_temp.gesture_type = DownVee;
        gesture_info_temp.Point_start.x = (gesture_buf[0] & 0xFF) | (gesture_buf[1] & 0xFF) << 8;
        gesture_info_temp.Point_start.y = (gesture_buf[2] & 0xFF) | (gesture_buf[3] & 0xFF) << 8;
        gesture_info_temp.Point_end.x = (gesture_buf[4] & 0xFF) | (gesture_buf[5] & 0xFF) << 8;
        gesture_info_temp.Point_end.y = (gesture_buf[6] & 0xFF) | (gesture_buf[7] & 0xFF) << 8;
        gesture_info_temp.Point_1st.x = (gesture_buf[16] & 0xFF) | (gesture_buf[17] & 0xFF) << 8;
        gesture_info_temp.Point_1st.y = (gesture_buf[18] & 0xFF) | (gesture_buf[19] & 0xFF) << 8;
        gesture_info_temp.Point_2nd.x = (gesture_buf[20] & 0xFF) | (gesture_buf[21] & 0xFF) << 8;
        gesture_info_temp.Point_2nd.y = (gesture_buf[22] & 0xFF) | (gesture_buf[23] & 0xFF) << 8;
        gesture_info_temp.Point_3rd.x = (gesture_buf[24] & 0xFF) | (gesture_buf[25] & 0xFF) << 8;
        gesture_info_temp.Point_3rd.y = (gesture_buf[26] & 0xFF) | (gesture_buf[27] & 0xFF) << 8;
       if(gesture_info_temp.Point_1st.x == gesture_info_temp.Point_2nd.x ){
           gesture_info_temp.Point_1st.x = gesture_info_temp.Point_3rd.x;
           gesture_info_temp.Point_1st.y = gesture_info_temp.Point_3rd.y;
       }else{
           gesture_info_temp.Point_1st.x = gesture_info_temp.Point_2nd.x;
           gesture_info_temp.Point_1st.y = gesture_info_temp.Point_2nd.y;
       }
       gesture_info_temp.Point_2nd.x = 0;
       gesture_info_temp.Point_2nd.y = 0;
       gesture_info_temp.Point_3rd.x = 0;
       gesture_info_temp.Point_3rd.y = 0;
        break;
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/14 modified for node function */
    case GESTURE_DOUBLE_LINE:
        gesture_info_temp.gesture_type = DouSwip;
        gesture_info_temp.Point_start.x = (gesture_buf[16] & 0xFF) | (gesture_buf[17] & 0xFF) << 8;
        gesture_info_temp.Point_start.y = (gesture_buf[18] & 0xFF) | (gesture_buf[19] & 0xFF) << 8;
        gesture_info_temp.Point_end.x = (gesture_buf[20] & 0xFF) | (gesture_buf[21] & 0xFF) << 8;
        gesture_info_temp.Point_end.y = (gesture_buf[22] & 0xFF) | (gesture_buf[23] & 0xFF) << 8;
        gesture_info_temp.Point_1st.x   = (gesture_buf[24] & 0xFF) | (gesture_buf[25] & 0xFF) << 8;
        gesture_info_temp.Point_1st.y   = (gesture_buf[26] & 0xFF) | (gesture_buf[27] & 0xFF) << 8;
        gesture_info_temp.Point_2nd.x   = (gesture_buf[28] & 0xFF) | (gesture_buf[29] & 0xFF) << 8;
        gesture_info_temp.Point_2nd.y   = (gesture_buf[30] & 0xFF) | (gesture_buf[31] & 0xFF) << 8;
        break;
    default:
        gesture_info_temp.gesture_type = UnkownGesture;
        break;
    }

    GTP_INFO("%s, gesture_id: 0x%x, gesture_type: %d, clockwise: %d, points: (%d, %d)(%d, %d)(%d, %d)(%d, %d)(%d, %d)(%d, %d)\n", \
                __func__, gestrue_id, gesture_info_temp.gesture_type, gesture_info_temp.clockwise, \
                gesture_info_temp.Point_start.x, gesture_info_temp.Point_start.y, \
                gesture_info_temp.Point_end.x, gesture_info_temp.Point_end.y, \
                gesture_info_temp.Point_1st.x, gesture_info_temp.Point_1st.y, \
                gesture_info_temp.Point_2nd.x, gesture_info_temp.Point_2nd.y, \
                gesture_info_temp.Point_3rd.x, gesture_info_temp.Point_3rd.y, \
                gesture_info_temp.Point_4th.x, gesture_info_temp.Point_4th.y);
	gesture_coordinate[0]=gesture_info_temp.gesture_type;
	gesture_coordinate[1]=gesture_info_temp.Point_start.x;
	gesture_coordinate[2]=gesture_info_temp.Point_start.y;
	gesture_coordinate[3]=gesture_info_temp.Point_end.x;
	gesture_coordinate[4]=gesture_info_temp.Point_end.y;
	gesture_coordinate[5]=gesture_info_temp.Point_1st.x;
	gesture_coordinate[6]=gesture_info_temp.Point_1st.y;
	gesture_coordinate[7]=gesture_info_temp.Point_2nd.x;
	gesture_coordinate[8]=gesture_info_temp.Point_2nd.y;
	gesture_coordinate[9]=gesture_info_temp.Point_3rd.x;
	gesture_coordinate[10]=gesture_info_temp.Point_3rd.y;
	gesture_coordinate[11]=gesture_info_temp.Point_4th.x;
	gesture_coordinate[12]=gesture_info_temp.Point_4th.y;
	gesture_coordinate[13]=gesture_info_temp.clockwise;
    /* report event key */
    if (gesture_info_temp.gesture_type != UnkownGesture) {
        input_report_key(dev, KEY_F4, 1);
        input_sync(dev);
        input_report_key(dev, KEY_F4, 0);
        input_sync(dev);
    }

}

s32 gesture_event_handler(struct input_dev * dev)
{
	u8 doze_buf[4] = { 0 };
	u8 gesture_buf[40] = { 0 };
	s32 ret = 0;
	int len, extra_len;

	if (DOZE_ENABLED == gesture_data.doze_status) {
		ret = ges_i2c_read_bytes(GTP_REG_WAKEUP_GESTURE, doze_buf, 4);
		GTP_DEBUG("0x%x = 0x%02X,0x%02X,0x%02X,0x%02X", GTP_REG_WAKEUP_GESTURE, doze_buf[0], doze_buf[1], doze_buf[2], doze_buf[3]);
		if (ret == 0 && doze_buf[0] != 0) {
            /*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2018/02/21 add for proc node*/
			/*if (!QUERYBIT(gestures_flag, doze_buf[0])) {
				GTP_INFO("Sorry, this gesture has been disabled.");
				doze_buf[0] = 0x00;
				ges_i2c_write_bytes(GTP_REG_WAKEUP_GESTURE, doze_buf, 1);
				gtp_enter_doze();
				return 0;
			}*/
			mutex_lock(&gesture_data_mutex);
			/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/27 modified for gesture function 2 */
			if (double_tap_flag == 2)
			{
				doze_buf[0] = 0;
				ret = ges_i2c_write_bytes(GTP_REG_WAKEUP_GESTURE, doze_buf, 1);
				if (ret < 0) {
						GTP_ERROR("clear gesture irq 0x814B failed.");
						mutex_unlock(&gesture_data_mutex);
						return 0;
				}
				mutex_unlock(&gesture_data_mutex);
				return 3;
			}
			len = doze_buf[1] & 0x7F;
			if (len > GESTURE_MAX_POINT_COUNT) {
				GTP_ERROR("Gesture contain too many points!(%d)", len);
				len = GESTURE_MAX_POINT_COUNT;
			}
			if (len > 0) {
				ret = ges_i2c_read_bytes(GTP_REG_GESTURE_START, gesture_buf, GTP_REG_GESTURE_DATA_LEN);
				if (ret < 0) {
					GTP_DEBUG("Read gesture data failed.");
					mutex_unlock(&gesture_data_mutex);
					return 0;
				}
			}
			extra_len = doze_buf[1] & 0x80 ? doze_buf[3] : 0;
			if (extra_len > 80) {
				GTP_ERROR("Gesture contain too many extra data!(%d)", extra_len);
				extra_len = 80;
			}
			if (extra_len > 0) {
				ret = ges_i2c_read_bytes(GTP_REG_WAKEUP_GESTURE + 4, &gesture_data.data[4 + len * 4], extra_len);
				if (ret < 0) {
					GTP_DEBUG("Read extra gesture data failed.");
					mutex_unlock(&gesture_data_mutex);
					return 0;
				}
			}

            doze_buf[2] &= ~0x30;
            doze_buf[2] |= extra_len > 0 ? 0x20 : 0x10;

			gesture_data.data[0] = doze_buf[0];	/* gesture type*/
			gesture_data.data[1] = len;	/* gesture points number*/
			gesture_data.data[2] = doze_buf[2];
			gesture_data.data[3] = extra_len;
			mutex_unlock(&gesture_data_mutex);
			GTP_INFO("Gesture: 0x%02X, points: %d", doze_buf[0], doze_buf[1]);

			doze_buf[0] = 0;
			ges_i2c_write_bytes(GTP_REG_WAKEUP_GESTURE, doze_buf, 1);
            /*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2018/02/21 add for proc node*/
			goodix_gesture_report(dev,gesture_data.data[0],gesture_buf);
			/*
            key_code = doze_buf[0] < 16 ?  KEY_F3: KEY_F2;
			GTP_INFO("Gesture: 0x%02X, points: %d", doze_buf[0], doze_buf[1]);
			doze_buf[0] = 0;
			ges_i2c_write_bytes(GTP_REG_WAKEUP_GESTURE, doze_buf, 1);
			input_report_key(dev, key_code, 1);
			input_sync(dev);
			input_report_key(dev, key_code, 0);
			input_sync(dev);
			*/
			GTP_INFO("doze enabled and get valid gesture data\n");
			return 2; /* doze enabled and get valid gesture data*/
		}
		return 1; /* doze enabled, but no invalid gesutre data*/
	}
	return 0; /* doze not enabled*/
}

void gesture_clear_wakeup_data(void)
{
	mutex_lock(&gesture_data_mutex);
	memset(gesture_data.data, 0, 4);
	mutex_unlock(&gesture_data_mutex);
}

#define GOODIX_MAGIC_NUMBER        'G'
#define NEGLECT_SIZE_MASK           (~(_IOC_SIZEMASK << _IOC_SIZESHIFT))

#define GESTURE_ENABLE_TOTALLY      _IO(GOODIX_MAGIC_NUMBER, 1)	// 1
#define GESTURE_DISABLE_TOTALLY     _IO(GOODIX_MAGIC_NUMBER, 2)
#define GESTURE_ENABLE_PARTLY       _IO(GOODIX_MAGIC_NUMBER, 3)
#define GESTURE_DISABLE_PARTLY      _IO(GOODIX_MAGIC_NUMBER, 4)
//#define SET_ENABLED_GESTURE         (_IOW(GOODIX_MAGIC_NUMBER, 5, u8) & NEGLECT_SIZE_MASK)
#define GESTURE_DATA_OBTAIN         (_IOR(GOODIX_MAGIC_NUMBER, 6, u8) & NEGLECT_SIZE_MASK)
#define GESTURE_DATA_ERASE          _IO(GOODIX_MAGIC_NUMBER, 7)

#define IO_IIC_READ                  (_IOR(GOODIX_MAGIC_NUMBER, 100, u8) & NEGLECT_SIZE_MASK)
#define IO_IIC_WRITE                 (_IOW(GOODIX_MAGIC_NUMBER, 101, u8) & NEGLECT_SIZE_MASK)
#define IO_RESET_GUITAR              _IO(GOODIX_MAGIC_NUMBER, 102)
#define IO_DISABLE_IRQ               _IO(GOODIX_MAGIC_NUMBER, 103)
#define IO_ENABLE_IRQ                _IO(GOODIX_MAGIC_NUMBER, 104)
#define IO_GET_VERISON               (_IOR(GOODIX_MAGIC_NUMBER, 110, u8) & NEGLECT_SIZE_MASK)
#define IO_PRINT                     (_IOW(GOODIX_MAGIC_NUMBER, 111, u8) & NEGLECT_SIZE_MASK)
#define IO_VERSION                   "V1.0-20141015"

#define CMD_HEAD_LENGTH             20

static s32 io_iic_read(u8 * data, void __user * arg)
{
	s32 err = -1;
	s32 data_length = 0;
	u16 addr = 0;

	err = copy_from_user(data, arg, CMD_HEAD_LENGTH);
	if (err) {
		GTP_DEBUG("Can't access the memory.");
		return err;
	}

	addr = data[0] << 8 | data[1];
	data_length = data[2] << 8 | data[3];

	err = ges_i2c_read_bytes(addr, &data[CMD_HEAD_LENGTH], data_length);
	if (!err) {
		err = copy_to_user(&((u8 __user *) arg)[CMD_HEAD_LENGTH], &data[CMD_HEAD_LENGTH], data_length);
		if (err) {
			GTP_ERROR("ERROR when copy to user.[addr: %04x], [read length:%d]", addr, data_length);
			return err;
		}
		err = CMD_HEAD_LENGTH + data_length;
	}
	GTP_DEBUG("IIC_READ.addr:0x%4x, length:%d, ret:%d", addr, data_length, err);
	GTP_DEBUG_ARRAY((&data[CMD_HEAD_LENGTH]), data_length);

	return err;
}

static s32 io_iic_write(u8 * data)
{
	s32 err = -1;
	s32 data_length = 0;
	u16 addr = 0;

	addr = data[0] << 8 | data[1];
	data_length = data[2] << 8 | data[3];

	err = ges_i2c_write_bytes(addr, &data[CMD_HEAD_LENGTH], data_length);
	if (!err) {
		err = CMD_HEAD_LENGTH + data_length;
	}

	GTP_DEBUG("IIC_WRITE.addr:0x%4x, length:%d, ret:%d", addr, data_length, err);
	GTP_DEBUG_ARRAY((&data[CMD_HEAD_LENGTH]), data_length);
	return err;
}

/* @return, 0:operate successfully
*         > 0: the length of memory size ioctl has accessed,
*        error otherwise.
*/
static long gtp_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	u32 value = 0;
	s32 ret = 0;		//the initial value must be 0
	u8 *data = NULL;

/*	GTP_DEBUG("IOCTL CMD:%x", cmd); */
/*	GTP_DEBUG("command:%d, length:%d, rw:%s", _IOC_NR(cmd), _IOC_SIZE(cmd),
(_IOC_DIR(cmd) & _IOC_READ) ? "read" : (_IOC_DIR(cmd) & _IOC_WRITE) ? "write" : "-");*/

	if (_IOC_DIR(cmd)) {
		s32 err = -1;
		s32 data_length = _IOC_SIZE(cmd);
		data = (u8 *) kzalloc(data_length, GFP_KERNEL);
		memset(data, 0, data_length);

		if (_IOC_DIR(cmd) & _IOC_WRITE) {
			err = copy_from_user(data, (void __user *)arg, data_length);
			if (err) {
				GTP_DEBUG("Can't access the memory.");
				kfree(data);
				return -1;
			}
		}
	} else {
		value = (u32) arg;
	}

	switch (cmd & NEGLECT_SIZE_MASK) {
	case IO_GET_VERISON:
		if ((u8 __user *) arg) {
			ret = copy_to_user(((u8 __user *) arg), IO_VERSION, sizeof(IO_VERSION));
			if (!ret) {
				ret = sizeof(IO_VERSION);
			}
			GTP_INFO("%s", IO_VERSION);
		}
		break;
	case IO_IIC_READ:
		ret = io_iic_read(data, (void __user *)arg);
		break;

	case IO_IIC_WRITE:
		ret = io_iic_write(data);
		break;

	case IO_RESET_GUITAR: 
		gtp_reset_guitar(i2c_client_point, 10);
		break;

	case IO_DISABLE_IRQ: {
		gtp_irq_disable();
#ifdef CONFIG_GTP_ESD_PROTECT
		gtp_esd_switch(i2c_client_point, SWITCH_OFF);
#endif
#ifdef CONFIG_GTP_CHARGER_DETECT
		gtp_charger_switch(1);
#endif
		break;
		}
	case IO_ENABLE_IRQ: {
			gtp_irq_enable();
		}
#ifdef CONFIG_GTP_ESD_PROTECT
		gtp_esd_switch(i2c_client_point ,SWITCH_ON);
#endif
#ifdef CONFIG_GTP_CHARGER_DETECT
		gtp_charger_switch(1);
#endif
		break;

	case IO_PRINT:
		if (data)
			GTP_INFO("%s", (char *)data);
		break;

	case GESTURE_ENABLE_TOTALLY:
		GTP_DEBUG("ENABLE_GESTURE_TOTALLY");
		gesture_data.enabled = 1;
		break;

	case GESTURE_DISABLE_TOTALLY:
		GTP_DEBUG("DISABLE_GESTURE_TOTALLY");
		gesture_data.enabled = 0;
		break;

	case GESTURE_ENABLE_PARTLY:
		SETBIT(gestures_flag, (u8) value);
		gesture_data.enabled = 1;
		GTP_DEBUG("ENABLE_GESTURE_PARTLY, gesture = 0x%02X, gesture_data.enabled = %d", value, gesture_data.enabled);
		break;

	case GESTURE_DISABLE_PARTLY:
		CLEARBIT(gestures_flag, (u8) value);
		GTP_DEBUG("DISABLE_GESTURE_PARTLY, gesture = 0x%02X, gesture_data.enabled = %d", value, gesture_data.enabled);
		break;

	case GESTURE_DATA_OBTAIN:
		GTP_DEBUG("OBTAIN_GESTURE_DATA");

		mutex_lock(&gesture_data_mutex);
		if (gesture_data.data[1] > GESTURE_MAX_POINT_COUNT) {
			gesture_data.data[1] = GESTURE_MAX_POINT_COUNT;
		}
		if (gesture_data.data[3] > 80) {
			gesture_data.data[3] = 80;
		}
		ret = copy_to_user(((u8 __user *) arg), &gesture_data.data, 4 + gesture_data.data[1] * 4 + gesture_data.data[3]);
		mutex_unlock(&gesture_data_mutex);
		if (ret) {
			GTP_ERROR("ERROR when copy gesture data to user.");
		} else {
			ret = 4 + gesture_data.data[1] * 4 + gesture_data.data[3];
		}
		break;

	case GESTURE_DATA_ERASE:
		GTP_DEBUG("ERASE_GESTURE_DATA");
		gesture_clear_wakeup_data();
		break;

	default:
		GTP_INFO("Unknown cmd.");
		ret = -1;
		break;
	}

	if (data != NULL) {
		kfree(data);
	}

	return ret;
}

#ifdef CONFIG_COMPAT
static long gtp_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	void __user *arg32 = compat_ptr(arg);

	if (!file->f_op || !file->f_op->unlocked_ioctl) {
		return -ENOMEM;
	}

	return file->f_op->unlocked_ioctl(file, cmd, (unsigned long)arg32);
}
#endif

static int gtp_gesture_open(struct inode *node, struct file *flip) {
        GTP_DEBUG("gesture node is opened.");
        return 0;
}

static int gtp_gesture_release(struct inode *node, struct file *filp) {
        GTP_DEBUG("gesture node is closed.");
        return 0;  
}

static const struct file_operations gtp_fops = {
	.owner = THIS_MODULE,
	.open = gtp_gesture_open,
	.release = gtp_gesture_release,
	.read = gtp_gesture_data_read,
	.write = gtp_gesture_data_write,
	.unlocked_ioctl = gtp_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = gtp_compat_ioctl,
#endif
};

s32 gtp_extents_init(void)
{
	struct proc_dir_entry *proc_entry = NULL;

	mutex_init(&gesture_data_mutex);
	mutex_init(&register_mutex);
	memset(gestures_flag, 0, sizeof(gestures_flag));
	memset((u8 *) & gesture_data, 0, sizeof(struct gesture_data));

	proc_entry = proc_create(GESTURE_NODE, 0666, NULL, &gtp_fops);
	if (proc_entry == NULL) {
		GTP_ERROR("Couldn't create proc entry[GESTURE_NODE]!");
		return -1;
	} else {
		GTP_INFO("Create proc entry[GESTURE_NODE] success!");
	}

	return 0;
}

void gtp_extents_exit(void)
{
	remove_proc_entry(GESTURE_NODE, NULL);
}
#endif /* GTP_GESTURE_WAKEUP */
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2018/02/21 add for proc node*/
static int tp_rawdata_open(struct seq_file *file, void* data)
{

    s32 ret = FAIL; // SUCCESS, FAIL
    struct goodix_ts_data *ts;
    u16 *raw_buf = NULL;
    struct i2c_client* client = i2c_client_point;
	int iRow = 0;
	int iCol = 0;
	int raw_num=0;
	int count=0;
	unsigned char temp_buf[2] = {13, 28};

    ts = i2c_get_clientdata(client);
    ret = gtp_parse_config(client);
    if (ret == FAIL)
    {
        GTP_DEBUG("failed to parse config...");
        ret = FAIL;
        goto open_test_exit;
    }
    raw_buf = (u16*)kmalloc(sizeof(u16)* (gt9xx_drv_num*gt9xx_sen_num), GFP_KERNEL);
    if (NULL == raw_buf)
    {
        GTP_DEBUG("failed to allocate mem for raw_buf!");
        ret = FAIL;
        goto open_test_exit;
    }

    GTP_DEBUG("\nStep 1: Send Rawdata Cmd");

    ret = gtp_raw_test_init();
    if (FAIL == ret)
    {
        GTP_DEBUG("Allocate memory for open test failed!");
        ret = FAIL;
        goto open_test_exit;
    }
    ret = gt9_read_raw_cmd(client);
    if (ret == FAIL)
    {
        GTP_DEBUG("Send Read Rawdata Cmd failed!");
        ret = FAIL;
        goto open_test_exit;
    }

    GTP_DEBUG("Step 3: Sample Rawdata");
    ret = gtp_read_rawdata(client, raw_buf);
    if (ret == FAIL)
    {
        GTP_DEBUG("Read Rawdata failed!");
        ret = FAIL;
        goto open_test_exit;
    }
	    /*--------------------Show RawData---------------------*/
    for (iRow = 0; iRow < temp_buf[0]; iRow++) {
        seq_printf(file, "\nTx%2d:", iRow + 1);
        for (iCol = 0; iCol < temp_buf[1]; iCol++) {
            seq_printf(file, "%5d:", raw_buf[raw_num]);
			raw_num++;
        }
	}


open_test_exit:
    if (raw_buf)
    {
        kfree(raw_buf);
    }
    if (test_rslt_buf)
    {
        kfree(test_rslt_buf);
        test_rslt_buf = NULL;
    }
    if (touchpad_sum)
    {
        kfree(touchpad_sum);
        touchpad_sum = NULL;
    }

    gt9_read_coor_cmd(client);  // back to read coordinates data
    GTP_DEBUG("---gtp open test end---");
	return count;
}


static int gtp_rawdata_show(struct inode* inode, struct file* file)
{
	return single_open(file, tp_rawdata_open, NULL);
}


static const struct file_operations tp_rawdata_fops = {
	.owner = THIS_MODULE,
	.open = gtp_rawdata_show,
	.read = seq_read,
	.release = single_release,
};

static int tp_diffdata_open(struct seq_file *file, void* data)
{
    s32 ret = FAIL; // SUCCESS, FAIL
    struct goodix_ts_data *ts;
    u16 *raw_buf = NULL;
    struct i2c_client* client = i2c_client_point;
	int iRow = 0;
	int iCol = 0;
	int raw_num=0;
	int count=0;
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/14 modified for node function */
	unsigned char temp_buf[2] = {13,28};

    ts = i2c_get_clientdata(client);
    ret = gtp_parse_config(client);
    if (ret == FAIL)
    {
        GTP_DEBUG("failed to parse config...");
        ret = FAIL;
        goto open_test_exit;
    }
    raw_buf = (u16*)kmalloc(sizeof(u16)* (gt9xx_drv_num*gt9xx_sen_num), GFP_KERNEL);
    if (NULL == raw_buf)
    {
        GTP_DEBUG("failed to allocate mem for raw_buf!");
        ret = FAIL;
        goto open_test_exit;
    }

    GTP_DEBUG("\nStep 1: Send Rawdata Cmd");

    ret = gtp_raw_test_init();
    if (FAIL == ret)
    {
        GTP_DEBUG("Allocate memory for open test failed!");
        ret = FAIL;
        goto open_test_exit;
    }
    ret = gt9_read_raw_cmd(client);
    if (ret == FAIL)
    {
        GTP_DEBUG("Send Read Rawdata Cmd failed!");
        ret = FAIL;
        goto open_test_exit;
    }

    GTP_DEBUG("Step 2: Sample Diffdata");

    ret = gtp_read_diff_data(client,raw_buf);
    if (ret == FAIL)
    {
        GTP_DEBUG("Read Diffdata failed!");
        ret = FAIL;
        goto open_test_exit;
    }

	for (iRow = 0; iRow < temp_buf[0]; iRow++) {
        seq_printf(file, "\nTx%2d:", iRow + 1);
        for (iCol = 0; iCol < temp_buf[1]; iCol++) {
            seq_printf(file, "%5d:", raw_buf[raw_num]);
			raw_num++;
        }
	}

open_test_exit:
    if (raw_buf)
    {
        kfree(raw_buf);
    }
    if (test_rslt_buf)
    {
        kfree(test_rslt_buf);
        test_rslt_buf = NULL;
    }
    if (touchpad_sum)
    {
        kfree(touchpad_sum);
        touchpad_sum = NULL;
    }

    gt9_read_coor_cmd(client);  // back to read coordinates data
    GTP_DEBUG("---gtp open test end---");
	return count;
}

static int gtp_diffdata_show(struct inode* inode, struct file* file)
{
	return single_open(file, tp_diffdata_open, NULL);
}

static const struct file_operations tp_diffdata_fops = {
	.owner = THIS_MODULE,
	.open = gtp_diffdata_show,
	.read = seq_read,
	.release = single_release,
};

extern const u16 max_limit_vale_id0[];
extern const u16 min_limit_vale_id0[];
extern const u16 accord_limit_vale_id0[];
static int gtp_data_limit_proc_show(struct seq_file *file, void* data)
{
	int iRow,iCol,numi=0;
	int jRow,jCol,numj=0;
	int kRow,kCol,numk=0;
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/14 modified for node function */
	unsigned char temp_buf[2] = {13,28};

	seq_printf(file, "max_limit_vale\n");
	for (iRow = 0; iRow < temp_buf[0]; iRow++) {
        seq_printf(file, "\nTx%2d:", iRow + 1);
        for (iCol = 0; iCol < temp_buf[1]; iCol++) {
            seq_printf(file, "%5d", max_limit_vale_id0[numi]);
			numi++;
        }
	}

	seq_printf(file, "\nmin_limit_vale");
	for (jRow = 0; jRow < temp_buf[0]; jRow++) {
        seq_printf(file, "\nTx%2d:", jRow + 1);
        for (jCol = 0; jCol < temp_buf[1]; jCol++) {
            seq_printf(file, "%5d", min_limit_vale_id0[numj]);
			numj++;
        }
	}

	seq_printf(file, "\naccord_limit_vale");
	for (kRow = 0; kRow < temp_buf[0]; kRow++) {
        seq_printf(file, "\nTx%2d:", kRow + 1);
        for (kCol = 0; kCol < temp_buf[1]; kCol++) {
            seq_printf(file, "%5d", accord_limit_vale_id0[numk]);
			numk++;
        }
	}

	return 0;
}
static int tp_data_limit_open (struct inode* inode, struct file* file)
{
	return single_open(file, gtp_data_limit_proc_show, NULL);
}

static const struct file_operations tp_data_limit_fops = {
	.owner = THIS_MODULE,
	.open = tp_data_limit_open,
	.read = seq_read,
	.release = single_release,
};

static ssize_t oppo_gtp_gesture_write(struct file *filp, const char *buff, size_t size, loff_t *pPos)
{
    char *temp_buf = NULL;
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/27 modified for gesture function 2 */
/*
  int ret = 0;
  u8 gesture_cmd[3] = {0xee, 0x00, 0x12};
*/
    temp_buf = kzalloc(size,GFP_KERNEL);
    if (!temp_buf) {
        return -ENOMEM;
    }
    if (buff != NULL) {
        if (copy_from_user(temp_buf, buff, size)) {
            kfree(temp_buf);
            return -EINVAL;;
        }
    }
    if (temp_buf[0] == '0') {
        double_tap_flag=0;
    }else if (temp_buf[0] == '1') {
        double_tap_flag= 1;
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/14 modified for gesture function */
    }else if (temp_buf[0] == '2') {
         double_tap_flag= 2;
 /*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/27 modified for gesture function 2 */
 /*
         ret = i2c_write_bytes(i2c_client_point, GTP_REG_SLEEP, gesture_cmd, 3);
         if (ret < 0) {
             GTP_ERROR("Write gesture data failed.");
         }
*/
    }
    GTP_INFO("Gesture write value: %d \n", double_tap_flag);
    gesture_data.enabled = double_tap_flag;
    kfree(temp_buf);

    return size;
}

static int gtp_gtp_gesture_show(struct seq_file *file, void* data)
{
	if(double_tap_flag == 1){
		seq_printf(file, "Gesture open\n");
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/14 modified for gesture function */
	}else if (double_tap_flag == 2){
		seq_printf(file, "Gesture open, but p-sensor closed\n");
	}else{
		seq_printf(file, "Gesture close\n");
	}

	return 0;
}
static int oppo_gtp_gesture_open (struct inode* inode, struct file* file)
{
	return single_open(file, gtp_gtp_gesture_show, NULL);
}

static const struct file_operations oppo_gtp_double_fops =
{
    .owner = THIS_MODULE,
    .write = oppo_gtp_gesture_write,
    .open  = oppo_gtp_gesture_open,
    .read  = seq_read,
};

/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/01 modified for coverity*/
static ssize_t oppo_gtp_game_write(struct file *filp, const char *buff, size_t size, loff_t *pPos)
{
	s32 ret = 0;
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/14 modified for node function */
	u8 game_open[1] = {0x46};
	u8 game_close[1] = {0x47};
	char *temp_buf = NULL;
	temp_buf = kzalloc(size,GFP_KERNEL);
	if (!temp_buf) {
		return -ENOMEM;
	}
	if (buff != NULL) {
		if (copy_from_user(temp_buf, buff, size)) {
			kfree(temp_buf);
			return -EINVAL;;
		}
	}
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/14 modified for node function */
	if (temp_buf[0] == '1') {
		GTP_INFO("Write game 1 data.");
		ret = i2c_write_bytes(i2c_client_point, GTP_REG_SLEEP, game_open, 1);
		if (ret < 0) {
			GTP_ERROR("Write game 1 data failed.");
		}
		game_control = 1;
	} else {
	    GTP_INFO("Write game 0 data.");
		ret = i2c_write_bytes(i2c_client_point, GTP_REG_SLEEP, game_close, 1);
		if (ret < 0) {
			GTP_ERROR("Write game 0 data failed.");
		}
		game_control = 0;
	}

	kfree(temp_buf);

	return size;
}
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/14 modified for node function */
static ssize_t gtp_gtp_game_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int len = 0;
	char *ptr = NULL;

	if ( *ppos ) {
	    GTP_INFO("is already read the file\n");
	    return 0;
	}
	*ppos += count;
	ptr = kzalloc(count,GFP_KERNEL);
	if(ptr == NULL){
	    GTP_INFO("%s, allocate memory fail\n", __func__);
	   return -ENOMEM;
	}
	len = snprintf(ptr, count,"%x\n", game_control);
	if (copy_to_user(buf, ptr, len)) {
	    GTP_INFO("%s,here:%d\n", __func__, __LINE__);
	}
	kfree(ptr);
	return len;
}

static const struct file_operations oppo_gtp_game_fops =
{
	.owner = THIS_MODULE,
	.write = oppo_gtp_game_write,
	.read  = gtp_gtp_game_read,
};

static int gtp_gtp_incell_show(struct seq_file *file, void* data)
{
	seq_printf(file, "0\n");

	return 0;
}
static int oppo_gtp_incell_open (struct inode* inode, struct file* file)
{
	return single_open(file, gtp_gtp_incell_show, NULL);
}

static const struct file_operations oppo_gtp_incell_fops =
{
	.owner = THIS_MODULE,
	.open  = oppo_gtp_incell_open,
	.read  = seq_read,
};
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/14 modified for node function */
/* tp_fw_update */
static ssize_t oppo_gtp_fw_update_write(struct file *file, const char __user *page, size_t size, loff_t *lo)
{
    int val = 0;
    char buf[4] = {0};
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/04/09 modified for don't update when charge of closed phone */
    GTP_INFO("%s enter, boot_mode = %d.\n", __func__, boot_mode);
    if (boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT)
    {
        GTP_INFO("boot mode is BOOT_MODE__CHARGE,not need update tp firmware\n");
        return size;
    }
    if (copy_from_user(buf, page, size)) {
        GTP_INFO("%s: read proc input error.\n", __func__);
        return size;
    }
    sscanf(buf, "%d", &val);
    firmwre_input_val = val;
    GTP_INFO("%s, write value: %d .\n", __func__,firmwre_input_val);
    if ((firmwre_input_val == 0) || (firmwre_input_val == 1) || (firmwre_input_val == 2) )
    {
	    gup_update_proc(NULL);
    } else {
        GTP_INFO("%s: input value=%d  is error.\n", __func__, firmwre_input_val);
	return size;
    }
    GTP_INFO("fw update finished\n");
    return size;
}

static const struct file_operations oppo_gtp_fw_update_fops =
{
	.owner = THIS_MODULE,
	.write = oppo_gtp_fw_update_write,
};


static ssize_t oppo_gtp_direction_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int len = 0;
	char *ptr = NULL;

	if ( *ppos ) {
	    GTP_INFO("%s, is already read the file\n", __func__);
	    return 0;
	}
	*ppos += count;

	ptr = kzalloc(count, GFP_KERNEL);
	if(ptr == NULL){
		GTP_INFO("%s, allocate memory fail\n", __func__);
		return -ENOMEM;
	}
	len = snprintf(ptr, count, "%x\n", limit_direction);
	if (copy_to_user(buf, ptr, len)) {
	    GTP_INFO("%s,here:%d\n", __func__, __LINE__);
	}
	kfree(ptr);
	return len;
}

static ssize_t oppo_gtp_direction_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	u8 *ptr = NULL;
	s32 ret = 0;
	u8 buf_value[1] = {0};
	ptr = kzalloc(count, GFP_KERNEL);
	if (ptr == NULL) {
		GTP_INFO("%s, allocate the memory fail\n", __func__);
		return -ENOMEM;
	}
	if(copy_from_user(ptr, buf, count)) {
		GTP_INFO("input value error\n");
		count = -1;
		goto OUT;
	}
	if (ptr[0] == '0') {
		GTP_INFO("limit direction: portrait\n");
		limit_direction = OPPO_GTP_PORTRAIT_MODE;
		buf_value[0] = 0x43;
		ret = i2c_write_bytes(i2c_client_point, GTP_REG_SLEEP, buf_value, 1);
		if (ret < 0) {
			GTP_ERROR("Write limit direction: portrait failed.\n");
		}
	} else if (ptr[0] == '1') {
		GTP_INFO("limit direction: landscape 90 degrees\n");
		limit_direction = OPPO_GTP_LANDSCAPE_90_DEGREES;
		buf_value[0] = 0x44;
		ret = i2c_write_bytes(i2c_client_point, GTP_REG_SLEEP, buf_value, 1);
		if (ret < 0) {
			GTP_ERROR("Write landscape 90 degreest failed. \n");
		}
       } else if (ptr[0] == '2') {
		GTP_INFO("limit direction: landscape 270 degrees\n");
		limit_direction = OPPO_GTP_LANDSCAPE_270_DEGREES;
		buf_value[0] = 0x45;
		ret = i2c_write_bytes(i2c_client_point, GTP_REG_SLEEP, buf_value, 1);
		if (ret < 0) {
			GTP_ERROR("Write landscape 270 degrees failed.\n");
		}
	} else{
               GTP_ERROR("Unknown command\n");
	}
   OUT:
	 kfree(ptr);
	return count;
}

static const struct file_operations oppo_gtp_direction_fops = {
	.owner = THIS_MODULE,
	.read = oppo_gtp_direction_read,
	.write = oppo_gtp_direction_write,
};

static int gtp_main_register_show(struct seq_file *file, void* data)
{
	u8 main_register_buff[2] ={0};
	ges_i2c_read_bytes(0x8144, main_register_buff, 2);
	seq_printf(file, "IC version:%d, 0x%02x\n",main_register_buff[1],main_register_buff[0]);
	ges_i2c_read_bytes(0x814A, main_register_buff, 2);
	seq_printf(file, "Sensorr_id[0]:%d,\n",main_register_buff[0]);
	ges_i2c_read_bytes(0x8047, main_register_buff, 2);
	seq_printf(file, "Param Ver:%d\n",main_register_buff[0]);
	return 0;
}

static int oppo_gtp_main_register_open (struct inode* inode, struct file* file)
{
	return single_open(file, gtp_main_register_show, NULL);
}

static const struct file_operations oppo_gtp_main_register_fops =
{
	.owner = THIS_MODULE,
	.open  = oppo_gtp_main_register_open,
	.read  = seq_read,
};

/* oppo_fts_tp_limit_enable */
static ssize_t oppo_gtp_tp_limit_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *ppos)
{
	char *ptr = NULL;
	GTP_INFO("%s, enter\n", __func__);
	ptr = kzalloc(count, GFP_KERNEL);

	if (ptr == NULL) {
		GTP_INFO("allocate the memory fail\n");
		return -ENOMEM;
	}
	if(copy_from_user(ptr, buf, count)) {
		GTP_INFO("input value error\n");
		count = -1;
		goto OUT;
	}
	if (ptr[0] == '0') {
		GTP_INFO("limit mode: landscape\n");
		limit_control = OPPO_GTP_LANDSCAPE_MODE;
	} else if (ptr[0] == '1') {
		GTP_INFO("limit mode: portrait\n");
		limit_control = OPPO_GTP_PORTRAIT_MODE;
	} else {
		GTP_INFO("Unknown command\n");
	}
	OUT:
	kfree(ptr);
	return count;
}

static ssize_t oppo_gtp_tp_limit_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int len = 0;
	char *ptr = NULL;

	if ( *ppos ) {
	    GTP_INFO("is already read the file\n");
	    return 0;
	}
	*ppos += count;
	ptr = kzalloc(count,GFP_KERNEL);
	if(ptr == NULL){
	    GTP_INFO("%s, allocate memory fail\n", __func__);
	   return -ENOMEM;
	}
	len = snprintf(ptr, count,"%x\n", limit_control);
	if (copy_to_user(buf, ptr, len)) {
	    GTP_INFO("%s,here:%d\n", __func__, __LINE__);
	}
	kfree(ptr);
	return len;
}

static const struct file_operations oppo_gtp_tp_limit_fops =
{
	.owner = THIS_MODULE,
	.write = oppo_gtp_tp_limit_write,
	.read = oppo_gtp_tp_limit_read,
};

/* fts_tp_irq_depth */
static ssize_t oppo_gtp_irq_depth_read(struct file *file, char *buf,
								  size_t len, loff_t *pos)
{
	size_t ret = 0;
	char *temp_buf = NULL;
	struct irq_desc *desc = irq_to_desc(oppo_irq);
	GTP_INFO("%s: enter \n", __func__);

	if (!OPPO_GTP_PROC_SEND_FLAG) {
		temp_buf = kzalloc(len, GFP_KERNEL);
		if (!temp_buf) {
		        GTP_INFO("%s, allocate memory failed!\n", __func__);
		        return -ENOMEM;
	       }
		ret += snprintf(temp_buf + ret, len - ret, "now depth=%d\n", desc->depth);
		if (copy_to_user(buf, temp_buf, len)) {
			GTP_INFO("%s,here:%d\n", __func__, __LINE__);
		}
		kfree(temp_buf);
		OPPO_GTP_PROC_SEND_FLAG = 1;
	} else {
		OPPO_GTP_PROC_SEND_FLAG = 0;
	}
	return ret;
}

static const struct file_operations oppo_gtp_irq_depath_fops = {
	.owner = THIS_MODULE,
	.read = oppo_gtp_irq_depth_read,
};

/* oppo_fts_debug_level */
static ssize_t oppo_gtp_debug_level_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *ppos)
{
    unsigned int tmp = 0;
    char cmd[10] = {0};

    if (count >= 10) {
        GTP_INFO("no command exceeds 10 chars\n");
	return -EINVAL;
    }
    memset(cmd, '\0', sizeof(cmd));
    if(copy_from_user(cmd, buf, count)) {
        GTP_INFO("input value error\n");
        return -EINVAL;
    }
    sscanf(cmd, "%d", &tmp);
    oppo_debug_level = tmp;
    GTP_INFO("debug_level is %d\n", oppo_debug_level);

    if ((oppo_debug_level != 0) && (oppo_debug_level != 1) && (oppo_debug_level != 2)) {
        GTP_INFO("debug level error %d\n", oppo_debug_level);
        oppo_debug_level = 0;
    }
    return count;
}

static ssize_t oppo_gtp_debug_level_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	size_t ret = 0;
	char *temp_buf = NULL;
	if ( *ppos ) {
		    printk("is already read the file\n");
		    return 0;
	}
        *ppos += count;
	GTP_INFO("%s: enter, %d \n", __func__, __LINE__);
	temp_buf = (char *)kzalloc(count, GFP_KERNEL);
       if (!temp_buf) {
	        GTP_INFO("%s, allocate memory failed!\n", __func__);
	        return -ENOMEM;
	}
	ret += snprintf(temp_buf, count, "%d\n", oppo_debug_level);
	if (copy_to_user(buf, temp_buf, count))
		GTP_INFO("%s,here:%d\n", __func__, __LINE__);
	kfree(temp_buf);
	return ret;
}

static const struct file_operations oppo_gtp_debug_level_fops =
{
	.owner = THIS_MODULE,
	.write = oppo_gtp_debug_level_write,
	.read = oppo_gtp_debug_level_read,
};

/*gtp_tp_register_rw*/
static int oppo_gtp_register_info_show(struct seq_file *file, void* data)
{
    int i = 0;

    mutex_lock(&register_mutex);

    if (gtp_rw_op.len < 0) {
        seq_printf(file, "Invalid cmd line\n");
    }else {
        seq_printf(file, "Read Reg: [%02X]-[%02X]\n", gtp_rw_op.reg, gtp_rw_op.reg + gtp_rw_op.print_len - 1);
        seq_printf(file, "Result: ");
        if (gtp_rw_op.res) {
            seq_printf(file, "failed, ret: %d\n", gtp_rw_op.res);
        } else {
            if (gtp_rw_op.opbuf) {
                for (i = 0; i < gtp_rw_op.print_len; i++) {
                    seq_printf(file, "%02X ", gtp_rw_op.opbuf[i]);
                }
                seq_printf(file, "\n");
            }
        }
    }
    mutex_unlock(&register_mutex);
    return 0;
}

static int shex_to_int_gtp(const char *hex_buf, int size)
{
    int i;
    int base = 1;
    int value = 0;
    char single;

    for (i = size - 1; i >= 0; i--) {
        single = hex_buf[i];

        if ((single >= '0') && (single <= '9')) {
            value += (single - '0') * base;
        } else if ((single >= 'a') && (single <= 'z')) {
            value += (single - 'a' + 10) * base;
        } else if ((single >= 'A') && (single <= 'Z')) {
            value += (single - 'A' + 10) * base;
        } else {
            return -EINVAL;
        }

        base *= 16;
    }
    return value;
}

static ssize_t oppo_gtp_register_info_write(struct file *filp, const char *buff, size_t size, loff_t *pPos)
{
    char cmd[512] = { 0 };
    int ret = 0;
   char * temp_buf=NULL;

    gtp_rw_op.len = size - 1;

	 if (gtp_rw_op.opbuf) {
		 kfree(gtp_rw_op.opbuf);
		 gtp_rw_op.opbuf = NULL;
	 }

     mutex_lock(&register_mutex);
    if (buff != NULL) {
        if (copy_from_user(cmd, buff, gtp_rw_op.len)) {
            GTP_INFO("copy data from user space, failed\n");
	    mutex_unlock(&register_mutex);
            return -EINVAL;
        }
    }

    GTP_INFO("%d, cmd len: %d, buf: %s\n", __LINE__, (int)gtp_rw_op.len, cmd);
	   //command format:  reg+length, such as 81441, reg=8144, length = 1;
	if(gtp_rw_op.len != 5){
	            GTP_INFO("input command format error . \n");
		    GTP_INFO("command format:  reg+length, such as 81441, reg=8144, length = 1. \n");
		    mutex_unlock(&register_mutex);
                    return -EINVAL;
	}
	gtp_rw_op.reg = shex_to_int_gtp(cmd, gtp_rw_op.len-1);
	gtp_rw_op.print_len = shex_to_int_gtp(&cmd[gtp_rw_op.len-1], 1);
	if (gtp_rw_op.len > 0) {
	        temp_buf = (char *)kzalloc(gtp_rw_op.len, GFP_KERNEL);
	        if (!temp_buf) {
	            GTP_ERROR("allocate memory failed!\n");
		    mutex_unlock(&register_mutex);
	            return -ENOMEM;
	        }
	}
        gtp_rw_op.opbuf = temp_buf;
	GTP_INFO("length= %x, reg addr %02x \n",  gtp_rw_op.print_len, gtp_rw_op.reg);
	ret = i2c_read_bytes(i2c_client_point, gtp_rw_op.reg, gtp_rw_op.opbuf, gtp_rw_op.print_len);
	if (ret < 0) {
		gtp_rw_op.res = 1;
		GTP_ERROR("read datafailed.");
	}
	gtp_rw_op.res = 0;
    mutex_unlock(&register_mutex);
    return size;
}

static int oppo_gtp_register_info_open(struct inode* inode, struct file* file)
{
    return single_open(file, oppo_gtp_register_info_show, PDE_DATA(inode));
}

static const struct file_operations oppo_gtp_register_info_fops = {
    .owner = THIS_MODULE,
    .open = oppo_gtp_register_info_open,
    .read = seq_read,
    .write = oppo_gtp_register_info_write,
};

s32 gtp_proc_init(struct i2c_client *client)
{
	int ret = 0;

//debug_level
	gtp_touchpanel_proc1 = proc_mkdir(GTP_DEBUG_INFO, gtp_android_touch_proc);
	if (gtp_touchpanel_proc1 == NULL ){
	GTP_INFO("Create proc entry[touchpanel_proc1] failed!");
	}
// rawdata
	oppo_gtp_baseline = proc_create(GTP_TP_RAWDATA, 0666, gtp_touchpanel_proc1, &tp_rawdata_fops);
	if (!oppo_gtp_baseline){
		dev_err(&client->dev, "create_proc_entry %s failed\n",GTP_TP_RAWDATA);
	}
	else {
		dev_info(&client->dev, "create proc entry %s success\n",GTP_TP_RAWDATA);
	}

//data_limit
	oppo_gtp_datalimt = proc_create(GTP_TP_DATA_LIMIT, 0666, gtp_touchpanel_proc1, &tp_data_limit_fops);
	if (!oppo_gtp_datalimt){
		dev_err(&client->dev, "create_proc_entry %s failed\n",GTP_TP_DATA_LIMIT);
	}
	else {
		dev_info(&client->dev, "create proc entry %s success\n",GTP_TP_DATA_LIMIT);
	}

//delta
	oppo_gtp_delta = proc_create(GTP_TP_DIFFDATA, 0666, gtp_touchpanel_proc1, &tp_diffdata_fops);
	if (!oppo_gtp_delta){
		dev_err(&client->dev, "create_proc_entry %s failed\n",GTP_TP_DIFFDATA);
	}
	else {
		dev_info(&client->dev, "create proc entry %s success\n",GTP_TP_DIFFDATA);
	}

//double_tap_enable
	oppo_gtp_tap = proc_create(GTP_TP_TAP, 0666, gtp_android_touch_proc, &oppo_gtp_double_fops);
	if (!oppo_gtp_tap){
		dev_err(&client->dev, "create_proc_entry %s failed\n",GTP_TP_TAP);
	}
	else {
		dev_info(&client->dev, "create proc entry %s success\n",GTP_TP_TAP);
	}

//game_switch_enable
	oppo_gtp_game = proc_create(GTP_TP_GAME, 0666, gtp_android_touch_proc, &oppo_gtp_game_fops);
	if (!oppo_gtp_game){
		dev_err(&client->dev, "create_proc_entry %s failed\n",GTP_TP_GAME);
	}
	else {
		dev_info(&client->dev, "create proc entry %s success\n",GTP_TP_GAME);
	}

//incell_panel
	oppo_gtp_incell = proc_create(GTP_TP_INCELL, 0666, gtp_android_touch_proc, &oppo_gtp_incell_fops);
	if (!oppo_gtp_incell){
		dev_err(&client->dev, "create_proc_entry %s failed\n",GTP_TP_INCELL);
	}
	else {
		dev_info(&client->dev, "create proc entry %s success\n",GTP_TP_INCELL);
	}
//oppo_register_info
	oppo_gtp_register_info = proc_create(OPPO_GTP_REGISTER_INFO, 0666, gtp_android_touch_proc, &oppo_gtp_register_info_fops);
	if (!oppo_gtp_register_info){
		dev_err(&client->dev, "create_proc_entry %s failed\n",OPPO_GTP_REGISTER_INFO);
	}
	else {
		dev_info(&client->dev, "create proc entry %s success\n",OPPO_GTP_REGISTER_INFO);
	}
//main_register
	oppo_gtp_main_register = proc_create(GTP_TP_MAIN_REGISTER, 0666, gtp_android_touch_proc, &oppo_gtp_main_register_fops);
	if (!oppo_gtp_main_register){
		dev_err(&client->dev, "create_proc_entry %s failed\n",GTP_TP_MAIN_REGISTER);
	}
	else {
		dev_info(&client->dev, "create proc entry %s success\n",GTP_TP_MAIN_REGISTER);
	}
//oppo_tp_direction
	oppo_gtp_tp_direction = proc_create(GTP_TP_DIRECTION, 0666, gtp_android_touch_proc, &oppo_gtp_direction_fops);
	if (!oppo_gtp_tp_direction){
		dev_err(&client->dev, "create_proc_entry %s failed\n",GTP_TP_DIRECTION);
	}
	else {
		dev_info(&client->dev, "create proc entry %s success\n",GTP_TP_DIRECTION);
	}
//irq_depth
	oppo_gtp_irq_depath = proc_create(OPPO_GTP_IRQ_DEPATH, 0666, gtp_android_touch_proc, &oppo_gtp_irq_depath_fops);
	if (!oppo_gtp_irq_depath){
		dev_err(&client->dev, "create_proc_entry %s failed\n",OPPO_GTP_IRQ_DEPATH);
	}
	else {
		dev_info(&client->dev, "create proc entry %s success\n",OPPO_GTP_IRQ_DEPATH);
	}
//oppo_tp_limit_enable
	oppo_gtp_tp_limit_enable = proc_create(OPPO_FTS_TP_LIMIT_ENABLE,0666, gtp_android_touch_proc, &oppo_gtp_tp_limit_fops);
	if (!oppo_gtp_tp_limit_enable){
		dev_err(&client->dev, "create_proc_entry %s failed\n",OPPO_FTS_TP_LIMIT_ENABLE);
	}
	else {
		dev_info(&client->dev, "create proc entry %s success\n",OPPO_FTS_TP_LIMIT_ENABLE);
	}
//tp_fw_update
	oppo_gtp_fw_update = proc_create(GTP_TP_FW_UPDATE, 0666, gtp_android_touch_proc, &oppo_gtp_fw_update_fops);
	if (!oppo_gtp_fw_update){
		dev_err(&client->dev, "create_proc_entry %s failed\n",GTP_TP_FW_UPDATE);
	}
	else {
		dev_info(&client->dev, "create proc entry %s success\n",GTP_TP_FW_UPDATE);
	}
//proc file: /proc/touchpanel/debug_level
	oppo_gtp_debug_level= proc_create(OPPO_GTP_DEBUG_LEVEL, 0644, gtp_android_touch_proc, &oppo_gtp_debug_level_fops);
	if (!oppo_gtp_debug_level){
		dev_err(&client->dev, "create_proc_entry %s failed\n",OPPO_GTP_DEBUG_LEVEL);
	}
	else {
		dev_info(&client->dev, "create proc entry %s success\n",OPPO_GTP_DEBUG_LEVEL);
	}
	return ret;
}
