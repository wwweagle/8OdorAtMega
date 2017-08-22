#include "commons.h"
#include "Arduino.h"




#ifndef ZXLIBGEN2_H
#define  ZXLIBGEN2_H




typedef unsigned long _prog_addressT;

typedef struct {
    unsigned char L_Hi;
    unsigned char L_Lo;
    unsigned char R_Hi;
    unsigned char R_Lo;
    const unsigned char fullDuty;
} PWM_T_G2;

typedef struct {
    volatile unsigned int timer;
    unsigned int onTime;
    unsigned int offTime;
    unsigned int ramp;
    volatile unsigned int ramping;
    unsigned int on;
    unsigned int side;
} LASER_T_G2;

typedef struct {
    volatile unsigned int current;
    volatile unsigned long filter;
    unsigned int flag;
    volatile unsigned int LCount;
    volatile unsigned int RCount;
} LICK_T_G2;

typedef struct {
    unsigned int stim1Length;
    unsigned int stim2Length;
    unsigned int respCueLength;
    unsigned int distractorLength;
    unsigned int currentDistractor;
    unsigned int distractorJudgingPair;
    unsigned int respCueValve;
    unsigned int shapingExtra;
    int falseAlarmPenaltyRepeat;

} STIM_T_G2;

typedef struct {
    int moving;
    int steadyThresh;
//    int moved;
//    long moveCounter;
    int steadySent;
    long steadyCounter;
} BALL_T_G2;

#define FALSE_ALARM_PENALTY_TIMEOUT 10
#define FALSE_ALARM_PENALTY_REPEAT 20


// #define _EE_WORD         2
// #define _memcpy_p2d16(dest, src, len)  _memcpy_helper(src, dest, len, 0)
// #define _wait_eedata() { while (NVMCONbits.WR); }
// #define _erase_eedata  _eedata_helper1
// #define _write_eedata_word  _eedata_helper3
// extern void _eedata_helper1(_prog_addressT dst, int len);
// extern void _eedata_helper3(_prog_addressT dst, int dat);
// extern _prog_addressT _memcpy_helper(_prog_addressT src, void *dst,
//         unsigned int len, int flags);
// extern void safe_wait_ms(int duration);


typedef unsigned int _delayT;
//laser trial type
#define laserOff 0
#define laserDuring3Baseline 3
#define laserDuring4Baseline 4
#define laserDuringBaseline 5
#define laserDuringDelay_Odor2 6
#define laserDuringBaseAndResponse 7
#define laserDuringBaselineNDelay 9
#define laserDuringDelay 10
#define laserDuringDelayChR2 11

#define laserRampDuringDelay 14
//#define laserLDuringDelay 15
//#define laserRDuringDelay 16
#define laserDuringOdor 20
#define laserDuring1stOdor 21
#define laserDuring2ndOdor 22
#define laserDuringBeginningToOneSecInITI 30
#define laserDuringEarlyDelay 40
#define laserDuringMidDelay 50
#define laserDuringLateDelay 60
#define laserNoDelayControlShort 69
#define laserNoDelayControl 70
#define laserDuringEarlyHalf 80
#define laserDuring1Terice 81
#define laserDuring2Terice 82
#define laserDuring3Terice 83
#define laserDuringLateHalf 90
#define laserDuring1Quarter 91
#define laserDuring2Quarter 92
#define laserDuring3Quarter 93
#define laserDuring4Quarter 94
#define laserDuringResponseDelay 95

#define laserDuring12s1Quarter 96
#define laserDuring12s2Quarter 97
#define laserDuring12s3Quarter 98
#define laserDuring12s4Quarter 99

#define trialTypeDnmsSwitch 100
#define trialTypeGoNogoSwitch 110
#define laser4sRamp 121
#define laser2sRamp 122
#define laser1sRamp 123
#define laser_5sRamp 124
#define laserSufficiency 130
#define laserBeforeDistractor 140
#define laserCoverDistractor 145
#define laserAfterDistractorMax 151
#define laserAfterMultiDistractor 160







