#include "Arduino.h"
#include "commons.h"


unsigned int hit = 0, miss = 0, falseAlarm = 0, correctRejection = 0, correctRatio = 0;
int currentMiss = 0;

void Valve_ON(int valve){
	digitalWrite(valve+21, HIGH);  //  2

}

void Valve_OFF(int valve){
	digitalWrite(valve+21, LOW);
}

void callResetGen2(){



  wdt_enable(WDTO_500MS);
}
