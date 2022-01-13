/**********************************************************************************
* Copyright (c)  2008-2015  Guangdong OPPO Mobile Comm Corp., Ltd
* VENDOR_EDIT
* Description:     Healthinfo Monitor  Kernel Driver
*
* Version   : 1.0
* Date       : 2018-06-05
* Author     : wenbin.liu@PSW.Kernel.MM
* ------------------------------ Revision History: --------------------------------
* <version>           <date>                <author>                            <desc>
* Revision 1.0        2018-05-24       wenbin.liu@PSW.Kernel.MM       Created for Healthinfomonitor
***********************************************************************************/

#include "oppo_healthinfo.h"
#include <asm/uaccess.h>
#include <linux/kernel.h>
#ifdef CONFIG_OPPO_MEM_MONITOR
#include <linux/memory_monitor.h>
#endif
static struct proc_dir_entry *oppo_healthinfo = NULL;
/* FG IO */
bool iowait_hung_ctrl = false;
unsigned int  iowait_hung_cnt = 0;
unsigned int  iowait_panic_cnt = 0;

static ssize_t healthinfo_switch_read(struct file *filp, char __user *buff, size_t count, loff_t *off)
{
        char page[128] = {0};
        int len = 0;
        
        len = sprintf( page, "switch : %s\n", oppo_healthinfo_switch ? "on":"off");

        if (len > *off) {
                len -= *off;
        } else {
                len = 0;
        }
        if (copy_to_user(buff, page, (len < count ? len : count))) {
                return -EFAULT;
        }
        *off += len < count ? len : count;
        return (len < count ? len : count);
}

static ssize_t healthinfo_switch_write(struct file *file, const char __user *buff, size_t len, loff_t *ppos)
{
	char write_data[16] = {0};
        
	if (copy_from_user(&write_data, buff, len)) {
		health_err("healthinfo_switch write error.\n");
		return -EFAULT;
	}

        if (strncmp(write_data, "en808", 5) == 0) {
                oppo_healthinfo_switch = true;
                health_debug("healthinfo_switch enable\n");
        } else if (strncmp(write_data, "dis808", 6) == 0) {
                oppo_healthinfo_switch = false;
                health_debug("healthinfo_switch disable\n");
        } else if (strncmp(write_data, "en606", 5) == 0) {
                iowait_hung_ctrl = true;
                health_debug("iowait_hung_ctrl enable\n");
        } else if (strncmp(write_data, "dis606", 6) == 0) {
                iowait_hung_ctrl = false;
                health_debug("iowait_hung_ctrl disable\n");
        } else {
                health_err("healthinfo_switch input error\n");
                return -EFAULT;
        }
	return len;
}

static const struct file_operations proc_switch_fops = {
	.read = healthinfo_switch_read,
        .write = healthinfo_switch_write,
};

static ssize_t cpu_load_read(struct file *filp, char __user *buff, size_t count, loff_t *off)
{
        char page[256] = {0};
        int len = 0;
        int load = healthinfo_get_cur_cpuload(oppo_healthinfo_switch);

        if(load < 0)
                load = 0;
        len = sprintf( page, "cur_cpuloading :    %d%%\n", load);

        if (len > *off) {
                len -= *off;
        } else {
                len = 0;
        }
        if (copy_to_user(buff, page, (len < count ? len : count))) {
                return -EFAULT;
        }
        *off += len < count ? len : count;
        return (len < count ? len : count);
}

static const struct file_operations proc_cpu_load_fops = {
	.read = cpu_load_read,
};

#ifdef CONFIG_SCHEDSTATS

#define LOW_THRESH_LEN_MAX 5
#define HIGH_THRESH_LEN_MAX 5
/* low thresh 10~1000ms*/
#define LOW_THRESH_MS_LOW 10
#define LOW_THRESH_MS_HIGH 1000
/* high thresh 100~5000ms*/
#define HIGH_THRESH_MS_LOW 100
#define HIGH_THRESH_MS_HIGH 5000

#define WAIT_PARAM_MAX 10    /* Max Len 10  XXXX:XXXX */
#define WAIT_PARAM_MIN 7    /* Max Len 7  XX:XXX */

