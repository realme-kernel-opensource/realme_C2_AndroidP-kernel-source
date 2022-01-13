/********************************************
 ** Copyright (C) 2018 OPPO Mobile Comm Corp. Ltd.
 ** ODM_HQ_EDIT
 ** File: hq_devinfo.c
 ** Description: Source file for proc devinfo
 ** Version :1.0
 ** Date : 2018/12/5
 ** Author: duwenchao@ODM_HQ.BSP.system
 ********************************************/
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/mmc.h>
#include <linux/hq_devinfo.h>
#include "oppoversion.h"
#include "../../input/touchscreen/mediatek/tpd.h"
#include "meta_display.c"
#define devinfo_name "devinfo"

extern char battery_name[20];
extern struct mmc_card *card_devinfo;
extern char *hq_lcm_name;
char buff[128];
char sensor_vendor[3][80];
Cam_buff cam_buff; 

struct sensor_devinfo sensorinfo[] = {
	{"lsm6ds3","ST"},
	{"bmi160","BOSCH"},
	{"bma253","BOSCH"},
	{"lis2doc","ST"},
	{"mmc3530","MICROCHIP"},
	{"memsicd5603x","MICROCHIP"},
	{"akm09918","AKM"},
	{"apds9922","AVAGOTECH"},
	{"STK3335_l","SensorTek"},
	{"stk3x3x_l","SensorTek"},
	{"STK3332_l","SensorTek"},
	{"lsm6ds3","ST"},
};

static void match_sub_board(int boardid){
	int MB = boardid & 0xf;
	int KB = (boardid >> 8) & 0x3;
	if(0x0 == MB){
		if(0x0 == KB){
			sprintf(buff, "Device version: %s\nDevice manufacture: %s","MTK","sub-match");
		}else{
			sprintf(buff, "Device version: %s\nDevice manufacture: %s","MTK","sub-unmatch");
		}
	}else if((0x3 == MB) || (0xc == MB) || (0xf == MB)){
		if((0x2 == KB) || ((0x0 == KB) && (0xc == MB))){
			sprintf(buff, "Device version: %s\nDevice manufacture: %s","MTK","sub-match");
		}else{
			sprintf(buff, "Device version: %s\nDevice manufacture: %s","MTK","sub-unmatch");
		}
	}else{
		sprintf(buff, "Device version: %s\nDevice manufacture: %s","MTK","UNKOWN");
	}
}

static void get_sub_board(void){
	char *ptr;
	ptr = strstr(saved_command_line, "board_id=");
	if(ptr != 0){
		ptr += strlen("board_id=");
		match_sub_board(simple_strtol(ptr, NULL, 10));
	}else{
		sprintf(buff, "Device version: %s\nDevice manufacture: %s","MTK","UNKOWN");
	}
}

static void register_info(char name[]){
	if(!strcmp(name, "emmc")){
		char manfid[8];
		int manfid_m = card_devinfo->cid.manfid;
		switch(manfid_m){
			case 0x15:
				strcpy(manfid, "SAMSUNG");
				break;
			case 0x45:
				strcpy(manfid, "SANDISK");
				break;
			case 0x13:
				strcpy(manfid, "MICRON");
				break;
			case 0x90:
				strcpy(manfid, "HYNIX");
				break;
			default:
				strcpy(manfid, "UNKOWN");
				break;
		}
		sprintf(buff, "Device version: %s\nDevice manufacture: %s",card_devinfo->cid.prod_name, manfid);
		return;
	}else if(!strcmp(name, "emmc_version")){
		sprintf(buff,"Device version: %s\nDevice manufacture: 0x%02x,0x%llx",card_devinfo->cid.prod_name,card_devinfo->cid.prv,*(unsigned long long*)card_devinfo->ext_csd.fwrev);	
		return;
	}else if(!strcmp(name, "lcd")){
		if(!strcmp(hq_lcm_name, "ili9881c_hd_dsi_vdo_txd_boe_zal1890") || !strcmp(hq_lcm_name, "ili9881c_hd_dsi_vdo_inx_boe_zal1890")){
			sprintf(buff, "Device version: %s\nDevice manufacture: %s","ili9881c","TXD_ILI");
		}else if(!strcmp(hq_lcm_name, "ili9881c_hd_dsi_vdo_ls_inx_zal1890")){
			sprintf(buff, "Device version: %s\nDevice manufacture: %s","ili9881c","LS_ILI");
		}else if(!strcmp(hq_lcm_name, "hx8394f_hd_dsi_vdo_hlt_hsd_zal1890")){
			sprintf(buff, "Device version: %s\nDevice manufacture: %s","hx8394f","HLT_HX");
		}else{
			sprintf(buff, "Device version: %s\nDevice manufacture: %s","UNKOWN","UNKOWN");
		}
		return;
	}else if(!strcmp(name, "tp")){
		if(!strcmp(oppo_tp_data.tp_dev_name, "fts_ts") || !strcmp(oppo_tp_data.tp_dev_name, "himax_tp") || !strcmp(oppo_tp_data.tp_dev_name, "gt9xx")){
			sprintf(buff, "Device version: 0x%x\nDevice manufacture: %s\nDevice fw_path: %s", oppo_tp_data.version, oppo_tp_data.manufacture, oppo_tp_data.fw_name);
		}else{
			sprintf(buff, "Device version: %s\nDevice manufacture: %s\nDevice fw_path: %s","UNKOWN","UNKOWN","UNKOWN");
		}
		return;
	}else if(!strcmp(name, "Sensor_gyro")){
		sprintf(buff, "Device version: %s\nDevice manufacture: %s","UNKOWN","UNKOWN");
		return;
	}else if(!strcmp(name, "Cam_b")){
		sprintf(buff, "Device version: %s\nDevice manufacture: %s",cam_buff.cam_b_name,"UNKOWN");
		return;
	}else if(!strcmp(name, "Cam_b2")){
		sprintf(buff, "Device version: %s\nDevice manufacture: %s",cam_buff.cam_b2_name,"UNKOWN");
		return;
	}else if(!strcmp(name, "Cam_f")){
		sprintf(buff, "Device version: %s\nDevice manufacture: %s",cam_buff.cam_f_name,"UNKOWN");
		return;
	}else if(!strcmp(name, "sub_mainboard")){
		get_sub_board();
		return;
	}
}

