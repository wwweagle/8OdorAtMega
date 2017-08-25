#include "EEPROM.h"
#include "TimerOne.h"
#include "zxLibGen2.h"

#include "commons.h"
// #include "zxLib.h"

volatile int isSending = 0;
volatile int sendLick = 0;
int lickBound_G2 = 400; // Smaller is sensitive
void setWaterLen(void);
void testNSetBound(void);

// DEBUG

STIM_T_G2 stims_G2 = {.stim1Length = 1000u,
                      .stim2Length = 1000u,
                      .respCueLength = 500u,
                      .distractorLength = 500u,
                      .currentDistractor = 7u,
                      .distractorJudgingPair = 8u,
                      .respCueValve = 8u,
                      .shapingExtra = 7u,
                      .falseAlarmPenaltyRepeat = 0};
LASER_T_G2 laser_G2 = {.timer = 0u,
                       .onTime = 65535u,
                       .offTime = 0u,
                       .ramp = 0u,
                       .ramping = 0u,
                       .on = 0u,
                       .side = 1u}; // 1L,2R,3LR
// PWM_T_G2 pwm_G2 = {.L_Hi = 0xfe, .R_Hi = 0xfe, .L_Lo = 0u, .R_Lo = 0u,
// .fullDuty = 0xfe};
LICK_T_G2 lick_G2 = {
    .current = 0u, .filter = 0u, .flag = 0u, .LCount = 0u, .RCount = 0u};
// BALL_T_G2 mball_G2 = {.moving = 0, .steadyCounter = 0, .steadySent = 0};
const char odorTypes_G2[] = " QRLYNHBA0123456789012345678901234567890123456789";
unsigned int laserSessionType_G2 = LASER_EVERY_TRIAL;
unsigned int taskType_G2 = DNMS_TASK;
unsigned int wait_Trial_G2 = 1u;
unsigned long timeSum_G2 = 0u;
int punishFalseAlarm_G2 = 0;
int psedoRanInput_G2;
int abortTrials;

int getFuncNumberGen2(int targetDigits, const char input[]) {
  int bitSet[targetDigits];
  int bitValue[targetDigits];
  unsigned int n;
  int iter;
  int iter1;

  for (iter = 0; iter < targetDigits; iter++) {
    bitSet[iter] = 0;
    bitValue[iter] = -6;
  }

  lcdHomeNClear();
  lcdWriteString_G2(input);

  for (iter = 0; iter < targetDigits; iter++) {
    while (!bitSet[iter]) {
      if (Serial.peek() > 0) {
        if (Serial.peek() == 0x2a) {
          protectedSerialSend_G2(61, 0);
          callResetGen2();
        }
        bitValue[iter] = Serial.read() - 0x30;
        bitSet[iter] = 1;
      }
    }
    protectedSerialSend_G2(SpCome, bitValue[iter]);
    LCD_set_xy(0, 1);
    for (iter1 = 0; iter1 < targetDigits; iter1++) {
      lcd_data(bitValue[iter1] + 0x30);
    }
  }
  n = 0;
  for (iter1 = 0; iter1 < targetDigits; iter1++) {
    n = n * 10 + bitValue[iter1];
  }
  return n;
}


static void turnOnLaser_G2(unsigned int i) {
  laser_G2.timer = 0;
  laser_G2.on = i;

  // digitalWrite(13, HIGH);

  if (i % 2) {
    // Out4 = 1;
    // Nop();
    // Nop();
    // PDC2 = pwm_G2.L_Lo;
  }

  if ((i >> 1) % 2) {
    // Out5 = 1;
    // Nop();
    // Nop();
    // PDC4 = pwm_G2.R_Lo;
  }

  lcdWriteChar_G2('L', 4, 1);
  protectedSerialSend_G2(SpLaserSwitch, i);
}



void POST(void) {
  lcdSplash_G2(__DATE__, __TIME__);
  delay(2000);
  int maxBound = 0;
  int minBound = 999;

  lcdSplash_G2("Mx   Mn   Cur   ", "Wtr     Vlv ");
  lcdWriteNumber_G2(lickBound_G2, 3, 14, 1);
  lcdWriteNumber_G2(WaterLen * 1000.0, 3, 4, 2);
  unsigned long startTime = millis();
  while (millis() < startTime + 5499ul) {
    int sum = 0;
    for (int i = 0; i < 15; i++) {
      sum += analogRead(0);
    }
    int val = sum / 15;
    if (val > maxBound) {
      maxBound = val;
      lcdWriteNumber_G2(maxBound, 3, 3, 1);
    }
    if (val < minBound) {
      minBound = val;
      lcdWriteNumber_G2(minBound, 3, 8, 1);
    }
    sendLargeValue(val);
    delay(500);
  }

  sendLargeValue(lickBound_G2);

  int valves[] = {1, 2, 3, 4, 8,11,12,13,14,18,21};

  for (int i = 0; i < (int) (sizeof(valves) / sizeof(int)); i++) {
    for (int j = 0; j < 8; j++) {
      lcdWriteNumber_G2(valves[i], 2, 13, 2);
      Valve_ON(valves[i]);
      wait_ms_G2(50);
      Valve_OFF(valves[i]);
      wait_ms_G2(200);
    }
  }
  turnOnLaser_G2(3);
  setWaterLen();
}


///////////////////////////////////////////
///////// COPY PASTE //////////////////////
///////////////////////////////////////////

// unsigned long timerCounter32;

unsigned int highLevelShuffleLength_G2 = 12u;

// const _prog_addressT EE_Addr_G2 = 0x7ff000;

static void zxLaserTrial_G2(int type, int firstOdor, STIM_T_G2 odors,
                            _delayT interOdorDelay, int secondOdor,
                            float waterPeroid, unsigned int ITI);
unsigned int getFuncNumberMarquee_G2(int targetDigits, const char input[],
                                     int l);

void initZXTMR_G2(void) {

  Timer1.detachInterrupt();
  Timer1.attachInterrupt(zxTimer1Gen2);

  lick_G2.LCount = 0u;
  lick_G2.RCount = 0u;
}

inline int filtered_G2(void) { return (millis() > lick_G2.filter + 50u); }

void zxTimer1Gen2() {
  if(laser_G2.on){
    digitalWrite(13, HIGH);
  }else{
    digitalWrite(13,LOW);
  }
  if (analogRead(0) < lickBound_G2 && lick_G2.current != LICKING_LEFT &&
      filtered_G2()) { // Low is lick
    lick_G2.filter = millis();
    lick_G2.current = LICKING_LEFT;
    lick_G2.LCount++;
    //  digitalWrite(38, HIGH);
    if (isSending) {
      sendLick = 1;
    } else {
      protectedSerialSend_G2(0, 2);
      sendLick = 0;
    }
  } else if (lick_G2.current && (analogRead(0) >= lickBound_G2)) {
    lick_G2.current = 0;
    //  digitalWrite(38, LOW);
  }
  if (Serial.peek() == 0x2a) {
    protectedSerialSend_G2(61, 0);
    Timer1.detachInterrupt();
    callResetGen2();
  }
}

static void resetTaskTimer_G2() { timeSum_G2 = millis(); }

static void waitTaskTimer_G2(unsigned int dT) {
  timeSum_G2 = timeSum_G2 + (unsigned long)dT;
  while (timeSum_G2 > millis())
    ;
}

static int isLikeOdorA_G2(int odor) {
  if (odor == 1 || odor == 3 || odor == 5 || odor == 7)
    return 1; // B,R,H
  return 0;
}

static int setType_G2(void) {

  int ports[] = {0, 1, 3, 5};
  return ports[getFuncNumberGen2(1, "12 34 56")];
}

static unsigned int setSessionNum_G2() {

  _delayT d[] = {0u, 1u, 5u, 10u, 12u, 15u, 20u, 24u, 30u};
  unsigned int n = getFuncNumberMarquee_G2(
      1,
      "1:1sess 2:5sess 3:10sess 4:12sess 5:15sess 6:20sess 7:24sess 8:30sess ",
      70);
  if (n > 0 && n < sizeof(d))
    return d[n];
  else
    return 5u;
}

static _delayT setDelay_G2() {

  _delayT d[] = {0u, 4u, 5u, 8u, 12u, 13u, 16u, 20u, 30u};
  unsigned int n = getFuncNumberMarquee_G2(
      1, "0:0s 1:4s 2:5s 3:8s 4:12s 5:13s 6:16s 7:20s 8:30s ", 50);
  if (n > 0 && n < sizeof(d))
    return d[n];
  else
    return 5u;
}

static int setLaser_G2() {
  int d[] = {LASER_NO_TRIAL, LASER_NO_TRIAL, LASER_OTHER_TRIAL,
             LASER_EVERY_TRIAL};
  unsigned int n = getFuncNumberGen2(1, "None 1+1-  Every");
  if (n > 0 && n < 4)
    return d[n];
  else
    return LASER_NO_TRIAL;
}

void protectedSerialSend_G2(int type, int value) {
  isSending = 1;
  byte val[] = {(byte)(type & 0x7F), (byte)(value | 0x80)};
  Serial.write(val, 2);
  if (sendLick) {
    val[0] = 0;
    val[1] = 2;
    Serial.write(val, 2);
  }
  sendLick = 0;
  isSending = 0;
}

static void turnOffLaser_G2() {
  laser_G2.on = 0;
  // digitalWrite(13, LOW);
  if (laser_G2.ramping == 0) {
  }
  lcdWriteChar_G2('.', 4, 1);
  protectedSerialSend_G2(SpLaserSwitch, 0);
}

