// MCU Testing Script - Channel A External Connections Part 2
// Channel_A_Ext_Conn2.cpp
// Switch Mode Power Supply - Senior Projects
// Azra Rangwala, Ilona Lameka, Ridwan Hussain

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

const int A_BUTT = 23; // MCU_A_GPIO23
const int V_KNOB  = 39; // MCU_A_ADC1_C3
const int C_KNOB = 34; // MCU_A_ADC1_C6

float gbl_vPotCalc, gbl_cPotCalc; 

int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); 
int prevState = 0;

void setup() 
{
  lcd.init();                  
  lcd.backlight();
  pinMode(A_BUTT, INPUT);
}

void loop() 
{
  readKnobs();
  delay(100);
  lcdScreen(gbl_vPotCalc, gbl_cPotCalc);
}

void readKnobs()
{
  // Getting the ADC Knob Values
  int vPotTotal = 0;
  int cPotTotal = 0;
  for (int i = 0; i < 100; i++) // Getting muliple samples due to ADC fluctuations
  {
    vPotTotal += analogRead(V_KNOB);
    cPotTotal += analogRead(C_KNOB);
    vTaskDelay(pdMS_TO_TICKS(1));
  }
  int vPot = vPotTotal / 100; 
  gbl_vPotCalc = ((vPot / 4095.0) * 19.0) + 1;

  int cPot = cPotTotal / 100;
  gbl_cPotCalc = (cPot / 4095.0) * 3.0;
}

void lcdScreen(float v, float c) 
{   
    lcd.setCursor(0, 0);
    char vDisplay[6];  
    dtostrf(v, 4, 2, vDisplay); 
    lcd.print(String("Voltage: ") + String(vDisplay) + String(" V       "));

    lcd.setCursor(0,1);
    char cDisplay[6];  
    dtostrf(c, 4, 2, cDisplay); 
    lcd.print(String("Current: ") + String(cDisplay) + String(" A"));
}
