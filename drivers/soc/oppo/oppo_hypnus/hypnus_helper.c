/*
 * Copyright (C) 2016 OPPO, Inc.
 * Author: Jie Cheng <jie.cheng@oppo.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/power_supply.h>
#include <soc/oppo/hypnus_helper.h>

extern unsigned int upmu_get_rgs_chrdet(void);

int hypnus_get_batt_capacity()
{
	union power_supply_propval ret = {0, };
	static struct power_supply *batt_psy;

	if (batt_psy == NULL)
		batt_psy = power_supply_get_by_name("battery");
	if (batt_psy && batt_psy->desc->get_property)
		batt_psy->desc->get_property(batt_psy, POWER_SUPPLY_PROP_CAPACITY, &ret);
	if (ret.intval >= 0 && ret.intval <=100)
		return ret.intval;
	else
		return DEFAULT_CAPACITY;
}
EXPORT_SYMBOL(hypnus_get_batt_capacity);

bool hypnus_get_charger_status()
{
	if (upmu_get_rgs_chrdet())
		return true;
	else
		return false;
}
EXPORT_SYMBOL(hypnus_get_charger_status);

bool hypnus_is_release()
{
#ifdef CONFIG_MT_ENG_BUILD
	return false;
#else
	return true;
#endif
}
EXPORT_SYMBOL(hypnus_is_release);

extern int get_heavy_task_threshold(void);
unsigned int sched_get_heavy_task_threshold(void)
{
       return  get_heavy_task_threshold();
}
EXPORT_SYMBOL(sched_get_heavy_task_threshold);