static void assertLaser_G2(int type, int step) {
  switch (type) {
  case laserOff:
    break;
  case laserDuringBeginningToOneSecInITI:
    if (step == atFirstOdorBeginning) {
      turnOnLaser_G2(3);
    } else if (step == atITIOneSecIn) {
      turnOffLaser_G2();
    }
    break;
  case laserDuringDelay:
    if (step == atDelay1SecIn) {
      turnOnLaser_G2(3);
    } else if (step == atDelayLast500mSBegin) {
      turnOffLaser_G2();
    }
    break;
  case laserDuringDelayChR2:
    if (step == atDelay1SecIn) {
      turnOnLaser_G2(3);
    } else if (step == atDelayLastSecBegin) {
      turnOffLaser_G2();
    }
    break;
  case laserDuringDelay_Odor2:
    if (step == atDelay1SecIn) {
      turnOnLaser_G2(3);
    } else if (step == atSecondOdorEnd) {
      turnOffLaser_G2();
    }
    break;
  case laserDuringBaselineNDelay:
    if (step == atDelay1SecIn || step == threeSecBeforeFirstOdor) {
      turnOnLaser_G2(3);
    } else if (step == atDelayLastSecBegin || step == at500mSBeforeFirstOdor) {
      turnOffLaser_G2();
    }
    break;
  case laserDuringOdor:
    if (step == atFirstOdorBeginning || step == atSecondOdorBeginning) {
      turnOnLaser_G2(3);
    } else if (step == atFirstOdorEnd || step == atSecondOdorEnd) {
      turnOffLaser_G2();
    }
    break;
  case laserDuring1stOdor:
    if (step == atFirstOdorBeginning) {
      turnOnLaser_G2(laser_G2.side);
    } else if (step == atFirstOdorEnd) {
      turnOffLaser_G2();
    }
    break;
  case laserDuring2ndOdor:
    if (step == atSecondOdorBeginning) {
      turnOnLaser_G2(3);
    } else if (step == atSecondOdorEnd) {
      turnOffLaser_G2();
    }
    break;
  case laserDuringEarlyHalf:
    if (step == atDelayBegin) {
      turnOnLaser_G2(3);
    } else if (step == atDelayMiddle) {
      turnOffLaser_G2();
    }
    break;

  case laserDuringLateHalf:
    if (step == atDelayMiddle) {
      turnOnLaser_G2(3);
    } else if (step == atDelayLast500mSBegin) {
      turnOffLaser_G2();
    }
    break;

  case laserDuring1Quarter:
    if (step == atDelayBegin) {
      turnOnLaser_G2(laser_G2.side);
    } else if (step == atDelay1_5SecIn) {
      turnOffLaser_G2();
    }
    break;
  case laserDuring2Quarter:
    if (step == atDelay2SecIn) {
      turnOnLaser_G2(laser_G2.side);
    } else if (step == atDelay500msToMiddle) {
      turnOffLaser_G2();
    }
    break;
  case laserDuring3Quarter:
    if (step == atDelayMiddle) {
      turnOnLaser_G2(laser_G2.side);
    } else if (step == atDelayLast2_5SecBegin) {
      turnOffLaser_G2();
    }
    break;
  case laserDuring4Quarter:
    if (step == atDelayLast2SecBegin) {
      turnOnLaser_G2(laser_G2.side);
    } else if (step == atDelayLast500mSBegin) {
      turnOffLaser_G2();
    }
    break;
  case laserDuring12s1Quarter:
    if (step == atDelayBegin) {
      turnOnLaser_G2(laser_G2.side);
    } else if (step == atDelay2_5SecIn) {
      turnOffLaser_G2();
    }
    break;
  case laserDuring12s2Quarter:
    if (step == atDelay3SecIn) {
      turnOnLaser_G2(laser_G2.side);
    } else if (step == atDelay500msToMiddle) {
      turnOffLaser_G2();
    }
    break;
  case laserDuring12s3Quarter:
    if (step == atDelayMiddle) {
      turnOnLaser_G2(laser_G2.side);
    } else if (step == atDelayMid2_5Sec) {
      turnOffLaser_G2();
    }
    break;
  case laserDuring12s4Quarter:
    if (step == atDelayMid3Sec) {
      turnOnLaser_G2(laser_G2.side);
    } else if (step == atDelayLast500mSBegin) {
      turnOffLaser_G2();
    }
    break;
  case laserDuringResponseDelay:
    if (step == atSecondOdorEnd) {
      turnOnLaser_G2(3);
    } else if (step == atRewardBeginning) {
      turnOffLaser_G2();
    }
    break;
  case laserNoDelayControl:
    if (step == oneSecBeforeFirstOdor) {
      turnOnLaser_G2(3);
    } else if (step == atRewardBeginning) {
      turnOffLaser_G2();
    }
    break;
  case laserNoDelayControlShort:
    if (step == atFirstOdorBeginning) {
      turnOnLaser_G2(3);
    } else if (step == atSecondOdorEnd) {
      turnOffLaser_G2();
    }
    break;
  case laserDuringBaseline:
    if (step == threeSecBeforeFirstOdor) {
      turnOnLaser_G2(3);
    } else if (step == atFirstOdorBeginning) {
      turnOffLaser_G2();
    }
    break;
  case laserDuring3Baseline:
    if (step == fourSecBeforeFirstOdor) {
      turnOnLaser_G2(3);
    } else if (step == oneSecBeforeFirstOdor) {
      turnOffLaser_G2();
    }
    break;
  case laserDuring4Baseline:
    if (step == fourSecBeforeFirstOdor) {
      turnOnLaser_G2(3);
    } else if (step == at500mSBeforeFirstOdor) {
      turnOffLaser_G2();
    }
    break;
  case laserDuringBaseAndResponse:
    if (step == oneSecBeforeFirstOdor || step == atSecondOdorEnd) {
      turnOnLaser_G2(3);
    } else if (step == at500mSBeforeFirstOdor || step == atRewardBeginning) {

      turnOffLaser_G2();
    }
    break;
  case laser4sRamp:
    if (step == atDelay_5SecIn) {
      turnOnLaser_G2(3);
    } else if (step == atDelayLast500mSBegin) {
      turnOffLaser_G2();
    }
    break;
  case laser2sRamp:
    if (step == atDelayLast2_5SecBegin) {
      turnOnLaser_G2(3);
    } else if (step == atDelayLast500mSBegin) {
      turnOffLaser_G2();
    }
    break;
  case laser1sRamp:
    if (step == atDelayLast1_5SecBegin) {
      turnOnLaser_G2(3);
    } else if (step == atDelayLast500mSBegin) {
      turnOffLaser_G2();
    }
    break;
  case laser_5sRamp:
    if (step == atDelayLastSecBegin) {
      turnOnLaser_G2(3);
    } else if (step == atDelayLast500mSBegin) {
      turnOffLaser_G2();
    }
    break;
  //        case laserDelayDistractorEarly:
  //            if (step == atDelay2SecIn) {
  //                turnOnLaser(3);
  //            } else if (step == atDelay2SecIn) {
  //                turnOffLaser();
  //            }
  //            break;
  //        case laserDelayDistractorLate:
  //            if (step == atDelay1SecIn) {
  //                turnOnLaser(3);
  //            } else if (step == atDelay2SecIn) {
  //                turnOffLaser();
  //            }
  //            break;
  case laserRampDuringDelay:
    if (step == atDelay1SecIn) {
      turnOnLaser_G2(laser_G2.side);
    } else if (step == atDelayLast500mSBegin) {
      turnOffLaser_G2();
    }
    break;
  //        case laserAfterDistractor:
  //            if (step == atDelayMid2_5Sec) {
  //                turnOnLaser(laser.side);
  //            } else if (step == atDelayLast2_5SecBegin) {
  //                turnOffLaser();
  //            }
  //            break;
  //        case laserAfterDistractorLong:
  //            if (step == atDelayMid2_5Sec) {
  //                turnOnLaser(laser.side);
  //            } else if (step == atDelayLast500mSBegin) {
  //                turnOffLaser();
  //            }
  //            break;
  case laserDuring1Terice:
    if (step == atFirstOdorEnd) {
      turnOnLaser_G2(laser_G2.side);
    } else if (step == atDelay3_5SIn) {
      turnOffLaser_G2();
    }
    break;
  case laserDuring2Terice:
    if (step == atDelay4_5SIn) {
      turnOnLaser_G2(laser_G2.side);
    } else if (step == atDelayMid1_5Sec) {
      turnOffLaser_G2();
    }
    break;
  case laserDuring3Terice:
    if (step == atDelayMid2_5Sec) {
      turnOnLaser_G2(laser_G2.side);
    } else if (step == atDelayLast500mSBegin) {
      turnOffLaser_G2();
    }
    break;
  case laserCoverDistractor:
    if (step == atPreDualTask) {
      turnOnLaser_G2(laser_G2.side);
    } else if (step == atPostDualTask) {
      turnOffLaser_G2();
    }
    break;

  case laserAfterDistractorMax:
    if (step == atPostDualTask) {
      turnOnLaser_G2(laser_G2.side);
    } else if (step == atDelayLast500mSBegin) {
      turnOffLaser_G2();
    }
    break;

  case laserAfterMultiDistractor:
    if (step == atPostDualTask) {
      turnOnLaser_G2(laser_G2.side);
    } else if (step == atPreDualTask) {
      turnOffLaser_G2();
    }
    break;
  }
}

void shuffleArray_G2(unsigned int *orgArray, unsigned int arraySize) {
  if (arraySize == 0 || arraySize == 1)
    return;

  unsigned int iter;
  for (iter = 0; iter < arraySize; iter++) {
    orgArray[iter] = iter;
  }
  int index, temp;
  for (iter = arraySize - 1; iter > 0; iter--) {

    index = rand() % (iter + 1);
    temp = orgArray[index];
    orgArray[index] = orgArray[iter];
    orgArray[iter] = temp;
  }
}

void lcdWriteString_G2(const char s[]) {

  int iter;
  for (iter = 0; s[iter] && iter < 16; iter++) {
    lcd_data(s[iter]);
  }
}

// int lcdMarqueeString_G2(const char s[], int l) {
//   // static unsigned int last;
//   // int startIdx = (timerCounterJ >> 9) % l;
//   // if (startIdx != last) {
//   //     last = startIdx;
//   //     lcdHomeNClear();
//   //     int i = 0;
//   //     for (; i < 16; i++) {
//   //         while (l - 1 < i + startIdx) {
//   //             startIdx -= l;
//   //         }
//   //         lcd_data(s[i + startIdx]);
//   //     }
//   //     return 1;
//   // }
//   // return 0;
// }
void lcd_data(char n) { protectedSerialSend_G2(SpLCD_Char, (int)n); }

void lcdWriteNumber_G2(int n, int digits, int x, int y) {

  if (digits < 1 || n < 0) {
    return;
  }
  LCD_set_xy(x - 1, y - 1);
  int tenK = n / 10000;
  int K = (n % 10000) / 1000;
  int H = (n % 1000) / 100;
  int ten = (n % 100) / 10;
  int one = n % 10;

  switch (digits) {
  case 5:
    lcd_data(tenK + 0x30);
  case 4:
    lcd_data(K + 0x30);
  case 3:
    lcd_data(H + 0x30);
  case 2:
    lcd_data(ten + 0x30);
  }
  lcd_data(one + 0x30);
}

void lcdWriteChar_G2(char ch, int x, int y) {
  LCD_set_xy(x - 1, y - 1);
  protectedSerialSend_G2(SpLCD_Char, ch);
}

void lcdHomeNClear() { protectedSerialSend_G2(SpLCD_SET_XY, 0x20); }

void LCD_set_xy(int x, int y) {
  int val = (((x & 0x0f) << 1) | (y & 0x01)) & 0x1F;
  // protectedSerialSend_G2(1, val);
  protectedSerialSend_G2(SpLCD_SET_XY, val); // 3bit cmd, 5bit value
}

unsigned int getFuncNumberMarquee_G2(int targetDigits, const char input[],
                                     int l) {

  return getFuncNumberGen2(targetDigits, input);
  //
  // #ifdef DEBUG
  //     int n = 0;
  //     return n;
  // #else
  //     int bitSet[targetDigits];
  //     int bitValue[targetDigits];
  //     unsigned int n;
  //     int iter;
  //     int iter1;
  //
  //     for (iter = 0; iter < targetDigits; iter++) {
  //         bitSet[iter] = 0;
  //         bitValue[iter] = -6;
  //     }
  //
  //     lcdHomeNClear();
  //     for (iter = 0; iter < targetDigits; iter++) {
  //         while (!bitSet[iter]) {
  //             if (lcdMarqueeString_G2(input, l)) {
  //                 line_2();
  //                 for (iter1 = 0; iter1 < targetDigits; iter1++) {
  //                     lcd_data(bitValue[iter1] + 0x30);
  //                 }
  //             }
  //             if (u2Received > 0) {
  //                 bitValue[iter] = u2Received - 0x30;
  //                 bitSet[iter] = 1;
  //                 u2Received = -1;
  //             } else {
  //                 Key_Event();
  //                 if (hardwareKey) {
  //                     hardwareKey = 0;
  //                     bitValue[iter] = key_val;
  //                     bitSet[iter] = 1;
  //                 }
  //             }
  //         }
  //         localSendOnce(SpCome, bitValue[iter]);
  //         safe_wait_ms(300);
  //     }
  //     n = 0;
  //     for (iter1 = 0; iter1 < targetDigits; iter1++) {
  //         n = n * 10 + bitValue[iter1];
  //     }
  //     return n;
  // #endif
}

static void processHit_G2(float waterPeroid, int valve, int id) {
  protectedSerialSend_G2(22, 1);
  if (valve == 1 || valve == 2) {
    Valve_ON(water_sweet);
  }
  wait_ms_G2(waterPeroid * 1000);
  Valve_OFF(water_sweet);
  currentMiss = 0;
  protectedSerialSend_G2(SpHit, id);
  lcdWriteNumber_G2(++hit, 3, 6, 1);
}

static void processFalse_G2(int id) {
  currentMiss = 0;
  protectedSerialSend_G2(SpFalseAlarm, id);
  lcdWriteNumber_G2(++falseAlarm, 3, 6, 2);
}

static void processMiss_G2(int id) {
  currentMiss++;
  protectedSerialSend_G2(SpMiss, id);
  lcdWriteNumber_G2(++miss, 3, 10, 1);
}

static void processLRTeaching_G2(float waterPeroid, int LR) {
  int rr = rand() % 3;
  if (rr == 0) {
    lcdWriteChar_G2(rr + 0x30, 4, 1);
    protectedSerialSend_G2(22, LR == 2 ? 1 : 2);
    //        FreqL = lick.LCount;
    Valve_ON(LR == 2 ? 1 : 3);
    wait_ms_G2(waterPeroid * 1000);
    Valve_OFF(LR == 2 ? 1 : 3);
    //        currentMiss = 0;
    protectedSerialSend_G2(SpWater_sweet, LR);
  }
}

