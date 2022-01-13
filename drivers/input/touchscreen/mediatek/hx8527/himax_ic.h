/* Himax Android Driver Sample Code for HX852xES chipset
*
* Copyright (C) 2017 Himax Corporation.
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
#ifndef H_HIMAX_IC
#define H_HIMAX_IC

#include "himax_platform.h"
#include "himax_common.h"

#include <linux/slab.h>

#define HIMAX_REG_RETRY_TIMES 5

#define HX_CMD_NOP					 0x00
#define HX_CMD_SETMICROOFF			 0x35
#define HX_CMD_SETROMRDY			 0x36
#define HX_CMD_TSSLPIN				 0x80
#define HX_CMD_TSSLPOUT 			 0x81
#define HX_CMD_TSSOFF				 0x82
#define HX_CMD_TSSON				 0x83
#define HX_CMD_ROE					 0x85
#define HX_CMD_RAE					 0x86
#define HX_CMD_RLE					 0x87
#define HX_CMD_CLRES				 0x88
#define HX_CMD_TSSWRESET			 0x9E
#define HX_CMD_SETDEEPSTB			 0xD7
#define HX_CMD_SET_CACHE_FUN		 0xDD
#define HX_CMD_SETIDLE				 0xF2
#define HX_CMD_SETIDLEDELAY 		 0xF3
#define HX_CMD_SELFTEST_BUFFER		 0x8D
#define HX_CMD_MANUALMODE			 0x42
#define HX_CMD_FLASH_ENABLE 		 0x43
#define HX_CMD_FLASH_SET_ADDRESS	 0x44
#define HX_CMD_FLASH_WRITE_REGISTER  0x45
#define HX_CMD_FLASH_SET_COMMAND	 0x47
#define HX_CMD_FLASH_WRITE_BUFFER	 0x48
#define HX_CMD_FLASH_PAGE_ERASE 	 0x4D
#define HX_CMD_FLASH_SECTOR_ERASE	 0x4E
#define HX_CMD_CB					 0xCB
#define HX_CMD_EA					 0xEA
#define HX_CMD_4A					 0x4A
#define HX_CMD_4F					 0x4F
#define HX_CMD_B9					 0xB9
#define HX_CMD_76					 0x76
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 add for hx8527*/
#define HX_CMD_OPPO_SP				 0xF3

#define HX_VER_FW_MAJ				 0x33
#define HX_VER_FW_MIN				 0x32
#define HX_VER_FW_CFG				 0x39

#ifdef HX_ESD_RECOVERY
extern u8 HX_ESD_RESET_ACTIVATE;
#endif
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 add for hx8527*/
extern char **g_hx_crtra_result;
extern int32_t g_hx_crtra_str_count;
extern char *g_hx_inspt_crtra_name[];
extern int himax_self_test_data_init(void);
extern void himax_self_test_data_deinit(void);
extern void deinit_criteria_file_ds(void);
extern int himax_get_criteria_size(void);
#define CRITERIA_ITEM_STR_MAX (128)