static ssize_t sched_latency_read(struct file *filp, char __user *buff, size_t count, loff_t *off)
{
        char page[1024] = {0};
        int len = 0;

        len = sprintf(page, "sched_low_thresh_ms: %d\n""sched_high_thresh_ms: %d\n"
                "sched_low_cnt: %lld\n""sched_high_cnt: %lld\n"
                "sched_total_ms: %lld\n""sched_total_cnt: %lld\n"
                "sched_fg_low_cnt: %lld\n""sched_fg_high_cnt: %lld\n"
                "sched_fg_total_ms: %lld\n""sched_fg_total_cnt: %lld\n"
                "sched_delta_ms: %lld\n",
                sched_latency_para.low_thresh_ms, sched_latency_para.high_thresh_ms,
                sched_latency_para.low_cnt, sched_latency_para.high_cnt,
                sched_latency_para.total_ms, sched_latency_para.total_cnt,
                sched_latency_para.fg_low_cnt, sched_latency_para.fg_high_cnt,
                sched_latency_para.fg_total_ms, sched_latency_para.fg_total_cnt,
                sched_latency_para.delta_ms);

        if (len > *off) {
                len -= *off;
        } else {
                len = 0;
        }
        if (copy_to_user(buff, page, (len < count ? len : count))) {
                return -EFAULT;
        }
        *off += len < count ? len : count;
        return (len < count ? len : count);
}

static ssize_t sched_latency_write(struct file *file, const char __user *buff, size_t len, loff_t *ppos)
{
	char write_data[32] = {0};
        char low_thresh[LOW_THRESH_LEN_MAX] = {0};
        char high_thresh[HIGH_THRESH_LEN_MAX] = {0};
        int temp_low = 0;
        int temp_high = 0;
        int low_len = 0;
        int high_len = 0;
        int i = 0;

	if (copy_from_user(&write_data, buff, len)) {
		health_err("sched_latency write error.\n");
		return -EFAULT;
	}
        write_data[len] = '\0';
        if (write_data[len - 1] == '\n') {
                write_data[len - 1] = '\0';
        }
        health_debug("input %s, %d\n", write_data, (int)len);

        if ((int)len > WAIT_PARAM_MAX || (int)len < WAIT_PARAM_MIN) {
                health_err("Para Len Err, len = %d\n", (int)len);
                goto THRESH_WRITE_ILLEGAL;
        }

        for (i = 0; i < len; i++){
                if (write_data[i] == ':') {
                        low_len = i+1;
                        if (low_len <= LOW_THRESH_LEN_MAX) {
                                strncpy(low_thresh, write_data, low_len);
                                low_thresh[i] = '\0';
                                high_len = len - low_len;
                                health_debug("low thresh %s, %d\n", low_thresh, low_len);
                                if (high_len <= HIGH_THRESH_LEN_MAX) {
                                        strncpy(high_thresh, (write_data + low_len), high_len);
                                        health_debug("high thresh %s, %d\n", high_thresh, high_len);
                                        if ((strspn(low_thresh,"0123456789") == strlen(low_thresh)) &&
                                                (strspn(high_thresh,"0123456789") == strlen(high_thresh)))
                                                break;
                                }
                        } else {
                                health_err("low thresh too long");
                        }
                        goto THRESH_WRITE_ILLEGAL;
                }
        }
        temp_low = (int)simple_strtol(low_thresh, NULL, 10);
        temp_high = (int)simple_strtol(high_thresh, NULL, 10);
        if (temp_low >= temp_high || temp_low < LOW_THRESH_MS_LOW || temp_low > LOW_THRESH_MS_HIGH ||
                temp_high < HIGH_THRESH_MS_LOW || temp_high > HIGH_THRESH_MS_HIGH) {
                /* Input Thresh Illegal */
                goto THRESH_WRITE_ILLEGAL;
        }
        sched_latency_para.high_thresh_ms = temp_high;
        sched_latency_para.low_thresh_ms = temp_low;

        health_debug("USR SPACE Update sched latency ms Thresh: %d, %d\n",
                sched_latency_para.low_thresh_ms, sched_latency_para.high_thresh_ms);
	return len;

THRESH_WRITE_ILLEGAL:
        health_err("Para input illegal\n");
        return -EFAULT;
}

static const struct file_operations proc_sched_latency_fops = {
       .read = sched_latency_read,
       .write = sched_latency_write,
};

static ssize_t iowait_read(struct file *filp, char __user *buff, size_t count, loff_t *off)
{
        char page[1024] = {0};
        int len = 0;

        len = sprintf(page, "iowait_low_thresh_ms: %d\n""iowait_high_thresh_ms: %d\n"
                "iowait_low_cnt: %lld\n""iowait_high_cnt: %lld\n"
                "iowait_total_ms: %lld\n""iowait_total_cnt: %lld\n"
                "iowait_fg_low_cnt: %lld\n""iowait_fg_high_cnt: %lld\n"
                "iowait_fg_total_ms: %lld\n""iowait_fg_total_cnt: %lld\n"
                "iowait_delta_ms: %lld\n",
                iowait_para.low_thresh_ms, iowait_para.high_thresh_ms,
                iowait_para.low_cnt, iowait_para.high_cnt,
                iowait_para.total_ms, iowait_para.total_cnt,
                iowait_para.fg_low_cnt, iowait_para.fg_high_cnt,
                iowait_para.fg_total_ms, iowait_para.fg_total_cnt,
                iowait_para.delta_ms);

        if (len > *off) {
                len -= *off;
        } else {
                len = 0;
        }
        if (copy_to_user(buff, page, (len < count ? len : count))) {
                return -EFAULT;
        }
        *off += len < count ? len : count;
        return (len < count ? len : count);
}

