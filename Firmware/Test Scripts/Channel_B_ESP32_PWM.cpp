// MCU Testing Script - Channel B ESP32
// Channel_B_ESP32_PWM.cpp
// Switch Mode Power Supply - Senior Projects
// Azra Rangwala, Ilona Lameka, Ridwan Hussain

#include <Arduino.h>

const int BUCK = 25; // MCU_A_GPIO25

int PWM = 102; // 10% Duty Cycle
int prevState = 0;

void setup() 
{
  ledcSetup(0, 35000, 10); // 35kHz, 10 bit resolution
  ledcAttachPin(BUCK, 0);
}

void loop() 
{
  if (!prevState)
  {
    ledcWrite(0, PWM);
    prevState = 1;
  }
}
