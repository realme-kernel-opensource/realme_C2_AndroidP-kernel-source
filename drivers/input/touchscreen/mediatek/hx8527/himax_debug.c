/* Himax Android Driver Sample Code for debug nodes
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

#include "himax_debug.h"
#include "himax_ic.h"

//struct himax_debug_data* debug_data;
extern uint8_t g_log_pt_cnt;

extern struct himax_ic_data* ic_data;
extern struct himax_ts_data *private_ts;
extern unsigned char	IC_TYPE;
extern unsigned char	IC_CHECKSUM;
extern int himax_input_register(struct himax_ts_data *ts);
#ifdef HX_CHIP_STATUS_MONITOR
extern struct chip_monitor_data *g_chip_monitor_data;
#endif

#ifdef HX_RST_PIN_FUNC
extern void himax_ic_reset(uint8_t loadconfig,uint8_t int_off);
#endif

#ifdef HX_TP_PROC_DIAG
#ifdef HX_TP_PROC_2T2R
extern bool Is_2T2R;
int		HX_RX_NUM_2					= 0;
int 		HX_TX_NUM_2					= 0;
#endif
uint8_t g_diag_arr_num = 0;
#endif

#if defined(HX_SMART_WAKEUP) || defined(HX_INSPECT_LPWUG_TEST)
uint32_t FAKE_POWER_KEY_SEND;
#endif

int g_max_mutual = 0;
int g_min_mutual = 255;
int g_max_self = 0;
int g_min_self = 255;

#if defined(HX_TP_PROC_SELF_TEST) || defined(CONFIG_TOUCHSCREEN_HIMAX_ITO_TEST)
int g_self_test_entered = 0;
#endif

struct timespec timeStart, timeEnd, timeDelta;
int g_switch_mode = 0;
extern void himax_idle_mode(struct i2c_client *client,int disable);
extern int himax_switch_mode(struct i2c_client *client,int mode);
extern void himax_return_event_stack(struct i2c_client *client);
extern void calcDataSize(uint8_t finger_num);

extern int g_f_boot_finish;

#ifdef HX_ZERO_FLASH
extern void himax_0f_operation(struct work_struct *work);
extern void himax_0f_operation_check(void);
extern void himax_sys_reset(void);
#endif

#ifdef HX_AUTO_UPDATE_FW
extern bool g_auto_update_flag;
#endif
//=============================================================================================================
//
//	Segment : Himax PROC Debug Function
//
//=============================================================================================================
#if defined(CONFIG_TOUCHSCREEN_HIMAX_DEBUG)

#if defined(CONFIG_TOUCHSCREEN_HIMAX_ITO_TEST)

static ssize_t himax_ito_test_read(struct file *file, char *buf,
                                   size_t len, loff_t *pos)
{
    ssize_t ret = 0;
    uint8_t result = 0;
    uint8_t status = 0;
	char *temp_buf;

	if(!HX_PROC_SEND_FLAG)
    {
		temp_buf = kzalloc(len,GFP_KERNEL);
        status = ito_get_step_status();

        switch(status)
        {
        case 0:
            ret += snprintf(temp_buf + ret, len - ret, "Step : START_TEST\n");
            break;
        case 1:
            ret += snprintf(temp_buf + ret, len - ret, "Step : RAW_DATA\n");
            break;
        case 2:
            ret += snprintf(temp_buf + ret, len - ret, "Step : PERCENT_TEST\n");
            break;
        case 3:
            ret += snprintf(temp_buf + ret, len - ret, "Step : DEV_TEST\n");
            break;
        case 4:
            ret += snprintf(temp_buf + ret, len - ret, "Step : NOISE_TEST\n");
            break;
        case 9:
            ret += snprintf(temp_buf + ret, len - ret, "Step : END_TEST\n");
            break;
        default:
            ret += snprintf(temp_buf + ret, len - ret, "Step : Null\n");
        }

        result = ito_get_result_status();
        if(result == 0xF)
            ret += snprintf(temp_buf + ret, len - ret, "ITO test is On-going! \n");
        else if(result == 0)
            ret += snprintf(temp_buf + ret, len - ret, "ITO test is Pass! \n");
        else if(result == 2)
            ret += snprintf(temp_buf + ret, len - ret, "Open config file fail! \n");
        else
            ret += snprintf(temp_buf + ret, len - ret, "ITO test is Fail! \n");
        HX_PROC_SEND_FLAG=1;
		if(copy_to_user(buf, temp_buf, len))
            I("%s,here:%d\n",__func__,__LINE__);
		kfree(temp_buf);
    }
    else
        HX_PROC_SEND_FLAG=0;

    return ret;
}

static ssize_t himax_ito_test_write(struct file *file, const char *buff,
                                    size_t len, loff_t *pos)
{
    struct himax_ts_data *ts = private_ts;
    uint8_t result = 0;
    char buf[80] = {0};

    if (len >= 80)
    {
        I("%s: no command exceeds 80 chars.\n", __func__);
        return -EFAULT;
    }
    if (copy_from_user(buf, buff, len))
    {
        return -EFAULT;
    }

    result = ito_get_result_status();
    I("%s: buf = %s, result = %d.\n", __func__, buf, result);

    if(buf[0] == '1' && result != 0xF)
    {
        I("%s: buf[0] = %c.\n", __func__, buf[0]);
        ito_set_step_status(0);
        queue_work(ts->ito_test_wq, &ts->ito_test_work);
    }

    return len;
}

static struct file_operations himax_proc_ito_test_ops =
{
    .owner = THIS_MODULE,
    .read = himax_ito_test_read,
    .write = himax_ito_test_write,
};
#endif

static ssize_t himax_CRC_test_read(struct file *file, char *buf,
                                   size_t len, loff_t *pos)
{
    ssize_t ret = 0;
    uint8_t result = 0;
    char *temp_buf;

    if(!HX_PROC_SEND_FLAG)
    {
		temp_buf = kzalloc(len,GFP_KERNEL);
        himax_sense_off(private_ts->client);
        msleep(20);
        result = himax_calculateChecksum(private_ts->client, false);
        himax_sense_on(private_ts->client, 0x01);
        if(result == 0)
            ret += snprintf(temp_buf + ret, len - ret, "CRC test is Pass! \n");
        else
            ret += snprintf(temp_buf + ret, len - ret, "CRC test is Fail! \n");
        if(copy_to_user(buf, temp_buf, len))
            I("%s,here:%d\n",__func__,__LINE__);

		kfree(temp_buf);
        HX_PROC_SEND_FLAG=1;
    }
    else
        HX_PROC_SEND_FLAG=0;

    return ret;
}

static struct file_operations himax_proc_CRC_test_ops =
{
    .owner = THIS_MODULE,
    .read = himax_CRC_test_read,
};

static ssize_t himax_vendor_read(struct file *file, char *buf,
                                 size_t len, loff_t *pos)
{
    ssize_t ret = 0;
    char *temp_buf;

    if(!HX_PROC_SEND_FLAG)
    {
		temp_buf = kzalloc(len,GFP_KERNEL);
        ret += snprintf(temp_buf + ret, len - ret, "FW_VER = 0x%2.2X \n",ic_data->vendor_fw_ver);

        if(IC_TYPE < 8)
            ret += snprintf(temp_buf + ret, len - ret, "CONFIG_VER = 0x%2.2X \n",ic_data->vendor_config_ver);
        else
        {
            ret += snprintf(temp_buf + ret, len - ret, "TOUCH_VER = 0x%2.2X \n",ic_data->vendor_touch_cfg_ver);
            ret += snprintf(temp_buf + ret, len - ret, "DISPLAY_VER = 0x%2.2X \n",ic_data->vendor_display_cfg_ver);
        }
        if(ic_data->vendor_cid_maj_ver < 0 && ic_data->vendor_cid_min_ver < 0)
            ret += snprintf(temp_buf + ret, len - ret, "CID_VER = NULL\n");
        else
            ret += snprintf(temp_buf + ret, len - ret, "CID_VER = 0x%2.2X \n",(ic_data->vendor_cid_maj_ver << 8 | ic_data->vendor_cid_min_ver));

        if(ic_data->vendor_panel_ver < 0)
            ret += snprintf(temp_buf + ret, len - ret, "PANEL_VER = NULL\n");
        else
            ret += snprintf(temp_buf + ret, len - ret, "PANEL_VER = 0x%2.2X \n",ic_data->vendor_panel_ver);

        ret += snprintf(temp_buf + ret, len - ret, "\n");

        ret += snprintf(temp_buf + ret, len - ret, "Himax Touch Driver Version:\n");
        ret += snprintf(temp_buf + ret, len - ret, "%s \n", HIMAX_DRIVER_VER);

        HX_PROC_SEND_FLAG=1;
        if(copy_to_user(buf, temp_buf, len))
            I("%s,here:%d\n",__func__,__LINE__);
        kfree(temp_buf);
    }
    else
        HX_PROC_SEND_FLAG=0;

    return ret;
}

static struct file_operations himax_proc_vendor_ops =
{
    .owner = THIS_MODULE,
    .read = himax_vendor_read,
};

static ssize_t himax_attn_read(struct file *file, char *buf,
                               size_t len, loff_t *pos)
{
    ssize_t ret = 0;
    struct himax_ts_data *ts_data;
    char *temp_buf;

    ts_data = private_ts;

    if(!HX_PROC_SEND_FLAG)
    {
		temp_buf = kzalloc(len,GFP_KERNEL);

        ret += snprintf(temp_buf + ret, len - ret, "attn = %x\n", himax_int_gpio_read(ts_data->pdata->gpio_irq));
        if(copy_to_user(buf, temp_buf, len))
            I("%s,here:%d\n",__func__,__LINE__);
        kfree(temp_buf);
        HX_PROC_SEND_FLAG=1;
    }
    else
        HX_PROC_SEND_FLAG=0;

    return ret;
}


static struct file_operations himax_proc_attn_ops =
{
    .owner = THIS_MODULE,
    .read = himax_attn_read,
};

static ssize_t himax_int_en_read(struct file *file, char *buf,
                                 size_t len, loff_t *pos)
{
    struct himax_ts_data *ts = private_ts;
    size_t ret = 0;
    char *temp_buf;

   if(!HX_PROC_SEND_FLAG)
    {
		temp_buf = kzalloc(len,GFP_KERNEL);

		ret += snprintf(temp_buf + ret, len - ret, "%d ", ts->irq_enabled);
        ret += snprintf(temp_buf + ret, len - ret, "\n");
        if(copy_to_user(buf, temp_buf, len))
            I("%s,here:%d\n",__func__,__LINE__);
        kfree(temp_buf);
        HX_PROC_SEND_FLAG=1;
    }
    else
        HX_PROC_SEND_FLAG=0;
    return ret;
}

static ssize_t himax_int_en_write(struct file *file, const char *buff,
                                  size_t len, loff_t *pos)
{
    struct himax_ts_data *ts = private_ts;
    char buf_tmp[12]= {0};
    int value, ret=0;

    if (len >= 12)
    {
        I("%s: no command exceeds 12 chars.\n", __func__);
        return -EFAULT;
    }
    if (copy_from_user(buf_tmp, buff, len))
    {
        return -EFAULT;
    }

    if (buf_tmp[0] == '0')
        value = false;
    else if (buf_tmp[0] == '1')
        value = true;
    else
        return -EINVAL;

    if (value)
    {
        ret = himax_int_en_set(ts->client);
        if (ret == 0)
        {
            ts->irq_enabled = 1;
            irq_enable_count = 1;
        }
    }
    else
    {
        himax_int_enable(ts->client->irq,0);
        free_irq(ts->client->irq, ts);
        ts->irq_enabled = 0;
    }

    return len;
}

static struct file_operations himax_proc_int_en_ops =
{
    .owner = THIS_MODULE,
    .read = himax_int_en_read,
    .write = himax_int_en_write,
};

static ssize_t himax_layout_read(struct file *file, char *buf,
                                 size_t len, loff_t *pos)
{
    struct himax_ts_data *ts = private_ts;
    size_t ret = 0;
    char *temp_buf;

    if(!HX_PROC_SEND_FLAG)
    {
		temp_buf = kzalloc(len,GFP_KERNEL);

        ret += snprintf(temp_buf + ret, len - ret, "%d ", ts->pdata->abs_x_min);
        ret += snprintf(temp_buf + ret, len - ret, "%d ", ts->pdata->abs_x_max);
        ret += snprintf(temp_buf + ret, len - ret, "%d ", ts->pdata->abs_y_min);
        ret += snprintf(temp_buf + ret, len - ret, "%d ", ts->pdata->abs_y_max);
        ret += snprintf(temp_buf + ret, len - ret, "\n");
        if(copy_to_user(buf, temp_buf, len))
            I("%s,here:%d\n",__func__,__LINE__);
        kfree(temp_buf);
        HX_PROC_SEND_FLAG=1;
    }
    else
        HX_PROC_SEND_FLAG=0;

    return ret;
}

static ssize_t himax_layout_write(struct file *file, const char *buff,
                                  size_t len, loff_t *pos)
{
    struct himax_ts_data *ts = private_ts;
    char buf_tmp[5];
    int i = 0, j = 0, k = 0, ret;
    unsigned long value;
    int layout[4] = {0};
    char buf[80] = {0};

    if (len >= 80)
    {
        I("%s: no command exceeds 80 chars.\n", __func__);
        return -EFAULT;
    }
    if (copy_from_user(buf, buff, len))
    {
        return -EFAULT;
    }

    for (i = 0; i < 20; i++)
    {
        if (buf[i] == ',' || buf[i] == '\n')
        {
            memset(buf_tmp, 0x0, sizeof(buf_tmp));
            if (i - j <= 5)
                memcpy(buf_tmp, buf + j, i - j);
            else
            {
                I("buffer size is over 5 char\n");
                return len;
            }
            j = i + 1;
            if (k < 4)
            {
                ret = kstrtoul(buf_tmp, 10, &value);
                layout[k++] = value;
            }
        }
    }
    if (k == 4)
    {
        ts->pdata->abs_x_min=layout[0];
        ts->pdata->abs_x_max=layout[1];
        ts->pdata->abs_y_min=layout[2];
        ts->pdata->abs_y_max=layout[3];
        I("%d, %d, %d, %d\n",
          ts->pdata->abs_x_min, ts->pdata->abs_x_max, ts->pdata->abs_y_min, ts->pdata->abs_y_max);
        input_unregister_device(ts->input_dev);
        himax_input_register(ts);
    }
    else
        I("ERR@%d, %d, %d, %d\n",
          ts->pdata->abs_x_min, ts->pdata->abs_x_max, ts->pdata->abs_y_min, ts->pdata->abs_y_max);
    return len;
}

static struct file_operations himax_proc_layout_ops =
{
    .owner = THIS_MODULE,
    .read = himax_layout_read,
    .write = himax_layout_write,
};

static ssize_t himax_debug_level_read(struct file *file, char *buf,
                                      size_t len, loff_t *pos)
{
    struct himax_ts_data *ts_data;
    size_t ret = 0;
    char *temp_buf;

    ts_data = private_ts;

    if(*pos)
        return 0;

    temp_buf = kzalloc(len,GFP_KERNEL);

    ret += snprintf(temp_buf + ret, len - ret, "%d\n", ts_data->debug_log_level);
    if(copy_to_user(buf, temp_buf, len))
        I("%s,here:%d\n",__func__,__LINE__);
    kfree(temp_buf);

    *pos += ret;

    return ret;
}

static ssize_t himax_debug_level_write(struct file *file, const char *buff,
                                       size_t len, loff_t *pos)
{
    struct himax_ts_data *ts;
    char buf_tmp[11];
    int i;
    ts = private_ts;

    if (len >= 12)
    {
        I("%s: no command exceeds 12 chars.\n", __func__);
        return -EFAULT;
    }
    if (copy_from_user(buf_tmp, buff, len))
    {
        return -EFAULT;
    }

    ts->debug_log_level = 0;
    for(i=0; i<len-1; i++)
    {
        if( buf_tmp[i]>='0' && buf_tmp[i]<='9' )
            ts->debug_log_level |= (buf_tmp[i]-'0');
        else if( buf_tmp[i]>='A' && buf_tmp[i]<='F' )
            ts->debug_log_level |= (buf_tmp[i]-'A'+10);
        else if( buf_tmp[i]>='a' && buf_tmp[i]<='f' )
            ts->debug_log_level |= (buf_tmp[i]-'a'+10);

        if(i!=len-2)
            ts->debug_log_level <<= 4;
    }

    if (ts->debug_log_level & BIT(3))
    {
        if (ts->pdata->screenWidth > 0 && ts->pdata->screenHeight > 0 &&
                (ts->pdata->abs_x_max - ts->pdata->abs_x_min) > 0 &&
                (ts->pdata->abs_y_max - ts->pdata->abs_y_min) > 0)
        {
            ts->widthFactor = (ts->pdata->screenWidth << SHIFTBITS)/(ts->pdata->abs_x_max - ts->pdata->abs_x_min);
            ts->heightFactor = (ts->pdata->screenHeight << SHIFTBITS)/(ts->pdata->abs_y_max - ts->pdata->abs_y_min);
            if (ts->widthFactor > 0 && ts->heightFactor > 0)
                ts->useScreenRes = 1;
            else
            {
                ts->heightFactor = 0;
                ts->widthFactor = 0;
                ts->useScreenRes = 0;
            }
        }
        else
            I("Enable finger debug with raw position mode!\n");
    }
    else
    {
        ts->useScreenRes = 0;
        ts->widthFactor = 0;
        ts->heightFactor = 0;
    }

    return len;
}

static struct file_operations himax_proc_debug_level_ops =
{
    .owner = THIS_MODULE,
    .read = himax_debug_level_read,
    .write = himax_debug_level_write,
};

#ifdef HX_TP_PROC_REGISTER
static ssize_t himax_proc_register_read(struct file *file, char *buf,
                                        size_t len, loff_t *pos)
{
    int ret = 0;
    uint16_t loop_i;
    uint8_t data[128];
    char *temp_buf;

    if(*pos)
        return 0;

    memset(data, 0x00, sizeof(data));

    temp_buf = kzalloc(len,GFP_KERNEL);

    I("himax_register_show: %02X,%02X,%02X,%02X\n", register_command[3],register_command[2],register_command[1],register_command[0]);
    himax_register_read(private_ts->client, register_command, 128, data, cfg_flag);

    ret += snprintf(temp_buf + ret, len - ret, "command:  %02X,%02X,%02X,%02X\n", register_command[3],register_command[2],register_command[1],register_command[0]);

    for (loop_i = 0; loop_i < 128; loop_i++)
    {
        ret += snprintf(temp_buf + ret, len - ret, "0x%2.2X ", data[loop_i]);
        if ((loop_i % 16) == 15)
            ret += snprintf(temp_buf + ret, len - ret, "\n");
    }
    ret += snprintf(temp_buf + ret, len - ret, "\n");
    if(copy_to_user(buf, temp_buf, len))
        I("%s,here:%d\n",__func__,__LINE__);
    kfree(temp_buf);

    *pos += ret;

    return ret;
}

static ssize_t himax_proc_register_write(struct file *file, const char *buff,
        size_t len, loff_t *pos)
{
    char *buf = NULL;
    char buf_tmp[16];
    uint8_t length = 0;
    unsigned long result    = 0;
    uint8_t loop_i          = 0;
    uint16_t base           = 2;
    char *data_str = NULL;
    uint8_t w_data[20];
    uint8_t x_pos[20];
    uint8_t count = 0;

    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/26 modified for coverity CID 67730*/
    buf = kzalloc(len, GFP_KERNEL);
    if (buf == NULL) {
        E("allocate the memory fail\n");
        return -ENOMEM;
    }

    if (copy_from_user(buf, buff, len))
    {
        E("input value error\n");
        kfree(buf);
        return -EFAULT;
    }

    memset(buf_tmp, 0x0, sizeof(buf_tmp));
    memset(w_data, 0x0, sizeof(w_data));
    memset(x_pos, 0x0, sizeof(x_pos));
    memset(register_command, 0x0, sizeof(register_command));

    I("himax %s \n",buf);

    if ((buf[0] == 'r' || buf[0] == 'w') && buf[1] == ':' && buf[2] == 'x')
    {

        length = strlen(buf);

        //I("%s: length = %d.\n", __func__,length);
        for (loop_i = 0; loop_i < length; loop_i++) //find postion of 'x'
        {
            if(buf[loop_i] == 'x')
            {
                x_pos[count] = loop_i;
                count++;
            }
        }

        data_str = strrchr(buf, 'x');
        I("%s: %s.\n", __func__,data_str);
        length = strlen(data_str+1) - 1;

        if (buf[0] == 'r')
        {
            if (buf[3] == 'F' && buf[4] == 'E' && length == 4)
            {
                length = length - base;
                cfg_flag = true;
                memcpy(buf_tmp, data_str + base +1, length);
            }
            else
            {
                cfg_flag = false;
                memcpy(buf_tmp, data_str + 1, length);
            }

            byte_length = length/2;
            if (!kstrtoul(buf_tmp, 16, &result))
            {
                for (loop_i = 0 ; loop_i < byte_length ; loop_i++)
                {
                    register_command[loop_i] = (uint8_t)(result >> loop_i*8);
                }
            }
        }
        else if (buf[0] == 'w')
        {
            if (buf[3] == 'F' && buf[4] == 'E')
            {
                cfg_flag = true;
                memcpy(buf_tmp, buf + base + 3, length);
            }
            else
            {
                cfg_flag = false;
                memcpy(buf_tmp, buf + 3, length);
            }
            if(count < 3)
            {
                byte_length = length/2;
                if (!kstrtoul(buf_tmp, 16, &result)) //command
                {
                    for (loop_i = 0 ; loop_i < byte_length ; loop_i++)
                    {
                        register_command[loop_i] = (uint8_t)(result >> loop_i*8);
                    }
                }
                if (!kstrtoul(data_str+1, 16, &result)) //data
                {
                    for (loop_i = 0 ; loop_i < byte_length ; loop_i++)
                    {
                        w_data[loop_i] = (uint8_t)(result >> loop_i*8);
                    }
                }
                himax_register_write(private_ts->client, register_command, byte_length, w_data, cfg_flag);
            }
            else
            {
                for (loop_i = 0; loop_i < count; loop_i++) //parsing addr after 'x'
                {
                    memset(buf_tmp, 0x0, sizeof(buf_tmp));
                    if(cfg_flag != 0 && loop_i != 0)
                        byte_length = 2;
                    else
                        byte_length = x_pos[1] - x_pos[0] - 2; //original
                    memcpy(buf_tmp, buf + x_pos[loop_i] + 1, byte_length);
                    //I("%s: buf_tmp = %s\n", __func__,buf_tmp);
                    if (!kstrtoul(buf_tmp, 16, &result))
                    {
                        if(loop_i == 0)
                        {
                            register_command[loop_i] = (uint8_t)(result);
                            //I("%s: register_command = %X\n", __func__,register_command[0]);
                        }
                        else
                        {
                            w_data[loop_i - 1] = (uint8_t)(result);
                            //I("%s: w_data[%d] = %2X\n", __func__,loop_i - 1,w_data[loop_i - 1]);
                        }
                    }
                }

                byte_length = count - 1;
                himax_register_write(private_ts->client, register_command, byte_length, &w_data[0], cfg_flag);
            }
        }
        else {
                kfree(buf);
                return len;
            }
    }
    kfree(buf);
    return len;
}