static ssize_t iowait_thresh_write(struct file *file, const char __user *buff, size_t len, loff_t *ppos)
{
	char write_data[32] = {0};
        char low_thresh[LOW_THRESH_LEN_MAX] = {0};
        char high_thresh[HIGH_THRESH_LEN_MAX] = {0};
        int temp_low = 0;
        int temp_high = 0;
        int low_len = 0;
        int high_len = 0;
        int i = 0;

	if (copy_from_user(&write_data, buff, len)) {
		health_err("iowait_thresh write error.\n");
		return -EFAULT;
	}
        write_data[len] = '\0';
        if (write_data[len - 1] == '\n') {
                write_data[len - 1] = '\0';
        }
        health_debug("input %s, %d\n", write_data, (int)len);

        if ((int)len > WAIT_PARAM_MAX || (int)len < WAIT_PARAM_MIN) {
                health_err("Para Len Err, len = %d\n", (int)len);
                goto THRESH_WRITE_ILLEGAL;
        }

        for (i = 0; i < len; i++){
                if (write_data[i] == ':') {
                        low_len = i+1;
                        if (low_len <= LOW_THRESH_LEN_MAX) {
                                strncpy(low_thresh, write_data, low_len);
                                low_thresh[i] = '\0';
                                high_len = len - low_len;
                                health_debug("low thresh %s, %d\n", low_thresh, low_len);
                                if (high_len <= HIGH_THRESH_LEN_MAX) {
                                        strncpy(high_thresh, (write_data + low_len), high_len);
                                        health_debug("high thresh %s, %d\n", high_thresh, high_len);
                                        if ((strspn(low_thresh,"0123456789") == strlen(low_thresh)) &&
                                                (strspn(high_thresh,"0123456789") == strlen(high_thresh)))
                                                break;
                                }
                        } else {
                                health_err("low thresh too long");
                        }
                        goto THRESH_WRITE_ILLEGAL;
                }
        }
        temp_low = (int)simple_strtol(low_thresh, NULL, 10);
        temp_high = (int)simple_strtol(high_thresh, NULL, 10);
        if (temp_low >= temp_high || temp_low < LOW_THRESH_MS_LOW || temp_low > LOW_THRESH_MS_HIGH ||
                temp_high < HIGH_THRESH_MS_LOW || temp_high > HIGH_THRESH_MS_HIGH) {
                /* Input Thresh Illegal */
                goto THRESH_WRITE_ILLEGAL;
        }
        iowait_para.high_thresh_ms = temp_high;
        iowait_para.low_thresh_ms = temp_low;

        health_debug("USR SPACE Update iowait ms Thresh: %d, %d\n",
                iowait_para.low_thresh_ms, iowait_para.high_thresh_ms);
	return len;

THRESH_WRITE_ILLEGAL:
        health_err("Para input illegal\n");
        return -EFAULT;
}

static const struct file_operations proc_iowait_fops = {
       .read = iowait_read,
       .write = iowait_thresh_write,
};
#endif /* CONFIG_SCHEDSTATS */
#ifdef CONFIG_OPPO_MEM_MONITOR
extern bool mem_monitor_enable;         /* mem_monitor switch */
static ssize_t alloc_wait_read(struct file *filp, char __user *buff, size_t count, loff_t *off)
{
        char page[1024] = {0};
        int len = 0;

        len = sprintf(page, "total_alloc_wait_h_cnt: %lld\n""total_alloc_wait_l_cnt: %lld\n"
                "fg_alloc_wait_h_cnt: %lld\n""fg_alloc_wait_l_cnt: %lld\n"
                "total_alloc_wait_max_ms: %lld\n""total_alloc_wait_max_order: %lld\n"
                "fg_alloc_wait_max_ms: %lld\n""fg_alloc_wait_max_order: %lld\n",
                allocwait_para.total_alloc_wait_h_cnt,allocwait_para.total_alloc_wait_l_cnt,
                allocwait_para.fg_alloc_wait_h_cnt,allocwait_para.fg_alloc_wait_l_cnt,
                allocwait_para.total_alloc_wait_max_ms,allocwait_para.total_alloc_wait_max_order,
                allocwait_para.fg_alloc_wait_max_ms,allocwait_para.fg_alloc_wait_max_order);

        if (len > *off) {
                len -= *off;
        } else {
                len = 0;
        }
        if (copy_to_user(buff, page, (len < count ? len : count))) {
                return -EFAULT;
        }
        *off += len < count ? len : count;
        return (len < count ? len : count);
}

