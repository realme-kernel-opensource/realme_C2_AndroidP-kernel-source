/********************************************
 ** Copyright (C) 2018 OPPO Mobile Comm Corp. Ltd.
 ** ODM_HQ_EDIT
 ** File: oppoversion.c
 ** Description: Source file for proc oppoversion
 ** Version :1.0
 ** Date : 2018/12/5
 ** Author: duwenchao@ODM_HQ.BSP.system
 ********************************************/
#include "oppoversion.h"

#define oppoversion_name "oppoVersion"

OPPOVERSION oppoversion;
char buff_oppo[32];

static int getinfo_for_oppoversion(int board_id){
	int MB = (board_id & 0xff);
	int KB = (board_id >> 8) & 0x3;
	int Pb = (board_id >> 6) & 0x3;
	
	if(0x0 == Pb){
		strcpy(oppoversion.pcbVersion, "V3");
	}else if(0x3 == Pb){
		strcpy(oppoversion.pcbVersion, "V4");
	}else if(0x2 == Pb){
		strcpy(oppoversion.pcbVersion, "V5");
	}else{
		strcpy(oppoversion.pcbVersion, "UNKOWN");
	}

	if(0x2 == KB){
		strcpy(oppoversion.Kboard, "1");
	}else if(0x0 == KB){
		strcpy(oppoversion.Kboard, "0");
	}else{
		strcpy(oppoversion.Kboard, "unknow");
	}
	
	switch(MB){
		//chenzhecong@ODM_HQ.BSP.SE 
		/*
		case 0x00:
			strcpy(oppoversion.operatorName, "3");
			strcpy(oppoversion.prjVersion, "18541");
			strcpy(oppoversion.modemType, "3");
			strcpy(oppoversion.Mboard, "0000");
			break;
		case 0x30:
			strcpy(oppoversion.operatorName, "2");
			strcpy(oppoversion.prjVersion, "18542");
			strcpy(oppoversion.modemType, "3");
			strcpy(oppoversion.Mboard, "1100");
			break;
		case 0x33:
			strcpy(oppoversion.operatorName, "1");
			strcpy(oppoversion.prjVersion, "18540");
			strcpy(oppoversion.modemType, "5");
			strcpy(oppoversion.Mboard, "1111");
			break;			
		case 0x20:
			strcpy(oppoversion.operatorName, "4");
			strcpy(oppoversion.prjVersion, "18545");
			strcpy(oppoversion.modemType, "3");
			strcpy(oppoversion.Mboard, "1000");
			break;	
		*/
		//EVT LNAÅäÖÃ²ÎÊýchenzhecong@ODM_HQ.BSP.SE	
		case 0x80:
			strcpy(oppoversion.operatorName, "3");
			strcpy(oppoversion.prjVersion, "18541");
			strcpy(oppoversion.modemType, "0");
			strcpy(oppoversion.Mboard, "80");
			break;
		case 0xb0:
			strcpy(oppoversion.operatorName, "2");
			strcpy(oppoversion.prjVersion, "18542");
			strcpy(oppoversion.modemType, "0");
			strcpy(oppoversion.Mboard, "B0");
			break;
		case 0xb3:
			strcpy(oppoversion.operatorName, "1");
			strcpy(oppoversion.prjVersion, "18540");
			strcpy(oppoversion.modemType, "1");
			strcpy(oppoversion.Mboard, "B3");
			break;
		case 0xa0:
			strcpy(oppoversion.operatorName, "4");
			strcpy(oppoversion.prjVersion, "18545");
			strcpy(oppoversion.modemType, "0");
			strcpy(oppoversion.Mboard, "A0");
			break;
		//DVT ·ÇLNAÅäÖÃ²ÎÊýchenzhecong@ODM_HQ.BSP.SE
		/*Tao.Wang@ODM_HQ.BSP.Board Id.Function, 2019/07/29 add new board id*/
		case 0x8C:
			strcpy(oppoversion.operatorName, "6");
			strcpy(oppoversion.prjVersion, "18547");
			strcpy(oppoversion.modemType, "0");
			strcpy(oppoversion.Mboard, "8C");
			break;
		/*Tao.Wang@ODM_HQ.BSP.Board Id.Function, 2019/07/29 add new board id*/
		/*Tao.Wang@ODM_HQ.BSP.Board Id.Function, 2019/08/12 add new board id for 2+32*/
		case 0x83:
			strcpy(oppoversion.operatorName, "7");
			strcpy(oppoversion.prjVersion, "18548");
			strcpy(oppoversion.modemType, "5");
			strcpy(oppoversion.Mboard, "83");
			break;
		/*Tao.Wang@ODM_HQ.BSP.Board Id.Function, 2019/08/12 add new board id for 2+32*/
		case 0xc0:
			strcpy(oppoversion.operatorName, "3");
			strcpy(oppoversion.prjVersion, "18541");
			strcpy(oppoversion.modemType, "4");
			strcpy(oppoversion.Mboard, "C0");
			break;
		case 0xf0:
			strcpy(oppoversion.operatorName, "2");
			strcpy(oppoversion.prjVersion, "18542");
			strcpy(oppoversion.modemType, "4");
			strcpy(oppoversion.Mboard, "F0");
			break;
		case 0xf3:
			strcpy(oppoversion.operatorName, "1");
			strcpy(oppoversion.prjVersion, "18540");
			strcpy(oppoversion.modemType, "5");
			strcpy(oppoversion.Mboard, "F3");
			break;
		case 0xe0:
			strcpy(oppoversion.operatorName, "4");
			strcpy(oppoversion.prjVersion, "18545");
			strcpy(oppoversion.modemType, "4");
			strcpy(oppoversion.Mboard, "E0");
			break;
		case 0xff:
			strcpy(oppoversion.operatorName, "5");
			strcpy(oppoversion.prjVersion, "18546");
			strcpy(oppoversion.modemType, "7");
			strcpy(oppoversion.Mboard, "FF");
			break;
		/*Tao.Wang@ODM_HQ.BSP.Board Id.Function, 2019/07/29 add new board id*/
		case 0xCC:
			strcpy(oppoversion.operatorName, "6");
			strcpy(oppoversion.prjVersion, "18547");
			strcpy(oppoversion.modemType, "4");
			strcpy(oppoversion.Mboard, "CC");
			break;
		/*Tao.Wang@ODM_HQ.BSP.Board Id.Function, 2019/07/29 add new board id*/
		/*Tao.Wang@ODM_HQ.BSP.Board Id.Function, 2019/08/12 add new board id for 2+32*/
		case 0xC3:
			strcpy(oppoversion.operatorName, "7");
			strcpy(oppoversion.prjVersion, "18548");
			strcpy(oppoversion.modemType, "5");
			strcpy(oppoversion.Mboard, "C3");
			break;
		/*Tao.Wang@ODM_HQ.BSP.Board Id.Function, 2019/08/12 add new board id for 2+32*/
		default:
			strcpy(oppoversion.operatorName, "unknow");
			strcpy(oppoversion.prjVersion, "unknow");
			strcpy(oppoversion.modemType, "unknow");
			strcpy(oppoversion.Mboard, "unknow");
			break;
	}

	return 0;
}