static struct file_operations himax_proc_register_ops =
{
    .owner = THIS_MODULE,
    .read = himax_proc_register_read,
    .write = himax_proc_register_write,
};
#endif

#ifdef HX_TP_PROC_DIAG
int32_t *getMutualBuffer(void)
{
    return diag_mutual;
}
int32_t *getMutualNewBuffer(void)
{
    return diag_mutual_new;
}
int32_t *getMutualOldBuffer(void)
{
    return diag_mutual_old;
}
int32_t *getSelfBuffer(void)
{
    return &diag_self[0];
}
uint8_t getXChannel(void)
{
    return x_channel;
}
uint8_t getYChannel(void)
{
    return y_channel;
}
uint8_t getDiagCommand(void)
{
    return g_diag_command;
}
void setXChannel(uint8_t x)
{
    x_channel = x;
}
void setYChannel(uint8_t y)
{
    y_channel = y;
}
void setMutualBuffer(void)
{
	diag_mutual = kzalloc(x_channel * y_channel * sizeof(int32_t), GFP_KERNEL);
}
void setMutualNewBuffer(void)
{
	diag_mutual_new = kzalloc(x_channel * y_channel * sizeof(int32_t), GFP_KERNEL);
}
void setMutualOldBuffer(void)
{
	diag_mutual_old = kzalloc(x_channel * y_channel * sizeof(int32_t), GFP_KERNEL);
}

#ifdef HX_TP_PROC_2T2R
int32_t *getMutualBuffer_2(void)
{
    return diag_mutual_2;
}
uint8_t getXChannel_2(void)
{
    return x_channel_2;
}
uint8_t getYChannel_2(void)
{
    return y_channel_2;
}
void setXChannel_2(uint8_t x)
{
    x_channel_2 = x;
}
void setYChannel_2(uint8_t y)
{
    y_channel_2 = y;
}
void setMutualBuffer_2(void)
{
    diag_mutual_2 = kzalloc(x_channel_2 * y_channel_2 * sizeof(int32_t), GFP_KERNEL);
}
#endif

#ifdef HX_TP_PROC_DIAG
int himax_set_diag_cmd(struct himax_ic_data *ic_data,struct himax_report_data *hx_touch_data)
{
	int32_t *mutual_data;
	int32_t *self_data;
    int 	mul_num;
    int 	self_num;
    //int RawDataLen = 0;

    hx_touch_data->diag_cmd = getDiagCommand();
    if (hx_touch_data->diag_cmd >= 1 && hx_touch_data->diag_cmd <= 7)
    {
        //Check event stack CRC
        if(!diag_check_sum(hx_touch_data))
        {
            goto bypass_checksum_failed_packet;
        }
#ifdef HX_TP_PROC_2T2R
        if(Is_2T2R && (hx_touch_data->diag_cmd >= 4 && hx_touch_data->diag_cmd <= 6))
        {
            mutual_data = getMutualBuffer_2();
            self_data 	= getSelfBuffer();

            // initiallize the block number of mutual and self
            mul_num = getXChannel_2() * getYChannel_2();

#ifdef HX_EN_SEL_BUTTON
            self_num = getXChannel_2() + getYChannel_2() + ic_data->HX_BT_NUM;
#else
            self_num = getXChannel_2() + getYChannel_2();
#endif
        }
        else
#endif
        {
            mutual_data = getMutualBuffer();
            self_data 	= getSelfBuffer();

            // initiallize the block number of mutual and self
            mul_num = getXChannel() * getYChannel();

#ifdef HX_EN_SEL_BUTTON
            self_num = getXChannel() + getYChannel() + ic_data->HX_BT_NUM;
#else
            self_num = getXChannel() + getYChannel();
#endif
        }

        diag_parse_raw_data(hx_touch_data,mul_num, self_num,hx_touch_data->diag_cmd, mutual_data,self_data);

    }
    else if (hx_touch_data->diag_cmd == 8)
    {
        memset(diag_coor, 0x00, sizeof(diag_coor));
        memcpy(&(diag_coor[0]), &hx_touch_data->hx_coord_buf[0], hx_touch_data->touch_info_size);
    }
    //assign state info data
    memcpy(&(hx_state_info[0]), &hx_touch_data->hx_state_info[0], 2);

    return NO_ERR;

bypass_checksum_failed_packet:
    return 1;
}
#endif
//#if defined(HX_DEBUG_LEVEL)
void himax_log_touch_data(uint8_t *buf,struct himax_report_data *hx_touch_data)
{
    int loop_i = 0;
	int j = 0;
    int print_size = 0;
	char rawlog[96];
	int32_t loc = 0;

    if(!hx_touch_data->diag_cmd)
        print_size = hx_touch_data->touch_info_size;
    else
        print_size = hx_touch_data->touch_all_size;

    for (loop_i = 0; loop_i < print_size; loop_i+=8)
    {
        if((loop_i + 7) >= print_size)
        {
            I("%s: over flow\n",__func__);
            break;
        }
        /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 modified for hx8527*/
		memset(rawlog, 0x00, sizeof(rawlog));
		for (j = 0, loc = 0; j < 8; j++)
			loc += snprintf(rawlog + loc, sizeof(rawlog) - loc, "[%2d] 0x%2.2X ", loop_i + j, buf[loop_i + j]);
        /*I("P%2d = 0x%2.2X P%2d = 0x%2.2X ", loop_i, buf[loop_i], loop_i + 1, buf[loop_i + 1]);
        I("P%2d = 0x%2.2X P%2d = 0x%2.2X ", loop_i + 2, buf[loop_i + 2], loop_i + 3, buf[loop_i + 3]);
        I("P%2d = 0x%2.2X P%2d = 0x%2.2X ", loop_i + 4, buf[loop_i + 4], loop_i + 5, buf[loop_i + 5]);
        I("P%2d = 0x%2.2X P%2d = 0x%2.2X ", loop_i + 6, buf[loop_i + 6], loop_i + 7, buf[loop_i + 7]);*/
        I("Raw: %s\n", rawlog);
    }
}
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 modified for hx8527*
 *Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/03/06 modified for debug_level*/
void himax_log_touch_event(int x, int y, int w, int loop_i ,int touched)
{
    struct himax_ts_data *ts = private_ts;

    if(touched == HX_FINGER_ON) {
        if(g_oppo_debug_level != 2) {
            I("Finger %d=> X:%d, Y:%d W:%d, Z:%d, F:%d\n", loop_i + 1, x, y, w, w, loop_i + 1);
            return;
        }
        if (ts->useScreenRes)
        {
            I("status: Screen:F:%02d Down, X:%d, Y:%d, W:%d\n",
            loop_i + 1, x * ts->widthFactor >> SHIFTBITS,
            y * ts->heightFactor >> SHIFTBITS, w);
        }
        else
        {
            I("status: Raw:F:%02d Down, X:%d, Y:%d, W:%d\n",
            loop_i + 1, x, y, w);
        }
    }
    else if(touched == HX_FINGER_LEAVE){
        if(g_oppo_debug_level != 2) {
            I("All Finger leave\n");
            return;
        }
        if (ts->useScreenRes)
        {
            I("status: Screen:F:%02d Up, X:%d, Y:%d\n",
            loop_i + 1, ts->pre_finger_data[loop_i][0] * ts->widthFactor >> SHIFTBITS,
            ts->pre_finger_data[loop_i][1] * ts->heightFactor >> SHIFTBITS);
        }
        else
        {
            I("status: Raw:F:%02d Up, X:%d, Y:%d\n",
            loop_i + 1, ts->pre_finger_data[loop_i][0],
            ts->pre_finger_data[loop_i][1]);
        }
    }
    else
        I("%s : wrong input!\n", __func__);

    g_log_pt_cnt = 0;
}
void himax_log_touch_int_devation(int touched)
{
    if(touched == HX_FINGER_ON)
    {
        getnstimeofday(&timeStart);
        /* I(" Irq start time = %ld.%06ld s\n",
        	timeStart.tv_sec, timeStart.tv_nsec/1000); */
    }
    else if(touched == HX_FINGER_LEAVE)
    {
        getnstimeofday(&timeEnd);
        timeDelta.tv_nsec = (timeEnd.tv_sec*1000000000+timeEnd.tv_nsec) - (timeStart.tv_sec*1000000000+timeStart.tv_nsec);
        /*I("Irq finish time = %ld.%06ld s\n",
        	timeEnd.tv_sec, timeEnd.tv_nsec/1000);*/
        I("Touch latency = %ld us\n", timeDelta.tv_nsec/1000);
    }
    else
        I("%s : wrong input!\n",__func__);
}
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 modified for hx8527*/
void himax_log_touch_event_detail(struct himax_ts_data *ts,int x,int y,int w,int loop_i,int touched,uint16_t old_finger)
{

    if (touched == HX_FINGER_ON)
    {
        if (old_finger >> loop_i == 0)
        {
            if (ts->useScreenRes)
            {
                I("status: Screen:F:%02d Down, X:%d, Y:%d, W:%d\n",
                  loop_i+1, x * ts->widthFactor >> SHIFTBITS,
                  y * ts->heightFactor >> SHIFTBITS, w);
            }
            else
            {
                I("status: Raw:F:%02d Down, X:%d, Y:%d, W:%d\n",
                  loop_i+1, x, y, w);
            }
        }
    }
    else if(touched == HX_FINGER_LEAVE)//if (old_finger >> loop_i == 1)
    {
        if (old_finger >> loop_i == 1)
        {
            if (ts->useScreenRes)
            {
                I("status: Screen:F:%02d Up, X:%d, Y:%d\n",
                  loop_i+1, ts->pre_finger_data[loop_i][0] * ts->widthFactor >> SHIFTBITS,
                  ts->pre_finger_data[loop_i][1] * ts->heightFactor >> SHIFTBITS);
            }
            else
            {
                I("status: Raw:F:%02d Up, X:%d, Y:%d\n",
                  loop_i+1, ts->pre_finger_data[loop_i][0],
                  ts->pre_finger_data[loop_i][1]);
            }
        }
    }
    else
    {
        I("%s : wrong input!\n",__func__);
    }
}
//#endif
static ssize_t himax_diag_arrange_write(struct file *file, const char *buff,
                                        size_t len, loff_t *pos)
{

    char buf[80] = {0};

    if (len >= 80)
    {
        I("%s: no command exceeds 80 chars.\n", __func__);
        return -EFAULT;
    }
    if (copy_from_user(buf, buff, len))
    {
        return -EFAULT;
    }

    g_diag_arr_num = buf[0] - '0';
    I("%s: g_diag_arr_num = %d \n", __func__,g_diag_arr_num);

    return len;
}

void himax_get_mutual_edge(void)
{
    int i = 0;
    for(i = 0; i < (x_channel * y_channel); i++)
    {
        if(diag_mutual[i] > g_max_mutual)
            g_max_mutual = diag_mutual[i];
        if(diag_mutual[i] < g_min_mutual)
            g_min_mutual = diag_mutual[i];
    }
}

void himax_get_self_edge(void)
{
    int i = 0;
    for(i = 0; i < (x_channel + y_channel); i++)
    {
        if(diag_self[i] > g_max_self)
            g_max_self = diag_self[i];
        if(diag_self[i] < g_min_self)
            g_min_self = diag_self[i];
    }
}

/* print first step which is row */
static struct file_operations himax_proc_diag_arrange_ops =
{
    .owner = THIS_MODULE,
    .write = himax_diag_arrange_write,
};

/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 add for hx8527*/
#define snprintf_check(b,c,r,fmt, ...) {\
    if (r >= c)	{return;}\
    r += snprintf((b+r), (c-r), fmt, ##__VA_ARGS__);\
}
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 modified for hx8527*/
static void fill_status_info(char *buffer, size_t cap)
{
    size_t ret = 0;
    if (!cap)
        return;

    snprintf_check(buffer, cap, ret, "ReCal = %d\t", hx_state_info[0] & 0x03);
    snprintf_check(buffer, cap, ret, "Base Line = %d\t", hx_state_info[0] >> 2 & 0x01);
    snprintf_check(buffer, cap, ret, "Palm = %d\t", hx_state_info[0] >> 3 & 0x01);
    snprintf_check(buffer, cap, ret, "Idle mode = %d\t", hx_state_info[0] >> 4 & 0x01);
    snprintf_check(buffer, cap, ret, "Water = %d\n", hx_state_info[0] >> 5 & 0x01);
    snprintf_check(buffer, cap, ret, "TX Hop = %d\t", hx_state_info[0] >> 6 & 0x01);
    snprintf_check(buffer, cap, ret, "AC mode = %d\t", hx_state_info[0] >> 7 & 0x01);
    snprintf_check(buffer, cap, ret, "Glove = %d\t", hx_state_info[1] & 0x01);
    snprintf_check(buffer, cap, ret, "Stylus = %d\t", hx_state_info[1] >> 1 & 0x01);
    snprintf_check(buffer, cap, ret, "Hovering = %d\t", hx_state_info[1] >> 2 & 0x01);
    snprintf_check(buffer, cap, ret, "Proximity = %d\t", hx_state_info[1] >> 3 & 0x01);
    snprintf_check(buffer, cap, ret, "KEY = %d\n", hx_state_info[1] >> 4 & 0x0F);
}

static void print_state_info(struct seq_file *s)
{
    char info[148] = {0};
    fill_status_info(info, sizeof(info));
    seq_printf(s, "%s", info);
}

static void himax_diag_arrange_print(struct seq_file *s, int i, int j, int transpose)
{

    if(transpose)
        seq_printf(s, "%6d", diag_mutual[ j + i*x_channel]);
    else
        seq_printf(s, "%6d", diag_mutual[ i + j*x_channel]);
}

/* ready to print second step which is column*/
static void himax_diag_arrange_inloop(struct seq_file *s, int in_init,int out_init,bool transpose, int j)
{
    int i;
    int in_max = 0;

    if(transpose)
        in_max = y_channel;
    else
        in_max = x_channel;

    if (in_init > 0) //bit0 = 1
    {
        for(i = in_init-1; i >= 0; i--)
        {
            himax_diag_arrange_print(s, i, j, transpose);
        }
        if(transpose)
        {
            if(out_init > 0)
                seq_printf(s, " %5d\n", diag_self[j]);
            else
                seq_printf(s, " %5d\n", diag_self[x_channel - j - 1]);
        }
    }
    else//bit0 = 0
    {
        for (i = 0; i < in_max; i++)
        {
            himax_diag_arrange_print(s, i, j, transpose);
        }
        if(transpose)
        {
            if(out_init > 0)
                seq_printf(s, " %5d\n", diag_self[x_channel - j - 1]);
            else
                seq_printf(s, " %5d\n", diag_self[j]);
        }
    }
}

/* print first step which is row */
static void himax_diag_arrange_outloop(struct seq_file *s, int transpose, int out_init, int in_init)
{
    int j;
    int out_max = 0;
    int self_cnt = 0;

    if(transpose)
        out_max = x_channel;
    else
        out_max = y_channel;

    if(out_init > 0) //bit1 = 1
    {
        self_cnt = 1;
        for(j = out_init-1; j >= 0; j--)
        {
            seq_printf(s, "%3c%02d%c",'[', j + 1,']');
            himax_diag_arrange_inloop(s, in_init, out_init, transpose, j);
            if(!transpose)
            {
                seq_printf(s, " %5d\n", diag_self[y_channel + x_channel - self_cnt]);
                self_cnt++;
            }
        }
    }
    else//bit1 = 0
    {
        //self_cnt = x_channel;
        for(j = 0; j < out_max; j++)
        {
            seq_printf(s, "%3c%02d%c",'[', j + 1,']');
            himax_diag_arrange_inloop(s, in_init, out_init, transpose, j);
            if(!transpose)
            {
                seq_printf(s, " %5d\n", diag_self[j + x_channel]);
            }
        }
    }
}

/* determin the output format of diag */
static void himax_diag_arrange(struct seq_file *s)
{
    int bit2,bit1,bit0;
    int i;

    /* rotate bit */
    bit2 = g_diag_arr_num >> 2;
    /* reverse Y */
    bit1 = g_diag_arr_num >> 1 & 0x1;
    /* reverse X */
    bit0 = g_diag_arr_num & 0x1;

    if (g_diag_arr_num < 4)
    {
        for (i = 0 ; i <= x_channel; i++)
            seq_printf(s, "%3c%02d%c",'[', i,']');
        seq_printf(s,"\n");
        himax_diag_arrange_outloop(s, bit2, bit1 * y_channel, bit0 * x_channel);
        seq_printf(s, "%6c",' ');
        if(bit0 == 1)
        {
            for (i = x_channel - 1; i >= 0; i--)
                seq_printf(s, "%6d", diag_self[i]);
        }
        else
        {
            for (i = 0; i < x_channel; i++)
                seq_printf(s, "%6d", diag_self[i]);
        }
    }
    else
    {
        for (i = 0 ; i <= y_channel; i++)
            seq_printf(s, "%3c%02d%c",'[', i,']');
        seq_printf(s,"\n");
        himax_diag_arrange_outloop(s, bit2, bit1 * x_channel, bit0 * y_channel);
        seq_printf(s, "%6c",' ');
        if(bit1 == 1)
        {
            for (i = x_channel + y_channel - 1; i >= x_channel; i--)
            {
                seq_printf(s, "%6d", diag_self[i]);
            }
        }
        else
        {
            for (i = x_channel; i < x_channel + y_channel; i++)
            {
                seq_printf(s, "%6d", diag_self[i]);
            }
        }
    }
}

static void *himax_diag_seq_start(struct seq_file *s, loff_t *pos)
{
    if (*pos>=1) return NULL;
    return (void *)((unsigned long) *pos+1);
}