static const struct file_operations proc_alloc_wait_fops = {
       .read = alloc_wait_read,
};
#endif /*CONFIG_OPPO_MEM_MONITOR*/

static int is_mem_monitor_enable(void)
{
#ifdef CONFIG_OPPO_MEM_MONITOR
        return (mem_monitor_enable ? 1:0);
#else
        return 0;
#endif
}

extern bool ion_cnt_enable;             /* Ion_cnt switch */
static ssize_t total_switch_stats_read(struct file *filp, char __user *buff, size_t count, loff_t *off)
{
        char page[1024] = {0};
        int len = 0;

        len = sprintf(page, "Ion_cnt_switch: %d\n"
                "Mem_monitor_switch: %d\n",
                ion_cnt_enable ? 1:0, is_mem_monitor_enable());

        if (len > *off) {
                len -= *off;
        } else {
                len = 0;
        }
        if (copy_to_user(buff, page, (len < count ? len : count))) {
                return -EFAULT;
        }
        *off += len < count ? len : count;
        return (len < count ? len : count);
}

static const struct file_operations proc_total_switch_stats_fops = {
       .read = total_switch_stats_read,
};

/* iowait hung show */
static ssize_t iowait_hung_read(struct file *filp, char __user *buff, size_t count, loff_t *off)
{
        char page[1024] = {0};
        int len = 0;

        len = sprintf(page, "iowait_hung_cnt: %u\n""iowait_panic_cnt: %u\n""iowait_hung_ctrl: %s\n",
                iowait_hung_cnt, iowait_panic_cnt, iowait_hung_ctrl ? "true":"false");

        if (len > *off) {
                len -= *off;
        } else {
                len = 0;
        }
        if (copy_to_user(buff, page, (len < count ? len : count))) {
                return -EFAULT;
        }
        *off += len < count ? len : count;
        return (len < count ? len : count);
}

static const struct file_operations proc_iowait_hung_fops = {
       .read = iowait_hung_read,
};

#define HEALTHINFO_PROC_NODE "oppo_healthinfo"
static int __init oppo_healthinfo_init(void)
{
	int ret = 0;
	struct proc_dir_entry *pentry;

        oppo_healthinfo =  proc_mkdir(HEALTHINFO_PROC_NODE, NULL);
        if(!oppo_healthinfo) {
		health_err("can't create oppo_healthinfo proc\n");
		goto ERROR_INIT_VERSION;
	}

        pentry = proc_create("healthinfo_switch", S_IRUGO | S_IWUGO, oppo_healthinfo, &proc_switch_fops);
	if(!pentry) {
		health_err("create healthinfo_switch proc failed.\n");
		goto ERROR_INIT_VERSION;
	}

        pentry = proc_create("total_switch_stats", S_IRUGO, oppo_healthinfo, &proc_total_switch_stats_fops);
        if(!pentry) {
                health_err("create total_switch_stats proc failed.\n");
                goto ERROR_INIT_VERSION;
        }

        pentry = proc_create("cpu_loading", S_IRUGO, oppo_healthinfo, &proc_cpu_load_fops);
	if(!pentry) {
		health_err("create cpu_loading proc failed.\n");
		goto ERROR_INIT_VERSION;
	}

        pentry = proc_create("iowait_hung", S_IRUGO, oppo_healthinfo, &proc_iowait_hung_fops);
	if(!pentry) {
		health_err("create iowait_hung proc failed.\n");
		goto ERROR_INIT_VERSION;
	}

#ifdef CONFIG_SCHEDSTATS
        pentry = proc_create("iowait", S_IRUGO | S_IWUGO, oppo_healthinfo, &proc_iowait_fops);
	if(!pentry) {
		health_err("create iowait proc failed.\n");
		goto ERROR_INIT_VERSION;
	}
        pentry = proc_create("sched_latency", S_IRUGO | S_IWUGO, oppo_healthinfo, &proc_sched_latency_fops);
	if(!pentry) {
		health_err("create sched_latency proc failed.\n");
		goto ERROR_INIT_VERSION;
	}
	force_schedstat_enabled();
#endif /* CONFIG_SCHEDSTATS */
#ifdef CONFIG_OPPO_MEM_MONITOR
	pentry = proc_create("alloc_wait", S_IRUGO, oppo_healthinfo, &proc_alloc_wait_fops);
	if(!pentry) {
		health_err("create alloc_wait proc failed.\n");
		goto ERROR_INIT_VERSION;
	}

#endif /*CONFIG_OPPO_MEM_MONITOR*/
        health_debug("Success \n");
	return ret;
ERROR_INIT_VERSION:
	remove_proc_entry(HEALTHINFO_PROC_NODE, NULL);
	return -ENOENT;
}

arch_initcall(oppo_healthinfo_init);

MODULE_DESCRIPTION("OPPO healthinfo");
MODULE_LICENSE("GPL v2");
