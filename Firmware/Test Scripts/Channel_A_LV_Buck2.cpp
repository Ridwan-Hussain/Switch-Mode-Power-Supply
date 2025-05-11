// MCU Testing Script - Channel A HV/LV
// Channel_A_LV_Buck2.cpp
// Switch Mode Power Supply - Senior Projects
// Azra Rangwala, Ilona Lameka, Ridwan Hussain

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

const int A_BUTT = 23; // MCU_A_GPIO23
const int HV_LV_OUT = 18; // MCU_A_GPIO18
const int BUCK = 25; // MCU_A_GPIO25
const int DRIVE = 4; // MCU_A_GPIO4
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); 
int prevState = 0;



void setup() 
{
  Serial.begin (115200);
  Serial.println("MCU Restarting...");
  lcd.init();                  
  lcd.backlight();
  pinMode(A_BUTT, INPUT);
  pinMode(HV_LV_OUT, OUTPUT);
  ledcSetup(0, 35000, 10); 
  ledcAttachPin(BUCK, 0);

  ledcSetup(1, 42000, 10); 
  ledcAttachPin(DRIVE, 1);

  digitalWrite(HV_LV_OUT, 1);
}

void loop() 
{
  if (digitalRead(A_BUTT) && !prevState)
  {
    int drive_PWM = 102;
    int buck_PWM = 100; //100 for increment 910 for dec
    for(int i = 0; (i <= 19 && digitalRead(A_BUTT)); i++)
    {
      
      lcd.setCursor(0, 0);
      lcd.print(String("Drive PWM: ") + String(drive_PWM) + String("              "));
      lcd.setCursor(0, 1);
      lcd.print(String("I: ") + String(i) + String("                   "));
      ledcWrite(1, drive_PWM);
      Serial.println(drive_PWM);
      drive_PWM += 10; //Math to increase by voltage
      delay(100); 
    }
    ledcWrite(1, drive_PWM-5); //297 is 12V
    delay(10);
    lcd.setCursor(0, 0);
    lcd.print(String(drive_PWM-5) + String("                   "));
    delay(100);
    
    ledcWrite(0, buck_PWM);
    delay(1000);
    //Serial.println(PWM);
    digitalWrite(HV_LV_OUT, 0);
    
    lcd.setCursor(0, 0);
    lcd.print("Buck                          ");
    lcd.setCursor(0, 1);
    lcd.print("                             ");
    delay(1000);

    for(int k = 0; (k <= 81 && digitalRead(A_BUTT)); k++)
    {
      lcd.setCursor(0, 0);
      lcd.print(String("Buck PWM: ") + String(buck_PWM) + String("              "));
      lcd.setCursor(0, 1);
      lcd.print(String("K: ") + String(k) + String("                   "));
      ledcWrite(0, buck_PWM);
      Serial.println(buck_PWM);
      buck_PWM += 10; //Math to increase by voltage
      delay(100);
    }
    prevState = 1;
  }
  else if (!digitalRead(A_BUTT))
  {
    ledcWrite(1, 0);
    digitalWrite(HV_LV_OUT, 1);
    ledcWrite(0, 0);
    lcd.setCursor(0, 0);
    lcd.print("No PWM                    ");
    prevState = 0;
  }
  delay(100);
}