static void set_oppoversion_init(void){
	strcpy(oppoversion.modemType, "unknow");
	strcpy(oppoversion.pcbVersion, "unknow");
	strcpy(oppoversion.operatorName, "unknow");
	strcpy(oppoversion.Kboard, "unknow");
	strcpy(oppoversion.Mboard, "unknow");
	strcpy(oppoversion.prjVersion, "unknow");
}

static unsigned int round_kbytes_to_readable_mbytes(unsigned int k){
	unsigned int r_size_m = 0;
	unsigned int in_mega = k/1024;

	if(in_mega > 64*1024){ //if memory is larger than 64G
		r_size_m = 128*1024; // It should be 128G
	}else if(in_mega > 32*1024){  //larger than 32G
		r_size_m = 64*1024; //should be 64G
	}else if(in_mega > 16*1024){
		r_size_m = 32*1024;
	}else if(in_mega > 8*1024){
		r_size_m = 16*1024;
	}else if(in_mega > 6*1024){
		r_size_m = 8*1024;
	}else if(in_mega > 4*1024){
		r_size_m = 6*1024;  //RAM may be 6G
	}else if(in_mega > 3*1024){
		r_size_m = 4*1024;
	}else if(in_mega > 2*1024){
		r_size_m = 3*1024; //RAM may be 3G
	}else if(in_mega > 1024){
		r_size_m = 2*1024;
	}else if(in_mega > 512){
		r_size_m = 1024;
	}else if(in_mega > 256){
		r_size_m = 512;
	}else if(in_mega > 128){
		r_size_m = 256;
	}else{
		k = 0;
	}
	return r_size_m;
}