static int waterNResult_G2(int firstOdor, int secondOdor, float waterPeroid,
                           int id) {
  int rtn = 0;
  int rewardWindow = (taskType_G2 == ODPA_RD_SHAPING_A_CATCH_LASER_TASK ||
                      taskType_G2 == ODPA_RD_SHAPING_B_CATCH_LASER_TASK ||
                      taskType_G2 == ODPA_RD_CATCH_LASER_TASK)
                         ? 1000
                         : 500;
  if (taskType_G2 == ODPA_RD_SHAPING_B_CATCH_LASER_TASK &&
      (unsigned int)firstOdor == stims_G2.shapingExtra) {
    firstOdor = secondOdor;
  }

  lick_G2.flag = 0;
  switch (taskType_G2) {

  /*
   *DNMS-LR
   */
  case DNMS_2AFC_TASK:
    //
    ///////////Detect/////////////////
    for (unsigned long startTime = millis();
         millis() < startTime + 500u &&
         !(lick_G2.flag == 2 || lick_G2.flag == 3);
         lick_G2.flag = lick_G2.current)
      ;
    /////Reward
    if (!lick_G2.flag) {
      processMiss_G2((firstOdor != secondOdor) ? 2 : 3);
    } else if (!(lick_G2.flag & 1) != !(firstOdor ^ secondOdor)) {
      processHit_G2(waterPeroid, lick_G2.flag & 1 ? 3 : 1, lick_G2.flag);
    } else {
      processFalse_G2(lick_G2.flag == LICKING_LEFT ? 2 : 3);
    }
    break;

  case DNMS_2AFC_TEACH:
    //
    ///////////Detect/////////////////
    for (unsigned long startTime = millis();
         millis() < startTime + 500u &&
         !(lick_G2.flag == 2 || lick_G2.flag == 3);
         lick_G2.flag = lick_G2.current)
      ;
    /////Reward
    if (!lick_G2.flag) {
      processMiss_G2((firstOdor != secondOdor) ? 2 : 3);
      processLRTeaching_G2(waterPeroid, (firstOdor != secondOdor) ? 2 : 3);
    } else if (!(lick_G2.flag & 1) != !(firstOdor ^ secondOdor)) {
      processHit_G2(waterPeroid, lick_G2.flag & 1 ? 3 : 1, lick_G2.flag);
    } else {
      processFalse_G2(lick_G2.flag == LICKING_LEFT ? 2 : 3);
      processLRTeaching_G2(waterPeroid, (firstOdor != secondOdor) ? 2 : 3);
    }
    break;

  case GONOGO_LR_TASK:
  case GONOGO_2AFC_TEACH:
    //
    ///////////Detect/////////////////
    for (unsigned long startTime = millis();
         millis() < startTime + 500u &&
         !(lick_G2.flag == 2 || lick_G2.flag == 3);
         lick_G2.flag = lick_G2.current)
      ;

    /////Reward
    if (lick_G2.flag == LICKING_LEFT && isLikeOdorA_G2(firstOdor)) {
      processHit_G2(waterPeroid, 1, 2);
    } else if (lick_G2.flag == LICKING_RIGHT && !isLikeOdorA_G2(firstOdor)) {
      processHit_G2(waterPeroid, 3, 3);
    } else if (lick_G2.flag) {
      processFalse_G2(lick_G2.flag == LICKING_LEFT ? 2 : 3);
    } else {
      processMiss_G2(isLikeOdorA_G2(firstOdor) ? 2 : 3);
    }
    break;

  case GONOGO_TASK:

    for (unsigned long startTime = millis();
         millis() < startTime + 500u && !lick_G2.flag;
         lick_G2.flag = lick_G2.current)
      ;

    /////Reward
    if (!lick_G2.flag) {
      if (!isLikeOdorA_G2(firstOdor)) {
        protectedSerialSend_G2(SpCorrectRejection, 1);
        lcdWriteNumber_G2(++correctRejection, 3, 10, 2);
      } else {
        processMiss_G2(1);
      }
    } else if (!isLikeOdorA_G2(firstOdor)) {
      processFalse_G2(1);
    } else {
      processHit_G2(waterPeroid, 1, 1);
    }
    break;

  default:
    /*///////////////
             *DNMS
             */ /////////////
    //        case DNMS_TASK:
    //        case SHAPING_TASK:
    //        case OPTO_ODPA_TASK:
    //        case OPTO_ODPA_SHAPING_TASK:
    //        case NO_ODOR_CATCH_TRIAL_TASK:
    //        case VARY_ODOR_LENGTH_TASK:
    //        case DELAY_DISTRACTOR:

    //        case _ASSOCIATE_TASK:
    //        case _ASSOCIATE_SHAPING_TASK:
    //

    ///////////Detect/////////////////
    for (unsigned long startTime = millis();
         millis() < startTime + rewardWindow && !lick_G2.flag;
         lick_G2.flag = lick_G2.current)
      ;

    /////Reward
    if (!lick_G2.flag) {
      if (isLikeOdorA_G2(firstOdor) == isLikeOdorA_G2(secondOdor)) {
        protectedSerialSend_G2(SpCorrectRejection, id);
        lcdWriteNumber_G2(++correctRejection, 3, 10, 2);
      } else {
        processMiss_G2(id);
        if ((taskType_G2 == SHAPING_TASK || taskType_G2 == ODPA_SHAPING_TASK ||
             taskType_G2 == ODPA_SHAPING_BALL_TASK ||
             taskType_G2 == DUAL_TASK_LEARNING ||
             taskType_G2 == DNMS_DUAL_TASK_LEARNING ||
             taskType_G2 == ODPA_RD_SHAPING_A_CATCH_LASER_TASK ||
             taskType_G2 == ODPA_RD_SHAPING_NO_LASER_TASK ||
             taskType_G2 == ODPA_RD_SHAPING_B_CATCH_LASER_TASK) &&
            ((rand() % 3) == 0)) {
          protectedSerialSend_G2(22, 1);
          Valve_ON(water_sweet);
          protectedSerialSend_G2(SpWater_sweet, 1);
          wait_ms_G2(waterPeroid * 1000);
          Valve_OFF(water_sweet);
        }
      }
    } else if (isLikeOdorA_G2(firstOdor) == isLikeOdorA_G2(secondOdor)) {
      processFalse_G2(id);
      rtn = SpFalseAlarm;
    } else {
      processHit_G2(waterPeroid, 1, id);
    }
    break;
  }
  return rtn;
}

static void distractor_G2(unsigned int distractOdor, unsigned int judgingPair,
                          float waterLen) {
  if (distractOdor == 0) {
    wait_ms_G2(1500u);
  } else {
    Valve_ON(distractOdor);
    // if (isLikeOdorA_G2(distractOdor)) Out2 = 1;
    // else Out3 = 1;
    protectedSerialSend_G2(isLikeOdorA_G2(distractOdor) ? SpOdor_C : SpOdor_D,
                           distractOdor);
    lcdWriteChar_G2(isLikeOdorA_G2(distractOdor) ? '.' : ':', 4, 1);
    wait_ms_G2(stims_G2.distractorLength - 10u);
    protectedSerialSend_G2(isLikeOdorA_G2(distractOdor) ? SpOdor_C : SpOdor_D,
                           0);
    Valve_OFF(distractOdor);
    // Out2 = 0;
    // Nop();
    // Nop();
    // Out3 = 0;
    // Nop();
    // Nop();
    lcdWriteChar_G2('D', 4, 1);
    wait_ms_G2(450u);
    waterNResult_G2(distractOdor, judgingPair, waterLen, 3);
  }
}

static void waitTrial_G2() {
  static int waitingLickRelease = 0;
  if (!wait_Trial_G2) {
    return;
  }
  while (lick_G2.current) {
    if (!waitingLickRelease) {
      protectedSerialSend_G2(20, 100);
      waitingLickRelease = 1;
    }
  }
  waitingLickRelease = 0;

  while (Serial.peek() != 0x31) {
    protectedSerialSend_G2(20, 1);
    delay(50);
  }
  while (Serial.available() > 0) {
    Serial.read();
  }
  // u2Received = -1;
}

static void zxLaserSessions_G2(int stim1, int stim2, int laserTrialType,
                               _delayT delay, unsigned int ITI,
                               int trialsPerSession, float WaterLen,
                               int missLimit, unsigned int totalSession) {

  //    wait_ms(1000);
  int currentTrial = 0;
  unsigned int currentSession = 0;
  int laserOnType = laserTrialType;

  // protectedSerialSend_G2(SpWater_bitter, pwm_G2.L_Lo);
  // protectedSerialSend_G2(SpValve8, pwm_G2.R_Lo);
  protectedSerialSend_G2(SpStepN, taskType_G2);

  while ((currentMiss < missLimit) && (currentSession++ < totalSession)) {
    //        protectedSerialSend(SpOdorDelay, delay);
    protectedSerialSend_G2(SpSess, 1);

    lcdSplash_G2("    H___M___ __%", "S__ F___C___A___");

    lcdWriteNumber_G2(currentSession, 2, 2, 2);
    hit = miss = falseAlarm = correctRejection =abortTrials= 0;
    unsigned int lastHit = 0;
    unsigned int shuffledList[4];
    unsigned int shuffledLongList[highLevelShuffleLength_G2];
    shuffleArray_G2(shuffledLongList, highLevelShuffleLength_G2);
    int firstOdor, secondOdor;
    int lastOdor1;
    int lastOdor2;
    for (currentTrial = 0;
         currentTrial < trialsPerSession && currentMiss < missLimit;) {
      shuffleArray_G2(shuffledList, 4);
      int iterOf4;
      for (iterOf4 = 0; iterOf4 < 4 && currentMiss < missLimit; iterOf4++) {
        //                wait_ms(1000);
        int index = shuffledList[iterOf4];
        switch (taskType_G2) {

        case DNMS_TASK:
          firstOdor = (index == 0 || index == 2) ? stim1 : stim2;
          secondOdor = (index == 1 || index == 2) ? stim1 : stim2;
          break;
        case SHAPING_TASK:
          firstOdor = (index == 0 || index == 2) ? stim1 : stim2;
          secondOdor = (firstOdor == stim1) ? stim2 : stim1;
          break;
        case GONOGO_TASK:
        case GONOGO_LR_TASK:
          firstOdor = (index == 0 || index == 2) ? stim1 : stim2;
          secondOdor = 0;
          break;
        case DNMS_2AFC_TASK:
        case DNMS_2AFC_TEACH:
          if (hit > lastHit || currentTrial == 0) {
            firstOdor = (index == 0 || index == 2) ? stim1 : stim2;
            secondOdor = (index == 1 || index == 2) ? stim1 : stim2;
            lastOdor1 = firstOdor;
            lastOdor2 = secondOdor;
          } else {
            firstOdor = lastOdor1;
            secondOdor = lastOdor2;
          }

          lastHit = hit;
          break;
        case GONOGO_2AFC_TEACH:
          if (hit > lastHit || currentTrial == 0) {
            firstOdor = (index == 0 || index == 2) ? stim1 : stim2;
            lastOdor1 = firstOdor;
            secondOdor = 0;
          } else {
            firstOdor = lastOdor1;
            secondOdor = 0;
          }

          lastHit = hit;
          break;
        case NO_ODOR_CATCH_TRIAL_TASK:
          firstOdor = (index == 0 || index == 2) ? stim1 : stim2;
          secondOdor = (index == 1 || index == 2) ? stim1 : stim2;

          break;

        case VARY_ODOR_LENGTH_TASK: {
          static int varyLengths[] = {250, 500, 750, 1000};
          firstOdor = (index == 0 || index == 2) ? stim1 : stim2;
          secondOdor = (index == 1 || index == 2) ? stim1 : stim2;
          unsigned int idxO1 = shuffledLongList[currentTrial] & 0x03;
          unsigned int idxO2 = shuffledLongList[currentTrial] >> 2;
          stims_G2.stim1Length = varyLengths[idxO1];
          stims_G2.stim2Length = varyLengths[idxO2];
          //                        lcdWriteNumber(idxO1,2,13,2);
          //                        lcdWriteNumber(idxO2,2,15,2);
          //                        lcdWriteNumber(odors.odor1Length,4,1,2);
          //                        lcdWriteNumber(odors.odor2Length,4,5,2);

          break;
        }
        case OPTO_ODPA_TASK:
          firstOdor = (index == 0 || index == 2) ? 9 : 10;
          secondOdor = (index == 1 || index == 2) ? stim1 : stim2;
          break;

        case OPTO_ODPA_SHAPING_TASK:
          firstOdor = (index == 0 || index == 2) ? 9 : 10;
          secondOdor = (firstOdor == 9) ? stim2 : stim1;
          break;
        case ODPA_TASK:
        case ODPA_BALL_TASK:
          firstOdor = (index == 0 || index == 2) ? stim1 : (stim1 + 1);
          secondOdor = (index == 1 || index == 2) ? stim2 : (stim2 + 1);
          break;

        case ODPA_SHAPING_TASK:
        case ODPA_SHAPING_BALL_TASK:
          firstOdor = (index == 0 || index == 2) ? stim1 : (stim1 + 1);
          secondOdor = (firstOdor == stim1) ? (stim2 + 1) : stim2;
          break;
        case ODPA_RD_SHAPING_NO_LASER_TASK:
        case ODPA_RD_SHAPING_A_CATCH_LASER_TASK:
          firstOdor = (index == 0 || index == 2) ? stim1 : (stim1 + 1);
          secondOdor = (firstOdor == stim1) ? (stim2 + 1) : stim2;
          if (currentTrial > 15 &&
              taskType_G2 == ODPA_RD_SHAPING_A_CATCH_LASER_TASK) {
            laserTrialType = laserDuringDelayChR2;
          } else {
            laserTrialType = laserOff;
          }
          break;
        case ODPA_RD_SHAPING_B_CATCH_LASER_TASK:
          switch (index) {
          case 0:
            firstOdor = stim1;
            secondOdor = stim2 + 1;
            break;
          case 1:
            firstOdor = stim1 + 1;
            secondOdor = stim2;
            break;
          case 2:
            firstOdor = stims_G2.shapingExtra;
            secondOdor = stim1;
            break;
          case 3:
            firstOdor = stims_G2.shapingExtra;
            secondOdor = stim2;
            break;
          }
          if (currentTrial > 15) {
            laserTrialType = laserDuringDelayChR2;
          } else {
            laserTrialType = laserOff;
          }
          break;
        case ODPA_RD_CATCH_LASER_TASK:
          if (!stims_G2.falseAlarmPenaltyRepeat) {
            firstOdor = (index == 0 || index == 2) ? stim1 : (stim1 + 1);
            secondOdor = (index == 1 || index == 2) ? stim2 : (stim2 + 1);
          }
          if (currentTrial > 15) {
            laserTrialType = laserDuringDelayChR2;
          } else {
            laserTrialType = laserOff;
          }
          break;

        case DUAL_TASK_LEARNING:
        case DUAL_TASK:
          firstOdor = (index == 0 || index == 2) ? stim1 : (stim1 + 1);
          secondOdor = (index == 1 || index == 2) ? stim2 : (stim2 + 1);
          switch (shuffledLongList[currentTrial] % 3) {
          case 0:
            stims_G2.currentDistractor = 0u;
            break;
          case 1:
            stims_G2.currentDistractor = 7u;
            break;
          case 2:
            stims_G2.currentDistractor = 8u;
            break;
          }
          break;
        case DUAL_TASK_EVERY_TRIAL:
          firstOdor = (index == 0 || index == 2) ? stim1 : (stim1 + 1);
          secondOdor = (index == 1 || index == 2) ? stim2 : (stim2 + 1);
          if (shuffledLongList[currentTrial] % 2)
            stims_G2.currentDistractor = 7u;
          else
            stims_G2.currentDistractor = 8u;
          break;

        case DUAL_TASK_ON_OFF_LASER_TASK:
          firstOdor = (index == 0 || index == 2) ? stim1 : (stim1 + 1);
          secondOdor = (index == 1 || index == 2) ? stim2 : (stim2 + 1);
          switch (shuffledLongList[currentTrial] % 8) {
          case 0:
          case 1:
          case 2:
          case 3:
            stims_G2.currentDistractor = 0u;
            break;
          case 4:
          case 6:
            stims_G2.currentDistractor = 7u;
            break;
          case 5:
          case 7:
            stims_G2.currentDistractor = 8u;
            break;
          }
          break;

        case DUAL_TASK_ODAP_ON_OFF_LASER_TASK:
          stims_G2.currentDistractor = (index % 2) ? 7u : 8u;
          switch (shuffledLongList[currentTrial] % 8) {
          case 0:
          case 1:
          case 2:
          case 3:
            firstOdor = secondOdor = 20u;
            break;
          case 4:
            firstOdor = stim1;
            secondOdor = stim2;
            break;
          case 5:
            firstOdor = stim1;
            secondOdor = stim2 + 1;
            break;
          case 6:
            firstOdor = stim1 + 1;
            secondOdor = stim2;
            break;
          case 7:
            firstOdor = stim1 + 1;
            secondOdor = stim2 + 1;
            break;
          }
          break;
        case DNMS_DUAL_TASK_LEARNING:
        case DNMS_DUAL_TASK:
          firstOdor = (index == 0 || index == 2) ? stim1 : stim2;
          secondOdor = (index == 1 || index == 2) ? stim1 : stim2;
          switch (shuffledLongList[currentTrial] % 3) {
          case 0:
            stims_G2.currentDistractor = 0u;
            break;
          case 1:
            stims_G2.currentDistractor = 7u;
            break;
          case 2:
            stims_G2.currentDistractor = 8u;
            break;
          }
          break;
        }

        lcdWriteChar_G2(odorTypes_G2[firstOdor], 1, 1);
        lcdWriteChar_G2(odorTypes_G2[secondOdor], 2, 1);

        //                int laserCurrentTrial;

        switch (laserSessionType_G2) {
        case LASER_NO_TRIAL:
          laserTrialType = laserOff;
          break;
        case LASER_EVERY_TRIAL:
          break;
        case LASER_OTHER_TRIAL:
          laserTrialType = (currentTrial % 2) == 0 ? laserOff : laserOnType;
          break;

        case LASER_LR_EACH_QUARTER:
          laser_G2.side = isLikeOdorA_G2(firstOdor) ? 1 : 2;
        case LASER_EACH_QUARTER:
          switch (currentTrial % 5) {
          case 0:
            laserTrialType = laserOff;
            break;
          case 1:
            laserTrialType = laserDuring1Quarter;
            break;
          case 2:
            laserTrialType = laserDuring2Quarter;
            break;
          case 3:
            laserTrialType = laserDuring3Quarter;
            break;
          case 4:
            laserTrialType = laserDuring4Quarter;
            break;
          }
          break;

        case LASER_12s_LR_EACH_QUARTER:
          laser_G2.side = isLikeOdorA_G2(firstOdor) ? 1 : 2;
        case LASER_12s_EACH_QUARTER:
          switch (currentTrial % 5) {
          case 0:
            laserTrialType = laserOff;
            break;
          case 1:
            laserTrialType = laserDuring12s1Quarter;
            break;
          case 2:
            laserTrialType = laserDuring12s2Quarter;
            break;
          case 3:
            laserTrialType = laserDuring12s3Quarter;
            break;
          case 4:
            laserTrialType = laserDuring12s4Quarter;
            break;
          }
          break;

        case LASER_VARY_LENGTH:
          switch (currentTrial % 5) {
          case 0:
            laserTrialType = laserOff;
            break;
          case 1:
            laserTrialType = laser4sRamp;
            break;
          case 2:
            laserTrialType = laser2sRamp;
            break;
          case 3:
            laserTrialType = laser1sRamp;
            break;
          case 4:
            laserTrialType = laser_5sRamp;
            break;
          }
          break;

        case LASER_LR_EVERYTRIAL:
          laser_G2.side = isLikeOdorA_G2(firstOdor) ? 1 : 2;
          break;

        case LASER_LR_EVERY_OTHER_TRIAL:
          laser_G2.side = isLikeOdorA_G2(firstOdor) ? 1 : 2;

          laserTrialType = (currentTrial % 2) == 0 ? laserOff : laserOnType;
          break;

        case LASER_INCONGRUENT_CATCH_TRIAL:
          if ((currentTrial > 3 && currentTrial < 8 &&
               isLikeOdorA_G2(firstOdor) && isLikeOdorA_G2(secondOdor)) ||
              (currentTrial > 7 && currentTrial < 12 &&
               isLikeOdorA_G2(firstOdor) && !isLikeOdorA_G2(secondOdor)) ||
              (currentTrial > 11 && currentTrial < 16 &&
               !isLikeOdorA_G2(firstOdor) && isLikeOdorA_G2(secondOdor)) ||
              (currentTrial > 15 && currentTrial < 20 &&
               !isLikeOdorA_G2(firstOdor) && !isLikeOdorA_G2(secondOdor))) {
            laser_G2.side = isLikeOdorA_G2(firstOdor) ? 2 : 1;
          } else {
            laser_G2.side = isLikeOdorA_G2(firstOdor) ? 1 : 2;
          }
          break;
        case LASER_13s_EarlyMidLate:
          switch (currentTrial % 4) {
          case 0:
            laserTrialType = laserOff;
            break;
          case 1:
            laserTrialType = laserDuring1Terice;
            break;
          case 2:
            laserTrialType = laserDuring2Terice;
            break;
          case 3:
            laserTrialType = laserDuring3Terice;
            break;
          }
          break;
        case LASER_DUAL_TASK_ON_OFF:
          switch (shuffledLongList[currentTrial] % 8) {
          case 0:
          case 1:
          case 4:
          case 5:
            laserTrialType = laserOff;
            break;
          case 2:
          case 3:
          case 6:
          case 7:
            laserTrialType = laserCoverDistractor;
            break;
          }
          break;
        case LASER_DUAL_TASK_ODAP_ON_OFF:
          laserTrialType = (index < 2) ? laserOff : laserCoverDistractor;
          break;
        case LASER_OTHER_BLOCK:
          if (psedoRanInput_G2)
            laserTrialType =
                currentTrial < (trialsPerSession / 2) ? laserOff : laserOnType;
          else
            laserTrialType =
                currentTrial < (trialsPerSession / 2) ? laserOnType : laserOff;
          break;
        }
        zxLaserTrial_G2(laserTrialType, firstOdor, stims_G2, delay, secondOdor,
                        WaterLen, ITI);
        currentTrial++;
      }
    }
    protectedSerialSend_G2(SpSess, 0);
  }
  protectedSerialSend_G2(SpTrain, 0); // send it's the end
  while (Serial.available() > 0) {
    Serial.read();
  }
}

