// MCU A
// Switch Mode Power Supply - Senior Projects
// Azra Rangwala, Ilona Lameka, Ridwan Hussain

// Things to be adjusted during testing:
// frequency/resolution of PWM
// tolerance values
// time delays
// voltage and current sense calculations
// PWM calculations

// round the sense and knob float values to 3 decimal points
// serial monitor
// change variable names to indicate when they are global or not, gbl
// change f1 to something more logical

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// ADC Pins
const int CUR_SENSE = 36; // MCU_A_ADC1_C0
const int V_KNOB  = 39; // MCU_A_ADC1_C3
const int C_KNOB = 34; // MCU_A_ADC1_C6
const int VOL_SENSE = 35; // MCU_A_ADC1_C7
const int B_FEEDBACK = 33; // MCU_A_ADC1_C5

// GPIO Input Pins
const int TEMP = 32; // MCU_A_GPIO32
const int A_FUSE = 12; // MCU_A_GPIO12
const int A_BUTT = 23; // MCU_A_GPIO23
const int PARALLEL_BUTT = 17; // MCU_A_GPIO17
const int SERIES_BUTT = 16; // MCU_A_GPIO16

// GPIO Output Pins
const int BUCK = 25; // MCU_A_GPIO25
const int OUT_ENABLE = 26; // MCU_A_GPIO26
const int SERIES_RELAY = 27; // MCU_A_GPIO27
const int PARALLEL_RELAY = 13; // MCU_A_GPIO13
const int KNOBS = 19; // MCU_A_GPIO19
const int HV_LV_OUT = 18; // MCU_A_GPIO18
const int DRIVE = 4; // MCU_A_GPIO4
const int FLT_ALERT = 2; // MCU_A_GPIO2

void dominos (void*); // Core 0
void subway (void*);  // Core 1
void death (void*);   // Core 1
void lcdScreen(float, float);
void readKnobs();
void readSense();
void IRAM_ATTR isrTemp();
void IRAM_ATTR onSetupTimer();
void IRAM_ATTR onOutputTimer();
void flyDrive(float);
void buckDrive();

int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); 

float vPotCalc, cPotCalc, vSenseCalc, cSenseCalc; //set to 0?
int stateA, drivePWM, buckPWM;
int prevStateA = 0;
int f1 = 0;
hw_timer_t* setupTimer = NULL;
hw_timer_t* outputTimer = NULL;

volatile bool interrupt = false;
volatile int faultState;
enum fault{OVERTEMP, SETUPOVERTIME, OUTPUTOVERTIME,  OVERCURRENT, BLOWNFUSE};

TaskHandle_t domTaskHandle = NULL;
TaskHandle_t subTaskHandle = NULL;
TaskHandle_t deathTaskHandle = NULL;