static void get_ram_size(void){
	#define K(x) ((x) << (PAGE_SHIFT - 10))
	struct sysinfo i;
	si_meminfo(&i);
	if(round_kbytes_to_readable_mbytes(K(i.totalram)) >= 1024){
		sprintf(buff_oppo,"%d",round_kbytes_to_readable_mbytes(K(i.totalram))/1024);
	}else{
		sprintf(buff_oppo,"%dMB",round_kbytes_to_readable_mbytes(K(i.totalram)));
	}
}

static int get_long_size(char *ptr){
	int ret = 0;
	char *p = ptr;
	while(strncmp(" ", p ,1)){
		p++;
		ret++;
	}
	return ret;
}

static int get_oppoversion(char name[]){
	char *ptr;
	if(!strcmp(name, "pcbVersion") || !strcmp(name, "operatorName") || !strcmp(name, "modemType") || !strcmp(name, "prjVersion")
		|| !strcmp(name, "mainboard") || !strcmp(name, "kboard")){
		ptr = strstr(saved_command_line, "board_id=");
		if(ptr != 0){
			ptr += strlen("board_id=");
			getinfo_for_oppoversion(simple_strtol(ptr, NULL, 10));
		}else{
			set_oppoversion_init();
		}
		return 0;
	}else if(!strcmp(name, "ramSize")){
		get_ram_size();
		return 0;
	}else if(!strcmp(name, "bootMode")){
		ptr = strstr(saved_command_line, "boot_reason=");
		if(ptr != 0){
			ptr += strlen("boot_reason=");
			sprintf(buff_oppo, "%d",simple_strtol(ptr, NULL, 10));
		}else{
			sprintf(buff_oppo, "%s","UNKOWN");
		}
		return 0;
	}else if(!strcmp(name, "serialID")){
		ptr = strstr(saved_command_line, "androidboot.serialno=");
		if(ptr != 0){
			ptr += strlen("androidboot.serialno=");
			snprintf(buff_oppo, get_long_size(ptr)+3, "0x%s",ptr);
		}else{
			sprintf(buff_oppo, "%s","UNKOWN");
		}
		return 0;
	}
	return 0;
}

static const char * const oppoversion_proc_list[] = {
	"pcbVersion",
	"operatorName",
	"modemType",
	"prjVersion",
	"kboard",
	"ramSize",
	"bootMode",
	"serialID",
	"mainboard"
};

HQ_OPPOVERSION_ATTR(pcbVersion, "%s",oppoversion.pcbVersion);
HQ_OPPOVERSION_ATTR(operatorName, "%s",oppoversion.operatorName);
HQ_OPPOVERSION_ATTR(modemType, "%s",oppoversion.modemType);
HQ_OPPOVERSION_ATTR(prjVersion, "%s",oppoversion.prjVersion);
HQ_OPPOVERSION_ATTR(kboard, "%s",oppoversion.Kboard);
HQ_OPPOVERSION_ATTR(mainboard, "%s",oppoversion.Mboard);
HQ_OPPOVERSION_ATTR(ramSize, "%s",buff_oppo);
HQ_OPPOVERSION_ATTR(bootMode, "%s",buff_oppo);
HQ_OPPOVERSION_ATTR(serialID, "%s",buff_oppo);

static const struct file_operations *proc_fops_list[] = {
	&pcbVersion_fops,
	&operatorName_fops,
	&modemType_fops,
	&prjVersion_fops,
	&kboard_fops,
	&ramSize_fops,
	&bootMode_fops,
	&serialID_fops,
	&mainboard_fops,
};

int oppoversion_init(void){
	struct proc_dir_entry *prEntry;
	struct proc_dir_entry *oppoversion_dir;
	int i, num;
	
	oppoversion_dir  = proc_mkdir(oppoversion_name, NULL);
	
	if (!oppoversion_dir) {
		pr_notice("[%s]: failed to create /proc/%s\n",__func__, oppoversion_name);
		return -1;
	}
	
	num = ARRAY_SIZE(oppoversion_proc_list);
	for (i = 0; i < num; i++) {
		prEntry = proc_create(oppoversion_proc_list[i], 0444, oppoversion_dir, proc_fops_list[i]);
		if (prEntry)
			continue;
		pr_notice("[%s]: failed to create /proc/bootdevice/%s\n", __func__, oppoversion_proc_list[i]);
	}

	return 0;
}
