/********************************************
 ** Copyright (C) 2018 OPPO Mobile Comm Corp. Ltd.
 ** ODM_HQ_EDIT
 ** File: focaltech_ex_proc.c
 ** Description: Source file for TP driver
 **          To Control TP driver
 ** Version :1.0
 ** Date : 2018/12/19
 ** Author: Yin.Zhang@ODM_HQ.BSP.TP
 ** ---------------- Revision History: --------------------------
 ** <version>    <date>          < author >              <desc>
 **  1.0           2018/12/19   Yin.Zhang@ODM_HQ   Source file for TP driver
 ********************************************/
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/delay.h>
#include <asm/uaccess.h>

#include "focaltech_test/focaltech_test.h"
#include "focaltech_core.h"

#define OPPO_FTS_TOUCHPANEL_NAME "touchpanel"
#define OPPO_FTS_DATA_LIMIT "data_limit"
#define OPPO_FTS_BASELINE_TEST "baseline_test"
#define OPPO_FTS_BLACK_SCREEN_TEST "black_screen_test"
#define OPPO_FTS_DEBUG_INFO "debug_info"
#define OPPO_FTS_BASELINE "baseline"
#define OPPO_FTS_MAIN_REGISTER "main_register"
#define OPPO_FTS_DEBUG_LEVEL "debug_level"
#define OPPO_FTS_IRQ_DEPATH "irq_depth"
#define OPPO_FTS_REGISTER_INFO "oppo_register_info"
#define OPPO_FTS_FW_UPDATE "tp_fw_update"
#define OPPO_FTS_INCELL_PANEL "incell_panel"
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2018/12/21 add for proc node*/
#define OPPO_FTS_GAME_SWITCH "game_switch_enable"
#define OPPO_FTS_TP_LIMIT_ENABLE "oppo_tp_limit_enable"
#define OPPO_FTS_TP_DIRECTION "oppo_tp_direction"
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2018/12/25 add for proc node*/
#define OPPO_FTS_COORDINATE "coordinate"
#define OPPO_FTS_GESTURE "double_tap_enable"
#define OPPO_FTS_DELTA "delta"

/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2018/12/28 modified for proc node*
 *Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/29 modified for proc node*/
#define OPPO_FTS_GAME_SWITCH_REG 0xB4
#define OPPO_FTS_LIMIT_MODE_REG 0x8C
#define OPPO_FTS_DIRECTION_REG 0x8C

#define OPPO_FTS_GAME_DISABLE 0x01
#define OPPO_FTS_GAME_ENABLE 0x00
#define OPPO_FTS_PORTRAIT_MODE 0x00
#define OPPO_FTS_LANDSCAPE_MODE 0x01
#define OPPO_FTS_LANDSCAPE_90_DEGREES 0x01
#define OPPO_FTS_LANDSCAPE_270_DEGREES 0x02

/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/03 modified for ito*/
#define OPPO_FTS_INI_FILE_NAME   "Conf_MultipleTest_winter_ft5446.ini"
static struct proc_dir_entry *oppo_fts_data_limit;
static struct proc_dir_entry *oppo_fts_baseline_test;
static struct proc_dir_entry *oppo_fts_black_screen_test;
static struct proc_dir_entry *oppo_fts_baseline;
static struct proc_dir_entry *oppo_fts_main_register;
static struct proc_dir_entry *oppo_fts_debug_level;
static struct proc_dir_entry *oppo_fts_irq_depath;
static struct proc_dir_entry *oppo_fts_register_info;
static struct proc_dir_entry *oppo_fts_fw_update;
static struct proc_dir_entry *oppo_fts_incell_tp;
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2018/12/21 add for proc node*/
static struct proc_dir_entry *oppo_fts_game_switch;
static struct proc_dir_entry *oppo_fts_tp_limit_enable;
static struct proc_dir_entry *oppo_fts_tp_direction;
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2018/12/25 add for proc node*/
static struct proc_dir_entry *oppo_fts_coordinate;
static struct proc_dir_entry *oppo_fts_gesture;
static struct proc_dir_entry *oppo_fts_differ;

/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/02/21 modified for gesture*/
static bool gesture_done = false;
bool OPPO_FTS_PROC_SEND_FLAG = false;
static int rawdata[TX_NUM_MAX][RX_NUM_MAX] = {{0}};
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2018/12/28 add for black screen ito test*/
bool oppo_fts_black_screen_test_flag = false;
bool g_psensor_near_flag = false;

enum {
    RWREG_OP_READ = 0,
    RWREG_OP_WRITE = 1,
};

static struct rwreg_operation_t {
    int type;            /*  0: read, 1: write */
    int reg;             /*  register */
    int len;             /*  read/write length */
    int val;             /*  length = 1; read: return value, write: op return */
    int res;             /*  0: success, otherwise: fail */
    char *opbuf;         /*  length >= 1, read return value, write: op return */
} rw_op;

enum enumTestNum_FT5X46{
    FT5X46_ENTER_FACTORY_MODE,
    FT5X46_RAWDATA_TEST,
    FT5X46_SCAP_CB_TEST,
    FT5X46_SCAP_RAWDATA_TEST,
    FT5X46_WEAK_SHORT_CIRCUIT_TEST,
    FT5X46_PANELDIFFER_TEST,
    FT5X46_PANELDIFFER_UNIFORMITY_TEST,
};

/*baseline test*/
static ssize_t oppo_fts_self_test_write(struct file *file, const char __user *buffer,
        size_t count, loff_t *pos)
{
    return -EPERM;
}

