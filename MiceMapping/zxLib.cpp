#include <LiquidCrystal.h>
#include "TimerOne.h"
#include "Arduino.h"
#include "commons.h"
#include "zxLib.h"
#include <EEPROM.h>

// LiquidCrystal lcd(50, 49, 48, 47, 46, 45, 44);

int waterValve = 21;
const int lickBound = 400; //Smaller is sensitive
boolean isLicking = false;
boolean hasLicked = false;
unsigned long targetTime;
int currentTrial, currentSession;
int laserTrialType;
unsigned int laserSessionType = LASER_EVERY_TRIAL;
float delayduration, ITI;
int  taskType, teachSesNo;
unsigned int highLevelShuffleLength = 12u;
boolean  multDistractor = false; // GNG number from 0 to 3
boolean GNGRorN = false; // reversal or not GNG
unsigned int DPAcurrentS = 1; // current odor for DPA  if ==2 odor pair reversal
int GNGPerf = 0, DPAPerf = 0;
int GNG_Switch_Num = 0, DPA_Switch_Num = 0;
int GNG_count = 0,  DPA_count = 0;
int GNG_array[20],   DPA_array[20];
boolean GNG_R_Shaping;
int DistractorSeq[4];
const char* odorTypes[] = {" ", "W", "B", "Y", "L", "R", "Q", "H", "N", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
                           "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
                           "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"
                          };
unsigned int sensorValue[2];
int DNMSstep;
int first_ITI = 0;
float second_ITI = 0;
boolean outTrial;
STIM_T stims = {.stim1Length = 1000u, .stim2Length = 1000u, .distractorLength = 500u, .currentDistractor = 5u, .distractorJudgingPair = 7u};
LASER_T laser = {.timer = 0u, .onTime = 65535u, .offTime = 0u, .ramp = 0u, .ramping = 0u, .on = 0u, .side = 1u}; //1L,2R,3LR


typedef unsigned int _delayT;


static void setWaterLen(){
  int newLen;
  EEPROM.get(sizeof(newLen), newLen);
  serialSend(SpLick,newLen);
  newLen = getFuncNumber(3, "Water Len in ms?");
  serialSend(SpLick,newLen);
  EEPROM.put(sizeof(newLen), newLen);
  serialSend(61, 0);
  callResetGen2();
}



void zxTimer1() {
  // serialSend(1,(analogRead(0) & 0b11111111));
  if (analogRead(0) < lickBound && !isLicking) { //Low is lick
    isLicking = true;
    //  digitalWrite(38, HIGH);
    serialSend(0, 1);
  } else if ( isLicking && (analogRead(0) > lickBound)) {
    isLicking = false;
    //  digitalWrite(38, LOW);
  }
  if (Serial.peek() == 0x2a) {
    serialSend(61,0);
    Timer1.detachInterrupt();
    callResetGen2();
  }
}


int getFuncNumber(int targetDigits, char* input) {
  int bitSet[targetDigits];
  int bitValue[targetDigits];
  unsigned int n;
  int iter;
  int iter1;

  for (iter = 0; iter < targetDigits; iter++) {
    bitSet[iter] = 0;
    bitValue[iter] = -6;
  }

  for (iter = 0; iter < targetDigits; iter++) {
    while (!bitSet[iter]) {
      if (Serial.peek() > 0) {
        if (Serial.peek() == 0x2a) {
          serialSend(61,0);
          Timer1.detachInterrupt();
          callResetGen2();
        }
        bitValue[iter] = Serial.read() - 0x30;
        bitSet[iter] = 1;
      }
    }
    serialSend(SpCome, bitValue[iter]);

  }
  n = 0;
  for (iter1 = 0; iter1 < targetDigits; iter1++) {
    n = n * 10 + bitValue[iter1];
  }
  return n;
}


void serialSend(int type,  int value) {
  byte toSend[] = {0x55, (byte) type, (byte) value, 0xAA};
  Serial.write(toSend, 4);
}


void resetTaskTimer() {
  targetTime = millis();
}

void waitTaskTimer(unsigned long t) {
  targetTime += t;
  while (millis() < targetTime);
}

void waitTrial() {

}

void assertLaserON(int type) {
  analogWrite(13, 255);
  serialSend(Splaser, (type != laserOff));
}

void assertLaserOFF(int type, boolean ramping) { // ramping
  if (ramping == true) {
    int value = 200;
    for (int i = 0; i < 200; i++) {
      analogWrite(13, value);
      delay(2);
      value--;
    }
    analogWrite(13, 0);
    waitTaskTimer(500u);
  } else {
    analogWrite(13, 0);
  }
  serialSend(Splaser, (type == laserOff));
}


boolean isLikeOdorA(int odor) {    // all in the list, FAorCR
  if (taskType == SIX_ODOR_DPA_SHAPING || taskType == SIX_ODOR_DPA_TRAINING) {
    if (odor == 1 || odor == 2 || odor == 6 ) return true;
    return false;
  }  else {   // DPA
    if (DPAcurrentS == 1) {
      if (odor == 1 || odor == 4 || odor == 6 || odor == 7) return true;
      return false;
    } else if (DPAcurrentS == 2) {  //DPA reversal
      if (odor == 1 || odor == 3 || odor == 6 || odor == 7) return true;
      return false;
    }
  }

}

void lcdWriteNumber(int n, int digits, int x, int y) {
  // lcd.setCursor(x - 1, y - 1);
  // lcd.print(n);
}


void FIDtest(int phase)   {
  int temp = 0;
  for (int i = 0; i < 5; i++) {
    temp += analogRead(1);
  }
  if (phase == 1) {
    sensorValue[1] = temp / 20; // end of ITI
  } else if  (phase == 2) {
    sensorValue[2] = temp / 20; // after odor valve off
  }

}

void lickTest(void) {
  int hi = analogRead(0) >> 2;
  int lo = analogRead(0) & 0x03;
  serialSend(0, hi);
  delay(10);
  serialSend(0, lo);
  delay(100);
}


void processMiss(int id) {
  currentMiss++;
  serialSend(SpMiss, id);
  lcdWriteNumber(++miss, 3, 10, 1);
}

void processFalse(int id) {
  currentMiss = 0;
  serialSend(SpFalseAlarm, id);
}

void processHit(float waterPeroid, int valve, int id) {
  serialSend(22, valve == 9 ? 1 : 2);
  Valve_ON(valve);
  wait_ms(waterPeroid * 1000);
  Valve_OFF(valve);
  currentMiss = 0;
  serialSend(SpHit, id);
}


void waterNResult(int firstOdor, int secondOdor, float waterPeroid, int id, int taskType, int currentSession) {
  hasLicked = false;

  for (unsigned long timeStamp = millis(); millis() < 500ul + timeStamp && !hasLicked; hasLicked = isLicking);

  /////Reward
  if (taskType == TEN_ODOR_DMS_SHAPING || taskType == TEN_ODOR_DMS_TRAINING || ((taskType == DNMS_SHAPING || taskType == FOUR_ODOR_DNMS_SHAPING ||
      taskType == DPA_DNMS_SHAPING || taskType == DNMS_TRAINING || taskType == FOUR_ODOR_DNMS_TRAINING || taskType == DPA_DNMS_TRAINING) && firstOdor > 4 )) {
    if (!hasLicked) {   // no lick
      if (firstOdor != secondOdor) {        // CR
        serialSend(SpCorrectRejection, id);
      } else {                 // miss
        processMiss(id);
        if (taskType == TEN_ODOR_DMS_SHAPING && currentSession <= teachSesNo) {
          serialSend(22, 1);
          Valve_ON(waterValve);
          serialSend(SpWater_sweet, 1);
          wait_ms(waterPeroid * 1000);
          Valve_OFF(waterValve);
        } else if (taskType == TEN_ODOR_DMS_SHAPING  && ((rand() % 3) == 0) && currentSession > teachSesNo) { //33% give water, 50% if %2
          //  mice will be delivered water 1/3 chance when miss in following session in GNG and DPA and DNMS trials
          serialSend(22, 1);
          Valve_ON(waterValve);
          serialSend(SpWater_sweet, 1);
          wait_ms(waterPeroid * 1000);
          Valve_OFF(waterValve);
        }
      }
    } else if (hasLicked) {
      if (firstOdor != secondOdor)  {
        processFalse(id);
      } else {
        processHit(waterPeroid, waterValve, id);
      }
    }
  }  else {    ////    DPA
    if (!hasLicked) {
      if (isLikeOdorA(firstOdor) == isLikeOdorA(secondOdor)) {
        serialSend(SpCorrectRejection, id);
        DPAPerf = 1;
        if (firstOdor > 4) {
          GNGPerf = 1;
        }
      } else if (isLikeOdorA(firstOdor) != isLikeOdorA(secondOdor)) {
        processMiss(id);
        DPAPerf = 0;
        if (firstOdor > 4) {
          GNGPerf = 0;
        }
        if ((taskType == DPA_SHAPING || taskType == DPA_R_SHAPING || taskType == DPA_GNG_SHAPING || taskType == GNG_TRAINING
             || taskType == DPA_R_SHAPING || taskType == DPA_DNMS_SHAPING || taskType == DPA_SHAPING_NoDelay)  && currentSession <= teachSesNo ) {
          // teach mice 1st session in DPA or GNG or DualTask
          serialSend(22, 1);
          Valve_ON(waterValve);
          serialSend(SpWater_sweet, 1);
          wait_ms(waterPeroid * 1000);
          Valve_OFF(waterValve);
        } else if ((taskType == DPA_SHAPING || taskType == DPA_R_SHAPING || taskType == DPA_GNG_SHAPING || taskType == GNG_TRAINING
                    || taskType == DPA_R_SHAPING || taskType == DPA_DNMS_SHAPING || taskType == DPA_SHAPING_NoDelay) && ((rand() % 3) == 0) && currentSession > teachSesNo) { //33% give water, 50% if %2
          //  mice will be delivered water 1/3 chance when miss in following session in GNG and DPA and DNMS trials
          serialSend(22, 1);
          Valve_ON(waterValve);
          serialSend(SpWater_sweet, 1);
          wait_ms(waterPeroid * 1000);
          Valve_OFF(waterValve);
        } else if ((taskType == DPA_GNG_SHAPING || taskType == GNG_TRAINING) && GNG_R_Shaping == true && GNG_count < 40 && firstOdor > 4) {
          // teach mice 10 GNG trials  when GNG reversed in dualtask or GNG
          serialSend(22, 1);
          Valve_ON(waterValve);
          serialSend(SpWater_sweet, 1);
          wait_ms(waterPeroid * 1000);
          Valve_OFF(waterValve);
        }
      }
    } else if (isLikeOdorA(firstOdor) == isLikeOdorA(secondOdor) && hasLicked) {
      processFalse(id);
      DPAPerf = 0;
      if (firstOdor > 4) {
        GNGPerf = 0;
      }
    } else {
      if (firstOdor > 4 && taskType == DPA_GNG_TRAINING) {
        GNGPerf = 1;
        processHit(0.03, waterValve, id);  // 25ms water for GNG
      } else {
        DPAPerf = 1;
        processHit(waterPeroid, waterValve, id); // ??ms water for DPA
      }
    }
  }

}


void stim(int place, int stim, int type) {
  FIDtest(1);

  Valve_ON(stim + 10);
  waitTaskTimer(500ul);

  if (place == 1) {
    if (type == laserDuring1stOdor_S || type == laserDuringTwoOdors_S ) {
      assertLaserON(type);
    } else if (((type == laserDuring1stOdor_D && stim < 3)  || (type == laserDuring2ndOdor_D  && stim > 4)) && taskType == DPA_DRT_TRAINING) {
      assertLaserON(type);
    } 
  } else if (place == 2 ) {
    if (((type == laserDuring4thOdor_D && stim < 5)  || (type == laserDuring3rdOdor_D  && stim > 4)) && taskType == DPA_DRT_TRAINING) {
      assertLaserON(type);
    } else if  (type == laserDuringResp_S || type == laserDuring2ndOdor_S) {
      assertLaserON(type);
    } else if ((type == laserDuring1stResp_D && stim == 7 ) || (type == laserDuring2ndResp_D && stim != 7)) {
      assertLaserON(type);
    }
  }

  Valve_ON(stim);
  int stimSend;
  if (stim == 1) {
    stimSend = SpOdor_A;
  } else if (stim == 2) {
    stimSend = SpOdor_B;
  } else if (stim == 3) {
    stimSend = SpOdor_C;
  } else if (stim == 4) {
    stimSend = SpOdor_D;
  } else if (stim == 5) {
    stimSend = SpOdor_E;
  } else if (stim == 6) {
    stimSend = SpOdor_F;
  } else if (stim == 7) {
    stimSend = SpOdor_G;
  } else if (stim == 8) {
    stimSend = SpOdor_H;
  } else if (stim == 9) {
    stimSend = SpOdor_I;
  } else if (stim == 10) {
    stimSend = SpOdor_J;
  }
  serialSend(stimSend, stim);
  delay(500);
  for (unsigned long timeStamp = millis(); millis() < 400ul + timeStamp;) {
    FIDtest(2);
    serialSend(FID_value, sensorValue[2] - sensorValue[1]);
    delay(200);
  }

  Valve_OFF(stim + 10);
  delay(100);
  Valve_OFF(stim);
//  if ((type == laserDuring1stOdor_S && place == 1) && ((type == laserDuring2ndOdor_S || type == laserDuringTwoOdors_S) && place == 2)) {
//    assertLaserOFF(type, false);
//  }
  if (place == 1) {
    if (type == laserDuring1stOdor_S ) {
      assertLaserOFF(type, false);
    } else if (((type == laserDuring1stOdor_D && stim < 3)  || (type == laserDuring2ndOdor_D  && stim > 4)) && taskType == DPA_DRT_TRAINING) {      
      assertLaserOFF(type, false);
    } 
  } else if (place == 2 ) {
    if (type == laserDuring2ndOdor_S || type == laserDuringTwoOdors_S) {
      assertLaserOFF(type, false);
    } else if (((type == laserDuring4thOdor_D && stim < 5)  || (type == laserDuring3rdOdor_D  && stim > 4)) && taskType == DPA_DRT_TRAINING) {
      assertLaserOFF(type, false);
    } 
  }
  serialSend(stimSend, 0);
  waitTaskTimer(1000u);
}

