// MCU Testing Script - Channel C Transformers
// Channel_C_Transformer2.cpp
// Switch Mode Power Supply - Senior Projects
// Azra Rangwala, Ilona Lameka, Ridwan Hussain

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

const int C_BUTT = 17; // MCU_B_GPIO17
const int DRIVE = 4; // MCU_B_GPIO4
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); 
// 10% = 102
// 15% = 153
// 20% = 204
int PWM = 200;
int prevStateA = 0;

void setup() 
{
  lcd.init();                  
  lcd.backlight();
  pinMode(C_BUTT, INPUT);
  ledcSetup(0, 50000, 10); // 50kHz
  ledcAttachPin(DRIVE, 0);
}

void loop() 
{
  if (digitalRead(C_BUTT) && !prevStateA)
  {
    ledcWrite(0, PWM);
    lcd.setCursor(0, 0);
    lcd.print(String("PWM: ") + String(PWM) + String("  "));
    prevStateA = 1;
  }
  else if(!digitalRead(C_BUTT))
  {
    ledcWrite(0, 0);
    lcd.setCursor(0, 0);
    lcd.print("PWM 0      ");
    prevStateA = 0;
  }
}
