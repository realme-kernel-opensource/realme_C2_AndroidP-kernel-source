/************************************************************************************
** Copyright (C), 2008-2017, OPPO Mobile Comm Corp., Ltd
** VENDOR_EDIT
** File: motor.c
**
** Description:
**	Definitions for oppo_motor common software.
**
** Version: 1.0
** Date created: 2018/01/14,20:27
** Author: Fei.Mo@PSW.BSP.Sensor
**
** --------------------------- Revision History: ------------------------------------
* <version>		<date>		<author>		<desc>
* Revision 1.0		2018/01/14	Fei.Mo@PSW.BSP.Sensor	Created
**************************************************************************************/
#include <linux/delay.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/serio.h>
#include <soc/oppo/oppo_project.h>
#include "oppo_motor/oppo_motor.h"
#include "oppo_motor/oppo_motor_notifier.h"

/*if can not compile success, please update vendor/oppo_motor*/

void oppo_parse_motor_info(struct oppo_motor_chip * chip)
{
	if (!chip)
		return;

	chip->motor_type = MOTOR_FI5;
	chip->motor_ic = STSPIN220;
	chip->dir_sign = POSITIVE;
	chip->is_support_ffd = false;
}
EXPORT_SYMBOL(oppo_parse_motor_info);