//#define atTrialStart 10
#define fourSecBeforeFirstOdor 4
#define threeSecBeforeFirstOdor 5
#define oneSecBeforeFirstOdor 10
#define at500mSBeforeFirstOdor 18
#define atFirstOdorBeginning 20
#define atFirstOdorEnd 30
#define atDelayBegin 40
#define atDelay_5SecIn 42
#define atDelay1SecIn 200
#define atDelay1_5SecIn 205
#define atDelay2SecIn 210
#define atDelay2_5SecIn 212
#define atDelay3SecIn 214
#define atDelay3_5SIn 216
#define atDelay4_5SIn 218
#define atPreDualTask 220
#define atPostDualTask 222
#define atDelay1sToMiddle 224
#define atDelay500msToMiddle 225
#define atDelayMiddle 230
#define atDelayMid500mSec 232
#define atDelayMid1Sec 233
#define atDelayMid1_5Sec 234
#define atDelayMid2Sec 235
#define atDelayMid2_5Sec 240
#define atDelayMid3Sec 245
#define atDelayLast2_5SecBegin 250
#define atDelayLast2SecBegin 255
#define atDelayLast1_5SecBegin 61
#define atDelayLastSecBegin 63
#define atDelayLast500mSBegin 65
//#define atDelayLast200mSBegin 68
#define atSecondOdorBeginning 70
#define atSecondOdorEnd 80
//#define atRewardDelayBeginning 90
#define atResponseCueBeginning 90
#define atResponseCueEnd 95

#define atRewardBeginning 100
//#define atRewardBeginning 110
#define atRewardEnd 120
#define atITIBeginning 130
#define atITIOneSecIn 140
#define atITILastSecIn 150
#define atITIEnd 160


//Laser Session Type, including ZJ's variety
#define LASER_OTHER_TRIAL 1
#define LASER_NO_TRIAL 2
#define LASER_EVERY_TRIAL 3
//#define laserFollowOdorA 4
//#define laserFollowOdorB 5
//#define laser1and2Half 6
//#define laser3and4Quarter 10
#define LASER_LR_EACH_QUARTER 20
#define LASER_EACH_QUARTER 21
#define LASER_12s_LR_EACH_QUARTER 25
#define LASER_12s_EACH_QUARTER 26
#define LASER_VARY_LENGTH 30
#define LASER_LR_EVERYTRIAL 40
#define LASER_LR_EVERY_OTHER_TRIAL 42
#define LASER_INCONGRUENT_CATCH_TRIAL 45
#define LASER_13s_EarlyMidLate 50
#define LASER_DUAL_TASK_ON_OFF 60
#define LASER_DUAL_TASK_ODAP_ON_OFF 61
#define LASER_AFTER_DISTR_3X 65
#define LASER_OTHER_BLOCK 80
#define LASER_SESS_UNDEFINED 100


// #define LICK_LEFT (!PORTDbits.RD14)
// #define LICK_RIGHT (!PORTDbits.RD15)
// #define LICK_ANY (!PORTDbits.RD14 || !PORTDbits.RD15)
// #define BALL_MOVING (PORTDbits.RD12 || PORTDbits.RD13)
#define LICKING_LEFT 2
#define LICKING_RIGHT 3
#define LICKING_BOTH 127

