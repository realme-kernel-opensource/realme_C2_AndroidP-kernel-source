/*
 *
 * yixue.ge add for oppo project
 *
 *
 */
#ifndef _OPPO_PROJECT_H_
#define _OPPO_PROJECT_H_

enum{
    HW_VERSION__UNKNOWN,
    HW_VERSION__10,
    HW_VERSION__11,
    HW_VERSION__12,
    HW_VERSION__13,
    HW_VERSION__14,
    HW_VERSION__15,
    HW_VERSION__16,
    HW_VERSION__17,
    HW_VERSION_MAX,
};

enum{
    RF_VERSION__UNKNOWN,
    RF_VERSION__11,
    RF_VERSION__12,
    RF_VERSION__13,
    RF_VERSION__21,
    RF_VERSION__22,
    RF_VERSION__23,
    RF_VERSION__31,
    RF_VERSION__32,
    RF_VERSION__33,
};

#define GET_PCB_VERSION() (get_PCB_Version())
#define GET_PCB_VERSION_STRING() (get_PCB_Version_String())

#define GET_MODEM_VERSION() (get_Modem_Version())
#define GET_OPERATOR_VERSION() (get_Operator_Version())

enum OPPO_PROJECT {
    OPPO_UNKOWN = 0,
    OPPO_18151 = 18151,
    OPPO_18531 = 18531,
};

enum OPPO_OPERATOR {
    OPERATOR_UNKOWN                   	= 0,
    /*for realme, start from 56*/
    OPERATOR_18531_ASIA_SIMPLE			= 56,
    OPERATOR_18531_ASIA					= 57,
    OPERATOR_18531_All_BAND				= 58,
    OPERATOR_18151_All_BAND				= 59,  // 18151
    OPERATOR_18151_MOBILE				= 60,  //18153
    OPERATOR_18151_CARRIER				= 61,  //18551
};

enum{
    SMALLBOARD_VERSION__0,
    SMALLBOARD_VERSION__1,
    SMALLBOARD_VERSION__2,
    SMALLBOARD_VERSION__3,
    SMALLBOARD_VERSION__4,
    SMALLBOARD_VERSION__5,
    SMALLBOARD_VERSION__6,
    SMALLBOARD_VERSION__UNKNOWN = 100,
};

typedef enum OPPO_PROJECT OPPO_PROJECT;

typedef struct
{
    unsigned int    nProject;
    unsigned int    nModem;
    unsigned int    nOperator;
    unsigned int    nPCBVersion;
} ProjectInfoCDTType;

unsigned int get_project(void);
unsigned int is_project(OPPO_PROJECT project );
unsigned int get_PCB_Version(void);
unsigned int get_Modem_Version(void);
unsigned int get_Operator_Version(void);

#endif