static int oppo_fts_self_test_show(struct seq_file *file, void* data)
{
    char fwname[128] = {0};
    char buf[] = FTS_INI_FILE_NAME;
    int i = 0;
    int test_failed_items = 0;
    struct fts_ts_data *ts_data = fts_data;
    struct input_dev *input_dev = ts_data->input_dev;

    FTS_FUNC_ENTER();

    if (ts_data->suspended) {
        FTS_INFO("In suspend, no test, return now");
        seq_printf(file, "1 error(s) In suspend, no test, return now\n");
        return 0;
    }

    memset(fwname, 0, sizeof(fwname));
    sprintf(fwname, "%s", buf);
    FTS_DEBUG("fwname:%s.", fwname);

    for( i = FT5X46_ENTER_FACTORY_MODE ; i < FT5X46_PANELDIFFER_UNIFORMITY_TEST;  i++ ){
        test_data.test_item[i].testresult = 0;
    }

    mutex_lock(&input_dev->mutex);
    disable_irq(ts_data->irq);

#if defined(FTS_ESDCHECK_EN) && (FTS_ESDCHECK_EN)
    fts_esdcheck_switch(DISABLE);
#endif

    fts_test_entry(fwname);

#if defined(FTS_ESDCHECK_EN) && (FTS_ESDCHECK_EN)
    fts_esdcheck_switch(ENABLE);
#endif

    enable_irq(ts_data->irq);
    mutex_unlock(&input_dev->mutex);

    for(i = FT5X46_ENTER_FACTORY_MODE; i < FT5X46_PANELDIFFER_UNIFORMITY_TEST; i++) {
        if(!(test_data.test_item[i].testresult)) {
            FTS_INFO("self test item[%d] : Failed\n", i);
            test_failed_items++;
        }
    }

    if(ito_test_result == true) {
        FTS_INFO("ITOtest result: Succeeded\n");
        seq_printf(file, "0 error(s) All test passed!\n");
    } else {
        FTS_INFO("ITOtest result: Failed items %d\n", test_failed_items);
        seq_printf(file, "1 error(s) Test failed!\n");
    }

	FTS_FUNC_EXIT();
    return 0;
}

static int oppo_fts_self_test_open(struct inode* inode, struct file* file)
{
    return single_open(file, oppo_fts_self_test_show, PDE_DATA(inode));
}

static const struct file_operations oppo_fts_self_test_fops = {
    .owner = THIS_MODULE,
    .open = oppo_fts_self_test_open,
    .read = seq_read,
    .write = oppo_fts_self_test_write,
};

/*black screen test*/
static ssize_t oppo_fts_black_screen_self_test_write(struct file *file, const char __user *buffer,
        size_t count, loff_t *pos)
{
    return -EPERM;
}

static int oppo_fts_black_screen_self_test_show(struct seq_file *file, void* data)
{
    char fwname[128] = {0};
    char buf[] = FTS_INI_FILE_NAME;
    int i = 0;
    int test_failed_items = 0;
    struct fts_ts_data *ts_data = fts_data;
    struct input_dev *input_dev = ts_data->input_dev;

    FTS_FUNC_ENTER();

    if (ts_data->suspended) {
        FTS_INFO("suspend mode, resume ic");
        tpd_resume(&ts_data->client->dev);
    } else {
        FTS_INFO("In resume, no test, return now");
        seq_printf(file, "1 error(s) In suspend, no test, return now\n");
        return 0;
    }
    memset(fwname, 0, sizeof(fwname));
    sprintf(fwname, "%s", buf);
    FTS_DEBUG("fwname:%s.", fwname);

    for(i = FT5X46_ENTER_FACTORY_MODE; i < FT5X46_PANELDIFFER_UNIFORMITY_TEST;  i++) {
        test_data.test_item[i].testresult = 0;
    }

    mutex_lock(&input_dev->mutex);
    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2018/12/28 add for black screen ito test*/
    oppo_fts_black_screen_test_flag = true;
    disable_irq(ts_data->irq);

#if defined(FTS_ESDCHECK_EN) && (FTS_ESDCHECK_EN)
    fts_esdcheck_switch(DISABLE);
#endif

    fts_test_entry(fwname);

#if defined(FTS_ESDCHECK_EN) && (FTS_ESDCHECK_EN)
    fts_esdcheck_switch(ENABLE);
#endif

    enable_irq(ts_data->irq);
    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2018/12/28 add for black screen ito test*/
    oppo_fts_black_screen_test_flag = false;
    mutex_unlock(&input_dev->mutex);

    for(i = FT5X46_ENTER_FACTORY_MODE; i < FT5X46_PANELDIFFER_UNIFORMITY_TEST; i++ ) {
        if(!(test_data.test_item[i].testresult)) {
            FTS_INFO("self test item[%d] : Failed\n", i);
            test_failed_items++;
        }
    }
    if(ito_test_result == true){
        FTS_INFO("ITOtest result: Succeeded\n");
        seq_printf(file, "0 error(s) All test passed!\n");
    } else {
        FTS_INFO("ITOtest result: Failed items %d\n", test_failed_items);
        seq_printf(file, "1 error(s) Test failed!\n");
    }
    tpd_suspend(&ts_data->client->dev);
	FTS_FUNC_EXIT();

    return 0;
}

static int oppo_fts_black_screen_self_test_open(struct inode* inode, struct file* file)
{
    return single_open(file, oppo_fts_black_screen_self_test_show, PDE_DATA(inode));
}

struct file_operations oppo_fts_black_screen_selftest_fops = {
    .owner = THIS_MODULE,
    .open = oppo_fts_black_screen_self_test_open,
    .read = seq_read,
    .write = oppo_fts_black_screen_self_test_write,
};

static int oppo_fts_data_limit_read_func(struct seq_file *s, void *v)
{
    int ret = 0;
    char *ini_file_data = NULL;
    int inisize = 0;

    inisize = fts_test_get_ini_size(OPPO_FTS_INI_FILE_NAME);
    FTS_INFO("ini_size = %d ", inisize);
    if (inisize <= 0) {
        FTS_ERROR("%s ERROR:Get firmware size failed",  __func__);
        return -EIO;
    }

    ini_file_data = fts_malloc(inisize + 1); // 1: end mark
    if (NULL == ini_file_data) {
        FTS_ERROR("fts_malloc failed in function:%s",  __func__);
        return -ENOMEM;
    }
    memset(ini_file_data, 0, inisize + 1);

    ret = fts_test_read_ini_data(OPPO_FTS_INI_FILE_NAME, ini_file_data);
    if (ret) {
        FTS_ERROR(" - ERROR: fts_test_read_ini_data failed" );
        goto GET_INI_DATA_ERR;
    } else {
        FTS_INFO("fts_test_read_ini_data successful");
    }

	seq_printf(s, "%s\n", ini_file_data);

    if (ini_file_data) {
        fts_free(ini_file_data);
    }
    return 0;

GET_INI_DATA_ERR:
    if (ini_file_data) {
        fts_free(ini_file_data);
    }
    return ret;
}

