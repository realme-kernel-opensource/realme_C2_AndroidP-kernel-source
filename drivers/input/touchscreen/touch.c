/***************************************************
 * File:touch.c
 * VENDOR_EDIT
 * Copyright (c)  2008- 2030  Oppo Mobile communication Corp.ltd.
 * Description:
 *             tp dev
 * Version:1.0:
 * Date created:2016/09/02
 * Author: hao.wang@Bsp.Driver
 * TAG: BSP.TP.Init
*/

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/serio.h>
#include "oppo_touchscreen/tp_devices.h"
#include "oppo_touchscreen/touchpanel_common.h"
#include <soc/oppo/oppo_project.h>
#include "touch.h"

#define MAX_LIMIT_DATA_LENGTH         100

#define TD4330_NF_CHIP_NAME "TD4330_NF"
/*if can not compile success, please update vendor/oppo_touchsreen*/
struct tp_dev_name tp_dev_names[] = {
     {TP_OFILM, "OFILM"},
     {TP_BIEL, "BIEL"},
     {TP_TRULY, "TRULY"},
     {TP_BOE, "BOE"},
     {TP_G2Y, "G2Y"},
     {TP_TPK, "TPK"},
     {TP_JDI, "JDI"},
     {TP_TIANMA, "TIANMA"},
     {TP_SAMSUNG, "SAMSUNG"},
     {TP_DSJM, "DSJM"},
     {TP_BOE_B8, "BOEB8"},
     {TP_INNOLUX, "INNOLUX"},
     {TP_HIMAX_DPT, "DPT"},
     {TP_AUO, "AUO"},
     {TP_DEPUTE, "DEPUTE"},
     {TP_UNKNOWN, "UNKNOWN"},
};

#define GET_TP_DEV_NAME(tp_type) ((tp_dev_names[tp_type].type == (tp_type))?tp_dev_names[tp_type].name:"UNMATCH")

int g_tp_dev_vendor = TP_UNKNOWN;
char *g_tp_chip_name;
static bool is_tp_type_got_in_match = false;    /*indicate whether the tp type is got in the process of ic match*/

/*
* this function is used to judge whether the ic driver should be loaded
* For incell module, tp is defined by lcd module, so if we judge the tp ic
* by the boot command line of containing lcd string, we can also get tp type.
*/
bool __init tp_judge_ic_match(char * tp_ic_name)
{
    pr_err("[TP] tp_ic_name = %s \n", tp_ic_name);
    pr_err("[TP] boot_command_line = %s \n", boot_command_line);

    //switch(get_project()) {
    //case 18151:
        is_tp_type_got_in_match = true;
        if (strstr(tp_ic_name, "td4320_nf") && strstr(boot_command_line, "dsjm_jdi_td4330")) {
            g_tp_dev_vendor = TP_DSJM;
            #ifdef CONFIG_TOUCHPANEL_MULTI_NOFLASH
            g_tp_chip_name = kzalloc(sizeof(TD4330_NF_CHIP_NAME), GFP_KERNEL);
            g_tp_chip_name = TD4330_NF_CHIP_NAME;
            #endif
            return true;
        }
        if (strstr(tp_ic_name, "td4320_nf") && strstr(boot_command_line, "dpt_jdi_td4330")) {
            g_tp_dev_vendor = TP_HIMAX_DPT;
            #ifdef CONFIG_TOUCHPANEL_MULTI_NOFLASH
            g_tp_chip_name = kzalloc(sizeof(TD4330_NF_CHIP_NAME), GFP_KERNEL);
            g_tp_chip_name = TD4330_NF_CHIP_NAME;
            #endif
            return true;
        }
    //default:
    //    pr_err("Invalid project\n");
    //    break;
    //}

    pr_err("[TP]Lcd module not found\n");
    return false;
}

/*
* For separate lcd and tp, tp can be distingwished by gpio pins
* different project may have different combination, if needed,
* add your combination with project distingwished by is_project function.
*/
static void tp_get_vendor_via_pin(struct hw_resource *hw_res, struct panel_info *panel_data)
{
    panel_data->tp_type = TP_UNKNOWN;
    return;
}

