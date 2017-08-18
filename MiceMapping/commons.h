#include "Arduino.h"
#include <avr/wdt.h>

#ifndef COMMONS_H
#define  COMMONS_H


#define SpLick    0  //        !PORTDbits.RD8
#define SpCome   1    //Keypad #[ZX] // Head fixed, always come
#define SpLick2  2   // lick2    //!PORTDbits.RD9
#define SpCome2  3    // Head fixed, always come
// the second word is for performance, false alarm,
#define SpFalseAlarm   4  // false alarm
#define SpCorrectRejection     5  // correct rejection
#define SpMiss     6  // miss
#define SpHit    7  // hit
// the third word is for the first set of valves
#define SpWater_sweet  8     // 1    //pwm1L    //same as old definition
#define SpOdor_A   9         // #define  2    //pwm1H   //same as old definition
#define SpOdor_B  10    //      3    //pwm2l    //same as old definition
#define SpOdor_C  11
#define SpOdor_D  12
#define SpOdor_E  13
#define SpOdor_F  14
#define SpOdor_G  15
#define SpOdor_H  16
#define SpOdor_I  17
#define SpOdor_J  18


//press key, s1s1 through s5s5
#define SpS1S1   20 //Trial Wait [ZX]
#define SpS1S2   21 //PermMessage
#define SpS1S3   22 //LickLFreq
#define SpS1S4   23 //Value High
#define SpS1S5   24 //Value Low
#define SpS2S1   25 //LCD_set_xy
#define SpLCD_SET_XY 25
#define SpS2S2   26 //LCDChar
#define SpLCD_Char 26
#define SpS2S3   27
#define SpS2S4   28
#define SpS2S5   29
#define SpS3S1   30
#define SpS3S2   31
#define SpS3S3   32
#define SpS3S4   33
#define SpS3S5   34
#define SpS4S1   35
#define SpS4S2   36
#define SpS4S3   37
#define SpS4S4   38
#define SpS4S5   39
#define SpS5S1   40
#define SpS5S2   41
#define SpS5S3   42
#define SpS5S4   43
#define SpS5S5   44


#define SpStepN         51
#define SpLaser         52
#define SptrialNperSess 49
#define SpSessHit       45
#define SpSessMiss      46
#define SpSessFA      47
#define SpSessCR        48
#define SpSessCorrRate  50

#define SpFirOdor       53
#define SpFirOdorL      54
#define SpOdorDelay     55
#define SpSecOdor       56
#define SpSecOdorL      57
#define Sptrialtype     58
#define SpITI           59  // 1 start 0 end
#define SpproLpun       60
#define SpSess          61  // 1 start 0 end
#define SpTrain         62  // 1 start 0 end
#define Splaser         65


//  #define SpHit2         67
//  #define SpMiss2           68
//  #define SpFalseAlarm2         69
//  #define SpCorrectRejection2         70
//
//  #define SpHit3         71
//  #define SpMiss3          72
//  #define SpFalseAlarm3         73
//  #define SpCorrectRejection3        74

#define SpHit4         75
#define SpMiss4         76
#define SpFalseAlarm4       77
#define SpCorrectRejection4      78
#define SpLaserSwitch    79

#define GNG_Revrsal_Point  67
#define DPA_Revrsal_Point  68
#define FID_value 70


extern unsigned int hit, miss, falseAlarm, correctRejection, correctRatio;
extern int currentMiss;

void Valve_ON(int valve);
void Valve_OFF(int valve);
void callResetGen2(void);


#endif
