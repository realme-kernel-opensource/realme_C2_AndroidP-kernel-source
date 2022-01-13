#ifdef __cplusplus
//extern "C" {
#endif

#include <linux/string.h>
//#ifdef __cplusplus
//#include "tool.h"
//#endif

#include "test_function.h"

//#if !GTP_TEST_PARAMS_FROM_INI
//#include "test_params.h"
//#endif
/*Le.Wang@ODM_HQ.BSP.TP.Function, 2019/01/21 modified for ito*/
#define GT_Drvnum  13
#define GT_Sennum  28
#define GT_Config_Len  228
extern int test_error_code;
 int current_data_index=0;
extern  u16 accord_limit_temp[];
extern  u16 jitter_limit_temp[];
//extern u16 max_limit_vale_re[];
//extern u16 min_limit_vale_re[];
//extern u16 accord_limit_vale_re[];
extern  u16 max_limit_vale_id0[];
extern  u16 min_limit_vale_id0[];
extern  u16 accord_limit_vale_id0[];
extern  u16 jitter_limit_vale_id0[];

extern  u16 max_limit_vale_id2[];
extern  u16 min_limit_vale_id2[];
extern  u16 accord_limit_vale_id2[];

extern int ITO_Sensor_ID;

char *save_path="/etc/";
char save_result_dir[250] = "/data/rawdata/";
char save_data_path[250];
//extern int *channel_max_accord;
//int check_types_ito=0x0007;
extern int ITO_TEST_COUNT;
char itosavepath[50]={'\0'};
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2018/02/21 add for proc node*/
const u8 module_cfg0[]={\
0x42,0xD0,0x02,0x18,0x06,0x0A,0xFC,0x32,0x21,0x8F,\
0x32,0x0F,0x64,0x46,0x01,0x05,0x03,0x03,0x60,0x62,\
0x00,0x00,0x05,0x16,0x19,0x19,0x14,0xC7,0x26,0xEE,\
0x40,0x42,0xB8,0x08,0xB8,0x08,0x00,0xC3,0x32,0x91,\
0x00,0x00,0x00,0x00,0x00,0x02,0x40,0x08,0x14,0x00,\
0x1A,0x23,0x45,0x94,0x90,0x76,0x19,0x28,0x00,0x04,\
0xB8,0x25,0x00,0xA5,0x2B,0x00,0x97,0x31,0x00,0x8A,\
0x38,0x00,0x80,0x40,0x00,0x80,0x00,0x00,0x00,0x00,\
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x00,\
0x00,0x00,0x00,0x00,0xFF,0x7F,0x03,0x00,0x0F,0x32,\
0x00,0x03,0x00,0x00,0x32,0x06,0x55,0x00,0x00,0x00,\
0x00,0x41,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,\
0x16,0x17,0x18,0x19,0x1A,0x1B,0x00,0x01,0x02,0x03,\
0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,\
0xFF,0xFF,0x00,0x02,0x04,0x05,0x06,0x08,0x0A,0x1E,\
0x1F,0x20,0x22,0x24,0x2A,0xFF,0xFF,0xFF,0xFF,0x82,\
0x31,0x3C,0x3C,0x00,0x8C,0x70,0x12,0x00,0x29,0x35,\
0x67,0x00,0x1E,0x96,0xFF,0x00,0x06,0x00,0x00,0x0F,\
0x2F,0x8B,0x86,0x65,0x01,0x00,0x04,0x00,0x23,0x00,\
0x00,0x00,0x00,0x00,0x00,0x00,0x32,0x46,0x00,0x00,\
0x0F,0x14,0x00,0x32,0x46,0x00,0x8C,0x8C,0xAA,0x3C,\
0x96,0x0A,0x3C,0x57,0xE3,0x04,0x5A,0x28,0x06,0x19,\
0x1E,0x03,0x0A,0x00,0x00,0x00,0xAF,0x01\
};
const u8 module_cfg2[]={\
};
//#include "test_params.h"
struct system_variable sys;
static mm_segment_t old_fs;
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/14 modified for node function */
loff_t file_pos = 0;
void TP_init(void)
{
  GTP_INFO("GXT:TP-INIT");
  //FILE *fp = NULL;
        sys.chip_type =_GT9P;
        sys.chip_name="GT9110";
        sys.config_length=186;
        sys.driver_num=42;
        sys.sensor_num=30;
        sys.max_driver_num=42;
        sys.max_sensor_num=30;
        sys.key_number=0;
	//fp = fopen("/sdcard/ITO_Test_Data.csv", "a+");
	//fclose(fp);

}
FILE *fopen(const char *path, const char *mode)
{

	FILE *filp = NULL;

	if (!strcmp(mode, "a+")) {
		if(file_pos == 0) {
			//filp = filp_open(path, O_RDWR | O_CREAT | O_TRUNC, 0666);
			filp = filp_open(path, O_RDWR | O_CREAT, 0666);
		}else {
			filp = filp_open(path, O_RDWR | O_CREAT, 0666);
			//filp = filp_open(path, O_RDWR | O_CREAT | O_TRUNC, 0666);
		}
		if (!IS_ERR(filp)) {
			filp->f_op->llseek(filp, 0, SEEK_END);
			//filp->f_op->llseek(filp, 0, SEEK_SET);

		}

		if(filp == NULL){
			pr_err("open file as a+ mode filp == NULL 1\n");
		}

	} else if (!strcmp(mode, "r")) {

		filp = filp_open(path, O_RDONLY, 0666);

	}



	old_fs = get_fs();

	set_fs(KERNEL_DS);
	if(filp == NULL){
		pr_err("open file as a+ mode filp == NULL 2\n");
	}
	return filp;
}

