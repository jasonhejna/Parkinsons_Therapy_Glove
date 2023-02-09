/*
 * Copyright (c) 2023, Jason Hejna
 * All rights reserved.
 * 
 * This source code is licensed under the Apache-2.0 license found in the
 * LICENSE file in the root directory of this source tree. 
 *
 *
 *
 * Project Vibrotactile Coordinated Reset (vCR) Glove Firmware
 * Description: Vibrotactile Coordinated Reset delivers vibratory stimulation to the fingertips of each hand. A specific pattern of vibration to each fingertip is delivered which theoretically disrupts abnormal synchrony in the brain.
 * Author: Jason Hejna
 * Date: 1/19/2023
 */

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// screen lib setup
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/SCL, /* data=*/SDA);  // pin remapping with ESP8266 HW I2C

#define boost_en 13
#define L1F A4
#define L2F A5
#define L3F 12
#define L4F 11
#define R1F 10
#define R2F 9
#define R3F 6
#define R4F 5

// time tracking
unsigned long StartTime = millis();
unsigned long CurrentTime = millis();
unsigned long ElapsedTime = CurrentTime - StartTime;
unsigned long seconds;
unsigned long minutes;
char time_str[8];

//bool active = true;
int random_numbers[] = { 0, 1, 2, 3 };
int inter_stimulus_jitter = random(51000, 82332);
int cumulative_jitter;
int active_pinout1;
int active_pinout2;
int sequence = 0;
int vCR_sequence = 0;
uint8_t pulse = 0;

void setup() {
  pinMode(L1F, OUTPUT);
  pinMode(L2F, OUTPUT);
  pinMode(L3F, OUTPUT);
  pinMode(L4F, OUTPUT);
  pinMode(R1F, OUTPUT);
  pinMode(R2F, OUTPUT);
  pinMode(R3F, OUTPUT);
  pinMode(R4F, OUTPUT);

  pinMode(boost_en, OUTPUT);
  digitalWrite(boost_en, HIGH);

  // display
  u8g2.begin();
  u8g2_prepare();
  update_display();
  u8g2.clearBuffer();
  u8g2.drawStr(10, 10, "PD GLOVE");
  u8g2.sendBuffer();
  delay(2000);
  u8g2.setFont(u8g2_font_luBS08_te);
  u8g2.clearBuffer();
  u8g2.drawStr(30, 0, "PD GLOVE");
  for (int i = 5; i > 0; i--) {
    itoa(i, time_str, 10);
    u8g2.clearBuffer();
    u8g2.drawStr(30, 0, "PD GLOVE");
    u8g2.drawStr(50, 20, time_str);
    u8g2.sendBuffer();
    delay(1000);
  }

  // time
  StartTime = millis();

  // random number generator
  randomSeed(analogRead(0));
  generate_randoms();
}

unsigned long pulse_count = 0;


void loop() {
  // Control Voltage to achieve 3.4V or 250Hz on out 
  for (pulse = 0; pulse < 97; pulse++) {
    digitalWrite(active_pinout1, HIGH);
    digitalWrite(active_pinout2, HIGH);
    delayMicroseconds(900);
    digitalWrite(active_pinout1, LOW);
    digitalWrite(active_pinout2, LOW);
    delayMicroseconds(130);
  }
  sequence++;
  if(sequence > 3) {
    sequence = 0;
    generate_randoms();
    check_report_time();
    vCR_sequence++;
    if(vCR_sequence > 2) {
      vCR_sequence = 0;
      if(cumulative_jitter <= 799992) {
        delayMicroseconds(799992 - cumulative_jitter); // nominal 799992
        delay(1500);
      }
      else{
        delayMicroseconds(1500000 - (799992 - cumulative_jitter)); // nominal 799992
      }
    }
  }
  advance_finger();
  inter_stimulus_jitter = random(51000, 82332); // jitter +- 23.5%
  cumulative_jitter += inter_stimulus_jitter;
  delayMicroseconds(inter_stimulus_jitter);  //66.666 ms average without jitter
}

void advance_finger(){
    int random_number = random_numbers[sequence];
    switch (random_number) {
      case 0:
        active_pinout1 = L1F;
        active_pinout2 = R4F;
        break;
      case 1:
        active_pinout1 = L2F;
        active_pinout2 = R3F;
        break;
      case 2:
        active_pinout1 = L3F;
        active_pinout2 = R2F;
        break;
      case 3:
        active_pinout1 = L4F;
        active_pinout2 = R1F;
        break;
    }
}

void generate_randoms() {
  for (int i = 0; i < 4; i++) {
    int n = random(0, 4);
    int temp = random_numbers[n];
    random_numbers[n] = random_numbers[i];
    random_numbers[i] = temp;
  }
}

void check_report_time() {
  CurrentTime = millis();
  ElapsedTime = CurrentTime - StartTime;
  seconds = ElapsedTime / 1000;
  u8g2.clearBuffer();
  u8g2.drawStr(30, 0, "PD GLOVE");
  if (seconds < 60) {
    itoa(seconds, time_str, 10);
    u8g2.drawStr(20, 20, time_str);
    u8g2.drawStr(50, 20, "seconds");
  } else {
    minutes = seconds / 60;
    itoa(minutes, time_str, 10);
    u8g2.drawStr(20, 20, time_str);
    u8g2.drawStr(50, 20, "minutes");
  }
  u8g2.sendBuffer();
}

void update_display() {
  u8g2.clearBuffer();
  u8g2.drawStr(10, 10, "PD GLOVE");
  u8g2.sendBuffer();
}

void u8g2_prepare(void) {
  u8g2.setFont(u8g2_font_ncenB14_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}
