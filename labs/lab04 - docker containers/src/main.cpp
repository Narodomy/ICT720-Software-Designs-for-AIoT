#include <Arduino.h>
#include "hw_mic.h"  // Include local library
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


// Function declarations
void task_read_mic(void *pvParameters);
void task_process_mic(void *pvParameters);
void on_message(char* topic, byte* payload, unsigned int length);


// Shared variables
SemaphoreHandle_t xSemaphore = NULL;
int mem_idx = 0; 
int32_t mic_samples[2 * 16000];
float avg_val=0.0;

WiFiClient espClient;
PubSubClient mqtt_client(espClient);
JsonDocument doc;

void setup() {
  Serial.begin(12600);

  // Connect to WiFi
  WiFi.begin("ProyWanat_2.4G", "Wanat@1809");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to the WiFi network");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
  
  // Prepare semaphore
  xSemaphore = xSemaphoreCreateBinary(); // Only 0 or 1
  // Create task to read mic
  xTaskCreate(
    task_read_mic,    // Task function
    "hw_mic_task",    // Name of task
    4096,             // Stack size of task
    NULL,             // Parameter of the task
    3,                // Priority of the task
    NULL              // Task handle to keep track of created task
  );

  // Create task to process mic
  xTaskCreate(
    task_process_mic, // Task function
    "process_mic_task", // Name of task
    4096,              // Stack size of task
    NULL,              // Parameter of the task
    2,                 // Priority of the task
    NULL               // Task handle to keep track of created task
  );
  mqtt_client.setServer("broker.emqx.io", 1883);
  mqtt_client.setCallback(on_message);
  mqtt_client.connect("Narodomy");
  mqtt_client.subscribe("ict720/#");
  mqtt_client.subscribe("ict720/meowsteam/narodomy/data");
  Serial.println("Connected to MQTT broker");
}

  
void loop() {
  float data = random(0, 100)/100.0;
  char payload[100];
  doc.clear();
  doc["name"] = "DomySCP208";
  doc["millis"] = millis();
  doc["mac"] = WiFi.macAddress().c_str();
  doc["rssi"] = WiFi.RSSI();
  doc["ip"] = WiFi.localIP().toString().c_str();
  serializeJson(doc, payload);
  mqtt_client.publish("ict720/meowsteam/narodomy/data", payload);
  mqtt_client.loop();
  delay(3000);
}

void task_read_mic(void *pvParameters) {
  // Set up
  int cur_mem_idx = 0;
  hw_mic_init(16000);

  // Loop
  while (1) {
    unsigned int num_samples = 1600;
    hw_mic_read(mic_samples + cur_mem_idx * 1600, &num_samples);
    mem_idx = cur_mem_idx; // Point out the data location
    cur_mem_idx = (cur_mem_idx + 1) % 2;
    xSemaphoreGive(xSemaphore);
  }
}

void task_process_mic(void *pvParameters) {
  // Set up
  Serial.begin(9600);

  // Loop
  while (1) {
    xSemaphoreTake(xSemaphore, portMAX_DELAY);
    for (int i = 0; i < 16000; i++) {
      avg_val += (float)abs(mic_samples[mem_idx * 1600 + i]) / 16000;
    }
    //Serial.println(avg_val);
  }
}

void on_message(char *topic, byte *payload, unsigned int length)
{ 
  char buf[200];
  memcpy(buf, payload, length);
  buf[length] = '\0';
  Serial.printf("Message arrived [%s] %s\n", topic, buf);
  deserializeJson(doc, buf);
  if (doc["cmd"] == "listen"){
    Serial.println("Start listening");
    doc.clear();
    doc["status"] = "ok";
    doc["value"] = avg_val;
    serializeJson(doc, buf);
    mqtt_client.publish("ict720/#", buf);
  }
  }

