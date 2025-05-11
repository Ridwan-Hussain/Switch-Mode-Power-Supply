// MCU Testing Script - Channel A Enable
// Channel_A_Enable1.cpp
// Switch Mode Power Supply - Senior Projects
// Azra Rangwala, Ilona Lameka, Ridwan Hussain

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

const int OUT_ENABLE = 26; // MCU_A_GPIO26
const int A_BUTT = 23; // MCU_A_GPIO23
const int BUCK = 25; // MCU_A_GPIO25
const int HV_LV_OUT = 18; // MCU_A_GPIO18
const int DRIVE = 4; // MCU_A_GPIO4

void dominos (void*); // Core 0
void subway (void*);  // Core 1

int once = 0;
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); 
int prevState = 0;

TaskHandle_t domTaskHandle = NULL;
TaskHandle_t subTaskHandle = NULL;

void setup() 
{
  lcd.init();                  
  lcd.backlight();

  pinMode(A_BUTT, INPUT);

  ledcSetup(0, 35000, 10); 
  ledcAttachPin(BUCK, 0);

  ledcSetup(1, 42000, 10);
  ledcAttachPin(DRIVE, 1);

  pinMode(HV_LV_OUT, OUTPUT);
  pinMode(OUT_ENABLE, OUTPUT);
  digitalWrite(HV_LV_OUT, 1); //have this be off somehwere

  // False Start Prevention
  if (digitalRead(A_BUTT))
  {
    while(1)
    {
      lcd.setCursor(0, 0);
      lcd.print("Turn Off Buttons");
      lcd.setCursor(0, 1);
      lcd.print("Restart SMPS               ");
    }
  }
  xTaskCreatePinnedToCore(dominos, "Dominatrix", 2048, NULL, 2, &domTaskHandle, 0);
  xTaskCreatePinnedToCore(subway, "Sub", 2048, NULL, 2, &subTaskHandle, 1);
}

void dominos (void* pvParameters) {
  for (;;) {
    int f1 = 4 + 4;
    vTaskDelay(pdMS_TO_TICKS(50)); // Feeding the dog
  }
}

void subway(void *pvParameters)
{
  for (;;)
  {
    if (!once)
    {
      delay(5000);
      int drive_PWM = 102; // DO NOT CHANGE
      int buck_PWM = 512; //100 for increment 910 for dec
      for(int i = 0; (i <= 19); i++)
      {
        ledcWrite(1, drive_PWM);
        drive_PWM += 10; //Math to increase by voltage
        delay(100); 
      }
      ledcWrite(1, drive_PWM-5); //297 is 12V
      delay(10000);
      
      ledcWrite(0, buck_PWM);
      delay(1000);
      digitalWrite(HV_LV_OUT, 0);
      once = 1;
    }
    
    if (digitalRead(A_BUTT) && !prevState)
    {
      digitalWrite(OUT_ENABLE, 1);
      lcd.setCursor(0, 0);
      lcd.print("OUT ON             ");
      prevState = 1;
    }
    else if (!digitalRead(A_BUTT))
    {
      digitalWrite(OUT_ENABLE, 0);
      lcd.setCursor(0, 0);
      lcd.print("OUT OFF               ");
      prevState = 0;
    }
    delay(100);
  }
}

void loop() {
  // Empty loop necessary for FreeRTOS
}

/*
// MCU Testing Script - Channel A Enable
// Channel_A_Enable1.cpp
// Switch Mode Power Supply - Senior Projects
// Azra Rangwala, Ilona Lameka, Ridwan Hussain

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

const int A_BUTT = 23; // MCU_A_GPIO23
const int HV_LV_OUT = 18; // MCU_A_GPIO18
const int OUT_ENABLE = 26; // MCU_A_GPIO26
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); 
int prevState = 0;

void setup() 
{
  //Serial.begin (115200);
  //Serial.println("MCU Restarting...");
  lcd.init();                  
  lcd.backlight();
  pinMode(A_BUTT, INPUT);
  pinMode(HV_LV_OUT, OUTPUT);
  pinMode(OUT_ENABLE, OUTPUT);
  digitalWrite(HV_LV_OUT, 1);
}

void loop() 
{
  if (digitalRead(A_BUTT) && !prevState)
  {
    digitalWrite(OUT_ENABLE, 1);
    lcd.setCursor(0, 0);
    lcd.print("Output Enabled                   ");
    prevState = 1;
  }
  else if (!digitalRead(A_BUTT))
  {
    digitalWrite(OUT_ENABLE, 0);
    lcd.setCursor(0, 0);
    lcd.print("No Output                    ");
    prevState = 0;
  }
  delay(100);
}
*/
