#include <Arduino.h>
#include <Audio.h>
#include "SPIFFS.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define I2S_DOUT      1
#define I2S_BCLK      2
#define I2S_LRC       3
#define RED_PIN       10
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

int result;
bool read_result;
volatile bool pin_state = 0;
bool deviceConnected = false;
esp_err_t error;

Audio audio;

hw_timer_t *timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile uint32_t isrCounter = 0;
volatile uint32_t lastIsrAt = 0;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device Connected");
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device Disconnected");
  }
  void onWrite(BLECharacteristic* pCharacteristic)
  {
    String value = pCharacteristic->getValue();
    Serial.print("Characteristic event, written: ");
    Serial.println(static_cast<int>(value[0])); // Print the integer value
  }
};

void BluetoothSetup()
{
  BLEDevice::init("Concert ESP32S3");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setValue("Hello World says Neil");
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
}

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
  BluetoothSetup();
}
 
void loop() {
    Serial.println("Hello there!");
    Serial.println(result);
    Serial.println(read_result);
    Serial.println(esp_err_to_name(error));
    audio.loop(); 
    vTaskDelay(1);
}