void shuffleArray(unsigned int * orgArray, unsigned int arraySize) {
  if (arraySize == 0 || arraySize == 1)
    return;

  int iter;
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


void distractor(int type, unsigned int distractOdor, unsigned int judgingPair, float waterLen, int taskType, int currentSession) {
  if (distractOdor == 0) {
    wait_ms(1500u);
  } else {
    GNG_count++;
    if (type == laserDuringOdor_Resp_GNG) {
      assertLaserON(type);
    }
    Valve_ON(distractOdor + 10);
    wait_ms(500u);
    Valve_ON(distractOdor);
    serialSend(isLikeOdorA(distractOdor) ? SpOdor_G : SpOdor_H, distractOdor);
    wait_ms(stims.distractorLength - 50u);
    Valve_OFF(distractOdor + 10);
    wait_ms(50u);;
    Valve_OFF(distractOdor);
    serialSend(isLikeOdorA(distractOdor) ? SpOdor_G : SpOdor_H, 0);
    FIDtest(2);
    serialSend(FID_value, sensorValue[2] - sensorValue[1]);

    wait_ms(500u);  // hold on 500ms

    int id = (taskType == GNG_TRAINING)  ? 1 : 3;
    waterNResult(distractOdor, judgingPair, waterLen, id, taskType, currentSession);
    if (type == laserDuringOdor_Resp_GNG) {
      assertLaserOFF(type, true);
    } else if (type == laserDuring2ndDelay_D) {
      assertLaserON(type);
    }
    GNG_array[(GNG_count - 1) % 20] = GNGPerf;
  }


  //  if (GNG_count > 29 && GNGRorN == true) {
  //    int sum = 0;
  //    for (int i = 0; i < 20; i++) {
  //      sum += GNG_array[i];
  //    }
  //    if (sum > 16) {
  //      GNG_array[20];
  //      GNG_Switch_Num++;
  //      GNG_count = 0;
  //      serialSend(GNG_Revrsal_Point, GNG_Switch_Num);
  //      if (stims.distractorJudgingPair == 6u) {
  //        stims.distractorJudgingPair = 5u;
  //      } else if (stims.distractorJudgingPair == 5u) {
  //        stims.distractorJudgingPair = 6u;
  //      }
  //    }
  //  }
}


void DPATrial(int type, int firstOdor, STIM_T odors, int delayduration, int secondOdor, float waterPeroid, float ITI,
              int taskType, int currentSession, float first_ITI, float second_ITI, boolean outTrial);


int DRTTraining(int type, int firstOdor, STIM_T odors, int delayduration, int secondOdor, float waterPeroid, float ITI, int taskType, int currentSession) {
  resetTaskTimer();
  serialSend(Sptrialtype, type);
  serialSend(Splaser, type);

  hasLicked = false;
  Valve_ON(firstOdor + 10);
  waitTaskTimer(500ul);
  Valve_ON(firstOdor);
  int stimSend;
  if (firstOdor == 5) {
    stimSend = SpOdor_E;
  } else if (firstOdor == 6) {
    stimSend = SpOdor_F;
  }
  serialSend(stimSend, firstOdor);
  waitTaskTimer(900ul);
  Valve_OFF(firstOdor + 10);
  waitTaskTimer(100ul);
  Valve_OFF(firstOdor);
  serialSend(stimSend, 0);

  if (type == laserDuringDelay_S) {
    assertLaserON(type);
  }
  unsigned long timeStamp;
  for (timeStamp = millis(); millis() < delayduration * 1000ul + timeStamp && !hasLicked; hasLicked = isLicking) { // Check delay lick
    if ( millis() == timeStamp + ( delayduration - 0.5 ) * 1000 ) {
      Valve_ON(secondOdor + 10); // 3-way valve
    }
  }
  if (type == laserDuringDelay_S) {
    assertLaserOFF(type, false);
  }
  waitTaskTimer(delayduration * 1000ul);

  if (hasLicked) {
    Valve_OFF(secondOdor + 10); // 3-way valve
    serialSend(SpITI, 0);
    unsigned long licktimeStamp = millis();
    float timeoutduration = ( delayduration + 0.5 + 1 ) * 1000 - ( licktimeStamp - timeStamp );
    timeStamp = millis();
    while (millis() < timeoutduration + timeStamp ) {
    };
    return 1;  // isdelaylick = 1
  }
  else {
    Valve_ON(secondOdor);;  // ResponseOdor
    serialSend(SpOdor_G, secondOdor);
    waitTaskTimer(900ul);
    Valve_OFF(secondOdor + 10);
    waitTaskTimer(100ul);
    Valve_OFF(secondOdor);
    serialSend(SpOdor_G, 0);

    if ( currentSession <= teachSesNo && firstOdor == 5) {   //
      waitTaskTimer(500ul);
      Valve_ON(waterValve);
      serialSend(SpWater_sweet, 1);
      wait_ms((waterPeroid) * 1000);
      Valve_OFF(waterValve);
      waitTaskTimer(500ul);
    } else {
      waitTaskTimer(1000ul);
    }
    int id = 1;
    waterNResult(firstOdor, secondOdor, waterPeroid, id, taskType, currentSession);
    waitTaskTimer(550ul); //water time sync
    serialSend(SpITI, 0);
    resetTaskTimer();
    waitTaskTimer((ITI - 0.5) * 1000ul);
    return 0;  // isdelaylick = 0
  }

}


void DRTTrial(int type, int firstOdor, STIM_T odors, int delayduration, int secondOdor, float waterPeroid, float ITI, int taskType, int currentSession) {

  if (taskType == DRT_TRAINING || taskType == DRT_DPA_TRAINING || taskType == DRT_DPA_SHAPING) {
    resetTaskTimer();
    serialSend(Sptrialtype, type);
    serialSend(Splaser, type);
  } else if (taskType == DPA_DRT_SHAPING || taskType == DPA_DRT_TRAINING) {
    waitTaskTimer(first_ITI * 1000ul);
  }

  ///////////////////////////////////
  stim(1, firstOdor, type);
  ////////////////////////////////////
  waitTaskTimer(500ul);

  if (taskType == DRT_TRAINING && (teachSesNo == 0 || currentSession > teachSesNo))  {
    if (type == laserDuringDelay_S) {
      assertLaserON(type);
      waitTaskTimer((delayduration - 1.5)  * 1000ul);
      assertLaserOFF(type, true);
    } else if (type == laserDuringEarlyDelay_S) {
      assertLaserON(type);
      waitTaskTimer((delayduration - 1) * 1000ul / 2 - 500ul);  // ramping down
      assertLaserOFF(type, true);
      waitTaskTimer(delayduration / 2 * 1000ul - 500ul);
    } else  if (type == laserDuringLateDelay_S) {
      waitTaskTimer((delayduration - 1) * 1000ul / 2);
      assertLaserON(type);
      waitTaskTimer((delayduration - 1) * 1000ul / 2 - 500ul);  // ramping down
      assertLaserOFF(type, true);
    } else {
      waitTaskTimer((delayduration - 1) * 1000ul);
    }
  }  else if (taskType == DRT_TRAINING &&  currentSession <= teachSesNo)  {
    waitTaskTimer((delayduration - 1) * 1000ul);
    ///////////////////////
    ///   dual task     ///
    ///////////////////////
  } else if (taskType == DPA_DRT_SHAPING || taskType == DPA_DRT_TRAINING) {
    if (type == laserDuring2ndDelay_D) {
      assertLaserON(type);
      waitTaskTimer(2500ul);
      assertLaserOFF(type, true);
    } else {
      waitTaskTimer(3000u);
    }
  } else if (taskType == DRT_DPA_TRAINING || taskType == DRT_DPA_SHAPING) {
    if (DistractorSeq[1] != 0) {  // Dual Task
      if (type == laserDuringDelay_D) {  // early delay WITH distractor
        assertLaserON(type);
      }
      DPATrial(laserTrialType, DistractorSeq[1], stims, delayduration, DistractorSeq[2], WaterLen, ITI, taskType, currentSession, first_ITI, second_ITI, false);
      if (type == laserDuringDelay_D || type == laserDuring3rdDelay_D) {  // early delay WITH distractor
        assertLaserOFF(type, true);
      } else {
        waitTaskTimer(500ul);
      }
    } else if ((DistractorSeq[1] == 0)) {
      if (type == laserDuringDelay_D) {  // early delay WITHOUT distractor
        assertLaserON(type);
        waitTaskTimer(12500ul);
        assertLaserOFF(type, true);
      } else if (type == laserDuring2ndDelay_D) {
        waitTaskTimer(4000ul);
        assertLaserON(type);
        waitTaskTimer(2500ul);
        assertLaserOFF(type, true);
        waitTaskTimer(6000ul);
      } else if (type == laserDuring3rdDelay_D) {
        waitTaskTimer(10050ul);
        assertLaserON(type);
        waitTaskTimer(2450ul);
        assertLaserOFF(type, true);
      } else {
        waitTaskTimer(13000ul);
      }
    }
  }
  //////////////////////////////////////////
  stim(2, secondOdor, type);
  ////////////////////////////////////////////

  if (taskType == DPA_DRT_TRAINING)  {
    if (type == laserDuring1stResp_D) {
      waitTaskTimer(500ul);
      assertLaserOFF(type, true);
    } else {
      waitTaskTimer(1000ul);
    }
  } else if (taskType == DRT_DPA_TRAINING) {
    if (type == laserDuring2ndResp_D) {
      waitTaskTimer(500ul);
      assertLaserOFF(type, true);
    } else {
      waitTaskTimer(1000ul);
    }
  }  else if (taskType == DRT_TRAINING || taskType == DPA_DRT_SHAPING) {
    if (teachSesNo == 0 || currentSession > teachSesNo) {
      if (type == laserDuring2ndOdor_S) {
        assertLaserOFF(type, false);
        waitTaskTimer(1000ul);
      } else if (type == laserDuringResp_S) {
        waitTaskTimer(500ul);
        assertLaserOFF(type, true);
      } else {
        waitTaskTimer(1000ul);
      }
    } else if (currentSession <= teachSesNo && firstOdor == 5) {  //
      waitTaskTimer(500ul);
      Valve_ON(waterValve);
      serialSend(SpWater_sweet, 1);
      wait_ms((waterPeroid) * 1000);
      Valve_OFF(waterValve);
      waitTaskTimer(500ul);
    } else {
      waitTaskTimer(1000ul);
    }
  }


  //Assess Performance here
  int id = (taskType == DRT_TRAINING || taskType == DRT_DPA_TRAINING || taskType == DRT_DPA_SHAPING) ? 1 : 3;

  waterNResult(firstOdor, secondOdor, waterPeroid, id, taskType, currentSession);
  waitTaskTimer(550ul); //water time sync

  ///--ITI1---///


  if (taskType == DRT_TRAINING || taskType == DRT_DPA_TRAINING ) {
    serialSend(SpITI, 0);
    if (type == laserDuringITI_S) {
      assertLaserON(type);
      waitTaskTimer(500ul);
    } else {
      waitTaskTimer(500ul);
    }
  } else if (taskType == DPA_DRT_SHAPING || taskType == DPA_DRT_TRAINING) {

    if (type == laserDuring3rdDelay_D ) {
      assertLaserON(type);
      waitTaskTimer(500ul);
    } else {
      waitTaskTimer(500ul);
    }
  } else if (taskType == DRT_DPA_TRAINING || taskType == DRT_DPA_SHAPING) {
    serialSend(SpITI, 0);
    //    if (type == laserDuringITI_D) {
    //      assertLaserON(type);
    //      waitTaskTimer(500ul);
    //    } else {
    waitTaskTimer(500ul);
    //    }
  }

  // Total Trials
  int totalTrials = hit + correctRejection + miss + falseAlarm;

  // Discrimination rate
  if (hit + correctRejection > 0) {
    correctRatio = 100 * (hit + correctRejection) / totalTrials;
  }

  resetTaskTimer();

  if (taskType == DRT_TRAINING || taskType == DRT_DPA_TRAINING) {
    if (type == laserDuringITI_S) {
      waitTaskTimer((ITI - 1.5) * 1000ul);
      assertLaserOFF(type, true);
    } else {
      waitTaskTimer((ITI - 1) * 1000ul); //another 4000 is at the beginning of the trials.
    }
  } else if (taskType == DPA_DRT_SHAPING || taskType == DPA_DRT_TRAINING) {
    waitTaskTimer((second_ITI - 0.5) * 1000ul);
  }
  waitTrial();
}



void DNMSTrial(int type, int firstOdor, STIM_T odors, int delayduration, int secondOdor, float waterPeroid, float ITI,
               int taskType, int currentSession, float first_ITI, float second_ITI, boolean outTrial) {

  if (taskType == DNMS_TRAINING || taskType == DNMS_SHAPING || taskType == FOUR_ODOR_DNMS_SHAPING  || taskType == FOUR_ODOR_DNMS_TRAINING ||
      taskType == TEN_ODOR_DMS_SHAPING  || taskType == TEN_ODOR_DMS_TRAINING) {
    resetTaskTimer();
    serialSend(Sptrialtype, type);
    serialSend(Splaser, type);
  }  else if (taskType == DPA_DNMS_SHAPING || taskType == DPA_DNMS_TRAINING) {
    resetTaskTimer();
    serialSend(Sptrialtype, type);
    serialSend(Splaser, type);
  } else if ((taskType == DPA_DNMS_SHAPING || taskType == DPA_DNMS_TRAINING ) && outTrial == false ) {
    waitTaskTimer(first_ITI  * 1000ul);
  }

  /////////////////////////////
  stim(1, firstOdor, type);
  /////////////////////////////

  waitTaskTimer(500ul);

  if (taskType == DNMS_TRAINING || taskType == DNMS_SHAPING || taskType == FOUR_ODOR_DNMS_SHAPING  || taskType == FOUR_ODOR_DNMS_TRAINING) {   //  non Dual Task
    if (type == laserDuringDelay_S) {
      assertLaserON(type);
      waitTaskTimer((delayduration - 1.5)  * 1000ul);
      assertLaserOFF(type, true);
    } else if (type == laserDuringEarlyDelay_S) {
      assertLaserON(type);
      waitTaskTimer( (delayduration - 1) * 1000ul / 2 - 500ul); // ramping down
      assertLaserOFF(type, true);
      waitTaskTimer(delayduration * 1000ul / 2 - 500ul);
    } else  if (type == laserDuringLateDelay_S) {
      waitTaskTimer((delayduration - 1) * 1000ul / 2);
      assertLaserON(type);
      waitTaskTimer((delayduration - 1) * 1000ul / 2 - 500ul); // ramping down
      assertLaserOFF(type, true);
    } else {
      waitTaskTimer((delayduration - 1) * 1000ul);
    }
  } else if (taskType == DPA_DNMS_SHAPING || taskType == DPA_DNMS_TRAINING) {
    if (type == laserDuring2ndDelay_D) {
      assertLaserON(type);
      waitTaskTimer(2500ul);
      assertLaserOFF(type, true);
    } else {
      waitTaskTimer(3000u);
    }
  }

  //////////////////////////////////////////
  stim(2, secondOdor, type);
  ////////////////////////////////////////////


  // Decision Making Period
  if (taskType == DNMS_TRAINING || taskType == FOUR_ODOR_DNMS_TRAINING || taskType == TEN_ODOR_DMS_TRAINING) {
    if (type == laserDuringResp_S) {
      waitTaskTimer(500ul);
      assertLaserOFF(type, true);
    } else {
      waitTaskTimer(1000ul);
    }
  } else if ( taskType == DNMS_SHAPING  || taskType == FOUR_ODOR_DNMS_SHAPING || taskType == TEN_ODOR_DMS_SHAPING) {
    if (currentSession <= teachSesNo) {
      waitTaskTimer(500ul);
      Valve_ON(waterValve);
      serialSend(SpWater_sweet, 1);
      wait_ms((waterPeroid) * 1000);
      Valve_OFF(waterValve);
      waitTaskTimer(500ul);
    } else {
      waitTaskTimer(1000ul);
    }
  } else if (taskType == DPA_DNMS_SHAPING || taskType == DPA_DNMS_TRAINING) {
    if (type == laserDuring1stResp_D) {
      waitTaskTimer(500ul);
      assertLaserOFF(type, true);
    } else {
      waitTaskTimer(1000ul);
    }
  }

  //Assess Performance here
  int id;
  if (taskType == DNMS_TRAINING || taskType == DNMS_SHAPING || taskType == FOUR_ODOR_DNMS_SHAPING  || taskType == FOUR_ODOR_DNMS_TRAINING
      || taskType == TEN_ODOR_DMS_SHAPING || taskType == TEN_ODOR_DMS_TRAINING) {
    id = 1;
  } else if (taskType == DPA_DNMS_SHAPING || taskType == DPA_DNMS_TRAINING) {
    id = 3;
  }
  waterNResult(firstOdor, secondOdor, waterPeroid, id, taskType, currentSession); // always DNMS trial
  waitTaskTimer(550ul); //water time sync


  ///--ITI1---///

  if (taskType == DNMS_SHAPING || taskType == DNMS_TRAINING || taskType == FOUR_ODOR_DNMS_SHAPING || taskType == FOUR_ODOR_DNMS_TRAINING) {
    serialSend(SpITI, 0);
    if (type == laserDuringITI_S) {
      assertLaserON(type);
      waitTaskTimer(500ul);
    } else {
      waitTaskTimer(500ul);
    }
  } else if (taskType == DPA_DNMS_SHAPING || taskType == DPA_DNMS_TRAINING) {
    if (type == laserDuring3rdDelay_D ) {
      assertLaserON(type);
      waitTaskTimer(500ul);
    } else {
      waitTaskTimer(500ul);
    }
  }


  // Total Trials
  int totalTrials = hit + correctRejection + miss + falseAlarm;

  resetTaskTimer();

  if (taskType == DNMS_TRAINING || taskType == DNMS_SHAPING  || taskType == FOUR_ODOR_DNMS_SHAPING  || taskType == FOUR_ODOR_DNMS_TRAINING
      || taskType == TEN_ODOR_DMS_SHAPING || taskType == TEN_ODOR_DMS_TRAINING) {
    if (type == laserDuringITI_S) {
      waitTaskTimer((ITI - 1.5) * 1000ul);
      assertLaserOFF(type, true);
    } else {
      waitTaskTimer(ITI * 1000ul - 500ul);
    }
  } else if (taskType == DPA_DNMS_SHAPING || taskType == DPA_DNMS_TRAINING) {
    waitTaskTimer((second_ITI - 0.5) * 1000ul);
  }
  waitTrial();
}

void DPATrialNoDelay(int type, int firstOdor, STIM_T odors, int delayduration, int secondOdor, float waterPeroid, float ITI, int taskType,
                     int currentSession, float first_ITI, float second_ITI, boolean outTrial) {

  resetTaskTimer();
  serialSend(Sptrialtype, type);
  serialSend(Splaser, type);

  Valve_ON(firstOdor + 10); // 1st odor's three-port valve
  waitTaskTimer(500ul);

  if (type == laserDuringTwoOdors_S) {
    assertLaserON(type);
  }

  int stimSend1, stimSend2;
  Valve_ON(firstOdor);   // 1st odor's two-port valve
  if (firstOdor == 1) {
    stimSend1 = SpOdor_A;
  } else if (firstOdor == 2) {
    stimSend1 = SpOdor_B;
  } else if (firstOdor == 3) {
    stimSend1 = SpOdor_C;
  } else if (firstOdor == 4) {
    stimSend1 = SpOdor_D;
  } else if (firstOdor == 5) {
    stimSend1 = SpOdor_E;
  } else if (firstOdor == 6) {
    stimSend1 = SpOdor_F;
  } else if (firstOdor == 7) {
    stimSend1 = SpOdor_G;
  } else if (firstOdor == 8) {
    stimSend1 = SpOdor_H;
  } else if (firstOdor == 9) {
    stimSend1 = SpOdor_I;
  } else if (firstOdor == 10) {
    stimSend1 = SpOdor_J;
  }
  serialSend(stimSend1, firstOdor);
  waitTaskTimer(400);

  Valve_ON(secondOdor + 10); // 2nd odor's three-port valve
  waitTaskTimer(500);

  Valve_ON(secondOdor);    // 2nd odor's two-port valve
  if (secondOdor == 1) {
    stimSend2 = SpOdor_A;
  } else if (secondOdor == 2) {
    stimSend2 = SpOdor_B;
  } else if (secondOdor == 3) {
    stimSend2 = SpOdor_C;
  } else if (secondOdor == 4) {
    stimSend2 = SpOdor_D;
  } else if (secondOdor == 5) {
    stimSend2 = SpOdor_E;
  } else if (secondOdor == 6) {
    stimSend2 = SpOdor_F;
  } else if (secondOdor == 7) {
    stimSend2 = SpOdor_G;
  } else if (secondOdor == 8) {
    stimSend2 = SpOdor_H;
  } else if (secondOdor == 9) {
    stimSend2 = SpOdor_I;
  } else if (secondOdor == 10) {
    stimSend2 = SpOdor_J;
  }
  serialSend(stimSend2, secondOdor);
  waitTaskTimer(900);

  Valve_OFF(firstOdor + 10);
  Valve_OFF(secondOdor + 10);

  waitTaskTimer(100);

  Valve_OFF(firstOdor);
  Valve_OFF(secondOdor);
  serialSend(stimSend1, 0);
  serialSend(stimSend2, 0);
  if (type == laserDuringTwoOdors_S) {
    assertLaserOFF(type, false);
  }

  if (taskType == DPA_SHAPING_NoDelay && currentSession <= teachSesNo) {
    waitTaskTimer(500ul);
    Valve_ON(waterValve);
    serialSend(SpWater_sweet, 1);
    wait_ms((waterPeroid) * 1000);
    Valve_OFF(waterValve);
    waitTaskTimer(500ul);
  } else {
    waitTaskTimer(1000ul);
  }

  int id = 1;
  waterNResult(firstOdor, secondOdor, waterPeroid, id, taskType, currentSession);
  waitTaskTimer(550ul); //water time sync

  resetTaskTimer();
  serialSend(SpITI, 0);
  waitTaskTimer(ITI * 1000ul);

}


void DPATrial(int type, int firstOdor, STIM_T odors, int delayduration, int secondOdor, float waterPeroid, float ITI, int taskType,
              int currentSession, float first_ITI, float second_ITI, boolean outTrial) {

  if (taskType == DPA_TRAINING || taskType == DPA_SHAPING || taskType == SIX_ODOR_DPA_SHAPING || taskType == SIX_ODOR_DPA_TRAINING) {
    resetTaskTimer();
    serialSend(Sptrialtype, type);
    serialSend(Splaser, type);
  } else if (taskType == DPA_DNMS_SHAPING || taskType == DPA_DNMS_TRAINING || taskType == DPA_GNG_TRAINING || taskType == DPA_GNG_TRAINING ||  taskType == DPA_DRT_SHAPING || taskType == DPA_DRT_TRAINING) {
    resetTaskTimer();
    serialSend(Sptrialtype, type);
    serialSend(Splaser, type);
  } else if (taskType == DRT_DPA_TRAINING || taskType == DRT_DPA_SHAPING) {
    waitTaskTimer(first_ITI * 1000ul);
  }

  DPA_count++;

  ///////////////////////////////////
  stim(1, firstOdor, type);
  ////////////////////////////////////

  waitTaskTimer(500ul);

  if (taskType == DPA_TRAINING || taskType == DPA_SHAPING || taskType == SIX_ODOR_DPA_SHAPING || taskType == SIX_ODOR_DPA_TRAINING)  {
    if (type == laserDuringDelay_S) {
      assertLaserON(type);
      waitTaskTimer((delayduration - 1.5)  * 1000ul);
      assertLaserOFF(type, true);
    } else if (type == laserDuringEarlyDelay_S) {
      assertLaserON(type);
      waitTaskTimer((delayduration - 1) * 1000ul / 2 - 500ul);  // ramping down
      assertLaserOFF(type, true);
      waitTaskTimer(delayduration / 2 * 1000ul - 500ul);
    } else  if (type == laserDuringLateDelay_S) {
      waitTaskTimer((delayduration - 1) * 1000ul / 2);
      assertLaserON(type);
      waitTaskTimer((delayduration - 1) * 1000ul / 2 - 500ul);  // ramping down
      assertLaserOFF(type, true);
    } else if (delayduration == 0) {

    } else {
      waitTaskTimer((delayduration - 1) * 1000ul);
    }
  } else if (taskType == DRT_DPA_TRAINING || taskType == DRT_DPA_SHAPING) {
    if (type == laserDuring2ndDelay_D) {
      assertLaserON(type);
      waitTaskTimer(2500ul);
      assertLaserOFF(type, true);
    } else {
      waitTaskTimer(3000u);
    }
    /////////////////////////////
    //        dual task        //
    /////////////////////////////
  } else if (taskType == DPA_GNG_TRAINING ) {  // whole delay is 8s
    if  (stims.currentDistractor == 0u) {
      if (type == laserDuringDelay_D) {
        assertLaserON(type);
        waitTaskTimer(6500ul);
        assertLaserOFF(type, true);
      } else if (type == laserDuring2ndDelay_D) {
        waitTaskTimer(4500ul);
        assertLaserON(type);
        waitTaskTimer(2000ul);
        assertLaserOFF(type, true);
      } else {
        waitTaskTimer(7000ul);
      }
    } else {
      if (type == laserDuringDelay_D) {   // early delay
        assertLaserON(type);
        waitTaskTimer(2500ul);
      } else {
        waitTaskTimer(2500ul);
      }
      distractor(type, stims.currentDistractor, stims.distractorJudgingPair, waterPeroid, taskType, currentSession);
      waitTaskTimer(1500ul);
      if (type == laserDuringDelay_D || type == laserDuring2ndDelay_D) {
        waitTaskTimer(2500ul);
        assertLaserOFF(type, true);
      } else {
        waitTaskTimer(3000ul);
      }
    }

  } else if (taskType == DPA_DRT_SHAPING || taskType == DPA_DRT_TRAINING) {
    if (DistractorSeq[1] != 0) {  // Dual Task
      if (type == laserDuringDelay_D) {  // early delay WITH distractor
        assertLaserON(type);
      }
      DRTTrial(type, DistractorSeq[1], stims, 4, DistractorSeq[2], waterPeroid, ITI, taskType, currentSession);
      if (type == laserDuringDelay_D || type == laserDuring3rdDelay_D) {  // early delay WITH distractor
        assertLaserOFF(type, true);
      } else {
        waitTaskTimer(500ul);
      }
    } else if ((DistractorSeq[1] == 0)) {
      if (type == laserDuringDelay_D) {  // early delay WITHOUT distractor
        assertLaserON(type);
        waitTaskTimer(12500ul);
        assertLaserOFF(type, true);
      } else if (type == laserDuring2ndDelay_D) {
        waitTaskTimer(4000ul);
        assertLaserON(type);
        waitTaskTimer(2500ul);
        assertLaserOFF(type, true);
        waitTaskTimer(6000ul);
      } else if (type == laserDuring3rdDelay_D) {
        waitTaskTimer(10050ul);
        assertLaserON(type);
        waitTaskTimer(2450ul);
        assertLaserOFF(type, true);
      } else {
        waitTaskTimer(13000ul);
      }
    }

  } else if (taskType == DPA_DNMS_SHAPING || taskType == DPA_DNMS_TRAINING) {
    if (DistractorSeq[1] != 0) {  // Dual Task
      if (type == laserDuringDelay_D) {  // early delay WITH distractor
        assertLaserON(type);
      }
      DNMSTrial(type, DistractorSeq[1], stims, 4, DistractorSeq[2], waterPeroid, ITI, taskType, currentSession, first_ITI, second_ITI, false);
      if (type == laserDuringDelay_D || type == laserDuring3rdDelay_D) {  // early delay WITH distractor
        assertLaserOFF(type, true);
      } else {
        waitTaskTimer(500ul);
      }
    } else if ((DistractorSeq[1] == 0)) {
      if (type == laserDuringDelay_D) {  // early delay WITHOUT distractor
        assertLaserON(type);
        waitTaskTimer(12500ul);
        assertLaserOFF(type, true);
      } else if (type == laserDuring2ndDelay_D) {
        waitTaskTimer(4500ul);
        assertLaserON(type);
        waitTaskTimer(2500ul);
        assertLaserOFF(type, true);
        waitTaskTimer(5500ul);
      } else if (type == laserDuring3rdDelay_D) {
        waitTaskTimer(10550ul);
        assertLaserON(type);
        waitTaskTimer(2450ul);
        assertLaserOFF(type, true);
      } else {
        waitTaskTimer(13000ul);
      }
    }
  }

  //////////////////////////////////////////
  stim(2, secondOdor, type);
  ////////////////////////////////////////////

  if ((taskType == DPA_DRT_TRAINING || taskType == DPA_DNMS_TRAINING || taskType == DPA_GNG_TRAINING) && (type == laserDuring2ndResp_D)) {
    waitTaskTimer(500ul);
    assertLaserOFF(type, true);
  } else if ((taskType == DRT_DPA_TRAINING  || taskType == DRT_DPA_SHAPING) && (type == laserDuring1stResp_D)) {
    waitTaskTimer(500ul);
    assertLaserOFF(type, true);
  } else if (taskType == DPA_TRAINING ||  taskType == SIX_ODOR_DPA_TRAINING) {
    if (type == laserDuringResp_S) {
      waitTaskTimer(500ul);
      assertLaserOFF(type, true);
    } else {
      waitTaskTimer(1000ul);
    }
  } else if ((taskType == DPA_SHAPING || taskType == SIX_ODOR_DPA_SHAPING || taskType == DPA_DRT_SHAPING || taskType == DPA_DNMS_SHAPING) && (currentSession <= teachSesNo)) {
    waitTaskTimer(500ul);
    Valve_ON(waterValve);
    serialSend(SpWater_sweet, 1);
    wait_ms((waterPeroid) * 1000);
    Valve_OFF(waterValve);
    waitTaskTimer(500ul);
  } else {
    waitTaskTimer(1000ul);
  }


  //Assess Performance here
  int id;
  if (taskType == DPA_TRAINING || taskType == DPA_SHAPING || taskType == SIX_ODOR_DPA_SHAPING || taskType == SIX_ODOR_DPA_TRAINING) {
    id = 1;
  } else if (taskType == DPA_DNMS_SHAPING || taskType == DPA_DNMS_TRAINING || taskType == DPA_GNG_TRAINING || taskType == DPA_GNG_TRAINING ||
             taskType == DPA_DRT_SHAPING || taskType == DPA_DRT_TRAINING || taskType == DRT_DPA_SHAPING || taskType == DRT_DPA_TRAINING) {
    id = (outTrial == 1)  ? 1 : 3;
  }

  waterNResult(firstOdor, secondOdor, waterPeroid, id, taskType, currentSession);

  waitTaskTimer(550ul); //water time sync


  ///--ITI1---///

  if (taskType == DPA_TRAINING || taskType == DPA_SHAPING || taskType == SIX_ODOR_DPA_SHAPING || taskType == SIX_ODOR_DPA_TRAINING) {
    serialSend(SpITI, 0);
    if (type == laserDuringITI_S) {
      assertLaserON(type);
      waitTaskTimer(500ul);
    } else {
      waitTaskTimer(500ul); //another 4000 is at the beginning of the trials.
    }

  } else if (taskType == DPA_DNMS_SHAPING || taskType == DPA_DNMS_TRAINING || taskType == DPA_GNG_TRAINING || taskType == DPA_DRT_SHAPING || taskType == DPA_DRT_TRAINING) {
    serialSend(SpITI, 0);
    //    if (type == laserDuringITI_D) {
    //      assertLaserON(type);
    //      waitTaskTimer(500ul);
    //    } else {
    waitTaskTimer(500ul); //another 4000 is at the beginning of the trials.
    //    }
  } else if (taskType == DRT_DPA_TRAINING || taskType == DRT_DPA_SHAPING) {
    if (type == laserDuring3rdDelay_D) {
      assertLaserON(type);
      waitTaskTimer(500ul);
    } else {
      waitTaskTimer(500ul); //another 4000 is at the beginning of the trials.
    }
  }

  // Total Trials
  int totalTrials = hit + correctRejection + miss + falseAlarm;

  // Discrimination rate
  if (hit + correctRejection > 0) {
    correctRatio = 100 * (hit + correctRejection) / totalTrials;
  }

  resetTaskTimer();

  if (taskType == DPA_TRAINING || taskType == DPA_SHAPING || taskType == SIX_ODOR_DPA_SHAPING || taskType == SIX_ODOR_DPA_TRAINING) {
    if (type == laserDuringITI_S) {
      waitTaskTimer((ITI - 1.5) * 1000ul);
      assertLaserOFF(type, true);
    } else {
      waitTaskTimer((ITI - 1) * 1000ul); //another 4000 is at the beginning of the trials.
    }
  } else if (taskType == DPA_DNMS_SHAPING || taskType == DPA_DNMS_TRAINING || taskType == DPA_GNG_TRAINING || taskType == DPA_DRT_SHAPING || taskType == DPA_DRT_TRAINING) {
    //    if (type == laserDuringITI_D) {
    //      waitTaskTimer((ITI - 1.5) * 1000ul);
    //      assertLaserOFF(type, true);
    //    } else {
    waitTaskTimer((ITI - 1) * 1000ul);
    //    }
  } else if (taskType == DRT_DPA_TRAINING  || taskType == DRT_DPA_SHAPING) {
    waitTaskTimer((second_ITI - 0.5) * 1000ul);
  }
  waitTrial();

  //  DPA_array[(DPA_count - 1) % 20] = DPAPerf;

  //  if (DPA_count > 39 && taskType == DPA_R_TRAINING) {
  //    int sum = 0;
  //    for (int i = 0; i < 20; i++) {
  //      sum += DPA_array[i];
  //    }
  //    if (sum > 16) {
  //      DPA_array[20];
  //      DPA_Switch_Num++;
  //      DPA_count = 0;
  //      serialSend(DPA_Revrsal_Point, DPA_Switch_Num);
  //      if (DPAcurrentS == 1) {
  //        DPAcurrentS = 2;
  //      } else if (DPAcurrentS == 2) {
  //        DPAcurrentS = 1;
  //      }
  //    }
  //  }
}


static void  GNGSession(int type, unsigned int ITI, int trialsPerSession, float WaterLen, int missLimit, int totalSession, int taskType) {
  int currentTrial = 0;
  currentSession = 0;
  int laserOnType = laserTrialType;
  int firstOdor;
  serialSend(SpStepN, taskType);
  while ((currentMiss < missLimit) && (currentSession++ < totalSession)) {
    serialSend(SpSess, 1);
    hit = miss = falseAlarm = correctRejection = 0;
    unsigned int lastHit = 0;
    unsigned int shuffledLongList[highLevelShuffleLength];
    shuffleArray(shuffledLongList, highLevelShuffleLength);

    for (currentTrial = 0; currentTrial < trialsPerSession && currentMiss < missLimit;) {
      resetTaskTimer();
      serialSend(Sptrialtype, type);
      serialSend(Splaser, type);
      int firstOdor = (shuffledLongList[currentTrial] % 2 == 0 ) ? 5 : 6;
      serialSend(Sptrialtype, taskType);
      serialSend(Splaser, (type != laserOff));
      distractor(type, firstOdor, stims.distractorJudgingPair, WaterLen, taskType, currentSession);
      waitTaskTimer(1500ul);
      serialSend(SpITI, 0);
      if (type == laserDuringITI_GNG) {
        assertLaserON(type);
        waitTaskTimer((ITI - 0.6) * 1000ul); //another 4000 is at the beginning of the trials.
        assertLaserOFF(type, true);
        waitTaskTimer(100ul);
      } else {
        waitTaskTimer(ITI * 1000ul); //another 4000 is at the beginning of the trials.
      }
      currentTrial++;
    }
    serialSend(SpSess, 0);
  }
  serialSend(SpTrain, 0); // send it's the end
}


static void DRTSession(int stim1, int laserTrialType, int delayduration, float ITI, int trialsPerSession, float WaterLen, int missLimit, unsigned int totalSession, int taskType) {

  int currentTrial = 0;
  currentSession = 0;
  int laserOnType = laserTrialType;
  serialSend(SpStepN, taskType);
  while ((currentMiss < missLimit) && (currentSession++ < totalSession)) {
    serialSend(SpSess, 1);
    hit = miss = falseAlarm = correctRejection = 0;
    unsigned int lastHit = 0;
    unsigned int shuffledList[4] = { 0, 1, 2, 3 };
    unsigned int shuffledLongList[highLevelShuffleLength];
    shuffleArray(shuffledLongList, highLevelShuffleLength);
    int firstOdor, secondOdor, isdelaylick;

    if (laserOnType == laserDuringDelay_S) {
      if (totalSession % 2 == 0) {
        if (currentSession % 2 ) {
          laserTrialType = laserOff;
        } else {
          laserTrialType = laserOnType;
        }
      } else {
        if (currentSession % 2 ) {
          laserTrialType = laserOnType;
        } else {
          laserTrialType = laserOff;
        }
      }
    }

    for (currentTrial = 0; currentTrial < trialsPerSession && currentMiss < missLimit;) {
      //      shuffleArray(shuffledList, 4);
      int iter;
      for (iter = 0; iter < 4 && currentMiss < missLimit; iter++) {
        int index = rand() % ( 4 - iter );
        firstOdor = (shuffledList[index] == 0 || shuffledList[index] == 1) ? stim1 : (stim1 + 1);
        secondOdor = 7;

//        DRTTrial(laserTrialType, firstOdor, stims, delayduration, secondOdor, WaterLen, ITI, taskType, currentSession);
         isdelaylick = DRTTraining(laserTrialType, firstOdor, stims, delayduration, secondOdor, WaterLen, ITI, taskType, currentSession);

        if ( isdelaylick == 1 ) {
          iter = iter - 1;
        } else {
          currentTrial++;
          int temp = shuffledList[index];
          shuffledList[index] = shuffledList[3 - iter];
          shuffledList[3 - iter] = temp;
        }
      }
    }
    serialSend(SpSess, 0);
  }
  serialSend(SpTrain, 0); // send it's the end
}


static void DNMSSession(int stim1,  int laserTrialType, int delayduration, float ITI, int trialsPerSession, float WaterLen, int missLimit, unsigned int totalSession, int taskType) {
  int currentTrial = 0;
  currentSession = 0;
  int laserOnType = laserTrialType;
  serialSend(SpStepN, taskType);
  while ((currentMiss < missLimit) && (currentSession++ < totalSession)) {
    serialSend(SpSess, 1);

    hit = miss = falseAlarm = correctRejection = 0;
    unsigned int lastHit = 0;
    unsigned int shuffledList[4];
    unsigned int shuffledLongList[highLevelShuffleLength];
    shuffleArray(shuffledLongList, highLevelShuffleLength);
    int firstOdor, secondOdor;

    for (currentTrial = 0; currentTrial < trialsPerSession && currentMiss < missLimit;) {
      shuffleArray(shuffledList, 4);
      int iter;
      for (iter = 0; iter < 4 && currentMiss < missLimit; iter++) {
        int index = shuffledList[iter];
        switch (taskType) {
          case DNMS_SHAPING:
            firstOdor = (index == 0 || index == 2) ? stim1 : (stim1 + 1);
            secondOdor = (firstOdor == stim1) ? (stim1 + 1)  : stim1;
            break;
          case   DNMS_TRAINING:
            firstOdor = (index == 0 || index == 2) ? stim1 : (stim1 + 1);
            secondOdor = (index == 1 || index == 2) ? stim1 : (stim1 + 1);
            break;
        }

        DNMSTrial(laserTrialType, firstOdor, stims, delayduration, secondOdor, WaterLen, ITI, taskType, currentSession, 0, 0, false);
        currentTrial++;
      }
    }
    serialSend(SpSess, 0);
  }
  serialSend(SpTrain, 0); // send it's the end

}


static void FourOdorDNMSSession(int stim1,  int laserTrialType, int delayduration, float ITI, int trialsPerSession, float WaterLen, int missLimit, unsigned int totalSession, int taskType) {

  int currentTrial = 0;
  currentSession = 0;
  int laserOnType = laserTrialType;
  serialSend(SpStepN, taskType);

  unsigned int *OdorListA,  *OdorListB, *OdorListC, *OdorListD;

  unsigned int OdorListAS[6] = {stim1 + 1, stim1 + 1, stim1 + 2, stim1 + 2, stim1 + 3, stim1 + 3};
  unsigned int OdorListBS[6] = {stim1, stim1, stim1 + 2, stim1 + 2, stim1 + 3, stim1 + 3};
  unsigned int OdorListCS[6] = {stim1, stim1, stim1 + 1, stim1 + 1, stim1 + 3, stim1 + 3};
  unsigned int OdorListDS[6] = {stim1, stim1, stim1 + 1, stim1 + 1, stim1 + 2, stim1 + 2};
  unsigned int OdorListAT[6] = {stim1, stim1 + 1, stim1 + 2, stim1 + 3, stim1, stim1};
  unsigned int OdorListBT[6] = {stim1 + 1, stim1 + 2, stim1 + 3, stim1, stim1 + 1, stim1 + 1};
  unsigned int OdorListCT[6] = {stim1 + 2, stim1 + 3, stim1, stim1 + 1, stim1 + 2, stim1 + 2};
  unsigned int OdorListDT[6] = {stim1 + 3, stim1, stim1 + 1, stim1 + 2, stim1 + 3, stim1 + 3};

  unsigned int AshuffledList[6], BshuffledList[6], CshuffledList[6], DshuffledList[6];

  if (taskType == FOUR_ODOR_DNMS_SHAPING) {
    OdorListA  = OdorListAS;   OdorListB  = OdorListBS;   OdorListC  = OdorListCS;   OdorListD  = OdorListDS;
  } else if (taskType == FOUR_ODOR_DNMS_TRAINING) {
    OdorListA  = OdorListAT;   OdorListB  = OdorListBT;   OdorListC  = OdorListCT;   OdorListD  = OdorListDT;
  }

  while ((currentMiss < missLimit) && (currentSession++ < totalSession)) {

    serialSend(SpSess, 1);

    hit = miss = falseAlarm = correctRejection = 0;
    int APos = 0, BPos = 0, CPos = 0, DPos = 0;
    shuffleArray(AshuffledList, 6); shuffleArray(BshuffledList, 6); shuffleArray(CshuffledList, 6); shuffleArray(DshuffledList, 6);
    unsigned int shuffledList[4];
    unsigned int shuffledLongList[highLevelShuffleLength];
    shuffleArray(shuffledLongList, highLevelShuffleLength);
    int firstOdor, secondOdor;

    for (currentTrial = 0; currentTrial < trialsPerSession && currentMiss < missLimit;) {
      shuffleArray(shuffledList, 4);
      int iter;
      for (iter = 0; iter < 4 && currentMiss < missLimit; iter++) {
        int index = shuffledList[iter];
        switch (index) {
          case 0:
            firstOdor = stim1;
            secondOdor = OdorListA[AshuffledList[APos]];
            APos++;
            break;
          case 1:
            firstOdor = stim1 + 1;
            secondOdor =  OdorListB[BshuffledList[BPos]];
            BPos++;
            break;
          case 2:
            firstOdor = stim1 + 2;
            secondOdor = OdorListC[CshuffledList[CPos]];
            CPos++;
            break;
          case 3:
            firstOdor = stim1 + 3;
            secondOdor = OdorListD[DshuffledList[DPos]];
            DPos++;
            break;
        }

        DNMSTrial(laserTrialType, firstOdor, stims, delayduration, secondOdor, WaterLen, ITI, taskType, currentSession, 0, 0, false);
        currentTrial++;
      }
    }
    serialSend(SpSess, 0);
  }
  serialSend(SpTrain, 0); // send it's the end
}


static void TenOdorDNMSSession(int stim1,  int laserTrialType, int delayduration, float ITI, int trialsPerSession, float WaterLen, int missLimit, unsigned int totalSession, int taskType) {

  int currentTrial = 0;
  int firstOdor, secondOdor;
  currentSession = 0;
  int laserOnType = laserTrialType;
  int blockNum = 0;
  serialSend(SpStepN, taskType);

  unsigned int OdorList[20] = {stim1, stim1 + 1, stim1 + 2, stim1 + 3, stim1 + 4, stim1 + 5, stim1 + 6, stim1 + 7, stim1 + 8, stim1 + 9,
                               stim1, stim1 + 1, stim1 + 2, stim1 + 3, stim1 + 4, stim1 + 5, stim1 + 6, stim1 + 7, stim1 + 8, stim1 + 9
                              };

  unsigned int OdorPosList[10][9];
  unsigned int MorNMList[10][2];
  unsigned int temp1[9];
  unsigned int temp2[2];
  for  (int i = 0; i < 10; i++) {
    shuffleArray(temp1, 9);
    for (int j = 0; j < 9; j++) {
      OdorPosList[i][j] = temp1[j];
    }
  }

  while ((currentMiss < missLimit) && (currentSession++ < totalSession)) {

    serialSend(SpSess, 1);
    hit = miss = falseAlarm = correctRejection = 0;
    unsigned int shuffledList[10];

    for  (int i = 0; i < 10; i++) {
      shuffleArray(temp2, 2);
      for (int j = 0; j < 2; j++) {
        MorNMList[i][j] = temp2[j];
      }
    }

    for (currentTrial = 0; currentTrial < trialsPerSession && currentMiss < missLimit;) {

      shuffleArray(shuffledList, 10);
      int iter;

      for (iter = 0; iter < 10 && currentMiss < missLimit; iter++) {
        int index = shuffledList[iter];

        firstOdor = stim1 + index;
        if (taskType == TEN_ODOR_DMS_SHAPING) {
          secondOdor = firstOdor;
        } else if (taskType == TEN_ODOR_DMS_TRAINING) {
          if (MorNMList[index][blockNum % 2] == 0)  {
            secondOdor = firstOdor;
          } else if (MorNMList[index][blockNum % 2] == 1)  {
            secondOdor = OdorList[index + OdorPosList[index][currentSession] + 1];
          }
        }
        DNMSTrial(laserTrialType, firstOdor, stims, delayduration, secondOdor, WaterLen, ITI, taskType, currentSession, 0, 0, false);
        currentTrial++;
      }
      blockNum++;
    }
    serialSend(SpSess, 0);
  }
  serialSend(SpTrain, 0); // send it's the end
}


static void DPASession(int stim1, int stim2, int laserTrialType, int delayduration, unsigned int ITI, int trialsPerSession, float WaterLen, int missLimit, unsigned int totalSession, int taskType) {

  int currentTrial = 0;
  currentSession = 0;
  int laserOnType = laserTrialType;
  serialSend(SpStepN, taskType);
  //  serialSend(SpLaser, laserTrialType);
  while ((currentMiss < missLimit) && (currentSession++ < totalSession)) {

    serialSend(SpSess, 1);
    hit = miss = falseAlarm = correctRejection = 0;
    unsigned int lastHit = 0;
    unsigned int shuffledList[4];
    unsigned int shuffledLongList[highLevelShuffleLength];
    shuffleArray(shuffledLongList, highLevelShuffleLength);
    int firstOdor, secondOdor;

    if (laserOnType != laserDuringDelay_S && laserOnType != laserDuringTwoOdors_S && laserOnType != laserOff) {
      if (totalSession % 2 == 0) {
        if (currentSession % 2 ) {
          laserTrialType = laserOff;
        } else {
          laserTrialType = laserOnType;
        }
      } else {
        if (currentSession % 2 ) {
          laserTrialType = laserOnType;
        } else {
          laserTrialType = laserOff;
        }
      }
    }

    for (currentTrial = 0; currentTrial < trialsPerSession && currentMiss < missLimit;) {
      shuffleArray(shuffledList, 4);
      int iter;
      for (iter = 0; iter < 4 && currentMiss < missLimit; iter++) {
        int index = shuffledList[iter];
        switch (taskType) {
          case DPA_SHAPING:
            firstOdor = (index == 0 || index == 1) ? stim1 : (stim1 + 1);
            secondOdor = (firstOdor == stim1) ? stim2  : (stim2 + 1);
            break;
          case DPA_R_SHAPING:
            firstOdor = (index == 0 || index == 1) ? stim1 : (stim1 + 1);
            secondOdor = (firstOdor == stim1) ? stim2  : (stim2 + 1);
            break;
          case   DPA_TRAINING:
            firstOdor = (index == 0 || index == 1) ? stim1 : (stim1 + 1);
            secondOdor = (index == 1 || index == 2) ? stim2 : (stim2 + 1);
            break;
          case   DPA_R_TRAINING:
            firstOdor = (index == 0 || index == 1) ? stim1 : (stim1 + 1);
            secondOdor = (index == 1 || index == 2) ? stim2 : (stim2 + 1);
            break;
          case DPA_SHAPING_NoDelay:
            firstOdor = (index == 0 || index == 1) ? stim1 : (stim1 + 1);
            secondOdor = (firstOdor == stim1) ? (stim2 + 1) :  stim2 ;
            break;
          case DPA_TRAINING_NoDelay:
            firstOdor = (index == 0 || index == 1) ? stim1 : (stim1 + 1);
            secondOdor = (index == 1 || index == 2) ? stim2 : (stim2 + 1);
            break;
        }
        if (delayduration != 0) {
          DPATrial(laserTrialType, firstOdor, stims, delayduration, secondOdor, WaterLen, ITI, taskType, currentSession, 0, 0, false);
        } else {
          DPATrialNoDelay(laserTrialType, firstOdor, stims, delayduration, secondOdor, WaterLen, ITI, taskType, currentSession, 0, 0, false);
        }

        currentTrial++;
      }
    }
    serialSend(SpSess, 0);
  }
  serialSend(SpTrain, 0); // send it's the end
}


static void SixOdorDPASession(int stim1, int stim2, int laserTrialType, int delayduration, unsigned int ITI, int trialsPerSession, float WaterLen, int missLimit, unsigned int totalSession, int taskType) {
  int currentTrial = 0;
  currentSession = 0;
  int laserOnType = laserTrialType;
  serialSend(SpStepN, taskType);

  while ((currentMiss < missLimit) && (currentSession++ < totalSession)) {
    serialSend(SpSess, 1);
    hit = miss = falseAlarm = correctRejection = 0;
    unsigned int lastHit = 0;
    unsigned int shuffledList[8];
    unsigned int shuffledLongList[highLevelShuffleLength];
    shuffleArray(shuffledLongList, highLevelShuffleLength);
    int firstOdor, secondOdor;

    for (currentTrial = 0; currentTrial < trialsPerSession && currentMiss < missLimit;) {
      shuffleArray(shuffledList, 8);
      int iter;
      for (iter = 0; iter < 8 && currentMiss < missLimit; iter++) {
        int index = shuffledList[iter];
        switch (taskType) {
          case SIX_ODOR_DPA_SHAPING:
            if (index == 0 || index == 1) firstOdor = stim1;
            if (index == 2 || index == 3) firstOdor = stim1 + 1;
            if (index == 4 || index == 5) firstOdor = stim1 + 2;
            if (index == 6 || index == 7) firstOdor = stim1 + 3;
            secondOdor = (firstOdor == stim1 || firstOdor == stim1 + 2) ? stim2  : (stim2 + 1);
            break;
          case   SIX_ODOR_DPA_TRAINING:
            if (index == 0 || index == 1) firstOdor = stim1;
            if (index == 2 || index == 3) firstOdor = stim1 + 1;
            if (index == 4 || index == 5) firstOdor = stim1 + 2;
            if (index == 6 || index == 7) firstOdor = stim1 + 3;
            secondOdor = (index % 2 == 1) ? stim2 : (stim2 + 1);
            break;
        }

        DPATrial(laserTrialType, firstOdor, stims, delayduration, secondOdor, WaterLen, ITI, taskType, currentSession, 0, 0, false);

        currentTrial++;
      }
    }
    serialSend(SpSess, 0);
  }
  serialSend(SpTrain, 0); // send it's the end
}


static void MixDPADNMSSessionsE(int stim1, int stim2, int laserTrialType, _delayT delay, unsigned int ITI, int trialsPerSession, float WaterLen, int missLimit, unsigned int totalSession, int taskType, boolean multDistractor) {

  int currentTrial = 0;
  currentSession = 0;
  int laserOnType = laserTrialType;

  int blockNum = 0;
  int repeatTime = totalSession * 1;
  unsigned int DPADNMS0[repeatTime], DPADNMS1[repeatTime], DPADNMS2[repeatTime], DPADNMS3[repeatTime];
  unsigned int DNMSDPA0[repeatTime], DNMSDPA1[repeatTime], DNMSDPA2[repeatTime], DNMSDPA3[repeatTime];
  shuffleArray(DPADNMS0, repeatTime);   shuffleArray(DNMSDPA0, repeatTime);
  shuffleArray(DPADNMS1, repeatTime);   shuffleArray(DNMSDPA1, repeatTime);
  shuffleArray(DPADNMS2, repeatTime);   shuffleArray(DNMSDPA2, repeatTime);
  shuffleArray(DPADNMS3, repeatTime);   shuffleArray(DNMSDPA3, repeatTime);
  unsigned int DPADNMSPos0 = 0, DPADNMSPos1 = 0, DPADNMSPos2 = 0, DPADNMSPos3 = 0;
  unsigned int DNMSDPAPos0 = 0, DNMSDPAPos1 = 0, DNMSDPAPos2 = 0, DNMSDPAPos3 = 0;
  unsigned int *singleGNGn;
  //  unsigned int temp1[4] = {0, 1, 2, 0}, temp2[4] = {1, 1, 0, 2}, temp3[4] = {2, 1, 0, 2}, temp4[4] = {1, 0, 2, 0}, temp5[4] = {0, 1, 2, 1}, temp6[4] = {2, 0, 2, 1},
  //                          temp7[4] = {2, 0, 0, 1}, temp8[4] = {1, 2, 1, 0}, temp9[4] = {1, 0, 2, 2}, temp10[4] = {0, 1, 0, 2}, temp11[4] = {1, 2, 1, 0}, temp12[4] = {1, 0, 2, 2};
  unsigned int DPAHitTrial = 0, DPACRTrial = 0;

  serialSend(SpStepN, taskType);

  //  unsigned int DistrRand[totalSession];
  //  shuffleArray(DistrRand, totalSession);

  while ((currentMiss < missLimit) && (currentSession++ < totalSession)) {

    serialSend(SpSess, 1);
    hit = miss = falseAlarm = correctRejection = 0;
    unsigned int shuffledList[4];
    unsigned int shuffledLongList[highLevelShuffleLength];
    shuffleArray(shuffledLongList, highLevelShuffleLength);
    int firstOdor, secondOdor;

    if (laserOnType != laserDuringDelay_D && laserOnType != laserOff) {
      if (totalSession % 2 == 0) {
        if (currentSession % 2 ) {
          laserTrialType = laserOff;
        } else {
          laserTrialType = laserOnType;
        }
      } else {
        if (currentSession % 2 ) {
          laserTrialType = laserOnType;
        } else {
          laserTrialType = laserOff;
        }
      }
    }

    for (currentTrial = 0; currentTrial < trialsPerSession && currentMiss < missLimit;) {
      shuffleArray(shuffledList, 4);

      //      switch (blockNum % 3) {
      //        case 0:
      //          switch (DistrRand[currentSession] % 4) {
      //            case 0 :
      //              singleGNGn = temp1;
      //              break;
      //            case 1 :
      //              singleGNGn = temp4;
      //              break;
      //            case 2 :
      //              singleGNGn = temp7;
      //              break;
      //            case 3 :
      //              singleGNGn = temp10;
      //              break;
      //          }
      //          break;
      //        case 1:
      //          switch (DistrRand[currentSession] % 4) {
      //            case 0 :
      //              singleGNGn = temp2;
      //              break;
      //            case 1 :
      //              singleGNGn = temp5;
      //              break;
      //            case 2 :
      //              singleGNGn = temp8;
      //              break;
      //            case 3 :
      //              singleGNGn = temp11;
      //              break;
      //          }
      //          break;
      //        case 2:
      //          switch (DistrRand[currentSession] % 4) {
      //            case 0 :
      //              singleGNGn = temp3;
      //              break;
      //            case 1 :
      //              singleGNGn = temp6;
      //              break;
      //            case 2 :
      //              singleGNGn = temp9;
      //              break;
      //            case 3 :
      //              singleGNGn = temp12;
      //              break;
      //          }
      //          break;
      //      }

      int iter;
      for (iter = 0; iter < 4 && currentMiss < missLimit; iter++) {          // every 4-trial
        int index = shuffledList[iter];
        int temp = (currentTrial - iter) / 4;
        switch (taskType) {

          //          case DPA_GNG_SHAPING:  //   4360
          //            firstOdor = (index == 0 || index == 1) ? stim1 : (stim1 + 1);
          //            secondOdor = (firstOdor == stim1) ? stim2  : (stim2 + 1);
          //            switch (singleGNGn[iter] % 3) {
          //              case 0:
          //                stims.currentDistractor = 0u;
          //                break;
          //              case 1:
          //                stims.currentDistractor = 5u;
          //                break;
          //              case 2:
          //                stims.currentDistractor = 6u;
          //                break;
          //            }
          //            break;
          //
          //          case DPA_GNG_TRAINING:  //   4361
          //            firstOdor = (index == 0 || index == 1) ? stim1 : (stim1 + 1);
          //            secondOdor = (index == 1 || index == 2) ? stim2 : (stim2 + 1);
          //            switch (singleGNGn[iter] % 3) {
          //              case 0:
          //                stims.currentDistractor = 0u;
          //                break;
          //              case 1:
          //                stims.currentDistractor = 5u;
          //                break;
          //              case 2:
          //                stims.currentDistractor = 6u;
          //                break;
          //            }
          //            break;

          case DPA_DRT_SHAPING:  //   4362
            firstOdor = (index == 0 || index == 1) ? stim1 : (stim1 + 1);
            secondOdor = (index == 1 || index == 2) ? stim2 : (stim2 + 1);
            first_ITI = 2.5;
            second_ITI = 2.45;
            switch (blockNum % 2) {
              case 0:
               if (blockNum / 2 % 2 == 1) {
                  DistractorSeq[1] = (index == 0 || index == 1) ? 5 : 6;
                } else {
                  DistractorSeq[1] = (index == 2 || index == 3) ? 5 : 6;
                }
                break;               
              case 1:
                 DistractorSeq[1] = 0u;
                 break;
            }
            DistractorSeq[2] = 7u;
            break;

          case DPA_DRT_TRAINING:  //   4363
            firstOdor = (index == 0 || index == 1) ? stim1 : (stim1 + 1);
            secondOdor = (index == 1 || index == 2) ? stim2 : (stim2 + 1);
            first_ITI = 2.5;
            second_ITI = 2.45;
            switch (blockNum % 2) {
              case 0:
                DistractorSeq[1] = 0u;
                break;
              case 1:
                if (blockNum / 2 % 3 == 0) {
                  DistractorSeq[1] = (index == 0 || index == 1) ? 5 : 6;
                } else if (blockNum / 2 % 3 == 1) {
                  DistractorSeq[1] = (index == 2 || index == 3) ? 5 : 6;
                } else if (blockNum / 2 % 3 == 2) {
                  DistractorSeq[1] = (index == 2 || index == 1) ? 5 : 6;
                }
                break;
            }
            DistractorSeq[2] = 7u;
            break;


          case   DRT_DPA_SHAPING:     //4364
            firstOdor = (index == 0 || index == 1) ? stim1 : (stim1 + 1);
            secondOdor = stim2;
            first_ITI = 2.5;
            second_ITI = 2.45;

            DistractorSeq[1] = (index == 0 || index == 1) ? 1 : 2;
            DistractorSeq[2] = ( DistractorSeq[1] == 1) ? 3 : 4;
            break;


          case   DRT_DPA_TRAINING:   //4365
            firstOdor = (index == 0 || index == 1) ? stim1 : (stim1 + 1);
            secondOdor = stim2;
            first_ITI = 2.5;
            second_ITI = 2.45;
            switch (singleGNGn[iter] % 3) {
              case 0:
                DistractorSeq[1] = 0u;
                break;
              case 1:
                DPAHitTrial++;
                if (DPAHitTrial % 2 == 0) {
                  DistractorSeq[1] = 1;
                  DistractorSeq[2] = 3;
                } else if (DPAHitTrial % 2 == 1) {
                  DistractorSeq[1] = 2;
                  DistractorSeq[2] = 4;
                }
                break;
              case 2:
                DPACRTrial++;
                if (DPACRTrial % 2 == 0) {
                  DistractorSeq[1] = 2;
                  DistractorSeq[2] = 3;
                } else if (DPACRTrial % 2 == 1) {
                  DistractorSeq[1] = 1;
                  DistractorSeq[2] = 4;
                }
                break;
            }
            break;


          case  DPA_DNMS_SHAPING: //

            firstOdor = (index == 0 || index == 1) ? stim1 : (stim1 + 1);
            secondOdor = (firstOdor == stim1) ? stim2  : (stim2 + 1);
            DistractorSeq[1] = (index == 0 || index == 2) ? 5 : 6;
            DistractorSeq[2] = ( DistractorSeq[1] == 5)  ? 6 : 5;
            first_ITI = 2.5;
            second_ITI = 2.45;
            break;

          case  DPA_DNMS_TRAINING:      //
            firstOdor = (index == 0 || index == 1) ? stim1 : (stim1 + 1);
            secondOdor = (index == 1 || index == 2) ? stim2 : (stim2 + 1);
            first_ITI = 2.5;
            second_ITI = 2.45;
            switch (temp % 2) {   // block design:  4 trial only DPA, 8 trial dual task
              case 0:
                DistractorSeq[1] = 0;
                DistractorSeq[2] = 0;
                break;
              case 1:
                switch (blockNum / 2 % 4) {
                  case 0:
                    DistractorSeq[1] = (index == 0 || index == 1) ? 6 : 5;
                    DistractorSeq[2] = (index == 1 || index == 2) ? 5 : 6;
                    break;
                  case 1:
                    DistractorSeq[1] = (index == 0 || index == 1) ? 6 : 5;
                    DistractorSeq[2] = (index == 1 || index == 2) ? 6 : 5;
                    break;
                  case 2:
                    DistractorSeq[1] = (index == 0 || index == 1) ? 5 : 6;
                    DistractorSeq[2] = (index == 1 || index == 2) ? 6 : 5;
                    break;
                  case 3:
                    DistractorSeq[1] = (index == 0 || index == 1) ? 5 : 6;
                    DistractorSeq[2] = (index == 1 || index == 2) ? 5 : 6;
                    break;
                }
            }
            break;

        }
        //////////////////////////////////////////////////////////////////////////////////
        switch (laserSessionType) {
          case LASER_NO_TRIAL:
            laserTrialType = laserOff;
            break;
          case LASER_EVERY_TRIAL:
            break;
          case LASER_OTHER_TRIAL:
            laserTrialType = (currentTrial % 2) == 0 ? laserOff : laserOnType;
            break;
        }
        if (taskType == DRT_DPA_TRAINING || taskType == DRT_DPA_SHAPING) {
          DRTTrial(laserTrialType, firstOdor, stims, delayduration, secondOdor, WaterLen, ITI, taskType, currentSession);
        } else {
          DPATrial(laserTrialType, firstOdor, stims, 13, secondOdor, WaterLen, ITI, taskType, currentSession, 3, 2.95, true);
        }
        currentTrial++;
      }
      blockNum++;
    }
    serialSend(SpSess, 0);

  }
  serialSend(SpTrain, 0); // send it's the end

}


void wait_ms(int ms) {
  unsigned long base = millis();
  while (((unsigned long)ms + base) > millis()) {
  }
}


void feedWaterFast(int waterLength, int totalWaterCount) {

  int wasLicking = 0;

  unsigned int waterCount = 0;
  unsigned int totalLickCount = 0;
  unsigned long timerCounterI = millis();
  if (totalWaterCount == 0) totalWaterCount = 400;

  while (waterCount < totalWaterCount) {
    if (isLicking && !wasLicking) {
      wasLicking = 1;
      if (millis() - timerCounterI >= 500) {
        Valve_ON(waterValve);
        timerCounterI = millis();
        waterCount++;
        serialSend(SpWater_sweet, waterCount);
      }
      totalLickCount++;
    } else if (wasLicking && !isLicking) {
      wasLicking = 0;
    }
    if (millis() - timerCounterI >= waterLength) {
      Valve_OFF(waterValve);
    }
  }
  Valve_OFF(waterValve);
}


void zxFunc(int n) {
  currentMiss = 0;
  int iter;
  switch (n) {

    case 4311: // test valve
      {
        int n = getFuncNumber(2, "Valve No");
        for (iter = 0; iter < 50; iter++) {
          Valve_ON(n);
          delay(WaterLen * 1000);
          Valve_OFF(n);
          delay(500);
        }
        break;
      }
    case 4312: // quick test valve
      {
        int n = getFuncNumber(2, "Valve No");
        for (iter = 0; iter < 50; iter++) {
          Valve_ON(n);
          delay(500);
          Valve_OFF(n);
          delay(500);
        }
        break;
      }

    case 4424: //lick
      {
        int totalWaterCount = getFuncNumber(2, "totalWaterCount");
        feedWaterFast(WaterLen * 1000, totalWaterCount);
        break;
      }

    case 4330:  //  DNMS shaping
      {
        laserTrialType = laserOff;
        taskType =  DNMS_SHAPING;
        highLevelShuffleLength = 24;

        int totalSession = getFuncNumber(2, "SessionNumber");
        teachSesNo = getFuncNumber(1, "SessionNumber");

        int n = getFuncNumber(1, "delay");
        switch (n) {
          case 1:
            delayduration = 4;
            ITI = 8;
            break;
          case 2:
            delayduration = 8;
            ITI = 12;
            break;
          case 3:
            delayduration = 12;
            ITI = 16;
            break;
        }
        delay(2000);
        //DNMSSession( stim1, laserTrialType, delayduration, ITI, trialsPerSession, WaterLen, missLimit, totalSession,taskType)
        DNMSSession(5, laserOff, delayduration, ITI, 24, WaterLen, 30, totalSession, taskType);
        break;
      }

    case 4331: // DNMS training
      {
        laserTrialType = laserOff;
        taskType =  DNMS_TRAINING;
        highLevelShuffleLength = 24;

        int totalSession = getFuncNumber(2, "SessionNumber");

        int  n = getFuncNumber(1, "delay");
        switch (n) {
          case 1:
            delayduration = 4;
            ITI = 8;
            break;
          case 2:
            delayduration = 8;
            ITI = 12;
            break;
          case 3:
            delayduration = 12;
            ITI = 16;
            break;
        }

        n = getFuncNumber(1, "laser");
        switch (n) {
          case 0:
            laserTrialType = laserOff;
            break;
          case 1:
            laserTrialType = laserDuringDelay_S;
            break;
          case 2:
            laserTrialType = laserDuringEarlyDelay_S;
            break;
          case 3:
            laserTrialType = laserDuringLateDelay_S;
            break;
          case 4:
            laserTrialType = laserDuringITI_S;
            break;
          case 5:
            laserTrialType = laserDuring1stOdor_S;
            break;
          case 6:
            laserTrialType = laserDuring2ndOdor_S;
            break;
          case 7:
            laserTrialType = laserDuringResp_S;
            break;
        }
        delay(2000);
        //DPASession( stim1, stim2, laserTrialType, delayduration, ITI, trialsPerSession, WaterLen, missLimit, totalSession,taskType)
        DNMSSession(5, laserTrialType, delayduration, ITI, 24, WaterLen * 1.1, 30, totalSession, taskType);

        break;
      }

    case 4332:  //  4 odor DNMS shaping
      {
        laserTrialType = laserOff;
        taskType =  FOUR_ODOR_DNMS_SHAPING;
        highLevelShuffleLength = 24;

        int totalSession = getFuncNumber(2, "SessionNumber");
        teachSesNo = getFuncNumber(1, "SessionNumber");

        int n = getFuncNumber(1, "delay");
        switch (n) {
          case 1:
            delayduration = 4;
            ITI = 8;
            break;
          case 2:
            delayduration = 8;
            ITI = 12;
            break;
          case 3:
            delayduration = 12;
            ITI = 16;
            break;
        }

        delay(2000);
        //FourOdorDNMSSession( stim1, laserTrialType, delayduration, ITI, trialsPerSession, WaterLen, missLimit, totalSession,taskType)
        FourOdorDNMSSession(5, laserOff, delayduration, ITI, 24, WaterLen, 30, totalSession, taskType);
        break;
      }

    case 4333:  //  4 odor DNMS training
      {
        laserTrialType = laserOff;
        taskType =  FOUR_ODOR_DNMS_TRAINING;
        highLevelShuffleLength = 24;

        int totalSession = getFuncNumber(2, "SessionNumber");

        int n = getFuncNumber(1, "delay");
        switch (n) {
          case 1:
            delayduration = 4;
            ITI = 8;
            break;
          case 2:
            delayduration = 8;
            ITI = 12;
            break;
          case 3:
            delayduration = 12;
            ITI = 16;
            break;
        }

        n = getFuncNumber(1, "laser");
        switch (n) {
          case 0:
            laserTrialType = laserOff;
            break;
          case 1:
            laserTrialType = laserDuringDelay_S;
            break;
          case 2:
            laserTrialType = laserDuringEarlyDelay_S;
            break;
          case 3:
            laserTrialType = laserDuringLateDelay_S;
            break;
          case 4:
            laserTrialType = laserDuringITI_S;
            break;
          case 5:
            laserTrialType = laserDuring1stOdor_S;
            break;
          case 6:
            laserTrialType = laserDuring2ndOdor_S;
            break;
          case 7:
            laserTrialType = laserDuringResp_S;
            break;
        }
        delay(2000);
        //FourOdorDNMSSession( stim1, laserTrialType, delayduration, ITI, trialsPerSession, WaterLen, missLimit, totalSession,taskType)
        FourOdorDNMSSession(5, laserTrialType, delayduration, ITI, 24, WaterLen * 1.1, 30, totalSession, taskType);
        break;
      }

    case 4334:  //  10 odor DNMS shaping
      {
        laserTrialType = laserOff;
        taskType =  TEN_ODOR_DMS_SHAPING;
        highLevelShuffleLength = 20;

        int totalSession = getFuncNumber(2, "SessionNumber");
        teachSesNo = getFuncNumber(1, "SessionNumber");

        int n = getFuncNumber(1, "delay");
        switch (n) {
          case 1:
            delayduration = 4;
            ITI = 8;
            break;
          case 2:
            delayduration = 8;
            ITI = 12;
            break;
          case 3:
            delayduration = 12;
            ITI = 16;
            break;
        }

        delay(2000);
        //FourOdorDNMSSession( stim1, laserTrialType, delayduration, ITI, trialsPerSession, WaterLen, missLimit, totalSession,taskType)
        TenOdorDNMSSession(1, laserOff, delayduration, ITI, 20, WaterLen, 30, totalSession, taskType);
        break;
      }

    case 4335:  //  10 odor DNMS training
      {
        laserTrialType = laserOff;
        taskType =  TEN_ODOR_DMS_TRAINING;
        highLevelShuffleLength = 20;

        int totalSession = getFuncNumber(2, "SessionNumber");

        int n = getFuncNumber(1, "delay");
        switch (n) {
          case 1:
            delayduration = 1;
            ITI = 1;
            break;
          case 2:
            delayduration = 8;
            ITI = 12;
            break;
          case 3:
            delayduration = 12;
            ITI = 16;
            break;
        }

        n = getFuncNumber(1, "laser");
        switch (n) {
          case 0:
            laserTrialType = laserOff;
            break;
          case 1:
            laserTrialType = laserDuringDelay_S;
            break;
          case 2:
            laserTrialType = laserDuringEarlyDelay_S;
            break;
          case 3:
            laserTrialType = laserDuringLateDelay_S;
            break;
          case 4:
            laserTrialType = laserDuringITI_S;
            break;
          case 5:
            laserTrialType = laserDuring1stOdor_S;
            break;
          case 6:
            laserTrialType = laserDuring2ndOdor_S;
            break;
          case 7:
            laserTrialType = laserDuringResp_S;
            break;
        }
        delay(2000);
        //FourOdorDNMSSession( stim1, laserTrialType, delayduration, ITI, trialsPerSession, WaterLen, missLimit, totalSession,taskType)
        TenOdorDNMSSession(1, laserTrialType, delayduration, ITI, 20, WaterLen * 1.1, 30, totalSession, taskType);
        break;
      }

    case 4340: // GNG reversal
      {
        laserTrialType = laserOff;
        multDistractor = false;
        highLevelShuffleLength = 20;

        taskType = GNG_TRAINING;

        int totalSession = getFuncNumber(2, "SessionNumber");
        n = getFuncNumber(1, "reversal or not");
        switch (n) {
          case 1:
            GNGRorN = false;
            break;
          case 2:
            GNGRorN =  true;
            break;
        }
        n = getFuncNumber(1, "give water 20 trials if miss after reverse");
        switch (n) {
          case 1:
            GNG_R_Shaping = false;
            break;
          case 2:
            GNG_R_Shaping = true;
            break;
        }
        n = getFuncNumber(1, "laser");
        switch (n) {
          case 0:
            laserTrialType = laserOff;
            break;
          case 1:
            laserTrialType = laserDuringITI_GNG;
            break;
          case 2:
            laserTrialType = laserDuringOdor_Resp_GNG;
            break;
        }

        highLevelShuffleLength = 20;
        //GNGSession(laserTrialType,ITI,trialsPerSession,WaterLen, missLimit, totalSession,taskType)
        delay(2000);
        GNGSession(laserOff, 4, 20, WaterLen, 10, totalSession, taskType);
        break;
      }


    case 4341: // DRT training
      {
        taskType = DRT_TRAINING;
        int totalSession = getFuncNumber(2, "SessionNumber");
        teachSesNo = getFuncNumber(1, "SessionNumber");
        n = getFuncNumber(1, "Delay");
        unsigned int ITI;
        switch (n) {
          case 1:
            delayduration = 4;
            ITI = 8;
            break;
          case 2:
            delayduration = 6;
            ITI = 10;
            break;
          case 3:
            delayduration = 10;
            ITI = 10;
            break;
        }
        n = getFuncNumber(1, "laser");
        switch (n) {
          case 0:
            laserTrialType = laserOff;
            break;
          case 1:
            laserTrialType = laserDuringDelay_S;
            break;
          case 2:
            laserTrialType = laserDuringEarlyDelay_S;
            break;
          case 3:
            laserTrialType = laserDuringLateDelay_S;
            break;
          case 4:
            laserTrialType = laserDuringITI_S;
            break;
          case 5:
            laserTrialType = laserDuring1stOdor_S;
            break;
          case 6:
            laserTrialType = laserDuring2ndOdor_S;
            break;
          case 7:
            laserTrialType = laserDuringResp_S;
            break;
        }
        highLevelShuffleLength = 20;
        delay(2000);
        //  DRTSession(stim1, laserTrialType, _delayT delay, ITI, trialsPerSession, WaterLen, missLimit, totalSession,taskType)
        DRTSession(5,  laserTrialType, delayduration, ITI, 20, WaterLen * 1.1, 10, totalSession, taskType);
        break;
      }

    case 4350: // DPA shaping
      {
        taskType =  DPA_SHAPING;
        int totalSession = getFuncNumber(2, "SessionNumber");
        teachSesNo = getFuncNumber(1, "SessionNumber");
        int n = getFuncNumber(1, "Delay");
        switch (n) {
          case 1:
            delayduration = 6;
            ITI = 10;
            break;
          case 2:
            delayduration = 8;
            ITI = 10;
            break;
          case 3:
            delayduration = 12;
            ITI = 10;
            break;
          case 4:
            delayduration = 0;
            ITI = 10;
            break;
        }
        delay(2000);
        //DPASession( stim1, stim2, laserTrialType, delayduration, ITI, trialsPerSession, WaterLen, missLimit, totalSession,taskType)
        if (delayduration != 0) {
          taskType =  DPA_SHAPING;
          DPASession(1, 3, laserTrialType, delayduration, ITI, 20, WaterLen, 10, totalSession, taskType);
        } else {
          taskType =  DPA_SHAPING_NoDelay;
          DPASession(1, 7, laserTrialType, delayduration, ITI, 20, WaterLen, 10, totalSession, taskType);
        }
        break;
      }

    case 4352: // DPA(R) shaping
      {
        taskType =  DPA_R_SHAPING;
        int totalSession = getFuncNumber(2, "SessionNumber");
        teachSesNo = getFuncNumber(1, "SessionNumber");
        int n = getFuncNumber(1, "Delay");
        switch (n) {
          case 1:
            delayduration = 4;
            ITI = 8;
            break;
          case 2:
            delayduration = 8;
            ITI = 10;
            break;
          case 3:
            delayduration = 12;
            ITI = 12;
            break;
        }
        delay(2000);
        //DPASession( stim1, stim2, laserTrialType, delayduration, ITI, trialsPerSession, WaterLen, missLimit, totalSession,taskType)
        DPASession(1, 3, laserOff, delayduration, ITI, 24, WaterLen, 10, totalSession, taskType);
        break;
      }

    case 4351: // DPA training
      {

        int totalSession = getFuncNumber(2, "SessionNumber");
        n = getFuncNumber(1, "Delay");
        unsigned int ITI;
        switch (n) {
          case 1:
            delayduration = 6;
            ITI = 10;
            break;
          case 2:
            delayduration = 8;
            ITI = 10;
            break;
          case 3:
            delayduration = 12;
            ITI = 10;
            break;
          case 4:
            delayduration = 0;
            ITI = 10;
        }
        n = getFuncNumber(1, "laser");
        switch (n) {
          case 0:
            laserTrialType = laserOff;
            break;
          case 1:
            laserTrialType = laserDuringDelay_S;
            break;
          case 2:
            laserTrialType = laserDuringEarlyDelay_S;
            break;
          case 3:
            laserTrialType = laserDuringLateDelay_S;
            break;
          case 4:
            laserTrialType = laserDuringITI_S;
            break;
          case 5:
            laserTrialType = laserDuring1stOdor_S;
            break;
          case 6:
            laserTrialType = laserDuring2ndOdor_S;
            break;
          case 7:
            laserTrialType = laserDuringResp_S;
            break;
          case 8:
            laserTrialType = laserDuringTwoOdors_S;
            break;

        }
        highLevelShuffleLength = 24;
        delay(2000);
        if (delayduration != 0) {
          taskType = DPA_TRAINING;
          DPASession(1, 3, laserTrialType, delayduration, ITI, 24, WaterLen, 10, totalSession, taskType);
        } else {
          taskType =  DPA_TRAINING_NoDelay;
          DPASession(1, 7, laserTrialType, delayduration, ITI, 24, WaterLen, 10, totalSession, taskType);
        }
        break;
      }


    case 4353: // DPA training REVERSAL
      {
        taskType = DPA_R_TRAINING;
        int totalSession = getFuncNumber(2, "SessionNumber");
        n = getFuncNumber(1, "Delay");
        unsigned int ITI;
        switch (n) {
          case 1:
            delayduration = 4;
            ITI = 8;
            break;
          case 2:
            delayduration = 8;
            ITI = 10;
            break;
          case 3:
            delayduration = 12;
            ITI = 12;
            break;
        }
        n = getFuncNumber(1, "laser");
        switch (n) {
          case 0:
            laserTrialType = laserOff;
            break;
          case 1:
            laserTrialType = laserDuringDelay_S;
            break;
          case 2:
            laserTrialType = laserDuringEarlyDelay_S;
            break;
          case 3:
            laserTrialType = laserDuringLateDelay_S;
            break;
          case 4:
            laserTrialType = laserDuringITI_S;
            break;
          case 5:
            laserTrialType = laserDuring1stOdor_S;
            break;
          case 6:
            laserTrialType = laserDuring2ndOdor_S;
            break;
          case 7:
            laserTrialType = laserDuringResp_S;
            break;
        }
        highLevelShuffleLength = 24;
        delay(2000);
        //  MixDPADNMSSessionsE(stim1, stim2, laserTrialType, _delayT delay, ITI, trialsPerSession, WaterLen, missLimit, totalSession,taskType)
        DPASession(1, 3, laserTrialType, delayduration, ITI, 20, WaterLen, 10, totalSession, taskType);
        break;
      }

    case 4354: // 6-odor DPA  shaping
      {
        taskType =  SIX_ODOR_DPA_SHAPING;
        int totalSession = getFuncNumber(2, "SessionNumber");
        teachSesNo = getFuncNumber(1, "SessionNumber");
        int n = getFuncNumber(1, "Delay");
        switch (n) {
          case 1:
            delayduration = 6;
            ITI = 10;
            break;
          case 2:
            delayduration = 8;
            ITI = 10;
            break;
          case 3:
            delayduration = 12;
            ITI = 10;
            break;
        }

        delay(2000);
        //SixOdorDPASession( stim1, stim2, laserTrialType, delayduration, ITI, trialsPerSession, WaterLen, missLimit, totalSession,taskType)
        SixOdorDPASession(1, 5, laserOff, delayduration, ITI, 24, WaterLen, 10, totalSession, taskType);
        break;
      }

    case 4355: // 6-odor DPA  training
      {
        taskType =  SIX_ODOR_DPA_TRAINING;
        int totalSession = getFuncNumber(2, "SessionNumber");
        int n = getFuncNumber(1, "Delay");
        switch (n) {
          case 1:
            delayduration = 6;
            ITI = 10;
            break;
          case 2:
            delayduration = 8;
            ITI = 10;
            break;
          case 3:
            delayduration = 12;
            ITI = 10;
            break;
        }

        n = getFuncNumber(1, "laser");
        switch (n) {
          case 0:
            laserTrialType = laserOff;
            break;
          case 1:
            laserTrialType = laserDuringDelay_S;
            break;
          case 2:
            laserTrialType = laserDuringEarlyDelay_S;
            break;
          case 3:
            laserTrialType = laserDuringLateDelay_S;
            break;
          case 4:
            laserTrialType = laserDuringITI_S;
            break;
          case 5:
            laserTrialType = laserDuring1stOdor_S;
            break;
          case 6:
            laserTrialType = laserDuring2ndOdor_S;
            break;
          case 7:
            laserTrialType = laserDuringResp_S;
            break;
        }
        delay(2000);
        //SixOdorDPASession( stim1, stim2, laserTrialType, delayduration, ITI, trialsPerSession, WaterLen, missLimit, totalSession,taskType)
        SixOdorDPASession(1, 5, laserTrialType, delayduration, ITI, 24, WaterLen * 1.1, 10, totalSession, taskType);
        break;
      }

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // Dual Task
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    case 4360: // DPA &GNG dual_task learning
      {
        laserTrialType = laserOff;
        multDistractor = false;
        taskType =  DPA_GNG_SHAPING;

        highLevelShuffleLength = 24;

        int totalSession = getFuncNumber(2, "SessionNumber");
        teachSesNo = getFuncNumber(1, "SessionNumber");

        int n = getFuncNumber(1, "reversal or not");
        switch (n) {
          case 1:
            GNGRorN = false;
            break;
          case 2:
            GNGRorN = true;
            break;
        }
        delay(2000);
        // MixDPADNMSSessionsE(stim1, stim2, laserTrialType, _delayT delay, ITI, trialsPerSession, WaterLen, missLimit, totalSession,taskType,multDistractor)
        MixDPADNMSSessionsE(1, 3, laserTrialType, 8u, 10u, 24u, WaterLen, 30, totalSession, taskType, multDistractor);
        break;
      }

    case 4361: // DPA & GNG training
      {
        int totalSession = getFuncNumber(2, "SessionNumber");

        int n = getFuncNumber(1, "laser");
        switch (n) {
          case 0:
            laserTrialType = laserOff;
            break;
          case 1:
            laserTrialType = laserDuringDelay_D;
            break;
          case 2:
            laserTrialType = laserDuring2ndDelay_D;
            break;
          case 4:
            laserTrialType = laserDuring1stResp_D;
            break;

        }
        multDistractor = false;

        taskType =  DPA_GNG_TRAINING;
        highLevelShuffleLength = 24;
        delay(2000);
        // MixDPADNMSSessionsE(stim1, stim2, laserTrialType, _delayT delay, ITI, trialsPerSession, WaterLen, missLimit, totalSession,taskType,multDistractor)
        MixDPADNMSSessionsE(1, 3, laserTrialType, 8u, 10u, 24u, WaterLen, 30, totalSession, taskType, multDistractor);
        break;
      }

    case 4362: // DPA & DRT dual_task learning
      {
        laserTrialType = laserOff;
        multDistractor = false;
        taskType =  DPA_DRT_SHAPING;
        highLevelShuffleLength = 24;

        int totalSession = getFuncNumber(2, "SessionNumber");
        teachSesNo = getFuncNumber(1, "SessionNumber");

        delay(2000);
        // MixDPADNMSSessionsE(stim1, stim2, laserTrialType, _delayT delay, ITI, trialsPerSession, WaterLen, missLimit, totalSession,taskType,multDistractor)
        MixDPADNMSSessionsE(1, 3, laserTrialType, 8u, 10u, 24u, WaterLen, 30, totalSession, taskType, multDistractor);
        break;
      }

    case 4363: // DPA & DRT training
      {
        int totalSession = getFuncNumber(2, "SessionNumber");

        int n = getFuncNumber(1, "laser");
        switch (n) {
          case 0:
            laserTrialType = laserOff;
            break;
          case 1:
            laserTrialType = laserDuringDelay_D;
            break;
          case 2:
            laserTrialType = laserDuring2ndDelay_D;
            break;
          case 3:
            laserTrialType = laserDuring3rdDelay_D;
            break;
          case 4:
            laserTrialType = laserDuring1stResp_D;
            break;
          case 5:
            laserTrialType = laserDuring2ndResp_D;
            break;
          case 6:
            laserTrialType = laserDuring1stOdor_D;
            break;
          case 7:
            laserTrialType = laserDuring2ndOdor_D;
            break;
          case 8:
            laserTrialType = laserDuring3rdOdor_D;
            break;
          case 9:
            laserTrialType = laserDuring4thOdor_D;
            break;
        }

        multDistractor = false;
        taskType =  DPA_DRT_TRAINING;
        highLevelShuffleLength = 24;
        delay(2000);
        // MixDPADNMSSessionsE(stim1, stim2, laserTrialType, _delayT delay, ITI, trialsPerSession, WaterLen, missLimit, totalSession,taskType,multDistractor)
        MixDPADNMSSessionsE(1, 3, laserTrialType, 8u, 10u, 24u, WaterLen, 30, totalSession, taskType, multDistractor);
        break;
      }



    case 4364: // DRT & DPA dual_task shaping
      {
        laserTrialType = laserOff;
        multDistractor = false;
        taskType =  DRT_DPA_SHAPING;
        highLevelShuffleLength = 20;

        int totalSession = getFuncNumber(2, "SessionNumber");
        teachSesNo = getFuncNumber(1, "SessionNumber");

        delay(2000);
        // MixDPADNMSSessionsE(stim1, stim2, laserTrialType, _delayT delay, ITI, trialsPerSession, WaterLen, missLimit, totalSession,taskType,multDistractor)
        MixDPADNMSSessionsE(5, 7, laserTrialType, 8u, 10u, 20u, WaterLen, 30, totalSession, taskType, multDistractor);
        break;
      }

    case 4365: // DRT & DPA training
      {
        int totalSession = getFuncNumber(2, "SessionNumber");

        int n = getFuncNumber(1, "laser");
        switch (n) {
          case 0:
            laserTrialType = laserOff;
            break;
          case 1:
            laserTrialType = laserDuringDelay_D;
            break;
          case 2:
            laserTrialType = laserDuring2ndDelay_D;
            break;
          case 3:
            laserTrialType = laserDuring3rdDelay_D;
            break;
          case 4:
            laserTrialType = laserDuring1stResp_D;
            break;
          case 5:
            laserTrialType = laserDuring2ndResp_D;
            break;
          case 6:
            laserTrialType = laserDuring1stOdor_D;
            break;
          case 7:
            laserTrialType = laserDuring2ndOdor_D;
            break;
          case 8:
            laserTrialType = laserDuring3rdOdor_D;
            break;
          case 9:
            laserTrialType = laserDuring4thOdor_D;
            break;
        }
        multDistractor = false;
        taskType =  DRT_DPA_TRAINING;
        highLevelShuffleLength = 24;
        delay(2000);
        MixDPADNMSSessionsE(5, 7, laserTrialType, 8u, 10u, 24u, WaterLen, 30, totalSession, taskType, multDistractor);
        break;
      }


    case 4370: // DPA & DNMS shaping
      {
        int totalSession = getFuncNumber(2, "SessionNumber");
        teachSesNo = getFuncNumber(1, "SessionNumber");

        laserTrialType = laserOff;
        taskType =  DPA_DNMS_SHAPING;
        highLevelShuffleLength = 24;

        delay(2000);
        // MixDPADNMSSessionsE(stim1, stim2, laserTrialType, _delayT delay, ITI, trialsPerSession, WaterLen, missLimit, totalSession,taskType,multDistractor)
        MixDPADNMSSessionsE(1, 3, laserTrialType, 8u, 10, 24u, WaterLen, 30, totalSession, taskType, multDistractor);
        break;
      }

    case 4371: // DPA & DNMS TRAINING
      {
        int totalSession = getFuncNumber(2, "SessionNumber");

        int n = getFuncNumber(1, "laser");
        switch (n) {
          case 0:
            laserTrialType = laserOff;
            break;
          case 1:
            laserTrialType = laserDuringDelay_D;
            break;
          case 2:
            laserTrialType = laserDuring2ndDelay_D;
            break;
          case 3:
            laserTrialType = laserDuring3rdDelay_D;
            break;
          case 4:
            laserTrialType = laserDuring1stResp_D;
            break;
          case 5:
            laserTrialType = laserDuring2ndResp_D;
            break;
        }

        taskType =  DPA_DNMS_TRAINING;
        highLevelShuffleLength = 24;

        delay(2000);
        // MixDPADNMSSessionsE(stim1, stim2, laserTrialType, _delayT delay, ITI, trialsPerSession, WaterLen, missLimit, totalSession,taskType,multDistractor)
        MixDPADNMSSessionsE(1, 3, laserTrialType, 8u, 10, 24u, WaterLen, 30, totalSession, taskType, multDistractor);
        break;
      }

    case 4400: // PID test
      analogWrite(12, 0);
      {
        int stim = getFuncNumber(2, "Valve No");
        delay(4000);
        FIDtest(1);
        delay(1000);
        int stimSend;
        if (stim == 1) {
          stimSend = SpOdor_A;
        } else if (stim == 2) {
          stimSend = SpOdor_B;
        } else if (stim == 3) {
          stimSend = SpOdor_C;
        } else if (stim == 4) {
          stimSend = SpOdor_D;
        } else if (stim == 5) {
          stimSend = SpOdor_E;
        } else if (stim == 6) {
          stimSend = SpOdor_F;
        } else if (stim == 7) {
          stimSend = SpOdor_G;
        } else if (stim == 8) {
          stimSend = SpOdor_H;
        } else if (stim == 9) {
          stimSend = SpOdor_I;
        } else if (stim == 10) {
          stimSend = SpOdor_J;
        }
        for (iter = 0; iter < 10; iter++) {

          Valve_ON(stim + 10);
          delay(500);
          Valve_ON(stim);
          analogWrite(13, 255); //BNC trigger
          serialSend(stimSend, iter + 1);
          delay(950);
          Valve_OFF(stim + 10);
          delay(50);
          Valve_OFF(stim);
          serialSend(stimSend, 0);
          analogWrite(13, 0); //BNC trigger
          for (unsigned long timeStamp = millis(); millis() < 10000ul + timeStamp;) {
            FIDtest(2);
            serialSend(FID_value, sensorValue[2] - sensorValue[1]);
            delay(1000);
          }
          FIDtest(1);
        }
        break;
      }

    case 4401: // odor system test
      {
        int stimSend;
        delay(3000);
        FIDtest(1);
        delay(1000);

        // while (1) {
        for (iter = 1; iter < 10; iter++) {
          Valve_ON(iter + 10);
          delay(500);
          Valve_ON(iter);
          analogWrite(13, 255); //BNC trigger

          if (iter == 1) {
            stimSend = SpOdor_A;
          } else if (iter == 2) {
            stimSend = SpOdor_B;
          } else if (iter == 3) {
            stimSend = SpOdor_C;
          } else if (iter == 4) {
            stimSend = SpOdor_D;
          } else if (iter == 5) {
            stimSend = SpOdor_E;
          } else if (iter == 6) {
            stimSend = SpOdor_F;
          } else if (iter == 7) {
            stimSend = SpOdor_G;
          } else if (iter == 8) {
            stimSend = SpOdor_H;
          } else if (iter == 9) {
            stimSend = SpOdor_I;
          } else if (iter == 10) {
            stimSend = SpOdor_J;
          }
          serialSend(stimSend, 1);
          delay(950);
          Valve_OFF(iter + 10);
          delay(50);
          Valve_OFF(iter);

          analogWrite(13, 0); //BNC trigger
          serialSend(stimSend, 0);

          for (unsigned long timeStamp = millis(); millis() < 5000ul + timeStamp;) {
            FIDtest(2);
            serialSend(FID_value, sensorValue[2] - sensorValue[1]);
            delay(1000);
          }
          FIDtest(1);
        }
        break;
      }

    case 4410: // ramping test
      {
        int value = 0;
        for (int i = 0; i < 250; i++) {
          analogWrite(13, value);
          delay(1000);
          value++;
          serialSend(Splaser, value);
        }
        break;
      }



    case 4455: // clean valve
      {
        for (int i = 0; i < 200; i++) {
          Valve_ON(1);
          Valve_ON(2);
          Valve_ON(3);

          Valve_OFF(4);
          Valve_OFF(5);
          Valve_OFF(6);

          delay(10 * 1000);

          Valve_OFF(1);
          Valve_OFF(2);
          Valve_OFF(3);

          Valve_ON(4);
          Valve_ON(5);
          Valve_ON(6);
          delay(10 * 1000);
        }
        break;
      }

    case 4456:
      lickTest();
      break;

    case 4444: // laser warming up
      analogWrite(13, 255);
      serialSend(Splaser, 1);
      break;

    case 4301:
      setWaterLen();
      break;

  }
}