static int oppo_fts_data_limit_read(struct inode *inode, struct file *file)
{
    return single_open(file, oppo_fts_data_limit_read_func, PDE_DATA(inode));
}

static const struct file_operations oppo_fts_data_limit_fops = {
    .owner = THIS_MODULE,
    .open  = oppo_fts_data_limit_read,
    .read  = seq_read,
    .release = single_release,
};

static int oppo_fts_baseline_rawdata_read_show(struct seq_file *file, void* data)
{
	int iRow = 0;
	int iCol = 0;
	int ret = 0;
	unsigned char temp_buf[2] = {0};

    memset(rawdata, 0, sizeof(rawdata));
    /*--------------------Get RawData---------------------*/
	ret = oppo_fts_proc_baseline_rawdata(rawdata, temp_buf);
	if (ret != ERROR_CODE_OK) {
		FTS_ERROR("get raw data failed!");
        return -EFAULT;
	}

    /*--------------------Show RawData---------------------*/
    for (iRow = 0; iRow < temp_buf[0]; iRow++) {
        //FTS_DEBUG("\nTx%2d:", iRow + 1);
        seq_printf(file, "\nTx%2d:", iRow + 1);
        for (iCol = 0; iCol < temp_buf[1]; iCol++) {
            //FTS_DEBUG("%5d", rawdata[iRow][iCol]);
            seq_printf(file, "%5d:", rawdata[iRow][iCol]);
        }
	}
	seq_printf(file, "\n");

	FTS_ERROR("%s,here:%d\n",__func__,__LINE__);
	return 0;
}

static int oppo_fts_baseline_rawdata_read(struct inode* inode, struct file* file)
{
    return single_open(file, oppo_fts_baseline_rawdata_read_show, PDE_DATA(inode));
}

struct file_operations oppo_fts_baseline_fops = {
    .owner = THIS_MODULE,
    .open  = oppo_fts_baseline_rawdata_read,
    .read  = seq_read,
    .release = single_release,
};