int fclose(FILE * filp)
{
	filp_close(filp, NULL);

	filp = NULL;

	set_fs(old_fs);

	return 0;
}
 size_t fread(void *buffer, size_t size, size_t count, FILE * filp)
{
	return filp->f_op->read(filp, (char *)buffer, count, &filp->f_pos);
}

size_t fwrite(const void *buffer, size_t size, size_t count, FILE * filp)
{
	ssize_t  writeCount = -1;
	writeCount = vfs_write(filp, (char *)buffer, size, &file_pos);
	//file_pos = file_pos + size;
	return writeCount;
}
extern int gtp_ito_lcmoff_flag;

//#if GTP_SAVE_TEST_DATA
 s32 Save_testing_data(char *save_test_data_dir, int test_types,u16 *current_rawdata_temp)
{
	FILE *fp = NULL;
	s32 ret;
	s32 tmp = 0;
	u8 *data = NULL;
	s32 i = 0, j=0;
	s32 bytes = 0;
	int max, min;
	int average;
	data = (u8 *) malloc(_GT9_MAX_BUFFER_SIZE);


	  GTP_INFO("Save_testing_data---in");
	if (NULL == data) {

		GTP_ERROR(" memory error!");
				GTP_INFO(" memory error!");

		return MEMORY_ERR;
	}
	for(i=0;i<_GT9_MAX_BUFFER_SIZE;i++)
		{
		data[i]=0;
		}
    if (gtp_ito_lcmoff_flag !=1){
	sprintf((char *)itosavepath, "/sdcard/ITO_Test_Data_lcmon.csv");
	}
	else{
	sprintf((char *)itosavepath, "/sdcard/ITO_Test_Data_lcmoff.csv");
	}
	//fp = fopen("/sdcard/ITO_Test_Data.csv", "a+");
	fp = fopen(itosavepath, "a+");
	//fp->f_op->llseek(fp, 0, SEEK_SET);
	//fp=fopen((char *)save_test_data_dir,"a+");
	if (NULL == fp) {
		//WARNING("open %s failed!", save_test_data_dir);
		GTP_ERROR("open %s failed!", save_test_data_dir);
		free(data);
		return FILE_OPEN_CREATE_ERR;
	}


	if (current_data_index == 0) {
		bytes = (s32) sprintf((char *)data, "Device Type:%s\n", "GT917D");
		bytes += (s32) sprintf((char *)&data[bytes], "Config:\n");
		if(ITO_Sensor_ID==0)
			{
			for (i = 0; i < GT_Config_Len; i++)
				{
					bytes += (s32) sprintf((char *)&data[bytes], "0x%02X,", module_cfg0[i]);
				}
			}
		if(ITO_Sensor_ID==2)
			{
			for (i = 0; i < GT_Config_Len; i++)
				{
					bytes += (s32) sprintf((char *)&data[bytes], "0x%02X,", module_cfg2[i]);
				}
			}


		bytes += (s32) sprintf((char *)&data[bytes], "\n");
		ret = fwrite(data, bytes, 1, fp);
		bytes = 0;
		if (ret < 0) {
			GTP_ERROR("write to file fail.");
			goto exit_save_testing_data;
		}

		if ((test_types & _MAX_CHECK) != 0) {
			bytes = (s32) sprintf((char *)data, "Channel maximum:\n");
			for (i = 0; i < GT_Sennum; i++) {
				for (j = 0; j < GT_Drvnum; j++) {
					if(ITO_Sensor_ID==0)
						{
						bytes += (s32) sprintf((char *)&data[bytes], "%d,", max_limit_vale_id0[i + j * GT_Sennum]);
						}
					if(ITO_Sensor_ID==2)
						{
						bytes += (s32) sprintf((char *)&data[bytes], "%d,", max_limit_vale_id2[i + j * GT_Sennum]);
						}

				}

				bytes += (s32) sprintf((char *)&data[bytes], "\n");
				ret = fwrite(data, bytes, 1, fp);
				bytes = 0;
				if (ret < 0) {
					GTP_ERROR("write to file fail.");
					goto exit_save_testing_data;
				}
			}

		/*	j = 0;
			bytes = 0;
			for (i = 0; i < sys.key_number; i++) {
				if (_node_in_key_chn(i + sys.sc_sensor_num * sys.sc_driver_num, module_cfg, sys.key_offest) < 0) {
					continue;
				}
				bytes += (s32) sprintf((char *)&data[bytes], "%d,", max_key_limit_value[j++]);
				//pr_err("max[%d]%d",i,max_key_limit_value[i]);
			}

			bytes += (s32) sprintf((char *)&data[bytes], "\n");
			ret = fwrite(data, bytes, 1, fp);
			bytes = 0;
			if (ret < 0) {
				GTP_ERROR("write to file fail.");
				goto exit_save_testing_data;
			}
			*/
		}

		if ((test_types & _MIN_CHECK) != 0) {
			//GTP_INFO("Channel minimum:");
			bytes = (s32) sprintf((char *)data, "\nChannel minimum:\n");
			for (i = 0; i < GT_Sennum; i++) {
				for (j = 0; j < GT_Drvnum; j++) {
					if(ITO_Sensor_ID==0)
						{
						bytes += (s32) sprintf((char *)&data[bytes], "%d,", min_limit_vale_id0[i + j * GT_Sennum]);
						}
					if(ITO_Sensor_ID==2)
						{
						bytes += (s32) sprintf((char *)&data[bytes], "%d,", min_limit_vale_id2[i + j * GT_Sennum]);
						}

					//bytes += (s32) sprintf((char *)&data[bytes], "%d,", min_limit_vale_re[i + j * 30]);
				}

				bytes += (s32) sprintf((char *)&data[bytes], "\n");
				ret = fwrite(data, bytes, 1, fp);
				bytes = 0;
				if (ret < 0) {
					GTP_ERROR("write to file fail.");
					goto exit_save_testing_data;
				}
			}

		/*	j = 0;
			bytes = 0;
			for (i = 0; i < sys.key_number; i++) {
				if (_node_in_key_chn(i + sys.sc_sensor_num * sys.sc_driver_num, module_cfg, sys.key_offest) < 0) {
					continue;
				}
				bytes += (s32) sprintf((char *)&data[bytes], "%d,", min_key_limit_value[j++]);
			}
			bytes += (s32) sprintf((char *)&data[bytes], "\n");
			ret = fwrite(data, bytes, 1, fp);
			if (ret < 0) {
				GTP_ERROR("write to file fail.");
				goto exit_save_testing_data;
			}
			*/
		}

		if ((test_types & _ACCORD_CHECK) != 0) {
			//GTP_INFO("Channel average:");
			bytes = (s32) sprintf((char *)data, "\nChannel average:(%d)\n", FLOAT_AMPLIFIER);
			for (i = 0; i < GT_Sennum; i++) {
				for (j = 0; j < GT_Drvnum; j++) {
					if(ITO_Sensor_ID==0)
						{
						bytes += (s32) sprintf((char *)&data[bytes], "%d,", accord_limit_vale_id0[i + j * GT_Sennum]);
						}
					if(ITO_Sensor_ID==2)
						{
						bytes += (s32) sprintf((char *)&data[bytes], "%d,", accord_limit_vale_id2[i + j * GT_Sennum]);
						}
					//bytes += (s32) sprintf((char *)&data[bytes], "%d,", accord_limit_vale_re[i + j * 30]);
				}

				bytes += (s32) sprintf((char *)&data[bytes], "\n");
				ret = fwrite(data, bytes, 1, fp);
				bytes = 0;
				if (ret < 0) {
					GTP_ERROR("write to file fail.");
					goto exit_save_testing_data;
				}
			}
			//bytes += (s32) sprintf((char *)&data[bytes], "\0");
			bytes = (s32) sprintf((char *)data, "\n");
			ret = fwrite(data, bytes, 1, fp);
			if (ret < 0) {
				GTP_ERROR("write to file fail.");
				goto exit_save_testing_data;
			}
			/*
			if (sys.chip_type == _GT1143) {
				bytes = (s32) sprintf((char *)data, "Channel line accord:(%d)\n", FLOAT_AMPLIFIER);
				for (i = 0; i < 30; i++) {
					for (j = 0; j < 42; j++) {
						bytes += (s32) sprintf((char *)&data[bytes], "%d,", accord_limit_vale_re[i + j * sys.sc_sensor_num]);
					}
					//bytes += (s32) sprintf((char *)&data[bytes], "\0");
					bytes += (s32) sprintf((char *)&data[bytes], "\n");
					ret = fwrite(data, bytes, 1, fp);
					bytes = 0;
					if (ret < 0) {
						GTP_ERROR("GXT,write to file fail.");
						goto exit_save_testing_data;
					}
				}
				//bytes += (s32) sprintf((char *)&data[bytes], "\0");
				bytes = (s32) sprintf((char *)data, "\n");
				ret = fwrite(data, bytes, 1, fp);
				if (ret < 0) {
					GTP_ERROR("GXT,write to file fail.");
					goto exit_save_testing_data;
				}
			}
		*/
		}

		bytes = (s32) sprintf((char *)data, " Rawdata\n");
		ret = fwrite(data, bytes, 1, fp);
		if (ret < 0) {
			GTP_ERROR("write to file fail.");
			goto exit_save_testing_data;
		}
	}


	bytes = (s32) sprintf((char *)data, "No.%d\n", current_data_index);
	ret = fwrite(data, bytes, 1, fp);
	if (ret < 0) {
		GTP_ERROR("write to file fail.");
		goto exit_save_testing_data;
	}

	max=1000;
	min=5000;
	average = 0;

	for (i = 0; i < GT_Sennum; i++) {
		bytes = 0;
		for (j = 0; j <GT_Drvnum; j++) {
			tmp = current_rawdata_temp[i + j * GT_Sennum];
			bytes += (s32) sprintf((char *)&data[bytes], "%d,", tmp);
			if (tmp > max) {
				max = tmp;
			}
			if (tmp < min) {
				min = tmp;
			}
			average += tmp;
		}
		bytes += (s32) sprintf((char *)&data[bytes], "\n");
		ret = fwrite(data, bytes, 1, fp);
		if (ret < 0) {
			GTP_ERROR("write to file fail.");
			goto exit_save_testing_data;
		}
	}
	average = average / (GT_Drvnum* GT_Sennum);

/*
	if (sys.key_number > 0) {
		bytes = (s32) sprintf((char *)data, "Key Rawdata:\n");
		for (i = 0; i < sys.key_number; i++) {
			if (_node_in_key_chn(i + sys.sc_sensor_num * sys.sc_driver_num, module_cfg, sys.key_offest) < 0) {
				continue;
			}
			bytes += (s32) sprintf((char *)&data[bytes], "%d,", current_rawdata_temp[i + sys.sc_sensor_num * sys.sc_driver_num]);
		}
		bytes += (s32) sprintf((char *)&data[bytes], "\n");
		ret = fwrite(data, bytes, 1, fp);
		if (ret < 0) {
			GTP_ERROR("write to file fail.");
			goto exit_save_testing_data;
		}
	}
*/
	bytes = (s32) sprintf((char *)data, "  Maximum:%d  Minimum:%d  Average:%d\n\n", max, min, average);
	ret = fwrite(data, bytes, 1, fp);
	if (ret < 0) {
		GTP_ERROR("write to file fail.");
		goto exit_save_testing_data;
	}

	if ((test_types & _ACCORD_CHECK) != 0) {
		bytes = (s32) sprintf((char *)data, "Channel_Accord :(%d)\n", FLOAT_AMPLIFIER);
		ret = fwrite(data, bytes, 1, fp);
		if (ret < 0) {
			GTP_ERROR("write to file fail.");
			goto exit_save_testing_data;
		}
		for (i = 0; i < GT_Sennum; i++) {
			bytes = 0;
			for (j = 0; j < GT_Drvnum; j++) {
				//GTP_ERROR("%d",accord_limit_temp[i+j*GT_Sennum]);
				bytes += (s32) sprintf((char *)&data[bytes], "%d.%d%d%d,",
					(accord_limit_temp[i + j * GT_Sennum])/1000,
					((accord_limit_temp[i + j * GT_Sennum])%1000)/100,
					((accord_limit_temp[i + j * GT_Sennum])%100)/10,
					(accord_limit_temp[i + j * GT_Sennum])%10);

			}
			bytes += (s32) sprintf((char *)&data[bytes], "\n");
			ret = fwrite(data, bytes, 1, fp);
			if (ret < 0) {
				GTP_ERROR("write to file fail.");
				goto exit_save_testing_data;
			}
		}

		bytes = (s32) sprintf((char *)data, "\n");
		ret = fwrite(data, bytes, 1, fp);
		if (ret < 0) {
			GTP_ERROR("write to file fail.");
			goto exit_save_testing_data;
		}
	}
	if((current_data_index == 15) && ((test_types & _JITTER_CHECK) != 0)) {
		bytes = (s32) sprintf((char *)data, "Diff data :\n");
		ret = fwrite(data, bytes, 1, fp);
		if (ret < 0) {
			GTP_ERROR("write to file fail.");
			goto exit_save_testing_data;
		}
		for (i = 0; i < GT_Sennum; i++) {
			bytes = 0;
			for (j = 0; j < GT_Drvnum; j++) {
				//GTP_ERROR("%d",accord_limit_temp[i+j*GT_Sennum]);
				bytes += (s32) sprintf((char *)&data[bytes], "%d,",
					jitter_limit_temp[i + j * GT_Sennum]);
			}
			bytes += (s32) sprintf((char *)&data[bytes], "\n");
			ret = fwrite(data, bytes, 1, fp);
			if (ret < 0) {
				GTP_ERROR("write to file fail.");
				goto exit_save_testing_data;
			}
		}
	}


exit_save_testing_data:
	/* pr_err("step4"); */
	free(data);
	/* pr_err("step3"); */
	fclose(fp);
	/* pr_err("step2"); */
	return ret;
}
 
 s32 Save_test_result_data(char *save_test_data_dir, int test_types)//, u8 * shortresult)
{
	FILE *fp = NULL;
	s32 ret;
	//s32 index;
	u8 *data = NULL;
	s32 bytes = 0;

	data = (u8 *) malloc(_GT9_MAX_BUFFER_SIZE);
	if (NULL == data) {
		GTP_ERROR("memory error!");
		return MEMORY_ERR;
	}
	GTP_ERROR("before fopen patch = %s\n",save_test_data_dir);
	//fp = fopen("/sdcard/ITO_Test_Data.csv", "a+");
	if(gtp_ito_lcmoff_flag != 1){
	  sprintf((char *)itosavepath, "/sdcard/ITO_Test_Data_lcmon.csv");
	  }
	  else{
      sprintf((char *)itosavepath, "/sdcard/ITO_Test_Data_lcmoff.csv");

	  }
	//sprintf((char *)itosavepath, "/sdcard/ITO_Test_Data.txt");
	fp = fopen(itosavepath, "a+");
	//fp=fopen((char *)save_test_data_dir,"a+");
	if (NULL == fp) {
		GTP_ERROR("open %s failed!", save_test_data_dir);
		free(data);
		return FILE_OPEN_CREATE_ERR;
	}

	bytes = (s32) sprintf((char *)data, "Test Result:");
	if (test_error_code == _CHANNEL_PASS) {
		bytes += (s32) sprintf((char *)&data[bytes], "Pass\n\n");
	} else {
		bytes += (s32) sprintf((char *)&data[bytes], "Fail\n\n");
	}
	bytes += (s32) sprintf((char *)&data[bytes], "Test items:\n");
	if ((test_types & _MAX_CHECK) != 0) {
		bytes += (s32) sprintf((char *)&data[bytes], "Max Rawdata:  ");
		if (test_error_code & _BEYOND_MAX_LIMIT) {
			bytes += (s32) sprintf((char *)&data[bytes], "NG !\n");
		} else {
			bytes += (s32) sprintf((char *)&data[bytes], "pass\n");
		}
	}

	if ((test_types & _MIN_CHECK) != 0) {
		bytes += (s32) sprintf((char *)&data[bytes], "Min Rawdata:  ");
		if (test_error_code & _BEYOND_MIN_LIMIT) {
			bytes += (s32) sprintf((char *)&data[bytes], "NG !\n");
		} else {
			bytes += (s32) sprintf((char *)&data[bytes], "pass\n");
		}
	}

	if ((test_types & _ACCORD_CHECK) != 0) {
		bytes += (s32) sprintf((char *)&data[bytes], "Area Accord:  ");

		if (test_error_code & _BETWEEN_ACCORD_AND_LINE) {
			bytes += (s32) sprintf((char *)&data[bytes], "Fuzzy !\n");
		} else {
			if (test_error_code & _BEYOND_ACCORD_LIMIT) {

				bytes += (s32) sprintf((char *)&data[bytes], "NG !\n");

			} else {

				bytes += (s32) sprintf((char *)&data[bytes], "pass\n");

			}
		}
	}
	//
	if ((test_types & _SHORT_CHECK) != 0) {
		bytes += (s32) sprintf((char *)&data[bytes], "Moudle Short Test:  ");
		if (test_error_code & _SENSOR_SHORT) {
			bytes += (s32) sprintf((char *)&data[bytes], "NG !\n");
		} else {
			bytes += (s32) sprintf((char *)&data[bytes], "pass\n");
		}
	}

	//
	if ((test_types & _OFFSET_CHECK) != 0) {

		bytes += (s32) sprintf((char *)&data[bytes], "Max Offest:  ");

		if (test_error_code & _BEYOND_OFFSET_LIMIT) {

			bytes += (s32) sprintf((char *)&data[bytes], "NG !\n");

		}

		else {

			bytes += (s32) sprintf((char *)&data[bytes], "pass\n");

		}
	}

	if ((test_types & _JITTER_CHECK) != 0) {

		bytes += (s32) sprintf((char *)&data[bytes], "Max Jitier:  ");

		if (test_error_code & _BEYOND_JITTER_LIMIT) {

			bytes += (s32) sprintf((char *)&data[bytes], "NG !\n");

		}

		else {

			bytes += (s32) sprintf((char *)&data[bytes], "pass\n");

		}
	}

	if (test_types & _UNIFORMITY_CHECK) {

		bytes += (s32) sprintf((char *)&data[bytes], "Uniformity:  ");

		if (test_error_code & _BEYOND_UNIFORMITY_LIMIT) {

			bytes += (s32) sprintf((char *)&data[bytes], "NG !\n");

		}

		else {

			bytes += (s32) sprintf((char *)&data[bytes], "pass\n");

		}
	}

	if ((test_types & _KEY_MAX_CHECK) != 0) {

		bytes += (s32) sprintf((char *)&data[bytes], "Key Max Rawdata:  ");

		if (test_error_code & _KEY_BEYOND_MAX_LIMIT) {

			bytes += (s32) sprintf((char *)&data[bytes], "NG !\n");

		}

		else {

			bytes += (s32) sprintf((char *)&data[bytes], "pass\n");

		}
	}

	if ((test_types & _KEY_MIN_CHECK) != 0) {

		bytes += (s32) sprintf((char *)&data[bytes], "Key Min Rawdata:  ");

		if (test_error_code & _KEY_BEYOND_MIN_LIMIT) {

			bytes += (s32) sprintf((char *)&data[bytes], "NG !\n");

		}

		else {

			bytes += (s32) sprintf((char *)&data[bytes], "pass\n");

		}
	}

	if (test_types & (_VER_EQU_CHECK | _VER_GREATER_CHECK | _VER_BETWEEN_CHECK)) {

		bytes += (s32) sprintf((char *)&data[bytes], "Device Version:  ");

		if (test_error_code & _VERSION_ERR) {

			bytes += (s32) sprintf((char *)&data[bytes], "NG !\n");

		}

		else {

			bytes += (s32) sprintf((char *)&data[bytes], "pass\n");

		}
	}

	if (test_types & _MODULE_TYPE_CHECK) {

		bytes += (s32) sprintf((char *)&data[bytes], "Module Type:  ");

		if (test_error_code & _MODULE_TYPE_ERR) {

			bytes += (s32) sprintf((char *)&data[bytes], "NG !\n");

		}

		else {

			bytes += (s32) sprintf((char *)&data[bytes], "pass\n");

		}
	}

	ret = fwrite(data, bytes, 1, fp);

	if (ret < 0) {

		GTP_ERROR("write to file fail.");

		free(data);

		fclose(fp);

		return ret;

	}


/*	if ((test_types & _MODULE_SHORT_CHECK) != 0) {

		bytes = (s32) sprintf((char *)data, "Module short test:  ");

		if (test_error_code & _GT_SHORT) {

			bytes += (s32) sprintf((char *)&data[bytes], "NG !\n\n\nError items:\nShort:\n");

			if (shortresult[0] > _GT9_UPLOAD_SHORT_TOTAL) {

				WARNING("short total over limit, data error!");

				shortresult[0] = 0;

			}

			for (index = 0; index < shortresult[0]; index++) {

				//pr_err("bytes=%d shortresult[0]=%d",bytes,shortresult[0]);
				if (shortresult[1 + index * 4] & 0x80) {

					bytes += (s32) sprintf((char *)&data[bytes], "Drv%d - ", shortresult[1 + index * 4] & 0x7F);

				}

				else {

					if (shortresult[1 + index * 4] == (sys.max_driver_num + 1)) {

						bytes += (s32) sprintf((char *)&data[bytes], "GND\\VDD%d - ", shortresult[1 + index * 4] & 0x7F);

					}

					else {

						bytes += (s32) sprintf((char *)&data[bytes], "Sen%d - ", shortresult[1 + index * 4] & 0x7F);

					}
				}
				if (shortresult[2 + index * 4] & 0x80) {

					bytes += (s32) sprintf((char *)&data[bytes], "Drv%d 之间短路", shortresult[2 + index * 4] & 0x7F);

				}

				else {

					if (shortresult[2 + index * 4] == (sys.max_driver_num + 1)) {

						bytes += (s32) sprintf((char *)&data[bytes], "GND\\VDD 之间短路");

					}

					else {

						bytes += (s32) sprintf((char *)&data[bytes], "Sen%d 之间短路", shortresult[2 + index * 4] & 0x7F);

					}
				}
				bytes += (s32) sprintf((char *)&data[bytes], "(R=%d Kohm)\n", (((shortresult[3 + index * 4] << 8) + shortresult[4 + index * 4]) & 0xffff) / 10);

				pr_err("%d&%d:", shortresult[1 + index * 4], shortresult[2 + index * 4]);

				pr_err("%dK", (((shortresult[3 + index * 4] << 8) + shortresult[4 + index * 4]) & 0xffff) / 10);

			}
		}

		else {

			bytes += (s32) sprintf((char *)&data[bytes], "pass\n");

		}
		ret = fwrite(data, bytes, 1, fp);

		if (ret < 0) {

			WARNING("write to file fail.");

			free(data);

			fclose(fp);

			return ret;

		}

	}
*/
	free(data);

	fclose(fp);

	return 1;

}

//#endif