#define SHAPING_TASK 10
#define DNMS_LEARNING_TASK 19
#define DNMS_TASK 20
//#define _ASSOCIATE_SHAPING_TASK 24
//#define _ASSOCIATE_TASK 25
#define GONOGO_TASK 30
#define DNMS_2AFC_TEACH 38
#define DNMS_LR_LEARNING_TASK 39
#define DNMS_2AFC_TASK 40
#define GONOGO_2AFC_TEACH 49
#define GONOGO_LR_TASK 50
#define NO_ODOR_CATCH_TRIAL_TASK 60
#define VARY_ODOR_LENGTH_TASK 70
#define OPTO_ODPA_TASK 80
#define OPTO_ODPA_SHAPING_TASK 82
#define DUAL_TASK_LEARNING 90
#define DUAL_TASK 93
#define DUAL_TASK_ON_OFF_LASER_TASK 95
#define DUAL_TASK_ODAP_ON_OFF_LASER_TASK 96
#define DUAL_TASK_DISTRx3_TASK 98
#define ODPA_SHAPING_TASK 99
#define ODPA_RD_SHAPING_A_CATCH_LASER_TASK 100
#define ODPA_RD_SHAPING_B_CATCH_LASER_TASK 101
#define ODPA_RD_CATCH_LASER_TASK 102
#define ODPA_RD_SHAPING_NO_LASER_TASK 103
#define ODPA_TASK 110
#define ODPA_SHAPING_BALL_TASK 115
#define ODPA_BALL_TASK 116
#define DNMS_DUAL_TASK_LEARNING 120
#define DNMS_DUAL_TASK 125
#define DUAL_TASK_EVERY_TRIAL  130
#define BALL_IMMOBILE 150





#define EEP_DUTY_LOW_L_OFFSET 0
#define EEP_DUTY_HIGH_L_OFFSET 2
#define EEP_DUTY_LOW_R_OFFSET 4
#define EEP_DUTY_HIGH_R_OFFSET 6



#define PERM_INFO 21

#define DMS_LR_Teach_LOFF 100
#define DMS_LR_Teach_LON 101
#define MSWOD_LR_Teach_LOFF 102
#define DMS_LR_Teach_ONOFF 103
#define DMS_LR_5Delay_Laser 104
#define DMS_LR_8Delay_Laser 105
#define DMS_LR_12Delay_Laser 106
#define DMS_LR_1Odor_Laser 107
#define DMS_LR_2Odor_Laser 108
#define DMS_LR_bothOdor_Laser 109
#define DMS_LR_baseline_Laser 110
#define DMS_LR_response_Laser 111


// #define SpBallMove 82
#define SpResponseCue 83
#define SpAbortTrial 84

#define water_sweet 21


//extern float odorLength;
// char * getVer_G2(void);

extern const char odorTypes_G2[];
extern unsigned int laserSessionType_G2;
extern unsigned int taskType_G2;
//extern unsigned int licking;
//extern unsigned int timeFilter;
//extern unsigned int lickFlag;
//extern unsigned int lickLCount;
//extern unsigned int lickRCount;

extern const int _DEBUGGING;

void zxTimer1Gen2(void);

void shuffleArray_G2(unsigned int orgArray[], unsigned int arraySize);
void lcdWriteNumber_G2(int n, int digits, int x, int y);
void lcdWriteChar_G2(char ch, int x, int y);
void lcdWriteString_G2(const char s[]);


//extern static void zxLaserTrial(int type, int FirstOdorIn, float odorLength, _delayT OdorDelayIn, int SecondOdorIn, float WaterLIn, int ITI, float delay_before_reward, int laserOnTrial);
// void callFunction(int n);


void wait_ms_G2(int time);
//void setLaser(void);

//void feedWater();
void lcdSplash_G2(const char s1[], const char s2[]);

void initZXTMR_G2(void);
void protectedSerialSend_G2(int type, int value);
int ppLaserSessions(int trialsPerSession, int missLimit, int totalSession);

int read_eeprom_G2(int offset);
// void write_eeprom_G2(int offset, int value);



void LCD_set_xy(int x, int y);

void callFunc(int n);
int getFuncNumberGen2(int targetDigits, const char input[]);
void serialSendGen2(unsigned char type,  unsigned char value);
void sendLargeValue(int val);
void lcd_data(char n);
void lcdHomeNClear();

#endif
