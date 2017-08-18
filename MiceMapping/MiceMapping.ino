#include "TimerOne.h"
#include "commons.h"
#include <Arduino.h>
#include <avr/wdt.h>

#include "zxLibGen2.h"

#define buildAll
#ifdef buildAll
#include "zxLib.h"
#endif

bool isGen2 = false;

void setup() {

  wdt_disable();

  // pinMode 5V+ flowmeter
  // pinMode 5V+ fan
  pinMode(22, OUTPUT); // odor1  two-port valve
  pinMode(23, OUTPUT); // odor2
  pinMode(24, OUTPUT); // odor3
  pinMode(25, OUTPUT); // odor4
  pinMode(26, OUTPUT); // odor5
  pinMode(27, OUTPUT); // odor6
  pinMode(28, OUTPUT); // odor7
  pinMode(29, OUTPUT); // odor8
  pinMode(30, OUTPUT); // odor9
  pinMode(31, OUTPUT); // odor10
  pinMode(32, OUTPUT); // odor1  three-port valve
  pinMode(33, OUTPUT); // odor2
  pinMode(34, OUTPUT); // odor3
  pinMode(35, OUTPUT); // odor4
  pinMode(36, OUTPUT); // odor5
  pinMode(37, OUTPUT); // odor6
  pinMode(38, OUTPUT); // odor7
  pinMode(39, OUTPUT); // odor8
  pinMode(40, OUTPUT); // odor9
  pinMode(41, OUTPUT); // odor10
  pinMode(42, OUTPUT); // water
  pinMode(43, OUTPUT);
  pinMode(44, OUTPUT);
  pinMode(45, OUTPUT);
  pinMode(46, OUTPUT);
  pinMode(52, OUTPUT);
  pinMode(53, OUTPUT);

  digitalWrite(22, LOW);
  digitalWrite(23, LOW);
  digitalWrite(24, LOW);
  digitalWrite(25, LOW);
  digitalWrite(26, LOW);
  digitalWrite(27, LOW);
  digitalWrite(28, LOW);
  digitalWrite(29, LOW);
  digitalWrite(30, LOW);
  digitalWrite(31, LOW);
  digitalWrite(32, LOW);
  digitalWrite(33, LOW);
  digitalWrite(34, LOW);
  digitalWrite(35, LOW);
  digitalWrite(36, LOW);
  digitalWrite(37, LOW);
  digitalWrite(38, LOW);
  digitalWrite(39, LOW);
  digitalWrite(40, LOW);
  digitalWrite(41, LOW);
  digitalWrite(42, LOW);
  digitalWrite(52, LOW);
  digitalWrite(53, LOW);

  analogWrite(13, 0);
  analogWrite(12, 0); // BNC ground

  Serial.begin(19200);
  for (int i = 0; i < 10; i++) {
    byte probe[] = {20, 0x01 | 0x80};
    Serial.write(probe, 2);
    delay(50);
    if (Serial.available() > 0 && Serial.read() == 0x31) {
      isGen2 = true;
      break;
    }
  }
#ifdef buildAll
  if (!isGen2) {
    Timer1.initialize(5000);
    Timer1.attachInterrupt(zxTimer1);

    Serial.flush();
    Serial.begin(9600);
  }
#endif
}

void loop() {
  // put your main code here, to run repeatedly:
  if (isGen2) {
    // for(int i=0;i<4;i++){
    // 	digitalWrite(LED_BUILTIN,HIGH);
    // 	delay(100);
    // 	digitalWrite(LED_BUILTIN,LOW);
    // 	delay(100);
    // 	digitalWrite(LED_BUILTIN,HIGH);
    // 	delay(100);
    // 	digitalWrite(LED_BUILTIN,LOW);
    // 	delay(500);
    // }

    int n = getFuncNumberGen2(4, "Main Function?");
    randomSeed(analogRead(1));
    switch (n / 100) {
    case 43:
    case 44:
    case 55:
      callFunc(n);
      break;
    }
  }

#ifdef buildAll
  if (!isGen2) {
		delay(500);
		byte init[] = {0, 1, 2, 3};
		Serial.write(init, 4);

    while (1) {
      // for(int i=0;i<3;i++){
      // 	digitalWrite(LED_BUILTIN,HIGH);
      // 	delay(250);
      // 	digitalWrite(LED_BUILTIN,LOW);
      // 	delay(250);
      // }

      int n = getFuncNumber(4, "Main Function?");
      randomSeed(analogRead(1));
      switch (n / 100) {
      case 43:
      case 44:
      case 55:
        zxFunc(n);
        break;
      }
    }
  }
#endif
}
