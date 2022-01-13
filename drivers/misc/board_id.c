/********************************************
 ** Copyright (C) 2018 OPPO Mobile Comm Corp. Ltd.
 ** ODM_HQ_EDIT
 ** File: readme.txt
 ** Description: Source file for get board id
 **          To get board id
 ** Version :1.0
 ** Date : 2018/11/29
 ** Author: zhangmengchun@ODM_HQ.BSP.system
 ********************************************/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/export.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <mt-plat/mtk_devinfo.h>

#define PROC_BOARD_ID_FILE "board_id"

typedef enum {
	BOARD_ID_UNKNOW = 0,
	BOARD_ID_ZAL1890A	= 9001,
	BOARD_ID_ZAL1890B	= 9002,
	BOARD_ID_ZAL1890C	= 9003,
	BOARD_ID_ZAL1890D	= 9004,
	BOARD_ID_ZAL1890E	= 9005,
	BOARD_ID_ZAL1890F	= 9006,
	BOARD_ID_ZAL1891A	= 9101,
	BOARD_ID_ZAL1891B	= 9102,
	BOARD_ID_ZAL1891C	= 9103,
	BOARD_ID_ZAL1891D	= 9104,
	BOARD_ID_ZAL1891E	= 9105,
	BOARD_ID_ZAL1891F	= 9106,
}BOARDID;

static struct proc_dir_entry *entry = NULL;
static BOARDID board_id;
extern char *saved_command_line;

static BOARDID get_board_id(int boardid){
	BOARDID id = BOARD_ID_UNKNOW;
	int boarid_tmp = 0xff & boardid;
	printk("[kernel] Read board id tmp: %d", boarid_tmp);
	switch(boarid_tmp){
		case 0x00:
			id = BOARD_ID_ZAL1890A;
			break;
		case 0x0c:
			id = BOARD_ID_ZAL1890B;
			break;
		case 0x03:
			id = BOARD_ID_ZAL1890C;
			break;
		case 0x30:
			id = BOARD_ID_ZAL1890D;
			break;
		case 0x3c:
			id = BOARD_ID_ZAL1890E;
			break;
		case 0x33:
			id = BOARD_ID_ZAL1890F;
			break;
		case 0x80:
			id = BOARD_ID_ZAL1891A;
			break;
		case 0x8c:
			id = BOARD_ID_ZAL1891B;
			break;
		case 0x83:
			id = BOARD_ID_ZAL1891C;
			break;
		case 0xb0:
			id = BOARD_ID_ZAL1891D;
			break;
		case 0xbc:
			id = BOARD_ID_ZAL1891E;
			break;
		case 0xb3:
			id = BOARD_ID_ZAL1891F;
			break;
		default:
			id = BOARD_ID_UNKNOW;
			break;
	}
	return id;
}

static void read_board_id(void)
{
	char *ptr;

	ptr = strstr(saved_command_line, "board_id=");
	if(ptr != 0){
		ptr += strlen("board_id=");
		board_id = get_board_id(simple_strtol(ptr, NULL, 10));		
	}else{
		board_id = BOARD_ID_UNKNOW;
	}
	pr_debug("[%s]:read board id: %d", __func__, board_id);
	printk("[kernel] Read board id: %d", board_id);
}

static int board_id_proc_show(struct seq_file *file, void* data)
{
	char temp[40] = {0};

	read_board_id();
	sprintf(temp, "%d\n", board_id);
	seq_printf(file, "%s\n", temp);

	return 0;
}

static int board_id_proc_open (struct inode* inode, struct file* file)
{
	return single_open(file, board_id_proc_show, inode->i_private);
}

static const struct file_operations board_id_proc_fops =
{
	.open = board_id_proc_open,
	.read = seq_read,
};

static int __init proc_board_id_init(void)
{
	entry = proc_create(PROC_BOARD_ID_FILE, 0644, NULL, &board_id_proc_fops);
	if (entry == NULL)
	{
		pr_err("[%s]: create_proc_entry entry failed\n", __func__);
	}

	return 0;
}

static void __exit proc_board_id_exit(void)
{
	printk("proc_board_id_exit\n");
}

late_initcall(proc_board_id_init);
module_exit(proc_board_id_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Get Board Id driver");