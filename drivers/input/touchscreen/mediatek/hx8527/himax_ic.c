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

#include "himax_ic.h"

extern struct himax_ts_data *private_ts;
static int iref_number = 11;
static bool iref_found = false;

extern int i2c_error_count;

#ifdef HX_CHIP_STATUS_MONITOR
extern struct chip_monitor_data *g_chip_monitor_data;
#endif
#if defined(HX_TP_SELF_TEST_DRIVER) || defined(CONFIG_TOUCHSCREEN_HIMAX_ITO_TEST)
uint8_t test_counter = 0;
uint8_t TEST_DATA_TIMES = 3;
#endif

//1uA
static unsigned char E_IrefTable_1[16][2] = { {0x20,0x0F},{0x20,0x1F},{0x20,0x2F},{0x20,0x3F},
    {0x20,0x4F},{0x20,0x5F},{0x20,0x6F},{0x20,0x7F},
    {0x20,0x8F},{0x20,0x9F},{0x20,0xAF},{0x20,0xBF},
    {0x20,0xCF},{0x20,0xDF},{0x20,0xEF},{0x20,0xFF}
};

//2uA
static unsigned char E_IrefTable_2[16][2] = { {0xA0,0x0E},{0xA0,0x1E},{0xA0,0x2E},{0xA0,0x3E},
    {0xA0,0x4E},{0xA0,0x5E},{0xA0,0x6E},{0xA0,0x7E},
    {0xA0,0x8E},{0xA0,0x9E},{0xA0,0xAE},{0xA0,0xBE},
    {0xA0,0xCE},{0xA0,0xDE},{0xA0,0xEE},{0xA0,0xFE}
};

//3uA
static unsigned char E_IrefTable_3[16][2] = { {0x20,0x0E},{0x20,0x1E},{0x20,0x2E},{0x20,0x3E},
    {0x20,0x4E},{0x20,0x5E},{0x20,0x6E},{0x20,0x7E},
    {0x20,0x8E},{0x20,0x9E},{0x20,0xAE},{0x20,0xBE},
    {0x20,0xCE},{0x20,0xDE},{0x20,0xEE},{0x20,0xFE}
};

//4uA
static unsigned char E_IrefTable_4[16][2] = { {0xA0,0x0D},{0xA0,0x1D},{0xA0,0x2D},{0xA0,0x3D},
    {0xA0,0x4D},{0xA0,0x5D},{0xA0,0x6D},{0xA0,0x7D},
    {0xA0,0x8D},{0xA0,0x9D},{0xA0,0xAD},{0xA0,0xBD},
    {0xA0,0xCD},{0xA0,0xDD},{0xA0,0xED},{0xA0,0xFD}
};

//5uA
static unsigned char E_IrefTable_5[16][2] = { {0x20,0x0D},{0x20,0x1D},{0x20,0x2D},{0x20,0x3D},
    {0x20,0x4D},{0x20,0x5D},{0x20,0x6D},{0x20,0x7D},
    {0x20,0x8D},{0x20,0x9D},{0x20,0xAD},{0x20,0xBD},
    {0x20,0xCD},{0x20,0xDD},{0x20,0xED},{0x20,0xFD}
};

//6uA
static unsigned char E_IrefTable_6[16][2] = { {0xA0,0x0C},{0xA0,0x1C},{0xA0,0x2C},{0xA0,0x3C},
    {0xA0,0x4C},{0xA0,0x5C},{0xA0,0x6C},{0xA0,0x7C},
    {0xA0,0x8C},{0xA0,0x9C},{0xA0,0xAC},{0xA0,0xBC},
    {0xA0,0xCC},{0xA0,0xDC},{0xA0,0xEC},{0xA0,0xFC}
};

//7uA
static unsigned char E_IrefTable_7[16][2] = { {0x20,0x0C},{0x20,0x1C},{0x20,0x2C},{0x20,0x3C},
    {0x20,0x4C},{0x20,0x5C},{0x20,0x6C},{0x20,0x7C},
    {0x20,0x8C},{0x20,0x9C},{0x20,0xAC},{0x20,0xBC},
    {0x20,0xCC},{0x20,0xDC},{0x20,0xEC},{0x20,0xFC}
};

extern unsigned long	FW_VER_MAJ_FLASH_ADDR;
extern unsigned long 	FW_VER_MIN_FLASH_ADDR;
extern unsigned long 	FW_CFG_VER_FLASH_ADDR;

extern unsigned long 	FW_VER_MAJ_FLASH_LENG;
extern unsigned long 	FW_VER_MIN_FLASH_LENG;

#ifdef HX_AUTO_UPDATE_FW
extern unsigned long 	CFG_VER_MAJ_FLASH_ADDR;
extern unsigned long 	CFG_VER_MIN_FLASH_ADDR;
extern unsigned long 	CID_VER_MAJ_FLASH_ADDR;
extern unsigned long 	CID_VER_MIN_FLASH_ADDR;
extern int g_i_FW_VER;
extern int g_i_CFG_VER;
extern int g_i_CID_MAJ;
extern int g_i_CID_MIN;
#endif

extern unsigned char	IC_TYPE;
extern unsigned char	IC_CHECKSUM;

extern struct himax_ic_data* ic_data;

extern uint8_t getFlashCommand(void);

extern void setFlashDumpComplete(uint8_t complete);
extern void setFlashDumpFail(uint8_t fail);
extern void setFlashDumpProgress(uint8_t progress);
extern void setSysOperation(uint8_t operation);
extern void setFlashDumpGoing(bool going);

extern uint8_t getXChannel(void);
extern uint8_t getYChannel(void);

#if defined(HX_TP_SELF_TEST_DRIVER)
extern char *g_hx_crtra_file;
extern char *g_test_ok_file;
extern char *g_test_ng_file;
extern char *g_sensor_result_file;
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 modified for ito*/
extern char *g_sensor_black_result_file;
extern bool oppo_hx_black_screen_test_flag;
/* ASCII format */
#define ASCII_LF	(0x0A)
#define ASCII_CR	(0x0D)
#define ASCII_COMMA	(0x2C)
#define ASCII_ZERO	(0x30)
#define CHAR_EL	'\0'
#define CHAR_NL	'\n'
#define ACSII_SPACE	(0x20)
/* for criteria */
int HX_CRITERIA_ITEM;
int HX_CRITERIA_SIZE;
int **g_inspection_criteria;
char *g_hx_inspt_crtra_name[] = {
	"CRITERIA_BNK_ULMT",
	"CRITERIA_BNK_DLMT",
	"CRITERIA_AVG_BNK_ULMT",
	"CRITERIA_AVG_BNK_DLMT",
	"CRITERIA_SLF_BNK_ULMT",
	"CRITERIA_SLF_BNK_DLMT",
};

/* Error code of Inspection */
typedef enum {
	HX_INSPECT_OK = 0,    /* OK */
	HX_INSPECT_ESPI,      /* SPI communication error */
	HX_BNK_FAIL,          /* Self test bank fail */
	HX_AVG_BNK_FAIL,      /* Self test avg bank fail */
	HX_SLF_BNK_FAIL,      /* Self test slf bank fail */
	HX_INSPECT_EFILE,     /* Criteria file error */
	HX_INSPECT_EOTHER,    /* All other errors */
} HX_INSPECT_ERR_ENUM;
#endif

#if defined(HX_TP_SELF_TEST_DRIVER) || defined(CONFIG_TOUCHSCREEN_HIMAX_ITO_TEST)
static int Selftest_flag;
uint16_t *mutual_bank;
uint16_t *self_bank;
extern int g_diag_command;
bool raw_data_chk_arr[20] = {false};
#endif

#ifdef HX_TP_PROC_2T2R
bool Is_2T2R = false;
int HX_2T2R_Addr = 0x96;//Need to check with project FW eng.
int HX_2T2R_en_setting = 0x02;//Need to check with project FW eng.
#endif

#ifdef HX_USB_DETECT_GLOBAL
//extern bool upmu_is_chr_det(void);
extern void himax_cable_detect_func(bool force_renew);
#endif
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/02/12 add for headset mode*/
extern void himax_headset_detect_func(bool force_renew);

extern int himax_loadSensorConfig(struct i2c_client *client, struct himax_i2c_platform_data *pdata);

#if defined(HX_ESD_RECOVERY)
u8 ESD_R36_FAIL = 0;

#endif

uint8_t	IC_STATUS_CHECK = 0xAA;
int himax_touch_data_size = 128;

int himax_get_touch_data_size(void)
{
    return himax_touch_data_size;
}

#ifdef HX_RST_PIN_FUNC
extern u8 HX_HW_RESET_ACTIVATE;
extern int himax_report_data_init(void);
extern int calculate_point_number(void);
extern void himax_rst_gpio_set(int pinnum, uint8_t value);

/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/17 add for hx8527*/
void himax_tp_direction_set(uint8_t current_state)
{
    uint8_t reg_addr = HX_CMD_OPPO_SP;
    himax_register_write(private_ts->client, &reg_addr, 1, &current_state, 0);
}

/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 add for hx8527*/
void himax_clear_reg_status(void)
{
    private_ts->oppo_sp_reg_state = 0;
    private_ts->oppo_sp_en = OPPO_SET_PORTRAIT(1, 0);
    //private_ts->oppo_swmp_control = 0;
}

void himax_pin_reset(void)
{
    I("%s: Now reset the Touch chip.\n", __func__);

    himax_rst_gpio_set(private_ts->rst_gpio, 0);
    msleep(20);
    himax_rst_gpio_set(private_ts->rst_gpio, 1);
    msleep(20);
    himax_clear_reg_status();
    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/17 add for hx8527*/
    himax_tp_direction_set(OPPO_EN_PORTRAIT);//keep portrait,after reset
}

void himax_reload_config(void)
{
    himax_loadSensorConfig(private_ts->client,private_ts->pdata);
    himax_power_on_init(private_ts->client);
    if(himax_report_data_init())
        E("%s: allocate data fail\n",__func__);;
    calculate_point_number();
}