/* main_register*/
static int oppo_fts_main_register_read_func(struct seq_file *s, void *v)
{
    u8 val = 0;
    struct i2c_client *client = fts_data->client;
    struct input_dev *input_dev = fts_data->input_dev;

    mutex_lock(&input_dev->mutex);
#if FTS_ESDCHECK_EN
    fts_esdcheck_proc_busy(1);
#endif

    fts_i2c_read_reg(client, FTS_REG_POWER_MODE, &val);
    seq_printf(s, "Power Mode:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_FW_VER, &val);
    seq_printf(s, "Power Mode:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_IDE_PARA_VER_ID, &val);
    seq_printf(s, "Param Ver:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_IDE_PARA_STATUS, &val);
    seq_printf(s, "Param status:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_GESTURE_EN, &val);
    seq_printf(s, "Gesture Mode:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_CHARGER_MODE_EN, &val);
    seq_printf(s, "charge stat:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_INT_CNT, &val);
    seq_printf(s, "INT count:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_FLOW_WORK_CNT, &val);
    seq_printf(s, "ESD count:0x%02x\n", val);

#if FTS_ESDCHECK_EN
    fts_esdcheck_proc_busy(0);
#endif
    mutex_unlock(&input_dev->mutex);

    return 0;
}

static int oppo_fts_main_register_open(struct inode *inode, struct file *file)
{
    return single_open(file, oppo_fts_main_register_read_func, PDE_DATA(inode));
}

static const struct file_operations oppo_fts_main_register_fops = {
    .owner = THIS_MODULE,
    .open  = oppo_fts_main_register_open,
    .read  = seq_read,
    .release = single_release,
};

/* oppo_fts_debug_level */
static ssize_t oppo_fts_debug_level_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *ppos)
{
    unsigned int tmp = 0;
    char cmd[10] = {0};
    struct fts_ts_data *ts_data = fts_data;

    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/26 modified for coverity CID 68008*/
    if (count >= 10) {
        FTS_ERROR("no command exceeds 10 chars\n");
	return -EINVAL;
    }
    memset(cmd, '\0', sizeof(cmd));
    if(copy_from_user(cmd, buf, count)) {
        FTS_ERROR("input value error\n");
        return -EINVAL;
    }

    sscanf(cmd, "%d", &tmp);

    ts_data->oppo_debug_level = tmp;

    FTS_INFO("debug_level is %d\n", ts_data->oppo_debug_level);

    if ((ts_data->oppo_debug_level != 0) && (ts_data->oppo_debug_level != 1) && (ts_data->oppo_debug_level != 2)) {
        FTS_ERROR("debug level error %d\n", ts_data->oppo_debug_level);
        ts_data->oppo_debug_level = 0;
    }

    return count;
}

static ssize_t oppo_fts_debug_level_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	size_t ret = 0;
	char *temp_buf = NULL;
	struct fts_ts_data *ts_data = fts_data;

	if ( *ppos ) {
	    printk("is already read the file\n");
        return 0;
	}
    *ppos += count;
	FTS_INFO("%s: enter, %d \n", __func__, __LINE__);

	temp_buf = (char *)kzalloc(count, GFP_KERNEL);
    if (!temp_buf) {
        FTS_ERROR("allocate memory failed!\n");
        return -ENOMEM;
	}
	ret += snprintf(temp_buf + ret, count - ret, "%d\n", ts_data->oppo_debug_level);

	if (copy_to_user(buf, temp_buf, count))
		FTS_ERROR("%s,here:%d\n", __func__, __LINE__);

	kfree(temp_buf);

	return ret;
}

static const struct file_operations oppo_fts_debug_level_fops =
{
	.owner = THIS_MODULE,
	.write = oppo_fts_debug_level_write,
	.read = oppo_fts_debug_level_read,
};

/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/17 modified for ft5446*/
/* tp_fw_update */
static ssize_t oppo_fts_fw_update_write(struct file *file, const char __user *page, size_t size, loff_t *lo)
{
    struct fts_ts_data *ts_data = fts_data;
    int val = 0;
    int ret = 0;
    char buf[4] = {0};
    if (!ts_data)
        return size;
    if (size > 2)
        return size;

    FTS_FUNC_ENTER();
    if (ts_data->boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT)
    {
        FTS_INFO("boot mode is MSM_BOOT_MODE__CHARGE,not need update tp firmware\n");
        return size;
    }

    if (copy_from_user(buf, page, size)) {
        FTS_INFO("%s: read proc input error.\n", __func__);
        return size;
    }

    sscanf(buf, "%d", &val);
    ts_data->firmware_update_type = val;
    FTS_INFO("%s, write value: %d .\n", __func__, ts_data->firmware_update_type);
    //schedule_work(&ts_data->fw_update_work);
    queue_work(ts_data->ts_workqueue, &ts_data->fwupg_work);

    ret = wait_for_completion_killable_timeout(&ts_data->fw_complete, FW_UPDATE_COMPLETE_TIMEOUT);
    if (ret < 0) {
        FTS_INFO("kill signal interrupt\n");
    }

    FTS_INFO("fw update finished\n");
    FTS_FUNC_EXIT();
    return size;

}

static const struct file_operations oppo_fts_fw_update_fops =
{
	.owner = THIS_MODULE,
	.write = oppo_fts_fw_update_write,
};

/* fts_tp_irq_depth */
static ssize_t oppo_fts_irq_depth_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *ppos)
{
	unsigned int tmp = 0;
	char cmd[10] = {0};
	struct fts_ts_data *ts_data = fts_data;

	if (count >= 10) {
		FTS_ERROR("no command exceeds 10 chars\n");
		return -EINVAL;
	}
	memset(cmd, '\0', sizeof(cmd));
	if(copy_from_user(cmd, buf, count)) {
		FTS_ERROR("input value error\n");
		return -EINVAL;
	}

	sscanf(cmd, "%d", &tmp);

	if (tmp) {
		enable_irq(ts_data->irq);
		FTS_INFO("force enable irq\n");
	} else {
		disable_irq(ts_data->irq);
 		FTS_INFO("force disable irq\n");
	}

	return count;
}

static ssize_t oppo_fts_irq_depth_read(struct file *file, char *buf,
								  size_t len, loff_t *pos)
{
	size_t ret = 0;
	char *temp_buf = NULL;
	struct irq_desc *desc = irq_to_desc(fts_data->irq);
	FTS_INFO("%s: enter, %d \n", __func__, __LINE__);

	if (!OPPO_FTS_PROC_SEND_FLAG) {
		temp_buf = kzalloc(len, GFP_KERNEL);
		if (!temp_buf) {
            FTS_ERROR("allocate memory failed!\n");
            return -ENOMEM;
        }

		ret += snprintf(temp_buf + ret, len - ret, "now depth=%d\n", desc->depth);
		if (copy_to_user(buf, temp_buf, len)) {
			FTS_ERROR("%s,here:%d\n", __func__, __LINE__);
		}

		kfree(temp_buf);
		OPPO_FTS_PROC_SEND_FLAG = 1;
	} else {
		OPPO_FTS_PROC_SEND_FLAG = 0;
	}

	return ret;
}

static const struct file_operations oppo_fts_irq_depath_fops = {
	.owner = THIS_MODULE,
	.write = oppo_fts_irq_depth_write,
	.read = oppo_fts_irq_depth_read,
};

/*fts_tp_register_rw*/
static int oppo_fts_register_info_show(struct seq_file *file, void* data)
{
    int i = 0;
    struct input_dev *input_dev = fts_data->input_dev;

    mutex_lock(&input_dev->mutex);

    if (rw_op.len < 0) {
        seq_printf(file, "Invalid cmd line\n");
    } else if (rw_op.len == 1) {
        if (rw_op.res == 0) {
            seq_printf(file, "Read %02X: %02X\n", rw_op.reg, rw_op.val);
        } else {
            seq_printf(file, "Read %02X failed, ret: %d\n", rw_op.reg,  rw_op.res);
        }
    } else {
        seq_printf(file, "Read Reg: [%02X]-[%02X]\n", rw_op.reg, rw_op.reg + rw_op.len - 1);
        seq_printf(file, "Result: ");
        if (rw_op.res) {
            seq_printf(file, "failed, ret: %d\n", rw_op.res);
        } else {
            if (rw_op.opbuf) {
                for (i = 0; i < rw_op.len; i++) {
                    seq_printf(file, "%02X ", rw_op.opbuf[i]);
                }
                seq_printf(file, "\n");
            }
        }
    }

    mutex_unlock(&input_dev->mutex);

    return 0;
}

static int shex_to_int(const char *hex_buf, int size)
{
    int i;
    int base = 1;
    int value = 0;
    char single;

    for (i = size - 1; i >= 0; i--) {
        single = hex_buf[i];

        if ((single >= '0') && (single <= '9')) {
            value += (single - '0') * base;
        } else if ((single >= 'a') && (single <= 'z')) {
            value += (single - 'a' + 10) * base;
        } else if ((single >= 'A') && (single <= 'Z')) {
            value += (single - 'A' + 10) * base;
        } else {
            return -EINVAL;
        }

        base *= 16;
    }

    return value;
}

static u8 shex_to_u8(const char *hex_buf, int size)
{
    return (u8)shex_to_int(hex_buf, size);
}

static int fts_parse_buf(const char *buf, size_t cmd_len)
{
    int length;
    char *temp_buf;

    rw_op.reg = shex_to_u8(buf + 1, 2);
    length = shex_to_int(buf + 3, 2);

    if (buf[0] == '1') {
        rw_op.len = length;
        rw_op.type = RWREG_OP_READ;
        FTS_DEBUG("read %02X, %d bytes", rw_op.reg, rw_op.len);
    } else {
        pr_err("data invalided!\n");
        return -EINVAL;
    }
    if (rw_op.len > 0) {
        temp_buf = (char *)kzalloc(rw_op.len, GFP_KERNEL);
        if (!temp_buf) {
            FTS_ERROR("allocate memory failed!\n");
            return -ENOMEM;
        }

        rw_op.opbuf = temp_buf;
    }
    return rw_op.len;
}

//write info: read 00 reg, echo 00 > oppo_register_info. read 00~0A reg, echo 1000A > oppo_register_info
static ssize_t oppo_fts_register_info_write(struct file *filp, const char *buff, size_t size, loff_t *pPos)
{
    struct input_dev *input_dev = fts_data->input_dev;
    struct i2c_client *client = fts_data->client;
    ssize_t cmd_length = 0;
	char cmd[512] = { 0 };

    mutex_lock(&input_dev->mutex);
    cmd_length = size - 1;

    if (buff != NULL) {
        if (copy_from_user(cmd, buff, cmd_length)) {
            FTS_ERROR("copy data from user space, failed\n");
            return -EINVAL;;
        }
    }

    if (rw_op.opbuf) {
        kfree(rw_op.opbuf);
        rw_op.opbuf = NULL;
    }

    FTS_DEBUG("cmd len: %d, buf: %s\n", (int)cmd_length, cmd);
    /* compatible old ops */
    if (2 == cmd_length) {
        rw_op.type = RWREG_OP_READ;
        rw_op.len = 1;

        rw_op.reg = shex_to_int(cmd, 2);
    } else if (cmd_length < 5) {
        FTS_ERROR("Invalid cmd buffer");
        mutex_unlock(&input_dev->mutex);
        return -EINVAL;
    } else {
        rw_op.len = fts_parse_buf(cmd, cmd_length);
    }

#if FTS_ESDCHECK_EN
    fts_esdcheck_proc_busy(1);
#endif
    if (rw_op.len < 0) {
        FTS_ERROR("cmd buffer error!");

    } else {
        if (RWREG_OP_READ == rw_op.type) {
            if (rw_op.len == 1) {
                u8 reg, val;
                reg = rw_op.reg & 0xFF;
                rw_op.res = fts_i2c_read_reg(client, reg, &val);
                rw_op.val = val;
            } else {
                char reg;
                reg = rw_op.reg & 0xFF;

                rw_op.res = fts_i2c_read(client, &reg, 1, rw_op.opbuf, rw_op.len);
            }

            if (rw_op.res < 0) {
                FTS_ERROR("Could not read 0x%02x", rw_op.reg);
            } else {
                FTS_INFO("read 0x%02x, %d bytes successful", rw_op.reg, rw_op.len);
                rw_op.res = 0;
            }
        } else {
            FTS_ERROR("Could not read 0x%02x", rw_op.reg);
        }
    }

#if FTS_ESDCHECK_EN
    fts_esdcheck_proc_busy(0);
#endif
    mutex_unlock(&input_dev->mutex);

    return size;
}

static int oppo_fts_register_info_open(struct inode* inode, struct file* file)
{
    return single_open(file, oppo_fts_register_info_show, PDE_DATA(inode));
}

static const struct file_operations oppo_fts_register_info_fops = {
    .owner = THIS_MODULE,
    .open = oppo_fts_register_info_open,
    .read = seq_read,
    .write = oppo_fts_register_info_write,
};

/* fts_tp_proc_incell */
static ssize_t oppo_fts_tp_incell_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
    size_t ret = 0;
    char *temp_buf = NULL;

    if ( *ppos ) {
        printk("is already read the file\n");
        return 0;
    }
    *ppos += count;
    FTS_INFO("%s: enter, %d \n", __func__, __LINE__);

    temp_buf = kzalloc(count, GFP_KERNEL);
    if (!temp_buf) {
        FTS_ERROR("allocate memory failed!\n");
        return -ENOMEM;
    }
    ret += snprintf(temp_buf + ret, count - ret, "%d\n", fts_data->ic_info.is_incell);

    if (copy_to_user(buf, temp_buf, count))
        FTS_ERROR("%s,here:%d\n", __func__, __LINE__);

    kfree(temp_buf);

    return ret;
}

static const struct file_operations oppo_fts_incell_fops = {
    .owner = THIS_MODULE,
    .read = oppo_fts_tp_incell_read,
};

/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2018/12/21 add for proc node*
 *Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/01/29 modified for tp limit_enable*/
/* oppo_fts_tp_limit_enable */
static ssize_t oppo_fts_tp_limit_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *ppos)
{
	char *ptr = NULL;
    struct fts_ts_data *ts_data = fts_data;
    //struct i2c_client *client = ts_data->client;

    FTS_FUNC_ENTER();
	ptr = kzalloc(count, GFP_KERNEL);
	if (ptr == NULL) {
		FTS_ERROR("allocate the memory fail\n");
		return -ENOMEM;
	}

	if(copy_from_user(ptr, buf, count)) {
		FTS_ERROR("input value error\n");
		count = -1;
		goto OUT;
	}

	if (ptr[0] == '0') {
		FTS_INFO("limit mode: landscape\n");
		ts_data->limit_control = OPPO_FTS_LANDSCAPE_MODE;
		//fts_i2c_write_reg(client, OPPO_FTS_LIMIT_MODE_REG, OPPO_FTS_LANDSCAPE_MODE);
	} else if (ptr[0] == '1') {
		FTS_INFO("limit mode: portrait\n");
		ts_data->limit_control = OPPO_FTS_PORTRAIT_MODE;
		//fts_i2c_write_reg(client, OPPO_FTS_LIMIT_MODE_REG, OPPO_FTS_PORTRAIT_MODE);
	} else {
		FTS_ERROR("Unknown command\n");
	}
OUT:
	kfree(ptr);
    FTS_FUNC_EXIT();

	return count;
}

static ssize_t oppo_fts_tp_limit_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int len = 0;
	char *ptr = NULL;
    struct fts_ts_data *ts_data = fts_data;

    FTS_FUNC_ENTER();
	if ( *ppos ) {
	    FTS_ERROR("is already read the file\n");
        return 0;
	}
    *ppos += count;

    ptr = kzalloc(count,GFP_KERNEL);
	if(ptr == NULL){
		FTS_ERROR("allocate memory fail\n");
		return -ENOMEM;
	}

    len = snprintf(ptr, count,"%x\n", ts_data->limit_control);

    if (copy_to_user(buf, ptr, len)) {
        FTS_ERROR("%s,here:%d\n", __func__, __LINE__);
    }

	kfree(ptr);
    FTS_FUNC_EXIT();

	return len;
}

static const struct file_operations oppo_fts_tp_limit_fops =
{
	.owner = THIS_MODULE,
	.write = oppo_fts_tp_limit_write,
	.read = oppo_fts_tp_limit_read,
};

/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2018/12/21 add for proc node*/
/* game_switch_enable */
static ssize_t oppo_fts_game_switch_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *ppos)
{
    int ret = 0;
	char *ptr = NULL;
    struct fts_ts_data *ts_data = fts_data;
    struct i2c_client *client = ts_data->client;

    FTS_FUNC_ENTER();
	ptr = kzalloc(count, GFP_KERNEL);
	if (ptr == NULL) {
		FTS_ERROR("allocate the memory fail\n");
		return -ENOMEM;
	}

	if(copy_from_user(ptr, buf, count)) {
		FTS_ERROR("input value error\n");
		count = -1;
		goto OUT;
	}

	if (ptr[0] == '0') {
		FTS_INFO("disable game play mode\n");
		ts_data->game_switch = false;
		ret = fts_i2c_write_reg(client, OPPO_FTS_GAME_SWITCH_REG, OPPO_FTS_GAME_DISABLE);
		if (ret < 0) {
		    FTS_ERROR("game switch write reg failed!\n");
		}
	} else if (ptr[0] == '1') {
		FTS_INFO("enable game play mode\n");
		ts_data->game_switch = true;
		ret = fts_i2c_write_reg(client, OPPO_FTS_GAME_SWITCH_REG, OPPO_FTS_GAME_ENABLE);
		if (ret < 0) {
		    FTS_ERROR("game switch write reg failed!\n");
		}
	} else {
		FTS_ERROR("Unknown command\n");
	}
OUT:
	kfree(ptr);
    FTS_FUNC_EXIT();

	return count;
}

