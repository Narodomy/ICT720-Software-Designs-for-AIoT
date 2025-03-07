#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// https://www.uuidgenerator.net/
#define SERVICE_UUID "880c71bf-f476-454a-9888-4b277c89d03c"
// #define CHARACTERISTIC_UUID "e892de27-8b6a-4a40-b177-3dcadf9aaaac"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;

void setup() {
  Serial.begin(9600);
  Serial.println("Starting the BLE Server...");

  BLEDevice::init("Narodomy");

  pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);
  // Additional for send message cases.
  // // (for reading/writing data)
  // pCharacteristic = pService->createCharacteristic(
  //                     CHARACTERISTIC_UUID,
  //                     BLECharacteristic::PROPERTY_READ | 
  //                     BLECharacteristic::PROPERTY_WRITE
  //                   );

  // // initial value
  // pCharacteristic->setValue("Hello from Narodomy!");

  // pService->start();
  // Advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // Helps with iPhone connections
  pAdvertising->setMinPreferred(0x12);
  pAdvertising->addServiceUUID(SERVICE_UUID);

  BLEDevice::startAdvertising();
  Serial.println("BLE Server is running...");
  
  // from the class.
  // pServer = BLEDevice::createServer();
  // BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  // pAdvertising->setScanResponse(true);
  // pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  // pAdvertising->setMinPreferred(0x12);
  // BLEDevice::startAdvertising();

  Serial.print("Free Heap Memory: ");
  Serial.println(esp_get_free_heap_size());
}

void loop() {
  delay(5000);
  Serial.print("Free Heap Memory: ");
  Serial.println(esp_get_free_heap_size());
}