// static void optoStim(int stim, int length, int place) {
//    int stimSend = likeOdorA(stim) ? 9 : 10;
//    protectedSerialSend(stimSend, stim);
//    lcdWriteChar(place == 1 ? '1' : '2', 4, 1);
//   i switch (stim) {
//        case 21:
//        {
//            int timePassed = 0;
//            while (timePassed < length) {
//                Out3 = 1;
//                wait_ms(85); //Uchida N.NS 2013
//                Out3 = 0;
//                wait_ms(85);
//                Out3 = 1;
//                wait_ms(85); //Uchida N.NS 2013
//                Out3 = 0;
//                wait_ms(245);
//                timePassed += 500;
//            }
//            break;
//        }
//        case 22:
//        {
//            int timePassed = 0;
//            while (timePassed < length) {
//                Out3 = 1;
//                wait_ms(85); //Uchida N.NS 2013
//                Out3 = 0;
//                wait_ms(165);
//                timePassed += 250;
//            }
//            break;
//        }
//        case 23:
//        {
//            int timePassed = 0;
//            while (timePassed < length) {
//                Out3 = 1;
//                wait_ms(170); //Uchida N.NS 2013
//                Out3 = 0;
//                wait_ms(330);
//                timePassed += 500;
//            }
//            break;
//        }
//    }
//    protectedSerialSend(stimSend, 0);
//    lcdWriteChar(place == 1 ? 'd' : 'D', 4, 1);
//}

static void stim_G2(int place, int stim, int type) {
  if (place == 1 || place == 2) {
    Valve_ON(stim + 10);
    waitTaskTimer_G2(500u);
  }
  switch (stim) {
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 20: // FAKE ODAP
    switch (place) {
    case 1:
      assertLaser_G2(type, atFirstOdorBeginning);
      break;
    case 2:
      assertLaser_G2(type, atSecondOdorBeginning);
      break;
    case 3:
      assertLaser_G2(type, atResponseCueBeginning);
      break;
    }

    Valve_ON(stim);
    int stimSend;
    switch (place) {
    case 1:
    case 2:
      if (isLikeOdorA_G2(stim)) {
        stimSend = 9;
        // Out2 = 1;
      } else {
        stimSend = 10;
        // Out3 = 1;
      }
      break;
    case 3:
      stimSend = SpResponseCue;
      break;
    }
    protectedSerialSend_G2(stimSend, stim);
    switch (place) {
    case 1:
      lcdWriteChar_G2('1', 4, 1);
      waitTaskTimer_G2(stims_G2.stim1Length - 100u);
      break;
    case 2:
      lcdWriteChar_G2('2', 4, 1);
      waitTaskTimer_G2(stims_G2.stim2Length - 100u);
      break;
    case 3:
      lcdWriteChar_G2('3', 4, 1);
      waitTaskTimer_G2(stims_G2.respCueLength - 100u);
      break;
    }
    Valve_OFF(stim + 10);
    waitTaskTimer_G2(100u);
    Valve_OFF(stim);
    // Out2 = 0;
    // Nop();
    // Nop();
    // Out3 = 0;
    // Nop();
    // Nop();
    protectedSerialSend_G2(stimSend, 0);
    switch (place) {
    case 1:
      assertLaser_G2(type, atFirstOdorEnd);
      lcdWriteChar_G2('d', 4, 1);
      waitTaskTimer_G2(
          1000u > stims_G2.stim1Length ? 1000u - stims_G2.stim1Length : 0);
      break;
    case 2:
      assertLaser_G2(type, atSecondOdorEnd);
      lcdWriteChar_G2('D', 4, 1);
      waitTaskTimer_G2(
          1000u > stims_G2.stim2Length ? 1000u - stims_G2.stim2Length : 0);
      break;
    case 3:
      assertLaser_G2(type, atResponseCueEnd);
      break;
    }
    break;
    //        case 20:
    //        case 21:
    //        case 22:
    //        case 23:
    //            optoStim(stim, place == 1 ? stims.stim1Length :
    //            stims.stim2Length, place);
    //            break;
  }
}

unsigned int responseDelayLicked_G2(unsigned int lengthInMilliSec) {
  unsigned long targetTime = millis() + lengthInMilliSec;
  lick_G2.flag = 0;
  while (millis() < targetTime) {
    if (lick_G2.current) {
      lick_G2.flag = 1;
    }
  }
  waitTaskTimer_G2(lengthInMilliSec);
  return lick_G2.flag;
}

