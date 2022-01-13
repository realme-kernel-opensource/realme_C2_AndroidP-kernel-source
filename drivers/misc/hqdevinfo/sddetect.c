/********************************************
 ** Copyright (C) 2018 OPPO Mobile Comm Corp. Ltd.
 ** ODM_HQ_EDIT
 ** File: sedetect.c
 ** Description: Source file for proc sdcard_detect
 ** Version :1.0
 ** Date : 2019/1/29
 ** Author: duwenchao@ODM_HQ.BSP.system
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
#include <linux/gpio.h>
#include <mt-plat/mtk_devinfo.h>


#define PROC_SAR_FILE "sdcard_detect"
#define SD_GPIO 1
extern int mtk_get_gpio_value(int gpio);
static int sddetect_proc_show(struct seq_file *file, void* data){
	int gpio_value = -1;
	gpio_value = mtk_get_gpio_value(SD_GPIO);
	seq_printf(file, "%d\n", gpio_value);

	return 0;
}

static int sddetect_proc_open (struct inode* inode, struct file* file){
	return single_open(file, sddetect_proc_show, inode->i_private);
}

static const struct file_operations sddetect_proc_fops = {
	.open = sddetect_proc_open,
	.read = seq_read,
};

static int __init sddetect_init(void){
	struct proc_dir_entry *prEntry;

	prEntry = proc_create(PROC_SAR_FILE, 0644, NULL, &sddetect_proc_fops);
	
	if (prEntry == NULL){
		pr_err("[%s]: create_proc_sddetect entry failed\n", __func__);
	}
		
	return 0;
}

static void __exit sddetect_exit(void)
{
	printk("proc_sddetect_exit\n");
}

late_initcall(sddetect_init);
module_exit(sddetect_exit);

MODULE_AUTHOR("Venco <duwenchao_hq@vanyol.com>");
MODULE_DESCRIPTION("Huaqin Devices Info Driver");
MODULE_LICENSE("GPL");