static void *himax_diag_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    return NULL;
}
static void himax_diag_seq_stop(struct seq_file *s, void *v)
{
}
static int himax_diag_seq_read(struct seq_file *s, void *v)
{
    size_t ret = 0;
    uint32_t loop_i;
    uint16_t mutual_num, self_num, width;
    int dsram_type = 0;

    dsram_type = g_diag_command/10;

#ifdef HX_TP_PROC_2T2R
    if(Is_2T2R &&(g_diag_command >= 4 && g_diag_command <= 6))
    {
        mutual_num	= x_channel_2 * y_channel_2;
        self_num	= x_channel_2 + y_channel_2; //don't add KEY_COUNT
        width		= x_channel_2;
        seq_printf(s, "ChannelStart: %4d, %4d\n\n", x_channel_2, y_channel_2);
    }
    else
#endif
    {
        mutual_num	= x_channel * y_channel;
        self_num	= x_channel + y_channel; //don't add KEY_COUNT
        width		= x_channel;
        seq_printf(s, "ChannelStart: %4d, %4d\n\n", x_channel, y_channel);
    }

    // start to show out the raw data in adb shell
    if ((g_diag_command >= 1 && g_diag_command <= 3) || (g_diag_command == 7))
    {
        himax_diag_arrange(s);
        seq_printf(s, "\n");
#ifdef HX_EN_SEL_BUTTON
        seq_printf(s, "\n");
        for (loop_i = 0; loop_i < ic_data->HX_BT_NUM; loop_i++)
            seq_printf(s, "%6d", diag_self[ic_data->HX_RX_NUM + ic_data->HX_TX_NUM + loop_i]);
#endif
        seq_printf(s, "ChannelEnd");
        seq_printf(s, "\n");
    }
#ifdef HX_TP_PROC_2T2R
    else if(Is_2T2R && g_diag_command >= 4 && g_diag_command <= 6)
    {
        for (loop_i = 0; loop_i <= width; loop_i++) {
            seq_printf(s, "%3c%02d%c",'[', loop_i,']');
        }
        seq_printf(s, "\n");
        seq_printf(s, "%2c[01]",' ');
        for (loop_i = 0; loop_i < mutual_num; loop_i++)
        {
            seq_printf(s, "%6d", diag_mutual_2[loop_i]);
            if ((loop_i % width) == (width - 1)) {
                seq_printf(s, " %6d\n", diag_self[width + loop_i/width]);
                if(loop_i/width < ((mutual_num / width)-1))
                    seq_printf(s, "%3c%02d%c",'[', (loop_i/width + 2),']');
            }
        }
        seq_printf(s, "\n");
        seq_printf(s, "%6c",' ');
        for (loop_i = 0; loop_i < width; loop_i++)
        {
            seq_printf(s, "%6d", diag_self[loop_i]);
            if (((loop_i) % width) == (width - 1))
                seq_printf(s, "\n");
        }

#ifdef HX_EN_SEL_BUTTON
        seq_printf(s, "\n");
        for (loop_i = 0; loop_i < HX_BT_NUM; loop_i++)
            seq_printf(s, "%6d", diag_self[ic_data->HX_RX_NUM_2 + ic_data->HX_TX_NUM_2 + loop_i]);
#endif
        seq_printf(s, "ChannelEnd");
        seq_printf(s, "\n");
    }
#endif
    else if (g_diag_command == 8)
    {
        for (loop_i = 0; loop_i < 128 ; loop_i++)
        {
            if ((loop_i % 16) == 0)
                seq_printf(s, "LineStart:");
            seq_printf(s, "%4x", diag_coor[loop_i]);
            if ((loop_i % 16) == 15)
                seq_printf(s, "\n");
        }
    }
    else if (dsram_type > 0 && dsram_type <= 8)
    {
        himax_diag_arrange(s);
        seq_printf(s, "ChannelEnd");
        seq_printf(s, "\n");
    }
    if((g_diag_command >= 1 && g_diag_command <= 7) || dsram_type > 0)
    {
        /* print Mutual/Slef Maximum and Minimum */
        himax_get_mutual_edge();
        himax_get_self_edge();
        seq_printf(s, "Mutual Max:%3d, Min:%3d\n",g_max_mutual,g_min_mutual);
        seq_printf(s, "Self Max:%3d, Min:%3d\n",g_max_self,g_min_self);

        /* recovery status after print*/
        g_max_mutual = 0;
        g_min_mutual = 255;
        g_max_self = 0;
        g_min_self = 255;
    }
    /*pring state info*/
    print_state_info(s);
    return ret;
}
static struct seq_operations himax_diag_seq_ops =
{
    .start	= himax_diag_seq_start,
    .next	= himax_diag_seq_next,
    .stop	= himax_diag_seq_stop,
    .show	= himax_diag_seq_read,
};
static int himax_diag_proc_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &himax_diag_seq_ops);
};
bool DSRAM_Flag = false;

//DSRAM thread
void himax_ts_diag_func(void)
{
    int i=0, j=0;
    unsigned int index = 0;
    int total_size = ic_data->HX_TX_NUM * ic_data->HX_RX_NUM * 2;
    uint8_t  info_data[total_size];
    int32_t *mutual_data;
    int32_t *mutual_data_new;
    int32_t *mutual_data_old;
    int32_t new_data;
    /* 1:common dsram,2:100 frame Max,3:N-(N-1)frame */
    int dsram_type = 0;
    char temp_buf[20];
    char write_buf[total_size*3];

    mutual_data = NULL;
    mutual_data_new = NULL;
    mutual_data_old = NULL;
    memset(write_buf, '\0', sizeof(write_buf));

    dsram_type = g_diag_command/10;

    I("%s:Entering g_diag_command=%d\n!",__func__,g_diag_command);

    if(dsram_type == 8)
    {
        dsram_type = 1;
        I("%s Sorting Mode run sram type1 ! \n",__func__);
    }

    himax_burst_enable(private_ts->client, 1);
    if(dsram_type == 1 || dsram_type == 2 || dsram_type == 4)
    {
        mutual_data = getMutualBuffer();
    }
    else if(dsram_type == 3)
    {
        mutual_data = getMutualBuffer();
        mutual_data_new = getMutualNewBuffer();
        mutual_data_old = getMutualOldBuffer();
    }
    himax_get_DSRAM_data(private_ts->client, info_data);

    index = 0;
    for (i = 0; i < ic_data->HX_TX_NUM; i++)
    {
        for (j = 0; j < ic_data->HX_RX_NUM; j++)
        {
            new_data = (((int8_t)info_data[index + 1]) << 8 | info_data[index]);
            if(dsram_type == 1 || dsram_type == 4)
            {
                mutual_data[i*ic_data->HX_RX_NUM+j] = new_data;
            }
            else if(dsram_type == 2)
            {
                //Keep max data
                if(mutual_data[i * ic_data->HX_RX_NUM + j] < new_data)
                    mutual_data[i * ic_data->HX_RX_NUM + j] = new_data;
            }
            else if(dsram_type == 3)
            {
                //Cal data for [N]-[N-1] frame
                mutual_data_new[i * ic_data->HX_RX_NUM + j] = new_data;
                mutual_data[i * ic_data->HX_RX_NUM + j] = mutual_data_new[i * ic_data->HX_RX_NUM + j] - mutual_data_old[i * ic_data->HX_RX_NUM + j];
            }
            index += 2;
        }
    }
    if(dsram_type == 3)
    {
        memcpy(mutual_data_old,mutual_data_new,x_channel * y_channel * sizeof(int16_t)); //copy N data to N-1 array
    }
    diag_max_cnt++;
    if(dsram_type >= 1 && dsram_type <= 3)
    {
        queue_delayed_work(private_ts->himax_diag_wq, &private_ts->himax_diag_delay_wrok, 1/10*HZ);
    }
    else if(dsram_type == 4)
    {
        for(i = 0; i < x_channel * y_channel; i++)
        {
            memset(temp_buf, '\0', sizeof(temp_buf));
            if(i == (x_channel * y_channel - 1))
                snprintf(temp_buf, sizeof(temp_buf), "%4d\n\n", mutual_data[i]);
            //I("%s :temp_buf = %s\n",__func__,temp_buf);
            else if(i % x_channel == (x_channel - 1))
                snprintf(temp_buf, sizeof(temp_buf), "%4d\n", mutual_data[i]);
            else
                snprintf(temp_buf, sizeof(temp_buf), "%4d\t", mutual_data[i]);
            //I("%s :mutual_data[%d] = %d, temp_buf = %s\n",__func__, i, mutual_data[i], temp_buf);
            strcat(write_buf, temp_buf);
        }

        //save raw data in file
        if (!IS_ERR(diag_sram_fn))
        {
            I("%s create file and ready to write\n",__func__);
            diag_sram_fn->f_op->write(diag_sram_fn, write_buf, sizeof(write_buf), &diag_sram_fn->f_pos);
            write_counter++;
            if(write_counter < write_max_count)
                queue_delayed_work(private_ts->himax_diag_wq, &private_ts->himax_diag_delay_wrok, 1/10*HZ);
            else
            {
                filp_close(diag_sram_fn,NULL);
                write_counter = 0;
            }
        }
    }
}

static ssize_t himax_diag_write(struct file *filp, const char __user *buff, size_t len, loff_t *data)
{
    char messages[80] = {0};
    uint8_t command[2] = {0x00, 0x00};
    uint8_t receive[1];

    /* 0: common , other: dsram*/
    int storage_type = 0;
    /* 1:IIR,2:DC,3:Bank,4:IIR2,5:IIR2_N,6:FIR2,7:Baseline,8:dump coord */
    int rawdata_type = 0;

    memset(receive, 0x00, sizeof(receive));

    if (len >= 80)
    {
        I("%s: no command exceeds 80 chars.\n", __func__);
        return -EFAULT;
    }
    if (copy_from_user(messages, buff, len))
    {
        return -EFAULT;
    }

    I("%s:g_switch_mode = %d\n",__func__,g_switch_mode);

    if (messages[1] == 0x0A)
    {
        g_diag_command =messages[0] - '0';
    }
    else
    {
        g_diag_command =(messages[0] - '0')*10 + (messages[1] - '0');
    }

    storage_type = himax_determin_diag_storage(g_diag_command);
    rawdata_type = himax_determin_diag_rawdata(g_diag_command);

    if(g_diag_command > 0 && rawdata_type == 0)
    {
        I("[Himax]g_diag_command=0x%x ,storage_type=%d, rawdata_type=%d! Maybe no support!\n"
          ,g_diag_command,storage_type,rawdata_type);
        g_diag_command = 0x00;
    }
    else
        I("[Himax]g_diag_command=0x%x ,storage_type=%d, rawdata_type=%d\n",g_diag_command,storage_type,rawdata_type);

    memset(diag_mutual, 0x00, x_channel * y_channel * sizeof(int32_t)); /* Set data 0 */
    memset(diag_self, 0x00, sizeof(diag_self));

    if (storage_type == 0 && rawdata_type > 0 && rawdata_type < 8)
    {
        I("%s,common\n",__func__);
        if(DSRAM_Flag)
        {
            //1. Clear DSRAM flag
            DSRAM_Flag = false;
            //2. Stop DSRAM thread
            cancel_delayed_work(&private_ts->himax_diag_delay_wrok);
            //3. Enable ISR
            himax_int_enable(private_ts->client->irq,1);

            /*(4) FW leave sram and return to event stack*/
            himax_return_event_stack(private_ts->client);
        }

        if(g_switch_mode == 2)
        {
            himax_idle_mode(private_ts->client,0);
            g_switch_mode = himax_switch_mode(private_ts->client,0);
        }

        if(g_diag_command == 0x04)
        {
#if defined(HX_TP_PROC_2T2R)
            command[0] = g_diag_command;
#else
            g_diag_command = 0x00;
            command[0] = 0x00;
#endif
        }
        else
            command[0] = g_diag_command;
        himax_diag_register_set(private_ts->client, command[0]);
    }
    else if (storage_type > 0 && storage_type < 8 && rawdata_type > 0 && rawdata_type < 8)
    {
        I("%s,dsram\n",__func__);

        diag_max_cnt = 0;

        //0. set diag flag
        if(DSRAM_Flag)
        {
            //(1) Clear DSRAM flag
            DSRAM_Flag = false;

            //(2) Stop DSRAM thread
            cancel_delayed_work(&private_ts->himax_diag_delay_wrok);
            //(3) Enable ISR
            himax_int_enable(private_ts->client->irq,1);

            /*(4) FW leave sram and return to event stack*/
            himax_return_event_stack(private_ts->client);
        }
        /* close sorting if turn on*/
        if(g_switch_mode == 2)
        {
            himax_idle_mode(private_ts->client,0);
            g_switch_mode = himax_switch_mode(private_ts->client,0);
        }

        switch(rawdata_type)
        {
        case 1:
            command[0] = 0x09;  //IIR
            break;
        case 2:
            command[0] = 0x0A;  //RAWDATA
            break;
        case 7:
            command[0] = 0x0B;   //DC
            break;
        default:
            command[0] = 0x00;
            E("%s: Sram no support this type !\n",__func__);
            break;
        }
        himax_diag_register_set(private_ts->client, command[0]);

        //1. Disable ISR
        himax_int_enable(private_ts->client->irq,0);

        //Open file for save raw data log
        if (storage_type == 4)
        {
            switch (rawdata_type)
            {
            case 1:
                diag_sram_fn = filp_open(IIR_DUMP_FILE,O_CREAT | O_WRONLY,0);
                break;
            case 2:
                diag_sram_fn = filp_open(DC_DUMP_FILE,O_CREAT | O_WRONLY,0);
                break;
            case 3:
                diag_sram_fn = filp_open(BANK_DUMP_FILE,O_CREAT | O_WRONLY,0);
                break;
            default:
                I("%s raw data type is not true. raw data type is %d \n",__func__, rawdata_type);
            }
        }

        //2. Start DSRAM thread
        queue_delayed_work(private_ts->himax_diag_wq, &private_ts->himax_diag_delay_wrok, 2*HZ/100);

        I("%s: Start get raw data in DSRAM\n", __func__);
        if (storage_type == 4)
            msleep(6000);
        //3. Set DSRAM flag
        DSRAM_Flag = true;


    }
    else if(storage_type == 8)
    {
        I("Soritng mode!\n");

        if(DSRAM_Flag)
        {
            //1. Clear DSRAM flag
            DSRAM_Flag = false;
            //2. Stop DSRAM thread
            cancel_delayed_work(&private_ts->himax_diag_delay_wrok);
            //3. Enable ISR
            himax_int_enable(private_ts->client->irq,1);

            /*(4) FW leave sram and return to event stack*/
            himax_return_event_stack(private_ts->client);
        }

        himax_idle_mode(private_ts->client,1);
        g_switch_mode = himax_switch_mode(private_ts->client,1);
        if(g_switch_mode == 2)
        {
            if(rawdata_type == 1)
                command[0] = 0x09; //IIR
            else if(rawdata_type == 2)
                command[0] = 0x0A; //DC
            else if(rawdata_type == 7)
                command[0] = 0x08; //BASLINE
            else
            {
                command[0] = 0x00;
                E("%s: Now Sorting Mode does not support this command=%d\n",__func__,g_diag_command);
            }
            himax_diag_register_set(private_ts->client, command[0]);
        }

        queue_delayed_work(private_ts->himax_diag_wq, &private_ts->himax_diag_delay_wrok, 2*HZ/100);
        DSRAM_Flag = true;

    }
    else
    {
        //set diag flag
        if(DSRAM_Flag)
        {
            I("return and cancel sram thread!\n");
            //(1) Clear DSRAM flag
            DSRAM_Flag = false;

            //(2) Stop DSRAM thread
            cancel_delayed_work(&private_ts->himax_diag_delay_wrok);
            //(3) Enable ISR
            himax_int_enable(private_ts->client->irq,1);

            /*(4) FW leave sram and return to event stack*/
            himax_return_event_stack(private_ts->client);
        }

        if(g_switch_mode == 2)
        {
            himax_idle_mode(private_ts->client,0);
            g_switch_mode = himax_switch_mode(private_ts->client,0);
        }

        if(g_diag_command != 0x00)
        {

            E("[Himax]g_diag_command error!diag_command=0x%x so reset\n",g_diag_command);
            command[0] = 0x00;
            if(g_diag_command != 0x08)
                g_diag_command = 0x00;
            himax_diag_register_set(private_ts->client, command[0]);
        }
        else
        {
            command[0] = 0x00;
            g_diag_command = 0x00;
            himax_diag_register_set(private_ts->client, command[0]);
            I("return to normal g_diag_command=0x%x\n",g_diag_command);
        }
    }
    return len;
}

static struct file_operations himax_proc_diag_ops =
{
    .owner = THIS_MODULE,
    .open = himax_diag_proc_open,
    .read = seq_read,
    .write = himax_diag_write,
};
#endif

#ifdef HX_TP_PROC_RESET
static ssize_t himax_reset_write(struct file *file, const char *buff,
                                 size_t len, loff_t *pos)
{
    char buf_tmp[12];

    if (len >= 12)
    {
        I("%s: no command exceeds 12 chars.\n", __func__);
        return -EFAULT;
    }
    if (copy_from_user(buf_tmp, buff, len))
    {
        return -EFAULT;
    }
#ifdef HX_RST_PIN_FUNC
    if (buf_tmp[0] == '1')
        himax_ic_reset(false,false);
    else if (buf_tmp[0] == '2')
        himax_ic_reset(false,true);
    else if (buf_tmp[0] == '3')
        himax_ic_reset(true,false);
    else if (buf_tmp[0] == '4')
        himax_ic_reset(true,true);
    //else if (buf_tmp[0] == '5')
    //ESD_HW_REST();
#endif
    return len;
}

static struct file_operations himax_proc_reset_ops =
{
    .owner = THIS_MODULE,
    .write = himax_reset_write,
};
#endif