static ssize_t oppo_fts_game_switch_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int len = 0;
	char *ptr = NULL;
    struct fts_ts_data *ts_data = fts_data;

    FTS_FUNC_ENTER();
	if ( *ppos ) {
	    FTS_ERROR("is already read the file\n");
        return 0;
	}
    *ppos += count;

    ptr = kzalloc(count, GFP_KERNEL);
	if(ptr == NULL){
		FTS_ERROR("allocate memory fail\n");
		return -ENOMEM;
	}

    len = snprintf(ptr, count, "%x\n", ts_data->game_switch);

    if (copy_to_user(buf, ptr, len)) {
        FTS_ERROR("%s,here:%d\n", __func__, __LINE__);
    }

	kfree(ptr);
    FTS_FUNC_EXIT();

	return len;
}

static const struct file_operations oppo_fts_game_switch_fops =
{
	.owner = THIS_MODULE,
	.write = oppo_fts_game_switch_write,
	.read  = oppo_fts_game_switch_read,
};

/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2018/12/21 add for proc node*/
static ssize_t oppo_fts_direction_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int len = 0;
	char *ptr = NULL;
    struct fts_ts_data *ts_data = fts_data;

    FTS_FUNC_ENTER();
	if ( *ppos ) {
	    FTS_ERROR("is already read the file\n");
        return 0;
	}
    *ppos += count;

    ptr = kzalloc(count, GFP_KERNEL);
	if(ptr == NULL){
		FTS_ERROR("allocate memory fail\n");
		return -ENOMEM;
	}

    len = snprintf(ptr, count, "%x\n", ts_data->limit_direction);

    if (copy_to_user(buf, ptr, len)) {
        FTS_ERROR("%s,here:%d\n", __func__, __LINE__);
    }

	kfree(ptr);
    FTS_FUNC_EXIT();
	return len;
}

