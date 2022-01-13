/* drivers/input/touchscreen/gt9xx_shorttp.c
 *
 * 2010 - 2012 Goodix Technology.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be a reference
 * to you, when you are integrating the GOODiX's CTP IC into your system,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * Version:1.0
 * Author: meta@goodix.com
 * Accomplished Date:2012/10/20
 * Revision record:
 *
 */

#include "gt9xx_openshort.h"
#include "test_function.h"
#include <linux/time.h>
//add by 101003082 for ITO test at 2018/06/07 begin
extern int current_data_index;
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/14 modified for node function */
extern loff_t file_pos;
extern unsigned int oppo_debug_level;
int test_error_code;
u8 gtp_ito_test_on = 0;
char *ito_save_path="/sdcard/";
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/01/18 modified for ito log save*/
int gtp_ito_lcmoff_flag=0;

static int gtp_ito_test_show(struct seq_file *file, void* data);

extern int ITO_TEST_COUNT;

extern  s32 Save_testing_data(char *save_test_data_dir, int test_types,u16 *current_rawdata_temp);
extern  s32 Save_test_result_data(char *save_test_data_dir, int test_types);//, u8 * shortresult);
//add by 101003082 for ITO test at 2018/06/07 end
//************** Customer Config Start ***********************//
// short test
#define GTP_SHORT_GND
#define GTP_VDD         33      // 2.8V

// open test
#define DEFAULT_TEST_ITEMS  (_MAX_TEST | _MIN_TEST | _ACCORD_TEST/*| _KEY_MAX_TEST | _KEY_MIN_TEST | _UNIFORMITY_TEST*/)


#define ALL_FLAG 		 0
#define IQC_FLAG 		 1
#define OQC_FLAG 		 2
#define REPAIR_FLAG  3

//TODO: define your own default or for Sensor_ID == 0 config here.

#define MAX_LIMIT_VALUE_GROUP1 5000	// screen max limit
#define MIN_LIMIT_VALUE_GROUP1 1000		// screen min limit
#define MAX_LIMIT_KEY_GROUP1 9000		// key_val max limit
#define MIN_LIMIT_KEY_GROUP1 0		// key_val min limit
#define UNIFORMITY_GROUP1 70			// screen uniformity in percent
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2018/02/21 add for proc node*/
#define CTP_TEST_CFG_GROUP1 {\
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
}

//TODO: define your config for Sensor_ID == 1 here, if needed
//REPAIR_FLAG 22%
#define MAX_LIMIT_VALUE_GROUP2 5000	// screen max limit
#define MIN_LIMIT_VALUE_GROUP2 1088		// screen min limit
#define MAX_LIMIT_KEY_GROUP2 9000		// key_val max limit
#define MIN_LIMIT_KEY_GROUP2 0		// key_val min limit
#define UNIFORMITY_GROUP2 70			// screen uniformity in percent

#define CTP_TEST_CFG_GROUP2 {\
}

//TODO: define your config for Sensor_ID == 2 here, if needed


#define MAX_LIMIT_VALUE_GROUP3 3500 	// screen max limit
#define MIN_LIMIT_VALUE_GROUP3 1000		// screen min limit
#define MAX_LIMIT_KEY_GROUP3 3600		// key_val max limit
#define MIN_LIMIT_KEY_GROUP3 1100		// key_val min limit
#define UNIFORMITY_GROUP3 70 			// screen uniformity in percent

#define CTP_TEST_CFG_GROUP3 {\
}

//TODO: define your config for Sensor_ID == 3 here, if needed


#define MAX_LIMIT_VALUE_GROUP4 3500 	// screen max limit
#define MIN_LIMIT_VALUE_GROUP4 1000		// screen min limit
#define MAX_LIMIT_KEY_GROUP4 3600		// key_val max limit
#define MIN_LIMIT_KEY_GROUP4 1100		// key_val min limit
#define UNIFORMITY_GROUP4 70 			// screen uniformity in percent

#define CTP_TEST_CFG_GROUP4 {\
}

//TODO: define your config for Sensor_ID == 4 here, if needed

#define MAX_LIMIT_VALUE_GROUP5 3500 	// screen max limit
#define MIN_LIMIT_VALUE_GROUP5 1000		// screen min limit
#define MAX_LIMIT_KEY_GROUP5 3600		// key_val max limit
#define MIN_LIMIT_KEY_GROUP5 1100		// key_val min limit
#define UNIFORMITY_GROUP5 70 			// screen uniformity in percent

#define CTP_TEST_CFG_GROUP5 {\
}

//TODO: define your config for Sensor_ID == 5 here, if needed


#define MAX_LIMIT_VALUE_GROUP6 3500 	// screen max limit
#define MIN_LIMIT_VALUE_GROUP6 1000		// screen min limit
#define MAX_LIMIT_KEY_GROUP6 3600		// key_val max limit
#define MIN_LIMIT_KEY_GROUP6 1100		// key_val min limit
#define UNIFORMITY_GROUP6 70 			// screen uniformity in percent

#define CTP_TEST_CFG_GROUP6 {\
}



#define DSP_SHORT_BURN_CHK          256 // burn short chuck size
#define _SHORT_INFO_MAX             50  // short test max show 50 pairs short channels
#define _BEYOND_INFO_MAX            20  // open test max show 20 infos for each test item
#define GTP_OPEN_SAMPLE_NUM         16  // open test raw data sampled count
#define GTP_TEST_INFO_MAX           200 // test info lines max count

#define TEST_RSLT_ARCHIVE_PATH      "/data/gtp_test_rslt.txt"
//****************** Customer Config End ***********************//
static u16 max_limit_value_info[] = {
	MAX_LIMIT_VALUE_GROUP1,
	MAX_LIMIT_VALUE_GROUP2,
	MAX_LIMIT_VALUE_GROUP3,
	MAX_LIMIT_VALUE_GROUP4,
	MAX_LIMIT_VALUE_GROUP5,
	MAX_LIMIT_VALUE_GROUP6
};

static u16 min_limit_value_info[] = {
	MIN_LIMIT_VALUE_GROUP1,
	MIN_LIMIT_VALUE_GROUP2,
	MIN_LIMIT_VALUE_GROUP3,
	MIN_LIMIT_VALUE_GROUP4,
	MIN_LIMIT_VALUE_GROUP5,
	MIN_LIMIT_VALUE_GROUP6
};

static u16 max_limit_key_info[] = {
	MAX_LIMIT_KEY_GROUP1,
	MAX_LIMIT_KEY_GROUP2,
	MIN_LIMIT_KEY_GROUP3,
	MAX_LIMIT_KEY_GROUP4,
	MAX_LIMIT_KEY_GROUP5,
	MAX_LIMIT_KEY_GROUP6
};

static u16 min_limit_key_info[] = {
	MIN_LIMIT_KEY_GROUP1,
	MIN_LIMIT_KEY_GROUP2,
	MIN_LIMIT_KEY_GROUP3,
	MIN_LIMIT_KEY_GROUP4,
	MIN_LIMIT_KEY_GROUP5,
	MIN_LIMIT_KEY_GROUP6
};

static u16 uniformity_lmt_info[] = {
	UNIFORMITY_GROUP1,
	UNIFORMITY_GROUP2,
	UNIFORMITY_GROUP3,
	UNIFORMITY_GROUP4,
	UNIFORMITY_GROUP5,
	UNIFORMITY_GROUP6
};

static u8 test_cfg_info_group1[] =  CTP_TEST_CFG_GROUP1;
static u8 test_cfg_info_group2[] =  CTP_TEST_CFG_GROUP2;
static u8 test_cfg_info_group3[] =  CTP_TEST_CFG_GROUP3;
static u8 test_cfg_info_group4[] =  CTP_TEST_CFG_GROUP4;
static u8 test_cfg_info_group5[] =  CTP_TEST_CFG_GROUP5;
static u8 test_cfg_info_group6[] =  CTP_TEST_CFG_GROUP6;

static u8 *send_test_cfg_buf[] = {
	test_cfg_info_group1,
	test_cfg_info_group2,
	test_cfg_info_group3,
	test_cfg_info_group4,
	test_cfg_info_group5,
	test_cfg_info_group6,
};

static u8 tset_cfg_info_len[] = {
	CFG_GROUP_LEN(test_cfg_info_group1),
        CFG_GROUP_LEN(test_cfg_info_group2),
        CFG_GROUP_LEN(test_cfg_info_group3),
        CFG_GROUP_LEN(test_cfg_info_group4),
        CFG_GROUP_LEN(test_cfg_info_group5),
        CFG_GROUP_LEN(test_cfg_info_group6)
};

u16 max_limit_value = 3500;     // screen max limit
u16 min_limit_value = 1000;     // screen min limit
u16 max_limit_key = 3600;       // key_val max limit
u16 min_limit_key = 1100;        // key_val min limit
u16 uniformity_lmt = 70;        // screen uniformity in percent

/************************default *******************************/
u16 max_limit_vale_re[]={\
4359,6008,5975,5905,6008,5924,5917,5825,5886,5836,5788,5704,5766,5756,5536,4870,4829,4758,4782,4743,4757,4713,4780,4740,4762,4736,4797,4785,3806,
5951,5983,5837,5822,5872,5840,5776,5732,5732,5723,5630,5580,5590,5587,5240,5266,5143,5102,5091,5087,5074,5056,5104,5097,5056,5052,5128,5135,4920,
5709,5633,5552,5553,5543,5504,5443,5424,5384,5338,5317,5311,5273,5279,4960,5018,5076,5044,5038,5045,5024,5016,5066,5072,5055,5049,5108,5114,4926,
5870,5676,5531,5520,5574,5420,5490,5451,5436,5416,5325,5289,5301,5315,4975,5185,5087,5058,5042,5048,5034,5023,5073,5077,5031,5021,5109,5116,4927,
5903,5658,5515,5508,5556,5371,5473,5433,5420,5396,5304,5272,5290,5305,4964,5207,5121,5088,5076,5081,5070,5058,5105,5109,5056,5053,5140,5151,4962,
5576,5536,5399,5384,5430,5290,5342,5301,5296,5275,5189,5157,5178,5199,4870,5167,5091,5060,5048,5058,5044,5032,5083,5097,5046,5035,5126,5139,4960,
2300,5458,5434,5339,5465,5249,5370,5254,5321,5231,5212,5116,5200,5160,4888,5179,5150,5077,5115,5076,5108,5056,5149,5114,5153,5100,5184,5161,5003,
2300,5114,5157,5065,5137,5023,5039,4954,4995,4890,4939,4874,4901,4853,4590,4922,4967,4930,5011,4975,4993,4940,5030,5017,5067,5020,5056,5009,4859,
5210,5157,5093,5079,5074,5030,4976,4957,4932,4894,4881,4880,4842,4852,4544,4992,4990,4993,5027,5035,5002,4996,5044,5074,5086,5073,5095,5070,4877,
5497,5125,5062,5053,5042,5000,4943,4929,4897,4860,4848,4852,4807,4829,4528,5027,5020,5024,5046,5060,5027,5020,5063,5094,5101,5098,5125,5100,4894,
5480,5077,5016,5011,5000,4960,4904,4885,4856,4814,4797,4803,4766,4786,4498,5046,5037,5045,5063,5079,5049,5063,5105,5133,5140,5135,5154,5136,4929,
5501,5065,5007,5003,4992,4951,4895,4880,4850,4808,4792,4789,4761,4778,4486,5097,5086,5095,5107,5126,5100,5115,5150,5174,5178,5164,5168,5160,4962,
5429,5060,5007,5006,4997,4957,4902,4884,4856,4817,4792,4799,4772,4790,4496,5219,5202,5206,5220,5233,5203,5199,5234,5261,5265,5247,5247,5234,5017,
3994,4674,4680,4631,4673,4587,4587,4528,4549,4463,4485,4456,4474,4454,4209,5256,5268,5233,5279,5241,5231,5200,5259,5247,5272,5224,5245,5206,4146,
};

