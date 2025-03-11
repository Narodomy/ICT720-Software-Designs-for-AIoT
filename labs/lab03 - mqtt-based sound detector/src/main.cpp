#include <Arduino.h>
#include <task.h>
#include <queue.h>
#include "hw_mic.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


// For good coding there must explains overview on top level then details.
void task_read_mic(void *pvParameters);
void task_process_mic(void *pvParameters);
void on_message(char* topic, byte* payload, unsigned int length);

void init_read_mic();
void init_process_mic();

// shared variables
SemaphoreHandle_t xSemaphore = NULL;
int mem_index = 0;
int32_t mic_samples[2 * 1600];
WiFiClient espClient;
PubSubClient mqtt_client(espClient);
JsonDocument doc;
float avg_val = 0;
// float bin[1600];



void setup() {
  delay(3000);
  Serial.begin(9600);
  WiFi.begin("ProyWanat_2.4G", "Wanat@1809");
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("RSSI: %d\n", WiFi.RSSI());

  // 1. prepare semaphore.
  xSemaphore = xSemaphoreCreateBinary();
  // 2. create task to read microphone.
  xTaskCreate(
    task_read_mic,    // task function
    "task read mic",  // name of task
    4096,           // stack size of task
    NULL,           // parameter of the task
    3,              // priority of the task
    NULL           // task handle to keep track of created task
  );
  // 3. create task to process microphone data
  xTaskCreate(
    task_process_mic,    // task function
    "task process mic",  // name of task
    4096,           // stack size of task
    NULL,           // parameter of the task
    2,              // priority of the task
    NULL           // task handle to keep track of created task
  );
  // Connect MQTT
  // supachai/esp32/cmd
  mqtt_client.setServer("broker.emqx.io",1883);
  mqtt_client.setCallback(on_message);
  mqtt_client.connect("narodomy_domain0x2c");
  mqtt_client.subscribe("ict720/narodom/esp32/beat");
  Serial.println("Conneted to MQTT broker");
}

void loop() {
  // read microphone.
  // very important.
  mqtt_client.loop();
  delay(1000);
}


void init_read_mic() {
  hw_mic_init(16000);
}

void init_process_mic() {
  Serial.begin(115200);
}

void task_read_mic(void *pvParameters) {
  // setup()
  init_read_mic();
  int cur_mem_index = 0;
  // loop()
  while(true) {
    unsigned int num_samples = 1600;
    hw_mic_read(mic_samples + cur_mem_index * 1600, &num_samples);
    mem_index = cur_mem_index;
    cur_mem_index = (cur_mem_index + 1) % 2;
    xSemaphoreGive(xSemaphore);
  }
}
void task_process_mic(void *pvParameters) {
  // setup()
  init_process_mic();
  // loop()
  while(true) {
    xSemaphoreTake(xSemaphore, portMAX_DELAY);
    avg_val = 0;
    for (int i = 0; i < 1600; i++) {
      avg_val += (float)abs(mic_samples[mem_index * 1600 + i])/  1600;
      // bin[i] = avg_val;
    }
    // Serial.println(avg_val);
  }
}

void on_message(char* topic, byte* payload, unsigned int length) {
  char buf[200];
  memcpy(buf, payload, length);
  buf[length] = '\0';
  Serial.printf("Received message from topic %s: %s\n", topic, buf);
  deserializeJson(doc, buf);
  if (doc["cmd"] == "listen") {
    // do something
    Serial.println("Start listening");
    doc.clear();
    doc["status"] = "ok";
    doc["value"] = avg_val;
    // for (int i = 0; i < 12; i++) {
    //   doc["party_values"].add(bin[i]);
    // }
    serializeJson(doc, buf);
    mqtt_client.publish("ict720/narodom/esp32/resp", buf);
  }
}
// void loop() {
//   delay(1000);
//   int32_t buf[160];
//   unsigned int num_samples = 16000;
//   hw_mic_read(buf, &num_samples);
//   float avg_val = 0;
//   for (int i = 0; i < num_samples; i++) {
//     avg_val += (float)abs(buf[i])/num_samples;
//   }  
//   Serial.printf("%f",avg_val);
// }