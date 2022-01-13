/********************************************
 ** Copyright (C) 2018 OPPO Mobile Comm Corp. Ltd.
 ** ODM_HQ_EDIT
 ** File: meta_display.c
 ** Description: Source file for proc meta_display
 ** Version :1.0
 ** Date : 2019/01/04
 ** Author: duwenchao@ODM_HQ.BSP.system
 ********************************************/
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include <mt-plat/mtk_boot_common.h>

#define META__DISPLAY_GREEN 0x00ff00
#define META__DISPLAY_BLUE  0xff0000
#define META_DELAY_TIME     30
extern void meta_display(unsigned int color);
static struct delayed_work displayBG;
static int GB = 0;
static void display_BG(struct work_struct *work){
	if(0 == GB){
		meta_display(META__DISPLAY_GREEN);
		GB = 1;
	}else if(1 == GB){
		meta_display(META__DISPLAY_BLUE);
		GB = 0;
	}
	schedule_delayed_work(&displayBG, msecs_to_jiffies(META_DELAY_TIME*1000));
}

void meta_display_bg(void){
	INIT_DELAYED_WORK(&displayBG, display_BG);
	schedule_delayed_work(&displayBG, msecs_to_jiffies(META_DELAY_TIME*1000));
}

void meta_work_around(void){
	if(is_meta_mode()){
		meta_display_bg();
	}
}