static ssize_t oppo_fts_direction_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char *ptr = NULL;
    struct fts_ts_data *ts_data = fts_data;
    struct i2c_client *client = ts_data->client;

    FTS_FUNC_ENTER();
	ptr = kzalloc(count, GFP_KERNEL);
	if (ptr == NULL) {
		FTS_ERROR("allocate the memory fail\n");
		return -ENOMEM;
	}

	if(copy_from_user(ptr, buf, count)) {
		FTS_ERROR("input value error\n");
		count = -1;
		goto OUT;
	}

	if (ptr[0] == '0') {
		FTS_INFO("limit direction: portrait\n");
		ts_data->limit_direction = OPPO_FTS_PORTRAIT_MODE;
		fts_i2c_write_reg(client, OPPO_FTS_DIRECTION_REG, OPPO_FTS_PORTRAIT_MODE);
	} else if (ptr[0] == '1') {
		FTS_INFO("limit direction: landscape 90 degrees\n");
		ts_data->limit_direction = OPPO_FTS_LANDSCAPE_90_DEGREES;
		fts_i2c_write_reg(client, OPPO_FTS_DIRECTION_REG, OPPO_FTS_LANDSCAPE_90_DEGREES);
    } else if (ptr[0] == '2') {
		FTS_INFO("limit direction: landscape 270 degrees\n");
		ts_data->limit_direction = OPPO_FTS_LANDSCAPE_270_DEGREES;
		fts_i2c_write_reg(client, OPPO_FTS_DIRECTION_REG, OPPO_FTS_LANDSCAPE_270_DEGREES);
	} else {
		FTS_ERROR("Unknown command\n");
	}
OUT:
	kfree(ptr);
    FTS_FUNC_EXIT();

	return count;
}

static const struct file_operations oppo_fts_direction_fops = {
	.owner = THIS_MODULE,
	.read = oppo_fts_direction_read,
	.write = oppo_fts_direction_write,
};

/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2018/12/25 add for proc node*/
/*oppo_proc_coordinate_read*/
static int oppo_fts_coordinate_show(struct seq_file *s, void *v)
{
    struct fts_ts_data *ts_data = fts_data;
    struct input_dev *input_dev = fts_data->input_dev;

    FTS_FUNC_ENTER();
    mutex_lock(&input_dev->mutex);

    FTS_INFO("%s:gesture_type = %d\n", __func__, ts_data->gesture.gesture_type);
    seq_printf(s, "%d,%d:%d,%d:%d,%d:%d,%d:%d,%d:%d,%d:%d,%d\n", ts_data->gesture.gesture_type,
            ts_data->gesture.Point_start.x, ts_data->gesture.Point_start.y, ts_data->gesture.Point_end.x, ts_data->gesture.Point_end.y,
            ts_data->gesture.Point_1st.x,   ts_data->gesture.Point_1st.y,   ts_data->gesture.Point_2nd.x, ts_data->gesture.Point_2nd.y,
            ts_data->gesture.Point_3rd.x,   ts_data->gesture.Point_3rd.y,   ts_data->gesture.Point_4th.x, ts_data->gesture.Point_4th.y,
            ts_data->gesture.clockwise);

    mutex_unlock(&input_dev->mutex);

    FTS_FUNC_EXIT();
    return 0;
}