static void hq_parse_sensor_devinfo(int type, char ic_name[], char vendor[]){
	sprintf(sensor_vendor[type], "Device version:  %s\nDevice manufacture:  %s\n",
			ic_name, vendor);
}

void hq_register_sensor_info(int type, char ic_name[]){
	int i;
	if(type >= 0 && type <3) {
		for (i = 0; i < sizeof(sensorinfo)/sizeof(struct sensor_devinfo); i++) {
			/* Jianmin.Niu@ODM.HQ.BSP.Sensors.Config 2019/2/1 Update string compare */
			if(!strncmp(ic_name, sensorinfo[i].ic_name, strlen(ic_name))) {
				hq_parse_sensor_devinfo(type, sensorinfo[i].ic_name, sensorinfo[i].vendor_name);
				break;
			}
		}
	}
	return;
}

static const char * const devinfo_proc_list[] = {
	"Cam_b",
	"Cam_f",
	"Cam_b2",
	"Sensor_accel",
	"Sensor_alsps",
	"Sensor_gyro",
	"Sensor_msensor",
	"lcd",
	"tp",
	"emmc",
	"emmc_version",
	"battery",
	"sub_mainboard"
};

HQ_DEVINFO_ATTR(Cam_b, "%s",buff);
HQ_DEVINFO_ATTR(Cam_f, "%s",buff);
HQ_DEVINFO_ATTR(Cam_b2, "%s",buff);
HQ_DEVINFO_ATTR(Sensor_accel, "%s",sensor_vendor[ACCEL_HQ]);
HQ_DEVINFO_ATTR(Sensor_alsps, "%s",sensor_vendor[ALSPS_HQ]);
HQ_DEVINFO_ATTR(Sensor_gyro, "%s",buff);
HQ_DEVINFO_ATTR(Sensor_msensor, "%s",sensor_vendor[MSENSOR_HQ]);
HQ_DEVINFO_ATTR(lcd, "%s",buff);
HQ_DEVINFO_ATTR(tp, "%s",buff);
HQ_DEVINFO_ATTR(emmc, "%s",buff);
HQ_DEVINFO_ATTR(emmc_version, "%s",buff);
HQ_DEVINFO_ATTR(battery, "Device version: 4.4V NON_VOOC\nDevice manufacture: %s",battery_name);
HQ_DEVINFO_ATTR(sub_mainboard, "%s",buff);

static const struct file_operations *proc_fops_list[] = {
	&Cam_b_fops,
	&Cam_f_fops,
	&Cam_b2_fops,
	&Sensor_accel_fops,
	&Sensor_alsps_fops,
	&Sensor_gyro_fops,
	&Sensor_msensor_fops,
	&lcd_fops,
	&tp_fops,
	&emmc_fops,
	&emmc_version_fops,
	&battery_fops,
	&sub_mainboard_fops,
};

static int __init devinfo_init(void){
	struct proc_dir_entry *prEntry;
	struct proc_dir_entry *devinfo_dir;
	int i, num;
	
	devinfo_dir  = proc_mkdir(devinfo_name, NULL);

	if (!devinfo_dir) {
		pr_notice("[%s]: failed to create /proc/%s\n",__func__, devinfo_name);
		return -1;
	}
	
	num = ARRAY_SIZE(devinfo_proc_list);
	for (i = 0; i < num; i++) {
		prEntry = proc_create(devinfo_proc_list[i], 0444, devinfo_dir, proc_fops_list[i]);
		if (prEntry)
			continue;
		pr_notice("[%s]: failed to create /proc/devinfo/%s\n", __func__, devinfo_proc_list[i]);
	}
	
	oppoversion_init();
	meta_work_around();
	
	return 0;
}

core_initcall(devinfo_init);
MODULE_AUTHOR("Venco <duwenchao_hq@vanyol.com>");
MODULE_DESCRIPTION("Huaqin Devices Info Driver");
MODULE_LICENSE("GPL");