enum fw_image_type
{
    fw_image_32k	= 0x01,
    fw_image_60k,
    fw_image_64k,
    fw_image_124k,
    fw_image_128k,
};
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/17 add for hx8527*/
void himax_tp_direction_set(uint8_t current_state);
int himax_hand_shaking(struct i2c_client *client);
void himax_set_SMWP_enable(struct i2c_client *client,uint8_t SMWP_enable, bool suspended);
void himax_set_HSEN_enable(struct i2c_client *client,uint8_t HSEN_enable, bool suspended);
void himax_usb_detect_set(struct i2c_client *client,uint8_t *cable_config);
int himax_determin_diag_rawdata(int diag_command);
int himax_determin_diag_storage(int diag_command);
void himax_diag_register_set(struct i2c_client *client, uint8_t diag_command);
void himax_flash_dump_func(struct i2c_client *client, uint8_t local_flash_command, int Flash_Size, uint8_t *flash_buffer);
int himax_chip_self_test(struct i2c_client *client);
void himax_burst_enable(struct i2c_client *client, uint8_t auto_add_4_byte);
void himax_register_read(struct i2c_client *client, uint8_t *read_addr, int read_length, uint8_t *read_data, bool cfg_flag);
void himax_flash_read(struct i2c_client *client, uint8_t *reg_byte, uint8_t *read_data);
void himax_flash_write_burst(struct i2c_client *client, uint8_t * reg_byte, uint8_t * write_data);
void himax_flash_write_burst_lenth(struct i2c_client *client, uint8_t *reg_byte, uint8_t *write_data, int length);
void himax_register_write(struct i2c_client *client, uint8_t *write_addr, int write_length, uint8_t *write_data, bool cfg_flag);
void himax_sense_off(struct i2c_client *client);
void himax_interface_on(struct i2c_client *client);
bool wait_wip(struct i2c_client *client, int Timing);
void himax_sense_on(struct i2c_client *client, uint8_t FlashMode);
void himax_chip_erase(struct i2c_client *client);
bool himax_block_erase(struct i2c_client *client);
bool himax_sector_erase(struct i2c_client *client, int start_addr);
void himax_sram_write(struct i2c_client *client, uint8_t *FW_content);
bool himax_sram_verify(struct i2c_client *client, uint8_t *FW_File, int FW_Size);
void himax_flash_programming(struct i2c_client *client, uint8_t *FW_content, int FW_Size);
int fts_ctpm_fw_upgrade_with_sys_fs_32k(struct i2c_client *client, unsigned char *fw, int len, bool change_iref);
int fts_ctpm_fw_upgrade_with_sys_fs_60k(struct i2c_client *client, unsigned char *fw, int len, bool change_iref);
int fts_ctpm_fw_upgrade_with_sys_fs_64k(struct i2c_client *client, unsigned char *fw, int len, bool change_iref);
int fts_ctpm_fw_upgrade_with_sys_fs_124k(struct i2c_client *client, unsigned char *fw, int len, bool change_iref);
int fts_ctpm_fw_upgrade_with_sys_fs_128k(struct i2c_client *client, unsigned char *fw, int len, bool change_iref);
void himax_touch_information(struct i2c_client *client);
int himax_read_i2c_status(struct i2c_client *client);
int  himax_read_ic_trigger_type(struct i2c_client *client);
void himax_read_FW_ver(struct i2c_client *client);
bool himax_ic_package_check(struct i2c_client *client);
void himax_power_on_init(struct i2c_client *client);
int cal_data_len(int raw_cnt_rmd, int HX_MAX_PT, int raw_cnt_max);
bool himax_read_event_stack(struct i2c_client *client, uint8_t *buf_ts, int length);
void himax_get_DSRAM_data(struct i2c_client *client, uint8_t *info_data);
bool diag_check_sum(struct himax_report_data *hx_touch_data); //return checksum value
void himax_get_raw_data(uint8_t diag_command, uint16_t mutual_num, uint16_t self_num);
void diag_parse_raw_data(struct himax_report_data *hx_touch_data,int mul_num, int self_num,uint8_t diag_cmd, int32_t *mutual_data, int32_t *self_data);
bool himax_calculateChecksum(struct i2c_client *client, bool change_iref);
bool himax_flash_lastdata_check(struct i2c_client *client);
uint8_t himax_read_DD_status(uint8_t *cmd_set, uint8_t *tmp_data);
int himax_read_FW_status(uint8_t *state_addr, uint8_t *tmp_addr);
void himax_resume_ic_action(struct i2c_client *client);
void himax_suspend_ic_action(struct i2c_client *client);
void himax_ic_reset(uint8_t loadconfig,uint8_t int_off);
int himax_fw_ver_bin(unsigned char *i_CTPM_FW);

#endif
