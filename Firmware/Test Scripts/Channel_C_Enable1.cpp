// MCU Testing Script - Channel C Enable
// Channel_C_Enable.cpp
// Switch Mode Power Supply - Senior Projects
// Azra Rangwala, Ilona Lameka, Ridwan Hussain

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

const int C_BUTT = 17; // MCU_C_GPIO23
const int VOL_SENSE = 32; // MCU_C_ADC1_C7
const int ENABLE = 13;


void dominos (void*); // Core 0
void subway (void*);  // Core 1
void readSense();

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

  pinMode(C_BUTT, INPUT);
  pinMode(ENABLE, OUTPUT);

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
    if (digitalRead(C_BUTT) && !prevState)
    {
      digitalWrite(ENABLE,1);
      prevState = 1;
    }
    else if (!digitalRead(C_BUTT))
    {
      digitalWrite(ENABLE,0);
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
  lcd.print(String(vSense));
}

void loop() {
  // Empty loop necessary for FreeRTOS
}