static void zxLaserTrial_G2(int type, int firstOdor, STIM_T_G2 odors,
                            _delayT interOdorDelay, int secondOdor,
                            float waterPeroid, unsigned int ITI) {
  resetTaskTimer_G2();
  protectedSerialSend_G2(Sptrialtype, type);
  protectedSerialSend_G2(Splaser, (type != laserOff));
  assertLaser_G2(type, fourSecBeforeFirstOdor);
  waitTaskTimer_G2(1000u);
  assertLaser_G2(type, threeSecBeforeFirstOdor);
  waitTaskTimer_G2(2000u);
  assertLaser_G2(type, oneSecBeforeFirstOdor);
  waitTaskTimer_G2(500u);
  assertLaser_G2(type, at500mSBeforeFirstOdor);
  // waitTaskTimer_G2(500u);

  /////////////////////////////////////////////////
  stim_G2(1, firstOdor, type);
  ////////////////////////////////////////////////

  switch (taskType_G2) {

  // Do nothing during Go Nogo Tasks
  case GONOGO_LR_TASK:
  case GONOGO_TASK:
  case GONOGO_2AFC_TEACH:
    assertLaser_G2(type, atSecondOdorEnd);
    break;

  default: ////////////////////////////////////DELAY/////////////////////
    if (interOdorDelay == 0) {
      waitTaskTimer_G2(200u); ////////////////NO DELAY////////////////////
    } else {

      assertLaser_G2(type, atDelayBegin);
      waitTaskTimer_G2(500u);
      assertLaser_G2(type, atDelay_5SecIn);
      waitTaskTimer_G2(500u);
      assertLaser_G2(type, atDelay1SecIn); ////////////////1Sec////////////

      if (interOdorDelay < 4) {
        waitTaskTimer_G2((interOdorDelay * 1000u) > 2000u
                             ? (interOdorDelay * 1000u) - 2000u
                             : 0);
      } else {

        waitTaskTimer_G2(500u);
        assertLaser_G2(type,
                       atDelay1_5SecIn); ////////////////1.5Sec////////////////
        waitTaskTimer_G2(500u);

        assertLaser_G2(type, atDelay2SecIn); /////////////2Sec/////////////

        /*/////////////////////////////////////////////////
         * ////////DISTRACTOR//////////////////////////////
         * //////////////////////////////////////////////*/
        if (taskType_G2 == DUAL_TASK_LEARNING || taskType_G2 == DUAL_TASK ||
            taskType_G2 == DUAL_TASK_ON_OFF_LASER_TASK ||
            taskType_G2 == DUAL_TASK_ODAP_ON_OFF_LASER_TASK ||
            taskType_G2 == DNMS_DUAL_TASK_LEARNING ||
            taskType_G2 == DNMS_DUAL_TASK ||
            taskType_G2 == DUAL_TASK_EVERY_TRIAL) {
          assertLaser_G2(type, atPreDualTask); //@2s
          distractor_G2(stims_G2.currentDistractor,
                        stims_G2.distractorJudgingPair, waterPeroid);
          waitTaskTimer_G2(1500u);
          assertLaser_G2(type, atPostDualTask); // distractor@3.5sec
          if (interOdorDelay > 8u) {
            waitTaskTimer_G2((interOdorDelay > 8u ? interOdorDelay - 8u : 0) *
                             500u);
          }
        } else if (interOdorDelay >= 12u) {
          waitTaskTimer_G2(500u);
          assertLaser_G2(type, atDelay2_5SecIn);
          waitTaskTimer_G2(500u);
          assertLaser_G2(type, atDelay3SecIn);
          waitTaskTimer_G2(500u); //->3.5s
          assertLaser_G2(type, atDelay3_5SIn);
          waitTaskTimer_G2(1000u); //->4.5s
          assertLaser_G2(type, atDelay4_5SIn);
          waitTaskTimer_G2(1000u);
          waitTaskTimer_G2((interOdorDelay > 12u ? interOdorDelay - 12u : 0) *
                           500u);
        } else {
          waitTaskTimer_G2(interOdorDelay * 500u > 3000u
                               ? interOdorDelay * 500u - 3000u
                               : 0);
          assertLaser_G2(type, atDelay1sToMiddle); // 13@5500
          waitTaskTimer_G2(500u);                  // 13@6000
        }

        assertLaser_G2(type, atDelay500msToMiddle); // distractor@6 //13@6000
        waitTaskTimer_G2(500u);
        assertLaser_G2(type, atDelayMiddle); // 13@6.5
        if (interOdorDelay >= 12) {
          waitTaskTimer_G2(500u); // 13@7
          assertLaser_G2(type, atDelayMid500mSec);
          waitTaskTimer_G2(500u); // 13@7.5
          assertLaser_G2(type, atDelayMid1Sec);
          waitTaskTimer_G2(500u); // 13@8S
          assertLaser_G2(type, atDelayMid1_5Sec);
          waitTaskTimer_G2(500u); // 13@8.5
          assertLaser_G2(type, atDelayMid2Sec);
          waitTaskTimer_G2(500u); // distractor@9s//13@9
          assertLaser_G2(type, atDelayMid2_5Sec);
          waitTaskTimer_G2(500u); // distractor@9.5s//13@9.5
          assertLaser_G2(type, atDelayMid3Sec);
          waitTaskTimer_G2((interOdorDelay > 11u ? interOdorDelay - 11u : 0) *
                           500u); // 13@10
        } else {
          waitTaskTimer_G2(interOdorDelay * 500uL > 2500u
                               ? interOdorDelay * 500uL - 2500u
                               : 0);
        }
        assertLaser_G2(type, atDelayLast2_5SecBegin);
        waitTaskTimer_G2(500u); // 13@10.5
        assertLaser_G2(
            type,
            atDelayLast2SecBegin); //////////////-2 Sec//////////////////////

        waitTaskTimer_G2(500u);
        assertLaser_G2(type, atDelayLast1_5SecBegin);
        waitTaskTimer_G2(500u);
      }
      assertLaser_G2(
          type,
          atDelayLastSecBegin); /////////////////////////-1 Sec////////////////
      waitTaskTimer_G2(500u);
      assertLaser_G2(type, atDelayLast500mSBegin);
      //            waitTimerJ(300u);
      //            assertLaser(type, atDelayLast200mSBegin);
      //            waitTimerJ(200u);
      // waitTaskTimer_G2(500u);
    }

    ///////////-Second odor-/////////////////
    stim_G2(2, secondOdor, type);
    //////////////////////////////////////////
    break;
  }
  unsigned int delayLick1 = responseDelayLicked_G2(1000u);
  Valve_ON(stims_G2.respCueValve + 10);
  unsigned int delayLick2 = responseDelayLicked_G2(500u);
  unsigned int delayLick = delayLick1 | delayLick2;
  lick_G2.flag = delayLick;
  //////////////-Response Cue-/////////////////
  int resultRtn = 0;
  if (delayLick) {
    protectedSerialSend_G2(SpAbortTrial, 1);
    abortTrials++;
    lcdWriteChar_G2('A', 4, 1);
    waitTaskTimer_G2(500u);
    Valve_OFF(stims_G2.respCueValve + 10);
  } else {
    assertLaser_G2(type, atRewardBeginning);
    lcdWriteChar_G2('R', 4, 1);
    stim_G2(3, stims_G2.respCueValve, type);
    // Assess Performance here
    int id =
        (taskType_G2 == DUAL_TASK || taskType_G2 == DUAL_TASK_LEARNING ||
         taskType_G2 == DUAL_TASK_ON_OFF_LASER_TASK ||
         taskType_G2 == DUAL_TASK_ODAP_ON_OFF_LASER_TASK ||
         taskType_G2 == DNMS_DUAL_TASK_LEARNING ||
         taskType_G2 == DNMS_DUAL_TASK || taskType_G2 == DUAL_TASK_EVERY_TRIAL)
            ? 2
            : 1;
    resultRtn = waterNResult_G2(firstOdor, secondOdor, waterPeroid, id);
  }
  waitTaskTimer_G2(1050u); // water time sync
  // Total Trials
  int totalTrials = hit + correctRejection + miss + falseAlarm + abortTrials;
  lcdWriteNumber_G2(abortTrials, 3, 14, 2);
  // Discrimination rate
  if (hit + correctRejection > 0) {

    correctRatio = 100 * (hit + correctRejection) / totalTrials;
    lcdWriteNumber_G2(correctRatio, 2, 14, 1);
  }
  lcdWriteChar_G2('I', 4, 1);

  ///--ITI1---///
  assertLaser_G2(type, atITIBeginning);
  waitTaskTimer_G2(1000u);
  assertLaser_G2(type, atITIOneSecIn);
  if (resultRtn == SpFalseAlarm) {
    if (punishFalseAlarm_G2 == FALSE_ALARM_PENALTY_TIMEOUT) {
      ITI *= 3u;
    } else if (punishFalseAlarm_G2 == FALSE_ALARM_PENALTY_REPEAT) {
      stims_G2.falseAlarmPenaltyRepeat = 1;
    }
  } else {
    stims_G2.falseAlarmPenaltyRepeat = 0;
  }

  if (ITI >= 5u) {
    unsigned int trialITI = ITI - 5u;
    while (trialITI > 60u) {
      resetTaskTimer_G2();
      waitTaskTimer_G2(60u * 1000u);
      trialITI -= 60u;
    }
    resetTaskTimer_G2();
    waitTaskTimer_G2(trialITI *
                     1000u); // another 4000 is at the beginning of the trials.
  }
  protectedSerialSend_G2(SpITI, 0);

  waitTrial_G2();
}

static void feedWaterFast_G2(int waterLength) {

  lick_G2.LCount = 0;
  lick_G2.RCount = 0;
  unsigned int waterCount = 0;
  unsigned int totalLickCount = 0;
  lcdSplash_G2("Total Lick", "");

  unsigned long startTime = millis() - 1000u;
  while (1) {
    if (lick_G2.LCount > totalLickCount) {
      if (millis() > startTime + 500) {
        Valve_ON(water_sweet);
        startTime = millis();
        lcdWriteNumber_G2(++waterCount, 4, 11, 2);
        protectedSerialSend_G2(SpWater_sweet, waterCount / 10);
      }
      totalLickCount = lick_G2.LCount;
      lcdWriteNumber_G2(totalLickCount, 4, 11, 1);
    }
    if (millis() > startTime + waterLength) {
      Valve_OFF(water_sweet);
    }
  }
}

void lcdSplash_G2(const char s1[], const char s2[]) {
  lcdHomeNClear();
  lcdWriteString_G2(s1);
  LCD_set_xy(0, 1);
  lcdWriteString_G2(s2);
  delay(1000);
}

void wait_ms_G2(int time) {

  unsigned long startTime = millis();
  while (millis() < startTime + time) {
  }
}

static void laserTrain_G2() {
  unsigned int freqs[] = {1, 5, 10, 20, 50, 100};
  laser_G2.onTime = 5;
  unsigned int idx = 0;
  for (idx = 0; idx < 6; idx++) {
    unsigned int duration = 1000 / freqs[idx];
    laser_G2.offTime = duration - laser_G2.onTime;
    turnOnLaser_G2(3);
    wait_ms_G2(duration * 10 - 1);
    turnOffLaser_G2();
    wait_ms_G2(2000);
  }
}

void testValve_G2(void) {
  unsigned int v = getFuncNumberGen2(2, "Valve No?");
  while (1) {
    Valve_ON(v+10);
    wait_ms_G2(500);
    Valve_ON(v);
    wait_ms_G2(1000);
    Valve_OFF(v);
    Valve_OFF(v+10);
    wait_ms_G2(1000);
  }
}

static void testVolume_G2(int waterLength) {
  unsigned int n = getFuncNumberGen2(2, "Valve #");
  int i;
  for (i = 0; i < 100; i++) {

    Valve_ON(n);
    wait_ms_G2(waterLength);
    Valve_OFF(n);
    wait_ms_G2(500 - waterLength);
  }
}

static void test_Laser_G2(void) {
  int i = 1;
  while (1) {

    laser_G2.on = i;
    // Out3 = i;
    getFuncNumberGen2(1, "Toggle Laser");
    i = (i + 1) % 2;
  }
}

// static void flashLaser_G2(void) {
//   lcdSplash_G2("Test Laser", "");
//   int count = 0;
//   while (1) {
//     lcdWriteNumber_G2(++count, 3, 10, 2);
//     resetTaskTimer_G2();
//     turnOnLaser_G2(3);
//     waitTaskTimer_G2(2000u);
//     turnOffLaser_G2();
//     waitTaskTimer_G2(5000u);
//   }
// }
//
/****************************************
 *for calibration between counter and time
 *****************************************
void correctTime() {
    int cy = 0;
    for (; cy < 10; cy++) {
        unsigned int i;
        Out2 = 1;
        for (i = 0; i < 64000; i++);
        Out2 = 0;
        for (i = 0; i < 64000; i++);
    }
}
 ****************************************/

static void testNSetBound() {
  unsigned long startTime = millis();
  while (millis() < startTime + 10000ul) {
    int sum = 0;
    for (int i = 0; i < 15; i++) {
      sum += analogRead(0);
    }
    int val = sum / 15;
    sendLargeValue(val);
    delay(500);
  }
  int newBound = getFuncNumberGen2(3, "New Lick Thres?");
  sendLargeValue(newBound);
  EEPROM.put(0, newBound);
  protectedSerialSend_G2(61, 0);
  callResetGen2();
}

static void setWaterLen() {
  int newLen;
  EEPROM.get(sizeof(newLen), newLen);
  sendLargeValue(newLen);
  newLen = getFuncNumberGen2(3, "Water Len in ms?");
  sendLargeValue(newLen);
  EEPROM.put(sizeof(newLen), newLen);
  protectedSerialSend_G2(61, 0);
  callResetGen2();
}

static void stepLaser_G2() {
  int laserOn = 3000;
  int laserOff_ = 16000;
  int step;
  int sCounter = 0;
  // PORTCbits.RC1 = 1;
  // pwm_G2.R_Lo = 0xfe;
  for (; sCounter < 20; sCounter++) {
    unsigned int powerx10 = getFuncNumberGen2(2, "Power x10");
    lcdSplash_G2("     Power", "Trial");
    lcdWriteNumber_G2(powerx10, 2, 12, 1);

    step = 20;
    for (; step > 0; step--) {
      lcdWriteNumber_G2(step, 2, 7, 2);
      wait_ms_G2(laserOff_);
      int tag = powerx10 * 10;
      // Out2 = 1;
      int timer;
      for (timer = 0; timer < 1000; timer++)
        ;
      // Out2 = 0;
      for (; tag > 0; tag--) {
        for (timer = 0; timer < 1000; timer++)
          ;
      }
      // Out2 = 1;
      for (timer = 0; timer < 1000; timer++)
        ;
      // Out2 = 0;
      turnOnLaser_G2(3);
      wait_ms_G2(laserOn);
      turnOffLaser_G2();
    }
  }
}

void varifyOpticalSuppression_G2() {
  lcdSplash_G2("L OFF   0/100", "");
  int i;
  for (i = 0; i < 100; i++) {
    resetTaskTimer_G2();
    // Out4 = 1;
    LCD_set_xy(3, 1);
    lcdWriteString_G2("ON ");
    lcdWriteNumber_G2(i + 1, 3, 7, 1);
    waitTaskTimer_G2(2000);
    // Out4 = 0;
    LCD_set_xy(3, 1);
    lcdWriteString_G2("OFF");
    protectedSerialSend_G2(Splaser, i);
    waitTaskTimer_G2(18000);
  }
}

void sendLargeValue(int val) {
  byte valHigh = (byte)(val / 100);
  byte valLow = (byte)(val % 100);
  protectedSerialSend_G2(23, valHigh);
  protectedSerialSend_G2(24, valLow);
}

