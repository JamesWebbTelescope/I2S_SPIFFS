#include <Arduino.h>
#include <Audio.h>
#include "SPIFFS.h"

#define I2S_DOUT      1
#define I2S_BCLK      2
#define I2S_LRC       3
 
int result;
bool read_result;
esp_err_t error;

Audio audio;

void setup() {
  Serial.begin(9600);
  
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  
  File file = SPIFFS.open("/text.txt");
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }
  
  Serial.println("File Content:");
  while(file.available()){
    result = file.read();
    Serial.write(file.read());
  }
  file.close();

  error = audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    
  // Set Volume
  audio.setVolume(21);
    
  // Open music file
  read_result = audio.connecttoFS(SPIFFS,"/05 Synthetic Bloodline.mp3");
}
 
void loop() {
    Serial.println("Hello there!");
    Serial.println(result);
    /*File file = SPIFFS.open("/05 Synthetic Bloodline.mp3", "r");
    Serial.println(file.name());
    if(!file){
      Serial.println("Failed to open file for reading");
      return;
    }
    
    Serial.println("File Content:");
    while(file.available()){
      result = file.read();
      Serial.println("Available");
      Serial.write(file.read());
    }
    file.close();*/
    Serial.println(read_result);
    Serial.println(esp_err_to_name(error));
    audio.loop(); 
    vTaskDelay(1);
}