#ifdef HX_TP_PROC_DEBUG
static ssize_t himax_debug_read(struct file *file, char *buf,
                                size_t len, loff_t *pos)
{
    size_t ret = 0;
    char *temp_buf;

    if(!HX_PROC_SEND_FLAG)
    {
        temp_buf = kzalloc(len,GFP_KERNEL);

        if (debug_level_cmd == 't')
        {
            if (fw_update_complete)
            {
                ret += snprintf(temp_buf + ret, len - ret, "FW Update Complete ");
            }
            else
            {
                ret += snprintf(temp_buf + ret, len - ret, "FW Update Fail ");
            }
        }
        else if (debug_level_cmd == 'h')
        {
            if (handshaking_result == 0)
            {
                ret += snprintf(temp_buf + ret, len - ret, "Handshaking Result = %d (MCU Running)\n",handshaking_result);
            }
            else if (handshaking_result == 1)
            {
                ret += snprintf(temp_buf + ret, len - ret, "Handshaking Result = %d (MCU Stop)\n",handshaking_result);
            }
            else if (handshaking_result == 2)
            {
                ret += snprintf(temp_buf + ret, len - ret, "Handshaking Result = %d (I2C Error)\n",handshaking_result);
            }
            else
            {
                ret += snprintf(temp_buf + ret, len - ret, "Handshaking Result = error \n");
            }
        }
        else if (debug_level_cmd == 'v')
        {
            ret += snprintf(temp_buf + ret, len - ret, "FW_VER = 0x%2.2X \n",ic_data->vendor_fw_ver);

            if(IC_TYPE < 8)
                ret += snprintf(temp_buf + ret, len - ret, "CONFIG_VER = 0x%2.2X \n",ic_data->vendor_config_ver);
            else
            {
                ret += snprintf(temp_buf + ret, len - ret, "TOUCH_VER = 0x%2.2X \n",ic_data->vendor_touch_cfg_ver);
                ret += snprintf(temp_buf + ret, len - ret, "DISPLAY_VER = 0x%2.2X \n",ic_data->vendor_display_cfg_ver);
            }
            if(ic_data->vendor_cid_maj_ver < 0 && ic_data->vendor_cid_min_ver < 0)
                ret += snprintf(temp_buf + ret, len - ret, "CID_VER = NULL\n");
            else
                ret += snprintf(temp_buf + ret, len - ret, "CID_VER = 0x%2.2X \n",(ic_data->vendor_cid_maj_ver << 8 | ic_data->vendor_cid_min_ver));

            if(ic_data->vendor_panel_ver < 0)
                ret += snprintf(temp_buf + ret, len - ret, "PANEL_VER = NULL\n");
            else
                ret += snprintf(temp_buf + ret, len - ret, "PANEL_VER = 0x%2.2X \n",ic_data->vendor_panel_ver);

            ret += snprintf(temp_buf + ret, len - ret, "\n");

            ret += snprintf(temp_buf + ret, len - ret, "Himax Touch Driver Version:\n");
            ret += snprintf(temp_buf + ret, len - ret, "%s \n", HIMAX_DRIVER_VER);
        }
        else if (debug_level_cmd == 'd')
        {
            ret += snprintf(temp_buf + ret, len - ret, "Himax Touch IC Information :\n");
            switch(IC_TYPE)
            {
            case HX_85XX_D_SERIES_PWON:
                ret += snprintf(temp_buf + ret, len - ret, "IC Type : HX852xD\n");
                break;
            case HX_85XX_E_SERIES_PWON:
                ret += snprintf(temp_buf + ret, len - ret, "IC Type : HX852xE\n");
                break;
            case HX_85XX_ES_SERIES_PWON:
                ret += snprintf(temp_buf + ret, len - ret, "IC Type : HX852xES\n");
                break;
            case HX_85XX_F_SERIES_PWON:
                ret += snprintf(temp_buf + ret, len - ret, "IC Type : HX852xF\n");
                break;
            case HX_83100A_SERIES_PWON:
                ret += snprintf(temp_buf + ret, len - ret, "IC Type : HX83100A\n");
                break;
            case HX_83102A_SERIES_PWON:
                ret += snprintf(temp_buf + ret, len - ret, "IC Type : HX83102A\n");
                break;
            case HX_83102B_SERIES_PWON:
                ret += snprintf(temp_buf + ret, len - ret, "IC Type : HX83102B\n");
                break;
            case HX_83103A_SERIES_PWON:
                ret += snprintf(temp_buf + ret, len - ret, "IC Type : HX83103A\n");
                break;
            case HX_83110A_SERIES_PWON:
                ret += snprintf(temp_buf + ret, len - ret, "IC Type : HX83110A\n");
                break;
            case HX_83110B_SERIES_PWON:
                ret += snprintf(temp_buf + ret, len - ret, "IC Type : HX83110B\n");
                break;
            case HX_83111B_SERIES_PWON:
                ret += snprintf(temp_buf + ret, len - ret, "IC Type : HX83111B\n");
                break;
            case HX_83112A_SERIES_PWON:
                ret += snprintf(temp_buf + ret, len - ret, "IC Type : HX83112A\n");
                break;
            case HX_83112B_SERIES_PWON:
                ret += snprintf(temp_buf + ret, len - ret, "IC Type : HX83112B\n");
                break;
            default:
                ret += snprintf(temp_buf + ret, len - ret, "IC Type error.\n");
            }
            switch(IC_CHECKSUM)
            {
            case HX_TP_BIN_CHECKSUM_SW:
                ret += snprintf(temp_buf + ret, len - ret, "IC Checksum : SW\n");
                break;
            case HX_TP_BIN_CHECKSUM_HW:
                ret += snprintf(temp_buf + ret, len - ret, "IC Checksum : HW\n");
                break;
            case HX_TP_BIN_CHECKSUM_CRC:
                ret += snprintf(temp_buf + ret, len - ret, "IC Checksum : CRC\n");
                break;
            default:
                ret += snprintf(temp_buf + ret, len - ret, "IC Checksum error.\n");
            }

            if (ic_data->HX_INT_IS_EDGE)
            {
                ret += snprintf(temp_buf + ret, len - ret, "Driver register Interrupt : EDGE TIRGGER\n");
            }
            else
            {
                ret += snprintf(temp_buf + ret, len - ret, "Driver register Interrupt : LEVEL TRIGGER\n");
            }
            if (private_ts->protocol_type == PROTOCOL_TYPE_A)
            {
                ret += snprintf(temp_buf + ret, len - ret, "Protocol : TYPE_A\n");
            }
            else
            {
                ret += snprintf(temp_buf + ret, len - ret, "Protocol : TYPE_B\n");
            }
            ret += snprintf(temp_buf + ret, len - ret, "RX Num : %d\n",ic_data->HX_RX_NUM);
            ret += snprintf(temp_buf + ret, len - ret, "TX Num : %d\n",ic_data->HX_TX_NUM);
            ret += snprintf(temp_buf + ret, len - ret, "BT Num : %d\n",ic_data->HX_BT_NUM);
            ret += snprintf(temp_buf + ret, len - ret, "X Resolution : %d\n",ic_data->HX_X_RES);
            ret += snprintf(temp_buf + ret, len - ret, "Y Resolution : %d\n",ic_data->HX_Y_RES);
            ret += snprintf(temp_buf + ret, len - ret, "Max Point : %d\n",ic_data->HX_MAX_PT);
            ret += snprintf(temp_buf + ret, len - ret, "XY reverse : %d\n",ic_data->HX_XY_REVERSE);
#ifdef HX_TP_PROC_2T2R
            if(Is_2T2R)
            {
                ret += snprintf(temp_buf + ret, len - ret, "2T2R panel\n");
                ret += snprintf(temp_buf + ret, len - ret, "RX Num_2 : %d\n",HX_RX_NUM_2);
                ret += snprintf(temp_buf + ret, len - ret, "TX Num_2 : %d\n",HX_TX_NUM_2);
            }
#endif
        }
        else if (debug_level_cmd == 'i')
        {
            if(himax_read_i2c_status(private_ts->client))
                ret += snprintf(temp_buf + ret, len - ret, "I2C communication is bad.\n");
            else
                ret += snprintf(temp_buf + ret, len - ret, "I2C communication is good.\n");
        }
        else if (debug_level_cmd == 'n')
        {
            if(himax_read_ic_trigger_type(private_ts->client) == 1) //Edgd = 1, Level = 0
                ret += snprintf(temp_buf + ret, len - ret, "IC Interrupt type is edge trigger.\n");
            else if(himax_read_ic_trigger_type(private_ts->client) == 0)
                ret += snprintf(temp_buf + ret, len - ret, "IC Interrupt type is level trigger.\n");
            else
                ret += snprintf(temp_buf + ret, len - ret, "Unkown IC trigger type.\n");
            if (ic_data->HX_INT_IS_EDGE)
                ret += snprintf(temp_buf + ret, len - ret, "Driver register Interrupt : EDGE TIRGGER\n");
            else
                ret += snprintf(temp_buf + ret, len - ret, "Driver register Interrupt : LEVEL TRIGGER\n");
        }
#if defined(HX_CHIP_STATUS_MONITOR)
        else if(debug_level_cmd=='c')
        {
            ret += snprintf(temp_buf + ret, len - ret, "chip_monitor :%d\n", g_chip_monitor_data->HX_CHIP_MONITOR_EN);
        }
#endif
        if(copy_to_user(buf, temp_buf, len))
            I("%s,here:%d\n",__func__,__LINE__);
        kfree(temp_buf);
        HX_PROC_SEND_FLAG=1;
    }
    else
        HX_PROC_SEND_FLAG=0;
    return ret;
}

/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 modified for hx8527*/
extern unsigned long FW_VER_MAJ_FLASH_ADDR;
extern unsigned long FW_VER_MIN_FLASH_ADDR;
extern unsigned long FW_CFG_VER_FLASH_ADDR;

static ssize_t himax_fw_update(unsigned char cmd, const char *fw_name, uint32_t chk)
{
    int32_t result = 0;
    int32_t fw_type;
    char fileName[128];
    const struct firmware *fw = NULL;
    uint32_t fw_ver = 0;
    uint32_t cfg_ver = 0;
    uint8_t retry = 0;

    memset(fileName, 0x00, sizeof(fileName));
    himax_int_enable(private_ts->client->irq,0);

#ifdef HX_CHIP_STATUS_MONITOR
        g_chip_monitor_data->HX_CHIP_POLLING_COUNT = 0;
        g_chip_monitor_data->HX_CHIP_MONITOR_EN = 0;
        cancel_delayed_work_sync(&private_ts->himax_chip_monitor);
#endif

    if (cmd != 0)
        debug_level_cmd = cmd;
    fw_update_complete = false;

    memset(fileName, 0, 128);
    // parse the file name
    snprintf(fileName, sizeof(fileName), "%s", fw_name);
    I("%s: upgrade from file(%s) start!\n", __func__, fileName);
    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/28 modified for enable oppo fw update*/
    if ((private_ts->firmware_update_type == 0) | (private_ts->firmware_update_type == 1)) {
        I("himax request_firmware : %s\n", fileName);
        result = request_firmware(&fw, fileName, private_ts->dev);
    } else {
        I(" himax request_firmware_select : %s\n", fileName);
        result = request_firmware_select(&fw, fileName, private_ts->dev);
    }
    if (result != 0) {
        I("fail to request_firmware fwpath: %s (ret:%d)\n", fileName, result);
        return result;
    }
    fw_ver = (fw->data[FW_VER_MAJ_FLASH_ADDR] << 8) | fw->data[FW_VER_MIN_FLASH_ADDR];
    cfg_ver = fw->data[FW_CFG_VER_FLASH_ADDR];
    I("%s: FW image: %02X, %02X, %02X, %02X, fw version = %d, cfg version = %d\n",
        __func__, fw->data[0], fw->data[1], fw->data[2], fw->data[3],
        fw_ver, cfg_ver);
    if (chk) {
        if ((ic_data->vendor_fw_ver != fw_ver) || (ic_data->vendor_config_ver != cfg_ver)) {
            I("firmware version in file and flash are different!going to update...\n");
        } else {
            I("firmware version in file and flash are the same! ignore update.\n");
            result = -1;
            goto UPDATE_END;
        }
    }

        fw_type = (fw->size)/1024;
        // start to upgrade
        himax_int_enable(private_ts->client->irq,0);
        I("Now FW size is : %dk\n",fw_type);
        switch(fw_type)
        {
        case 32:
                /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/04/15 modified for hx8527*/
                do {
                    if (fts_ctpm_fw_upgrade_with_sys_fs_32k(private_ts->client,
                        (unsigned char *)fw->data, fw->size, false) == 0) {
                        E("%s: TP upgrade error, line: %d\n", __func__, __LINE__);
                        fw_update_complete = false;
                    } else {
                        I("%s: TP upgrade OK, line: %d\n", __func__, __LINE__);
                        fw_update_complete = true;
                        break;
                    }
                } while (++retry < 3);
                break;
        case 60:
                if (fts_ctpm_fw_upgrade_with_sys_fs_60k(private_ts->client,
                    (unsigned char *)fw->data, fw->size, false) == 0) {
                        E("%s: TP upgrade error, line: %d\n", __func__, __LINE__);
                        fw_update_complete = false;
                } else {
                    I("%s: TP upgrade OK, line: %d\n", __func__, __LINE__);
                    fw_update_complete = true;
                }
                break;
        case 64:
            if (fts_ctpm_fw_upgrade_with_sys_fs_64k(private_ts->client,
                (unsigned char *)fw->data, fw->size, false) == 0) {
                    E("%s: TP upgrade error, line: %d\n", __func__, __LINE__);
                    fw_update_complete = false;
            } else {
                I("%s: TP upgrade OK, line: %d\n", __func__, __LINE__);
                fw_update_complete = true;
            }
            break;
        case 124:
                if (fts_ctpm_fw_upgrade_with_sys_fs_124k(private_ts->client,
                    (unsigned char *)fw->data, fw->size, false) == 0) {
                        E("%s: TP upgrade error, line: %d\n", __func__, __LINE__);
                        fw_update_complete = false;
                } else {
                    I("%s: TP upgrade OK, line: %d\n", __func__, __LINE__);
                    fw_update_complete = true;
                }
                break;
        case 128:
                if (fts_ctpm_fw_upgrade_with_sys_fs_128k(private_ts->client,
                    (unsigned char *)fw->data, fw->size, false) == 0) {
                        E("%s: TP upgrade error, line: %d\n", __func__, __LINE__);
                        fw_update_complete = false;
                } else {
                    I("%s: TP upgrade OK, line: %d\n", __func__, __LINE__);
                    fw_update_complete = true;
                }
                break;

        default:
                E("%s: Flash command fail: %d\n", __func__, __LINE__);
                fw_update_complete = false;
                break;
            }
UPDATE_END:
            release_firmware(fw);
    himax_read_FW_ver(private_ts->client);
    himax_touch_information(private_ts->client);
    return result;
}

static ssize_t himax_update_fw_version(const unsigned char cmd)
{
    himax_int_enable(private_ts->client->irq,0);
#ifdef HX_RST_PIN_FUNC
    himax_ic_reset(false,false);
#endif
    if (cmd != 0)
        debug_level_cmd = cmd;
    himax_read_FW_ver(private_ts->client);
#ifdef HX_RST_PIN_FUNC
    himax_ic_reset(true,false);
#endif
    himax_int_enable(private_ts->client->irq,1);
    return 0;
}

static ssize_t himax_debug_write(struct file *file, const char *buff,
                                 size_t len, loff_t *pos)
{
    int result = 0;
    char buf[80] = {0};

    if (len >= 80)
    {
        I("%s: no command exceeds 80 chars.\n", __func__);
        return -EFAULT;
    }
    if (copy_from_user(buf, buff, len))
    {
        return -EFAULT;
    }

    if ( buf[0] == 'h') //handshaking
    {
        debug_level_cmd = buf[0];

        himax_int_enable(private_ts->client->irq,0);

        handshaking_result = himax_hand_shaking(private_ts->client); //0:Running, 1:Stop, 2:I2C Fail

        himax_int_enable(private_ts->client->irq,1);

        return len;
    }

    else if ( buf[0] == 'v') //firmware version
    {
        return himax_update_fw_version(buf[0]);
    }

    else if ( buf[0] == 'd') //ic information
    {
        debug_level_cmd = buf[0];
        return len;
    }

    else if (buf[0] == 't')
    {
		result = himax_fw_update(buf[0], &buf[2], 0);
        if (result < 0)
            return result;
        else
            goto firmware_upgrade_done;
            //return count;
    }
    else if (buf[0] == 'i' && buf[1] == '2' && buf[2] == 'c') //i2c commutation
    {
        debug_level_cmd = 'i';
        return len;
    }

    else if (buf[0] == 'i' && buf[1] == 'n' && buf[2] == 't') //INT trigger
    {
        debug_level_cmd = 'n';
        return len;
    }
#if defined(HX_CHIP_STATUS_MONITOR)
    else if(buf[0] == 'c')
    {
        debug_level_cmd = buf[0];
        g_chip_monitor_data->HX_CHIP_POLLING_COUNT = 0;
        g_chip_monitor_data->HX_CHIP_MONITOR_EN = 0;
        cancel_delayed_work_sync(&private_ts->himax_chip_monitor);
        return len;
    }
#endif
#ifdef HX_ZERO_FLASH
    else if(buf[0] == 'z')
    {
        himax_0f_operation_check();
        return len;
    }
    else if(buf[0] == 'p')
    {
        I("NOW debug echo r!\n");
        //himax_program_sram();
        private_ts->himax_0f_update_wq = create_singlethread_workqueue("HMX_update_0f_reuqest_write");
        if (!private_ts->himax_0f_update_wq)
            E(" allocate syn_update_wq failed\n");

        INIT_DELAYED_WORK(&private_ts->work_0f_update, himax_0f_operation);
        queue_delayed_work(private_ts->himax_0f_update_wq, &private_ts->work_0f_update, msecs_to_jiffies(100));
        return len;
    }
    else if(buf[0] == 'x')
    {
        himax_sys_reset();
        return len;
    }
#endif
    /* others,do nothing */
    else
    {
        debug_level_cmd = 0;
        return len;
    }

firmware_upgrade_done:
#ifdef HX_RST_PIN_FUNC
    himax_ic_reset(true,false);
#else
    himax_sense_on(private_ts->client,0);
#endif
    himax_int_enable(private_ts->client->irq,1);

#ifdef HX_CHIP_STATUS_MONITOR
    g_chip_monitor_data->HX_CHIP_POLLING_COUNT = 0;
    g_chip_monitor_data->HX_CHIP_MONITOR_EN = 1;
    queue_delayed_work(private_ts->himax_chip_monitor_wq, &private_ts->himax_chip_monitor, g_chip_monitor_data->HX_POLLING_TIMES*HZ);
#endif

    //todo himax_chip->tp_firmware_upgrade_proceed = 0;
    //todo himax_chip->suspend_state = 0;
    //todo enable_irq(himax_chip->irq);
    return len;
}

static struct file_operations himax_proc_debug_ops =
{
    .owner = THIS_MODULE,
    .read = himax_debug_read,
    .write = himax_debug_write,
};

static ssize_t himax_proc_FW_debug_read(struct file *file, char *buf,
                                        size_t len, loff_t *pos)
{
    int ret = 0;
    uint8_t loop_i = 0;
    uint8_t tmp_data[64];
    char *temp_buf;

    if(!HX_PROC_SEND_FLAG)
    {
        temp_buf = kzalloc(len,GFP_KERNEL);

        cmd_set[0] = 0x01;
        if(himax_read_FW_status(cmd_set, tmp_data) == NO_ERR)
        {
            ret += snprintf(temp_buf + ret, len - ret, "0x%02X%02X%02X%02X :\t",cmd_set[5],cmd_set[4],cmd_set[3],cmd_set[2]);
            for (loop_i = 0; loop_i < cmd_set[1]; loop_i++)
            {
                ret += snprintf(temp_buf + ret, len - ret, "%5d\t", tmp_data[loop_i]);
            }
            ret += snprintf(temp_buf + ret, len - ret, "\n");
        }
        cmd_set[0] = 0x02;
        if(himax_read_FW_status(cmd_set, tmp_data) == NO_ERR)
        {
            for (loop_i = 0; loop_i < cmd_set[1]; loop_i = loop_i + 2)
            {
                if ((loop_i % 16) == 0)
                    ret += snprintf(temp_buf + ret, len - ret, "0x%02X%02X%02X%02X :\t",
                    cmd_set[5],cmd_set[4],cmd_set[3]+(((cmd_set[2]+ loop_i)>>8)&0xFF), (cmd_set[2] + loop_i)&0xFF);
                ret += snprintf(temp_buf + ret, len - ret, "%5d\t", tmp_data[loop_i] + (tmp_data[loop_i + 1] << 8));
                if ((loop_i % 16) == 14)
                    ret += snprintf(temp_buf + ret, len - ret, "\n");
            }

        }
        ret += snprintf(temp_buf + ret, len - ret, "\n");
        if(copy_to_user(buf, temp_buf, len))
            I("%s,here:%d\n",__func__,__LINE__);
        kfree(temp_buf);
        HX_PROC_SEND_FLAG=1;
    }
    else
        HX_PROC_SEND_FLAG=0;
    return ret;
}

static struct file_operations himax_proc_fw_debug_ops =
{
    .owner = THIS_MODULE,
    .read = himax_proc_FW_debug_read,
};

static ssize_t himax_proc_DD_debug_read(struct file *file, char *buf,
                                        size_t len, loff_t *pos)
{
    int ret = 0;
    uint8_t tmp_data[64];
    uint8_t loop_i = 0;
    char *temp_buf;

    if(!HX_PROC_SEND_FLAG)
    {
        temp_buf = kzalloc(len,GFP_KERNEL);

        if(mutual_set_flag == 1)
        {
            if(himax_read_DD_status(cmd_set, tmp_data) == NO_ERR)
            {
                for (loop_i = 0; loop_i < cmd_set[0]; loop_i++)
                {
                    if ((loop_i % 8) == 0)
                        ret += snprintf(temp_buf + ret, len - ret, "0x%02X : ", loop_i);
                    ret += snprintf(temp_buf + ret, len - ret, "0x%02X ", tmp_data[loop_i]);
                    if ((loop_i % 8) == 7)
                        ret += snprintf(temp_buf + ret, len - ret, "\n");
                }
            }
        }
        //else
        ret += snprintf(temp_buf + ret, len - ret, "\n");
        if(copy_to_user(buf, temp_buf, len))
            I("%s,here:%d\n",__func__,__LINE__);
        kfree(temp_buf);
        HX_PROC_SEND_FLAG=1;
    }
    else
        HX_PROC_SEND_FLAG=0;
    return ret;
}

static ssize_t himax_proc_DD_debug_write(struct file *file, const char *buff,
        size_t len, loff_t *pos)
{
    uint8_t i = 0;
    uint8_t cnt = 2;
    unsigned long result = 0;
    char buf_tmp[20];
    char buf_tmp2[4];

    if (len >= 20)
    {
        I("%s: no command exceeds 20 chars.\n", __func__);
        return -EFAULT;
    }
    if (copy_from_user(buf_tmp, buff, len))
    {
        return -EFAULT;
    }
    memset(buf_tmp2, 0x0, sizeof(buf_tmp2));

    if (buf_tmp[2] == 'x' && buf_tmp[6] == 'x' && buf_tmp[10] == 'x')
    {
        mutual_set_flag = 1;
        for (i = 3; i < 12; i = i + 4)
        {
            memcpy(buf_tmp2, buf_tmp + i, 2);
            if (!kstrtoul(buf_tmp2, 16, &result))
                cmd_set[cnt] = (uint8_t)result;
            else
                I("String to oul is fail in cnt = %d, buf_tmp2 = %s",cnt, buf_tmp2);
            cnt--;
        }
        I("cmd_set[2] = %02X, cmd_set[1] = %02X, cmd_set[0] = %02X\n",cmd_set[2],cmd_set[1],cmd_set[0]);
    }
    else
        mutual_set_flag = 0;

    return len;
}

