#include <Arduino.h>
#include <task.h>
#include <queue.h>
#include "hw_mic.h"


// For good coding there must explains overview on top level then details.
void task_read_mic(void *pvParameters);
void task_process_mic(void *pvParameters);

void init_read_mic();
void init_process_mic();

// shared variables
SemaphoreHandle_t xSemaphore = NULL;
int mem_index = 0;
int32_t mic_samples[2 * 1600];

void setup() {
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
}

void loop() {
  // read microphone.
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
    float avg_val = 0;
    for (int i = 0; i < 1600; i++) {
      avg_val += (float)abs(mic_samples[mem_index * 1600 + i])/  1600;
    }
    Serial.println(avg_val);
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