void himax_irq_switch(int switch_on)
{
    int ret = 0;
    if(switch_on)
    {

        if (private_ts->use_irq)
            himax_int_enable(private_ts->client->irq,switch_on);
        else
            hrtimer_start(&private_ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
    }
    else
    {
        if (private_ts->use_irq)
            himax_int_enable(private_ts->client->irq,switch_on);
        else
        {
            hrtimer_cancel(&private_ts->timer);
            ret = cancel_work_sync(&private_ts->work);
        }
    }
}

void himax_ic_reset(uint8_t loadconfig,uint8_t int_off)
{
    struct himax_ts_data *ts = private_ts;

#ifdef HX_ESD_RECOVERY
    HX_HW_RESET_ACTIVATE = 1;
#endif

    I("%s,status: loadconfig=%d,int_off=%d\n",__func__,loadconfig,int_off);

    if (ts->rst_gpio >= 0)
    {
        if(int_off)
        {
            himax_irq_switch(0);
        }

        himax_pin_reset();
        if(loadconfig)
        {
            himax_reload_config();
        }
        if(int_off)
        {
            himax_irq_switch(1);
        }
    }

}
#endif

#if defined(HX_ESD_RECOVERY)
int g_zero_event_count = 0;

int himax_ic_esd_recovery(int hx_esd_event,int hx_zero_event,int length)
{
    int shaking_ret = 0;

    /* hand shaking status: 0:Running, 1:Stop, 2:I2C Fail */
    shaking_ret = himax_hand_shaking(private_ts->client);
    if(shaking_ret == 2)
    {
        I("[HIMAX TP MSG]: I2C Fail.\n");
        goto err_workqueue_out;
    }
    if(hx_esd_event == length)
    {
        goto checksum_fail;
    }
    else if(shaking_ret == 1 && hx_zero_event == length)
    {
        I("[HIMAX TP MSG]: ESD event checked - ALL Zero.\n");
        goto checksum_fail;
    }
    else
        goto workqueue_out;

checksum_fail:
    return CHECKSUM_FAIL;
err_workqueue_out:
    return ERR_WORK_OUT;
workqueue_out:
    return WORK_OUT;
}

void himax_esd_ic_reset(void)
{
    uint8_t read_R36[2] = {0};

    while(ESD_R36_FAIL <=3 )
    {
		HX_ESD_RESET_ACTIVATE = 1;
#ifdef HX_RST_PIN_FUNC
		himax_pin_reset();
#endif
        if(himax_loadSensorConfig(private_ts->client, private_ts->pdata) < 0)
        {
            ESD_R36_FAIL++;
            continue;
        }
        if(i2c_himax_read(private_ts->client, 0x36, read_R36, 2, 10) < 0)
        {
            ESD_R36_FAIL++;
            continue;
        }
        if(read_R36[0] != 0x0F || read_R36[1] != 0x53)
        {
            E("[HimaxError] %s R36 Fail : R36[0]=%d,R36[1]=%d,R36 Counter=%d \n",__func__,read_R36[0],read_R36[1],ESD_R36_FAIL);
            ESD_R36_FAIL++;
        }
        else
        {
            himax_power_on_init(private_ts->client);
            if(himax_report_data_init())
                E("%s: allocate data fail\n",__func__);;
            calculate_point_number();
            break;
        }
    }
    /* reset status */
    ESD_R36_FAIL = 0;
}
#endif

int himax_hand_shaking(struct i2c_client *client)    //0:Running, 1:Stop, 2:I2C Fail
{
    int ret, result;
    uint8_t hw_reset_check[1];
    uint8_t hw_reset_check_2[1];
    uint8_t buf0[2];


    memset(hw_reset_check, 0x00, sizeof(hw_reset_check));
    memset(hw_reset_check_2, 0x00, sizeof(hw_reset_check_2));

    buf0[0] = 0xF2;
    if (IC_STATUS_CHECK == 0xAA)
    {
        buf0[1] = 0xAA;
        IC_STATUS_CHECK = 0x55;
    }
    else
    {
        buf0[1] = 0x55;
        IC_STATUS_CHECK = 0xAA;
    }

    ret = i2c_himax_master_write(client, buf0, 2, DEFAULT_RETRY_CNT);
    if (ret < 0)
    {
        E("[Himax]:write 0xF2 failed line: %d \n",__LINE__);
        goto work_func_send_i2c_msg_fail;
    }
    msleep(50);

    buf0[0] = 0xF2;
    buf0[1] = 0x00;
    ret = i2c_himax_master_write(client, buf0, 2, DEFAULT_RETRY_CNT);
    if (ret < 0)
    {
        E("[Himax]:write 0x92 failed line: %d \n",__LINE__);
        goto work_func_send_i2c_msg_fail;
    }
    msleep(2);

    ret = i2c_himax_read(client, 0x90, hw_reset_check, 1, DEFAULT_RETRY_CNT);
    if (ret < 0)
    {
        E("[Himax]:i2c_himax_read 0x90 failed line: %d \n",__LINE__);
        goto work_func_send_i2c_msg_fail;
    }

    if ((IC_STATUS_CHECK != hw_reset_check[0]))
    {
        msleep(2);
        ret = i2c_himax_read(client, 0x90, hw_reset_check_2, 1, DEFAULT_RETRY_CNT);
        if (ret < 0)
        {
            E("[Himax]:i2c_himax_read 0x90 failed line: %d \n",__LINE__);
            goto work_func_send_i2c_msg_fail;
        }

        if (hw_reset_check[0] == hw_reset_check_2[0])
        {
            result = 1;
        }
        else
        {
            result = 0;
        }
    }
    else
    {
        result = 0;
    }

    return result;

work_func_send_i2c_msg_fail:
    return 2;
}

void himax_idle_mode(struct i2c_client *client,int disable)
{

}

int himax_determin_diag_rawdata(int diag_command)
{
    return (diag_command/10 > 0) ? 0 : diag_command%10;
}

int himax_determin_diag_storage(int diag_command)
{
    return 0;
}

int himax_switch_mode(struct i2c_client *client,int mode)
{
    return 1;
}

void himax_return_event_stack(struct i2c_client *client)
{

}

void himax_diag_register_set(struct i2c_client *client, uint8_t diag_command)
{
    uint8_t command_F1h[2] = {0xF1, 0x00};

    //diag_command = diag_command - '0';
    if(diag_command > 8)
    {
        E("[Himax]Diag command error!diag_command=0x%x\n",diag_command);
        return;
    }
    command_F1h[1] = diag_command;

    i2c_himax_write(client, command_F1h[0],&command_F1h[1], 1, DEFAULT_RETRY_CNT);
}

void himax_flash_dump_func(struct i2c_client *client, uint8_t local_flash_command, int Flash_Size, uint8_t *flash_buffer)
{
    int i=0, j=0, k=0, l=0;
    int buffer_ptr = 0;
    uint8_t x81_command[2] = {HX_CMD_TSSLPOUT,0x00};
    uint8_t x82_command[2] = {HX_CMD_TSSOFF,0x00};
    uint8_t x43_command[4] = {HX_CMD_FLASH_ENABLE,0x00,0x00,0x00};
    uint8_t x44_command[4] = {HX_CMD_FLASH_SET_ADDRESS,0x00,0x00,0x00};
    uint8_t x59_tmp[4] = {0,0,0,0};
    uint8_t page_tmp[128];


    himax_int_enable(client->irq,0);

#ifdef HX_CHIP_STATUS_MONITOR
    g_chip_monitor_data->HX_CHIP_POLLING_COUNT = 0;
    g_chip_monitor_data->HX_CHIP_MONITOR_EN = 0;
    cancel_delayed_work_sync(&private_ts->himax_chip_monitor);
#endif

    setFlashDumpGoing(true);

    local_flash_command = getFlashCommand();
#ifdef HX_RST_PIN_FUNC
    himax_ic_reset(false,false);
#endif

    if ( i2c_himax_master_write(client, x81_command, 1, 3) < 0 )//sleep out
    {
        E("%s i2c write 81 fail.\n",__func__);
        goto Flash_Dump_i2c_transfer_error;
    }
    msleep(120);
    if ( i2c_himax_master_write(client, x82_command, 1, 3) < 0 )
    {
        E("%s i2c write 82 fail.\n",__func__);
        goto Flash_Dump_i2c_transfer_error;
    }
    msleep(100);

    x43_command[1] = 0x01;
    if ( i2c_himax_write(client, x43_command[0],&x43_command[1], 1, DEFAULT_RETRY_CNT) < 0)
    {
        goto Flash_Dump_i2c_transfer_error;
    }
    msleep(100);

    for( i=0 ; i<8 ; i++)
    {
        for(j=0 ; j<32 ; j++)
        {
            //I(" Step 2 i=%d , j=%d %s\n",i,j,__func__);
            //read page start
            for(k=0; k<128; k++)
            {
                page_tmp[k] = 0x00;
            }
            for(k=0; k<32; k++)
            {
                x44_command[1] = k;
                x44_command[2] = j;
                x44_command[3] = i;
                if ( i2c_himax_write(client, x44_command[0],&x44_command[1], 3, DEFAULT_RETRY_CNT) < 0 )
                {
                    E("%s i2c write 44 fail.\n",__func__);
                    goto Flash_Dump_i2c_transfer_error;
                }
                if ( i2c_himax_write_command(client, 0x46, DEFAULT_RETRY_CNT) < 0)
                {
                    E("%s i2c write 46 fail.\n",__func__);
                    goto Flash_Dump_i2c_transfer_error;
                }
                //msleep(2);
                if ( i2c_himax_read(client, 0x59, x59_tmp, 4, DEFAULT_RETRY_CNT) < 0)
                {
                    E("%s i2c write 59 fail.\n",__func__);
                    goto Flash_Dump_i2c_transfer_error;
                }
                //msleep(2);
                for(l=0; l<4; l++)
                {
                    page_tmp[k*4+l] = x59_tmp[l];
                }
                //msleep(10);
            }
            //read page end

            for(k=0; k<128; k++)
            {
                flash_buffer[buffer_ptr++] = page_tmp[k];

            }
            setFlashDumpProgress(i*32 + j);
        }
    }

    I("Complete~~~~~~~~~~~~~~~~~~~~~~~\n");
    if(local_flash_command==0x01)
    {
        I(" buffer_ptr = %d \n",buffer_ptr);

        for (i = 0; i < buffer_ptr; i++)
        {
            I("%2.2X ", flash_buffer[i]);

            if ((i % 16) == 15)
            {
                I("\n");
            }
        }
        I("End~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    }
    else if (local_flash_command == 2)
    {
        struct file *fn;

        fn = filp_open(FLASH_DUMP_FILE,O_CREAT | O_WRONLY,0);
        if (!IS_ERR(fn))
        {
            fn->f_op->write(fn,flash_buffer,buffer_ptr*sizeof(uint8_t),&fn->f_pos);
            filp_close(fn,NULL);
        }
    }

#ifdef HX_RST_PIN_FUNC
    himax_ic_reset(true,false);
#endif

    himax_int_enable(client->irq,1);
#ifdef HX_CHIP_STATUS_MONITOR
    g_chip_monitor_data->HX_CHIP_POLLING_COUNT = 0;
    g_chip_monitor_data->HX_CHIP_MONITOR_EN = 1;
    queue_delayed_work(private_ts->himax_chip_monitor_wq, &private_ts->himax_chip_monitor, g_chip_monitor_data->HX_POLLING_TIMES*HZ);
#endif
    setFlashDumpGoing(false);

    setFlashDumpComplete(1);
    setSysOperation(0);
    return;

Flash_Dump_i2c_transfer_error:

#ifdef HX_RST_PIN_FUNC
    himax_ic_reset(true,false);
#endif

    himax_int_enable(client->irq,1);
#ifdef HX_CHIP_STATUS_MONITOR
    g_chip_monitor_data->HX_CHIP_POLLING_COUNT = 0;
    g_chip_monitor_data->HX_CHIP_MONITOR_EN = 1;
    queue_delayed_work(private_ts->himax_chip_monitor_wq, &private_ts->himax_chip_monitor, g_chip_monitor_data->HX_POLLING_TIMES*HZ);
#endif
    setFlashDumpGoing(false);
    setFlashDumpComplete(0);
    setFlashDumpFail(1);
    setSysOperation(0);
}

#if defined(HX_TP_SELF_TEST_DRIVER) || defined(CONFIG_TOUCHSCREEN_HIMAX_ITO_TEST)
void himax_get_raw_data(uint8_t diag_command, uint16_t mutual_num, uint16_t self_num)
{
    uint8_t command_F1h_bank[2] = {0xF1, diag_command};
    int i = 0;

    I("Start get raw data\n");

	for (i = 0; i < mutual_num; i++)
		mutual_bank[i] = 0xFF;
	for (i = 0; i < self_num; i++)
		self_bank[i] = 0xFF;

    Selftest_flag = 1;
    I(" mutual_num = %d, self_num = %d \n",mutual_num, self_num);
    I(" Selftest_flag = %d \n",Selftest_flag);

	himax_sense_off(private_ts->client);
    i2c_himax_write(private_ts->client, command_F1h_bank[0],&command_F1h_bank[1], 1, DEFAULT_RETRY_CNT);
	himax_sense_on(private_ts->client,0x01);
    msleep(100);

    himax_int_enable(private_ts->client->irq,1);
    for (i = 0; i < 100; i++) //check for max 1000 times
    {
        I(" conuter = %d ,Selftest_flag = %d\n",i,Selftest_flag);
        if(Selftest_flag == 0)
            break;
        msleep(10);
    }

    himax_int_enable(private_ts->client->irq,0);
    I(" Write diag cmd = %d \n",command_F1h_bank[1]);
}
#endif
#ifdef HX_TP_PROC_SELF_TEST

/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 add for hx8527*/
void himax_init_criteria_size(void)
{
    HX_CRITERIA_ITEM = 3;
    HX_CRITERIA_SIZE = HX_CRITERIA_ITEM*2;
}

/* parsing Criteria start */
int himax_get_criteria_size(void)
{
    int result = 0;

    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 add for hx8527*/
    if (!HX_CRITERIA_SIZE)
        himax_init_criteria_size();

    result = HX_CRITERIA_SIZE;

    return result;
}

/* claculate 10's power function */
int himax_power_cal(int pow, int number)
{
	int i = 0;
	int result = 1;

	for (i = 0; i < pow; i++)
		result *= 10;
	result = result * number;

	return result;

}

/* String to int */
int hiamx_parse_str2int(char *str)
{
	int i = 0;
	int temp_cal = 0;
	int result = 0;
	int str_len = strlen(str);
	int negtive_flag = 0;
	for (i = 0; i < strlen(str); i++) {
		if (str[i] == '-') {
			negtive_flag = 1;
			continue;
		}
		else if (str[i] == 'n') {
			if ((str[i + 1] == 'u') && (str[i + 2] == 'l') && (str[i + 3] == 'l'))
				return -1;
		}
		temp_cal = str[i] - '0';
		result += himax_power_cal(str_len-i-1, temp_cal); /* str's the lowest char is the number's the highest number
															So we should reverse this number before using the power function
															-1: starting number is from 0 ex:10^0 = 1,10^1=10*/
	}

	if (negtive_flag == 1) {
		result = 0 - result;
	}

	return result;
}

int himax_count_comma(const struct firmware *file_entry)
{
	int i = 0;
	int result = 0;
	for (i = 0; i < file_entry->size; i++) {
		if (file_entry->data[i] == ASCII_COMMA)
			result++;
	}
	return result;
}

/* Get sub-string from original string by using some charaters */
int himax_saperate_comma(const struct firmware *file_entry, char **result, int str_size)
{
	int count = 0;
	int str_count = 0; /* now string*/
	int char_count = 0; /* now char count in string*/

	do {
		switch (file_entry->data[count]) {
		case ASCII_COMMA:
		case ACSII_SPACE:
		case ASCII_CR:
		case ASCII_LF:
			count++;
			/* If end of line as above condifiton, differencing the count of char.
				If char_count != 0 it's meaning this string is parsing over .
				The Next char is belong to next string */
			if (char_count != 0) {
				char_count = 0;
				str_count++;
			}
			break;
		default:
			result[str_count][char_count++] = file_entry->data[count];
			count++;
			break;
		}
	} while (count < file_entry->size && str_count < str_size);

	return 0;
}

int hx_diff_str(char *str1, char *str2)
{
	int i = 0;
	int result = 0; /* zero is all same, non-zero is not same index*/
	int str1_len = strlen(str1);
	int str2_len = strlen(str2);
	if(str1_len != str2_len) {
		I("%s:Size different!\n", __func__);
		return -1;
	}

	for(i = 0;i < str1_len;i++) {
		if(str1[i] != str2[i]) {
			result = i+1;
			I("%s: different in %d!\n", __func__,result);
			return result;
		}
	}

	return result;
}

int hx_get_crtra_by_name(char **result)
{
	int i = 0;
	/* count of criteria type */
	int count_type = 0;
	/* count of criteria data */
	int count_data = 0;
	int err = HX_INSPECT_OK;
	int crtra_count = himax_get_criteria_size();
	int all_mut_len = ic_data->HX_TX_NUM*ic_data->HX_RX_NUM;

	/* get criteria and assign to a global array(2-Dimensional/int) */
	for (i = 0; i < (crtra_count * (all_mut_len) + crtra_count); i++) {
		/* It have get one page(all mutual) criteria data!
			And we should skip the string of criteria name!
		*/
		if(i==0 || i == ((i / (all_mut_len))+(i / (all_mut_len) * (all_mut_len)))) {
			count_data = 0;
			/* change to next type */
			if(i != 0)
				count_type++;
			if(hx_diff_str(g_hx_inspt_crtra_name[count_type], result[i]) != 0) {
				E("%s:Name Not match!\n",__func__);
				E("can recognize[%d]=%s\n", count_type, g_hx_inspt_crtra_name[count_type]);
				E("get from file[%d]=%s\n", i, result[i]);
				E("Please check criteria file again!\n");
				err = HX_INSPECT_EFILE;
				return err;
			}
			continue;
		}
		/* change string to int*/
		g_inspection_criteria[count_type][count_data] = hiamx_parse_str2int(result[i]);
		/* dbg
		I("[%d]g_inspection_criteria[%d][%d]=%d\n", i, count_type, count_data, g_inspection_criteria[count_type][count_data]);
		*/
		count_data++;
	}

	return err;
}

/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 add for hx8527*/
char **g_hx_crtra_result;
int32_t g_hx_crtra_str_count;

void init_criteria_file_ds(void)
{
    int all_mut_len = ic_data->HX_TX_NUM*ic_data->HX_RX_NUM;
    int crtra_count = himax_get_criteria_size();
    /* size of criteria include name string */
    int data_size = ((all_mut_len) * crtra_count) + crtra_count;
    int32_t i;
    g_hx_crtra_str_count = data_size;

    if (!g_hx_crtra_result) {
        g_hx_crtra_result = kzalloc(data_size * sizeof(char *), GFP_KERNEL);
        for (i = 0 ; i < data_size; i++)
            g_hx_crtra_result[i] = kzalloc(CRITERIA_ITEM_STR_MAX * sizeof(char), GFP_KERNEL);
    } else {
        for (i = 0 ; i < data_size; i++)
            memset(g_hx_crtra_result[i], 0x00, CRITERIA_ITEM_STR_MAX * sizeof(char));
    }
}

void deinit_criteria_file_ds(void)
{
    int32_t i;

    if (g_hx_crtra_result) {
        for (i = 0 ; i < g_hx_crtra_str_count; i++)
            kfree(g_hx_crtra_result[i]);
        kfree(g_hx_crtra_result);
    }
}

int himax_parse_criteria_file(void)
{
    int err = 0;
    const struct firmware *file_entry = NULL;
    char *file_name = g_hx_crtra_file;//"hx_criteria.csv";
    // int i = 0;

    int crtra_count = himax_get_criteria_size();
    int data_size = 0; /* The maximum of number Data*/
    int all_mut_len = ic_data->HX_TX_NUM*ic_data->HX_RX_NUM;
    int result_all_len = 0;
    int file_size = 0;

    I("%s,Entering \n", __func__);
    I("file name = %s\n", file_name);

    /* default path is /system/etc/firmware */
    err = request_firmware(&file_entry, file_name, private_ts->dev);
    if (err < 0) {
        E("%s,fail in line%d error code=%d\n", __func__, __LINE__, err);
        err = HX_INSPECT_EFILE;
        goto END_FUNC_REQ_FAIL;
    }

    /* size of criteria include name string */
    data_size = ((all_mut_len) * crtra_count) + crtra_count;

    /* init the array which store original criteria and include name string*/
    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 modified for hx8527*/
    init_criteria_file_ds();

    result_all_len = data_size;
    file_size =	file_entry->size;
    I("Now result_all_len=%d\n",result_all_len);
    I("Now file_size=%d\n",file_size);

    /* dbg */
    I("first 4 bytes 0x%2X,0x%2X,0x%2X,0x%2X !\n", file_entry->data[0], file_entry->data[1], file_entry->data[2], file_entry->data[3]);

    /* parse value in to result array(1-Dimensional/String) */
    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 modified for hx8527*/
    himax_saperate_comma(file_entry, g_hx_crtra_result, data_size);

    err = hx_get_crtra_by_name(g_hx_crtra_result);
    if(err != HX_INSPECT_OK) {
        E("%s:Load criteria from file fail, go end!\n", __func__);
        goto END_FUNC;
    }

    /* for dbg
    for (i = 0; i < (((ic_data->HX_TX_NUM*ic_data->HX_RX_NUM)*6)+6); i++)
    {
        if(i%32 ==0 && i >0)
            I("\n");
        I("[%d]%s", i, result[i]);

    }*/
END_FUNC:
    release_firmware(file_entry);
END_FUNC_REQ_FAIL:
    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 delete for hx8527*/
    // deinit_criteria_file_ds();
    I("%s,END \n", __func__);
    return err;
}
/* parsing Criteria end */

int himax_self_test_data_init(void)
{
    int ret = HX_INSPECT_OK;
    int i = 0;

    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 modified for hx8527*/
    himax_init_criteria_size();
    I("There is %d HX_CRITERIA_ITEM and %d HX_CRITERIA_SIZE\n", HX_CRITERIA_ITEM, HX_CRITERIA_SIZE);

    if (!g_inspection_criteria) {
        g_inspection_criteria = kzalloc(sizeof(int*)*HX_CRITERIA_SIZE, GFP_KERNEL);
        for(i = 0;i < HX_CRITERIA_SIZE;i++) {
            g_inspection_criteria[i] = kzalloc(sizeof(int)*(ic_data->HX_TX_NUM*ic_data->HX_RX_NUM), GFP_KERNEL);
        }
    } else {
        for(i = 0;i < HX_CRITERIA_SIZE;i++) {
            memset(g_inspection_criteria[i], 0x00, sizeof(int)*(ic_data->HX_TX_NUM*ic_data->HX_RX_NUM));
        }
    }

    ret = himax_parse_criteria_file();
    return ret;
}

void himax_self_test_data_deinit(void)
{
	int i;

	if(g_inspection_criteria != NULL) {
		for (i = 0; i < HX_CRITERIA_SIZE; i++) {
			kfree(g_inspection_criteria[i]);
		}
		kfree(g_inspection_criteria);
		I("Now it have free the g_inspection_criteria!\n");
	} else {
		I("No Need to free g_inspection_criteria!\n");
	}
}

uint32_t himax_selftest_print(uint8_t *output_buffer, uint16_t *mutual_bank, uint16_t *self_bank, uint32_t len, int i, int j, int transpose, uint32_t ret)
{
    uint8_t x_channel = getXChannel();

    if(transpose)
        ret += snprintf(output_buffer + ret, len - ret, "%6d", (uint8_t)mutual_bank[ j + i*x_channel]);
    else
        ret += snprintf(output_buffer + ret, len - ret, "%6d", (uint8_t)mutual_bank[ i + j*x_channel]);

    return ret;
}

uint32_t himax_selftest_inloop(uint8_t *output_buffer, uint16_t *mutual_bank, uint16_t *self_bank, uint32_t len, int in_init,int out_init,bool transpose, int j, uint32_t ret)
{
    int i;
    int in_max = 0;
    uint8_t x_channel = getXChannel();
    uint8_t y_channel = getYChannel();

    if(transpose)
        in_max = y_channel;
    else
        in_max = x_channel;

    if (in_init > 0) //bit0 = 1
    {
        for(i = in_init-1; i >= 0; i--)
        {
            ret = himax_selftest_print(output_buffer, mutual_bank, self_bank, len, i, j, transpose, ret);
        }
        if(transpose)
        {
            if(out_init > 0)
                ret += snprintf(output_buffer + ret, len - ret, " %5d\n", self_bank[j]);
            else
                ret += snprintf(output_buffer + ret, len - ret, " %5d\n", self_bank[x_channel - j - 1]);
        }
    }
    else	//bit0 = 0
    {
        for (i = 0; i < in_max; i++)
        {
            ret = himax_selftest_print(output_buffer, mutual_bank, self_bank, len, i, j, transpose, ret);
        }
        if(transpose)
        {
            if(out_init > 0)
                ret += snprintf(output_buffer + ret, len - ret, " %5d\n", self_bank[x_channel - j - 1]);
            else
                ret += snprintf(output_buffer + ret, len - ret, " %5d\n", self_bank[j]);
        }
    }

    return ret;
}

uint32_t himax_selftest_outloop(uint8_t *output_buffer, uint16_t *mutual_bank, uint16_t *self_bank, uint32_t len, int transpose, int out_init, int in_init, uint32_t ret)
{
    int j;
    int out_max = 0;
    int self_cnt = 0;
    uint8_t x_channel = getXChannel();
    uint8_t y_channel = getYChannel();

    if(transpose)
        out_max = x_channel;
    else
        out_max = y_channel;

    if(out_init > 0) //bit1 = 1
    {
        self_cnt = 1;
        for(j = out_init-1; j >= 0; j--)
        {
            ret += snprintf(output_buffer + ret, len - ret, "%3c%02d%c",'[', j + 1,']');
            ret = himax_selftest_inloop(output_buffer, mutual_bank, self_bank, len, in_init, out_init, transpose, j, ret);
            if(!transpose)
            {
                ret += snprintf(output_buffer + ret, len - ret, " %5d\n", self_bank[y_channel + x_channel - self_cnt]);
                self_cnt++;
            }
        }
    }
    else	//bit1 = 0
    {
        for(j = 0; j < out_max; j++)
        {
            ret += snprintf(output_buffer + ret, len - ret, "%3c%02d%c",'[', j + 1,']');
            ret = himax_selftest_inloop(output_buffer, mutual_bank, self_bank, len, in_init, out_init, transpose, j, ret);
            if(!transpose)
            {
                ret += snprintf(output_buffer + ret, len - ret, " %5d\n", self_bank[j + x_channel]);
            }
        }
    }

    return ret;
}

uint32_t himax_selftest_arrange(uint8_t *output_buffer, uint16_t *mutual_bank, uint16_t *self_bank, uint32_t len, uint32_t ret)
{
    int bit2,bit1,bit0;
    int i;
    int g_diag_arr_num = 0;
    uint8_t x_channel = getXChannel();
    uint8_t y_channel = getYChannel();

    /* rotate bit */
    bit2 = g_diag_arr_num >> 2;
    /* reverse Y */
    bit1 = g_diag_arr_num >> 1 & 0x1;
    /* reverse X */
    bit0 = g_diag_arr_num & 0x1;

    for (i = 0 ; i <= x_channel; i++)
    {
        ret += snprintf(output_buffer + ret, len - ret, "%3c%02d%c",'[', i,']');
    }
    ret += snprintf(output_buffer + ret, len - ret, "\n");

    ret = himax_selftest_outloop(output_buffer, mutual_bank, self_bank, len, bit2, bit1 * y_channel, bit0 * x_channel, ret);

    ret += snprintf(output_buffer + ret, len - ret, "%6c",' ');
    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/26 modified for coverity CID 67594*/
    for (i = 0; i < x_channel; i++) {
        ret += snprintf(output_buffer + ret, len - ret, "%6d", self_bank[i]);
    }

    return ret;
}

uint32_t himax_selftest_read(uint8_t *output_buffer, uint16_t *mutual_bank, uint16_t *self_bank, uint32_t len)
{
    uint32_t ret = 0;
    uint16_t mutual_num, self_num, width;
    uint8_t x_channel = getXChannel();
    uint8_t y_channel = getYChannel();
    int i;
    int max_self = 0;
    int min_self = 255;
    int bank_max = 0;
    int bank_min = 255;

    mutual_num	= x_channel * y_channel;
    self_num	= x_channel + y_channel;
    width		= x_channel;

    ret += snprintf(output_buffer + ret, len - ret, "ChannelStart: %4d, %4d\n\n", x_channel, y_channel);

    ret = himax_selftest_arrange(output_buffer, mutual_bank, self_bank, len, ret);
    ret += snprintf(output_buffer + ret, len - ret, "\n");
    ret += snprintf(output_buffer + ret, len - ret, "ChannelEnd");
    ret += snprintf(output_buffer + ret, len - ret, "\n");

    for(i = 0; i < (x_channel * y_channel); i++)
    {
        if(mutual_bank[i] > bank_max)
            bank_max = mutual_bank[i];
        if(mutual_bank[i] < bank_min)
            bank_min = mutual_bank[i];
    }

    for(i = 0; i < (x_channel + y_channel); i++)
    {
        if(self_bank[i] > max_self)
            max_self = self_bank[i];
        if(self_bank[i] < min_self)
            min_self = self_bank[i];
    }
    ret += snprintf(output_buffer + ret, len - ret, "Mutual Max:%3d, Min:%3d\n", bank_max, bank_min);
    ret += snprintf(output_buffer + ret, len - ret, "Self Max:%3d, Min:%3d\n", max_self, min_self);

    return ret;
}

void self_test_output(uint8_t *RB1H, uint16_t *mutual_bank, uint16_t *self_bank)
{
    struct file *fn = NULL;
    static uint8_t *output_buffer = NULL;
    mm_segment_t fs;
    loff_t pos = 0;
    uint32_t ret = 0;
    static uint32_t pre_len = 0;
    uint32_t len = (ic_data->HX_RX_NUM*ic_data->HX_TX_NUM) * 6 * 2;
    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 modified for hx8527*/
    if (!output_buffer) {
        output_buffer = kzalloc(len * sizeof(uint8_t), GFP_KERNEL);
        pre_len = len;
    } else if (pre_len != len) {
        kfree(output_buffer);
        output_buffer = kzalloc(len * sizeof(uint8_t), GFP_KERNEL);
        pre_len = len;
    }
    if (output_buffer == NULL)
    {
        I("[Himax]: Output buffer memory allocate fail\n");
        goto OPEN_FAIL;
    }

    ret = himax_selftest_read(output_buffer, mutual_bank, self_bank, len);

    fs = get_fs();
    set_fs(get_ds());
    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 modified for ito*/
    if (!oppo_hx_black_screen_test_flag) {
        fn = filp_open(g_sensor_result_file, O_TRUNC|O_CREAT|O_RDWR, 0660);
    } else {
        fn = filp_open(g_sensor_black_result_file, O_TRUNC|O_CREAT|O_RDWR, 0660);
    }
    /*Kai.Zhang@ODM_HQ.BSP.TP.Function, 2019/05/27 modified for ito*/
    if (!IS_ERR(fn)) {
        I("%s create data file and ready to write\n", __func__);
        vfs_write(fn, output_buffer, ret * sizeof(uint8_t), &pos);
    } else {
        fn = NULL;
        set_fs(fs);
        return;
    } 
 /*   if (RB1H[0] == 0xAA)
    {
        fn = filp_open(g_test_ok_file, O_TRUNC|O_CREAT|O_RDWR, 0660);
    }
    else
    {
        fn = filp_open(g_test_ng_file, O_TRUNC|O_CREAT|O_RDWR, 0660);
    }

    if (!IS_ERR(fn)) {
        I("%s create data file and ready to write\n", __func__);
        vfs_write(fn, output_buffer, ret*sizeof(uint8_t), &pos);
        filp_close(fn, NULL);
    }

    pos = 0;
    fn = NULL;
    ret = 0;*/
    memset(output_buffer, 0x0, len);
    if (RB1H[0] == 0xAA)
    {
        ret += snprintf(output_buffer + ret, len - ret, "\nSelf_Test Pass\n");
    }
    else
    {
        ret += snprintf(output_buffer + ret, len - ret, "\nSelf_Test Fail\n");
    }

    if (!IS_ERR(fn)) {
        I("%s create result file and ready to write\n", __func__);
        vfs_write(fn, output_buffer, ret * sizeof(uint8_t), &pos);
        filp_close(fn, NULL);
    }

    /*Kai.Zhang@ODM_HQ.BSP.TP.Function, 2019/05/27 modified for ito*/
    /* if (fn != NULL) {
        filp_close(fn, NULL);
    }  */
    set_fs(fs);
    //kfree(output_buffer);
OPEN_FAIL:
    return;
}

#ifdef HX_TP_SELF_TEST_DRIVER
extern uint32_t HX_BASELINE_TEST_ERROR_CNT;

static uint8_t Self_Test_Bank(uint8_t *RB1H)
{
    uint16_t i;
    uint16_t x_channel = ic_data->HX_RX_NUM;
    uint16_t y_channel = ic_data->HX_TX_NUM;
    uint16_t mutual_num = x_channel * y_channel;
    uint16_t self_num = x_channel + y_channel;
    uint32_t bank_sum, m;
    uint8_t bank_avg;
    uint8_t bank_ulmt[mutual_num];
    uint8_t bank_dlmt[mutual_num];
    uint8_t bank_min, bank_max;
    uint8_t slf_tx_fail_cnt, slf_rx_fail_cnt;
    uint16_t mut_fail_cnt;
    int fail_flag;
    uint8_t bank_val_tmp;

    uint8_t set_bnk_ulmt[mutual_num];
    uint8_t set_bnk_dlmt[mutual_num];
    uint8_t set_avg_bnk_ulmt;
    uint8_t set_avg_bnk_dlmt;
    uint8_t set_slf_bnk_ulmt[self_num];
    uint8_t set_slf_bnk_dlmt[self_num];

    static uint16_t pre_mutual_num = 0;
    static uint16_t pre_self_num = 0;

    I("Start self_test\n");
    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 modified for hx8527*/
    if (!mutual_bank) {
        mutual_bank = kzalloc(mutual_num * sizeof(uint16_t), GFP_KERNEL);
        pre_mutual_num = mutual_num;
    } else if (pre_mutual_num != mutual_num) {
        kfree(mutual_bank);
        mutual_bank = kzalloc(mutual_num * sizeof(uint16_t), GFP_KERNEL);
        pre_mutual_num = mutual_num;
    }
    if (!self_bank) {
        self_bank = kzalloc(self_num * sizeof(uint16_t), GFP_KERNEL);
        pre_self_num = self_num;
    } else if (pre_self_num != self_num) {
        kfree(self_bank);
        self_bank = kzalloc(self_num * sizeof(uint16_t), GFP_KERNEL);
        pre_self_num = self_num;
    }
    himax_get_raw_data(0x03, mutual_num, self_num); //get bank data
    if (RB1H[0] == 0x80)
    {
        I(" Enter Test flow \n");
        for (i = 0; i < mutual_num; i++) {
            set_bnk_ulmt[i] = (uint8_t)(g_inspection_criteria[0][i]);
            set_bnk_dlmt[i] = (uint8_t)(g_inspection_criteria[1][i]);
        }

        set_avg_bnk_ulmt = (uint8_t)(g_inspection_criteria[2][0]);
        set_avg_bnk_dlmt = (uint8_t)(g_inspection_criteria[3][0]);

        for (i = 0; i < self_num; i++) {
            set_slf_bnk_ulmt[i] = (uint8_t)(g_inspection_criteria[4][i]);
            set_slf_bnk_dlmt[i] = (uint8_t)(g_inspection_criteria[5][i]);
        }

        fail_flag = 0;
        bank_sum = 0;
        bank_avg = 0;
        mut_fail_cnt = 0;

        //Calculate Bank Average
        for (m = 0; m < mutual_num; m++)
        {
            bank_sum += (uint8_t)mutual_bank[m];
        }
        I(" bank_sum = %d \n", bank_sum);
        bank_avg = (bank_sum / mutual_num);
        I(" bank_avg = %d \n", bank_avg);
        //======Condition 1======Check average bank with absolute value
        if ((bank_avg > set_avg_bnk_ulmt) || (bank_avg < set_avg_bnk_dlmt)) {
            fail_flag = 1;
            E("Average bank test fail!!\n");
        }

        I(" fail_flag = %d\n", fail_flag);
        if (fail_flag)
        {
            RB1H[0] = 0xF1;     //Fail ID for Condition 1
            RB1H[1] = bank_avg;
            RB1H[2] = set_avg_bnk_ulmt;
            RB1H[3] = set_avg_bnk_dlmt;
            RB1H[4] = 0xFF;
            for (i = 0; i < 8; i++)
            {
                I(" RB1H[%d] = %X \n", i, RB1H[i]);
            }
            HX_BASELINE_TEST_ERROR_CNT++;
        }
        else
        {
            //======Condition 2======Check every block's bank with average value
#if 1//def SLF_TEST_BANK_ABS_LMT
            memcpy(bank_ulmt, set_bnk_ulmt, mutual_num);
            memcpy(bank_dlmt, set_bnk_dlmt, mutual_num);
#else
            if ((bank_avg + set_bnk_ulmt) > 245)
                bank_ulmt = 245;
            else
                bank_ulmt = bank_avg + set_bnk_ulmt;

            if (bank_avg > set_bnk_dlmt)
            {
                bank_dlmt = bank_avg - set_bnk_dlmt;
                if (bank_dlmt < 10)
                    bank_dlmt = 10;
            }
            else
                bank_dlmt = 10;
#endif

            bank_min = 0xFF;
            bank_max = 0x00;
            // I(" bank_ulmt = %d, bank_dlmt = %d \n", bank_ulmt, bank_dlmt);
            for (m = 0; m < mutual_num; m++)
            {
                bank_val_tmp = (uint8_t)mutual_bank[m];
                if ((bank_val_tmp > bank_ulmt[m]) || (bank_val_tmp < bank_dlmt[m]))
                {
                    fail_flag = 1;
                    E("Mutual block [%d] = %d, bank test fail!!\n", m, bank_val_tmp);
                    mut_fail_cnt++;
                }

                //Bank information record
                if (bank_val_tmp > bank_max)
                    bank_max = bank_val_tmp;
                else if (bank_val_tmp < bank_min)
                    bank_min = bank_val_tmp;
            }

            I(" fail_flag = %d, mut_fail_cnt = %d \n", fail_flag, mut_fail_cnt);
            if (fail_flag)
            {
                RB1H[0] = 0xF2; //Fail ID for Condition 2
                RB1H[1] = (uint8_t) mut_fail_cnt;
                RB1H[2] = (uint8_t)(mut_fail_cnt >> 8);
                RB1H[3] = bank_avg;
                RB1H[4] = bank_max;
                RB1H[5] = bank_min;
                RB1H[6] = bank_ulmt[0];
                RB1H[7] = bank_dlmt[0];
                for (i = 0; i < 8; i++)
                {
                    I(" RB1H[%d] = %X \n", i, RB1H[i]);
                }
                /*for (m = 0; m < mutual_num; m++)
                {
                    I(" mutual_bank[%d] = %X \n", m, mutual_bank[m]);
                }
                for (m = 0; m < self_num; m++)
                {
                    I(" self_bank[%d] = %X \n", m, self_bank[m]);
                }*/
                HX_BASELINE_TEST_ERROR_CNT++;
            }
            else
            {
                //======Condition 3======Check every self channel bank
                slf_rx_fail_cnt = 0x00; //Check SELF RX BANK
                slf_tx_fail_cnt = 0x00; //Check SELF TX BANK
                for (i = 0; i < (x_channel + y_channel); i++)
                {
                    bank_val_tmp = (uint8_t)self_bank[i];
                    if ((bank_val_tmp > set_slf_bnk_ulmt[i]) ||
                            (bank_val_tmp < set_slf_bnk_dlmt[i]))
                    {
                        fail_flag = 1;
                        E("Self block [%d] = %d, bank test fail!!\n", i, bank_val_tmp);
                        if (i < x_channel)
                            slf_rx_fail_cnt++;
                        else
                            slf_tx_fail_cnt++;
                    }
                }

                I(" slf_rx_fail_cnt = %d, slf_tx_fail_cnt = %d \n", slf_rx_fail_cnt, slf_tx_fail_cnt);
                if (fail_flag)
                {
                    RB1H[0] = 0xF3; //Fail ID for Condition 3
                    RB1H[1] = slf_rx_fail_cnt;
                    RB1H[2] = slf_tx_fail_cnt;
                    RB1H[3] = set_slf_bnk_ulmt[0];
                    RB1H[4] = set_slf_bnk_dlmt[0];
                    RB1H[5] = 0xFF;
                    for (i = 0; i < 8; i++)
                    {
                        I(" RB1H[%d] = %X \n", i, RB1H[i]);
                    }
                    /*for (m = 0; m < mutual_num; m++)
                    {
                        I(" mutual_bank[%d] = %X \n", m, mutual_bank[m]);
                    }
                    for (m = 0; m < self_num; m++)
                    {
                        I(" self_bank[%d] = %X \n", m, self_bank[m]);
                    }*/
                    HX_BASELINE_TEST_ERROR_CNT++;
                }
                else
                {
                    RB1H[0] = 0xAA; ////PASS ID
                    RB1H[1] = bank_avg;
                    RB1H[2] = bank_max;
                    RB1H[3] = bank_min;
                }
            }
        }
        self_test_output(RB1H, mutual_bank, self_bank);
    }
    //kfree(mutual_bank);
    //kfree(self_bank);
    return RB1H[0];
}
#endif

int himax_chip_self_test(struct i2c_client *client)
{
    uint8_t cmdbuf[11];
    uint8_t valuebuf[16];
    int pf_value=-1;
    int retry_readB1=10;
#ifdef HX_TP_SELF_TEST_DRIVER
    uint8_t RB1H[8];
    uint32_t ret = HX_INSPECT_OK;
#else
    int i=0;
#endif

    HX_BASELINE_TEST_ERROR_CNT = 0;
    memset(cmdbuf, 0x00, sizeof(cmdbuf));
    memset(valuebuf, 0x00, sizeof(valuebuf));
#ifdef HX_TP_SELF_TEST_DRIVER
    I("%s:IN\n", __func__);

    ret = himax_self_test_data_init();
    if (ret != HX_INSPECT_OK) {
        E("himax_self_test_data_init fail!\n");
        goto END_FUNC;
    }

    memset(RB1H, 0x00, sizeof(RB1H));

    Selftest_flag = 1;
    g_diag_command = 0x03;
    himax_int_enable(client->irq,1);
    //Get Criteria
    i2c_himax_read(client, 0xB1, RB1H, 8, DEFAULT_RETRY_CNT);

    msleep(10);
    I("[Himax]: self-test RB1H[0]=%x\n",RB1H[0]);
    RB1H[0] = RB1H[0] | 0x80;
    I("[Himax]: disable reK RB1H[0]=%x\n",RB1H[0]);
    i2c_himax_write(client, 0xB1, &RB1H[0], 1, DEFAULT_RETRY_CNT);
    msleep(10);
#else
    cmdbuf[0] = 0x06;
    i2c_himax_write(client, 0xF1,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
    msleep(120);
#endif
    i2c_himax_write(client, HX_CMD_TSSON,&cmdbuf[0], 0, DEFAULT_RETRY_CNT);
    msleep(120);

    i2c_himax_write(client, HX_CMD_TSSLPOUT,&cmdbuf[0], 0, DEFAULT_RETRY_CNT);
#ifdef HX_TP_SELF_TEST_DRIVER
    msleep(120);
    valuebuf[0] = Self_Test_Bank(&RB1H[0]);
#else
    memset(valuebuf, 0x00, sizeof(valuebuf));
    msleep(2000);
#endif
    while(valuebuf[0]==0x00 && retry_readB1 > 0)
    {
        i2c_himax_read(client, 0xB1, valuebuf, 2, DEFAULT_RETRY_CNT);
        msleep(100);
        retry_readB1--;
    }

    i2c_himax_write(client, HX_CMD_TSSOFF,&cmdbuf[0], 0, DEFAULT_RETRY_CNT);
    msleep(120);

    i2c_himax_write(client, HX_CMD_TSSLPIN,&cmdbuf[0], 0, DEFAULT_RETRY_CNT);
    msleep(120);

#ifdef HX_TP_SELF_TEST_DRIVER
    himax_int_enable(client->irq,0);
    g_diag_command = 0x00;
    Selftest_flag = 0;
    RB1H[0] = 0x00;
    I("[Himax]: enable reK RB1H[0]=%x\n",RB1H[0]);
    i2c_himax_write(client, 0xB1, &RB1H[0], 1, DEFAULT_RETRY_CNT);
#else
    cmdbuf[0] = 0x00;
    i2c_himax_write(client, 0xF1,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
    msleep(120);

    i2c_himax_read(client, 0xB1, valuebuf, 8, DEFAULT_RETRY_CNT);
    msleep(10);

    for(i=0; i<8; i++)
    {
        I("[Himax]: After slf test 0xB1 buff_back[%d] = 0x%x\n",i,valuebuf[i]);
    }

    msleep(30);
#endif
    if (valuebuf[0]==0xAA)
    {
        I("[Himax]: self-test pass\n");
        pf_value = 0x0;
    }
    else if (valuebuf[0]==0xF1)
    {
        I("[Himax]: Bank Average fail\n");
        pf_value = 0x1;
    }
    else if (valuebuf[0]==0xF2)
    {
        I("[Himax]: Mutual block's bank fail\n");
        pf_value = 0x2;
    }
    else if (valuebuf[0]==0xF3)
    {
        I("[Himax]: Self block's bank fail\n");
        pf_value = 0x3;
    }


END_FUNC:
    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 modified for hx8527*/
    // himax_self_test_data_deinit();
    I("%s:OUT", __func__);

    return pf_value;
}

#endif
void himax_set_HSEN_enable(struct i2c_client *client, uint8_t HSEN_enable, bool suspended)
{
    uint8_t buf[4];

    i2c_himax_read(client, 0x8F, buf, 1, DEFAULT_RETRY_CNT);

    if(HSEN_enable == 1 && !suspended)
        buf[0] |= 0x40 ;
    else
        buf[0] &= 0xBF;

    if ( i2c_himax_write(client, 0x8F,buf, 1, DEFAULT_RETRY_CNT) < 0)
        E("%s i2c write fail.\n",__func__);
}

int himax_palm_detect(uint8_t *buf)
{
    int loop_i = 0;
    int base = 0;
    int x = 0;
    int y = 0;
    int w = 0;

    loop_i = 0;
    base = loop_i * 4;
    x = buf[base] << 8 | buf[base + 1];
    y = (buf[base + 2] << 8 | buf[base + 3]);
    w = buf[(private_ts->nFinger_support * 4) + loop_i];
    I(" %s HX_PALM_REPORT_loopi=%d,base=%x,X=%x,Y=%x,W=%x \n",__func__,loop_i,base,x,y,w);

    if((!atomic_read(&private_ts->suspend_mode))&&(x==0xFA5A)&&(y==0xFA5A)&&(w==0x00))
    {
        return NO_ERR;
    }
    else
    {
        return GESTURE_DETECT_FAIL;
    }
}

void himax_set_SMWP_enable(struct i2c_client *client, uint8_t SMWP_enable, bool suspended)
{
    uint8_t buf[4];

    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/07 modified for gesture*/
    if (!suspended) {
        I("%s : device does not suppend, return.", __func__);
        return;
    }
    i2c_himax_read(client, 0x8F, buf, 1, DEFAULT_RETRY_CNT);
    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/07 modified for gesture*/
    //if(SMWP_enable == 1 && suspended)
    if(SMWP_enable == 1)
        buf[0] |= 0x20 ;
    else
        buf[0] &= 0xDF;

    if ( i2c_himax_write(client, 0x8F,buf, 1, DEFAULT_RETRY_CNT) < 0)
        E("%s i2c write fail.\n",__func__);
}

void himax_usb_detect_set(struct i2c_client *client,uint8_t *cable_config)
{
    uint8_t tmp_data[4];
    uint8_t retry_cnt = 0;
    struct himax_ts_data *ts = private_ts;

    cable_config[1] = ts->headset_connected + ts->usb_connected;
    do
    {
        i2c_himax_master_write(client, cable_config,2, DEFAULT_RETRY_CNT);

        i2c_himax_read(client, cable_config[0], tmp_data, 1, DEFAULT_RETRY_CNT);
        //I("%s: tmp_data[0]=%d, retry_cnt=%d \n", __func__, tmp_data[0],retry_cnt);
        retry_cnt++;
    }
    while(tmp_data[0] != cable_config[1] && retry_cnt < HIMAX_REG_RETRY_TIMES);

}

void himax_register_read(struct i2c_client *client, uint8_t *read_addr, int read_length, uint8_t *read_data, bool cfg_flag)
{
    uint8_t outData[4];

    if (cfg_flag)
    {
        outData[0] = 0x14;
        i2c_himax_write(client, 0x8C,&outData[0], 1, DEFAULT_RETRY_CNT);

        msleep(10);

        outData[0] = 0x00;
        outData[1] = read_addr[0];
        i2c_himax_write(client, 0x8B,&outData[0], 2, DEFAULT_RETRY_CNT);
        msleep(10);

        i2c_himax_read(client, 0x5A, read_data, read_length, DEFAULT_RETRY_CNT);
        msleep(10);

        outData[0] = 0x00;
        i2c_himax_write(client, 0x8C,&outData[0], 1, DEFAULT_RETRY_CNT);
    }
    else
    {
        i2c_himax_read(client, read_addr[0], read_data, read_length, DEFAULT_RETRY_CNT);
    }
}

void himax_register_write(struct i2c_client *client, uint8_t *write_addr, int write_length, uint8_t *write_data, bool cfg_flag)
{
    uint8_t outData[4];

    if (cfg_flag)
    {
        outData[0] = 0x14;
        i2c_himax_write(client, 0x8C, &outData[0], 1, DEFAULT_RETRY_CNT);
        msleep(10);

        outData[0] = 0x00;
        outData[1] = write_addr[0];
        i2c_himax_write(client, 0x8B, &outData[0], 2, DEFAULT_RETRY_CNT);
        msleep(10);

        i2c_himax_write(client, 0x40, &write_data[0], write_length, DEFAULT_RETRY_CNT);
        msleep(10);

        outData[0] = 0x00;
        i2c_himax_write(client, 0x8C, &outData[0], 1, DEFAULT_RETRY_CNT);

        I("CMD: FE(%x), %x, %d\n", write_addr[0],write_data[0], write_length);
    }
    else
    {
        i2c_himax_write(client, write_addr[0], &write_data[0], write_length, DEFAULT_RETRY_CNT);
    }
}

void himax_interface_on(struct i2c_client *client)
{
    return;
}

bool wait_wip(struct i2c_client *client, int Timing)
{
    return true;
}

void himax_sense_off(struct i2c_client *client)
{
    i2c_himax_write_command(client, HX_CMD_TSSOFF, DEFAULT_RETRY_CNT);
    msleep(50);

    i2c_himax_write_command(client, HX_CMD_TSSLPIN, DEFAULT_RETRY_CNT);
    msleep(50);
}



void himax_sense_on(struct i2c_client *client, uint8_t FlashMode)
{
    i2c_himax_write_command(client, HX_CMD_TSSON, DEFAULT_RETRY_CNT);
    msleep(30);

    i2c_himax_write_command(client, HX_CMD_TSSLPOUT, DEFAULT_RETRY_CNT);
    msleep(50);
}

static int himax_ManualMode(struct i2c_client *client, int enter)
{
    uint8_t cmd[2];
    cmd[0] = enter;
    if ( i2c_himax_write(client, HX_CMD_MANUALMODE,&cmd[0], 1, 3) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return 0;
    }
    return 0;
}

static int himax_FlashMode(struct i2c_client *client, int enter)
{
    uint8_t cmd[2];
    cmd[0] = enter;
    if ( i2c_himax_write(client, HX_CMD_FLASH_ENABLE,&cmd[0], 1, 3) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return 0;
    }
    return 0;
}

static int himax_lock_flash(struct i2c_client *client, int enable)
{
    uint8_t cmd[5];

    if (i2c_himax_write(client, 0xAA,&cmd[0], 0, 3) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return 0;
    }

    /* lock sequence start */
    cmd[0] = 0x01;
    cmd[1] = 0x00;
    cmd[2] = 0x06;
    if (i2c_himax_write(client, HX_CMD_FLASH_ENABLE,&cmd[0], 3, 3) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return 0;
    }

    cmd[0] = 0x03;
    cmd[1] = 0x00;
    cmd[2] = 0x00;
    if (i2c_himax_write(client, HX_CMD_FLASH_SET_ADDRESS,&cmd[0], 3, 3) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return 0;
    }

    if(enable!=0)
    {
        cmd[0] = 0x63;
        cmd[1] = 0x02;
        cmd[2] = 0x70;
        cmd[3] = 0x03;
    }
    else
    {
        cmd[0] = 0x63;
        cmd[1] = 0x02;
        cmd[2] = 0x30;
        cmd[3] = 0x00;
    }

    if (i2c_himax_write(client, HX_CMD_FLASH_WRITE_REGISTER,&cmd[0], 4, 3) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return 0;
    }

    if (i2c_himax_write_command(client, HX_CMD_4A, 3) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return 0;
    }
    msleep(50);

    if (i2c_himax_write(client, 0xA9,&cmd[0], 0, 3) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return 0;
    }

    return 0;
    /* lock sequence stop */
}


static void himax_changeIref(struct i2c_client *client, int selected_iref)
{

    unsigned char temp_iref[16][2] = {	{0x00,0x00},{0x00,0x00},{0x00,0x00},{0x00,0x00},
        {0x00,0x00},{0x00,0x00},{0x00,0x00},{0x00,0x00},
        {0x00,0x00},{0x00,0x00},{0x00,0x00},{0x00,0x00},
        {0x00,0x00},{0x00,0x00},{0x00,0x00},{0x00,0x00}
    };
    uint8_t cmd[10];
    int i = 0;
    int j = 0;

    I("%s: start to check iref,iref number = %d\n",__func__,selected_iref);

    if (i2c_himax_write(client, 0xAA,&cmd[0], 0, 3) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return;
    }

    for(i=0; i<16; i++)
    {
        for(j=0; j<2; j++)
        {
            if(selected_iref == 1)
            {
                temp_iref[i][j] = E_IrefTable_1[i][j];
            }
            else if(selected_iref == 2)
            {
                temp_iref[i][j] = E_IrefTable_2[i][j];
            }
            else if(selected_iref == 3)
            {
                temp_iref[i][j] = E_IrefTable_3[i][j];
            }
            else if(selected_iref == 4)
            {
                temp_iref[i][j] = E_IrefTable_4[i][j];
            }
            else if(selected_iref == 5)
            {
                temp_iref[i][j] = E_IrefTable_5[i][j];
            }
            else if(selected_iref == 6)
            {
                temp_iref[i][j] = E_IrefTable_6[i][j];
            }
            else if(selected_iref == 7)
            {
                temp_iref[i][j] = E_IrefTable_7[i][j];
            }
        }
    }

    if(!iref_found)
    {
        //Read Iref
        //Register 0x43
        cmd[0] = 0x01;
        cmd[1] = 0x00;
        cmd[2] = 0x0A;
        if( i2c_himax_write(client, HX_CMD_FLASH_ENABLE,&cmd[0], 3, 3) < 0)
        {
            E("%s: i2c access fail!\n", __func__);
            return;
        }

        //Register 0x44
        cmd[0] = 0x00;
        cmd[1] = 0x00;
        cmd[2] = 0x00;
        if( i2c_himax_write(client, HX_CMD_FLASH_SET_ADDRESS,&cmd[0], 3, 3) < 0)
        {
            E("%s: i2c access fail!\n", __func__);
            return ;
        }

        //Register 0x46
        if( i2c_himax_write(client, 0x46,&cmd[0], 0, 3) < 0)
        {
            E("%s: i2c access fail!\n", __func__);
            return ;
        }

        //Register 0x59
        if( i2c_himax_read(client, 0x59, cmd, 4, 3) < 0)
        {
            E("%s: i2c access fail!\n", __func__);
            return ;
        }

        //find iref group , default is iref 3
        for (i = 0; i < 16; i++)
        {
            if ((cmd[0] == temp_iref[i][0]) &&
                    (cmd[1] == temp_iref[i][1]))
            {
                iref_number = i;
                iref_found = true;
                break;
            }
        }

        if(!iref_found )
        {
            E("%s: Can't find iref number!\n", __func__);
            return ;
        }
        else
        {
            I("%s: iref_number=%d, cmd[0]=0x%x, cmd[1]=0x%x\n", __func__, iref_number, cmd[0], cmd[1]);
        }
    }

    msleep(5);

    //iref write
    //Register 0x43
    cmd[0] = 0x01;
    cmd[1] = 0x00;
    cmd[2] = 0x06;
    if( i2c_himax_write(client, HX_CMD_FLASH_ENABLE,&cmd[0], 3, 3) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return ;
    }

    //Register 0x44
    cmd[0] = 0x00;
    cmd[1] = 0x00;
    cmd[2] = 0x00;
    if( i2c_himax_write(client, HX_CMD_FLASH_SET_ADDRESS,&cmd[0], 3, 3) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return ;
    }

    //Register 0x45
    cmd[0] = temp_iref[iref_number][0];
    cmd[1] = temp_iref[iref_number][1];
    cmd[2] = 0x17;
    cmd[3] = 0x28;

    if( i2c_himax_write(client, HX_CMD_FLASH_WRITE_REGISTER,&cmd[0], 4, 3) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return ;
    }

    //Register 0x4A
    if( i2c_himax_write(client, HX_CMD_4A,&cmd[0], 0, 3) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return ;
    }

    //Read SFR to check the result
    //Register 0x43
    cmd[0] = 0x01;
    cmd[1] = 0x00;
    cmd[2] = 0x0A;
    if( i2c_himax_write(client, HX_CMD_FLASH_ENABLE,&cmd[0], 3, 3) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return ;
    }

    //Register 0x44
    cmd[0] = 0x00;
    cmd[1] = 0x00;
    cmd[2] = 0x00;
    if( i2c_himax_write(client, HX_CMD_FLASH_SET_ADDRESS,&cmd[0], 3, 3) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return ;
    }

    //Register 0x46
    if( i2c_himax_write(client, 0x46,&cmd[0], 0, 3) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return ;
    }

    //Register 0x59
    if( i2c_himax_read(client, 0x59, cmd, 4, 3) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return ;
    }

    I( "%s:cmd[0]=%d,cmd[1]=%d,temp_iref_1=%d,temp_iref_2=%d\n",__func__, cmd[0], cmd[1], temp_iref[iref_number][0], temp_iref[iref_number][1]);

    if(cmd[0] != temp_iref[iref_number][0] || cmd[1] != temp_iref[iref_number][1])
    {
        E("%s: IREF Read Back is not match.\n", __func__);
        E("%s: Iref [0]=%d,[1]=%d\n", __func__,cmd[0],cmd[1]);
    }
    else
    {
        I("%s: IREF Pass",__func__);
    }

    if (i2c_himax_write(client, 0xA9,&cmd[0], 0, 3) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return;
    }

}

bool himax_calculateChecksum(struct i2c_client *client, bool change_iref)
{

    int iref_flag = 0;
    uint8_t cmd[10];

    memset(cmd, 0x00, sizeof(cmd));

    //Sleep out
    if( i2c_himax_write(client, HX_CMD_TSSLPOUT,&cmd[0], 0, DEFAULT_RETRY_CNT) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return 0;
    }
    msleep(120);

    while(true)
    {

        if(change_iref)
        {
            if(iref_flag == 0)
            {
                himax_changeIref(client,2); //iref 2
            }
            else if(iref_flag == 1)
            {
                himax_changeIref(client,5); //iref 5
            }
            else if(iref_flag == 2)
            {
                himax_changeIref(client,1); //iref 1
            }
            else
            {
                goto CHECK_FAIL;
            }
            iref_flag ++;
        }

        cmd[0] = 0x00;
        cmd[1] = 0x04;
        cmd[2] = 0x0A;
        cmd[3] = 0x02;

        if (i2c_himax_write(client, 0xED,&cmd[0], 4, DEFAULT_RETRY_CNT) < 0)
        {
            E("%s: i2c access fail!\n", __func__);
            return 0;
        }

        //Enable Flash
        cmd[0] = 0x01;
        cmd[1] = 0x00;
        cmd[2] = 0x02;

        if (i2c_himax_write(client, HX_CMD_FLASH_ENABLE,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
        {
            E("%s: i2c access fail!\n", __func__);
            return 0;
        }
        cmd[0] = 0x05;
        if (i2c_himax_write(client, 0xD2,&cmd[0], 1, DEFAULT_RETRY_CNT) < 0)
        {
            E("%s: i2c access fail!\n", __func__);
            return 0;
        }

        cmd[0] = 0x01;
        if (i2c_himax_write(client, 0x53,&cmd[0], 1, DEFAULT_RETRY_CNT) < 0)
        {
            E("%s: i2c access fail!\n", __func__);
            return 0;
        }

        msleep(200);

        if (i2c_himax_read(client, 0xAD, cmd, 4, DEFAULT_RETRY_CNT) < 0)
        {
            E("%s: i2c access fail!\n", __func__);
            return -1;
        }

        I("%s 0xAD[0,1,2,3] = %d,%d,%d,%d \n",__func__,cmd[0],cmd[1],cmd[2],cmd[3]);

        if (cmd[0] == 0 && cmd[1] == 0 && cmd[2] == 0 && cmd[3] == 0 )
        {
            himax_FlashMode(client,0);
            goto CHECK_PASS;
        }
        else
        {
            himax_FlashMode(client,0);
            goto CHECK_FAIL;
        }


CHECK_PASS:
        if(change_iref)
        {
            if(iref_flag < 3)
            {
                continue;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }

CHECK_FAIL:
        return 1;
    }
    return 1;
}

bool himax_flash_lastdata_check(struct i2c_client *client)
{
	return 0;
}

int fts_ctpm_fw_upgrade_with_sys_fs_32k(struct i2c_client *client,unsigned char *fw, int len, bool change_iref)
{
    unsigned char* ImageBuffer = fw;
    int fullFileLength = len;
    int i;
    uint8_t cmd[5], last_byte, prePage;
    int FileLength = 0;
    uint8_t checksumResult = 0;

    I("Enter %s", __func__);
    if (len != 0x8000)   //32k
    {
        E("%s: The file size is not 32K bytes, len = %d \n", __func__, fullFileLength);
        return false;
    }

#ifdef HX_RST_PIN_FUNC
    himax_ic_reset(false,false);
#endif

    FileLength = fullFileLength;

    if ( i2c_himax_write(client, HX_CMD_TSSLPOUT,&cmd[0], 0, DEFAULT_RETRY_CNT) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return 0;
    }

    msleep(120);

    himax_lock_flash(client,0);

    cmd[0] = 0x05;
    cmd[1] = 0x00;
    cmd[2] = 0x02;
    if ( i2c_himax_write(client, HX_CMD_FLASH_ENABLE,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return 0;
    }

    if ( i2c_himax_write(client, 0x4F,&cmd[0], 0, DEFAULT_RETRY_CNT) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return 0;
    }
    msleep(50);

    himax_ManualMode(client,1);
    himax_FlashMode(client,1);

    FileLength = (FileLength + 3) / 4;
    for (i = 0, prePage = 0; i < FileLength; i++)
    {
        last_byte = 0;
        cmd[0] = i & 0x1F;
        if (cmd[0] == 0x1F || i == FileLength - 1)
        {
            last_byte = 1;
        }
        cmd[1] = (i >> 5) & 0x1F;
        cmd[2] = (i >> 10) & 0x1F;
        if ( i2c_himax_write(client, HX_CMD_FLASH_SET_ADDRESS,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
        {
            E("%s: i2c access fail!\n", __func__);
            return 0;
        }

        if (prePage != cmd[1] || i == 0)
        {
            prePage = cmd[1];
            cmd[0] = 0x01;
            cmd[1] = 0x09;//cmd[2] = 0x02;
            if ( i2c_himax_write(client, HX_CMD_FLASH_ENABLE,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
            {
                E("%s: i2c access fail!\n", __func__);
                return 0;
            }

            cmd[0] = 0x01;
            cmd[1] = 0x0D;//cmd[2] = 0x02;
            if ( i2c_himax_write(client, HX_CMD_FLASH_ENABLE,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
            {
                E("%s: i2c access fail!\n", __func__);
                return 0;
            }

            cmd[0] = 0x01;
            cmd[1] = 0x09;//cmd[2] = 0x02;
            if ( i2c_himax_write(client, HX_CMD_FLASH_ENABLE,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
            {
                E("%s: i2c access fail!\n", __func__);
                return 0;
            }
        }

        memcpy(&cmd[0], &ImageBuffer[4*i], 4);
        if ( i2c_himax_write(client, HX_CMD_FLASH_WRITE_REGISTER,&cmd[0], 4, DEFAULT_RETRY_CNT) < 0)
        {
            E("%s: i2c access fail!\n", __func__);
            return 0;
        }

        cmd[0] = 0x01;
        cmd[1] = 0x0D;//cmd[2] = 0x02;
        if ( i2c_himax_write(client, HX_CMD_FLASH_ENABLE,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
        {
            E("%s: i2c access fail!\n", __func__);
            return 0;
        }

        cmd[0] = 0x01;
        cmd[1] = 0x09;//cmd[2] = 0x02;
        if ( i2c_himax_write(client, HX_CMD_FLASH_ENABLE,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
        {
            E("%s: i2c access fail!\n", __func__);
            return 0;
        }

        if (last_byte == 1)
        {
            cmd[0] = 0x01;
            cmd[1] = 0x01;//cmd[2] = 0x02;
            if ( i2c_himax_write(client, HX_CMD_FLASH_ENABLE,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
            {
                E("%s: i2c access fail!\n", __func__);
                return 0;
            }

            cmd[0] = 0x01;
            cmd[1] = 0x05;//cmd[2] = 0x02;
            if ( i2c_himax_write(client, HX_CMD_FLASH_ENABLE,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
            {
                E("%s: i2c access fail!\n", __func__);
                return 0;
            }

            cmd[0] = 0x01;
            cmd[1] = 0x01;//cmd[2] = 0x02;
            if ( i2c_himax_write(client, HX_CMD_FLASH_ENABLE,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
            {
                E("%s: i2c access fail!\n", __func__);
                return 0;
            }

            cmd[0] = 0x01;
            cmd[1] = 0x00;//cmd[2] = 0x02;
            if ( i2c_himax_write(client, HX_CMD_FLASH_ENABLE,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
            {
                E("%s: i2c access fail!\n", __func__);
                return 0;
            }

            msleep(10);
            if (i == (FileLength - 1))
            {
                himax_FlashMode(client,0);
                himax_ManualMode(client,0);
                checksumResult = himax_calculateChecksum(client,change_iref);//
                //himax_ManualMode(client,0);
                himax_lock_flash(client,1);

                if (!checksumResult) //Success
                {
                    return 1;
                }
                else //Fail
                {
                    E("%s: checksumResult fail!\n", __func__);
                    return 0;
                }
            }
        }
    }
    return 0;
}

int fts_ctpm_fw_upgrade_with_sys_fs_60k(struct i2c_client *client, unsigned char *fw, int len, bool change_iref)
{

    return 0;
}

int fts_ctpm_fw_upgrade_with_sys_fs_64k(struct i2c_client *client, unsigned char *fw, int len, bool change_iref)
{

    return 0;
}

int fts_ctpm_fw_upgrade_with_sys_fs_124k(struct i2c_client *client, unsigned char *fw, int len, bool change_iref)
{

    return 0;
}

int fts_ctpm_fw_upgrade_with_sys_fs_128k(struct i2c_client *client, unsigned char *fw, int len, bool change_iref)
{

    return 0;
}

void himax_touch_information(struct i2c_client *client)
{
#ifndef HX_FIX_TOUCH_INFO
    char data[12] = {0};

    I("%s:IC_TYPE =%d\n", __func__,IC_TYPE);

    if(IC_TYPE == HX_85XX_ES_SERIES_PWON)
    {
        data[0] = 0x8C;
        data[1] = 0x14;
        i2c_himax_master_write(client, &data[0],2,DEFAULT_RETRY_CNT);
        msleep(10);
        data[0] = 0x8B;
        data[1] = 0x00;
        data[2] = 0x70;
        i2c_himax_master_write(client, &data[0],3,DEFAULT_RETRY_CNT);
        msleep(10);
        i2c_himax_read(client, 0x5A, data, 12, DEFAULT_RETRY_CNT);
        ic_data->HX_RX_NUM = data[0];				 // FE(70)
        ic_data->HX_TX_NUM = data[1];				 // FE(71)
        ic_data->HX_MAX_PT = (data[2] & 0xF0) >> 4; // FE(72)
#ifdef HX_EN_SEL_BUTTON
        ic_data->HX_BT_NUM = (data[2] & 0x0F); //FE(72)
#endif
        /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/07 modified for gesture*/
        if((data[4] & 0x01) == 0x01)  //FE(74)
        {
            ic_data->HX_XY_REVERSE = true;
            ic_data->HX_Y_RES = data[6]*256 + data[7]; //FE(76),FE(77)
            ic_data->HX_X_RES = data[8]*256 + data[9]; //FE(78),FE(79)
        }
        else
        {
            ic_data->HX_XY_REVERSE = false;
            ic_data->HX_X_RES = data[6]*256 + data[7]; //FE(76),FE(77)
            ic_data->HX_Y_RES = data[8]*256 + data[9]; //FE(78),FE(79)
        }
        data[0] = 0x8C;
        data[1] = 0x00;
        i2c_himax_master_write(client, &data[0],2,DEFAULT_RETRY_CNT);
        msleep(10);
#ifdef HX_EN_MUT_BUTTON
        data[0] = 0x8C;
        data[1] = 0x14;
        i2c_himax_master_write(client, &data[0],2,DEFAULT_RETRY_CNT);
        msleep(10);
        data[0] = 0x8B;
        data[1] = 0x00;
        data[2] = 0x64;
        i2c_himax_master_write(client, &data[0],3,DEFAULT_RETRY_CNT);
        msleep(10);
        i2c_himax_read(client, 0x5A, data, 4, DEFAULT_RETRY_CNT);
        ic_data->HX_BT_NUM = (data[0] & 0x03);
        data[0] = 0x8C;
        data[1] = 0x00;
        i2c_himax_master_write(client, &data[0],2,DEFAULT_RETRY_CNT);
        msleep(10);
#endif
#ifdef HX_TP_PROC_2T2R
        data[0] = 0x8C;
        data[1] = 0x14;
        i2c_himax_master_write(client, &data[0],2,DEFAULT_RETRY_CNT);
        msleep(10);

        data[0] = 0x8B;
        data[1] = 0x00;
        data[2] = HX_2T2R_Addr;
        i2c_himax_master_write(client, &data[0],3,DEFAULT_RETRY_CNT);
        msleep(10);

        i2c_himax_read(client, 0x5A, data, 10, DEFAULT_RETRY_CNT);

        ic_data->HX_RX_NUM_2 = data[0];
        ic_data->HX_TX_NUM_2 = data[1];

        I("%s:Touch Panel Type=%d \n", __func__,data[2]);
        if((data[2] & 0x02)==HX_2T2R_en_setting)//2T2R type panel
            Is_2T2R = true;
        else
            Is_2T2R = false;

        data[0] = 0x8C;
        data[1] = 0x00;
        i2c_himax_master_write(client, &data[0],2,DEFAULT_RETRY_CNT);
        msleep(10);
#endif
        data[0] = 0x8C;
        data[1] = 0x14;
        i2c_himax_master_write(client, &data[0],2,DEFAULT_RETRY_CNT);
        msleep(10);
        data[0] = 0x8B;
        data[1] = 0x00;
        data[2] = 0x02;
        i2c_himax_master_write(client, &data[0],3,DEFAULT_RETRY_CNT);
        msleep(10);
        i2c_himax_read(client, 0x5A, data, 10, DEFAULT_RETRY_CNT);
        if((data[1] & 0x01) == 1)  //FE(02)
        {
            ic_data->HX_INT_IS_EDGE = true;
        }
        else
        {
            ic_data->HX_INT_IS_EDGE = false;
        }
        data[0] = 0x8C;
        data[1] = 0x00;
        i2c_himax_master_write(client, &data[0],2,DEFAULT_RETRY_CNT);
        msleep(10);
        I("%s:HX_RX_NUM =%d,HX_TX_NUM =%d,HX_MAX_PT=%d \n", __func__,ic_data->HX_RX_NUM,ic_data->HX_TX_NUM,ic_data->HX_MAX_PT);

        if (i2c_himax_read(client, HX_VER_FW_CFG, data, 1, 3) < 0)
        {
            E("%s: i2c access fail!\n", __func__);
        }
        ic_data->vendor_config_ver = data[0];
        I("config_ver=%x.\n",ic_data->vendor_config_ver);

    }
    else
    {
        ic_data->HX_RX_NUM				= 0;
        ic_data->HX_TX_NUM				= 0;
        ic_data->HX_BT_NUM				= 0;
        ic_data->HX_X_RES				= 0;
        ic_data->HX_Y_RES				= 0;
        ic_data->HX_MAX_PT				= 0;
        ic_data->HX_XY_REVERSE			= false;
        ic_data->HX_INT_IS_EDGE			= false;
    }
#else
    ic_data->HX_RX_NUM				= FIX_HX_RX_NUM;
    ic_data->HX_TX_NUM				= FIX_HX_TX_NUM;
    ic_data->HX_BT_NUM				= FIX_HX_BT_NUM;
    ic_data->HX_X_RES				= FIX_HX_X_RES;
    ic_data->HX_Y_RES				= FIX_HX_Y_RES;
    ic_data->HX_MAX_PT				= FIX_HX_MAX_PT;
    ic_data->HX_XY_REVERSE			= FIX_HX_XY_REVERSE;
    ic_data->HX_INT_IS_EDGE			= FIX_HX_INT_IS_EDGE;
#ifdef HX_TP_PROC_2T2R
    Is_2T2R 						= true;
    ic_data->HX_RX_NUM_2 			= FIX_HX_RX_NUM_2;
    ic_data->HX_TX_NUM_2 			= FIX_HX_TX_NUM_2;
#endif
    I("%s:HX_RX_NUM =%d,HX_TX_NUM =%d,HX_MAX_PT=%d,HX_X_RES =%d,HX_Y_RES =%d,HX_INT_IS_EDGE=%d \n", __func__,ic_data->HX_RX_NUM,ic_data->HX_TX_NUM,ic_data->HX_MAX_PT,ic_data->HX_X_RES,ic_data->HX_Y_RES,ic_data->HX_INT_IS_EDGE);
#endif
}

static int himax_read_Sensor_ID(struct i2c_client *client)
{
    uint8_t val_high[1], val_low[1], ID0=0, ID1=0;
    char data[3];
    const int normalRetry = 10;
    int sensor_id;

    data[0] = 0x56;
    data[1] = 0x02;
    data[2] = 0x02;/*ID pin PULL High*/
    i2c_himax_master_write(client, &data[0],3,normalRetry);
    msleep(1);

    //read id pin high
    i2c_himax_read(client, 0x57, val_high, 1, normalRetry);

    data[0] = 0x56;
    data[1] = 0x01;
    data[2] = 0x01;/*ID pin PULL Low*/
    i2c_himax_master_write(client, &data[0],3,normalRetry);
    msleep(1);

    //read id pin low
    i2c_himax_read(client, 0x57, val_low, 1, normalRetry);

    if((val_high[0] & 0x01) ==0)
        ID0=0x02;/*GND*/
    else if((val_low[0] & 0x01) ==0)
        ID0=0x01;/*Floating*/
    else
        ID0=0x04;/*VCC*/

    if((val_high[0] & 0x02) ==0)
        ID1=0x02;/*GND*/
    else if((val_low[0] & 0x02) ==0)
        ID1=0x01;/*Floating*/
    else
        ID1=0x04;/*VCC*/
    if((ID0==0x04)&&(ID1!=0x04))
    {
        data[0] = 0x56;
        data[1] = 0x02;
        data[2] = 0x01;/*ID pin PULL High,Low*/
        i2c_himax_master_write(client, &data[0],3,normalRetry);
        msleep(1);

    }
    else if((ID0!=0x04)&&(ID1==0x04))
    {
        data[0] = 0x56;
        data[1] = 0x01;
        data[2] = 0x02;/*ID pin PULL Low,High*/
        i2c_himax_master_write(client, &data[0],3,normalRetry);
        msleep(1);

    }
    else if((ID0==0x04)&&(ID1==0x04))
    {
        data[0] = 0x56;
        data[1] = 0x02;
        data[2] = 0x02;/*ID pin PULL High,High*/
        i2c_himax_master_write(client, &data[0],3,normalRetry);
        msleep(1);

    }
    sensor_id=(ID1<<4)|ID0;

    data[0] = 0xE4;
    data[1] = sensor_id;
    i2c_himax_master_write(client, &data[0],2,normalRetry);/*Write to MCU*/
    msleep(1);

    return sensor_id;

}

int himax_read_i2c_status(struct i2c_client *client)
{
    return i2c_error_count;//
}

int himax_read_ic_trigger_type(struct i2c_client *client)
{
    char data[12] = {0};
    int trigger_type = false;

    himax_sense_off(client);
    data[0] = 0x8C;
    data[1] = 0x14;
    i2c_himax_master_write(client, &data[0],2,DEFAULT_RETRY_CNT);
    msleep(10);
    data[0] = 0x8B;
    data[1] = 0x00;
    data[2] = 0x02;
    i2c_himax_master_write(client, &data[0],3,DEFAULT_RETRY_CNT);
    msleep(10);
    i2c_himax_read(client, 0x5A, data, 10, DEFAULT_RETRY_CNT);
    if((data[1] & 0x01) == 1)  //FE(02)
    {
        trigger_type = true;
    }
    else
    {
        trigger_type = false;
    }
    data[0] = 0x8C;
    data[1] = 0x00;
    i2c_himax_master_write(client, &data[0],2,DEFAULT_RETRY_CNT);
    himax_sense_on(client,0x01);

    return trigger_type;
}

void himax_read_FW_ver(struct i2c_client *client)
{
    uint8_t data[64];

    //read fw version
    if (i2c_himax_read(client, HX_VER_FW_MAJ, data, 1, 3) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return;
    }
    ic_data->vendor_fw_ver =  data[0]<<8;

    if (i2c_himax_read(client, HX_VER_FW_MIN, data, 1, 3) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return;
    }
    ic_data->vendor_fw_ver = data[0] | ic_data->vendor_fw_ver;

    //read config version
    if (i2c_himax_read(client, HX_VER_FW_CFG, data, 1, 3) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        return;
    }
    ic_data->vendor_config_ver = data[0];
    //read sensor ID
    ic_data->vendor_sensor_id = himax_read_Sensor_ID(client);

    I("sensor_id=%x.\n",ic_data->vendor_sensor_id);
    I("fw_ver=%x.\n",ic_data->vendor_fw_ver);
    I("config_ver=%x.\n",ic_data->vendor_config_ver);
    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/07 modified for fw*/
    oppo_tp_data.version = ic_data->vendor_config_ver;
    ic_data->vendor_panel_ver = -1;

    ic_data->vendor_cid_maj_ver = -1;
    ic_data->vendor_cid_min_ver = -1;

    return;
}

bool himax_ic_package_check(struct i2c_client *client)
{
    uint8_t cmd[3];

    memset(cmd, 0x00, sizeof(cmd));

	msleep(50);
    if (i2c_himax_read(client, 0xD1, cmd, 3, DEFAULT_RETRY_CNT) < 0)
        return false ;

    if(cmd[0] == 0x05 && cmd[1] == 0x85 &&
            (cmd[2] == 0x25 || cmd[2] == 0x26 || cmd[2] == 0x27 || cmd[2] == 0x28))
    {
        IC_TYPE         		= HX_85XX_ES_SERIES_PWON;
        IC_CHECKSUM 			= HX_TP_BIN_CHECKSUM_CRC;
        //Himax: Set FW and CFG Flash Address
        FW_VER_MAJ_FLASH_ADDR   = 133;  //0x0085
        FW_VER_MAJ_FLASH_LENG   = 1;;
        FW_VER_MIN_FLASH_ADDR   = 134;  //0x0086
        FW_VER_MIN_FLASH_LENG   = 1;
        FW_CFG_VER_FLASH_ADDR	= 132;  //0x0084
        /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/17 add for hx8527*/
        himax_tp_direction_set(OPPO_EN_PORTRAIT);//keep portrait,when boot
        I("Himax IC package 852x ES\n");
    }
    else
    {
        E("Himax IC package incorrect!!PKG[0]=%x,PKG[1]=%x,PKG[2]=%x\n",cmd[0],cmd[1],cmd[2]);
        return false ;
    }
    return true;
}

void himax_power_on_init(struct i2c_client *client)
{
    I("%s:\n", __func__);
    himax_sense_on(client,0x01);
    himax_sense_off(client);
    himax_touch_information(client);
    himax_sense_on(client,0x01);
}

void himax_get_DSRAM_data(struct i2c_client *client, uint8_t *info_data)
{

}

void himax_burst_enable(struct i2c_client *client, uint8_t auto_add_4_byte)
{

}

//ts_work
int cal_data_len(int raw_cnt_rmd, int HX_MAX_PT, int raw_cnt_max)
{
    int RawDataLen;
    if (raw_cnt_rmd != 0x00)
    {
        RawDataLen = 128 - ((HX_MAX_PT+raw_cnt_max+3)*4) - 1;
    }
    else
    {
        RawDataLen = 128 - ((HX_MAX_PT+raw_cnt_max+2)*4) - 1;
    }
    return RawDataLen;
}

bool himax_read_event_stack(struct i2c_client *client, uint8_t *buf, int length)
{
    uint8_t cmd[4];

    if(length > 56)
        length = 128;
    //=====================
    //Read event stack
    //=====================

    cmd[0] = 0x31;
    if ( i2c_himax_read(client, 0x86, buf, length,DEFAULT_RETRY_CNT) < 0)
    {
        E("%s: i2c access fail!\n", __func__);
        goto err_workqueue_out;
    }

    return 1;

err_workqueue_out:
    return 0;
}

/* return checksum result */
bool diag_check_sum( struct himax_report_data *hx_touch_data )
{
    uint16_t check_sum_cal = 0;
    int i;

    //Check 128th byte CRC
    for (i = 0, check_sum_cal = 0; i < (hx_touch_data->touch_all_size - hx_touch_data->touch_info_size); i++)
    {
        check_sum_cal += hx_touch_data->hx_rawdata_buf[i];
    }
    if (check_sum_cal % 0x100 != 0)
    {
        return 0;
    }
    return 1;
}

void diag_parse_raw_data(struct himax_report_data *hx_touch_data,int mul_num, int self_num,uint8_t diag_cmd, int32_t *mutual_data, int32_t *self_data)
{
    int index = 0;
    int temp1, temp2,i;
#if defined(HX_TP_SELF_TEST_DRIVER) || defined(CONFIG_TOUCHSCREEN_HIMAX_ITO_TEST)
    int cnt = 0;
    uint8_t command_F1h_bank[2] = {0xF1, 0x00};
#endif

    //Himax: Check Raw-Data Header
    if (hx_touch_data->hx_rawdata_buf[0] == hx_touch_data->hx_rawdata_buf[1]
            && hx_touch_data->hx_rawdata_buf[1] == hx_touch_data->hx_rawdata_buf[2]
            && hx_touch_data->hx_rawdata_buf[2] == hx_touch_data->hx_rawdata_buf[3]
            && hx_touch_data->hx_rawdata_buf[0] > 0)
    {
        index = (hx_touch_data->hx_rawdata_buf[0] - 1) * hx_touch_data->rawdata_size;
        //I("Header[%d]: %x, %x, %x, %x, mutual: %d, self: %d\n", index, buf[56], buf[57], buf[58], buf[59], mul_num, self_num);
        for (i = 0; i < hx_touch_data->rawdata_size; i++)
        {
            temp1 = index + i;

            if (temp1 < mul_num)
            {
                //mutual
                mutual_data[index + i] = hx_touch_data->hx_rawdata_buf[i + 4]; //4: RawData Header
#if defined(HX_TP_SELF_TEST_DRIVER) || defined(CONFIG_TOUCHSCREEN_HIMAX_ITO_TEST)
                if (Selftest_flag == 1)
                {
                    //mutual_bank[index + i] = hx_touch_data->hx_rawdata_buf[i + 4];
                    raw_data_chk_arr[hx_touch_data->hx_rawdata_buf[0] - 1] = true;
                }
#endif
            }
            else
            {
                //self
                temp1 = i + index;
                temp2 = self_num + mul_num;

                if (temp1 >= temp2)
                {
                    break;
                }

                self_data[i+index-mul_num] = hx_touch_data->hx_rawdata_buf[i + 4]; //4: RawData Header
#if defined(HX_TP_SELF_TEST_DRIVER) || defined(CONFIG_TOUCHSCREEN_HIMAX_ITO_TEST)
                if (Selftest_flag == 1)
                {
                    //self_bank[i+index-mul_num] = hx_touch_data->hx_rawdata_buf[i + 4];
                    raw_data_chk_arr[hx_touch_data->hx_rawdata_buf[0] - 1] = true;
                }
#endif
            }
        }
#if defined(HX_TP_SELF_TEST_DRIVER) || defined(CONFIG_TOUCHSCREEN_HIMAX_ITO_TEST)
        if (Selftest_flag == 1)
        {
            cnt = 0;
            for(i = 0; i < hx_touch_data->rawdata_frame_size; i++)
            {
                if(raw_data_chk_arr[i] == true)
                    cnt++;
            }
            if (cnt == hx_touch_data->rawdata_frame_size)
            {
                I("test_counter = %d\n", test_counter);
                test_counter++;
            }
            if(test_counter == TEST_DATA_TIMES)
            {
				for(i = 0; i < mul_num; i++)
					mutual_bank[i] = (uint16_t)mutual_data[i];
				for(i = 0; i < self_num; i++)
					self_bank[i] = (uint16_t)self_data[i];

                for(i = 0; i < hx_touch_data->rawdata_frame_size; i++)
                {
                    raw_data_chk_arr[i] = false;
                }
                test_counter = 0;
                Selftest_flag = 0;
                g_diag_command = 0;
                command_F1h_bank[1] = 0x00;
                i2c_himax_write(private_ts->client, command_F1h_bank[0],&command_F1h_bank[1], 1, DEFAULT_RETRY_CNT);
                msleep(10);
            }
        }
#endif
    }
}

uint8_t himax_read_DD_status(uint8_t *cmd_set, uint8_t *tmp_data)
{
    return -1;
}

int himax_read_FW_status(uint8_t *state_addr, uint8_t *tmp_addr)
{
    return -1;
}

#ifdef HX_TP_PROC_GUEST_INFO
#define HX_GUEST_INFO_SIZE	10
#define HX_GUEST_INFO_LEN_SIZE	4
int g_guest_info_ongoing 			= 0; //0 stop //1 ongoing
char g_guest_str[10][128];



int himax_guest_info_get_status(void)
{
	return g_guest_info_ongoing;
}
void himax_guest_info_set_status(int setting)
{
	g_guest_info_ongoing = setting;
}

void himax_guest_info_read(uint32_t start_addr,uint8_t *flash_tmp_buffer)
{
}

int himax_read_project_id(void)
{
	int custom_info_temp = 0;
	char *temp_str = "DO NOT support this function!\n";
	himax_guest_info_set_status(1);
	for(custom_info_temp = 0; custom_info_temp < HX_GUEST_INFO_SIZE;custom_info_temp++)
	{
		memcpy(&g_guest_str[custom_info_temp],temp_str,30);
	}
	himax_guest_info_set_status(0);
	return NO_ERR;
}
#endif

#if defined(HX_SMART_WAKEUP)||defined(HX_HIGH_SENSE)||defined(HX_USB_DETECT_GLOBAL)
void himax_resend_cmd_func(bool suspended)
{
    struct himax_ts_data *ts;

    ts = private_ts;

    if(!suspended) /* if entering resume need to sense off first*/
    {
        if (ts->SMWP_enable) {
            himax_int_enable(ts->client->irq,0);
        }
#ifdef HX_RESUME_HW_RESET
        himax_ic_reset(false,false);
#else
        himax_sense_off(ts->client);
#endif
    }

#ifdef HX_SMART_WAKEUP
    himax_set_SMWP_enable(ts->client,ts->SMWP_enable,suspended);
#endif
#ifdef HX_HIGH_SENSE
    himax_set_HSEN_enable(ts->client,ts->HSEN_enable,suspended);
#endif
#ifdef HX_USB_DETECT_GLOBAL
    himax_cable_detect_func(true);
#endif
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/02/12 add for headset mode*/
    himax_headset_detect_func(true);
}

void himax_rst_cmd_recovery_func(bool suspended)
{

}
#endif

void himax_resume_ic_action(struct i2c_client *client)
{
    i2c_himax_write_command(client, HX_CMD_TSSON, DEFAULT_RETRY_CNT);
    msleep(30);

    i2c_himax_write_command(client, HX_CMD_TSSLPOUT, DEFAULT_RETRY_CNT);

    return;
}

void himax_suspend_ic_action(struct i2c_client *client)
{
    i2c_himax_write_command(client, HX_CMD_TSSOFF, DEFAULT_RETRY_CNT);
    msleep(40);

    i2c_himax_write_command(client, HX_CMD_TSSLPIN, DEFAULT_RETRY_CNT);

    return;
}

#ifdef HX_AUTO_UPDATE_FW
int himax_fw_ver_bin(unsigned char *i_CTPM_FW)
{
	I("%s:Entering!\n", __func__);
	if (i_CTPM_FW != NULL) {
		I("Catch fw version in bin file!\n");
		g_i_FW_VER = (i_CTPM_FW[FW_VER_MAJ_FLASH_ADDR] << 8) | i_CTPM_FW[FW_VER_MIN_FLASH_ADDR];
        g_i_CFG_VER = i_CTPM_FW[FW_CFG_VER_FLASH_ADDR];
        g_i_CID_MAJ = 0xFFFFFFFF;
        g_i_CID_MIN = 0xFFFFFFFF;
	} else {
		I("FW data is null!\n");
		return 1;
	}
	return NO_ERR;
}
#endif
