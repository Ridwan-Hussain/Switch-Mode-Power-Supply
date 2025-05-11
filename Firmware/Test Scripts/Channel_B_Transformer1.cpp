// MCU Testing Script - Channel B Transformers
// Channel_B_Transformer1.cpp
// Switch Mode Power Supply - Senior Projects
// Azra Rangwala, Ilona Lameka, Ridwan Hussain

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

const int A_BUTT = 23; // MCU_B_GPIO23
const int DRIVE = 12; // MCU_B_GPIO12
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); 
// 10% = 102
// 15% = 153
// 20% = 204
int PWM = 102;
int prevStateA = 0;

void setup() 
{
  lcd.init();                  
  lcd.backlight();
  pinMode(A_BUTT, INPUT);
  ledcSetup(0, 42000, 10); // 42kHz
  ledcAttachPin(DRIVE, 0);
}

void loop() 
{
  if (digitalRead(A_BUTT) && !prevStateA)
  {
    ledcWrite(0, PWM);
    lcd.setCursor(0, 0);
    lcd.print(String("PWM: ") + String(PWM) + String("  "));
    prevStateA = 1;
  }
  else if(!digitalRead(A_BUTT))
  {
    ledcWrite(0, 0);
    lcd.setCursor(0, 0);
    lcd.print("PWM 0      ");
    prevStateA = 0;
  }
}