void setup() 
{
  Serial.begin (115200);
  Serial.println("MCU Restarting...");
  delay(100);
  lcd.init();                  
  lcd.backlight();

  pinMode(A_BUTT, INPUT);
  pinMode(PARALLEL_BUTT, INPUT);
  pinMode(SERIES_BUTT, INPUT);
  
  pinMode(TEMP, INPUT);
  attachInterrupt(TEMP, isrTemp, ONLOW);

  pinMode(A_FUSE, INPUT);

  ledcSetup(0, 50000, 10);
  ledcAttachPin(DRIVE, 0);

  ledcSetup(1, 50000, 10);
  ledcAttachPin(BUCK, 1);

  pinMode(OUT_ENABLE, OUTPUT);
  pinMode(HV_LV_OUT, OUTPUT);

  setupTimer = timerBegin(0,80, true);  // State S2 Timer
  timerAttachInterrupt(setupTimer, &onSetupTimer, true);
  timerAlarmWrite(setupTimer, 30000000, false); //30s

  outputTimer = timerBegin(1,80, true); // State S3 Timer
  timerAttachInterrupt(outputTimer, &onOutputTimer, true);
  timerAlarmWrite(outputTimer, 10000000, false); //10s

  // False Start Prevention
  if (digitalRead(A_BUTT) || digitalRead(PARALLEL_BUTT) || digitalRead(SERIES_BUTT))
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

// Always running on Core 0
// Output + Interrupt Sensing
void dominos (void* pvParameters) {
  for (;;) {
    readSense();
    if (interrupt && !f1) // Happens only once 
    {
      vTaskDelete(subTaskHandle);
      vTaskDelay(pdMS_TO_TICKS(5));
      xTaskCreatePinnedToCore(death, "death", 2048, NULL, 8, &deathTaskHandle, 1);
      f1 = 1; // Restart MCU to get f1 = 0
    }
    vTaskDelay(pdMS_TO_TICKS(5)); // Feeding the dog
  }
}


void subway(void *pvParameters)
{
  for (;;)
  {
    Serial.println("subway");
    stateA = digitalRead(A_BUTT);
    if(stateA == 0)
    {
      ledcWrite(0, 0); //Drive
      ledcWrite(1, 0); //Buck
      digitalWrite(HV_LV_OUT, 0);
      digitalWrite(OUT_ENABLE, 0);
      readKnobs();
      lcdScreen(vPotCalc, cPotCalc);
      prevStateA = 0;
      vTaskDelay(pdMS_TO_TICKS(10));
    }
    else if((stateA == 1) && (prevStateA == 0))  // possibly two timers HV vs LV timer
    {
      timerRestart(setupTimer);
      timerAlarmEnable(setupTimer);
      if (vPotCalc > 10)
      {
        flyDrive(vPotCalc);
      }
      else
      {
        flyDrive(12);
        if (digitalRead(A_BUTT) && !interrupt)
        {
          digitalWrite(HV_LV_OUT, 1);
        }
        buckDrive();
      }
      timerAlarmDisable(setupTimer);
      prevStateA = 1;
    }
    else
    {
      Serial.println("s3");
      digitalWrite(OUT_ENABLE, 1);
      lcdScreen(vSenseCalc, cSenseCalc);
      // Fuse Check
      if (digitalRead(A_FUSE) && !(vSenseCalc > 0)) //check fi this actually works
      {
        faultState = BLOWNFUSE;
        interrupt = true;
      }
      // Current Check
      if ((cSenseCalc - cPotCalc) > 0.01) // if big difference in curret, kill immediately, if its below 20 mA, wait, then decide to kill or not
      {
        vTaskDelay(pdMS_TO_TICKS(15)); //delay to make sure its not a current spike //change delay later
        if ((cSenseCalc - cPotCalc) > 0.01)
        {
          faultState = OVERCURRENT;
          interrupt = true;
        }
      }
      // Voltage Check
      if (abs(vPotCalc-vSenseCalc) > 0.05) //check current in fly and drive while loops, die if over current
      {
        timerRestart(outputTimer);
        timerAlarmEnable(outputTimer);
        if (vPotCalc > 10)
        {
          Serial.println("trying");
          flyDrive(vPotCalc);
        }
        else
        {
          buckDrive();
        }
        timerAlarmDisable(outputTimer);
      }
      
      vTaskDelay(pdMS_TO_TICKS(15));
    }
  }
}

void readKnobs()
{
  // Getting the ADC Knob Values
  int vPotTotal = 0;
  int cPotTotal = 0;
  for (int i = 0; i < 50; i++) // Getting muliple samples due to ADC fluctuations
  {
    vPotTotal += analogRead(V_KNOB);
    cPotTotal += analogRead(C_KNOB);
    vTaskDelay(pdMS_TO_TICKS(1));
  }
  int vPot = vPotTotal / 50; 
  vPotCalc = ((vPot / 4095.0) * 19.0) + 1;

  int cPot = cPotTotal / 50;
  cPotCalc = (cPot / 4095.0) * 3.0;
}

void readSense()
{
  int vSense = analogRead(VOL_SENSE);
  vSenseCalc = ((vSense / 4095.0) * 20.0); // math might have to change for v and c

  int cSense = analogRead(CUR_SENSE);
  cSenseCalc = (cSense / 4095.0) * 3.0;
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

void IRAM_ATTR isrTemp() 
{
  detachInterrupt(TEMP);
  faultState = OVERTEMP;
  interrupt = true;
}

void IRAM_ATTR onSetupTimer()
{
  timerAlarmDisable(setupTimer);
  timerDetachInterrupt(setupTimer);
  faultState = SETUPOVERTIME;
  interrupt = true;
}

void IRAM_ATTR onOutputTimer()
{
  timerAlarmDisable(outputTimer);
  timerDetachInterrupt(outputTimer);
  faultState = OUTPUTOVERTIME;
  interrupt = true;
}

void death (void* pvParameters)
{
  for (;;)
  {
    Serial.println("dying");
    detachInterrupt(TEMP);
    ledcWrite(0, 0);
    ledcWrite(1, 0);
    digitalWrite(OUT_ENABLE, 0); 
    digitalWrite(HV_LV_OUT, 0);
    lcd.setCursor(0, 0);
    Serial.println("this again");
    switch(faultState)
    {
      case OVERTEMP:
      {
        lcd.print("Over Temperature");
        break;
      }
      case SETUPOVERTIME:
      {
        lcd.print("Setup Overtime    ");
        break;
      }
      case OUTPUTOVERTIME:
      {
        lcd.print("Output Overtime    ");
        break;
      }
      case OVERCURRENT:
      {
        lcd.print("Over Current    ");
        break;
      }
      case BLOWNFUSE:
      {
        lcd.print("Blown Fuse       ");
        break;
      }
    
    }
    lcd.setCursor(0, 1);
    lcd.print(String("Restart SMPS: E") + String(faultState) + String("       "));
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// fix math
// add a min and max duty cycle clamping

// Controls the Flyback PWM signals
void flyDrive(float vComp) 
{
  if (prevStateA == 0) // Only allows change to the PWM rate if a new Pot Value was given
  {
    drivePWM = (((vComp - 10) / 10) * 154) + 256; //fix math
    ledcWrite(0, drivePWM);
  }
  float err = vComp - vSenseCalc;
  do
  {
    drivePWM =  ((err / 10) * 500); //fix math
    ledcWrite(0, drivePWM);
    Serial.println(drivePWM);
    vTaskDelay(pdMS_TO_TICKS(2));
    err = vComp - vSenseCalc;
  }
  while(!interrupt && (abs(err) > 0.05) && digitalRead(A_BUTT));
  vTaskDelay(pdMS_TO_TICKS(5));
}

// Controls the Buck Converter PWM signals
void buckDrive() 
{
  if (prevStateA == 0)
  {
    buckPWM = (((vPotCalc - 10) / 10) * 154) + 256;
    ledcWrite(1, buckPWM);
  }
  float err = vPotCalc - vSenseCalc;
  do
  {
    buckPWM = ceil((((err / 10) * 154) + 256));
    ledcWrite(1, buckPWM);
    Serial.println("buck");
    Serial.println(buckPWM);
    vTaskDelay(pdMS_TO_TICKS(2));
    err = vPotCalc - vSenseCalc;
  }
  while(!interrupt && (abs(err) > 0.05) && digitalRead(A_BUTT));
}

void loop() {
  // Empty loop necessary for FreeRTOS
}