static struct file_operations himax_proc_dd_debug_ops =
{
    .owner = THIS_MODULE,
    .read = himax_proc_DD_debug_read,
    .write = himax_proc_DD_debug_write,
};

#endif

#ifdef HX_TP_PROC_FLASH_DUMP

uint8_t getFlashCommand(void)
{
    return flash_command;
}

static uint8_t getFlashDumpProgress(void)
{
    return flash_progress;
}

static uint8_t getFlashDumpComplete(void)
{
    return flash_dump_complete;
}

static uint8_t getFlashDumpFail(void)
{
    return flash_dump_fail;
}

uint8_t getSysOperation(void)
{
    return sys_operation;
}

static uint8_t getFlashReadStep(void)
{
    return flash_read_step;
}

bool getFlashDumpGoing(void)
{
    return flash_dump_going;
}

void setFlashBuffer(void)
{
    flash_buffer = kzalloc(Flash_Size * sizeof(uint8_t), GFP_KERNEL);
}

void setSysOperation(uint8_t operation)
{
    sys_operation = operation;
}

void setFlashDumpProgress(uint8_t progress)
{
    flash_progress = progress;
    //I("setFlashDumpProgress : progress = %d ,flash_progress = %d \n",progress,flash_progress);
}

void setFlashDumpComplete(uint8_t status)
{
    flash_dump_complete = status;
}

void setFlashDumpFail(uint8_t fail)
{
    flash_dump_fail = fail;
}

static void setFlashCommand(uint8_t command)
{
    flash_command = command;
}

static void setFlashReadStep(uint8_t step)
{
    flash_read_step = step;
}

void setFlashDumpGoing(bool going)
{
    flash_dump_going = going;
}

static ssize_t himax_proc_flash_read(struct file *file, char *buf,
                                     size_t len, loff_t *pos)
{
    ssize_t ret = 0;
    int loop_i;
    uint8_t local_flash_read_step=0;
    uint8_t local_flash_complete = 0;
    uint8_t local_flash_progress = 0;
    uint8_t local_flash_command = 0;
    uint8_t local_flash_fail = 0;
    char *temp_buf;

    local_flash_complete = getFlashDumpComplete();
    local_flash_progress = getFlashDumpProgress();
    local_flash_command = getFlashCommand();
    local_flash_fail = getFlashDumpFail();

    I("flash_progress = %d \n",local_flash_progress);
    if(!HX_PROC_SEND_FLAG)
    {
        temp_buf = kzalloc(len,GFP_KERNEL);
        if (local_flash_fail)
        {
            ret += snprintf(temp_buf + ret, len - ret, "FlashStart:Fail \n");
            ret += snprintf(temp_buf + ret, len - ret, "FlashEnd");
            ret += snprintf(temp_buf + ret, len - ret, "\n");
            if(copy_to_user(buf, temp_buf, len))
                I("%s,here:%d\n",__func__,__LINE__);
            kfree(temp_buf);
            HX_PROC_SEND_FLAG=1;
            return ret;
        }

        if (!local_flash_complete)
        {
            ret += snprintf(temp_buf + ret, len - ret, "FlashStart:Ongoing:0x%2.2x \n",flash_progress);
            ret += snprintf(temp_buf + ret, len - ret, "FlashEnd");
            ret += snprintf(temp_buf + ret, len - ret, "\n");
            if(copy_to_user(buf, temp_buf, len))
                I("%s,here:%d\n",__func__,__LINE__);
            kfree(temp_buf);
            HX_PROC_SEND_FLAG=1;
            return ret;
        }

        if (local_flash_command == 1 && local_flash_complete)
        {
            ret += snprintf(temp_buf + ret, len - ret, "FlashStart:Complete \n");
            ret += snprintf(temp_buf + ret, len - ret, "FlashEnd");
            ret += snprintf(temp_buf + ret, len - ret, "\n");
            if(copy_to_user(buf, temp_buf, len))
                I("%s,here:%d\n",__func__,__LINE__);
            kfree(temp_buf);
            HX_PROC_SEND_FLAG=1;
            return ret;
        }

        if (local_flash_command == 3 && local_flash_complete)
        {
            ret += snprintf(temp_buf + ret, len - ret, "FlashStart: \n");
            for(loop_i = 0; loop_i < 128; loop_i++)
            {
                ret += snprintf(temp_buf + ret, len - ret, "x%2.2x", flash_buffer[loop_i]);
                if ((loop_i % 16) == 15)
                {
                    ret += snprintf(temp_buf + ret, len - ret, "\n");
                }
            }
            ret += snprintf(temp_buf + ret, len - ret, "FlashEnd");
            ret += snprintf(temp_buf + ret, len - ret, "\n");
            if(copy_to_user(buf, temp_buf, len))
                I("%s,here:%d\n",__func__,__LINE__);
            kfree(temp_buf);
            HX_PROC_SEND_FLAG=1;
            return ret;
        }

        //flash command == 0 , report the data
        local_flash_read_step = getFlashReadStep();

        ret += snprintf(temp_buf + ret, len - ret, "FlashStart:%2.2x \n",local_flash_read_step);

        for (loop_i = 0; loop_i < 1024; loop_i++)
        {
            ret += snprintf(temp_buf + ret, len - ret, "x%2.2X", flash_buffer[local_flash_read_step*1024 + loop_i]);

            if ((loop_i % 16) == 15)
            {
                ret += snprintf(temp_buf + ret, len - ret, "\n");
            }
        }

        ret += snprintf(temp_buf + ret, len - ret, "FlashEnd");
        ret += snprintf(temp_buf + ret, len - ret, "\n");
        if(copy_to_user(buf, temp_buf, len))
            I("%s,here:%d\n",__func__,__LINE__);
        kfree(temp_buf);
        HX_PROC_SEND_FLAG=1;
    }
    else
        HX_PROC_SEND_FLAG=0;
    return ret;
}

static ssize_t himax_proc_flash_write(struct file *file, const char *buff,
                                      size_t len, loff_t *pos)
{
    char buf_tmp[6];
    unsigned long result = 0;
    char buf[80] = {0};

    if (len >= 80)
    {
        I("%s: no command exceeds 80 chars.\n", __func__);
        return -EFAULT;
    }
    if (copy_from_user(buf, buff, len))
    {
        return -EFAULT;
    }
    memset(buf_tmp, 0x0, sizeof(buf_tmp));

    I("%s: buf = %s\n", __func__, buf);

    if (getSysOperation() == 1)
    {
        E("%s: PROC is busy , return!\n", __func__);
        return len;
    }

    if (buf[0] == '0')
    {
        setFlashCommand(0);
        if (buf[1] == ':' && buf[2] == 'x')
        {
            memcpy(buf_tmp, buf + 3, 2);
            I("%s: read_Step = %s\n", __func__, buf_tmp);
            if (!kstrtoul(buf_tmp, 16, &result))
            {
                I("%s: read_Step = %lu \n", __func__, result);
                setFlashReadStep(result);
            }
        }
    }
    else if (buf[0] == '1')// 1_32,1_60,1_64,1_24,1_28 for flash size 32k,60k,64k,124k,128k
    {
        setSysOperation(1);
        setFlashCommand(1);
        setFlashDumpProgress(0);
        setFlashDumpComplete(0);
        setFlashDumpFail(0);
        if ((buf[1] == '_' ) && (buf[2] == '3' ) && (buf[3] == '2' ))
        {
            Flash_Size = FW_SIZE_32k;
        }
        else if ((buf[1] == '_' ) && (buf[2] == '6' ))
        {
            if (buf[3] == '0')
            {
                Flash_Size = FW_SIZE_60k;
            }
            else if (buf[3] == '4')
            {
                Flash_Size = FW_SIZE_64k;
            }
        }
        else if ((buf[1] == '_' ) && (buf[2] == '2' ))
        {
            if (buf[3] == '4')
            {
                Flash_Size = FW_SIZE_124k;
            }
            else if (buf[3] == '8')
            {
                Flash_Size = FW_SIZE_128k;
            }
        }
        queue_work(private_ts->flash_wq, &private_ts->flash_work);
    }
    else if (buf[0] == '2') // 2_32,2_60,2_64,2_24,2_28 for flash size 32k,60k,64k,124k,128k
    {
        setSysOperation(1);
        setFlashCommand(2);
        setFlashDumpProgress(0);
        setFlashDumpComplete(0);
        setFlashDumpFail(0);
        if ((buf[1] == '_' ) && (buf[2] == '3' ) && (buf[3] == '2' ))
        {
            Flash_Size = FW_SIZE_32k;
        }
        else if ((buf[1] == '_' ) && (buf[2] == '6' ))
        {
            if (buf[3] == '0')
            {
                Flash_Size = FW_SIZE_60k;
            }
            else if (buf[3] == '4')
            {
                Flash_Size = FW_SIZE_64k;
            }
        }
        else if ((buf[1] == '_' ) && (buf[2] == '2' ))
        {
            if (buf[3] == '4')
            {
                Flash_Size = FW_SIZE_124k;
            }
            else if (buf[3] == '8')
            {
                Flash_Size = FW_SIZE_128k;
            }
        }
        queue_work(private_ts->flash_wq, &private_ts->flash_work);
    }
    return len;
}

static struct file_operations himax_proc_flash_ops =
{
    .owner = THIS_MODULE,
    .read = himax_proc_flash_read,
    .write = himax_proc_flash_write,
};

void himax_ts_flash_func(void)
{
    uint8_t local_flash_command = 0;

    himax_int_enable(private_ts->client->irq,0);
    setFlashDumpGoing(true);

    //sector = getFlashDumpSector();
    //page = getFlashDumpPage();

    local_flash_command = getFlashCommand();

    msleep(100);

    I("%s: local_flash_command = %d enter.\n", __func__,local_flash_command);

    if ((local_flash_command == 1 || local_flash_command == 2)|| (local_flash_command==0x0F))
    {
        himax_flash_dump_func(private_ts->client, local_flash_command,Flash_Size, flash_buffer);
    }

    I("Complete~~~~~~~~~~~~~~~~~~~~~~~\n");

    if (local_flash_command == 2)
    {
        struct file *fn;

        fn = filp_open(FLASH_DUMP_FILE,O_CREAT | O_WRONLY,0);
        if (!IS_ERR(fn))
        {
            I("%s create file and ready to write\n",__func__);
            fn->f_op->write(fn,flash_buffer,Flash_Size*sizeof(uint8_t),&fn->f_pos);
            filp_close(fn,NULL);
        }
    }

    himax_int_enable(private_ts->client->irq,1);
    setFlashDumpGoing(false);

    setFlashDumpComplete(1);
    setSysOperation(0);
    return;

    /*	Flash_Dump_i2c_transfer_error:

    	himax_int_enable(private_ts->client->irq,1);
    	setFlashDumpGoing(false);
    	setFlashDumpComplete(0);
    	setFlashDumpFail(1);
    	setSysOperation(0);
    	return;
    */
}

#endif

#ifdef HX_TP_PROC_SELF_TEST
char *g_hx_crtra_file = HIMAX_TEST_FILE;
char *g_sensor_result_file = HIMAX_TEST_RST_FILE;
char *g_sensor_black_result_file = HIMAX_BLACK_TEST_RST_FILE;
bool oppo_hx_black_screen_test_flag = false;
static ssize_t himax_self_test_read(struct file *file, char *buf,
                                    size_t len, loff_t *pos)
{
    int val = 0x00;
    ssize_t ret = 0;
    char *temp_buf = NULL;

    mutex_lock(&private_ts->himax_mutex);
    I("%s: enter, %d \n", __func__, __LINE__);

    if ((g_hx_crtra_file == NULL) || (g_sensor_result_file == NULL)) 
    {
        E("%s: No file path!\n", __func__);
        mutex_unlock(&private_ts->himax_mutex);
        return -EINVAL;
    }

    if(!HX_PROC_SEND_FLAG)
    {
        temp_buf = kzalloc(len,GFP_KERNEL);

        himax_int_enable(private_ts->client->irq,0);//disable irq
        g_self_test_entered = 1;
        val = himax_chip_self_test(private_ts->client);
#ifdef HX_ESD_RECOVERY
        HX_ESD_RESET_ACTIVATE = 1;
#endif
        himax_int_enable(private_ts->client->irq,1);//enable irq
        himax_sense_on(private_ts->client, 0x01);

        if (val == 0x00)
        {
            ret += snprintf(temp_buf + ret, len - ret, "Self_Test Pass\n");
        }
        else if (val >= 0x01 && val <= 0x03)
        {
            ret += snprintf(temp_buf + ret, len - ret, "Self_Test Fail\n");
        }
                else
                {
                    ret += snprintf(temp_buf + ret, len - ret, "self test data init Fail\n");
                }

        g_self_test_entered = 0;
        if(copy_to_user(buf, temp_buf, len))
            I("%s,here:%d\n",__func__,__LINE__);
        kfree(temp_buf);
        HX_PROC_SEND_FLAG=1;
    }
    else {
        HX_PROC_SEND_FLAG=0;
    }

    mutex_unlock(&private_ts->himax_mutex);
    return ret;
}

/*
static ssize_t himax_chip_self_test_store(struct device *dev,struct device_attribute *attr, const char *buf, size_t count)
{
	char buf_tmp[2];
	unsigned long result = 0;

	memset(buf_tmp, 0x0, sizeof(buf_tmp));
	memcpy(buf_tmp, buf, 2);
	if(!kstrtoul(buf_tmp, 16, &result))
		{
			sel_type = (uint8_t)result;
		}
	I("sel_type = %x \r\n", sel_type);
	return count;
}
*/

static struct file_operations himax_proc_self_test_ops =
{
    .owner = THIS_MODULE,
    .read = himax_self_test_read,
};
#endif

#ifdef HX_TP_PROC_SENSE_ON_OFF
static ssize_t himax_sense_on_off_write(struct file *file, const char *buff,
                                        size_t len, loff_t *pos)
{
    char buf[80] = {0};

    if (len >= 80)
    {
        I("%s: no command exceeds 80 chars.\n", __func__);
        return -EFAULT;
    }
    if (copy_from_user(buf, buff, len))
    {
        return -EFAULT;
    }

    if(buf[0] == '0')
    {
        himax_sense_off(private_ts->client);
        I("Sense off \n");
    }
    else if(buf[0] == '1')
    {
        if(buf[1] == 's')
        {
            himax_sense_on(private_ts->client, 0x00);
            I("Sense on re-map on, run sram \n");
        }
        else
        {
            himax_sense_on(private_ts->client, 0x01);
            I("Sense on re-map off, run flash \n");
        }
    }
    else
    {
        I("Do nothing \n");
    }
    return len;
}

static struct file_operations himax_proc_sense_on_off_ops =
{
    .owner = THIS_MODULE,
    .write = himax_sense_on_off_write,
};
#endif

#ifdef HX_HIGH_SENSE
static ssize_t himax_HSEN_read(struct file *file, char *buf,
                               size_t len, loff_t *pos)
{
    struct himax_ts_data *ts = private_ts;
    size_t ret = 0;
    char *temp_buf;

    if(!HX_PROC_SEND_FLAG)
    {
        temp_buf = kzalloc(len,GFP_KERNEL);

        ret += snprintf(temp_buf + ret, len - ret, "%d\n", ts->HSEN_enable);
        if(copy_to_user(buf, temp_buf, len))
            I("%s,here:%d\n",__func__,__LINE__);
        kfree(temp_buf);
        HX_PROC_SEND_FLAG=1;
    }
    else
        HX_PROC_SEND_FLAG=0;
    return ret;
}

static ssize_t himax_HSEN_write(struct file *file, const char *buff,
                                size_t len, loff_t *pos)
{
    struct himax_ts_data *ts = private_ts;
    char buf[80] = {0};


    if (len >= 80)
    {
        I("%s: no command exceeds 80 chars.\n", __func__);
        return -EFAULT;
    }
    if (copy_from_user(buf, buff, len))
    {
        return -EFAULT;
    }

    if (buf[0] == '0')
    {
        ts->HSEN_enable = 0;
    }
    else if (buf[0] == '1')
    {
        ts->HSEN_enable = 1;
    }
    else
        return -EINVAL;

    himax_set_HSEN_enable(ts->client, ts->HSEN_enable, ts->suspended);

    I("%s: HSEN_enable = %d.\n", __func__, ts->HSEN_enable);

    return len;
}

static struct file_operations himax_proc_HSEN_ops =
{
    .owner = THIS_MODULE,
    .read = himax_HSEN_read,
    .write = himax_HSEN_write,
};
#endif

#ifdef HX_SMART_WAKEUP
static ssize_t himax_SMWP_read(struct file *file, char *buf,
                               size_t len, loff_t *pos)
{
    size_t ret = 0;
    struct himax_ts_data *ts = private_ts;
    char *temp_buf;

    if(!HX_PROC_SEND_FLAG)
    {
        temp_buf = kzalloc(len,GFP_KERNEL);

        ret += snprintf(temp_buf + ret, len - ret, "%d\n", ts->SMWP_enable);
        if(copy_to_user(buf, temp_buf, len))
            I("%s,here:%d\n",__func__,__LINE__);
        kfree(temp_buf);
        HX_PROC_SEND_FLAG=1;
    }
    else
        HX_PROC_SEND_FLAG=0;

    return ret;
}

static ssize_t himax_SMWP_write(struct file *file, const char *buff,
                                size_t len, loff_t *pos)
{
    struct himax_ts_data *ts = private_ts;
    char buf[80] = {0};

    if (len >= 80)
    {
        I("%s: no command exceeds 80 chars.\n", __func__);
        return -EFAULT;
    }
    if (copy_from_user(buf, buff, len))
    {
        return -EFAULT;
    }


    if (buf[0] == '0')
    {
        ts->SMWP_enable = 0;
    }
    else if (buf[0] == '1')
    {
        ts->SMWP_enable = 1;
    }
    else
        return -EINVAL;

    himax_set_SMWP_enable(ts->client, ts->SMWP_enable, ts->suspended);

    HX_SMWP_EN = ts->SMWP_enable;
    I("%s: SMART_WAKEUP_enable = %d.\n", __func__, HX_SMWP_EN);

    return len;
}

static struct file_operations himax_proc_SMWP_ops =
{
    .owner = THIS_MODULE,
    .read = himax_SMWP_read,
    .write = himax_SMWP_write,
};
#endif

#ifdef HX_ESD_RECOVERY
static ssize_t himax_esd_cnt_read(struct file *file, char *buf,
                                  size_t len, loff_t *pos)
{
    ssize_t ret = 0;
    char *temp_buf;

    I("%s: enter, %d \n", __func__, __LINE__);
    if(!HX_PROC_SEND_FLAG)
    {
        temp_buf = kzalloc(len,GFP_KERNEL);

        ret += snprintf(temp_buf + ret, len - ret, "EB_cnt = %d, EC_cnt = %d, ED_cnt = %d\n",hx_EB_event_flag, hx_EC_event_flag, hx_ED_event_flag);
        if(copy_to_user(buf, temp_buf, len))
            I("%s,here:%d\n",__func__,__LINE__);
        kfree(temp_buf);
        HX_PROC_SEND_FLAG=1;
    }
    else
        HX_PROC_SEND_FLAG=0;
    return ret;
}

static ssize_t himax_esd_cnt_write(struct file *file, const char *buff,
                                   size_t len, loff_t *pos)
{
    int i =0;
    char buf[12] = {0};

    if (len >= 12)
    {
        I("%s: no command exceeds 80 chars.\n", __func__);
        return -EFAULT;
    }
    if (copy_from_user(buf, buff, len))
    {
        return -EFAULT;
    }

    I("Clear ESD Flag \n");
    if (buf[i] == '0')
    {
        hx_EB_event_flag = 0;
        hx_EC_event_flag = 0;
        hx_ED_event_flag = 0;
    }

    return len;
}

static struct file_operations himax_proc_esd_cnt_ops =
{
    .owner = THIS_MODULE,
    .read = himax_esd_cnt_read,
    .write = himax_esd_cnt_write,
};
#endif

#ifdef HX_TP_PROC_GUEST_INFO
extern char g_guest_str[10][128];



extern int himax_guest_info_get_status(void);
extern void himax_guest_info_set_status(int setting);


