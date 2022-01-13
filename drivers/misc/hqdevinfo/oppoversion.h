/********************************************
 ** Copyright (C) 2018 OPPO Mobile Comm Corp. Ltd.
 ** ODM_HQ_EDIT
 ** File: oppoversion.h
 ** Description: Source file for proc oppoversion
 ** Version :1.0
 ** Date : 2018/12/5
 ** Author: duwenchao@ODM_HQ.BSP.system
 ********************************************/
#ifndef _HQ_OPPOVERSION_HEAD_
#define _HQ_OPPOVERSION_HEAD_

#include "linux/hq_devinfo.h"

#define HQ_OPPOVERSION_ATTR(name, fmt, args...) \
static int hq_##name##_show(struct seq_file *m, void *v){\
	get_oppoversion(#name);\
	seq_printf(m, fmt, args);\
	return 0;\
}\
static int hq_##name##_open(struct inode *inode, struct file *file){\
	return single_open(file, hq_##name##_show, inode->i_private);\
}\
static const struct file_operations name##_fops = {\
	.open = hq_##name##_open,\
	.read = seq_read,\
}

typedef struct{
	char pcbVersion[8];
	char operatorName[16];
	char modemType[16];
	char prjVersion[8];
	char Kboard[8];
	char Mboard[8];
}OPPOVERSION;

int oppoversion_init(void);
#endif
