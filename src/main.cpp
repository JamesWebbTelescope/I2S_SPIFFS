#include <Arduino.h>
#include <Audio.h>
#include "SPIFFS.h"

#define I2S_DOUT      1
#define I2S_BCLK      2
#define I2S_LRC       3
#define RED_PIN       10
 
int result;
bool read_result;
volatile bool pin_state = 0;
esp_err_t error;

Audio audio;

hw_timer_t *timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile uint32_t isrCounter = 0;
volatile uint32_t lastIsrAt = 0;

void IRAM_ATTR onTimer() {
  // Increment the counter and set the time of ISR
  digitalWrite(RED_PIN, !digitalRead(RED_PIN));
  //digitalWrite(RED_PIN, !digitalRead(RED_PIN));
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_RED, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  
  File file = SPIFFS.open("/text.txt");
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }

  timerSemaphore = xSemaphoreCreateBinary();

  // Set timer frequency to 1Mhz
  timer = timerBegin(1000000);

  // Attach onTimer function to our timer.
  timerAttachInterrupt(timer, &onTimer);

  timerAlarm(timer, 1000000, true, 0);

  error = audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    
  // Set Volume
  audio.setVolume(21);
    
  // Open music file
  read_result = audio.connecttoFS(SPIFFS,"/05 Synthetic Bloodline.mp3");
}
 
void loop() {
    Serial.println("Hello there!");
    Serial.println(result);
    Serial.println(read_result);
    Serial.println(esp_err_to_name(error));
    audio.loop(); 
    vTaskDelay(1);
}