static ssize_t himax_proc_guest_info_read(struct file *file, char *buf,
                                     size_t len, loff_t *pos)
{
    int ret = 0;
    int i = 0;
    int j = 0;
    char *temp_buf;

    I("guest info progress\n");
    if(!HX_PROC_SEND_FLAG)
    {
        temp_buf = kzalloc(len,GFP_KERNEL);

        if (himax_guest_info_get_status())
        {
            ret += sprintf(temp_buf + ret, "Not Ready\n");
            if(copy_to_user(buf, temp_buf, len))
                I("%s,here:%d\n",__func__,__LINE__);
            kfree(temp_buf);
            HX_PROC_SEND_FLAG=1;
            return ret;
        }
        for(j = 0;j<4;j++)
        {
            ret += sprintf(temp_buf + ret,"g_guest_str%d:\n",j);
            for(i = 0;i < 128; i++)
            {
                if(i % 16 == 0 && i > 0)
                    ret += sprintf(temp_buf + ret,"\n");
                ret += sprintf(temp_buf + ret,"0x%02X\t",g_guest_str[j][i]);

            }
            ret += sprintf(temp_buf + ret,"\n");
            I("str[%d] %s\n",j,g_guest_str[j]);
        }
        ret += sprintf(temp_buf + ret,"g_guest_str8:\n");
        for(i = 0;i < 128; i++)
        {
            if(i % 16 == 0 && i > 0)
                ret += sprintf(temp_buf + ret,"\n");
            ret += sprintf(temp_buf + ret,"0x%02X\t",g_guest_str[8][i]);

        }
        ret += sprintf(temp_buf + ret,"\n");

        I("str[8] %s\n",g_guest_str[8]);

        ret += sprintf(temp_buf + ret,"g_guest_str9:\n");
        for(i = 0;i < 128; i++)
        {
            if(i % 16 == 0 && i > 0)
                ret += sprintf(temp_buf + ret,"\n");
            ret += sprintf(temp_buf + ret,"0x%02X\t",g_guest_str[9][i]);
        }
        ret += sprintf(temp_buf + ret,"\n");

        I("str[8] %s\n",g_guest_str[9]);

        if(copy_to_user(buf, temp_buf, len))
            I("%s,here:%d\n",__func__,__LINE__);
        kfree(temp_buf);
        HX_PROC_SEND_FLAG=1;
    }
    else
        HX_PROC_SEND_FLAG=0;
    return ret;
}

static ssize_t himax_proc_guest_info_write(struct file *file, const char *buff,
                                      size_t len, loff_t *pos)
{

    //unsigned long result = 0;
    char buf[80] = {0};
    // int shift = 0;

    if (len >= 80)
    {
        I("%s: no command exceeds 80 chars.\n", __func__);
        return -EFAULT;
    }
    if (copy_from_user(buf, buff, len))
    {
        return -EFAULT;
    }

    I("%s: buf = %s\n", __func__, buf);

    if (getSysOperation() == 1)
    {
        E("%s: PROC is busy , return!\n", __func__);
        return len;
    }

    if (buf[0] == 'r')
    {
        I("%s,Test to get",__func__);
         queue_work(private_ts->guest_info_wq, &private_ts->guest_info_work);

    }

    return len;
}

static struct file_operations himax_proc_guest_info_ops =
{
    .owner = THIS_MODULE,
    .read = himax_proc_guest_info_read,
    .write = himax_proc_guest_info_write,
};
#endif

#ifdef HX_TP_PROC_SELF_TEST
extern void himax_pin_reset(void);
uint32_t HX_BASELINE_TEST_ERROR_CNT;

/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 modified for hx8527
 *Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/11 modified for ito*/
static ssize_t himax_baseline_test_read(struct file *file, char *buf,
                                    size_t len, loff_t *pos)
{
    int val=0x00;
    ssize_t ret = 0;
    char temp_buf[30];

    if(*pos)
        return 0;

    mutex_lock(&private_ts->himax_mutex);
    I("%s: enter, %d \n", __func__, __LINE__);

    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 modified for ito*/
    if ((g_hx_crtra_file == NULL) || (g_sensor_result_file == NULL) || (g_sensor_black_result_file == NULL)) 
    {
        E("%s: No file path!\n", __func__);
        mutex_unlock(&private_ts->himax_mutex);
        return -EINVAL;
    }

    himax_int_enable(private_ts->client->irq,0);//disable irq
    if (private_ts->suspended) {
        private_ts->power(1);
        oppo_hx_black_screen_test_flag = true;
    }
    himax_pin_reset();
    g_self_test_entered = 1;
    val = himax_chip_self_test(private_ts->client);
    if (private_ts->suspended)
        private_ts->power(0);
    else {
        himax_ic_reset(1, 0);
        himax_int_enable(private_ts->client->irq,1);//enable irq
        himax_sense_on(private_ts->client, 0x01);
    }
    /*zhangyin@ODM_HQ.BSP.TP.Function, 2018/12/14 modified for ito test result
      zhangyin@ODM_HQ.BSP.TP.Function, 2018/12/14 modified for ito test result
      zhangyin@ODM_HQ.BSP.TP.Function, 2019/02/23 modified for ito test*/
    if (val == 0x00) {
        I("0 error(s) All test passed!\n");
        ret = snprintf(temp_buf, sizeof(temp_buf), "0 error(s) All test passed!\n");
    } else if((val >= 0x01) && (val <= 0x03)) {
        I("%d error(s).\n", HX_BASELINE_TEST_ERROR_CNT);
        ret = snprintf(temp_buf, sizeof(temp_buf), "%d error(s).\n", HX_BASELINE_TEST_ERROR_CNT);
    } else {
        I(" test error(s), data init failed!\n");
        ret = snprintf(temp_buf, sizeof(temp_buf), "test error(s), data init failed!\n");
    }
    g_self_test_entered = 0;
    oppo_hx_black_screen_test_flag = false;
    if(copy_to_user(buf, temp_buf, ret))
        I("%s,here:%d\n",__func__,__LINE__);

    mutex_unlock(&private_ts->himax_mutex);

    *pos += ret;

    return ret;
}

static struct file_operations himax_proc_baseline_test_ops =
{
    .owner = THIS_MODULE,
    .read = himax_baseline_test_read,
};

#endif

/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 modified for hx8527*/
static ssize_t himax_debug_info_common(struct seq_file *s, void *v, uint8_t cmd)
{
    size_t ret = 0;
    uint32_t loop_i;
    uint16_t mutual_num, self_num, width;
    int dsram_type = 0;
 /*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/27 modified for debug info */
    I("%s: enter, %d \n", __func__, __LINE__);
    g_diag_command = cmd;
    himax_diag_register_set(private_ts->client, g_diag_command);
    mdelay(500);
#ifdef HX_TP_PROC_2T2R
    if(Is_2T2R &&(g_diag_command >= 4 && g_diag_command <= 6))
    {
        mutual_num  = x_channel_2 * y_channel_2;
        self_num    = x_channel_2 + y_channel_2; //don't add KEY_COUNT
        width       = x_channel_2;
        seq_printf(s, "ChannelStart: %4d, %4d\n\n", x_channel_2, y_channel_2);
    }
    else
#endif
    {
        mutual_num  = x_channel * y_channel;
        self_num    = x_channel + y_channel; //don't add KEY_COUNT
        width       = x_channel;
        seq_printf(s, "ChannelStart: %4d, %4d\n\n", x_channel, y_channel);
    }

    // start to show out the raw data in adb shell
    if ((g_diag_command >= 1 && g_diag_command <= 3) || (g_diag_command == 7))
    {
        himax_diag_arrange(s);
        seq_printf(s, "\n");
#ifdef HX_EN_SEL_BUTTON
        seq_printf(s, "\n");
        for (loop_i = 0; loop_i < ic_data->HX_BT_NUM; loop_i++)
            seq_printf(s, "%6d", diag_self[ic_data->HX_RX_NUM + ic_data->HX_TX_NUM + loop_i]);
#endif
        seq_printf(s, "ChannelEnd");
        seq_printf(s, "\n");
    }
#ifdef HX_TP_PROC_2T2R
    else if(Is_2T2R && g_diag_command >= 4 && g_diag_command <= 6)
    {
        for (loop_i = 0; loop_i < mutual_num; loop_i++)
        {
            seq_printf(s, "%4d", diag_mutual_2[loop_i]);
            if ((loop_i % width) == (width - 1))
                seq_printf(s, " %4d\n", diag_self[width + loop_i/width]);
        }
        seq_printf(s, "\n");
        for (loop_i = 0; loop_i < width; loop_i++)
        {
            seq_printf(s, "%4d", diag_self[loop_i]);
            if (((loop_i) % width) == (width - 1))
                seq_printf(s, "\n");
        }

#ifdef HX_EN_SEL_BUTTON
        seq_printf(s, "\n");
        for (loop_i = 0; loop_i < HX_BT_NUM; loop_i++)
            seq_printf(s, "%4d", diag_self[ic_data->HX_RX_NUM_2 + ic_data->HX_TX_NUM_2 + loop_i]);
#endif
        seq_printf(s, "ChannelEnd");
        seq_printf(s, "\n");
    }
#endif
    else if (g_diag_command == 8)
    {
        for (loop_i = 0; loop_i < 128 ; loop_i++)
        {
            if ((loop_i % 16) == 0)
                seq_printf(s, "LineStart:");
            seq_printf(s, "%4x", diag_coor[loop_i]);
            if ((loop_i % 16) == 15)
                seq_printf(s, "\n");
        }
    }

    if((g_diag_command >= 1 && g_diag_command <= 7) || dsram_type > 0)
    {
        /* print Mutual/Slef Maximum and Minimum */
        himax_get_mutual_edge();
        himax_get_self_edge();
        seq_printf(s, "Mutual Max:%3d, Min:%3d\n",g_max_mutual,g_min_mutual);
        seq_printf(s, "Self Max:%3d, Min:%3d\n",g_max_self,g_min_self);

        /* recovery status after print*/
        g_max_mutual = 0;
        g_min_mutual = 255;
        g_max_self = 0;
        g_min_self = 255;
    }
    /*pring state info*/
    // print_state_info(s);
    g_diag_command = 0;
    himax_diag_register_set(private_ts->client, g_diag_command);
 /*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/27 modified for debug info */
    I("%s: exit, %d \n", __func__, __LINE__);
    return ret;
}

static int himax_debug_info_baseline_seq_read(struct seq_file *s, void *v)
{
    return himax_debug_info_common(s, v, 2);
}


static struct seq_operations himax_debug_info_baseline_seq_ops =
{
    .start  = himax_diag_seq_start,
    .next   = himax_diag_seq_next,
    .stop   = himax_diag_seq_stop,
    .show   = himax_debug_info_baseline_seq_read,
};

static int himax_debug_info_baseline_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &himax_debug_info_baseline_seq_ops);
}

static struct file_operations himax_proc_debug_info_baseline_ops =
{
    .owner = THIS_MODULE,
    .open = himax_debug_info_baseline_open,
    .read = seq_read,
};

static int himax_debug_info_delta_seq_read(struct seq_file *s, void *v)
{
    return himax_debug_info_common(s, v, 1);
}


static struct seq_operations himax_debug_info_delta_seq_ops =
{
    .start  = himax_diag_seq_start,
    .next   = himax_diag_seq_next,
    .stop   = himax_diag_seq_stop,
    .show   = himax_debug_info_delta_seq_read,
};

static int himax_debug_info_delta_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &himax_debug_info_delta_seq_ops);
}

static struct file_operations himax_proc_debug_info_delta_ops =
{
    .owner = THIS_MODULE,
    .open = himax_debug_info_delta_open,
    .read = seq_read,
};


static ssize_t himax_incell_panel_read(struct file *file, char *buf,
                                    size_t len, loff_t *pos)
{
    ssize_t ret = 0;
    char temp_buf[8];

    if(*pos)
        return 0;

    memset(temp_buf, 0x0, sizeof(temp_buf));
    mutex_lock(&private_ts->himax_mutex);
    I("%s: enter, %d \n", __func__, __LINE__);

    ret = snprintf(temp_buf, sizeof(temp_buf), "0\n");
    if(copy_to_user(buf, temp_buf, ret)) {
        I("%s,here:%d\n",__func__,__LINE__);
        mutex_unlock(&private_ts->himax_mutex);
        return -EFAULT;
    }

    mutex_unlock(&private_ts->himax_mutex);
    *pos += ret;

    return ret;
}

static struct file_operations himax_proc_incell_panel_ops =
{
    .owner = THIS_MODULE,
    .read = himax_incell_panel_read,
};

uint32_t g_oppo_debug_level;
static ssize_t himax_oppo_debug_level_write(struct file *file, const char *buff,
                                      size_t len, loff_t *pos)
{
    char buf[80] = {0};
    int32_t dbgLevel = 0;
    int32_t ret = 0;

    if (len >= 80)
    {
        I("%s: no command exceeds 80 chars.\n", __func__);
        return -EFAULT;
    }
    if (copy_from_user(buf, buff, len))
    {
        return -EFAULT;
    }

    ret = sscanf(buf, "%d", &dbgLevel);

    if (ret >= 1) {
        g_oppo_debug_level = 0;
        switch(dbgLevel) {
            case 2:
                g_oppo_debug_level	= dbgLevel & BIT(1);
                break;
            case 1:
            case 0:
                g_oppo_debug_level = dbgLevel & BIT(0);
                break;
            default:
            g_oppo_debug_level = 0;
        };
    }
    I("%s: buf = %s, debug_level = %d\n", __func__, buf, g_oppo_debug_level);

    return len;
}


static ssize_t himax_oppo_debug_level_read(struct file *file, char *buf,
                                    size_t len, loff_t *pos)
{
    ssize_t ret = 0;
    char temp_buf[8];

    if(*pos)
        return 0;

    memset(temp_buf, 0x0, sizeof(temp_buf));
    I("%s: enter, %d \n", __func__, __LINE__);
    mutex_lock(&private_ts->himax_mutex);
    ret = snprintf(temp_buf, sizeof(temp_buf), "%d\n", g_oppo_debug_level);
    if(copy_to_user(buf, temp_buf, ret))
        I("%s,here:%d\n",__func__,__LINE__);

    mutex_unlock(&private_ts->himax_mutex);
    *pos += ret;

    return ret;
}

static struct file_operations himax_proc_oppo_debug_level_ops =
{
    .owner = THIS_MODULE,
    .write = himax_oppo_debug_level_write,
    .read = himax_oppo_debug_level_read,
};
#include <linux/irq.h>
static ssize_t himax_irq_depth_read(struct file *file, char *buf,
                                size_t len, loff_t *pos)
{
    size_t ret = 0;
    char temp_buf[30];
    struct irq_desc *desc = irq_to_desc(private_ts->client->irq);

    if(*pos)
        return 0;

    memset(temp_buf, 0x0, sizeof(temp_buf));
    I("%s: enter, %d \n", __func__, __LINE__);
    mutex_lock(&private_ts->himax_mutex);
    ret = snprintf(temp_buf, sizeof(temp_buf), "now depth=%d\n", desc->depth);
    if (copy_to_user(buf, temp_buf, ret))
        I("%s,here:%d\n", __func__, __LINE__);

    mutex_unlock(&private_ts->himax_mutex);
    *pos += ret;

    return ret;
}

static struct file_operations himax_proc_irq_depth_ops = {
    .owner = THIS_MODULE,
    .read = himax_irq_depth_read,
};

static ssize_t himax_double_tap_enable_write(struct file *file, const char *buff,
                                      size_t len, loff_t *pos)
{
    char buf[80] = {0};
    int32_t en = 0;
    int32_t ret = 0;
    int32_t i;

    if (len >= 80)
    {
        I("%s: no command exceeds 80 chars.\n", __func__);
        return -EFAULT;
    }
    if (copy_from_user(buf, buff, len))
    {
        return -EFAULT;
    }

    ret = sscanf(buf, "%d", &en);
#ifdef HX_SMART_WAKEUP
    if (ret >= 1) {
        private_ts->oppo_swmp_control = en;
        switch(en) {
        case 0:
            private_ts->SMWP_enable = 0;
            private_ts->psensor_stus= 0;
            private_ts->gesture_cust_en[0] = 0;
            break;
        case 1:
            if (private_ts->SMWP_enable == 0 && private_ts->psensor_stus == 1) {
                private_ts->SMWP_enable = 1;
                private_ts->psensor_stus=0;
                private_ts->gesture_cust_en[0] = 1;
                i2c_himax_write_command(private_ts->client, HX_CMD_TSSON, DEFAULT_RETRY_CNT);
                msleep(10);
                break;
            } else {
                private_ts->SMWP_enable = 1;
                for (i = 0; i < 16; i++)
                    private_ts->gesture_cust_en[i] = 1;
                //private_ts->gesture_cust_en[0] = 1;
                break;
            }
        case 2:
            if (private_ts->SMWP_enable == 1 && private_ts->psensor_stus == 0) {
                private_ts->SMWP_enable = 0;
                private_ts->psensor_stus= 1;
                //for (i = 0; i < 16; i++)
                //  private_ts->gesture_cust_en[i] = 1;
                private_ts->gesture_cust_en[0] = 0;
                i2c_himax_write_command(private_ts->client, HX_CMD_TSSOFF, DEFAULT_RETRY_CNT);
                msleep(10);
                break;
            }
        default:
            private_ts->SMWP_enable = 0;
            private_ts->psensor_stus= 0;
            //for (i = 0; i < 16; i++)
            //  private_ts->gesture_cust_en[i] = 0;
            private_ts->gesture_cust_en[0] = 0;
            private_ts->oppo_swmp_control = 0;
        };
        himax_set_SMWP_enable(private_ts->client, private_ts->SMWP_enable,
            private_ts->suspended);
    }
#endif
    I("%s: buf = %s, double_tap_enable = %d,private_ts->psensor_stus = %d\n", __func__, buf, private_ts->oppo_swmp_control,private_ts->psensor_stus);

    return len;
}


static ssize_t himax_double_tap_enable_read(struct file *file, char *buf,
                                    size_t len, loff_t *pos)
{
    ssize_t ret = 0;
    char temp_buf[8];

    if(*pos)
        return 0;

    memset(temp_buf, 0x0, sizeof(temp_buf));
    I("%s: enter, %d \n", __func__, __LINE__);
    mutex_lock(&private_ts->himax_mutex);
    ret = snprintf(temp_buf, sizeof(temp_buf), "%d\n", private_ts->oppo_swmp_control);
    if(copy_to_user(buf, temp_buf, ret))
        I("%s,here:%d\n",__func__,__LINE__);

    mutex_unlock(&private_ts->himax_mutex);
    *pos += ret;

    return ret;
}

static struct file_operations himax_proc_double_tap_enable_ops =
{
    .owner = THIS_MODULE,
    .write = himax_double_tap_enable_write,
    .read = himax_double_tap_enable_read,
};

static ssize_t himax_main_register_read(struct file *file, char *buf,
								  size_t len, loff_t *pos)
{
    size_t ret = 0;
    char temp_buf[144] = {0};

    if(*pos)
        return 0;

    fill_status_info(temp_buf, sizeof(temp_buf));
    I("%s: enter, %d \n", __func__, __LINE__);
    mutex_lock(&private_ts->himax_mutex);
    if (copy_to_user(buf, temp_buf, sizeof(temp_buf))) {
        I("%s,here:%d\n", __func__, __LINE__);
        mutex_unlock(&private_ts->himax_mutex);
        return -EFAULT;
    }

    ret = strlen(temp_buf);

    mutex_unlock(&private_ts->himax_mutex);

    *pos += ret;

    return ret;
}

static struct file_operations himax_proc_main_register_ops = {
    .owner = THIS_MODULE,
    .read = himax_main_register_read,
};

static int himax_debug_info_data_limit_seq_read(struct seq_file *s, void* v)
{
    size_t ret = 0;
    int32_t i, j;
    uint32_t id_matched;
    uint32_t line_cnt;
    const uint32_t line_max = 16;

    I("%s: enter, %d \n", __func__, __LINE__);
    ret = himax_self_test_data_init();
    if (ret) {
        E("himax_self_test_data_init fail!\n");
        seq_printf(s, "%s file error, returned.\n", HIMAX_TEST_FILE);

        return ret;
    }

    for (i = 0; i < g_hx_crtra_str_count; i++) {
        if (strlen(g_hx_crtra_result[i]) == 0) {
            continue;
        }

        id_matched = 0;
        for (j = 0; j < himax_get_criteria_size(); j++) {
            if (strncmp(g_hx_crtra_result[i], g_hx_inspt_crtra_name[j],
                strlen(g_hx_inspt_crtra_name[j])) == 0) {
                seq_printf(s, "\n\n");
                id_matched = 1;
                break;
            }
        }
        seq_printf(s, "%s", g_hx_crtra_result[i]);
        if (!id_matched) {
            seq_printf(s, ",");
            if ((++line_cnt) % line_max == 0)
                seq_printf(s, "\n");
        } else {
            seq_printf(s, ":\n");
            line_cnt = 0;
        }
    }
    seq_printf(s, "\n");

    return ret;
}