static int oppo_fts_coordinate_open(struct inode *inode, struct file *file)
{
    return single_open(file, oppo_fts_coordinate_show, PDE_DATA(inode));
}

static const struct file_operations oppo_fts_coordinate_fops = {
    .owner = THIS_MODULE,
    .open  = oppo_fts_coordinate_open,
    .read  = seq_read,
    .release = single_release,
};
/*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2019/02/21 modified for gesture*/
/*oppo_proc_gesture*/
static ssize_t oppo_fts_gesture_write(struct file *filp, const char *buff, size_t size, loff_t *pPos)
{
    char *temp_buf = NULL;
    struct fts_ts_data *ts_data = fts_data;
    struct input_dev *input_dev = fts_data->input_dev;

    FTS_FUNC_ENTER();
    temp_buf = kzalloc(size,GFP_KERNEL);
    if (!temp_buf) {
        FTS_ERROR("allocate the memory fail\n");
        return -ENOMEM;
    }

    if (buff != NULL) {
        if (copy_from_user(temp_buf, buff, size)) {
            FTS_ERROR("copy data from user space, failed\n");
            kfree(temp_buf);
            return -EINVAL;;
        }
    }

    mutex_lock(&input_dev->mutex);
    FTS_DEBUG("[GESTURE] fts_data->suspended = %d, fts_gesture_data.mode = %d, gesture_done = %d\n", fts_data->suspended, fts_gesture_data.mode, gesture_done);
    if (temp_buf[0] == '1') {
        if ((ts_data->suspended == true) && (gesture_done == true)) {
            FTS_INFO("[GESTURE] p-sensor far away!");
            ts_data->gesture_enable = true;
             g_psensor_near_flag = false;
        } else {
            FTS_INFO("[GESTURE]enable gesture");
            fts_gesture_data.mode = ENABLE;
            ts_data->gesture_enable = true;
            gesture_done = true;
        }
    } else if (temp_buf[0] == '0') {
        FTS_INFO("[GESTURE]disabale gesture");
        fts_gesture_data.mode = DISABLE;
        ts_data->gesture_enable = false;
        gesture_done = false;
    } else if (temp_buf[0] == '2') {
        if ((ts_data->suspended == true) && (gesture_done == true)) {
            FTS_INFO("[GESTURE] p-sensor closed!");
            ts_data->gesture_enable = false;
            g_psensor_near_flag = true;
        }
    } else {
        FTS_ERROR("[GESTURE] Unknown command\n");
    }

    mutex_unlock(&input_dev->mutex);
    kfree(temp_buf);
    FTS_FUNC_EXIT();

    return size;
}

static ssize_t oppo_fts_gesture_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    int len = 0;
    u8 val = 0;
    char *ptr = NULL;
    struct fts_ts_data *ts_data = fts_data;
    struct i2c_client *client = fts_data->client;

    FTS_FUNC_ENTER();
    if ( *ppos ) {
        FTS_ERROR("is already read the file\n");
        return 0;
    }
    *ppos += count;

    ptr = kzalloc(count, GFP_KERNEL);
    if(ptr == NULL){
        FTS_ERROR("allocate memory fail\n");
        return -ENOMEM;
    }

    fts_i2c_read_reg(client, FTS_REG_GESTURE_EN, &val);
    len = snprintf(ptr, count, "Gesture Mode: %s\nReg(0xD0) = 0x%02x\ngesture_enble: %d\n", fts_gesture_data.mode ? "On" : "Off", val, ts_data->gesture_enable);

    if (copy_to_user(buf, ptr, count)) {
        FTS_ERROR("[APK]: copy to user error!!");
    }

    kfree(ptr);

    FTS_FUNC_EXIT();
    return len;
}

static const struct file_operations oppo_fts_gesture_fops =
{
    .owner = THIS_MODULE,
    .write = oppo_fts_gesture_write,
    .read = oppo_fts_gesture_read,
};

static int tp_differ_proc_open (struct inode* inode, struct file* file)
{
    FTS_FUNC_ENTER();
    return single_open(file, tp_differ_proc_show, NULL);
}