void callFunc(int n) {
  currentMiss = 0;

  EEPROM.get(0, lickBound_G2);

  srand(analogRead(0));
  initZXTMR_G2();
  switch (n) {
  //            int m;
  // ZX's functions

  case 4301:
    testNSetBound();
    break;
  case 4302:
    setWaterLen();
    break;

  case 4303:
    lcdSplash_G2("Each Quarter", "Delay LR Laser");
    laserSessionType_G2 = LASER_LR_EACH_QUARTER;
    laser_G2.ramp = 500;
    zxLaserSessions_G2(5, 6, laserDuringDelay, 12u, 24u, 20, WaterLen, 20,
                       setSessionNum_G2());
    break;
  case 4304: {
    lcdSplash_G2("DNMS ", "Shaping");
    laserSessionType_G2 = LASER_NO_TRIAL;
    taskType_G2 = SHAPING_TASK;
    int type = setType_G2();
    zxLaserSessions_G2(type, type + 1, laserDuringDelay, 4u, 8u, 20, WaterLen,
                       50, setSessionNum_G2());
    break;
  }

  case 4305:
    POST();

    break;
  //        case 4305:
  //        {
  //            splash("DNMS Task", "");
  //            taskType = DNMS_TASK;
  //            laserSessionType = LASER_NO_TRIAL;
  //            int type = setType();
  //            zxLaserSessions(type, type + 1, laserDuringDelayChR2, 5u, 10u,
  //            20, WaterLen, 20, setSessionNum());
  //            break;
  //        }
  case 4306: {
    lcdSplash_G2("OB Opto Stim", "Go No-Go");
    //            zxGoNogoSessions(type,type+1,3, 20, 1, 0.5, 4);
    laserSessionType_G2 = LASER_NO_TRIAL;
    taskType_G2 = GONOGO_TASK;
    int type = setType_G2();
    zxLaserSessions_G2(type, type + 1, laserDuring3Baseline, 0u, 5u, 20,
                       WaterLen, 20, setSessionNum_G2());
    break;
  }
  case 4307: {
    lcdSplash_G2("OB Opto Stim", "");
    //            zxGoNogoSessions(type,type+1,3, 20, 1, 0.5, 4);
    laserSessionType_G2 = LASER_LR_EVERYTRIAL;
    taskType_G2 = OPTO_ODPA_TASK;
    int type = setType_G2();
    zxLaserSessions_G2(type, type + 1, laserDuring1stOdor, 4u, 8u, 20, WaterLen,
                       30, setSessionNum_G2());
    break;
  }

  case 4308: {
    lcdSplash_G2("OB Opto Stim", "Shaping");
    //            zxGoNogoSessions(type,type+1,3, 20, 1, 0.5, 4);
    laserSessionType_G2 = LASER_LR_EVERYTRIAL;
    taskType_G2 = OPTO_ODPA_SHAPING_TASK;
    int type = setType_G2();
    zxLaserSessions_G2(type, type + 1, laserDuring1stOdor, 4u, 8u, 20, WaterLen,
                       30, setSessionNum_G2());
    break;
  }
  case 4310: {
    unsigned int m = getFuncNumberGen2(2, "Time in ms");
    testVolume_G2(m);
    break;
  }
  case 4311:
    testVolume_G2(50);
    break;

  case 4312: {
    lcdSplash_G2("Response Delay", "Laser Control");
    laserSessionType_G2 = LASER_OTHER_TRIAL;
    int type = setType_G2();
    zxLaserSessions_G2(type, type + 1, laserDuringResponseDelay, 5u, 10u, 20,
                       WaterLen, 20, setSessionNum_G2());
    break;
  }
  case 4313: {
    lcdSplash_G2("GoNogo Control", "For DNMS");

    laserSessionType_G2 = LASER_OTHER_TRIAL;
    taskType_G2 = GONOGO_TASK;
    int type = setType_G2();
    zxLaserSessions_G2(type, type + 1, laserDuring3Baseline, 0u, 5u, 20,
                       WaterLen, 20, setSessionNum_G2());
    break;
  }
  case 4314: {
    lcdSplash_G2("Vary Length", "Delay Laser");
    laserSessionType_G2 = LASER_VARY_LENGTH;
    laser_G2.ramp = 500;
    int type = setType_G2();
    zxLaserSessions_G2(type, type + 1, laserDuringDelay, 5u, 10u, 20, WaterLen,
                       15, setSessionNum_G2());
    laser_G2.ramp = 0;
    break;
  }
  case 4315:
    lcdSplash_G2("Each Quarter", "Delay Laser");
    laserSessionType_G2 = LASER_EACH_QUARTER;
    laser_G2.ramp = 500;
    zxLaserSessions_G2(5, 6, laserDuringDelay, 8u, 16u, 20, WaterLen, 20,
                       setSessionNum_G2());
    laser_G2.ramp = 0;
    break;

  case 4316:
    stepLaser_G2();
    break;

  case 4317:
    taskType_G2 = NO_ODOR_CATCH_TRIAL_TASK;
    laser_G2.ramp = 500;
    lcdSplash_G2("No Odor Catch", "");

    laserSessionType_G2 = LASER_LR_EVERYTRIAL;
    zxLaserSessions_G2(5, 6, laserRampDuringDelay, 12u, 24u, 20, WaterLen, 20,
                       setSessionNum_G2());
    break;

  //        case 4317:
  //            correctTime();
  //            break;

  case 4318:
    lcdSplash_G2("No Trial Wait", "LR Laser");
    wait_Trial_G2 = 0;
    break;

  case 4319: {
    taskType_G2 = DNMS_TASK;
    laser_G2.ramp = 500;
    lcdSplash_G2("LR LASER DNMS", "Sufficiency");
    _delayT delay = setDelay_G2();
    laserSessionType_G2 = LASER_LR_EVERYTRIAL;
    zxLaserSessions_G2(5, 6, laserRampDuringDelay, delay, delay * 2u, 20u,
                       WaterLen, 20, setSessionNum_G2());
    break;
  }

  case 4320:
    lcdSplash_G2("12s Each Quarter", "Delay LR Laser");
    laserSessionType_G2 = LASER_12s_LR_EACH_QUARTER;
    laser_G2.ramp = 500;
    zxLaserSessions_G2(5, 6, laserDuringDelay, 12u, 24u, 20, WaterLen, 25,
                       setSessionNum_G2());
    laser_G2.ramp = 0;
    break;
  case 4321:
    //            setLaser();
    //            taskType = _DNMS_TASK;
    //            splash("NoDelay Control", "For DNMS");
    //            laserTrialType = _LASER_OTHER_TRIAL;
    //            m = getFuncNumberGen2(1, "Short Long B+R");
    //            switch (m) {
    //                case 1: m = laserNoDelayControlShort;
    //                    break;
    //                case 2: m = laserNoDelayControl;
    //                    break;
    //                case 3: m = laserDuringBaseAndResponse;
    //                    break;
    //            }
    //            zxLaserSessions(type,type+1,m, 0.2, 5u, 20, WaterLen, 20,
    //            setSessionNum());
    break;
  // case 4322:
  //     variableVoltage_G2();
  //     break;

  case 4323:
    lcdSplash_G2("DNMS LR TASK", "Ramp Laser ++");
    protectedSerialSend_G2(PERM_INFO, DMS_LR_Teach_LON);
    taskType_G2 = DNMS_2AFC_TEACH;
    laserSessionType_G2 = LASER_EVERY_TRIAL;
    laser_G2.ramp = 500;
    zxLaserSessions_G2(5, 6, laserRampDuringDelay, 5u, 10u, 20, WaterLen, 30,
                       setSessionNum_G2());
    break;

  case 4324:
    lcdSplash_G2("DNMS LR TASK", "Ramp Laser +-");
    protectedSerialSend_G2(PERM_INFO, DMS_LR_Teach_ONOFF);
    taskType_G2 = DNMS_2AFC_TEACH;
    laserSessionType_G2 = LASER_OTHER_TRIAL;
    laser_G2.ramp = 500;
    zxLaserSessions_G2(5, 6, laserRampDuringDelay, 5u, 10u, 20, WaterLen, 30,
                       setSessionNum_G2());
    break;

  case 4325:
    lcdSplash_G2("BaseLine RQ", "Control");
    laserSessionType_G2 = LASER_OTHER_TRIAL;
    zxLaserSessions_G2(5, 6, laserDuring3Baseline, 5u, 10u, 20, WaterLen, 20,
                       setSessionNum_G2());
    break;

  case 4326:
    taskType_G2 = DNMS_TASK;
    laser_G2.ramp = 500;
    lcdSplash_G2("Incongrument", "LR Laser");
    laserSessionType_G2 = LASER_INCONGRUENT_CATCH_TRIAL;
    zxLaserSessions_G2(5, 6, laserRampDuringDelay, 12u, 24u, 20, WaterLen, 20,
                       setSessionNum_G2());
    break;

  case 4327:
    lcdSplash_G2("DNMS LR TEACH", "No Laser");
    protectedSerialSend_G2(PERM_INFO, DMS_LR_Teach_LOFF);
    taskType_G2 = DNMS_2AFC_TEACH;
    laserSessionType_G2 = LASER_NO_TRIAL;
    zxLaserSessions_G2(5, 6, laserRampDuringDelay, 5u, 10u, 20, WaterLen, 30,
                       setSessionNum_G2());
    break;

  case 4328:
    lcdSplash_G2("MW/OD LR TEACH", "No Laser");
    protectedSerialSend_G2(PERM_INFO, MSWOD_LR_Teach_LOFF);
    taskType_G2 = DNMS_2AFC_TEACH;
    laserSessionType_G2 = LASER_NO_TRIAL;
    zxLaserSessions_G2(5, 6, laserRampDuringDelay, 0u, 10u, 20, WaterLen, 30,
                       setSessionNum_G2());
    break;

  case 4329:
    lcdSplash_G2("LR DNMS 5s Delay", "Laser + -");
    protectedSerialSend_G2(PERM_INFO, DMS_LR_5Delay_Laser);
    taskType_G2 = DNMS_2AFC_TEACH;
    laserSessionType_G2 = LASER_OTHER_TRIAL;
    laser_G2.ramp = 500;
    zxLaserSessions_G2(5, 6, laserRampDuringDelay, 5u, 10u, 20, WaterLen, 30,
                       setSessionNum_G2());
    break;

  case 4330:
    lcdSplash_G2("LR DNMS 8s Delay", "Laser + -");
    protectedSerialSend_G2(PERM_INFO, DMS_LR_8Delay_Laser);
    taskType_G2 = DNMS_2AFC_TEACH;
    laserSessionType_G2 = LASER_OTHER_TRIAL;
    laser_G2.ramp = 500;
    zxLaserSessions_G2(5, 6, laserRampDuringDelay, 8u, 16u, 20, WaterLen, 30,
                       setSessionNum_G2());
    break;

  case 4331: {
    lcdSplash_G2("Delay+Odor2", "Control");
    laserSessionType_G2 = LASER_OTHER_TRIAL;
    int type = setType_G2();
    zxLaserSessions_G2(type, type + 1, laserDuringDelay_Odor2, 5u, 10u, 20,
                       WaterLen, 20, setSessionNum_G2());
    break;
  }
  case 4332: {
    lcdSplash_G2("Odor", "Control");
    laserSessionType_G2 = LASER_OTHER_TRIAL;
    unsigned int m = getFuncNumberGen2(1, "1st 2nd BothOdor");
    int type = setType_G2();
    zxLaserSessions_G2(type, type + 1,
                       m == 1 ? laserDuring1stOdor
                              : m == 2 ? laserDuring2ndOdor : laserDuringOdor,
                       5u, 10u, 20, WaterLen, 50, setSessionNum_G2());
    break;
  }
  case 4333:
    test_Laser_G2();
    break;

  case 4334:
    lcdSplash_G2("LR DNMS 12s Delay", "Laser + -");
    protectedSerialSend_G2(PERM_INFO, DMS_LR_12Delay_Laser);
    taskType_G2 = DNMS_2AFC_TEACH;
    laserSessionType_G2 = LASER_OTHER_TRIAL;
    laser_G2.ramp = 500;
    zxLaserSessions_G2(5, 6, laserRampDuringDelay, 12u, 24u, 20, WaterLen, 30,
                       setSessionNum_G2());
    break;

  case 4335:
    lcdSplash_G2("LR DNMS 1st Odor", "Laser + -");
    protectedSerialSend_G2(PERM_INFO, DMS_LR_1Odor_Laser);
    taskType_G2 = DNMS_2AFC_TEACH;
    laserSessionType_G2 = LASER_OTHER_TRIAL;
    zxLaserSessions_G2(5, 6, laserDuring1stOdor, 5u, 10u, 20, WaterLen, 30,
                       setSessionNum_G2());
    break;

  case 4336:
    lcdSplash_G2("LR DNMS 2nd Odor", "Laser + -");
    protectedSerialSend_G2(PERM_INFO, DMS_LR_2Odor_Laser);
    taskType_G2 = DNMS_2AFC_TEACH;
    laserSessionType_G2 = LASER_OTHER_TRIAL;
    zxLaserSessions_G2(5, 6, laserDuring2ndOdor, 5u, 10u, 20, WaterLen, 30,
                       setSessionNum_G2());
    break;

  case 4337:
    lcdSplash_G2("LR DNMS 1+2 Odor", "Laser + -");
    protectedSerialSend_G2(PERM_INFO, DMS_LR_bothOdor_Laser);
    taskType_G2 = DNMS_2AFC_TEACH;
    laserSessionType_G2 = LASER_OTHER_TRIAL;
    zxLaserSessions_G2(5, 6, laserDuringOdor, 5u, 10u, 20, WaterLen, 30,
                       setSessionNum_G2());
    break;

  case 4338:
    lcdSplash_G2("LR DNMS Baseline", "Laser + -");
    protectedSerialSend_G2(PERM_INFO, DMS_LR_baseline_Laser);
    taskType_G2 = DNMS_2AFC_TEACH;
    laserSessionType_G2 = LASER_OTHER_TRIAL;
    zxLaserSessions_G2(5, 6, laserDuringBaseline, 5u, 10u, 20, WaterLen, 30,
                       setSessionNum_G2());
    break;

  case 4339:
    lcdSplash_G2("LR DNMS Response", "Laser + -");
    protectedSerialSend_G2(PERM_INFO, DMS_LR_response_Laser);
    taskType_G2 = DNMS_2AFC_TEACH;
    laserSessionType_G2 = LASER_OTHER_TRIAL;
    zxLaserSessions_G2(5, 6, laserDuringResponseDelay, 5u, 10u, 20, WaterLen,
                       30, setSessionNum_G2());
    break;

  case 4340: {
    taskType_G2 = DNMS_TASK;
    laser_G2.ramp = 500;
    lcdSplash_G2("LR LASER DNMS", "Sufficiency");
    _delayT delay = setDelay_G2();
    laserSessionType_G2 = LASER_LR_EVERY_OTHER_TRIAL;
    zxLaserSessions_G2(5, 6, laserRampDuringDelay, delay, delay * 2u, 20u,
                       WaterLen, 20, setSessionNum_G2());
    break;
  }

  //        case 4334:
  //            splash("DNMS Shaping", "");
  //            laserTrialType = _LASER_NO_TRIAL;
  //            taskType = _SHAPING_TASK;
  //            zxLaserSessions(5,6, laserDuringDelay, 4u, 8u, 20, 0.5, 50,
  //            setSessionNum());
  //            break;

  //        case 4335:
  //        {
  //            splash("DNMS 4s", "DC laser,");
  //            taskType = _DNMS_TASK;
  //            zxLaserSessions(5,6, laserDuringDelay, 4u, 8u, 20, 0.5, 50,
  //            setSessionNum());
  //            break;
  //        }

  case 4341:
    lcdSplash_G2("DNMS 5s Shaping", "RQ");
    laserSessionType_G2 = LASER_NO_TRIAL;
    taskType_G2 = SHAPING_TASK;
    zxLaserSessions_G2(5, 6, laserDuringDelayChR2, 5u, 10u, 20, WaterLen, 50,
                       setSessionNum_G2());
    break;

  //        case 4342:
  //        {
  //            splash("DNMS 5s Water+", "RQ DC laser");
  //            laserTrialType = _LASER_EVERY_TRIAL;
  //            taskType = _DNMS_TASK;
  //            zxLaserSessions(5,6, laserDuringDelayChR2, 5u, 10u, 20, 0.1, 50,
  //            setSessionNum());
  //            break;
  //        }
  //        case 4343:
  //        {
  //            splash("DNMS 5s ++", "RQ DC laser");
  //            laserSessionType = LASER_EVERY_TRIAL;
  //            taskType = DNMS_TASK;
  //            zxLaserSessions(5, 6, laserDuringDelayChR2, 5u, 10u, 20,
  //            WaterLen,
  //            50, setSessionNum());
  //            break;
  //        }
  //        case 4344:
  //        {
  //            splash("DNMS 5s", "RQ NoLaser");
  //            laserSessionType = LASER_NO_TRIAL;
  //            taskType = DNMS_TASK;
  //            zxLaserSessions(5, 6, laserDuringDelayChR2, 5u, 10u, 20,
  //            WaterLen,
  //            50, setSessionNum());
  //            break;
  //        }
  //
  //        case 4345:
  //        {
  //            splash("DNMS 8s ++", "RQ DC laser");
  //            laserSessionType = LASER_EVERY_TRIAL;
  //            taskType = DNMS_TASK;
  //            zxLaserSessions(5, 6, laserDuringDelayChR2, 8u, 16u, 20,
  //            WaterLen,
  //            50, setSessionNum());
  //            break;
  //        }

  //        case 4351:
  //            splash("Rule Match Shap", "C-B D-A");
  //            laserTrialType = _LASER_NO_TRIAL;
  //            taskType = _ASSOCIATE_SHAPING_TASK;
  //            zxLaserSessions(0, laserDuringDelayChR2, 5u, 10, 20u, WaterLen,
  //            50,
  //            setSessionNum());
  //            break;
  //
  //        case 4352:
  //            splash("Rule Match", "C-B D-A");
  //            laserTrialType = _LASER_NO_TRIAL;
  //            taskType = _ASSOCIATE_TASK;
  //            zxLaserSessions(0, laserDuringDelayChR2, 5u, 10, 20u, WaterLen,
  //            50,
  //            setSessionNum());
  //            break;

  //        case 4353:
  //        {
  //            splash("DNMS 5s +-", "RQ DC laser");
  //            laserSessionType = LASER_OTHER_TRIAL;
  //            taskType = DNMS_TASK;
  //            zxLaserSessions(5, 6, laserDuringDelayChR2, 5u, 10u, 20,
  //            WaterLen,
  //            50, setSessionNum());
  //            break;
  //        }

  // case 4354:
  //     rampTweak_G2(1, 0);
  //     break;

  //        case 4355:
  //        {
  //            splash("DNMS 8s +-", "RQ DC laser");
  //            laserSessionType = LASER_OTHER_TRIAL;
  //            taskType = DNMS_TASK;
  //            zxLaserSessions(5, 6, laserDuringDelayChR2, 8u, 16u, 20,
  //            WaterLen,
  //            50, setSessionNum());
  //            break;
  //        }

  // case 4355:
  //     resetLaserPower_G2();
  //     break;
  //
  // case 4356:
  //     rampTweak_G2(0, 0);
  //     break;
  //
  // case 4357:
  //     rampTweak_G2(0, 1);
  //     break;
  //
  // case 4358:
  //     flashLaser_G2();
  //     break;
  //
  // case 4359:
  //     rampTweak_G2(1, 1);
  //     break;

  case 4360: {
    lcdSplash_G2("Dual Task", "Training");
    highLevelShuffleLength_G2 = 24;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    taskType_G2 = DUAL_TASK_LEARNING;
    zxLaserSessions_G2(sampleType, testType, laserOff, 13u, 13u, 24u, WaterLen,
                       30, setSessionNum_G2());
    break;
  }

  case 4361: {
    lcdSplash_G2("Dual Task", "");
    highLevelShuffleLength_G2 = 24;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    taskType_G2 = DUAL_TASK;
    laserSessionType_G2 = LASER_OTHER_TRIAL;
    laser_G2.ramp = 500;
    zxLaserSessions_G2(sampleType, testType, laserAfterDistractorMax, 13u, 13u,
                       24u, WaterLen, 30, setSessionNum_G2());
    break;
  }
  case 4362: {
    lcdSplash_G2("Distractor|Laser", "On Off");
    highLevelShuffleLength_G2 = 24;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    taskType_G2 = DUAL_TASK_ON_OFF_LASER_TASK;
    laserSessionType_G2 = LASER_DUAL_TASK_ON_OFF;
    laser_G2.ramp = 500;
    zxLaserSessions_G2(sampleType, testType, laserOff, 13u, 13u, 24u, WaterLen,
                       30, setSessionNum_G2());
    break;
  }
  case 4363: {
    lcdSplash_G2("Distractor|ODAP", "On Off");
    highLevelShuffleLength_G2 = 24;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    taskType_G2 = DUAL_TASK_ODAP_ON_OFF_LASER_TASK;
    laserSessionType_G2 = LASER_DUAL_TASK_ODAP_ON_OFF;
    laser_G2.ramp = 500;
    zxLaserSessions_G2(sampleType, testType, laserOff, 13u, 13u, 24u, WaterLen,
                       30, setSessionNum_G2());
    break;
  }

  case 4364: {
    lcdSplash_G2("Dual Task", "Training Laser");
    highLevelShuffleLength_G2 = 24;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    taskType_G2 = DUAL_TASK_LEARNING;
    laserSessionType_G2 = LASER_EVERY_TRIAL;
    laser_G2.ramp = 500;
    zxLaserSessions_G2(sampleType, testType, laserAfterDistractorMax, 13u, 13u,
                       24u, WaterLen, 30, setSessionNum_G2());
    break;
  }

  case 4365: {
    lcdSplash_G2("Dual Task", "3x Distr");
    highLevelShuffleLength_G2 = 72;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    taskType_G2 = DUAL_TASK_DISTRx3_TASK;
    laserSessionType_G2 = LASER_OTHER_TRIAL;
    laser_G2.ramp = 500;
    zxLaserSessions_G2(sampleType, testType, laserAfterMultiDistractor, 14u,
                       14u, 24u, WaterLen, 30, setSessionNum_G2());
    break;
  }
  case 4366: {
    lcdSplash_G2("Dual Task", "");
    highLevelShuffleLength_G2 = 48;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    taskType_G2 = DUAL_TASK_EVERY_TRIAL;
    laserSessionType_G2 = LASER_NO_TRIAL;
    zxLaserSessions_G2(sampleType, testType, laserAfterDistractorMax, 13u, 13u,
                       48u, WaterLen, 30, setSessionNum_G2());
    break;
  }

  case 4367: {
    lcdSplash_G2("Dual Task", "Block Laser");
    highLevelShuffleLength_G2 = 48;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    taskType_G2 = DUAL_TASK_EVERY_TRIAL;
    laserSessionType_G2 = LASER_OTHER_BLOCK;
    laser_G2.ramp = 500;
    int sesss = setSessionNum_G2();
    psedoRanInput_G2 = getFuncNumberGen2(1, "Rand = 0 or 1");
    zxLaserSessions_G2(sampleType, testType, laserAfterDistractorMax, 13u, 13u,
                       48u, WaterLen, 30, sesss);
    break;
  }

  case 4368: {
    lcdSplash_G2("Dual Task Block", "Baseline");
    highLevelShuffleLength_G2 = 48;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    taskType_G2 = DUAL_TASK_EVERY_TRIAL;
    laserSessionType_G2 = LASER_OTHER_BLOCK;
    laser_G2.ramp = 500;
    int sesss = setSessionNum_G2();
    psedoRanInput_G2 = getFuncNumberGen2(1, "Rand = 0 or 1");
    zxLaserSessions_G2(sampleType, testType, laserDuring4Baseline, 13u, 13u,
                       48u, WaterLen, 30, sesss);
    break;
  }

  case 4369: {
    lcdSplash_G2("Dual Task", "3x Distr BLOCK");
    highLevelShuffleLength_G2 = 72;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    taskType_G2 = DUAL_TASK_DISTRx3_TASK;
    laserSessionType_G2 = LASER_OTHER_BLOCK;
    laser_G2.ramp = 500;
    int sesss = setSessionNum_G2();
    psedoRanInput_G2 = getFuncNumberGen2(1, "Rand = 0 or 1");
    zxLaserSessions_G2(sampleType, testType, laserAfterMultiDistractor, 14u,
                       14u, 48u, WaterLen, 30, sesss);
    break;
  }

  case 4370: {
    lcdSplash_G2("ODPA", "Shaping");
    laserSessionType_G2 = LASER_NO_TRIAL;
    taskType_G2 = ODPA_SHAPING_TASK;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();

    zxLaserSessions_G2(sampleType, testType, laserOff, 13u, 13u, 24u, WaterLen,
                       30, setSessionNum_G2());
    break;
  }

  case 4371: {
    lcdSplash_G2("ODPA", "No Laser");
    laserSessionType_G2 = LASER_NO_TRIAL;
    taskType_G2 = ODPA_TASK;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    zxLaserSessions_G2(sampleType, testType, laserOff, 13u, 13u, 24u, WaterLen,
                       30, setSessionNum_G2());
    break;
  }

  case 4372: {
    lcdSplash_G2("ODPA", "Laser");
    laserSessionType_G2 = LASER_OTHER_TRIAL;
    laser_G2.ramp = 500;
    taskType_G2 = ODPA_TASK;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    zxLaserSessions_G2(sampleType, testType, laserDuringDelay, 13u, 13u, 24u,
                       WaterLen, 30, setSessionNum_G2());
    break;
  }

  case 4373: {
    lcdSplash_G2("ODPA Both Odor", "Laser");
    laserSessionType_G2 = LASER_OTHER_TRIAL;

    taskType_G2 = ODPA_TASK;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    zxLaserSessions_G2(sampleType, testType, laserDuringOdor, 5u, 10u, 20u,
                       WaterLen, 30, setSessionNum_G2());
    break;
  }

  case 4374: {

    lcdSplash_G2("ODPA", "Block Laser");
    highLevelShuffleLength_G2 = 48;
    laserSessionType_G2 = LASER_OTHER_BLOCK;
    laser_G2.ramp = 500;
    taskType_G2 = ODPA_TASK;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    int sesss = setSessionNum_G2();
    psedoRanInput_G2 = getFuncNumberGen2(1, "Rand = 0 or 1");
    zxLaserSessions_G2(sampleType, testType, laserDuringDelay, 13u, 13u, 48u,
                       WaterLen, 30, sesss);
    break;
  }

  case 4375: {
    lcdSplash_G2("ODPA R_D", "Shaping_A");
    highLevelShuffleLength_G2 = 20;
    laserSessionType_G2 = LASER_SESS_UNDEFINED;
    taskType_G2 = ODPA_RD_SHAPING_NO_LASER_TASK;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    zxLaserSessions_G2(sampleType, testType, laserOff, 5u, 8u, 20u, WaterLen,
                       100, setSessionNum_G2());
    break;
  }

  case 4376: {
    lcdSplash_G2("ODPA R_D", "Shaping_B");
    highLevelShuffleLength_G2 = 20;
    laserSessionType_G2 = LASER_SESS_UNDEFINED;
    taskType_G2 = ODPA_RD_SHAPING_B_CATCH_LASER_TASK;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    zxLaserSessions_G2(sampleType, testType, laserOff, 5u, 8u, 20u, WaterLen,
                       100, setSessionNum_G2());
    break;
  }

  case 4377: {
    lcdSplash_G2("ODPA R_D", "");
    highLevelShuffleLength_G2 = 20;
    laserSessionType_G2 = LASER_SESS_UNDEFINED;
    taskType_G2 = ODPA_RD_CATCH_LASER_TASK;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    zxLaserSessions_G2(sampleType, testType, laserOff, 5u, 8u, 20u, WaterLen,
                       20, setSessionNum_G2());
    break;
  }

  case 4378: {
    lcdSplash_G2("ODPA R_D", "TIMEOUT");
    highLevelShuffleLength_G2 = 20;
    laserSessionType_G2 = LASER_SESS_UNDEFINED;
    taskType_G2 = ODPA_RD_CATCH_LASER_TASK;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    punishFalseAlarm_G2 = FALSE_ALARM_PENALTY_TIMEOUT;
    zxLaserSessions_G2(sampleType, testType, laserOff, 5u, 8u, 20u, WaterLen,
                       20, setSessionNum_G2());
    break;
  }
  case 4379: {
    lcdSplash_G2("ODPA R_D", "REPEAT");
    highLevelShuffleLength_G2 = 20;
    laserSessionType_G2 = LASER_SESS_UNDEFINED;
    taskType_G2 = ODPA_RD_CATCH_LASER_TASK;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    punishFalseAlarm_G2 = FALSE_ALARM_PENALTY_REPEAT;
    zxLaserSessions_G2(sampleType, testType, laserOff, 5u, 8u, 20u, WaterLen,
                       20, setSessionNum_G2());
    break;
  }

  case 4380: {
    lcdSplash_G2("ODPA", "Shaping");
    laserSessionType_G2 = LASER_NO_TRIAL;
    taskType_G2 = ODPA_SHAPING_TASK;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    unsigned int delay = setDelay_G2();
    zxLaserSessions_G2(sampleType, testType, laserOff, delay, delay, 24u,
                       WaterLen, 30, setSessionNum_G2());
    break;
  }

  case 4381: {
    lcdSplash_G2("ODPA", "No Laser");
    laserSessionType_G2 = LASER_NO_TRIAL;
    taskType_G2 = ODPA_TASK;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    unsigned int delay = setDelay_G2();
    zxLaserSessions_G2(sampleType, testType, laserOff, delay, delay, 24u,
                       WaterLen, 30, setSessionNum_G2());
    break;
  }

  case 4382: {
    lcdSplash_G2("ODPA", "Laser");
    laserSessionType_G2 = LASER_OTHER_TRIAL;
    laser_G2.ramp = 500;
    taskType_G2 = ODPA_TASK;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    unsigned int delay = setDelay_G2();
    zxLaserSessions_G2(sampleType, testType, laserDuringDelay, delay, delay,
                       24u, WaterLen, 30, setSessionNum_G2());
    break;
  }
  case 4383: {
    lcdSplash_G2("DualTask-8s", "Training");
    highLevelShuffleLength_G2 = 24;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    taskType_G2 = DUAL_TASK_LEARNING;
    zxLaserSessions_G2(sampleType, testType, laserOff, 8u, 8u, 24u, WaterLen,
                       30, setSessionNum_G2());
    break;
  }

  case 4384: {
    lcdSplash_G2("DualTask-8s", "No Laser");
    highLevelShuffleLength_G2 = 24;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    taskType_G2 = DUAL_TASK;
    laserSessionType_G2 = LASER_NO_TRIAL;
    zxLaserSessions_G2(sampleType, testType, laserOff, 8u, 8u, 24u, WaterLen,
                       30, setSessionNum_G2());
    break;
  }

  case 4390: {
    lcdSplash_G2("ODPA", "Early Mid Late");
    laserSessionType_G2 = LASER_13s_EarlyMidLate;
    taskType_G2 = ODPA_TASK;
    lcdSplash_G2("Sample Odor", "");
    int sampleType = setType_G2();
    lcdSplash_G2("Test Odor", "");
    int testType = setType_G2();
    laser_G2.ramp = 500;
    zxLaserSessions_G2(sampleType, testType, laserOff, 13u, 13u, 24u, WaterLen,
                       30, setSessionNum_G2());
    break;
  }

  case 4399: {
    lcdSplash_G2("DNMS Dual Task", "Learning Laser");
    highLevelShuffleLength_G2 = 24;
    lcdSplash_G2("Odor", "");
    int odorType = setType_G2();
    taskType_G2 = DNMS_DUAL_TASK_LEARNING;
    laserSessionType_G2 = LASER_OTHER_TRIAL;
    laser_G2.ramp = 500;
    zxLaserSessions_G2(odorType, odorType + 1, laserAfterDistractorMax, 13u,
                       26u, 24u, WaterLen, 30, setSessionNum_G2());
    break;
  }

  case 4400: {
    lcdSplash_G2("DNMS Dual Task", "Training");
    highLevelShuffleLength_G2 = 24;
    lcdSplash_G2("Odor", "");
    int odorType = setType_G2();
    taskType_G2 = DNMS_DUAL_TASK_LEARNING;
    zxLaserSessions_G2(odorType, odorType + 1, laserOff, 13u, 26u, 24u,
                       WaterLen, 30, setSessionNum_G2());
    break;
  }

  case 4401: {
    lcdSplash_G2("DNMS Dual Task", "Laser");
    highLevelShuffleLength_G2 = 24;
    lcdSplash_G2("Odor", "");
    int odorType = setType_G2();
    taskType_G2 = DNMS_DUAL_TASK;
    laserSessionType_G2 = LASER_OTHER_TRIAL;
    laser_G2.ramp = 500;
    zxLaserSessions_G2(odorType, odorType + 1, laserAfterDistractorMax, 13u,
                       26u, 24u, WaterLen, 30, setSessionNum_G2());
    break;
  }

  // case 4410:
  //   write_eeprom_G2(EEP_DUTY_LOW_R_OFFSET, 0x7f);
  //   write_eeprom_G2(EEP_DUTY_LOW_L_OFFSET, 0x7f);
  //   break;

  case 4411:
    lcdSplash_G2("During Delay", "");
    //            setLaser();
    zxLaserSessions_G2(2, 3, laserDuringDelay, 4u, 8u, 20, WaterLen, 50,
                       setSessionNum_G2());
    break;

  case 4412: {
    lcdSplash_G2("Vary Delay", "Shaping");
    laserSessionType_G2 = LASER_NO_TRIAL;
    taskType_G2 = SHAPING_TASK;
    _delayT delay = setDelay_G2();
    int type = setType_G2();
    zxLaserSessions_G2(type, type + 1, laserDuringDelay, delay, delay * 2u, 20u,
                       WaterLen, 50, setSessionNum_G2());
    break;
  }

  //
  //        case 4413:
  //        {
  //            splash("DNMS LR Distra.", "Laser EveryTrial");
  //            taskType = DNMS_2AFC_TASK;
  //            laserTrialType = LASER_EVERY_TRIAL;
  //            zxLaserSessions(5, 6, laserDelayDistractor, 5u, 10u, 20,
  //            WaterLen,
  //            200, setSessionNum());
  //            break;
  //        }
  //        case 4414:
  //        {
  //            splash("Varying Delay", "DC laser,");
  //            taskType = DNMS_TASK;
  //            zxLaserSessions(5, 6, laserDuringDelay, 4u, 8u, 20, 0.125, 50,
  //            setSessionNum());
  //            break;
  //        }
  case 4415: {
    lcdSplash_G2("Varying Delay", "DC laser,");
    _delayT delay = setDelay_G2();
    taskType_G2 = DNMS_TASK;
    laserSessionType_G2 = setLaser_G2();
    int type = setType_G2();
    zxLaserSessions_G2(type, type + 1, laserDuringDelayChR2, delay, delay * 2u,
                       20u, WaterLen, 50, setSessionNum_G2());
    break;
  }

  //        case 4416:
  //        {
  //            splash("Varying Delay", "DC laser,");
  //            _delayT delay = setDelay();
  //            taskType = DNMS_TASK;
  //            laserSessionType = setLaser();
  //            zxLaserSessions(5, 6, laserDuringDelayChR2, delay, delay * 2u,
  //            20u, 0.036, 50, setSessionNum());
  //            break;
  //        }

  case 4418:
    lcdSplash_G2("Feed Water", "");
    feedWaterFast_G2(36);
    break;

  case 4419:
    lcdSplash_G2("Test Volume", "");
    testVolume_G2(36);
    break;

  case 4421: {
    lcdSplash_G2("DNMS Shaping", "");
    laserSessionType_G2 = LASER_NO_TRIAL;
    taskType_G2 = SHAPING_TASK;
    int type = setType_G2();
    zxLaserSessions_G2(type, type + 1, laserDuringDelay, 4u, 8u, 20, WaterLen,
                       50, setSessionNum_G2());
    break;
  }
  //        case 4422:
  //        {
  //            splash("DNMS Shaping", "Long Water");
  //            laserSessionType = LASER_NO_TRIAL;
  //            taskType = SHAPING_TASK;
  //            int type = setType();
  //            zxLaserSessions(type, type + 1, laserDuringDelay, 4u, 8u, 20,
  //            0.1, 50, 40);
  //            break;
  //        }
  case 4423:
    testValve_G2();
    break;

  case 4424:
    feedWaterFast_G2((int)(WaterLen * 1000));
    break;

  case 4425: {
    lcdSplash_G2("Learning", "DC laser");
    int type = setType_G2();
    zxLaserSessions_G2(type, type + 1, laserDuringDelay, 4u, 8u, 20, WaterLen,
                       60, setSessionNum_G2());
    break;
  }
  //        case 4426:
  //            shiftingLaser();
  //            break;

  case 4431: {
    lcdSplash_G2("VarDelay LR", "DC laser");
    _delayT delay = setDelay_G2();
    taskType_G2 = DNMS_2AFC_TASK;
    int type = setType_G2();
    zxLaserSessions_G2(type, type + 1, laserDuringDelay, delay, delay * 2u, 20u,
                       WaterLen, 50, setSessionNum_G2());
    break;
  }
  case 4432: {
    taskType_G2 = GONOGO_LR_TASK;
    lcdSplash_G2("GoNogo LR", "");
    unsigned int m = getFuncNumberGen2(1, "No Pre Odor Post");
    int laserPeriod;
    laserPeriod = laserDuring4Baseline;
    switch (m) {
    case 1:
      taskType_G2 = GONOGO_2AFC_TEACH;
      laserSessionType_G2 = LASER_NO_TRIAL;
      break;
    case 3:
      laserPeriod = laserDuring1stOdor;
      break;
    case 4:
      laserPeriod = laserDuringResponseDelay;
      break;
    }
    int type = setType_G2();
    zxLaserSessions_G2(type, type + 1, laserPeriod, 0u, 5u, 20, WaterLen, 50,
                       setSessionNum_G2());
    break;
  }

  case 4433: {
    lcdSplash_G2("DNMS LR", "Laser EveryTrial");
    taskType_G2 = DNMS_2AFC_TASK;
    laserSessionType_G2 = LASER_EVERY_TRIAL;
    zxLaserSessions_G2(5, 6, laserDuringDelayChR2, 5u, 10u, 20, WaterLen, 200,
                       setSessionNum_G2());
    break;
  }

  case 4434:
    laserTrain_G2();
    break;

  case 4435: {
    lcdSplash_G2("Baseline LR", "Laser EveryTrial");
    taskType_G2 = DNMS_2AFC_TASK;
    zxLaserSessions_G2(5, 6, laserDuring3Baseline, 5u, 10u, 20, WaterLen, 200,
                       setSessionNum_G2());
    break;
  }

  //        case 4436:
  //        {
  //            splash("DNMS LR", "Laser EveryTrial");
  //            taskType = _DNMS_LR_TASK;
  //            laserTrialType = _LASER_EVERY_TRIAL;
  //            zxLaserSessions(5,6, laserDelayDistractor, 5u, 10u, 20,
  //            WaterLen,
  //            200, setSessionNum());
  //            break;
  //        }

  case 4441: {
    lcdSplash_G2("First Odor LR", "Laser EveryTrial");
    taskType_G2 = DNMS_2AFC_TASK;
    zxLaserSessions_G2(5, 6, laserDuring1stOdor, 5u, 10u, 20, WaterLen, 200,
                       setSessionNum_G2());
    break;
  }
  case 4442: {
    lcdSplash_G2("Second Odor LR", "Laser EveryTrial");
    taskType_G2 = DNMS_2AFC_TASK;
    zxLaserSessions_G2(5, 6, laserDuring2ndOdor, 5u, 10u, 20, WaterLen, 200,
                       setSessionNum_G2());
    break;
  }
  case 4443: {
    lcdSplash_G2("Response Delay LR", "Laser EveryTrial");
    taskType_G2 = DNMS_2AFC_TASK;
    zxLaserSessions_G2(5, 6, laserDuringResponseDelay, 5u, 10u, 20, WaterLen,
                       200, setSessionNum_G2());
    break;
  }

  // case 4445:
  //     feedWaterLR_G2(50);
  //     break;

  case 4446: {
    unsigned int f = getFuncNumberMarquee_G2(3, "0123456798ABCDEFGHIJKLMN", 24);
    // lcdHomeNClear();
    lcdWriteNumber_G2(f, 4, 1, 1);
    while (1)
      ;
  }

  case 4450:
    varifyOpticalSuppression_G2();
    break;
  }


}
