// MCU Testing Script - Channel A Current Sense & OVP
// Channel_A_CURR_SENSE.cpp
// Switch Mode Power Supply - Senior Projects
// Azra Rangwala, Ilona Lameka, Ridwan Hussain

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

const int CUR_SENSE = 36; // MCU_A_ADC1_C0
const int A_BUTT = 23; // MCU_A_GPIO23
const int BUCK = 25; // MCU_A_GPIO25
const int HV_LV_OUT = 18; // MCU_A_GPIO18
const int DRIVE = 4; // MCU_A_GPIO4

void dominos (void*); // Core 0
void subway (void*);  // Core 1
void readSense();

int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); 

float cSenseCalc; //set to 0?
int prevState = 0;

TaskHandle_t domTaskHandle = NULL;
TaskHandle_t subTaskHandle = NULL;

void setup() 
{
  //Serial.begin (115200);
  //Serial.println("MCU Restarting...");
  lcd.init();                  
  lcd.backlight();

  pinMode(A_BUTT, INPUT);

  ledcSetup(0, 35000, 10); 
  ledcAttachPin(BUCK, 0);

  ledcSetup(1, 42000, 10);
  ledcAttachPin(DRIVE, 1);

  pinMode(HV_LV_OUT, OUTPUT);

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
    readSense();
    vTaskDelay(pdMS_TO_TICKS(50)); // Feeding the dog
  }
}

void subway(void *pvParameters)
{
  for (;;)
  {
    // For Transformer AND Buck
    if (digitalRead(A_BUTT) && !prevState)
    {
      int drive_PWM = 102;
      int buck_PWM = 100; //100 for increment 910 for dec
      for(int i = 0; (i <= 19 && digitalRead(A_BUTT)); i++)
      {
        ledcWrite(1, drive_PWM);
        drive_PWM += 10; //Math to increase by voltage
        delay(100); 
      }
      ledcWrite(1, drive_PWM-5); //297 is 12V
      delay(100);
      
      ledcWrite(0, buck_PWM);
      delay(1000);
      digitalWrite(HV_LV_OUT, 0);
      delay(15000);

      for(int k = 0; (k <= 41 && digitalRead(A_BUTT)); k++)
      {
        ledcWrite(0, buck_PWM);
        buck_PWM += 20; //Math to increase by voltage
        delay(3000);
      }
      prevState = 1;
    }
    else if (!digitalRead(A_BUTT))
    {
      ledcWrite(1, 0);
      digitalWrite(HV_LV_OUT, 1);
      ledcWrite(0, 0);
      prevState = 0;
    }
    delay(100);
  
  /*
  // FOR JUST BUCK
  if (digitalRead(A_BUTT) && !prevState)
    {
      digitalWrite(HV_LV_OUT, 0);
      delay(100);
      for(int i = 0; (i <= 2 && digitalRead(A_BUTT)); i++)
      {
        int buck_PWMs[] = {910, 512, 100};
        ledcWrite(0, buck_PWMs[i]);
        delay(5000);
        
      }
      prevState = 1;
    }
    else if (!digitalRead(A_BUTT))
    {
      digitalWrite(HV_LV_OUT, 1);
      ledcWrite(0, 0);
      prevState = 0;
    }
    vTaskDelay(pdMS_TO_TICKS(100)); */
  }
}

void readSense()
{
  int curr = 0;
  int cTotal = 0;
  for (int i = 0; i < 300; i++) // Getting muliple samples due to ADC fluctuations
  {
    cTotal += analogRead(CUR_SENSE);
    vTaskDelay(pdMS_TO_TICKS(1));
  }
  int cSense = cTotal / 300; 
  lcd.setCursor(0, 0);
  lcd.print(String("Current:") + String(cSense) + String("          "));
}

void loop() {
  // Empty loop necessary for FreeRTOS
}
