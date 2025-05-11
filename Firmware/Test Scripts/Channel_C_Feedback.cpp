// MCU Testing Script - Channel C Feedback
// Channel_C_Feedback.cpp
// Switch Mode Power Supply - Senior Projects
// Azra Rangwala, Ilona Lameka, Ridwan Hussain

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

const int A_BUTT = 17; // MCU_B_GPIO23

const int OUT_ENABLE = 13; // MCU_B_GPIO26 //set to 0
const int VOL_SENSE = 32; // MCU_B_ADC1_C7
const int DRIVE = 4; // MCU_A_GPIO4
const int BUCK = 25; // MCU_A_GPIO25

void dominos (void*); // Core 0
void subway (void*);  // Core 1
void readSense();

int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); 
int once = 0;
float vSenseCalc;//set to 0?
int stateA;
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
  digitalWrite(HV_LV_OUT, 1);

  xTaskCreatePinnedToCore(dominos, "Dominatrix", 2048, NULL, 2, &domTaskHandle, 0);
  xTaskCreatePinnedToCore(subway, "Sub", 2048, NULL, 2, &subTaskHandle, 1);
}

void dominos (void* pvParameters) {
  for (;;) {
    readSense();
    vTaskDelay(pdMS_TO_TICKS(50)); // Feeding the dog
  }
}

void subway(void *pvParameters)
{
  for (;;)
  {
    if (digitalRead(A_BUTT) && !prevState)
    {
      // LV TEST
      int drive_PWM = 102;
      int buck_PWM = 100; //100 for increment 910 for dec
      for(int i = 0; (i <= 19 && digitalRead(A_BUTT)); i++)
      {
        ledcWrite(1, drive_PWM);
        Serial.println(drive_PWM);
        drive_PWM += 10; //Math to increase by voltage
        delay(100); 
      }
      ledcWrite(1, drive_PWM-5); //297 is 12V
      delay(100);
      
      ledcWrite(0, buck_PWM);
      delay(1000);
      digitalWrite(HV_LV_OUT, 0);
      delay(15000);
      digitalWrite(OUT_ENABLE, 1);
      for(int k = 0; (k <= 41 && digitalRead(A_BUTT)); k++)
      {
        ledcWrite(0, buck_PWM);
        Serial.println(buck_PWM);
        buck_PWM += 20; //Math to increase by voltage
        delay(10000);
      }
      prevState = 1;
    }
    else if (!digitalRead(A_BUTT))
    {
      ledcWrite(1, 0);
      digitalWrite(OUT_ENABLE, 0);
      digitalWrite(HV_LV_OUT, 1);
      ledcWrite(0, 0);
      prevState = 0;
    }
    delay(100);
  }
}

void readSense()
{
  int vTotal = 0;
  for (int i = 0; i < 300; i++) // Getting muliple samples due to ADC fluctuations
  {
    vTotal += analogRead(VOL_SENSE);
    vTaskDelay(pdMS_TO_TICKS(1));
  }
  int vSense = vTotal / 300; 
  lcd.setCursor(0, 0);
  lcd.print(String("Voltage:") + String(vSense) + String("          "));
}

void loop() {
  // Empty loop necessary for FreeRTOS
}