u16 min_limit_vale_re[]={\
1777,2483,2493,2465,2487,2445,2439,2410,2413,2374,2385,2364,2363,2356,2271,2017,2031,2015,2037,2025,2024,2003,2026,2023,2020,2004,2041,2026,1616,
2404,2457,2423,2422,2418,2403,2374,2367,2347,2327,2319,2314,2296,2300,2164,2148,2145,2146,2153,2154,2143,2137,2148,2159,2159,2152,2167,2156,2079,
2442,2377,2317,2312,2333,2314,2294,2278,2275,2269,2232,2214,2220,2223,2079,2139,2146,2149,2155,2155,2144,2134,2148,2158,2148,2143,2166,2156,2085,
2422,2373,2340,2339,2334,2256,2292,2285,2270,2251,2242,2241,2224,2227,2090,2141,2138,2141,2151,2153,2139,2128,2140,2150,2149,2145,2158,2149,2077,
2422,2352,2319,2317,2312,2251,2274,2269,2252,2233,2224,2223,2206,2212,2074,2148,2144,2146,2155,2158,2143,2131,2143,2152,2154,2151,2160,2146,2080,
2280,2317,2286,2282,2279,2198,2241,2232,2218,2199,2193,2193,2178,2182,2045,2141,2139,2139,2147,2148,2139,2129,2144,2155,2158,2154,2157,2146,2077,
1200,2287,2304,2266,2296,2179,2254,2216,2234,2188,2208,2179,2191,2171,2055,2152,2170,2151,2183,2166,2176,2151,2186,2181,2182,2161,2197,2173,2115,
1200,2116,2110,2073,2124,1968,2083,2037,2066,2029,2023,1984,2019,2001,1896,2055,2046,2016,2034,2017,2031,2011,2049,2035,2028,2010,2061,2049,1989,
2190,2122,2071,2067,2086,1963,2049,2032,2033,2024,1990,1977,1987,1996,1868,2076,2046,2034,2029,2033,2024,2026,2046,2051,2029,2031,2068,2068,1999,
2314,2113,2062,2060,2077,1956,2040,2026,2024,2018,1983,1972,1981,1992,1863,2095,2063,2053,2049,2053,2043,2047,2062,2070,2046,2044,2079,2078,2007,
2313,2100,2052,2049,2065,1944,2029,2014,2013,2008,1975,1963,1974,1983,1856,2110,2080,2071,2067,2067,2057,2060,2076,2080,2056,2056,2089,2089,2013,
2319,2092,2044,2043,2060,1938,2023,2008,2007,2001,1968,1959,1972,1982,1854,2134,2103,2093,2090,2088,2078,2079,2097,2099,2074,2071,2102,2104,2022,
2285,2091,2045,2041,2060,1937,2024,2011,2009,2005,1972,1963,1977,1986,1860,2185,2147,2138,2134,2135,2119,2118,2134,2134,2106,2103,2130,2130,2041,
1626,1939,1921,1898,1938,1800,1904,1872,1891,1868,1857,1832,1861,1854,1749,2184,2160,2134,2148,2132,2131,2112,2137,2120,2104,2090,2125,2123,1675,
};
//1000*accord
u16 accord_limit_vale_re[]={\
559, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 412,
 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400,
 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400,
 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400,
 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400,
 739, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400,
1289, 729, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400,
1289, 743, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400,
 766, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400,
 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400,
 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400,
 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400,
 405, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400,
 568, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400,
};
u16 accord_limit_temp[]={\
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
u16 jitter_limit_temp[]={\
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

/************************sensor_id is 0**********************/
const u16 max_limit_vale_id0[]={
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
};

const u16 min_limit_vale_id0[]={
50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,
50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,
50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,
50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,
50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,
50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,
50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,
50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,
50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,
50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,
50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,
50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,
50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,
};

const u16 accord_limit_vale_id0[]={\
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
};

const u16 jitter_limit_vale_id0[]={\
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,
};
/************************sensor_id is 1**********************/
u16 max_limit_vale_id1[]={\
};

u16 min_limit_vale_id1[]={\
};

u16 accord_limit_vale_id1[]={\
};

u16 jitter_limit_vale_id1[]={\
};

/************************sensor_id is 2**********************/
const u16 max_limit_vale_id2[]={\
};
const u16 min_limit_vale_id2[]={\
};
//1000*accord
const u16 accord_limit_vale_id2[]={\
};

const u16 jitter_limit_vale_id2[]={\
};
/************************sensor_id is 3**********************/
u16 max_limit_vale_id3[]={\
};

u16 min_limit_vale_id3[]={\
};

u16 accord_limit_vale_id3[]={\
};
u16 jitter_limit_vale_id3[]={\
};
/************************sensor_id is 4**********************/
u16 max_limit_vale_id4[]={\
};

u16 min_limit_vale_id4[]={\
};

u16 accord_limit_vale_id4[]={\
};

u16 jitter_limit_vale_id4[]={\
};
/************************sensor_id is 5**********************/
u16 max_limit_vale_id5[]={\
};

u16 min_limit_vale_id5[]={\
};

u16 accord_limit_vale_id5[]={\
};

u16 jitter_limit_vale_id5[]={\
};


extern u8 grp_cfg_version;
extern s32 gtp_i2c_read(struct i2c_client *, u8 *, s32);
extern s32 gtp_i2c_write(struct i2c_client *, u8 *, s32);
extern s32 i2c_read_bytes(struct i2c_client *client, u16 addr, u8 *buf, s32 len);
extern s32 i2c_write_bytes(struct i2c_client *client, u16 addr, u8 *buf, s32 len);
extern void gtp_reset_guitar(struct i2c_client*, s32);
extern s32 gup_enter_update_mode(struct i2c_client *);
//extern s32 gup_leave_update_mode(void);
extern s32 gtp_send_cfg(struct i2c_client *client);
extern s32 gtp_read_version(struct i2c_client *client, u16* version);
extern void mt_eint_unmask(unsigned int line);
extern void mt_eint_mask(unsigned int line);
extern void gtp_irq_disable(void);
extern void gtp_irq_enable(void);
extern s32 gtp_i2c_read_dbl_check(struct i2c_client *client, u16 addr, u8 *rxbuf, int len);
extern struct i2c_client *i2c_client_point;



//extern u8 gtp_rawdiff_mode;
//extern struct i2c_client * i2c_client_point;
extern u8 cfg_len;
extern int tpd_halt;
extern u8 lcm_id;
extern u8 panel_id;
u8 MZ_CTPTEST_RE_FLAG=0;
u8 TP_id = 0;
u8  gt9xx_drv_num = MAX_DRIVER_NUM; // default driver and sensor number
u8  gt9xx_sen_num = MAX_SENSOR_NUM;
u16 gt9xx_pixel_cnt = MAX_DRIVER_NUM * MAX_SENSOR_NUM;
u16 gt9xx_sc_pxl_cnt = MAX_DRIVER_NUM * MAX_SENSOR_NUM;
struct gt9xx_short_info *short_sum;

u8 chip_type_gt9f = 0;
u8 have_key = 0;
u8 gt9xx_sc_drv_num;
u8 key_is_isolated; // 0: no, 1: yes
u8 key_iso_pos[5];

struct kobject *goodix_debug_kobj;
static u8  rslt_buf_idx = 0;
s32 *test_rslt_buf;
struct gt9xx_open_info *touchpad_sum;
static struct gt9xx_iot_result_info  *Ito_result_info;//gexiantao@20180502

//********** Test Result Archive *********//
static u16 key_isolated_avrg[4] = {0};
static u32 avrg_raw_len = 0;
static u32 *avrg_raw_buf;

#define _MIN_ERROR_NUM      (GTP_OPEN_SAMPLE_NUM * 9 / 10)

static char *result_lines[GTP_TEST_INFO_MAX];
static char tmp_info_line[80];
static u16 RsltIndex;

s32 gtp_jitter_test(struct i2c_client * client);
static void gtp_arch_file_append(char *str, s32 len);
static void gtp_arch_file_append_no_len(char *str)
{
    gtp_arch_file_append(str, strlen(str));
}

static void append_info_line(void)
{
    if(RsltIndex < GTP_TEST_INFO_MAX) {
	    if (strlen(tmp_info_line) != 0) {
		    result_lines[RsltIndex] = (char *)kzalloc(strlen(tmp_info_line)+1, GFP_KERNEL);
		    memcpy(result_lines[RsltIndex], tmp_info_line, strlen(tmp_info_line));
		    //gtp_arch_file_append_no_len(tmp_info_line);

		    ++RsltIndex;
	    }
    }
}



#define SET_INFO_LINE_INFO(fmt, args...)       do{ memset(tmp_info_line, '\0', 80);\
                                                   sprintf(tmp_info_line, "<Sysfs-INFO>"fmt"\n", ##args);\
                                                   GTP_INFO(fmt, ##args);\
                                                append_info_line();} while(0)

#define SET_INFO_LINE_ERR(fmt, args...)        do { memset(tmp_info_line, '\0', 80);\
                                                   sprintf(tmp_info_line, "<Sysfs-ERROR>"fmt"\n", ##args);\
                                                   GTP_ERROR(fmt, ##args);\
                                                   append_info_line();}while(0)


static void gtp_arch_file_append(char *str, s32 len)
{
    struct file *arch_filp;
    mm_segment_t old_fs;

    arch_filp = filp_open(TEST_RSLT_ARCHIVE_PATH, O_RDWR | O_CREAT, 0666);

    if (IS_ERR(arch_filp))
    {
        SET_INFO_LINE_ERR("Failed to open %s for test result archive!", TEST_RSLT_ARCHIVE_PATH);
        return;
    }
    old_fs = get_fs();
    set_fs(KERNEL_DS);
    arch_filp->f_op->llseek(arch_filp, 0, SEEK_END);    // append
    //arch_filp->f_op->write(arch_filp, str, len, &arch_filp->f_pos);
    vfs_write(arch_filp, str, len, &arch_filp->f_pos);
    filp_close(arch_filp, NULL);
    set_fs(old_fs);
}


static u8 cfg_drv_order[MAX_DRIVER_NUM];
static u8 cfg_sen_order[MAX_SENSOR_NUM];

static void gtptest_irq_disable(struct i2c_client * client)
{
    //mt_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);  
	//struct goodix_ts_data *ts;
	//ts = i2c_get_clientdata(client);
	gtp_irq_disable();
}

static void gtptest_irq_enable(struct i2c_client * client)
{
    //mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
    //struct goodix_ts_data *ts;
	//ts = i2c_get_clientdata(client);
	gtp_irq_enable();
}

/*
 * Initialize cfg_drv_order and cfg_sen_order, which is used for report short channels
 *
 */

s32 gt9xx_short_parse_cfg(void)
{
    u8 i = 0;
    u8 drv_num = 0, sen_num = 0;

    u8 config[256] = {(u8)(GTP_REG_CONFIG_DATA >> 8), (u8)GTP_REG_CONFIG_DATA, 0};
	
	//struct goodix_ts_data *ts = i2c_get_clientdata(i2c_client_point);

    if (gtp_i2c_read(i2c_client_point, config, GTP_CONFIG_MAX_LENGTH + GTP_ADDR_LENGTH) <= 0)
    {
        SET_INFO_LINE_ERR("Failed to read config!");
        return FAIL;
    }

    drv_num = (config[GTP_ADDR_LENGTH + GT9_REG_SEN_DRV_CNT-GT9_REG_CFG_BEG] & 0x1F)
                        + (config[GTP_ADDR_LENGTH + GT9_REG_SEN_DRV_CNT+1 -GT9_REG_CFG_BEG] & 0x1F);
    sen_num = (config[GTP_ADDR_LENGTH + GT9_REG_SEN_DRV_CNT+2-GT9_REG_CFG_BEG] & 0x0F)
                        + ((config[GTP_ADDR_LENGTH + GT9_REG_SEN_DRV_CNT+2-GT9_REG_CFG_BEG]>>4) & 0x0F);

    if (drv_num < MIN_DRIVER_NUM || drv_num > MAX_DRIVER_NUM)
    {
        GTP_ERROR("driver number error!");
        return FAIL;
    }
    if (sen_num < MIN_SENSOR_NUM || sen_num > MAX_SENSOR_NUM)
    {
        GTP_ERROR("sensor number error!");
        return FAIL;
    }
    // get sensor and driver order
    memset(cfg_sen_order, 0xFF, MAX_SENSOR_NUM);
    for (i = 0; i < sen_num; ++i)
    {
        cfg_sen_order[i] = config[GTP_ADDR_LENGTH + GT9_REG_SEN_ORD - GT9_REG_CFG_BEG + i];
    }

    memset(cfg_drv_order, 0xFF, MAX_DRIVER_NUM);
    for (i = 0; i < drv_num; ++i)
    {
        cfg_drv_order[i] = config[GTP_ADDR_LENGTH + GT9_REG_DRV_ORD - GT9_REG_CFG_BEG + i];
    }

    return SUCCESS;
}

/*
 * @param:
 *      phy_chnl: ic detected short channel, is_driver: it's driver or not
 * @Return:
 *      0xff: the ic channel is not used, otherwise: the tp short channel
 */
u8 gt9_get_short_tp_chnl(u8 phy_chnl, u8 is_driver)
{
    u8 i = 0;
    if (is_driver) {
        for (i = 0; i < MAX_DRIVER_NUM; ++i)
        {
            if (cfg_drv_order[i] == phy_chnl) {
                return i;
            }
            else if (cfg_drv_order[i] == 0xFF) {
                return 0xFF;
            }
        }
    }
    else
    {
        for (i = 0; i < MAX_SENSOR_NUM; ++i)
        {
            if (cfg_sen_order[i] == phy_chnl) {
                return i;
            }
            else if (cfg_sen_order[i] == 0xFF) {
                return 0xFF;
            }
        }
    }
    return 0xFF;
}


static s32 gtp_i2c_end_cmd(struct i2c_client *client)
{
    u8  end_cmd[3] = {GTP_READ_COOR_ADDR >> 8, GTP_READ_COOR_ADDR & 0xFF, 0};
    s32 ret = 0;

    ret = gtp_i2c_write(client, end_cmd, 3);
    if (ret < 0)
    {
        SET_INFO_LINE_INFO("I2C write end_cmd  error!");
    }
    return ret;
}
/*
static void gtp_hopping_switch(struct i2c_client *client, s32 on)
{
    s32 j = 0;
    static u8 hopping_enabled = 0;
    u8 chksum = 0;
    u8 config[256] = {(u8)(GTP_REG_CONFIG_DATA >> 8), (u8)GTP_REG_CONFIG_DATA, 0};

    if (gtp_i2c_read(client, config, GTP_CONFIG_MAX_LENGTH + GTP_ADDR_LENGTH) < 0)
    {
        SET_INFO_LINE_ERR("Failed to read config!");
        return;
    }
    if (!on)
    {
        // disable hopping
        if (config[GTP_ADDR_LENGTH + 0x807D - GTP_REG_CONFIG_DATA] & 0x80)
        {
            GTP_INFO("Disable hopping.");
            config[GTP_ADDR_LENGTH + 0x807D - GTP_REG_CONFIG_DATA] &= 0x7F;
            // calculate checksum
            for (j = 0; j < (cfg_len-2); ++j)
            {
                chksum += config[GTP_ADDR_LENGTH + j];
            }
            config[cfg_len] = (~chksum) + 1;
            config[cfg_len+1] = 0x01;

            gtp_i2c_write(client, config, GTP_CONFIG_MAX_LENGTH + GTP_ADDR_LENGTH);
            hopping_enabled = 1;
        }
    }
    else if (hopping_enabled)
    {
        if (0x00 == (config[GTP_ADDR_LENGTH + 0x807D - GTP_REG_CONFIG_DATA] & 0x80))
        {
            GTP_INFO("Enable hopping.");
            config[GTP_ADDR_LENGTH + 0x807D - GTP_REG_CONFIG_DATA] |= 0x80;
            // calculate checksum
            for (j = 0; j < (cfg_len-2); ++j)
            {
                chksum += config[GTP_ADDR_LENGTH + j];
            }
            config[cfg_len] = (~chksum) + 1;
            config[cfg_len+1] = 0x01;

            gtp_i2c_write(client, config, GTP_CONFIG_MAX_LENGTH + GTP_ADDR_LENGTH);
            hopping_enabled = 0;
        }
    }
}
*/
static void gtp_open_test_init(struct i2c_client *client)
{
	u8 sensor_id = 0;
	u8 tmp[4] = {(u8)(GTP_REG_SENSOR_ID >> 8), (u8)GTP_REG_SENSOR_ID, 0};
	u8 config[256] = {(u8)(GTP_REG_CONFIG_DATA >> 8), (u8)GTP_REG_CONFIG_DATA, 0};
	u8 test_config[GTP_CONFIG_MAX_LENGTH + GTP_ADDR_LENGTH] = {
	    (u8)(GTP_REG_CONFIG_DATA >> 8), (u8)GTP_REG_CONFIG_DATA, 0};

			if (gtp_i2c_read(i2c_client_point, config, GTP_CONFIG_MAX_LENGTH + GTP_ADDR_LENGTH) <= 0)
    	{
        	SET_INFO_LINE_ERR("Failed to read config!");
        	return ;
    	}

    gt9xx_drv_num = (config[GTP_ADDR_LENGTH + GT9_REG_SEN_DRV_CNT-GT9_REG_CFG_BEG] & 0x1F)
                    + (config[GTP_ADDR_LENGTH + GT9_REG_SEN_DRV_CNT+1 -GT9_REG_CFG_BEG] & 0x1F);
    gt9xx_sen_num = (config[GTP_ADDR_LENGTH + GT9_REG_SEN_DRV_CNT+2-GT9_REG_CFG_BEG] & 0x0F)
                    + ((config[GTP_ADDR_LENGTH + GT9_REG_SEN_DRV_CNT+2-GT9_REG_CFG_BEG]>>4) & 0x0F);

    GTP_INFO("Driver num: %d, Sensor Num: %d", gt9xx_drv_num, gt9xx_sen_num);

    gt9xx_pixel_cnt = gt9xx_drv_num * gt9xx_sen_num;


	if (gtp_i2c_read(client, tmp, 1 + GTP_ADDR_LENGTH) <= 0)
	{
		SET_INFO_LINE_ERR("Failed to read sensor id!");
		return;
	}
	else
	{
		sensor_id = tmp[GTP_ADDR_LENGTH];
         if (sensor_id >= 0x06)
         {
                GTP_ERROR("Invalid sensor_id(0x%02X), No Config Sent!", sensor_id);
                return;
         }
		SET_INFO_LINE_INFO("sensor_id: %d\n", sensor_id);
	}

#if 1
	if (tset_cfg_info_len[sensor_id]!= 0)
	{
		max_limit_value = max_limit_value_info[sensor_id];     // screen max limit
		min_limit_value = min_limit_value_info[sensor_id];     // screen min limit
		max_limit_key = max_limit_key_info[sensor_id];       // key_val max limit
		min_limit_key = min_limit_key_info[sensor_id];        // key_val min limit
		uniformity_lmt = uniformity_lmt_info[sensor_id];        // screen uniformity in percent

		memset(&test_config[GTP_ADDR_LENGTH], 0, GTP_CONFIG_MAX_LENGTH);
    		memcpy(&test_config[GTP_ADDR_LENGTH], send_test_cfg_buf[sensor_id], tset_cfg_info_len[sensor_id]);

		SET_INFO_LINE_INFO("max_limit_value: %d\n", max_limit_value);
		SET_INFO_LINE_INFO("min_limit_value: %d\n", min_limit_value);
		SET_INFO_LINE_INFO("max_limit_key: %d\n", max_limit_key);
		SET_INFO_LINE_INFO("min_limit_key: %d\n", min_limit_key);
		SET_INFO_LINE_INFO("uniformity_lmt %d\n", uniformity_lmt);

		gtp_i2c_write(client, test_config, tset_cfg_info_len[sensor_id] + GTP_ADDR_LENGTH);
	}
#endif
    GTP_DEBUG("gtp_open_test_init-------sensor_id=%d, gt9xx_pixel_cnt=%d\n",sensor_id,gt9xx_pixel_cnt); 
	switch (sensor_id)	
		{	
           GTP_DEBUG("sensor_id=%d, gt9xx_pixel_cnt=%d\n",sensor_id,gt9xx_pixel_cnt); 
			case 0:
				GTP_ERROR("GTP:Use sensor_id 0 standard");			
				memcpy(&max_limit_vale_re[0], &max_limit_vale_id0[0], (2 * gt9xx_pixel_cnt));
				memcpy(&min_limit_vale_re[0], &min_limit_vale_id0[0], (2 * gt9xx_pixel_cnt));
				memcpy(&accord_limit_vale_re[0], &accord_limit_vale_id0[0], (2 * gt9xx_pixel_cnt));		
				break;
				
			case 1:
				GTP_ERROR("GTP:Use sensor_id 1 standard"); 
				memcpy(&max_limit_vale_re[0], &max_limit_vale_id1[0], (2 * gt9xx_pixel_cnt));
				memcpy(&min_limit_vale_re[0], &min_limit_vale_id1[0], (2 * gt9xx_pixel_cnt));
				memcpy(&accord_limit_vale_re[0], &accord_limit_vale_id1[0], (2 * gt9xx_pixel_cnt));				
				break;
				
			case 2:
				GTP_ERROR("GTP:Use sensor_id 2 standard"); 
				memcpy(&max_limit_vale_re[0], &max_limit_vale_id2[0], (2 * gt9xx_pixel_cnt));
				memcpy(&min_limit_vale_re[0], &min_limit_vale_id2[0], (2 * gt9xx_pixel_cnt));
				memcpy(&accord_limit_vale_re[0], &accord_limit_vale_id2[0], (2 * gt9xx_pixel_cnt));	
				break;
				
			case 3:
				GTP_ERROR("GTP:Use sensor_id 3 standard"); 
				memcpy(&max_limit_vale_re[0], &max_limit_vale_id3[0], (2 * gt9xx_pixel_cnt));
				memcpy(&min_limit_vale_re[0], &min_limit_vale_id3[0], (2 * gt9xx_pixel_cnt));
				memcpy(&accord_limit_vale_re[0], &accord_limit_vale_id3[0], (2 * gt9xx_pixel_cnt));				
				break;
			case 4:
				GTP_ERROR("GTP:Use sensor_id 4 standard"); 
				memcpy(&max_limit_vale_re[0], &max_limit_vale_id4[0], (2 * gt9xx_pixel_cnt));
				memcpy(&min_limit_vale_re[0], &min_limit_vale_id4[0], (2 * gt9xx_pixel_cnt));
				memcpy(&accord_limit_vale_re[0], &accord_limit_vale_id4[0], (2 * gt9xx_pixel_cnt));				
				break;
			case 5:
				GTP_ERROR("GTP:Use sensor_id 5 standard"); 
				memcpy(&max_limit_vale_re[0], &max_limit_vale_id5[0], (2 * gt9xx_pixel_cnt));
				memcpy(&min_limit_vale_re[0], &min_limit_vale_id5[0], (2 * gt9xx_pixel_cnt));
				memcpy(&accord_limit_vale_re[0], &accord_limit_vale_id5[0], (2 * gt9xx_pixel_cnt));				
				break;
				
			default:		
				GTP_ERROR("GTP:Unrecognized sensor_id,keep last!");
				//memcpy(&max_limit_vale_re[0], &max_limit_vale_re22[0], 336);
				//memcpy(&min_limit_vale_re[0], &min_limit_vale_re22[0], 336);
				//memcpy(&accord_limit_vale_re[0], &accord_limit_vale_re22[0], 336);				
				break;
		}
}



s32 gtp_parse_config(struct i2c_client *client)
{
    u8 i = 0;
    u8 key_pos = 0;
    u8 key_val = 0;
    u8 config[256] = {(u8)(GTP_REG_CONFIG_DATA >> 8), (u8)GTP_REG_CONFIG_DATA, 0};
    u8 type_buf[12] = {0x80, 0x00};

    if (gtp_i2c_read(client, config, GTP_CONFIG_MAX_LENGTH + GTP_ADDR_LENGTH) <= 0)
    {
        SET_INFO_LINE_ERR("Failed to read config!");
        return FAIL;
    }

    gt9xx_drv_num = (config[GTP_ADDR_LENGTH + GT9_REG_SEN_DRV_CNT-GT9_REG_CFG_BEG] & 0x1F)
                    + (config[GTP_ADDR_LENGTH + GT9_REG_SEN_DRV_CNT+1 -GT9_REG_CFG_BEG] & 0x1F);
    gt9xx_sen_num = (config[GTP_ADDR_LENGTH + GT9_REG_SEN_DRV_CNT+2-GT9_REG_CFG_BEG] & 0x0F)
                    + ((config[GTP_ADDR_LENGTH + GT9_REG_SEN_DRV_CNT+2-GT9_REG_CFG_BEG]>>4) & 0x0F);

    GTP_INFO("Driver num: %d, Sensor Num: %d", gt9xx_drv_num, gt9xx_sen_num);
    if (gt9xx_drv_num < MIN_DRIVER_NUM || gt9xx_drv_num > MAX_DRIVER_NUM)
    {
        SET_INFO_LINE_ERR("driver number error!");
        return FAIL;
    }
    if (gt9xx_sen_num < MIN_SENSOR_NUM || gt9xx_sen_num > MAX_SENSOR_NUM)
    {
        SET_INFO_LINE_ERR("sensor number error!");
        return FAIL;
    }
    gt9xx_sc_pxl_cnt = gt9xx_pixel_cnt = gt9xx_drv_num * gt9xx_sen_num;

    gtp_i2c_read(client, type_buf, 12);
    if (!memcmp(&type_buf[2], "GOODIX_GT9", 10))
    {
        chip_type_gt9f = 0;
        GTP_INFO("Chip type: GT9XX");
    }
    else
    {
        chip_type_gt9f = 1;
        GTP_INFO("Chip type: GT9XXF");
    }

    have_key = config[0x804E - GT9_REG_CFG_BEG + GTP_ADDR_LENGTH] & 0x01;

    if (!have_key)
    {
        GTP_INFO("No key");
        return SUCCESS;
    }
    GTP_INFO("Have Key");
    gt9xx_sc_drv_num = gt9xx_drv_num - 1;

    for (i = 0; i < 5; ++i)
    {
        key_iso_pos[i] = 0;
    }

    key_is_isolated = 0;
    for (i = 0; i < 4; ++i)
    {
        // all keys are multiples of 0x08 -> isolated keys
        key_val = config[GTP_ADDR_LENGTH + GT9_REG_KEY_VAL - GT9_REG_CFG_BEG + i];
        key_pos = key_val%0x08;
        GTP_DEBUG("key_val[%d] = 0x%02x", i+1, key_val);
        if ((key_pos != 0))
        {
            key_is_isolated = 0;
            GTP_DEBUG("Key is not isolated!");
            break;
        }
        else if (key_val == 0x00)       // no more key
        {
            continue;
        }
        else
        {
            key_iso_pos[0]++;       // isolated key count
            key_iso_pos[i+1] = key_val/0x08 - 1;
            key_is_isolated = 1;
        }
    }

    gt9xx_sc_pxl_cnt = gt9xx_pixel_cnt - (gt9xx_drv_num-gt9xx_sc_drv_num) * gt9xx_sen_num;
    GTP_DEBUG("drv num: %d, sen num: %d, sc drv num: %d", gt9xx_drv_num, gt9xx_sen_num, gt9xx_sc_drv_num);
    if (key_is_isolated)
    {
        GTP_DEBUG("Isolated [%d key(s)]: %d, %d, %d, %d", key_iso_pos[0], key_iso_pos[1], key_iso_pos[2], key_iso_pos[3], key_iso_pos[4]);
    }

    return SUCCESS;
}

/*
 * Function:
 *      write one byte to specified register
 * Input:
 *      reg: the register address
 *      val: the value to write into
 * Return:
 *      i2c_write function return
 */
s32 gtp_write_register(struct i2c_client * client, u16 reg, u8 val)
{
    s32 ret = 0;
    u8 buf[3];
    buf[0] = (u8) (reg >> 8);
    buf[1] = (u8) reg;
    buf[2] = val;
    //return gtp_i2c_write(client, buf, 3);
    ret = i2c_write_bytes(client, reg, &buf[2], 1);

    if (ret < 0)
    {
        return ret;
    }
    else
    {
        return 1;
    }
}
/*
 * Function:
 *      read one byte from specified register into buf
 * Input:
 *      reg: the register
 *      buf: the buffer for one byte
 * Return:
 *      i2c_read function return
 */
s32 gtp_read_register(struct i2c_client * client, u16 reg, u8* buf)
{
    s32 ret = 0;
    buf[0] = (u8)(reg >> 8);
    buf[1] = (u8)reg;
    //return gtp_i2c_read(client, buf, 3);

    ret = i2c_read_bytes(client, reg, &buf[2], 1);

    if (ret < 0)
    {
        return ret;
    }
    else
    {
        return 2;
    }
}

/*
 * Function:
 *      burn dsp_short code
 * Input:
 *      i2c_client
 * Return:
 *      SUCCESS: burning succeed, FAIL: burning failed
 */
s32 gtp_burn_dsp_short(struct i2c_client *client)
{
    s32 ret = 0;
    u8 *opr_buf;
    u16 i = 0;
    u16 addr = GTP_REG_DSP_SHORT;
    u16 opr_len = 0;
    u16 left = 0;
    u16 retry = 0;
    u8 read_buf[3] = {0x00};

    GTP_DEBUG("Start writing dsp_short code");
    opr_buf = (u8*)kmalloc(sizeof(u8) * (DSP_SHORT_BURN_CHK+2), GFP_KERNEL);
    if (!opr_buf)
    {
        SET_INFO_LINE_ERR("failed to allocate memory for check buffer!");
        return FAIL;
    }

    left = sizeof(dsp_short);
    while (left > 0)
    {
        opr_buf[0] = (u8)(addr >> 8);
        opr_buf[1] = (u8)(addr);
        if (left > DSP_SHORT_BURN_CHK)
        {
            opr_len = DSP_SHORT_BURN_CHK;
        }
        else
        {
            opr_len = left;
        }
        memcpy(&opr_buf[2], &dsp_short[addr-GTP_REG_DSP_SHORT], opr_len);

        ret = gtp_i2c_write(client, opr_buf, 2 + opr_len);
        if ( ret < 0 )
        {
            GTP_ERROR("write dsp_short code failed!");
            kfree(opr_buf);
            return FAIL;
        }
        addr += opr_len;
        left -= opr_len;
    }

    // check code: 0xC000~0xCFFF
    GTP_DEBUG("Start checking dsp_short code");
    addr = GTP_REG_DSP_SHORT;
    left = sizeof(dsp_short);
    while (left > 0)
    {
        memset(opr_buf, 0, opr_len + 2);
        opr_buf[0] = (u8)(addr >> 8);
        opr_buf[1] = (u8)(addr);


        if (left > DSP_SHORT_BURN_CHK)
        {
            opr_len = DSP_SHORT_BURN_CHK;
        }
        else
        {
            opr_len = left;
        }

        ret = gtp_i2c_read(client, opr_buf, opr_len+2);
        if (ret < 0)
        {
            kfree(opr_buf);
            return FAIL;
        }
        for (i = 0; i < opr_len; ++i)
        {
            if (opr_buf[i+2] != dsp_short[addr-GTP_REG_DSP_SHORT+i])
            {
                GTP_ERROR("check dsp_short code failed!");

                gtp_write_register(client, addr + i, dsp_short[addr-GTP_REG_DSP_SHORT+i]);

                GTP_DEBUG("(%d)Location: %d, 0x%02X, 0x%02X", retry+1, addr - GTP_REG_DSP_SHORT + i, opr_buf[i+2], dsp_short[addr-GTP_REG_DSP_SHORT+i]);

                msleep(1);
                gtp_read_register(client, addr + i, read_buf);
                opr_buf[i+2] = read_buf[2];
                i--;
                retry++;
                if (retry >= 200)
                {
                    GTP_DEBUG("Burn dsp retry timeout!");
                    kfree(opr_buf);
                    return FAIL;
                }
            }
        }

        addr += opr_len;
        left -= opr_len;
    }
    kfree(opr_buf);
    return SUCCESS;
}
/*
 * Function:
 *      check the resistor between shortlike channels if less than threshold confirm as short
 * INPUT:
 *      Short like Information struct pointer
 * Returns:
 *      SUCCESS: it's shorted FAIL: otherwise
 */
s32 gtp_short_resist_check(struct gt9xx_short_info *short_node)
{
    s32 short_resist = 0;
    struct gt9xx_short_info *node = short_node;
    u8 master = node->master;
    u8 slave = node->slave;
    u8 chnnl_tx[4] = { GT9_DRV_HEAD|13, GT9_DRV_HEAD|28,
                    GT9_DRV_HEAD|29, GT9_DRV_HEAD|42 };
    s32 numberator = 0;
    u32 amplifier = 1000;  // amplify 1000 times to emulate float computing


    // Tx-ABIST & Tx_ABIST
    if ((((master > chnnl_tx[0]) && (master <= chnnl_tx[1])) &&
        ((slave > chnnl_tx[0]) && (slave <= chnnl_tx[1])) ) ||
        (((master >= chnnl_tx[2]) && (master <= chnnl_tx[3])) &&
        ((slave >= chnnl_tx[2]) && (slave <= chnnl_tx[3]))))
    {
        numberator = node->self_data * 40 * amplifier;
        short_resist = numberator/(node->short_code) - 40 * amplifier;
    }
    // Receiver is Rx-odd(1,3,5)
    else if ((node->slave & (GT9_DRV_HEAD | 0x01)) == 0x01)
    {
        numberator = node->self_data * 60 * amplifier;
        short_resist = numberator/node->short_code - 40 * amplifier;
    }
    else
    {
        numberator = node->self_data * 60 * amplifier;
        short_resist = numberator / node->short_code - 60 * amplifier;
    }
    GTP_DEBUG("self_data = %d" ,node->self_data);
    GTP_DEBUG("master = 0x%02X, slave = 0x%02X", node->master, node->slave);
    GTP_DEBUG("short_code = %d, short_resist = %d", node->short_code, short_resist);

    if (short_resist < 0)
    {
        short_resist = 0;
    }

    if (short_resist < (gt900_resistor_threshold * amplifier))
    {
        node->impedance = short_resist / amplifier;
        return SUCCESS;
    }
    else
    {
        return FAIL;
    }
}



/*
 * Function:
 *      compute the result, whether there are shorts or not
 * Input:
 *      i2c_client
 * Return:
 *      SUCCESS
 */
s32 gtp_compute_rslt(struct i2c_client *client)
{
    u16 short_code;
    u8 i = 0, j = 0;
    u16 result_addr;
    u8 *result_buf;
    u16 *self_data;
    s32 ret = 0;
    u16 data_len = 3 + (MAX_DRIVER_NUM + MAX_SENSOR_NUM) * 2 + 2; // a short data frame length
    struct gt9xx_short_info short_node;
    u16 node_idx = 0; // short_sum index: 0~_SHORT_INFO_MAX

    u8 tx_short_num = 0;
    u8 rx_short_num = 0;

    u8 master, slave;

    self_data = (u16*)kmalloc(sizeof(u16) * ((MAX_DRIVER_NUM + MAX_SENSOR_NUM)), GFP_KERNEL);
    result_buf = (u8*)kmalloc(sizeof(u8) * (data_len+2), GFP_KERNEL);
    short_sum = (struct gt9xx_short_info *) kmalloc(sizeof(struct gt9xx_short_info) * _SHORT_INFO_MAX, GFP_KERNEL);

    if (!self_data || !result_buf || !short_sum)
    {
        SET_INFO_LINE_ERR("allocate memory for short result failed!");
        if (self_data)
        {
            kfree(self_data);
        }
        if (result_buf)
        {
            kfree(self_data);
        }
        if (short_sum)
        {
            kfree(short_sum);
        }
        return FAIL;
    }

    // Get Selfdata
    result_buf[0] = 0xA4;
    result_buf[1] = 0xA1;
    gtp_i2c_read(client, result_buf, 2 + 144);
    for (i = 0, j = 0; i < 144; i += 2)
    {
        self_data[j++] = (u16)(result_buf[2+i] << 8) + (u16)(result_buf[2+i+1]);
    }
    GTP_DEBUG("Self Data:");
    GTP_DEBUG_ARRAY(result_buf+2, 144);


    // Get TxShortNum & RxShortNum
    result_buf[0] = 0x88;
    result_buf[1] = 0x02;
    gtp_i2c_read(client, result_buf, 2 + 2);
    tx_short_num = result_buf[2];
    rx_short_num = result_buf[3];

    GTP_DEBUG("Tx Short Num: %d, Rx Short Num: %d", tx_short_num, rx_short_num);

    //
    result_addr = 0x8860;
    data_len = 3 + (MAX_DRIVER_NUM + MAX_SENSOR_NUM) * 2 + 2;
    for (i = 0; i < tx_short_num; ++i)
    {
        result_buf[0] = (u8) (result_addr >> 8);
        result_buf[1] = (u8) (result_addr);
        ret = gtp_i2c_read(client, result_buf, data_len+2);
        if (ret < 0)
        {
            SET_INFO_LINE_ERR("read result data failed!");
        }
        GTP_DEBUG("Result Buffer: ");
        GTP_DEBUG_ARRAY(result_buf+2, data_len);

        short_node.master_is_driver = 1;
        short_node.master = result_buf[2];

        // Tx - Tx
        for (j = i + 1; j < MAX_DRIVER_NUM; ++j)
        {
            short_code = (result_buf[2+3+j*2] << 8) + result_buf[2+3+j*2+1];
            if (short_code > gt900_short_threshold)
            {
                short_node.slave_is_driver = 1;
                short_node.slave = ChannelPackage_TX[j] | GT9_DRV_HEAD;
                short_node.self_data = self_data[j];
                short_node.short_code = short_code;

                ret = gtp_short_resist_check(&short_node);
                if (ret == SUCCESS)
                {
                    if (node_idx < _SHORT_INFO_MAX)
                    {
                        short_sum[node_idx++] = short_node;
                    }
                }
            }
        }
        // Tx - Rx
        for (j = 0; j < MAX_SENSOR_NUM; ++j)
        {
            short_code = (result_buf[2+3+84+j*2] << 8) + result_buf[2+3+84+j*2+1];

            if (short_code > gt900_short_threshold)
            {
                short_node.slave_is_driver = 0;
                short_node.slave = j | GT9_SEN_HEAD;
                short_node.self_data = self_data[MAX_DRIVER_NUM + j];
                short_node.short_code = short_code;

                ret = gtp_short_resist_check(&short_node);
                if (ret == SUCCESS)
                {
                    if (node_idx < _SHORT_INFO_MAX)
                    {
                        short_sum[node_idx++] = short_node;
                    }
                }
            }
        }

        result_addr += data_len;
    }

    result_addr = 0xA0D2;
    data_len = 3 + MAX_SENSOR_NUM * 2 + 2;
    for (i = 0; i < rx_short_num; ++i)
    {
        result_buf[0] = (u8) (result_addr >> 8);
        result_buf[1] = (u8) (result_addr);
        ret = gtp_i2c_read(client, result_buf, data_len + 2);
        if (ret < 0)
        {
            SET_INFO_LINE_ERR("read result data failed!");
        }

        GTP_DEBUG("Result Buffer: ");
        GTP_DEBUG_ARRAY(result_buf+2, data_len);

        short_node.master_is_driver = 0;
        short_node.master = result_buf[2];

        // Rx - Rx
        for (j = 0; j < MAX_SENSOR_NUM; ++j)
        {
            if ((j == i) || ( (j < i) && (j & 0x01) == 0))
            {
                continue;
            }
            short_code = (result_buf[2+3+j*2] << 8) + result_buf[2+3+j*2+1];

            if (short_code > gt900_short_threshold)
            {
                short_node.slave_is_driver = 0;
                short_node.slave = j | GT9_SEN_HEAD;
                short_node.self_data = self_data[MAX_DRIVER_NUM + j];
                short_node.short_code = short_code;

                ret = gtp_short_resist_check(&short_node);
                if (ret == SUCCESS)
                {
                    if (node_idx < _SHORT_INFO_MAX)
                    {
                        short_sum[node_idx++] = short_node;
                    }
                }
            }
        }

        result_addr += data_len;
    }

    if (node_idx == 0)
    {
        ret = SUCCESS;
    }
    else
    {
        for (i = 0, j = 0; i < node_idx; ++i)
        {
            GTP_DEBUG("Orignal Shorted Channels: %s%d, %s%d",
                (short_sum[i].master_is_driver) ? "Drv" : "Sen", short_sum[i].master & (~GT9_DRV_HEAD),
                (short_sum[i].slave_is_driver) ? "Drv" : "Sen", short_sum[i].slave & (~GT9_DRV_HEAD));

            if ((short_sum[i].master_is_driver))
            {
                master = gt9_get_short_tp_chnl(short_sum[i].master-GT9_DRV_HEAD, 1);
            }
            else
            {
                master = gt9_get_short_tp_chnl(short_sum[i].master, 0);
            }

            if ((short_sum[i].slave_is_driver))
            {
                slave = gt9_get_short_tp_chnl(short_sum[i].slave-GT9_DRV_HEAD, 1);
            }
            else
            {
                slave = gt9_get_short_tp_chnl(short_sum[i].slave, 0);
            }

            if (master == 0xFF && slave == 0xFF)
            {
                GTP_DEBUG("unbonded channel (%d, %d) shorted!", short_sum[i].master, short_sum[i].slave);
                continue;
            }
            else
            {
                short_sum[j].slave = slave;
                short_sum[j].master = master;
                short_sum[j].slave_is_driver = short_sum[i].slave_is_driver;
                short_sum[j].master_is_driver = short_sum[i].master_is_driver;
                short_sum[j].impedance = short_sum[i].impedance;
                short_sum[j].self_data = short_sum[i].self_data;
                short_sum[j].short_code = short_sum[i].short_code;
                ++j;
            }
        }
        node_idx = j;
        if (node_idx == 0)
        {
            ret = SUCCESS;
        }
        else
        {
            for (i = 0; i < node_idx; ++i)
            {
                SET_INFO_LINE_INFO("  %s%02d & %s%02d Shorted! (R = %dKOhm)",
                (short_sum[i].master_is_driver) ? "Drv" : "Sen", short_sum[i].master,
                (short_sum[i].slave_is_driver) ? "Drv" : "Sen", short_sum[i].slave,
                short_sum[i].impedance);
            }
            ret = FAIL;
        }
    }
    kfree(self_data);
    kfree(short_sum);
    kfree(result_buf);
    return ret;
}

s32 gt9_test_gnd_vdd_short(struct i2c_client *client)
{
    u8 *data;
    s32 ret = 0;
    s32 i = 0;
    u16 len = (MAX_DRIVER_NUM + MAX_SENSOR_NUM) * 2;
    u16 short_code = 0;
    s32 r = -1;
    u32 short_res = 0;
    u8 short_chnl = 0;
    u16 amplifier = 1000;

    data = (u8 *)kmalloc(sizeof(u8) * (len + 2), GFP_KERNEL);
    if (NULL == data)
    {
       SET_INFO_LINE_ERR("failed to allocate memory for gnd vdd test data buffer");
       return FAIL;
    }

    data[0] = 0xA5;
    data[1] = 0x31;
    gtp_i2c_read(client, data, 2 + len);

    GTP_DEBUG_ARRAY(data+2, len);
    ret = SUCCESS;
    for (i = 0; i < len; i += 2)
    {
        short_code = (data[2+i] << 8) + (data[2 + i + 1]);
        if (short_code == 0)
        {
            continue;
        }
        if ((short_code & 0x8000) == 0)        // short with GND
        {
        #ifdef GTP_SHORT_GND
            r = 5266285 * 10 / (short_code & (~0x8000)) - 40 * amplifier;
        #endif
        }
        else    // short with VDD
        {
            //r = ( 1/(((float)(short_code&(~0x8000)))/0.9*0.7/1024/(sys.avdd-0.9)/40) ) -40;
        #ifdef GTP_VDD
            r = 40*9*1024*(100*GTP_VDD - 900)/((short_code&(~0x8000))*7) - 40*1000;
            GTP_DEBUG("vdd short_code: %d", short_code & (~0x8000));
        #endif
        }
        GTP_DEBUG("resistor: %d, short_code: %d", r, short_code);
        
        short_res = (r >= 0) ? r : 0xFFFF;
        if (short_res == 0xFFFF)
        {
        }
        else
        {
            if (short_res < (gt900_gnd_resistor_threshold * amplifier))
            {
                if (i < MAX_DRIVER_NUM * 2)       // driver
                {
                    short_chnl = gt9_get_short_tp_chnl(ChannelPackage_TX[i/2], 1);
                    GTP_INFO("driver%02d & gnd/vdd shorted!", short_chnl);
                    if (short_chnl == 0xFF)
                    {
                        GTP_INFO("unbonded channel");
                    }
                    else
                    {
                        SET_INFO_LINE_INFO("  Drv%02d & GND/VDD Shorted! (R = %dKOhm)", short_chnl, short_res/amplifier);
                    }
                }
                else
                {
                    short_chnl = gt9_get_short_tp_chnl((i/2) - MAX_DRIVER_NUM, 0);
                    GTP_INFO("sensor%02d & gnd/vdd shorted!", short_chnl);
                    if (short_chnl == 0xFF)
                    {
                        GTP_INFO("unbonded channel");
                    }
                    else
                    {
                        SET_INFO_LINE_INFO("  Sen%02d & GND/VDD Shorted! (R = %dKOhm)", short_chnl, short_res/amplifier);
                    }
                }
                ret = FAIL;
            }
        }
    }
    return ret;
}


/*
 * leave short test
 */
void gt9xx_leave_short_test(struct i2c_client *client)
{

    gtp_reset_guitar(client, 60);
    msleep(100);
    printk("---gtp short test out reset---");
    //gtp_send_cfg(client);
    SET_INFO_LINE_INFO("");
    SET_INFO_LINE_INFO("---gtp short test end---");
}


/*
 * Function:
 *      gt9 series ic short test function
 * Input:
 *      I2c_client, i2c device
 * Return:
 *      SUCCESS: test succeed, FAIL: test failed
 */
s32 gt9xx_short_test(struct i2c_client * client)
{
    s32 ret = 0;
    s32 ret2 = 0;
    u8 i = 0;
    u8 opr_buf[60] = {0};
    u8 retry = 0;
    u8 drv_sen_chksum = 0;
    u8 retry_load = 0;
    u8 old_i2c_addr = client->addr;
   // struct goodix_ts_data *ts = i2c_get_clientdata(client);
/*
#ifdef CONFIG_GTP_ESD_PROTECT
	struct goodix_ts_data *ts = i2c_get_clientdata(client);
    //gtptest_irq_disable(client);
    //disable_irq(client->irq);
    ts->gtp_is_suspend = 1;     // suspend esd
#endif
*/
    // step 1: reset guitar, delay 1ms,  hang up ss51 and dsp
    SET_INFO_LINE_INFO("---gtp short test---");
    SET_INFO_LINE_INFO("Step 1: reset guitar, hang up ss51 dsp");
    
    ret = gt9xx_short_parse_cfg();
    if (FAIL == ret)
    {
        SET_INFO_LINE_ERR("You May check your IIC connection.");
        goto short_test_exit;
    }

load_dsp_again:
    // RST output low last at least 2ms

gtp_gpio_output(GTP_RST_GPIO, 0);
  //  GTP_GPIO_OUTPUT(GTP_RST_PORT, 0);
    msleep(30);

    // select I2C slave addr,INT:0--0xBA;1--0x28.
    //GTP_GPIO_OUTPUT(GTP_INT_PORT, client->addr == 0x14);
    /*if (client->addr == 0x14) {
		pinctrl_select_state(ts->ts_pinctrl, ts->eint_output_high);
	} else {
		pinctrl_select_state(ts->ts_pinctrl, ts->eint_output_low);
	}*/
	gtp_gpio_output(GTP_IRQ_GPIO, client->addr == 0x14);
    msleep(2);

    // RST output high reset guitar
   // GTP_GPIO_OUTPUT(GTP_RST_PORT, 1);
   gtp_gpio_output(GTP_RST_GPIO, 1);
    msleep(6);

    while(retry++ < 200)
    {
        // Hold ss51 & dsp
        ret = gtp_write_register(client, _rRW_MISCTL__SWRST_B0_, 0x0C);
        if( (ret <= 0))
        {
            GTP_DEBUG("Hold ss51 & dsp I2C error,retry:%d", retry);
            gtp_reset_guitar(client, 10);
            continue;
        }
        //GTP_DEBUG("Hold ss51 & dsp confirm 0x4180 failed,value:%d", opr_buf[GTP_ADDR_LENGTH]);
        msleep(2);

        //confirm hold
        opr_buf[GTP_ADDR_LENGTH] = 0x00;

        ret = gtp_read_register(client, _rRW_MISCTL__SWRST_B0_, opr_buf);
        if(ret <= 0)
        {
            GTP_DEBUG("Hold ss51 & dsp I2C error,retry:%d", retry);
            gtp_reset_guitar(client, 10);
            continue;
        }
        if(0x0C == opr_buf[GTP_ADDR_LENGTH])
        {
            GTP_DEBUG("Hold ss51 & dsp confirm SUCCESS");
            break;
        }
    }
    if(retry >= 200)
    {
        SET_INFO_LINE_ERR("Enter update Hold ss51 failed.");
        goto short_test_exit;
    }
    /* DSP_CK and DSP_ALU_CK PowerOn */
    ret2 = gtp_write_register(client, 0x4010, 0x00);
    if (ret2 <= 0)
    {
	SET_INFO_LINE_ERR("Enter update PowerOn DSP failed.");
	goto short_test_exit;
    }
    
    // step2: burn dsp_short code
    GTP_INFO("step 2: burn dsp_short code");
    gtp_write_register(client, _bRW_MISCTL__TMR0_EN, 0x00); // clear watchdog
    gtp_write_register(client, _bRW_MISCTL__CACHE_EN, 0x00); // clear cache
    gtp_write_register(client, _rRW_MISCTL__BOOTCTL_B0_, 0x02); // boot from sram
    //gtp_write_register(client, _bWO_MISCTL__CPU_SWRST_PULSE, 0x01); // reset software

    gtp_write_register(client, _bRW_MISCTL__SRAM_BANK, 0x00); // select bank 0
    gtp_write_register(client, _bRW_MISCTL__MEM_CD_EN, 0x01); // allow AHB bus accessing code sram

    // ---: burn dsp_short code
    ret = gtp_burn_dsp_short(client);

    if (ret != SUCCESS)
    {
        if(retry_load++ < 5)
        {
            GTP_ERROR("Load dsp failed,times %d retry load!", retry_load);
            goto load_dsp_again;
        }
        else
        {
            SET_INFO_LINE_INFO("Step 2: burn dsp_short code");
            SET_INFO_LINE_ERR("burn dsp_short Timeout!");
            goto short_test_exit;
        }
    }

    SET_INFO_LINE_INFO("Step 2: burn dsp_short code");
	    // step3: run dsp_short, read results
    SET_INFO_LINE_INFO("Step 3: run dsp_short code, confirm it's runnin'");
    gtp_write_register(client, _rRW_MISCTL__SHORT_BOOT_FLAG, 0x00); // clear dsp_short running flag
    gtp_write_register(client, _rRW_MISCTL__BOOT_OPT_B0_, 0x03);//set scramble

    gtp_write_register(client, _bWO_MISCTL__CPU_SWRST_PULSE, 0x01); // reset software

    gtp_write_register(client, _rRW_MISCTL__SWRST_B0_, 0x08);   // release dsp

    client->addr = 0x14;    // for constant iic address
    msleep(50);
    // confirm dsp is running
    i = 0;
    while (1)
    {
        opr_buf[2] = 0x00;
        gtp_read_register(client, _rRW_MISCTL__SHORT_BOOT_FLAG, opr_buf);
        if (opr_buf[2] == 0xAA)
        {
            break;
        }
        ++i;
        if (i >= 20)
        {
            SET_INFO_LINE_ERR("step 3: dsp is not running!");
            goto short_test_exit;
        }
        msleep(10);
    }
    // step4: host configure ic, get test result
    
    SET_INFO_LINE_INFO("Step 4: host config ic, get test result");
    // Short Threshold
    GTP_DEBUG(" Short Threshold: 10");
    opr_buf[0] = 0x88;
    opr_buf[1] = 0x04;
    opr_buf[2] = 0;
    opr_buf[3] = 10;
    gtp_i2c_write(client, opr_buf, 4);

    // ADC Read Delay
    GTP_DEBUG(" ADC Read Delay: 150");
    opr_buf[0] = 0x88;
    opr_buf[1] = 0x06;
    opr_buf[2] = (u8)(150 >> 8);
    opr_buf[3] = (u8)(150);
    gtp_i2c_write(client, opr_buf, 4);

    // DiffCode Short Threshold
    GTP_DEBUG(" DiffCode Short Threshold: 20");
    opr_buf[0] = 0x88;
    opr_buf[1] = 0x51;
    opr_buf[2] = (u8)(20 >> 8);
    opr_buf[3] = (u8)(20);
    gtp_i2c_write(client, opr_buf, 4);

    // Config Driver & Sensor Order
#if GTP_DEBUG_ON
    printk("<<-GTP-DEBUG->> Driver Map:\n");
    printk("IC Driver:");
    for (i = 0; i < MAX_DRIVER_NUM; ++i)
    {
        printk(" %2d", cfg_drv_order[i]);
    }
    printk("\n");
    printk("TP Driver:");
    for (i = 0; i < MAX_DRIVER_NUM; ++i)
    {
        printk(" %2d", i);
    }
    printk("\n");

    printk("<<-GTP-DEBUG->> Sensor Map:\n");
    printk("IC Sensor:");
    for (i = 0; i < MAX_SENSOR_NUM; ++i)
    {
        printk(" %2d", cfg_sen_order[i]);
    }
    printk("\n");
    printk("TP Sensor:");
    for (i = 0; i < MAX_SENSOR_NUM; ++i)
    {
        printk(" %2d", i);
    }
    printk("\n");
#endif

    opr_buf[0] = 0x88;
    opr_buf[1] = 0x08;
    for (i = 0; i < MAX_DRIVER_NUM; ++i)
    {
        opr_buf[2 + i] = cfg_drv_order[i];
        drv_sen_chksum += cfg_drv_order[i];
    }
    gtp_i2c_write(client, opr_buf, MAX_DRIVER_NUM + 2);

    opr_buf[0] = 0x88;
    opr_buf[1] = 0x32;
    for (i = 0; i < MAX_SENSOR_NUM; ++i)
    {
        opr_buf[2+i] = cfg_sen_order[i];
        drv_sen_chksum += cfg_sen_order[i];
    }
    gtp_i2c_write(client, opr_buf, MAX_SENSOR_NUM + 2);

    opr_buf[0] = 0x88;
    opr_buf[1] = 0x50;
    opr_buf[2] = 0 - drv_sen_chksum;
    gtp_i2c_write(client, opr_buf, 2 + 1);

    // clear waiting flag, run dsp
    gtp_write_register(client, _rRW_MISCTL__SHORT_BOOT_FLAG, 0x04);

    // inquirying test status until it's okay
    for (i = 0;;++i)
    {
        gtp_read_register(client, 0x8800, opr_buf);
        if (opr_buf[2] == 0x88)
        {
            break;
        }
        msleep(50);
        if ( i > 100 )
        {
            SET_INFO_LINE_ERR("step 4: inquiry test status timeout!");
            goto short_test_exit;
        }
    }

    // step 5: compute the result
    /* short flag:
          bit0: Rx & Rx
          bit1: Tx & Tx
          bit2: Tx & Rx
          bit3: Tx/Rx & GND/VDD
    */
    gtp_read_register(client, 0x8801, opr_buf);
    GTP_DEBUG("short_flag = 0x%02X", opr_buf[2]);
    SET_INFO_LINE_INFO("");
    SET_INFO_LINE_INFO("Short Test Result:");
    
    GTP_INFO("ctptest_TP-short---gt900_resistor_threshold = %d", gt900_resistor_threshold);
    GTP_INFO("ctptest_TP-shrlt---gt900_gnd_resistor_threshold = %d", gt900_gnd_resistor_threshold);
    
    if ((opr_buf[2] & 0x0f) == 0)
    {
        SET_INFO_LINE_INFO("  PASS!");
        ret = SUCCESS;
    }
    else
    {
        ret2 = SUCCESS;
        if ((opr_buf[2] & 0x08) == 0x08)
        {
            ret2 = gt9_test_gnd_vdd_short(client);
        }
        ret = gtp_compute_rslt(client);
        if (ret == SUCCESS && ret2 == SUCCESS)
        {
            SET_INFO_LINE_INFO("  PASS!");
            ret = SUCCESS;
        }
        else
        {
            ret = FAIL;
        }
    }
    // boot from rom and download code from flash to ram
    gtp_write_register(client, _rRW_MISCTL__BOOT_CTL_, 0x99);
    gtp_write_register(client, _rRW_MISCTL__BOOTCTL_B0_, 0x08);

    client->addr = old_i2c_addr;
    gt9xx_leave_short_test(client);
    //gtptest_irq_enable(client);
    //enable_irq(client->irq);
/*
#ifdef CONFIG_GTP_ESD_PROTECT
ts->gtp_is_suspend = 0;     // resume esd
#endif
*/
    gtp_arch_file_append_no_len("\n");
    return ret;

short_test_exit:
    // boot from rom and download code from flash to ram
    gtp_write_register(client, _rRW_MISCTL__BOOT_CTL_, 0x99);
    gtp_write_register(client, _rRW_MISCTL__BOOTCTL_B0_, 0x08);

    client->addr = old_i2c_addr;
    gt9xx_leave_short_test(client);
    //gtptest_irq_enable(client);
    //enable_irq(client->irq);
 /* 
#ifdef CONFIG_GTP_ESD_PROTECT
ts->gtp_is_suspend = 0;     // resume esd
#endif
*/
    gtp_arch_file_append_no_len("\n");
    return FAIL;
}

u32 endian_mode(void)
{
    union {s32 i; s8 c;}endian;

    endian.i = 1;

    if (1 == endian.c)
    {
        return MYBIG_ENDIAN;
    }
    else
    {
        return MYLITLE_ENDIAN;
    }
}
/*
*********************************************************************************************************
* Function:
*   send read rawdata cmd
* Input:
*   i2c_client* client: i2c device
* Return:
*   SUCCESS: send process succeed, FAIL: failed
*********************************************************************************************************
*/
s32 gt9_read_raw_cmd(struct i2c_client* client)
{
    u8 raw_cmd[3] = {(u8)(GTP_REG_READ_RAW >> 8), (u8)GTP_REG_READ_RAW, 0x01};
    s32 ret = -1;
    GTP_DEBUG("Send read raw data command");
    ret = gtp_i2c_write(client, raw_cmd, 3);
    if(ret <= 0)
    {
        SET_INFO_LINE_ERR("i2c write failed.");
        return FAIL;
    }
    msleep(10);
    return SUCCESS;
}

s32 gt9_read_coor_cmd(struct i2c_client *client)
{
    u8 raw_cmd[3] = {(u8)(GTP_REG_READ_RAW >> 8), (u8)GTP_REG_READ_RAW, 0x0};
    s32 ret = -1;

    ret = gtp_i2c_write(client, raw_cmd, 3);
    if (ret < 0)
    {
        SET_INFO_LINE_ERR("i2c write coor cmd failed!");
        return FAIL;
    }
    msleep(10);
    return SUCCESS;
}
/*
*********************************************************************************************************
* Function:
*   read rawdata from ic registers
* Input:
*   u16* data: rawdata buffer
*   i2c_client* client: i2c device
* Return:
*   SUCCESS: read process succeed, FAIL:  failed
*********************************************************************************************************
*/
s32 gtp_read_rawdata(struct i2c_client* client, u16* data)
{
    s32 ret = -1;
    u16 retry = 0;
    u8 read_state[3] = {(u8)(GTP_REG_RAW_READY>>8), (u8)GTP_REG_RAW_READY, 0};
    u16 i = 0, j = 0;
    u8 *read_rawbuf;
    u8 tail, head;

    read_rawbuf = (u8*)kmalloc(sizeof(u8) * (gt9xx_drv_num*gt9xx_sen_num * 2 + GTP_ADDR_LENGTH), GFP_KERNEL);

    if (NULL == read_rawbuf)
    {
        SET_INFO_LINE_ERR("failed to allocate for read_rawbuf");
        return FAIL;
    }

    if(data == NULL)
    {
        SET_INFO_LINE_ERR("Invalid raw buffer.");
        goto have_error;
    }

    msleep(10);
    while (retry++ < GTP_WAIT_RAW_MAX_TIMES)
    {
        ret = gtp_i2c_read(client, read_state, 3);
        if(ret <= 0)
        {
            SET_INFO_LINE_ERR("i2c read failed.return: %d", ret);
            continue;
        }
        if ((read_state[GTP_ADDR_LENGTH] & 0x80) == 0x80)
        {
            GTP_DEBUG("Raw data is ready.");
            break;
        }
        if ((retry%20) == 0)
		{
            GTP_DEBUG("(%d)read_state[2] = 0x%02X", retry, read_state[GTP_ADDR_LENGTH]);
			if (retry == 100)
			{
				gt9_read_raw_cmd(client);
			}
		}
        msleep(5);
    }
    if (retry >= GTP_WAIT_RAW_MAX_TIMES)
    {
        SET_INFO_LINE_ERR("Wait raw data ready timeout.");
        goto have_error;
    }

    if (chip_type_gt9f)
    {
        read_rawbuf[0] = (u8)( GTP_REG_RAW_DATA_GT9F >> 8);
        read_rawbuf[1] = (u8)( GTP_REG_RAW_DATA_GT9F );
    }
    else
    {
        read_rawbuf[0] = (u8)( GTP_REG_RAW_DATA >> 8);
        read_rawbuf[1] = (u8)( GTP_REG_RAW_DATA );
    }

    ret = gtp_i2c_read(client, read_rawbuf, GTP_ADDR_LENGTH + ((gt9xx_drv_num*gt9xx_sen_num)*2));
    if(ret <= 0)
    {
        SET_INFO_LINE_ERR("i2c read rawdata failed.");
        goto have_error;
    }
    gtp_i2c_end_cmd(client);    // clear buffer state

    if (endian_mode() == MYBIG_ENDIAN)
    {
        head = 0;
        tail =1;
        GTP_DEBUG("Big Endian.");
    }
    else
    {
        head = 1;
        tail = 0;
        GTP_DEBUG("Little Endian.");
    }

    for(i=0,j = 0; i < ((gt9xx_drv_num*gt9xx_sen_num)*2); i+=2)
    {
        data[i/2] = (u16)(read_rawbuf[i+head+GTP_ADDR_LENGTH]<<8) + (u16)read_rawbuf[GTP_ADDR_LENGTH+i+tail];
    #if GTP_DEBUG_ON
        printk("%4d ", data[i/2]);
        ++j;
        if((j%gt9xx_drv_num) == 0)	//add by zhoujinhong
            printk("\n");
    #endif
    }

    kfree(read_rawbuf);
    return SUCCESS;
have_error:
    kfree(read_rawbuf);
    return FAIL;
}
/*
*********************************************************************************************************
* Function:
*   read rawdata from ic registers
* Input:
*   u16* data: rawdata buffer
*   i2c_client* client: i2c device
* Return:
*   SUCCESS: read process succeed, FAIL:  failed
*********************************************************************************************************
*/
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2018/02/21 add for proc node*/
s32 gtp_read_diff_data(struct i2c_client* client,u16* data)
{
    s32 ret = -1;
    u16 retry = 0;
    u8 read_state[3] = {(u8)(GTP_REG_RAW_READY>>8), (u8)GTP_REG_RAW_READY, 0};
    u16 i = 0, j = 0;
    u8 *read_diffbuf;
    u8 tail, head;

    read_diffbuf = (u8*)kmalloc(sizeof(u8) * (gt9xx_drv_num*gt9xx_sen_num * 2 + GTP_ADDR_LENGTH), GFP_KERNEL);

    if (NULL == read_diffbuf)
    {
        SET_INFO_LINE_ERR("failed to allocate for read_rawbuf");
        return FAIL;
    }

	if(data == NULL)
		{
			SET_INFO_LINE_ERR("Invalid raw buffer.");
			goto have_error;
		}

    msleep(10);
    while (retry++ < GTP_WAIT_RAW_MAX_TIMES)
    {
        ret = gtp_i2c_read(client, read_state, 3);
        if(ret <= 0)
        {
            SET_INFO_LINE_ERR("i2c read failed.return: %d", ret);
            continue;
        }
        if ((read_state[GTP_ADDR_LENGTH] & 0x80) == 0x80)
        {
            GTP_DEBUG("Raw data is ready.");
            break;
        }
        if ((retry%20) == 0)
		{
            GTP_DEBUG("(%d)read_state[2] = 0x%02X", retry, read_state[GTP_ADDR_LENGTH]);
			if (retry == 100)
			{
				gt9_read_raw_cmd(client);
			}
		}
        msleep(5);
    }
    if (retry >= GTP_WAIT_RAW_MAX_TIMES)
    {
        SET_INFO_LINE_ERR("Wait raw data ready timeout.");
        goto have_error;
    }
    SET_INFO_LINE_ERR("Wait diff data ready.");
    if (chip_type_gt9f)
    {
        read_diffbuf[0] = (u8)( GTP_REG_RAW_DATA_GT9F >> 8);
        read_diffbuf[1] = (u8)( GTP_REG_RAW_DATA_GT9F );
    }
    else
    {
        //read_diffbuf[0] = (u8)( GTP_REG_DIFF_DATA >> 8);
        //read_diffbuf[1] = (u8)( GTP_REG_DIFF_DATA );
        read_diffbuf[0] = 0xA1;
        read_diffbuf[1] = 0x60;
    }

    ret = gtp_i2c_read(client, read_diffbuf, GTP_ADDR_LENGTH + ((gt9xx_drv_num*gt9xx_sen_num)*2));
    if(ret <= 0)
    {
        SET_INFO_LINE_ERR("i2c read rawdata failed.");
        goto have_error;
    }
    gtp_i2c_end_cmd(client);    // clear buffer state

    if (endian_mode() == MYBIG_ENDIAN)
    {
        head = 0;
        tail =1;
        GTP_DEBUG("Big Endian.");
    }
    else
    {
        head = 1;
        tail = 0;
        GTP_DEBUG("Little Endian.");
    }

    for(i=0,j = 0; i < ((gt9xx_drv_num*gt9xx_sen_num)*2); i+=2)
    {
        data[i/2] = (u16)(read_diffbuf[i+head+GTP_ADDR_LENGTH]<<8) + (u16)read_diffbuf[GTP_ADDR_LENGTH+i+tail];
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/03/14 modified for node function */
        if(data[i/2] > 32767) {
			data[i/2] = 65535-data[i/2];
		}
		if ( (10< data[i/2]) &&(data[i/2] < 40)){
			data[i/2] = 0;
		}
		if ((i > 2) && (data[(i-2)/2] < 30)&&(data[i/2] > 230)){
			data[i/2] = 10;
		}

    #if GTP_DEBUG_ON
        printk("%4d", data[i/2]);
        ++j;
        if((j%gt9xx_drv_num) == 0)	//add by zhoujinhong
            printk("\n");
    #endif
    }

    kfree(read_diffbuf);
    return SUCCESS;
have_error:
    kfree(read_diffbuf);
    return FAIL;
}


s32 gtp_read_diffdata(struct i2c_client* client)
{
    s32 ret = -1;
    u16 retry = 0;
    u8 read_state[3] = {(u8)(GTP_REG_RAW_READY>>8), (u8)GTP_REG_RAW_READY, 0};
    u16 i = 0, j = 0;
    u8 *read_diffbuf;
    u8 tail, head;

    read_diffbuf = (u8*)kmalloc(sizeof(u8) * (gt9xx_drv_num*gt9xx_sen_num * 2 + GTP_ADDR_LENGTH), GFP_KERNEL);

    if (NULL == read_diffbuf)
    {
        SET_INFO_LINE_ERR("failed to allocate for read_rawbuf");
        return FAIL;
    }

    msleep(10);
    while (retry++ < GTP_WAIT_RAW_MAX_TIMES)
    {
        ret = gtp_i2c_read(client, read_state, 3);
        if(ret <= 0)
        {
            SET_INFO_LINE_ERR("i2c read failed.return: %d", ret);
            continue;
        }
        if ((read_state[GTP_ADDR_LENGTH] & 0x80) == 0x80)
        {
            GTP_DEBUG("Raw data is ready.");
            break;
        }
        if ((retry%20) == 0)
		{
            GTP_DEBUG("(%d)read_state[2] = 0x%02X", retry, read_state[GTP_ADDR_LENGTH]);
			if (retry == 100)
			{
				gt9_read_raw_cmd(client);
			}
		}
        msleep(5);
    }
    if (retry >= GTP_WAIT_RAW_MAX_TIMES)
    {
        SET_INFO_LINE_ERR("Wait raw data ready timeout.");
        goto have_error;
    }
    SET_INFO_LINE_ERR("Wait diff data ready.");
    if (chip_type_gt9f)
    {
        read_diffbuf[0] = (u8)( GTP_REG_RAW_DATA_GT9F >> 8);
        read_diffbuf[1] = (u8)( GTP_REG_RAW_DATA_GT9F );
    }
    else
    {
        //read_diffbuf[0] = (u8)( GTP_REG_DIFF_DATA >> 8);
        //read_diffbuf[1] = (u8)( GTP_REG_DIFF_DATA );
        read_diffbuf[0] = 0xA1;
        read_diffbuf[1] = 0x60;
    }

    ret = gtp_i2c_read(client, read_diffbuf, GTP_ADDR_LENGTH + ((gt9xx_drv_num*gt9xx_sen_num)*2));
    if(ret <= 0)
    {
        SET_INFO_LINE_ERR("i2c read rawdata failed.");
        goto have_error;
    }
    gtp_i2c_end_cmd(client);    // clear buffer state

    if (endian_mode() == MYBIG_ENDIAN)
    {
        head = 0;
        tail =1;
        GTP_DEBUG("Big Endian.");
    }
    else
    {
        head = 1;
        tail = 0;
        GTP_DEBUG("Little Endian.");
    }

    for(i=0,j = 0; i < ((gt9xx_drv_num*gt9xx_sen_num)*2); i+=2)
    {
        jitter_limit_temp[i/2] = (u16)(read_diffbuf[i+head+GTP_ADDR_LENGTH]<<8) + (u16)read_diffbuf[GTP_ADDR_LENGTH+i+tail];
/*	if(jitter_limit_temp[i/2] > 32767) {
		jitter_limit_temp[i/2] = 65535-jitter_limit_temp[i/2];
	}
*/
    #if GTP_DEBUG_ON
        printk("%4d", jitter_limit_temp[i/2]);
        ++j;
        if((j%gt9xx_drv_num) == 0)	//add by zhoujinhong
            printk("\n");
    #endif
    }

    kfree(read_diffbuf);
    return SUCCESS;
have_error:
    kfree(read_diffbuf);
    return FAIL;
}
/*
*********************************************************************************************************
* Function:
*   rawdata test initilization function
* Input:
*   u32 check_types: test items
*********************************************************************************************************
*/
s32 gtp_raw_test_init(void)
{
    u16 i = 0;

    test_rslt_buf = (s32*) kmalloc(sizeof(s32)*GTP_OPEN_SAMPLE_NUM, GFP_KERNEL);
    touchpad_sum = (struct gt9xx_open_info*) kmalloc(sizeof(struct gt9xx_open_info) * (4 * _BEYOND_INFO_MAX + 1), GFP_KERNEL);

    if (have_key && key_is_isolated)
    {
        avrg_raw_len = gt9xx_sc_pxl_cnt;
    }
    else
    {
        avrg_raw_len = gt9xx_pixel_cnt;
    }
    avrg_raw_buf = (u32 *)kzalloc(sizeof(u32) * avrg_raw_len, GFP_KERNEL);

    if (NULL == test_rslt_buf || NULL == touchpad_sum || NULL == avrg_raw_buf)
    {
        return FAIL;
    }
    memset(touchpad_sum, 0, sizeof(struct gt9xx_open_info) * (4 * _BEYOND_INFO_MAX + 1));

    for (i = 0; i < (4 * _BEYOND_INFO_MAX); ++i)
    {
        touchpad_sum[i].driver = 0xFF;
    }

    for (i = 0; i < GTP_OPEN_SAMPLE_NUM; i++)
    {
        test_rslt_buf[i] = _CHANNEL_PASS;
    }
    return SUCCESS;
}
/*
*********************************************************************************************************
* Function:
*   touchscreen rawdata max limit test
* Input:
*   u16* raw_buf: rawdata buffer
*********************************************************************************************************
*/
static s32 gtp_raw_max_test_re(u16 *raw_buf)
{
    u16 i, j;
    u8 driver, sensor;
    u8 sum_base = 0 * _BEYOND_INFO_MAX;
    u8 new_flag = 0;
    s32 ret = SUCCESS;

    for (i = 0; i < gt9xx_sc_pxl_cnt; i++)
    {
        if (raw_buf[i] > max_limit_vale_re[i])//max_limit_value)
        {
            test_rslt_buf[rslt_buf_idx] |= _BEYOND_MAX_LIMIT;
            driver = (i/gt9xx_sen_num);
            sensor = (i%gt9xx_sen_num);
            new_flag = 0;
            for (j = sum_base; j < (sum_base+_BEYOND_INFO_MAX); ++j)
            {
                if (touchpad_sum[j].driver == 0xFF)
                {
                    new_flag = 1;
                    break;
                }
                if ((driver == touchpad_sum[j].driver) && (sensor == touchpad_sum[j].sensor))
                {
                    touchpad_sum[j].times++;
                    new_flag = 0;
                    break;
                }
            }
            if (new_flag)   // new one
            {
                touchpad_sum[j].driver = driver;
                touchpad_sum[j].sensor = sensor;
                touchpad_sum[j].beyond_type |= _BEYOND_MAX_LIMIT;
                touchpad_sum[j].raw_val = raw_buf[i];
                touchpad_sum[j].times = 1;
		if(2 == oppo_debug_level){
                        GTP_INFO("[%d, %d]rawdata: %d, raw max limit: %d", driver, sensor, raw_buf[i], max_limit_vale_re[i]);
		}
	    }
			return FAIL;
        }	
	}
	return ret;
}


/*
*********************************************************************************************************
* Function:
*   touchscreen rawdata min limit test
* Input:
*   u16* raw_buf: rawdata buffer
*********************************************************************************************************
*/
static s32 gtp_raw_min_test_re(u16 *raw_buf)
{
    u16 i, j=0;
    u8 driver, sensor;
    u8 sum_base = 1 * _BEYOND_INFO_MAX;
    u8 new_flag = 0;
    s32 ret = SUCCESS;

    for (i = 0; i < gt9xx_sc_pxl_cnt; i++)
    {
        if (raw_buf[i] < min_limit_vale_re[i])//min_limit_value)
        {
            test_rslt_buf[rslt_buf_idx] |= _BEYOND_MIN_LIMIT;
            driver = (i/gt9xx_sen_num);
            sensor = (i%gt9xx_sen_num);
            new_flag = 0;
            for (j = sum_base; j < (sum_base+_BEYOND_INFO_MAX); ++j)
            {
                if (touchpad_sum[j].driver == 0xFF)
                {
                    new_flag = 1;
                    break;
                }
                if ((driver == touchpad_sum[j].driver) && (sensor == touchpad_sum[j].sensor))
                {
                    touchpad_sum[j].times++;
                    new_flag = 0;
                    break;
                }
            }
            if (new_flag)   // new one
            {
                touchpad_sum[j].driver = driver;
                touchpad_sum[j].sensor = sensor;
                touchpad_sum[j].beyond_type |= _BEYOND_MIN_LIMIT;
                touchpad_sum[j].raw_val = raw_buf[i];
                touchpad_sum[j].times = 1;
		if(2 == oppo_debug_level){
                        GTP_INFO("[%d, %d]rawdata: %d, raw min limit: %d", driver, sensor, raw_buf[i], min_limit_vale_re[i]);
		}
	   }
			ret = FAIL;
        }
    }
    return ret;
}
 
//AreaAccord---------------------------

/*
*********************************************************************************************************
* \BA\AF\CA\FD\C3\FB\B3\C6: AreaAccordCheck
* \B9\A6\C4\DC\C3\E8\CA\F6: \CF\E0\C1\DA\CA\FD\BE\DDƫ\B2\EE\B2\E2\CAԣ\AC\BC\B4\BC\EC\B2\E2ĳһͨ\B5\C0\D3\EB\C6\E4\C9ϡ\A2\CF¡\A2\D7\F3\A1\A2\D3\D2ͨ\B5\C0\B5\B1ǰֵ֮\B2\EE\D3\EB\B8\C3ͨ\B5\C0\B5\B1ǰֵ\B5ı\C8ֵ\CAǷ�\FD\C9\E8
*			\B6\A8ֵ
* \B2\CE    \CA\FD: \CE\DE
* \B7\B5 \BB\D8 ֵ: unsigned char			0:\B2\E2\CA\D4δͨ\B9\FD
*									1:\B2\E2\CA\D4ͨ\B9\FD
* \CD\EA\B3\C9ʱ\BC\E4: 2011-05-11
*********************************************************************************************************
*/
unsigned char AreaAccordCheck(u16* raw_buf)
{
    int i,j,index;
    u16 temp;
	u16 accord_temp;
    s32 ret = SUCCESS;

    for (i = 0; i < gt9xx_sen_num; i++)			
    {
		for(j = 0; j < gt9xx_drv_num; j++)
		{
			index = i+j*gt9xx_sen_num;
			
			accord_temp = 0;
			temp = 0;
	
			if (j == 0)	//\B5\DAһ\C1\D0
			{
				if(raw_buf[i+(j+1)*gt9xx_sen_num] > raw_buf[index])
					accord_temp = ((1000*(raw_buf[i+(j+1)*gt9xx_sen_num] - raw_buf[index])) / raw_buf[index]);
				else
					accord_temp = ((1000*(raw_buf[index] - raw_buf[i+(j+1)*gt9xx_sen_num])) / raw_buf[index]);
			}
			else if(j == gt9xx_drv_num-1) 	//\D7\EE\BA\F3һ\C1\D0
			{
				if(raw_buf[i+(j-1)*gt9xx_sen_num] > raw_buf[index])
					accord_temp = ((1000*(raw_buf[i+(j-1)*gt9xx_sen_num] - raw_buf[index])) / raw_buf[index]);
				else
					accord_temp = ((1000*(raw_buf[index] - raw_buf[i+(j-1)*gt9xx_sen_num])) / raw_buf[index]);
			}
			else		//\D6м\E4\C1\D0
			{
				if(raw_buf[i+(j+1)*gt9xx_sen_num] > raw_buf[index])
					accord_temp = ((1000*(raw_buf[i+(j+1)*gt9xx_sen_num] - raw_buf[index]))/ raw_buf[index]);
				else
					accord_temp = ((1000*(raw_buf[index] - raw_buf[i+(j+1)*gt9xx_sen_num]))/ raw_buf[index]);
				if(raw_buf[i+(j-1)*gt9xx_sen_num] > raw_buf[index])
					temp = ((1000*(raw_buf[i+(j-1)*gt9xx_sen_num] - raw_buf[index])) / raw_buf[index]);
				else
					temp = ((1000*(raw_buf[index] - raw_buf[i+(j-1)*gt9xx_sen_num])) / raw_buf[index]);

				if (temp > accord_temp)
				{
					accord_temp = temp;
				}
			}			
			if (i == 0)		//\B5\DAһ\D0\D0
	        {
				if(raw_buf[i+1+j*gt9xx_sen_num] > raw_buf[index])
					temp = ((1000*(raw_buf[i+1+j*gt9xx_sen_num] - raw_buf[index])) / raw_buf[index]);
				else
					temp = ((1000*(raw_buf[index] - raw_buf[i+1+j*gt9xx_sen_num])) / raw_buf[index]);

	            if (temp > accord_temp)
	            {
	                accord_temp = temp;
	            }
	        }
	        else if (i == gt9xx_sen_num-1)		//\D7\EE\BA\F3һ\D0\D0
	        {
				if(raw_buf[i-1+j*gt9xx_sen_num] > raw_buf[index])
					temp = ((1000*(raw_buf[i-1+j*gt9xx_sen_num] - raw_buf[index])) / raw_buf[index]);
				else
					temp = ((1000*(raw_buf[index] - raw_buf[i-1+j*gt9xx_sen_num])) / raw_buf[index]);

	            if (temp > accord_temp)
	            {
	                accord_temp = temp;
	            }
	        }
	        else		//\D6м\E4\D0\D0
	        {
				if(raw_buf[i+1+j*gt9xx_sen_num] > raw_buf[index])
					temp = ((1000*(raw_buf[i+1+j*gt9xx_sen_num] - raw_buf[index]))/ raw_buf[index]);
				else
					temp = ((1000*(raw_buf[index] - raw_buf[i+1+j*gt9xx_sen_num]))/ raw_buf[index]);

	            if (temp > accord_temp)
	            {
	                accord_temp = temp;
	            }
				if(raw_buf[i-1+j*gt9xx_sen_num] > raw_buf[index])
					temp = ((1000*(raw_buf[i-1+j*gt9xx_sen_num] - raw_buf[index])) /raw_buf[index]);
				else
					temp = ((1000*(raw_buf[index] - raw_buf[i-1+j*gt9xx_sen_num])) /raw_buf[index]);

	            if (temp > accord_temp)
	            {
	                accord_temp = temp;
	            }
	        }
	        
	        //accord_limit_temp[index]=accord_temp;
            //GTP_DEBUG("AreaAccordCheck----index==%d",index);
          if (accord_temp > accord_limit_vale_re[index])
          {         
            GTP_ERROR("AreaAccordCheck----gt9xx_drv_num=%d,gt9xx_sen_num=%d",gt9xx_drv_num,gt9xx_sen_num);
            ret = FAIL;
          }
   	  accord_limit_temp[index] = accord_temp;		
	}
    } // end of for (j = 0; j < gt9xx_sen_num*DRIVER_NUM; j++)
	return ret;
}

/*
*********************************************************************************************************
* Function:
*   analyse rawdata retrived from ic registers
* Input:
*   u16 *raw_buf, buffer for rawdata,
*   u32 check_types, test items
* Return:
*   SUCCESS: test process succeed, FAIL: failed
*********************************************************************************************************
*/
static u32 gtp_raw_test_re(u16 *raw_buf, u32 check_types)
{
	s32 ret =0;

    if (raw_buf == NULL)
    {
        GTP_DEBUG("Invalid raw buffer pointer!");
        return FAIL;
    }

    if (check_types & _MAX_TEST)
    {
        GTP_INFO("accord max test");
        ret = gtp_raw_max_test_re(raw_buf);
        GTP_INFO("accord---max ret = %d", ret);
       //gexiantao@20180502-----------------------------
           if(ret)
            {
                 Ito_result_info[RAWDATA_MAXDATA_ID].testitem=RAWDATA_MAXDATA_ID;
                 Ito_result_info[RAWDATA_MAXDATA_ID].result='P';
                 printk("gtp_raw_test_re max test result = pass\n");
            }
            else
            {
                Ito_result_info[RAWDATA_MAXDATA_ID].testitem=RAWDATA_MAXDATA_ID;
                Ito_result_info[RAWDATA_MAXDATA_ID].result='F';
				test_error_code|=0x01;
                printk("gtp_raw_test_re max test result = failed\n");
            }
            //gexiantao@20180502-----------------------------
    }

    if (check_types & _MIN_TEST)
    {
    GTP_INFO("accord min test");
        ret += gtp_raw_min_test_re(raw_buf);//0x02:SUCCESS;0:FAIL
         GTP_INFO("accord---min ret = %d", ret);
//gexiantao@20180502-----------------------------
          if(ret)
            {
                 Ito_result_info[RAWDATA_MINDATA_ID].testitem=RAWDATA_MINDATA_ID;
                 Ito_result_info[RAWDATA_MINDATA_ID].result='P';
                 printk("gtp_raw_test_re min test result = pass\n");
            }
            else
            {
                Ito_result_info[RAWDATA_MINDATA_ID].testitem=RAWDATA_MINDATA_ID;
                Ito_result_info[RAWDATA_MINDATA_ID].result='F';
				test_error_code|=0x02;
                printk("gtp_raw_test_re min test result = failed\n");
            }
            //gexiantao@20180502-----------------------------
    }
    
    if(ret != 2){
    ret =0;    
    }	

    if (check_types & _ACCORD_TEST)//_UNIFORMITY_TEST)     // accord
    {
        GTP_INFO("accord check");
      ret += AreaAccordCheck(raw_buf);			// //0x04:SUCCESS;0:FAIL //\CF\E0\C1\DA\CA\FD\BE\DDƫ\B2\EE\B2\E2\CA\D4    
       GTP_INFO("accord---accord ret = %d", ret);
      //gexiantao@20180502-----------------------------
          if(ret)
            {
                 Ito_result_info[RAWDATA_ACCORD_ID].testitem=RAWDATA_ACCORD_ID;
                 Ito_result_info[RAWDATA_ACCORD_ID].result='P';
                 printk("gtp_raw_test_re check result = pass\n");
            }
            else
            {
                Ito_result_info[RAWDATA_ACCORD_ID].testitem=RAWDATA_ACCORD_ID;
                Ito_result_info[RAWDATA_ACCORD_ID].result='F';
				test_error_code|=0x04;
                printk("gtp_raw_test_re check result = failed\n");
            }
       //gexiantao@20180502-----------------------------
    }
	GTP_INFO("gtp_raw_test_re result ret = %d", ret);

	if (ret == 0x03)
	{
	    SET_INFO_LINE_INFO("  PASS!");
	    ret = SUCCESS;
	}
	else
	{
	    ret = FAIL;
	}
	
    return ret;
}

/*
====================================================================================================
* Function:
*   output the test result
* Return:
*   return the result. if result == 0, the TP is ok, otherwise list the beyonds
====================================================================================================
*/
void gtp_set_avrg_rawbuf(u16 *raw_buf, s32 index)
{
    s32 i = 0;

    if (0 == index)
    {
        for (i = 0; i < avrg_raw_len; ++i)
        {
           avrg_raw_buf[i] = raw_buf[i];
        }
        if (have_key && key_is_isolated)
        {
            for (i = 0; i < 4; ++i)
            {
                key_isolated_avrg[i] = raw_buf[avrg_raw_len + key_iso_pos[i+1]];
            }
        }
    }
    else
    {
        for (i = 0; i < avrg_raw_len; ++i)
        {
            avrg_raw_buf[i] += raw_buf[i];
        }
        if (have_key && key_is_isolated)
        {
            for (i = 0; i < 4; ++i)
            {
                key_isolated_avrg[i] += raw_buf[avrg_raw_len + key_iso_pos[i+1]];
            }
        }
    }
}

void gtp_opentest_arch_raw(void)
{
    u32 i = 0, j = 0;
    u16 avrg_min = 0, avrg_max = 0, avrg_arg = 0;
    long tmp = 0;
    u32 buf_len = 0;
    char *line_buf;

    line_buf = (char *) kzalloc(200, GFP_KERNEL);
    if (!line_buf)
    {
        SET_INFO_LINE_INFO("Failed to allocate memory for line buf");
        return;
    }
    for (i = 0; i < avrg_raw_len; ++i)
    {
        avrg_raw_buf[i] /= GTP_OPEN_SAMPLE_NUM;
        tmp += avrg_raw_buf[i];
        if (avrg_raw_buf[i] > avrg_max)
        {
            avrg_max = avrg_raw_buf[i];
        }
        if (0 == i)
        {
            avrg_min = avrg_raw_buf[i];
        }
        else if (avrg_raw_buf[i] < avrg_min)
        {
            avrg_min = avrg_raw_buf[i];
        }
    }
    avrg_arg = tmp / avrg_raw_len;

    sprintf(line_buf, "\nAverage Raw Data(Drv*Sen: %d*%d, Key:%d): \n", gt9xx_drv_num, gt9xx_sen_num, have_key);
    gtp_arch_file_append_no_len(line_buf);
    for (i = 0; i < avrg_raw_len; i += gt9xx_sen_num)
    {
        memset(line_buf, 0, 200);
        line_buf[0] = '\t';
        buf_len = 1;
        for (j = 0; j < gt9xx_sen_num; ++j)
        {
            buf_len += sprintf(&line_buf[buf_len], "%04d  ", avrg_raw_buf[i+j]);
        }
        gtp_arch_file_append_no_len(line_buf);
        gtp_arch_file_append_no_len("\n");
    }
    gtp_arch_file_append_no_len("Statics:\n");
    memset(line_buf, 0, 200);
    sprintf(line_buf, "\tmax: %04d  min: %04d  average: %04d\n", avrg_max, avrg_min, avrg_arg);
    gtp_arch_file_append_no_len(line_buf);

    if (have_key && key_is_isolated)
    {
        gtp_arch_file_append_no_len("Key Average: \n");
        for (i = 0; i < 4; ++i)
        {
            key_isolated_avrg[i] /= GTP_OPEN_SAMPLE_NUM;
        }
        memset(line_buf, 0, 200);
        line_buf[0] = '\t';
        buf_len = 1;
        for (i = 0; i < key_iso_pos[0]; ++i)
        {
            buf_len += sprintf(&line_buf[buf_len], "key%d: %04d  ", i+1, key_isolated_avrg[i]);
        }
        gtp_arch_file_append_no_len(line_buf);
    }

}

/*
 ===================================================
 * Function:
 *      test gt9 series ic open test
 * Input:
 *      client, i2c_client
 * Return:
 *      SUCCESS: test process success, FAIL, test process failed
 *
 ===================================================
*/
s32 gt9xx_open_test(struct i2c_client * client)
{
    u16 i = 0;
    s32 ret = FAIL; // SUCCESS, FAIL
	s32 ret1 = FAIL; // SUCCESS, FAIL
    u16 *raw_buf = NULL;
/*
#ifdef CONFIG_GTP_ESD_PROTECT
struct goodix_ts_data *ts = i2c_get_clientdata(client);
    //gtptest_irq_disable(client);
    ts->gtp_is_suspend = 1;     // suspend esd
#endif
*/
    gtp_rawdiff_mode = 1;
	//add by 101003082 for ITO test at 2018/06/07 begin
	ITO_TEST_COUNT++;
	if(ITO_TEST_COUNT>50)
		{
		ITO_TEST_COUNT=0;
		}
	//add by 101003082 for ITO test at 2018/06/07 end
    SET_INFO_LINE_INFO("---gtp open test---");
    GTP_INFO("Parsing configuration...");	
    file_pos = 0;
    gtp_open_test_init(client);
    ret = gtp_parse_config(client);
    if (ret == FAIL)
    {
        SET_INFO_LINE_ERR("failed to parse config...");
        ret = FAIL;
        goto open_test_exit;
    }
    raw_buf = (u16*)kmalloc(sizeof(u16)* (gt9xx_drv_num*gt9xx_sen_num), GFP_KERNEL);
    if (NULL == raw_buf)
    {
        SET_INFO_LINE_ERR("failed to allocate mem for raw_buf!");
        ret = FAIL;
        goto open_test_exit;
    }

    GTP_INFO("Step 1: Send Rawdata Cmd");
    ret = gtp_raw_test_init();
    if (FAIL == ret)
    {
        SET_INFO_LINE_ERR("Allocate memory for open test failed!");
        ret = FAIL;
        goto open_test_exit;
    }
    ret = gt9_read_raw_cmd(client);
    if (ret == FAIL)
    {
        SET_INFO_LINE_ERR("Send Read Rawdata Cmd failed!");
        ret = FAIL;
        goto open_test_exit;
    }
	  msleep(20);
    GTP_INFO("Step 2: Sample Rawdata");
    for (i = 0; i < GTP_OPEN_SAMPLE_NUM; ++i)
    {
        //gtp_rawdiff_mode = 1;
        rslt_buf_idx = i;
        ret = gtp_read_rawdata(client, raw_buf);
        current_data_index=i;
		
	gtp_set_avrg_rawbuf(raw_buf, i);
       // ret = gtp_raw_test(raw_buf, DEFAULT_TEST_ITEMS);
        ret = gtp_raw_test_re(raw_buf, DEFAULT_TEST_ITEMS);
        //jitter test start
        if(current_data_index == (GTP_OPEN_SAMPLE_NUM -1)) {
        	gtp_read_diffdata(client);
        	gtp_jitter_test(client);
        }
        //jitter test end
        GTP_INFO("GTP:Save_testing_data---in");
        ret1= Save_testing_data(ito_save_path, 0x0017,raw_buf);//0x0017--jitter+max+min+accord
        if (ret1 == FAIL)
        {
            SET_INFO_LINE_ERR("Read Rawdata failed!");
            ret = FAIL;
            goto open_test_exit;
        }

#if GTP_SAVE_TEST_DATA

        SET_INFO_LINE_ERR("GXT SAVE DATA!");

        if(current_data_index==15)
        {
        	Save_test_result_data("/sdcard/", 0x0057);//0x0057--short+jitter+max+min+accord
        }
        //msleep(80);
#endif
        if (ret == FAIL)
        {
            gtp_i2c_end_cmd(client);
            continue;
        }
    }
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2019/01/18 modified for ito log save*/
    gtp_ito_lcmoff_flag = 1;
    GTP_INFO("Step 3: Analyse Result");
	#if 0
    SET_INFO_LINE_INFO("Total %d Sample Data, Max Show %d Info for each Test Item", GTP_OPEN_SAMPLE_NUM, _BEYOND_INFO_MAX);
    if (0 == gtp_get_test_result())
    {
        ret = SUCCESS;
    }
    else
    {
        ret = FAIL;
    }
	#endif
open_test_exit:
    SET_INFO_LINE_INFO("---gtp open test end---");
    gtp_opentest_arch_raw();
    gtp_arch_file_append_no_len("\n");
    if (raw_buf)
    {
        kfree(raw_buf);
    }
    if (test_rslt_buf)
    {
        kfree(test_rslt_buf);
        test_rslt_buf = NULL;
    }
    if (touchpad_sum)
    {
        kfree(touchpad_sum);
        touchpad_sum = NULL;
    }
    if (avrg_raw_buf)
    {
        kfree(avrg_raw_buf);
        avrg_raw_buf = NULL;
    }
    //gtptest_irq_enable(client);
    /*
#ifdef CONFIG_GTP_ESD_PROTECT
ts->gtp_is_suspend = 0;     // resume esd
#endif
*/
    gtp_rawdiff_mode = 0;
    gt9_read_coor_cmd(client);	// back to read coordinates data
    //msleep(50);
    gtp_reset_guitar(client, 20);
    //msleep(100);
    gtp_send_cfg(client);
		msleep(300);
    return ret;
}
/*
 ===================================================
 * Function:
 *      test gt9 series ic jitter test
 * Input:
 *      client, i2c_client
 * Return:
 *      SUCCESS: test process success, FAIL, test process failed
 *
 ===================================================
*/
s32 gtp_jitter_test(struct i2c_client * client)
{
	s32 result = SUCCESS;
	u16 over_count = 0;
	u16 i = 0;

	for(i = 0; i < (gt9xx_drv_num*gt9xx_sen_num); i++)
    {
		if(jitter_limit_temp[i] > jitter_limit_vale_id0[i]){
			over_count ++;
        }
        if(over_count > 1) {
        	result = FAIL;
        	break;
        }
    }

    if(result == SUCCESS)
    {
         Ito_result_info[JITTER_TEST_ID].testitem=JITTER_TEST_ID;
         Ito_result_info[JITTER_TEST_ID].result='P';
         printk("gtp_jitter_test test result = pass\n");
    } else {
        Ito_result_info[JITTER_TEST_ID].testitem=JITTER_TEST_ID;
        Ito_result_info[JITTER_TEST_ID].result='F';
		test_error_code|=0x10;
        printk("gtp_jitter_test test result = failed\n");
    }
    return result;

}

/*
 ===================================================
 * Function:
 *      test gt9 series ic open test
 * Input:
 *      client, i2c_client
 * Return:
 *      SUCCESS: test process success, FAIL, test process failed
 *
 ===================================================
*/
s32 gt9xx_open_test_accord(struct i2c_client * client)
{
    u16 i = 0;
    s32 ret = 0; // SUCCESS, FAIL
    u16 *raw_buf = NULL;


#ifdef CONFIG_GTP_ESD_PROTECT
	//struct goodix_ts_data *ts = i2c_get_clientdata(client);
    //gtptest_irq_disable(client);
    tpd_halt = 1;     // suspend esd
#endif
    gtp_rawdiff_mode = 1;

    SET_INFO_LINE_INFO("---gtp accord test---");
    GTP_INFO("Parsing configuration...");
    gtp_open_test_init(client);
    ret = gtp_parse_config(client);
    if (ret == FAIL)
    {
        SET_INFO_LINE_ERR("failed to parse config...");
        ret = FAIL;
        goto open_test_exit;
    }
    raw_buf = (u16*)kmalloc(sizeof(u16)* (gt9xx_drv_num*gt9xx_sen_num), GFP_KERNEL);
    if (NULL == raw_buf)
    {
        SET_INFO_LINE_ERR("failed to allocate mem for raw_buf!");
        ret = FAIL;
        goto open_test_exit;
    }

    GTP_INFO("Step 1: Send Rawdata Cmd");

    ret = gtp_raw_test_init();
    if (FAIL == ret)
    {
        SET_INFO_LINE_ERR("Allocate memory for open test failed!");
        ret = FAIL;
        goto open_test_exit;
    }
    ret = gt9_read_raw_cmd(client);
    if (ret == FAIL)
    {
        SET_INFO_LINE_ERR("Send Read Rawdata Cmd failed!");
        ret = FAIL;
        goto open_test_exit;
    }
    GTP_INFO("Step 2: accord Sample Rawdata");
    for (i = 0; i < GTP_OPEN_SAMPLE_NUM; ++i)
    {
        //gtp_rawdiff_mode = 1;
        rslt_buf_idx = i;
        ret = gtp_read_rawdata(client, raw_buf);
        if (ret == FAIL)
        {
            SET_INFO_LINE_ERR("Read Rawdata failed!");
            ret = FAIL;
            goto open_test_exit;
        }
//        gtp_set_avrg_rawbuf(raw_buf, i);
        ret = gtp_raw_test_re(raw_buf, DEFAULT_TEST_ITEMS);
        GTP_INFO("accord---test_re ret = %d", ret);
        if (ret == FAIL)
        {
            gtp_i2c_end_cmd(client);
            continue;
        }
    }
    GTP_INFO("Step 3: accord Analyse Result");
    SET_INFO_LINE_INFO("Total %d Sample Data, Max Show %d Info for each Test Item", GTP_OPEN_SAMPLE_NUM, _BEYOND_INFO_MAX);
  /*
  if (0 == gtp_get_test_result())
    {
        ret = SUCCESS;
    }
    else
    {
        ret = FAIL;
    }*/

open_test_exit:
    SET_INFO_LINE_INFO("---gtp accord test end---");
	gtp_opentest_arch_raw();
	gtp_arch_file_append_no_len("\n");
    if (raw_buf)
    {
        kfree(raw_buf);
    }
    if (test_rslt_buf)
    {
        kfree(test_rslt_buf);
        test_rslt_buf = NULL;
    }
    if (touchpad_sum)
    {
        kfree(touchpad_sum);
        touchpad_sum = NULL;
    }
    if (avrg_raw_buf)
    {
        kfree(avrg_raw_buf);
        avrg_raw_buf = NULL;
    }
    //gtptest_irq_enable(client);
#ifdef CONFIG_GTP_ESD_PROTECT
    tpd_halt = 0;     // resume esd
#endif
    gtp_rawdiff_mode = 0;
    gt9_read_coor_cmd(client);	// back to read coordinates data
    msleep(50);
    gtp_reset_guitar(client, 20);
    msleep(100);
    gtp_send_cfg(client);
	msleep(300);
    return ret;
}

/*
 ===================================================
 * Function:
 *      test gt9 series ic open test
 * Input:
 *      client, i2c_client
 * Return:
 *      SUCCESS: test process success, FAIL, test process failed
 *
 ===================================================
*/
s32 gt9xx_open_test_re(struct i2c_client * client)
{
    u16 i = 0;
    s32 ret = FAIL; // SUCCESS, FAIL
    u16 *raw_buf = NULL;


    GTP_INFO("Step 5: accord Sample Rawdata");
    for (i = 0; i < GTP_OPEN_SAMPLE_NUM; ++i)
    {
        //gtp_rawdiff_mode = 1;
        rslt_buf_idx = i;
        ret = gtp_read_rawdata(client, raw_buf);
        if (ret == FAIL)
        {
            SET_INFO_LINE_ERR("Read Rawdata failed!");
            ret = FAIL;
            return ret;
        }
 //       gtp_set_avrg_rawbuf(raw_buf, i);
        ret = gtp_raw_test_re(raw_buf, DEFAULT_TEST_ITEMS);
        if (ret == FAIL)
        {
            gtp_i2c_end_cmd(client);
            continue;
        }
    }     
    #if 0
    GTP_INFO("Step 6: Analyse Result");
    SET_INFO_LINE_INFO("Total %d Sample Data, Max Show %d Info for each Test Item", GTP_OPEN_SAMPLE_NUM, _BEYOND_INFO_MAX);
    if (0 == gtp_get_test_result())
    {
        ret = SUCCESS;
    }
    else
    {
        ret = FAIL;
    }
    #endif
    return ret;
}

//add by 101003082 for ITO test at 2018/04/17 begin
static ssize_t gtp_ito_test_write(struct file *file, const char __user *buffer,
        size_t count, loff_t *pos)
{
	return -EPERM;
}

/*
 * return value:
 * 0x0: open test failed, short test failed
 * 0x1: open test success, short test failed
 * 0x2: open test failed, short test success
 * 0x3: open test success, short test success
 */
 /*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2018/02/21 add for proc node*/
/*
static void focal_press_powerkey(void)
{
    GTP_INFO(" %s POWER KEY event %x press\n",  KEY_POWER);
    input_report_key(tpd->dev, KEY_POWER, 1);
    input_sync(tpd->dev);
    GTP_INFO(" %s POWER KEY event %x release\n",  KEY_POWER);
    input_report_key(tpd->dev, KEY_POWER, 0);
    input_sync(tpd->dev);
}
*/
 static int gtp_ito_test_off_start(struct seq_file *file, void* data)
{
    int count1=0;
	GTP_INFO("Start panel off test");
    msleep(600);
    count1 = gtp_ito_test_show(file,data);
    gtp_ito_lcmoff_flag=0;
    return count1;
}

 /*Le.Wang@ODM_HQ.BSP.TP.Function, 2019/01/18 modified for ito*/
static int gtp_ito_test_show(struct seq_file *file, void* data)
{
	int ret = 0;
	int count=0;
	int count_err=0;
	int ito_TestResult_gtp = 0;
	int i=0;
	int trytimes=0;
    //gexiantao@20180502-----------------------------
    //init result info

    // modify for esd start-----------------------
#ifdef CONFIG_GTP_ESD_PROTECT
    gtp_esd_switch(i2c_client_point, SWITCH_OFF);
    #endif
    gtp_ito_test_on=1;
    gtptest_irq_disable(i2c_client_point);
    msleep(30);//delay 2s for esd close
      // modify for esd end-----------------------

    trytimes=0;
    Ito_result_info= (struct gt9xx_iot_result_info*) kmalloc(sizeof(struct gt9xx_iot_result_info) * 5, GFP_KERNEL);
    memset(Ito_result_info, 0, sizeof(struct gt9xx_iot_result_info) * 5);
    printk("gtp_ito_test_show enter!!\n");
    for(i=0;i<ITO_TEST_ITEM_NUM-1;i++)
    {
        Ito_result_info[i].stringline='-';
    }
    //-------------------------------gexiantao@20180502
   // gtptest_irq_disable(i2c_connect_client);//

    // reset to normal mode
    //gtp_reset_guitar(i2c_client_point, 20);
	test_error_code=0;//init error code
    // short test
    ret = gt9xx_short_test(i2c_client_point);
    //gexiantao@20180502-----------------------------
    if(ret)
    {
         Ito_result_info[SHORT_TEST_ID].testitem=SHORT_TEST_ID;
         Ito_result_info[SHORT_TEST_ID].result='P';
         printk("gtp_ito_test_show short test result = pass \n");
    }
    else
    {
        Ito_result_info[SHORT_TEST_ID].testitem=SHORT_TEST_ID;
        Ito_result_info[SHORT_TEST_ID].result='F';
        test_error_code|=0x20;
        printk("gtp_ito_test_show short test result = failed \n");
    }
   //------------------------------gexiantao@20180502-
do{
    // open test
    ret += gt9xx_open_test(i2c_client_point);
    trytimes++;
    }while((((test_error_code&0x01)==0x01)||((test_error_code&0x02)==0x02)||((test_error_code&0x04)==0x04))&&(trytimes<1));

	if (ret == 2)
	{
		ito_TestResult_gtp = 1;
	}
	else
	{
		ito_TestResult_gtp = 0;
	}
	//gexiantao@20180502-----------------------------
	//Ito_result_info[RAWDATA_OTHER_ID].testitem=RAWDATA_OTHER_ID;
    //Ito_result_info[RAWDATA_OTHER_ID].result='P';
    //-----------------------------gexiantao@20180502

    //gtptest_irq_enable(i2c_connect_client);
    printk("gtp_ito_test_show();\n");
	//count= seq_printf(file,"%d",ito_TestResult_gtp);
    //gexiantao@20180502-----------------------------
	for(i=0;i<ITO_TEST_ITEM_NUM;i++)
	{
	    printk("%d%c",Ito_result_info[i].testitem,Ito_result_info[i].result);
	    //count= seq_printf(file,"%d%c",Ito_result_info[i].testitem,Ito_result_info[i].result);
		//seq_printf(file,"%d%c",Ito_result_info[i].testitem,Ito_result_info[i].result);
		//Ito_result_info[i].result='P';
	        if(Ito_result_info[i].result=='F')
	        {
				count_err++;
	        }
		if(i < ITO_TEST_ITEM_NUM-1)
		{
			printk("%c",Ito_result_info[i].stringline);
			//count= seq_printf(file,"%c",Ito_result_info[i].stringline);
			//seq_printf(file,"%c",Ito_result_info[i].stringline);
		}
	}
	//seq_printf(file,"\t\n");
		if(count_err==0)
		{
			seq_printf(file,"%d error(s) All test passed!",count_err);
			seq_printf(file,"\t\n");
		}
		else
		{
			seq_printf(file,"%d error(s)",count_err);
			seq_printf(file,"\t\n");
		}
	printk("\ngtp_ito_test_show end!!\n");
	//-------------------------------gexiantao@20180502
// modify for esd start-----------------------
#ifdef CONFIG_GTP_ESD_PROTECT
     gtp_esd_switch(i2c_client_point, SWITCH_ON);
     //ts->gtp_is_suspend = 0;     // suspend esd
#endif
  gtp_ito_test_on=0;
 // modify for esd end-----------------------
 gtptest_irq_enable(i2c_client_point);//
    return count;
}

static int gtp_ito_test_open_on (struct inode* inode, struct file* file)
{
	return single_open(file, gtp_ito_test_show, NULL);
}
static int gtp_ito_test_open_off (struct inode* inode, struct file* file)
{
	return single_open(file, gtp_ito_test_off_start, NULL);
}


//lcmon
static const struct file_operations ito_test_ops_on = {
    .owner = THIS_MODULE,
    .open = gtp_ito_test_open_on,
    .read = seq_read,
    .write = gtp_ito_test_write,
};

//lcmoff
static const struct file_operations ito_test_ops_off = {
    .owner = THIS_MODULE,
    .open = gtp_ito_test_open_off,
    .read = seq_read,
    .write = gtp_ito_test_write,
};
/*Jianchao.Gu@ODM_HQ.BSP.TP.Function, 2018/02/21 add for proc node*/
extern int gesture_coordinate[14];
static int oppo_gtp_coordinate_show(struct seq_file *s, void *v)
{
    seq_printf(s, "%d,%d:%d,%d:%d,%d:%d,%d:%d,%d:%d,%d:%d,%d\n", gesture_coordinate[0],
            gesture_coordinate[1], gesture_coordinate[2], gesture_coordinate[3], gesture_coordinate[4],
            gesture_coordinate[5], gesture_coordinate[6], gesture_coordinate[7], gesture_coordinate[8],
            gesture_coordinate[9], gesture_coordinate[10],gesture_coordinate[11],gesture_coordinate[12],
            gesture_coordinate[13]);

    return 0;
}

static int oppo_fts_coordinate_open(struct inode *inode, struct file *file)
{
    return single_open(file, oppo_gtp_coordinate_show, NULL);
}

static const struct file_operations oppo_gtp_coordinate_fops = {
		.owner = THIS_MODULE,
		.open = oppo_fts_coordinate_open,
		.read = seq_read,
		//.write = gtp_ito_test_write,
};
static struct proc_dir_entry *gtp_ito_test_proc_on;
static struct proc_dir_entry *gtp_ito_test_proc_off;
struct proc_dir_entry *gtp_android_touch_proc;
static struct proc_dir_entry *oppo_gtp_coordinate;


int gtp_create_ito_test_proc(struct i2c_client *client)
{
	//lcmon
	ITO_TEST_COUNT=0;

	gtp_android_touch_proc = proc_mkdir(GTP_ANDROID_TOUCH, NULL);
	gtp_ito_test_proc_on = proc_create(GTP_ITO_TEST_lcmon, 0666,
					      gtp_android_touch_proc, &ito_test_ops_on);
		if (!gtp_ito_test_proc_on){
			dev_err(&client->dev, "create_proc_entry %s failed\n",
				GTP_ITO_TEST_lcmon);}
		  else {
			dev_info(&client->dev, "create proc entry %s success\n",
				 GTP_ITO_TEST_lcmon);
	            }
        //lcmoff
	gtp_ito_test_proc_off = proc_create(GTP_ITO_TEST_lcmoff, 0666,
					      gtp_android_touch_proc, &ito_test_ops_off);
		if (!gtp_ito_test_proc_off){
			dev_err(&client->dev, "create_proc_entry %s failed\n",
				GTP_ITO_TEST_lcmoff);}
		  else {
			dev_info(&client->dev, "create proc entry %s success\n",
				 GTP_ITO_TEST_lcmoff);
				 }

	 oppo_gtp_coordinate = proc_create(OPPO_GTP_COORDINATE, 0666, gtp_android_touch_proc, &oppo_gtp_coordinate_fops);
	 if (!oppo_gtp_coordinate){
		 dev_err(&client->dev, "create_proc_entry %s failed\n",
			 OPPO_GTP_COORDINATE);}
	   else {
		 dev_info(&client->dev, "create proc entry %s success\n",
			  OPPO_GTP_COORDINATE);
		 }
	 return 0;
}
//add by 101003082 for ITO test at 2018/04/17 end

/*
 * return value:
 * 0x0: open test failed, short test failed
 * 0x1: open test success, short test failed
 * 0x2: open test failed, short test success
 * 0x3: open test success, short test success

static ssize_t gtp_sysfs_ctptest_show(struct device *dev,struct device_attribute *attr, char *buf)
{
    int ret = 0;
	int count=0;
	int ito_TestResult_gtp = 0;
	
    gtptest_irq_disable(i2c_connect_client);

    // reset to normal mode
    gtp_reset_guitar(i2c_connect_client, 20);

    // short test
    ret = gt9xx_short_test(i2c_connect_client);

    // open test
    ret += gt9xx_open_test(i2c_connect_client);

	
	if (ret == 2)
	{
		ito_TestResult_gtp = 1;
	}
	else
	{
		ito_TestResult_gtp = 0;
	}	
    gtptest_irq_enable(i2c_connect_client);
	count= sprintf(buf,"%d\n",ito_TestResult_gtp);	
	printk("gtp_sysfs_ctptest_show();\n");
    return count;

}

*/ 

/*
static ssize_t gtp_sysfs_ctptest_store(struct device *dev,struct device_attribute *attr, const char *buf, size_t count)
{
    return -EPERM;
}

*/
//static DEVICE_ATTR(factory_check, S_IRUGO|S_IWUSR, gtp_sysfs_ctptest_show, gtp_sysfs_ctptest_store);


//static DEVICE_ATTR(appid, S_IRUGO|S_IWUSR, gtp_sysfs_appid_show, gtp_sysfs_appid_store);
//static DEVICE_ATTR(mode, S_IRUGO, gtp_sysfs_mode_show, NULL);
//static DEVICE_ATTR(fwupdate, S_IRUGO|S_IWUSR, gtp_sysfs_fwupdate_show, gtp_sysfs_fwupdate_store);



/* add your attr in here
static struct attribute *df_gtp_test_attributes[] = {
    &dev_attr_factory_check.attr,
    NULL
};

static struct attribute_group df_gtp_test_attribute_group = {
    .attrs = df_gtp_test_attributes
};
*/
/*******************************************************
Description:
    Goodix debug sysfs init function.

Parameter:
    none.

return:
    Executive outcomes. 0---succeed.
*******************************************************/
/*
s32 gtp_test_sysfs_init(struct platform_device * gtpinfo_device)
{
    s32 ret ;
	goodix_debug_kobj = &gtpinfo_device->dev.kobj;
	ret = sysfs_create_group(&gtpinfo_device->dev.kobj, &df_gtp_test_attribute_group);
    if (0 != ret) {
        sysfs_remove_group(&gtpinfo_device->dev.kobj, &df_gtp_test_attribute_group);
    	}
    

    SET_INFO_LINE_INFO("Starting initializing gtp_debug_sysfs");
	
    if (goodix_debug_kobj == NULL)
    {
        GTP_ERROR("%s: subsystem_register failed\n", __func__);
        return -ENOMEM;
    }

    //ret = sysfs_create_file(goodix_debug_kobj, &dev_attr_ctptest.attr);
    //ret |= sysfs_create_file(goodix_debug_kobj, &dev_attr_appid.attr);
    //ret |= sysfs_create_file(goodix_debug_kobj, &dev_attr_mode.attr);
   // ret |= sysfs_create_file(goodix_debug_kobj, &dev_attr_fwupdate.attr);


    if (ret)
    {
        GTP_ERROR("%s: sysfs_create_version_file failed\n", __func__);
        return ret;
    }

    GTP_INFO("Goodix debug sysfs create success!\n");
    return 0 ;
}
*/
//void gtp_test_sysfs_deinit(void)
//{

	//huaqin add by xudongfang for temporary ito test at 2018/2/11 start
	//platform_device_unregister(&hwinfo_device);
	//huaqin add by xudongfang for temporary ito test at 2018/2/11 end

    //sysfs_remove_file(goodix_debug_kobj, &dev_attr_ctptest.attr);
    //sysfs_remove_file(goodix_debug_kobj, &dev_attr_appid.attr);
    //sysfs_remove_file(goodix_debug_kobj, &dev_attr_mode.attr);
   // sysfs_remove_file(goodix_debug_kobj, &dev_attr_fwupdate.attr);
    

//}
