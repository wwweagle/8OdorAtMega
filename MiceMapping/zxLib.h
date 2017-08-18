#include <LiquidCrystal.h>

#ifndef ZXLIB_H
#define  ZXLIB_H


//laser trial type

#define laserOff 0

#define laserDuringITI_GNG 1
#define laserDuringOdor_Resp_GNG 2

#define laserDuringDelay_S 11
#define laserDuringEarlyDelay_S 12
#define laserDuringLateDelay_S 13
#define laserDuringResp_S 14
#define laserDuringITI_S 15
#define laserDuring1stOdor_S 16
#define laserDuring2ndOdor_S 17
#define laserDuringTwoOdors_S 18

#define laserDuringDelay_D 21
#define laserDuring2ndDelay_D 22
#define laserDuring3rdDelay_D 23
#define laserDuring1stResp_D 24  // descion making
#define laserDuring2ndResp_D 25  // descion making
#define laserDuring1stOdor_D 26
#define laserDuring2ndOdor_D 27
#define laserDuring3rdOdor_D 28
#define laserDuring4thOdor_D 29

#define trialTypeDnmsSwitch 100
#define trialTypeGoNogoSwitch 110
#define laser4sRamp 121
#define laser2sRamp 122
#define laser1sRamp 123
#define laser_5sRamp 124
#define laserSufficiency 130
#define laserBeforeDistractor 140
#define laserCoverDistractor 145
#define laserAfterDistractor 149


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


// #define LICK_LEFT (PORTDbits.RD12 || !PORTDbits.RD14)
// #define LICK_RIGHT (PORTDbits.RD13 || !PORTDbits.RD15)
// #define LICK_ANY (PORTDbits.RD12 || PORTDbits.RD13 || !PORTDbits.RD14 || !PORTDbits.RD15)
// #define LICKING_LEFT 2
// #define LICKING_RIGHT 3
// #define LICKING_BOTH 127



#define GNG_TRAINING 30
#define DRT_TRAINING 31
#define DNMS_SHAPING 19
#define DNMS_TRAINING 20
#define FOUR_ODOR_DNMS_SHAPING 21
#define FOUR_ODOR_DNMS_TRAINING 22
#define TEN_ODOR_DMS_SHAPING  23
#define TEN_ODOR_DMS_TRAINING   24
#define DPA_TRAINING_NoDelay 88
#define DPA_SHAPING_NoDelay 89
#define DRT_DPA_SHAPING 90
#define DRT_DPA_TRAINING 91
#define DPA_GNG_SHAPING 92
#define DPA_GNG_TRAINING 93
#define DPA_DRT_SHAPING 94  // delay repsonse task
#define DPA_DRT_TRAINING 95
#define DPA_DNMS_SHAPING 96
#define DPA_DNMS_TRAINING 97
#define DPA_SHAPING 98
#define DPA_TRAINING 99
#define DPA_R_SHAPING 100
#define DPA_R_TRAINING 101
#define SIX_ODOR_DPA_SHAPING 102
#define SIX_ODOR_DPA_TRAINING 103


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


// extern LiquidCrystal lcd;

typedef struct {
  unsigned int stim1Length;
  unsigned int stim2Length;
  unsigned int distractorLength;
  unsigned int currentDistractor;
  unsigned int distractorJudgingPair;
} STIM_T;



typedef struct {
  unsigned int timer;
  unsigned int onTime;
  unsigned int offTime;
  unsigned int ramp;
  unsigned int ramping;
  unsigned int on;
  unsigned int side;
} LASER_T;


void zxFunc(int n);
void zxTimer1();
int getFuncNumber(int targetDigits, char* input);
void serialSend(int type, int value);
void wait_ms(int ms);


#endif