static struct seq_operations himax_debug_info_data_limit_seq_ops =
{
    .start  = himax_diag_seq_start,
    .next   = himax_diag_seq_next,
    .stop   = himax_diag_seq_stop,
    .show   = himax_debug_info_data_limit_seq_read,
};


static int himax_debug_info_data_limit_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &himax_debug_info_data_limit_seq_ops);
}

static struct file_operations himax_proc_debug_info_data_limit_ops =
{
    .owner = THIS_MODULE,
    .open = himax_debug_info_data_limit_open,
    .read = seq_read,
};

static ssize_t himax_tp_fw_update_write(struct file *file, const char *buff,
                                      size_t len, loff_t *pos)
{
    char buf[80] = {0};
    int32_t en = 0;
    int32_t ret = 0;
    uint32_t chk = 0;

    if (len >= 80)
    {
        I("%s: no command exceeds 80 chars.\n", __func__);
        return -EFAULT;
    }

    if (private_ts->boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT)
    {
        I("boot mode is MSM_BOOT_MODE__CHARGE,not need update tp firmware\n");
        return len;
    }

    if (copy_from_user(buf, buff, len))
    {
        return -EFAULT;
    }

    ret = sscanf(buf, "%d", &en);
    private_ts->firmware_update_type = en;
    if (ret >= 1) {
        switch(en) {
        case 0:
        case 2:
            /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/04/15 modified for fw update*/
            if (g_auto_update_flag) {
                I("force update!\n");
            } else {
                himax_update_fw_version(0);
                chk = 1;
                I("compare fw version!\n");
            }
        case 1:
            /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/26 modified for oppo fw update*/
            if ((private_ts->firmware_update_type == 0) || (private_ts->firmware_update_type ==1)) {
                ret = himax_fw_update(0, HX_FW_FILE, chk);
            } else {
                ret = himax_fw_update(0, HX_FW_FILE_SIGNED, chk);
            }
            if (ret < 0)
                goto END_FW_UPDATE_WRITE;
#ifdef HX_RST_PIN_FUNC
            himax_ic_reset(true, false);
#else
            himax_sense_on(private_ts->client, 0);
#endif
            /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/30 modified for hx8527*/
            himax_read_FW_ver(private_ts->client);
END_FW_UPDATE_WRITE:
            himax_int_enable(private_ts->client->irq, 1);

#ifdef HX_CHIP_STATUS_MONITOR
            g_chip_monitor_data->HX_CHIP_POLLING_COUNT = 0;
            g_chip_monitor_data->HX_CHIP_MONITOR_EN = 1;
            queue_delayed_work(private_ts->himax_chip_monitor_wq, &private_ts->himax_chip_monitor, g_chip_monitor_data->HX_POLLING_TIMES*HZ);
#endif
            break;
        default:
            return 0;
        };
    }

    I("%s: buf = %s, tp_fw_update = %d\n", __func__, buf, en);

    return len;
}


static ssize_t himax_tp_fw_update_read(struct file *file, char *buf,
                                    size_t len, loff_t *pos)
{
    ssize_t ret = 0;
    char temp_buf[256];
    int32_t llen = sizeof(temp_buf);

    if(*pos)
        return 0;

    memset(temp_buf, 0x00, llen);
    I("%s: enter, %d \n", __func__, __LINE__);
    mutex_lock(&private_ts->himax_mutex);
    if (fw_update_complete)	{
        ret += snprintf(temp_buf + ret, llen - ret, "FW Update Complete:\n");
    } else {
        ret += snprintf(temp_buf + ret, llen - ret, "FW Update Fail:\n");
    }
    ret += snprintf(temp_buf + ret, llen - ret, "FW_VER = 0x%2.2X \n",ic_data->vendor_fw_ver);
    if(IC_TYPE < 8)
        ret += snprintf(temp_buf + ret, llen - ret, "CONFIG_VER = 0x%2.2X \n",ic_data->vendor_config_ver);
    else {
        ret += snprintf(temp_buf + ret, llen - ret, "TOUCH_VER = 0x%2.2X \n",ic_data->vendor_touch_cfg_ver);
        ret += snprintf(temp_buf + ret, llen - ret, "DISPLAY_VER = 0x%2.2X \n",ic_data->vendor_display_cfg_ver);
    }
    if(ic_data->vendor_cid_maj_ver < 0 && ic_data->vendor_cid_min_ver < 0)
        ret += snprintf(temp_buf + ret, llen - ret, "CID_VER = NULL\n");
    else
        ret += snprintf(temp_buf + ret, llen - ret, "CID_VER = 0x%2.2X \n",(ic_data->vendor_cid_maj_ver << 8 | ic_data->vendor_cid_min_ver));

    if(ic_data->vendor_panel_ver < 0)
        ret += snprintf(temp_buf + ret, llen - ret, "PANEL_VER = NULL\n");
    else
        ret += snprintf(temp_buf + ret, llen - ret, "PANEL_VER = 0x%2.2X \n",ic_data->vendor_panel_ver);

    ret += snprintf(temp_buf + ret, llen - ret, "\n");
    ret += snprintf(temp_buf + ret, llen - ret, "Himax Touch Driver Version:\n");
    ret += snprintf(temp_buf + ret, llen - ret, "%s \n", HIMAX_DRIVER_VER);
    if(copy_to_user(buf, temp_buf, strlen(temp_buf)))
        I("%s,here:%d\n",__func__,__LINE__);

    mutex_unlock(&private_ts->himax_mutex);
    *pos += ret;

    return ret;
}

static struct file_operations himax_proc_tp_fw_update_ops =
{
    .owner = THIS_MODULE,
    .write = himax_tp_fw_update_write,
    .read = himax_tp_fw_update_read,
};

static ssize_t himax_coor_read(struct file *file, char *buf, /*coordinate*/
                                    size_t len, loff_t *pos)
{
    size_t ret = 0;
    int i = 0;
    char temp_buf[60];
    int32_t llen = sizeof(temp_buf);

    if(*pos)
        return 0;

    memset(temp_buf, 0x00, llen);
    I("%s: Enter.\n", __func__);
    for (i = 0; i < 8; i++) {
        if (i == 0) {
            ret += snprintf(temp_buf + ret, llen - ret, "%d,", hx_gesture_coor[i]);
        } else if (i == 7) {
            ret += snprintf(temp_buf + ret, llen - ret, "%d\n", hx_gesture_coor[i * 2 - 1]);
        } else {
            ret += snprintf(temp_buf + ret, llen - ret, "%d:", hx_gesture_coor[i * 2 - 1]);
            ret += snprintf(temp_buf + ret, llen - ret, "%d,", hx_gesture_coor[i * 2]);
        }
    }
    if (copy_to_user(buf, temp_buf, llen))
        I("%s,here:%d\n", __func__, __LINE__);
    *pos += ret;

    return ret;
}

static struct file_operations himax_proc_coor_ops = {
    .owner = THIS_MODULE,
    .read = himax_coor_read,
};

static ssize_t himax_GESTURE_read(struct file *file, char *buf,
                                size_t len, loff_t *pos)
{
    struct himax_ts_data *ts = private_ts;
    int i = 0;
    size_t ret = 0;
    char temp_buf[288];
    int32_t llen = sizeof(temp_buf);

    if(*pos)
        return 0;

    memset(temp_buf, 0x00, llen);

    for (i = 0; i < 16; i++)
        ret += snprintf(temp_buf + ret, llen - ret, "ges_en[%d]=%d \n", i, ts->gesture_cust_en[i]);

    if (copy_to_user(buf, temp_buf, llen))
        I("%s,here:%d\n", __func__, __LINE__);
    *pos += ret;

    return ret;
}

static ssize_t himax_GESTURE_write(struct file *file, const char *buff,
                                size_t len, loff_t *pos)
{
    struct himax_ts_data *ts = private_ts;
    int i = 0;
    char buf[80] = {0};

    if (len >= 80) {
        I("%s: no command exceeds 80 chars.\n", __func__);
        return -EFAULT;
    }

    if (copy_from_user(buf, buff, len)) {
        return -EFAULT;
    }

    I("himax_GESTURE_store= %s \n", buf);

    for (i = 0; i < 16; i++) {
        if (buf[i] == '0')
            ts->gesture_cust_en[i] = 0;
        else if (buf[i] == '1')
            ts->gesture_cust_en[i] = 1;
        else
            ts->gesture_cust_en[i] = 0;

        I("gesture en[%d]=%d \n", i, ts->gesture_cust_en[i]);
    }

    return len;
}

static struct file_operations himax_proc_Gesture_ops = {
    .owner = THIS_MODULE,
    .read = himax_GESTURE_read,
    .write = himax_GESTURE_write,
};

static ssize_t himax_game_switch_enable_write(struct file *file, const char *buff,
                                      size_t len, loff_t *pos)
{
    char buf[80] = {0};
    int32_t en = 0;
    int32_t ret = 0;
    uint8_t current_state;
    uint8_t reg_addr = HX_CMD_OPPO_SP;

    mutex_lock(&private_ts->himax_mutex);
    current_state = private_ts->oppo_sp_reg_state;
    if (len >= 80)
    {
        I("%s: no command exceeds 80 chars.\n", __func__);
        return -EFAULT;
    }
    if (copy_from_user(buf, buff, len))
    {
        return -EFAULT;
    }

    ret = sscanf(buf, "%d", &en);
    if (ret >= 1) {
        switch(en) {
        case 1:
        case 0:
            current_state = OPPO_SET_GAME_SWITCH((uint8_t)en, current_state);
            private_ts->oppo_sp_en = OPPO_SET_GAME_SWITCH((uint8_t)en, private_ts->oppo_sp_en);
            break;
        };
        if (current_state != private_ts->oppo_sp_reg_state) {
            himax_register_write(private_ts->client, &reg_addr, 1, &current_state, 0);
            private_ts->oppo_sp_reg_state = current_state;
        }
    }
    mutex_unlock(&private_ts->himax_mutex);
    I("%s: current_state = %X, game_switch_enable = %X\n", __func__,
        current_state, OPPO_GET_GAME_SWITCH(private_ts->oppo_sp_reg_state));

    return len;
}


static ssize_t himax_game_switch_enable_read(struct file *file, char *buf,
                                    size_t len, loff_t *pos)
{
    ssize_t ret = 0;
    char temp_buf[8];

    if(*pos)
        return 0;

    memset(temp_buf, 0x0, sizeof(temp_buf));
    I("%s: enter, %d \n", __func__, __LINE__);

    ret = snprintf(temp_buf, sizeof(temp_buf), "%d\n",
        OPPO_GET_GAME_SWITCH(private_ts->oppo_sp_reg_state));
    if(copy_to_user(buf, temp_buf, ret))
        I("%s,here:%d\n",__func__,__LINE__);

    *pos += ret;

    return ret;
}

static struct file_operations himax_proc_game_switch_enable_ops =
{
    .owner = THIS_MODULE,
    .write = himax_game_switch_enable_write,
    .read = himax_game_switch_enable_read,
};
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/17 modified for hx8527*/
static ssize_t himax_oppo_tp_direction_write(struct file *file, const char *buff,
                                      size_t len, loff_t *pos)
{
    char buf[80] = {0};
    int32_t en = 0;
    int32_t ret = 0;
    uint8_t current_state;
    uint8_t reg_addr = HX_CMD_OPPO_SP;

    mutex_lock(&private_ts->himax_mutex);
    current_state = private_ts->oppo_sp_en;
    if (len >= 80)
    {
        I("%s: no command exceeds 80 chars.\n", __func__);
        return -EFAULT;
    }
    if (copy_from_user(buf, buff, len))
    {
        return -EFAULT;
    }

    ret = sscanf(buf, "%d", &en);
    if (ret >= 1) {
        switch(en) {
        case 1:
            current_state = OPPO_SET_LANSCAPE_FRONT_LEFT(1, current_state);
            break;
        case 2:
            current_state = OPPO_SET_LANSCAPE_FRONT_RIGHT(1, current_state);
            break;
        case 0:
        default:
            current_state = OPPO_SET_PORTRAIT(1, current_state);
            break;
        };
        private_ts->oppo_sp_en = current_state;

        if (current_state != private_ts->oppo_sp_reg_state) {
            himax_register_write(private_ts->client, &reg_addr, 1, &current_state, 0);
            private_ts->oppo_sp_reg_state = current_state;
        }
    }
    mutex_unlock(&private_ts->himax_mutex);
    I("%s: oppo_tp_direction = %d, Portrait = %d, Lanscape 90degree = %d, Lanscape 270degree = %d\n",
        __func__, current_state, OPPO_GET_PORTRAIT(private_ts->oppo_sp_en),
        OPPO_GET_LANSCAPE_FRONT_RIGHT(private_ts->oppo_sp_en),
        OPPO_GET_LANSCAPE_FRONT_LEFT(private_ts->oppo_sp_en));

    return len;
}

/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/17 modified for hx8527*/
static ssize_t himax_oppo_tp_direction_read(struct file *file, char *buf,
                                    size_t len, loff_t *pos)
{
    ssize_t ret = 0;
    char temp_buf[8];
    int32_t tp_direction;

    if(*pos)
        return 0;
    memset(temp_buf, 0x0, sizeof(temp_buf));
    switch(private_ts->oppo_sp_en&OPPO_EN_DIRECTION_MASK) {
    case OPPO_EN_PORTRAIT:
        tp_direction = 0;
        break;
    case OPPO_EN_LANSCAPE_FRONT_LEFT:
        tp_direction = 1;
        break;
    case OPPO_EN_LANSCAPE_FRONT_RIGHT:
        tp_direction = 2;
        break;
    default:
        tp_direction = 0;
        private_ts->oppo_sp_en = OPPO_SET_PORTRAIT(1, private_ts->oppo_sp_en);
    }
    I("%s: enter, %d \n", __func__, __LINE__);

    ret = snprintf(temp_buf, sizeof(temp_buf), "%d\n", tp_direction);
    if(copy_to_user(buf, temp_buf, ret))
        I("%s,here:%d\n",__func__,__LINE__);
    *pos += ret;

    return ret;
}

static struct file_operations himax_proc_oppo_tp_direction_ops =
{
    .owner = THIS_MODULE,
    .write = himax_oppo_tp_direction_write,
    .read = himax_oppo_tp_direction_read,
};

static ssize_t himax_oppo_tp_limit_enable_write(struct file *file, const char *buff,
                                      size_t len, loff_t *pos)
{
    char buf[80] = {0};
    int32_t en = 0;
    int32_t ret = 0;
    uint8_t current_state;
    uint8_t reg_addr = HX_CMD_OPPO_SP;

    mutex_lock(&private_ts->himax_mutex);
    current_state = private_ts->oppo_sp_reg_state;
    if (len >= 80)
    {
        I("%s: no command exceeds 80 chars.\n", __func__);
        return -EFAULT;
    }
    if (copy_from_user(buf, buff, len))
    {
        return -EFAULT;
    }

    ret = sscanf(buf, "%d", &en);
    if (ret >= 1) {
        switch(en) {
            //lanscape
            case 0:
                if (OPPO_EN_LANSCAPE_FRONT_LEFT & current_state)
                    current_state = OPPO_SET_LANSCAPE_FRONT_LEFT(1,current_state);
                if (OPPO_EN_LANSCAPE_FRONT_RIGHT & current_state)
                    current_state = OPPO_SET_LANSCAPE_FRONT_RIGHT(1,current_state);
                private_ts->oppo_sp_en = current_state;
                break;
            //portrait
            case 1:
                current_state = OPPO_SET_PORTRAIT(1, current_state);
                private_ts->oppo_sp_en = current_state;
                break;
        };
        if (current_state != private_ts->oppo_sp_reg_state) {
            himax_register_write(private_ts->client, &reg_addr, 1, &current_state, 0);
            private_ts->oppo_sp_reg_state = current_state;
        }
    }
    mutex_unlock(&private_ts->himax_mutex);
    I("%s: current_state = %X, game_switch_enable = %X\n", __func__,
        current_state, OPPO_GET_GAME_SWITCH(private_ts->oppo_sp_reg_state));

    return len;
}


static ssize_t himax_oppo_tp_limit_enable_read(struct file *file, char *buf,
                                    size_t len, loff_t *pos)
{
    ssize_t ret = 0;
    char temp_buf[8];
    uint8_t current_state;

    if(*pos)
        return 0;

    memset(temp_buf, 0x0, sizeof(temp_buf));
    I("%s: enter, %d \n", __func__, __LINE__);

    current_state = private_ts->oppo_sp_reg_state & OPPO_EN_DIRECTION_MASK;
    ret = snprintf(temp_buf, sizeof(temp_buf), "%d\n",
        (current_state?1:0));
    if(copy_to_user(buf, temp_buf, ret))
        I("%s,here:%d\n",__func__,__LINE__);
    *pos += ret;
    return ret;
}

static struct file_operations himax_proc_oppo_tp_limit_enable_ops =
{
    .owner = THIS_MODULE,
    .write = himax_oppo_tp_limit_enable_write,
    .read = himax_oppo_tp_limit_enable_read,
};


