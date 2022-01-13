/**********************************************************************************
* Copyright (c)  2008-2015  Guangdong OPPO Mobile Comm Corp., Ltd
* VENDOR_EDIT
* Description: Charger IC management module for charger system framework.
*                          Manage all charger IC and define abstarct function flow.
* Version   : 1.0
* Date       : 2015-06-22
* Author     : fanhui@PhoneSW.BSP
* ------------------------------ Revision History: --------------------------------
* <version>           <date>                <author>                            <desc>
* Revision 1.0        2018-05-24       wenbin.liu@PSW.Kernel.MM       Created for Healthinfomonitor
***********************************************************************************/

#ifndef _OPPO_HEALTHINFO_H_
#define _OPPO_HEALTHINFO_H_

#include <linux/latencytop.h>
#include <linux/sched.h>
#include <linux/cpumask.h>
#include <linux/cpuidle.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#ifdef CONFIG_OPPO_MEM_MONITOR
#include <linux/memory_monitor.h>
#endif /*CONFIG_OPPO_MEM_MONITOR*/

#define health_err(fmt, ...) \
        printk(KERN_ERR "[HM_ERR][%s]"fmt, __func__, ##__VA_ARGS__)
#define health_debug(fmt, ...) \
        printk(KERN_INFO "[HM_INFO][%s]"fmt, __func__, ##__VA_ARGS__)

#ifdef CONFIG_SCHEDSTATS
struct wait_para {
        int low_thresh_ms;
        int high_thresh_ms;
        u64 low_cnt;
        u64 high_cnt;
        u64 total_ms;
        u64 total_cnt;
        u64 fg_low_cnt;
        u64 fg_high_cnt;
        u64 fg_total_ms;
        u64 fg_total_cnt;
        u64 delta_ms;
};
extern struct wait_para iowait_para;
extern struct wait_para sched_latency_para;
#endif /* CONFIG_SCHEDSTATS */

bool oppo_healthinfo_switch = false;
extern int healthinfo_get_cur_cpuload(bool para_switch);
#endif /* _OPPO_HEALTHINFO_H_*/