static const struct file_operations oppo_fts_differ_fops =
{
    .owner = THIS_MODULE,
    .open = tp_differ_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

int32_t oppo_fts_extra_proc_init(void)
{
    struct proc_dir_entry *oppo_fts_touchpanel_proc = NULL;
    struct proc_dir_entry *oppo_fts_debug_info = NULL;

    oppo_fts_touchpanel_proc = proc_mkdir(OPPO_FTS_TOUCHPANEL_NAME, NULL);
    if(oppo_fts_touchpanel_proc == NULL) {
        FTS_ERROR("create oppo_fts_touchpanel_proc fail\n");
        return -ENOMEM;
    }

    oppo_fts_debug_info = proc_mkdir(OPPO_FTS_DEBUG_INFO, oppo_fts_touchpanel_proc);
    if(oppo_fts_debug_info == NULL) {
        FTS_ERROR("create oppo_debug_info fail\n");
        return -ENOMEM;
    }

    //proc file: /proc/touchpanel/baseline_test
    oppo_fts_baseline_test = proc_create(OPPO_FTS_BASELINE_TEST, 0666, oppo_fts_touchpanel_proc, &oppo_fts_self_test_fops);
    if (oppo_fts_baseline_test == NULL) {
        FTS_ERROR("create proc/touchpanel/baseline_test Failed!\n");
        return -ENOMEM;
    }
    //proc file: /proc/touchpanel/black_screen_test
    oppo_fts_black_screen_test = proc_create(OPPO_FTS_BLACK_SCREEN_TEST, 0666, oppo_fts_touchpanel_proc, &oppo_fts_black_screen_selftest_fops);
    if (oppo_fts_black_screen_test == NULL) {
        FTS_ERROR("create proc/touchpanel/black_screen_test Failed!\n");
        return -ENOMEM;
    }

    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2018/12/25 modified for baseline*/
    //proc file: /proc/touchpanel/debug_info/baseline
    oppo_fts_baseline = proc_create(OPPO_FTS_BASELINE, 0666, oppo_fts_debug_info, &oppo_fts_baseline_fops);
    if (oppo_fts_baseline == NULL) {
        FTS_ERROR("create proc/touchpanel/debug_info/baseline Failed!\n");
        return -ENOMEM;
    }

    //proc file: /proc/touchpanel/debug_info/main_register
    oppo_fts_main_register = proc_create(OPPO_FTS_MAIN_REGISTER, 0666, oppo_fts_debug_info, &oppo_fts_main_register_fops);
    if (oppo_fts_main_register == NULL) {
        FTS_ERROR("create proc/touchpanel/debug_info/main_register Failed!\n");
        return -ENOMEM;
    }

    //proc file: /proc/touchpanel/debug_info/data_limit
    oppo_fts_data_limit = proc_create(OPPO_FTS_DATA_LIMIT, 0666, oppo_fts_debug_info, &oppo_fts_data_limit_fops);
    if (oppo_fts_data_limit == NULL) {
        FTS_ERROR("create proc/touchpanel/debug_info/data_limit Failed!\n");
        return -ENOMEM;
    }

    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2018/12/25 add for delta*/
    //proc file: /proc/touchpanel/debug_info/delta
    oppo_fts_differ = proc_create(OPPO_FTS_DELTA, 0666, oppo_fts_debug_info, &oppo_fts_differ_fops);
    if (oppo_fts_differ == NULL) {
        FTS_TEST_ERROR( "[focal] %s() - ERROR: create fts_tp_differ proc()  failed.",  __func__);
        return -ENOMEM;
    }

    //proc file: /proc/touchpanel/debug_level
    oppo_fts_debug_level= proc_create(OPPO_FTS_DEBUG_LEVEL, 0644, oppo_fts_touchpanel_proc, &oppo_fts_debug_level_fops);
    if (oppo_fts_debug_level == NULL) {
        FTS_ERROR("create proc/touchpanel/debug_level Failed!\n");
        return -ENOMEM;
    }

    //proc file: /proc/touchpanel/irq_depth
    oppo_fts_irq_depath = proc_create(OPPO_FTS_IRQ_DEPATH, 0666, oppo_fts_touchpanel_proc, &oppo_fts_irq_depath_fops);
    if (oppo_fts_irq_depath == NULL) {
        FTS_ERROR("create proc/touchpanel/irq_depath Failed!\n");
        return -ENOMEM;
    }

    //proc file: /proc/touchpanel/oppo_register_info
    oppo_fts_register_info = proc_create(OPPO_FTS_REGISTER_INFO, 0664, oppo_fts_touchpanel_proc, &oppo_fts_register_info_fops);
    if (oppo_fts_register_info == NULL) {
        FTS_ERROR("create proc/touchpanel/oppo_register_info Failed!\n");
        return -ENOMEM;
    }

    //proc file: /proc/touchpanel/tp_fw_update
    oppo_fts_fw_update = proc_create(OPPO_FTS_FW_UPDATE,0644, oppo_fts_touchpanel_proc, &oppo_fts_fw_update_fops);
    if (oppo_fts_fw_update == NULL) {
        FTS_ERROR("create proc/touchpanel/tp_fw_update Failed!\n");
        return -ENOMEM;
    }

    //proc file: /proc/touchpanel/incell_panel
    oppo_fts_incell_tp = proc_create(OPPO_FTS_INCELL_PANEL, 0664, oppo_fts_touchpanel_proc, &oppo_fts_incell_fops);
    if (oppo_fts_incell_tp == NULL) {
        FTS_ERROR("create proc/fts_incell Failed!\n");
        return -ENOMEM;
    }

    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2018/12/21 add for proc node*/
    //proc file: /proc/touchpanel/game_switch_enable
    oppo_fts_game_switch = proc_create(OPPO_FTS_GAME_SWITCH,0666, oppo_fts_touchpanel_proc, &oppo_fts_game_switch_fops);
    if (oppo_fts_game_switch == NULL) {
        FTS_ERROR("create proc/touchpanel/oppo_game_switch_enable Failed!\n");
        return -ENOMEM;
    }

    //proc file: /proc/touchpanel/oppo_tp_direction
    oppo_fts_tp_limit_enable = proc_create(OPPO_FTS_TP_LIMIT_ENABLE,0666, oppo_fts_touchpanel_proc, &oppo_fts_tp_limit_fops);
    if (oppo_fts_tp_limit_enable == NULL) {
        FTS_ERROR("create proc/touchpanel/oppo_tp_limit_enable Failed!\n");
        return -ENOMEM;
    }

    //proc file: /proc/touchpanel/oppo_tp_limit_enable
    oppo_fts_tp_direction = proc_create(OPPO_FTS_TP_DIRECTION, 0664, oppo_fts_touchpanel_proc,&oppo_fts_direction_fops);
    if (oppo_fts_tp_direction == NULL) {
        FTS_ERROR("create proc/nvt_direction Failed!\n");
        return -ENOMEM;
    }

    /*Yin.Zhang@ODM_HQ.BSP.TP.Function, 2018/12/25 add for proc node*/
    //proc file: /proc/touchpanel/coordinate
    oppo_fts_coordinate = proc_create(OPPO_FTS_COORDINATE, 0666, oppo_fts_touchpanel_proc, &oppo_fts_coordinate_fops);
    if (oppo_fts_coordinate == NULL) {
        FTS_ERROR("create proc/touchpanel/coordinate Failed!\n");
        return -ENOMEM;
    }

    //proc file: /proc/touchpanel/double_tap_enable
    oppo_fts_gesture = proc_create(OPPO_FTS_GESTURE, 0666, oppo_fts_touchpanel_proc, &oppo_fts_gesture_fops);
    if (oppo_fts_gesture == NULL) {
        FTS_ERROR("create proc/touchpanel/double_tap_enable Failed!\n");
        return -ENOMEM;
    }

    return 0;
}