int himax_touch_proc_init(void)
{
    himax_touch_proc_dir = proc_mkdir( HIMAX_PROC_TOUCH_FOLDER, NULL);
    if (himax_touch_proc_dir == NULL)
    {
        E(" %s: himax_touch_proc_dir file create failed!\n", __func__);
        return -ENOMEM;
    }

    himax_proc_debug_level_file = proc_create(HIMAX_PROC_DEBUG_LEVEL_FILE, (S_IWUSR|S_IRUGO),
                                  himax_touch_proc_dir, &himax_proc_debug_level_ops);
    if (himax_proc_debug_level_file == NULL)
    {
        E(" %s: proc debug_level file create failed!\n", __func__);
        goto fail_1;
    }

    himax_proc_vendor_file = proc_create(HIMAX_PROC_VENDOR_FILE, (S_IRUGO),
                                         himax_touch_proc_dir, &himax_proc_vendor_ops);
    if(himax_proc_vendor_file == NULL)
    {
        E(" %s: proc vendor file create failed!\n", __func__);
        goto fail_2;
    }

    himax_proc_attn_file = proc_create(HIMAX_PROC_ATTN_FILE, (S_IRUGO),
                                       himax_touch_proc_dir, &himax_proc_attn_ops);
    if(himax_proc_attn_file == NULL)
    {
        E(" %s: proc attn file create failed!\n", __func__);
        goto fail_3;
    }

    himax_proc_int_en_file = proc_create(HIMAX_PROC_INT_EN_FILE, (S_IWUSR|S_IRUGO),
                                         himax_touch_proc_dir, &himax_proc_int_en_ops);
    if(himax_proc_int_en_file == NULL)
    {
        E(" %s: proc int en file create failed!\n", __func__);
        goto fail_4;
    }

    himax_proc_layout_file = proc_create(HIMAX_PROC_LAYOUT_FILE, (S_IWUSR|S_IRUGO),
                                         himax_touch_proc_dir, &himax_proc_layout_ops);
    if(himax_proc_layout_file == NULL)
    {
        E(" %s: proc layout file create failed!\n", __func__);
        goto fail_5;
    }

#ifdef HX_TP_PROC_RESET
    himax_proc_reset_file = proc_create(HIMAX_PROC_RESET_FILE, (S_IWUSR),
                                        himax_touch_proc_dir, &himax_proc_reset_ops);
    if(himax_proc_reset_file == NULL)
    {
        E(" %s: proc reset file create failed!\n", __func__);
        goto fail_6;
    }
#endif

#ifdef HX_TP_PROC_DIAG
    himax_proc_diag_file = proc_create(HIMAX_PROC_DIAG_FILE, (S_IWUSR|S_IRUGO),
                                       himax_touch_proc_dir, &himax_proc_diag_ops);
    if(himax_proc_diag_file == NULL)
    {
        E(" %s: proc diag file create failed!\n", __func__);
        goto fail_7;
    }
    himax_proc_diag_arrange_file = proc_create(HIMAX_PROC_DIAG_ARR_FILE, (S_IWUSR|S_IRUGO),
                                   himax_touch_proc_dir, &himax_proc_diag_arrange_ops);
    if(himax_proc_diag_arrange_file == NULL)
    {
        E(" %s: proc diag file create failed!\n", __func__);
        goto fail_7_1;
    }
#endif

#ifdef HX_TP_PROC_REGISTER
    himax_proc_register_file = proc_create(HIMAX_PROC_REGISTER_FILE, (S_IWUSR|S_IRUGO),
                                           himax_touch_proc_dir, &himax_proc_register_ops);
    if(himax_proc_register_file == NULL)
    {
        E(" %s: proc register file create failed!\n", __func__);
        goto fail_8;
    }
#endif

#ifdef HX_TP_PROC_DEBUG
    himax_proc_debug_file = proc_create(HIMAX_PROC_DEBUG_FILE, (S_IWUSR|S_IRUGO),
                                        himax_touch_proc_dir, &himax_proc_debug_ops);
    if(himax_proc_debug_file == NULL)
    {
        E(" %s: proc debug file create failed!\n", __func__);
        goto fail_9;
    }

    himax_proc_fw_debug_file = proc_create(HIMAX_PROC_FW_DEBUG_FILE, (S_IWUSR|S_IRUGO),
                                           himax_touch_proc_dir, &himax_proc_fw_debug_ops);
    if(himax_proc_fw_debug_file == NULL)
    {
        E(" %s: proc fw debug file create failed!\n", __func__);
        goto fail_9_1;
    }

    himax_proc_dd_debug_file = proc_create(HIMAX_PROC_DD_DEBUG_FILE, (S_IWUSR|S_IRUGO),
                                           himax_touch_proc_dir, &himax_proc_dd_debug_ops);
    if(himax_proc_dd_debug_file == NULL)
    {
        E(" %s: proc DD debug file create failed!\n", __func__);
        goto fail_9_2;
    }
#endif

#ifdef HX_TP_PROC_FLASH_DUMP
    himax_proc_flash_dump_file = proc_create(HIMAX_PROC_FLASH_DUMP_FILE, (S_IWUSR|S_IRUGO),
                                 himax_touch_proc_dir, &himax_proc_flash_ops);
    if(himax_proc_flash_dump_file == NULL)
    {
        E(" %s: proc flash dump file create failed!\n", __func__);
        goto fail_10;
    }
#endif

#ifdef HX_TP_PROC_SELF_TEST
    himax_proc_self_test_file = proc_create(HIMAX_PROC_SELF_TEST_FILE, (S_IWUSR|S_IRUGO|S_IWUGO),
                                            himax_touch_proc_dir, &himax_proc_self_test_ops);
    if(himax_proc_self_test_file == NULL)
    {
        E(" %s: proc self_test file create failed!\n", __func__);
        goto fail_11;
    }
#endif

#ifdef HX_HIGH_SENSE
    himax_proc_HSEN_file = proc_create(HIMAX_PROC_HSEN_FILE, (S_IWUSR|S_IRUGO|S_IWUGO),
                                       himax_touch_proc_dir, &himax_proc_HSEN_ops);
    if(himax_proc_HSEN_file == NULL)
    {
        E(" %s: proc HSEN file create failed!\n", __func__);
        goto fail_13;
    }
#endif

#ifdef HX_SMART_WAKEUP
    himax_proc_SMWP_file = proc_create(HIMAX_PROC_SMWP_FILE, (S_IWUSR|S_IRUGO|S_IWUGO),
                                       himax_touch_proc_dir, &himax_proc_SMWP_ops);
    if(himax_proc_SMWP_file == NULL)
    {
        E(" %s: proc SMWP file create failed!\n", __func__);
        goto fail_14;
    }
    himax_proc_GESTURE_file = proc_create(HIMAX_PROC_GESTURE_FILE, (S_IWUSR|S_IRUGO|S_IWUGO),
                                          himax_touch_proc_dir, &himax_proc_Gesture_ops);
    if(himax_proc_GESTURE_file == NULL)
    {
        E(" %s: proc GESTURE file create failed!\n", __func__);
        goto fail_15;
    }
#endif

#ifdef HX_TP_PROC_SENSE_ON_OFF
    himax_proc_SENSE_ON_OFF_file = proc_create(HIMAX_PROC_SENSE_ON_OFF_FILE, (S_IWUSR|S_IRUGO|S_IWUGO),
                                   himax_touch_proc_dir, &himax_proc_sense_on_off_ops);
    if(himax_proc_SENSE_ON_OFF_file == NULL)
    {
        E(" %s: proc SENSE_ON_OFF file create failed!\n", __func__);
        goto fail_16;
    }
#endif
#ifdef HX_ESD_RECOVERY
    himax_proc_ESD_cnt_file = proc_create(HIMAX_PROC_ESD_CNT_FILE, (S_IWUSR|S_IRUGO|S_IWUGO),
                                          himax_touch_proc_dir, &himax_proc_esd_cnt_ops);
    if(himax_proc_ESD_cnt_file == NULL)
    {
        E(" %s: proc ESD cnt file create failed!\n", __func__);
        goto fail_17;
    }
#endif
    himax_proc_CRC_test_file = proc_create(HIMAX_PROC_CRC_TEST_FILE, (S_IWUSR|S_IRUGO|S_IWUGO),
                                           himax_touch_proc_dir, &himax_proc_CRC_test_ops);
    if(himax_proc_CRC_test_file == NULL)
    {
        E(" %s: proc CRC test file create failed!\n", __func__);
        goto fail_18;
    }
#ifdef HX_TP_PROC_GUEST_INFO
    himax_proc_guest_info_file = proc_create(HIMAX_PROC_GUEST_INFO_FILE, (S_IWUSR|S_IRUGO|S_IWUGO),
                                           himax_touch_proc_dir, &himax_proc_guest_info_ops);
    if(himax_proc_guest_info_file == NULL)
    {
        E(" %s: proc guest information file create failed!\n", __func__);
        goto fail_19;
    }
#endif

    himax_proc_baseline_test_file = proc_create(HIMAX_PROC_BASELINE_TEST_FILE, (S_IRUGO),
                                       himax_touch_proc_dir, &himax_proc_baseline_test_ops);
    if(himax_proc_baseline_test_file == NULL)
    {
        E(" %s: proc baseline test file create failed!\n", __func__);
        goto fail_20;
    }
    himax_proc_black_screen_test_file = proc_create(HIMAX_PROC_BLACK_SCREEN_TEST_FILE, (S_IRUGO),
                                       himax_touch_proc_dir, &himax_proc_baseline_test_ops);
    if(himax_proc_black_screen_test_file == NULL)
    {
        E(" %s: proc black screen test file create failed!\n", __func__);
        goto fail_21;
    }
    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/02 add for hx8527*/
    himax_proc_debug_subdir = proc_mkdir( HIMAX_PROC_DEBUG_SUBFOLDER, himax_touch_proc_dir);
    if (himax_proc_debug_subdir == NULL)
    {
        E(" %s: himax_proc_debug_subdir file create failed!\n", __func__);
        goto fail_22;
    }

    himax_proc_debug_baseline_file = proc_create(HIMAX_PROC_DEBUG_BASELINE_FILE, (S_IRUGO),
                                       himax_proc_debug_subdir, &himax_proc_debug_info_baseline_ops);
    if (himax_proc_debug_baseline_file == NULL)
    {
        E(" %s: proc debug baseline file create failed!\n", __func__);
        goto fail_23;
    }

    himax_proc_debug_delta_file = proc_create(HIMAX_PROC_DEBUG_DELTA_FILE, (S_IRUGO),
                                       himax_proc_debug_subdir, &himax_proc_debug_info_delta_ops);
    if (himax_proc_debug_delta_file == NULL)
    {
        E(" %s: proc debug delta file create failed!\n", __func__);
        goto fail_24;
    }

    himax_proc_oppo_debug_level_file = proc_create(HIMAX_PROC_OPPO_DEBUG_LEVEL_FILE, (S_IWUGO|S_IRUGO),
                                  himax_touch_proc_dir, &himax_proc_oppo_debug_level_ops);
    if (himax_proc_oppo_debug_level_file == NULL)
    {
        E(" %s: proc oppo debug level file create failed!\n", __func__);
        goto fail_25;
    }

    himax_proc_incell_panel_file = proc_create(HIMAX_PROC_INCELL_PANEL_FILE, (S_IRUGO),
                                  himax_touch_proc_dir, &himax_proc_incell_panel_ops);
    if (himax_proc_incell_panel_file == NULL)
    {
        E(" %s: proc incell panel file create failed!\n", __func__);
        goto fail_26;
    }

    himax_proc_irq_depth_file = proc_create(HIMAX_PROC_IRQ_DEPTH_FILE, (S_IRUGO),
                                       himax_touch_proc_dir, &himax_proc_irq_depth_ops);
    if (himax_proc_irq_depth_file == NULL)
    {
        E(" %s: proc irq depth file create failed!\n", __func__);
        goto fail_27;
    }

    himax_proc_double_tap_enable_file = proc_create(HIMAX_PROC_DOUBLE_TAP_ENABLE_FILE, (S_IWUGO|S_IRUGO),
                                  himax_touch_proc_dir, &himax_proc_double_tap_enable_ops);
    if (himax_proc_double_tap_enable_file == NULL)
    {
        E(" %s: proc double tap enable file create failed!\n", __func__);
        goto fail_28;
    }

    himax_proc_main_register_file = proc_create(HIMAX_PROC_MAIN_REGISTER_FILE, (S_IRUGO),
                                       himax_touch_proc_dir, &himax_proc_main_register_ops);
    if (himax_proc_main_register_file == NULL)
    {
        E(" %s: proc main register file create failed!\n", __func__);
        goto fail_29;
    }

    himax_proc_debug_data_limit_file = proc_create(HIMAX_PROC_DEBUG_DATA_LIMIT_FILE, (S_IRUGO),
                                himax_proc_debug_subdir, &himax_proc_debug_info_data_limit_ops);
    if (himax_proc_debug_data_limit_file == NULL)
    {
        E(" %s: proc debug data limit file create failed!\n", __func__);
        goto fail_30;
    }

    himax_proc_tp_fw_update_file = proc_create(HIMAX_PROC_TP_FW_UPDATE_FILE, (S_IWUGO|S_IRUGO),
                                  himax_touch_proc_dir, &himax_proc_tp_fw_update_ops);
    if (himax_proc_tp_fw_update_file == NULL)
    {
        E(" %s: proc tp_fw_update file create failed!\n", __func__);
        goto fail_31;
    }
#ifdef HX_GESTURE_TRACK
    himax_proc_coordinate_file = proc_create(HIMAX_PROC_COORDINATE_FILE, (S_IRUGO),
                                  himax_touch_proc_dir, &himax_proc_coor_ops);
    if (himax_proc_coordinate_file == NULL)
    {
        E(" %s: proc coordinate file create failed!\n", __func__);
        goto fail_32;
    }
#endif
    himax_proc_game_switch_enable_file = proc_create(HIMAX_PROC_GAME_SWITCH_ENABLE_FILE, (S_IWUGO|S_IRUGO),
                                  himax_touch_proc_dir, &himax_proc_game_switch_enable_ops);
    if (himax_proc_game_switch_enable_file == NULL)
    {
        E(" %s: proc game_switch_enable file create failed!\n", __func__);
        goto fail_33;
    }

    himax_proc_oppo_tp_direction_file = proc_create(HIMAX_PROC_OPPO_TP_DIRECTION_FILE, (S_IWUGO|S_IRUGO),
                                  himax_touch_proc_dir, &himax_proc_oppo_tp_direction_ops);
    if (himax_proc_oppo_tp_direction_file == NULL)
    {
        E(" %s: proc oppo_tp_direction file create failed!\n", __func__);
        goto fail_34;
    }

    himax_proc_oppo_tp_limit_enable_file = proc_create(HIMAX_PROC_OPPO_TP_LIMIT_ENABLE_FILE, (S_IWUGO|S_IRUGO),
                                  himax_touch_proc_dir, &himax_proc_oppo_tp_limit_enable_ops);
    if (himax_proc_oppo_tp_limit_enable_file == NULL)
    {
        E(" %s: proc oppo_tp_limit_enable file create failed!\n", __func__);
        goto fail_35;
    }

    return 0;

fail_35:
    remove_proc_entry( HIMAX_PROC_OPPO_TP_DIRECTION_FILE, himax_touch_proc_dir);
fail_34:
    remove_proc_entry( HIMAX_PROC_GAME_SWITCH_ENABLE_FILE, himax_touch_proc_dir);
fail_33:
#ifdef HX_GESTURE_TRACK
    remove_proc_entry( HIMAX_PROC_COORDINATE_FILE, himax_touch_proc_dir);
fail_32:
#endif
    remove_proc_entry( HIMAX_PROC_TP_FW_UPDATE_FILE, himax_touch_proc_dir);
fail_31:
    remove_proc_entry( HIMAX_PROC_DEBUG_DATA_LIMIT_FILE, himax_proc_debug_subdir);
fail_30:
    remove_proc_entry( HIMAX_PROC_MAIN_REGISTER_FILE, himax_touch_proc_dir);
fail_29:
    remove_proc_entry( HIMAX_PROC_DOUBLE_TAP_ENABLE_FILE, himax_touch_proc_dir);
fail_28:
    remove_proc_entry( HIMAX_PROC_IRQ_DEPTH_FILE, himax_touch_proc_dir);
fail_27:
    remove_proc_entry( HIMAX_PROC_INCELL_PANEL_FILE, himax_touch_proc_dir);
fail_26:
    remove_proc_entry( HIMAX_PROC_OPPO_DEBUG_LEVEL_FILE, himax_touch_proc_dir);
fail_25:
    remove_proc_entry( HIMAX_PROC_DEBUG_DELTA_FILE, himax_proc_debug_subdir);
fail_24:
    remove_proc_entry( HIMAX_PROC_DEBUG_BASELINE_FILE, himax_proc_debug_subdir);
fail_23:
    remove_proc_entry( HIMAX_PROC_DEBUG_SUBFOLDER, himax_touch_proc_dir);
fail_22:
    remove_proc_entry(HIMAX_PROC_BLACK_SCREEN_TEST_FILE, himax_touch_proc_dir);
fail_21:
    remove_proc_entry(HIMAX_PROC_BASELINE_TEST_FILE, himax_touch_proc_dir);
fail_20:
#ifdef HX_TP_PROC_GUEST_INFO
fail_19:
    remove_proc_entry(HIMAX_PROC_GUEST_INFO_FILE, himax_touch_proc_dir);
#endif
fail_18:
#ifdef HX_ESD_RECOVERY
    remove_proc_entry( HIMAX_PROC_ESD_CNT_FILE, himax_touch_proc_dir );
fail_17:
#endif
#ifdef HX_TP_PROC_SENSE_ON_OFF
    remove_proc_entry( HIMAX_PROC_SENSE_ON_OFF_FILE, himax_touch_proc_dir );
fail_16:
#endif
#ifdef HX_SMART_WAKEUP
    remove_proc_entry( HIMAX_PROC_GESTURE_FILE, himax_touch_proc_dir );
fail_15:
    remove_proc_entry( HIMAX_PROC_SMWP_FILE, himax_touch_proc_dir );
fail_14:
#endif
#ifdef HX_HIGH_SENSE
    remove_proc_entry( HIMAX_PROC_HSEN_FILE, himax_touch_proc_dir );
fail_13:
#endif
#ifdef HX_TP_PROC_SELF_TEST
    remove_proc_entry( HIMAX_PROC_SELF_TEST_FILE, himax_touch_proc_dir );
fail_11:
#endif
#ifdef HX_TP_PROC_FLASH_DUMP
    remove_proc_entry( HIMAX_PROC_FLASH_DUMP_FILE, himax_touch_proc_dir );
fail_10:
#endif
#ifdef HX_TP_PROC_DEBUG
    remove_proc_entry( HIMAX_PROC_DEBUG_FILE, himax_touch_proc_dir );
fail_9:
    remove_proc_entry( HIMAX_PROC_FW_DEBUG_FILE, himax_touch_proc_dir );
fail_9_1:
    remove_proc_entry( HIMAX_PROC_DD_DEBUG_FILE, himax_touch_proc_dir );
fail_9_2:
#endif
#ifdef HX_TP_PROC_REGISTER
    remove_proc_entry( HIMAX_PROC_REGISTER_FILE, himax_touch_proc_dir );
fail_8:
#endif
#ifdef HX_TP_PROC_DIAG
    remove_proc_entry( HIMAX_PROC_DIAG_FILE, himax_touch_proc_dir );
fail_7:
    remove_proc_entry( HIMAX_PROC_DIAG_ARR_FILE, himax_touch_proc_dir );
fail_7_1:
#endif
#ifdef HX_TP_PROC_RESET
    remove_proc_entry( HIMAX_PROC_RESET_FILE, himax_touch_proc_dir );
fail_6:
#endif
    remove_proc_entry( HIMAX_PROC_LAYOUT_FILE, himax_touch_proc_dir );
fail_5:
    remove_proc_entry( HIMAX_PROC_INT_EN_FILE, himax_touch_proc_dir );
fail_4:
    remove_proc_entry( HIMAX_PROC_ATTN_FILE, himax_touch_proc_dir );
fail_3:
    remove_proc_entry( HIMAX_PROC_VENDOR_FILE, himax_touch_proc_dir );
fail_2:
    remove_proc_entry( HIMAX_PROC_DEBUG_LEVEL_FILE, himax_touch_proc_dir );
fail_1:
    remove_proc_entry( HIMAX_PROC_TOUCH_FOLDER, NULL );
    return -ENOMEM;
}

void himax_touch_proc_deinit(void)
{
    deinit_criteria_file_ds();
    himax_self_test_data_deinit();
    remove_proc_entry( HIMAX_PROC_CRC_TEST_FILE, himax_touch_proc_dir );
#ifdef HX_ESD_RECOVERY
    remove_proc_entry( HIMAX_PROC_ESD_CNT_FILE, himax_touch_proc_dir );
#endif
#ifdef HX_TP_PROC_SENSE_ON_OFF
    remove_proc_entry( HIMAX_PROC_SENSE_ON_OFF_FILE, himax_touch_proc_dir );
#endif
#ifdef HX_SMART_WAKEUP
    remove_proc_entry( HIMAX_PROC_GESTURE_FILE, himax_touch_proc_dir );
    remove_proc_entry( HIMAX_PROC_SMWP_FILE, himax_touch_proc_dir );
#endif
#ifdef HX_DOT_VIEW
    remove_proc_entry( HIMAX_PROC_HSEN_FILE, himax_touch_proc_dir );
#endif
#ifdef HX_TP_PROC_SELF_TEST
    remove_proc_entry(HIMAX_PROC_SELF_TEST_FILE, himax_touch_proc_dir);
#endif
#ifdef HX_TP_PROC_FLASH_DUMP
    remove_proc_entry(HIMAX_PROC_FLASH_DUMP_FILE, himax_touch_proc_dir);
#endif
#ifdef HX_TP_PROC_DEBUG
    remove_proc_entry( HIMAX_PROC_DEBUG_FILE, himax_touch_proc_dir );
    remove_proc_entry( HIMAX_PROC_FW_DEBUG_FILE, himax_touch_proc_dir );
    remove_proc_entry( HIMAX_PROC_DD_DEBUG_FILE, himax_touch_proc_dir );
#endif
#ifdef HX_TP_PROC_REGISTER
    remove_proc_entry(HIMAX_PROC_REGISTER_FILE, himax_touch_proc_dir);
#endif
#ifdef HX_TP_PROC_DIAG
    remove_proc_entry(HIMAX_PROC_DIAG_FILE, himax_touch_proc_dir);
#endif
#ifdef HX_TP_PROC_RESET
    remove_proc_entry( HIMAX_PROC_RESET_FILE, himax_touch_proc_dir );
#endif
#ifdef HX_TP_PROC_GUEST_INFO
    remove_proc_entry(HIMAX_PROC_GUEST_INFO_FILE, himax_touch_proc_dir);
#endif
    remove_proc_entry( HIMAX_PROC_LAYOUT_FILE, himax_touch_proc_dir );
    remove_proc_entry( HIMAX_PROC_INT_EN_FILE, himax_touch_proc_dir );
    remove_proc_entry( HIMAX_PROC_ATTN_FILE, himax_touch_proc_dir );
    remove_proc_entry( HIMAX_PROC_VENDOR_FILE, himax_touch_proc_dir );
    remove_proc_entry( HIMAX_PROC_DEBUG_LEVEL_FILE, himax_touch_proc_dir );
    remove_proc_entry( HIMAX_PROC_TOUCH_FOLDER, NULL );

/* Free global memory that declare in himax_debug.h*/
#ifdef HX_TP_PROC_DIAG
	if (diag_mutual != NULL)
		kfree(diag_mutual);
	if (diag_mutual_new != NULL)
		kfree(diag_mutual_new);
	if (diag_mutual_old != NULL)
		kfree(diag_mutual_old);
	if (flash_buffer != NULL)
		kfree(flash_buffer);
#endif
#ifdef HX_TP_PROC_2T2R
	if (diag_mutual_2 != NULL)
		kfree(diag_mutual_2);
#endif
}
#endif