/*
* If no gpio pins used to distingwish tp module, maybe have other ways(like command line)
* add your way of getting vendor info with project distingwished by is_project function.
*/
static void tp_get_vendor_separate(struct hw_resource *hw_res, struct panel_info *panel_data)
{
    panel_data->tp_type = TP_UNKNOWN;
    return;
}

int tp_util_get_vendor(struct hw_resource *hw_res, struct panel_info *panel_data)
{
    char* vendor;

    #ifdef CONFIG_TOUCHPANEL_MULTI_NOFLASH
    if (g_tp_chip_name != NULL) {
        panel_data->chip_name = g_tp_chip_name;
    }
    #endif
    panel_data->test_limit_name = kzalloc(MAX_LIMIT_DATA_LENGTH, GFP_KERNEL);
    if (panel_data->test_limit_name == NULL) {
        pr_err("[TP]panel_data.test_limit_name kzalloc error\n");
    }

    panel_data->extra= kzalloc(MAX_LIMIT_DATA_LENGTH, GFP_KERNEL);
    if (panel_data->extra == NULL) {
        pr_err("[TP]panel_data.test_limit_name kzalloc error\n");
    }

    /*TP is first distingwished by gpio pins, and then by other ways*/
    if (is_tp_type_got_in_match) {
        panel_data->tp_type = g_tp_dev_vendor;

        if(is_project(OPPO_18151)) {
            if (strstr(boot_command_line, "dsjm_jdi_td4330")) {
                memcpy(panel_data->manufacture_info.version, "0xDD075E", 8);
            }
            if (strstr(boot_command_line, "dpt_jdi_td4330")) {
                memcpy(panel_data->manufacture_info.version, "0xDD075D", 8);
            }
        }
    } else if (gpio_is_valid(hw_res->id1_gpio) || gpio_is_valid(hw_res->id2_gpio) || gpio_is_valid(hw_res->id3_gpio)) {
        tp_get_vendor_via_pin(hw_res, panel_data);
    } else {
        tp_get_vendor_separate(hw_res, panel_data);
    }

    if (panel_data->tp_type == TP_UNKNOWN) {
        pr_err("[TP]%s type is unknown\n", __func__);
        return 0;
    }

    vendor = GET_TP_DEV_NAME(panel_data->tp_type);

    strcpy(panel_data->manufacture_info.manufacture, vendor);
    snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
            "tp/%d/FW_%s_%s.img",
            get_project(), panel_data->chip_name, vendor);

    if (panel_data->test_limit_name) {
        snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
            "tp/%d/LIMIT_%s_%s.img",
            get_project(), panel_data->chip_name, vendor);
    }

    if (panel_data->extra) {
       snprintf(panel_data->extra, MAX_LIMIT_DATA_LENGTH,
            "tp/%d/BOOT_FW_%s_%s.ihex",
            get_project(), panel_data->chip_name, vendor);
    } 
	panel_data->manufacture_info.fw_path = panel_data->fw_name;

    switch(get_project()) {
    case OPPO_18151:
	    if (strstr(boot_command_line, "dsjm_jdi_td4330")) {
                panel_data->firmware_headfile.firmware_data = FW_18151_TD4330_NF_DSJM;
                panel_data->firmware_headfile.firmware_size = sizeof(FW_18151_TD4330_NF_DSJM);
        } else if (strstr(boot_command_line, "dpt_jdi_td4330")) {
                panel_data->firmware_headfile.firmware_data = FW_18151_TD4330_NF_DPT;
                panel_data->firmware_headfile.firmware_size = sizeof(FW_18151_TD4330_NF_DPT);
        } else {
            panel_data->firmware_headfile.firmware_data = NULL;
            panel_data->firmware_headfile.firmware_size = 0;
        }
        break;

    default:
        panel_data->firmware_headfile.firmware_data = NULL;
        panel_data->firmware_headfile.firmware_size = 0;
    }

    pr_info("Vendor:%s\n", vendor);
    pr_info("Fw:%s\n", panel_data->fw_name);
    pr_info("Limit:%s\n", panel_data->test_limit_name==NULL?"NO Limit":panel_data->test_limit_name);
    pr_info("Extra:%s\n", panel_data->extra==NULL?"NO Extra":panel_data->extra);
    pr_info("is matched %d, type %d\n", is_tp_type_got_in_match, panel_data->tp_type);
    return 0;
}
