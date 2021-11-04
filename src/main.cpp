#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "queue.h"

const uint8_t sw1Pin = 2;
const uint8_t sw2Pin = 3;
const uint8_t sw3Pin = 4;

const uint8_t ledRedPin = 6;
const uint8_t ledYellowPin = 7;
const uint8_t ledGreenPin = 8;

QueueHandle_t ledRedQueue;
QueueHandle_t ledYellowQueue;
QueueHandle_t ledGreenQueue;

struct SwTaskParams
{
  uint8_t swPin;
  QueueHandle_t &ledQueue;
};

SwTaskParams swTaskParams[3] = {
    {sw1Pin, ledRedQueue},
    {sw2Pin, ledYellowQueue},
    {sw3Pin, ledGreenQueue}};

void vSenderTask(void *pvParameters);
void vLedRedTask(void *pvParameters);
void vLedYellowTask(void *pvParameters);
void vLedGreenTask(void *pvParameters);

void setup()
{
  Serial.begin(9600);

  ledRedQueue = xQueueCreate(5, sizeof(int32_t));
  ledYellowQueue = xQueueCreate(5, sizeof(int32_t));
  ledGreenQueue = xQueueCreate(5, sizeof(int32_t));

  xTaskCreate(vSenderTask, "Switch 1 Task", 100, (void *)&swTaskParams[0], 1, NULL);
  xTaskCreate(vLedRedTask, "Red LED Task", 100, NULL, 1, NULL);

  //xTaskCreate(vSenderTask, "Switch 2 Task", 100, (void *)&swTaskParams[1], 1, NULL);
  //xTaskCreate(vLedYellowTask, "Yellow LED Task", 100, NULL, 1, NULL);

  //xTaskCreate(vSenderTask, "Switch 3 Task", 100, (void *)&swTaskParams[2], 1, NULL);
  //xTaskCreate(vLedGreenTask, "Green LED Task", 100, NULL, 1, NULL);
}

void vSenderTask(void *pvParameters)
{
  SwTaskParams *params = (SwTaskParams *)pvParameters;

  uint8_t swPin = params->swPin;
  QueueHandle_t &ledQueue = params->ledQueue;

  BaseType_t qStatus;
  int32_t valueToSend;

  pinMode(swPin, INPUT_PULLUP);

  while (1)
  {
    valueToSend = !digitalRead(swPin);
    qStatus = xQueueSend(ledQueue, &valueToSend, 0);

    vTaskDelay(100);
  }
}

void vLedRedTask(void *pvParameters)
{
  int32_t valueReceived = 0;
  BaseType_t qStatus;
  const TickType_t xTicksToWait = pdMS_TO_TICKS(100);

  pinMode(ledRedPin, OUTPUT);

  int lastState = 0;
  unsigned long lifeTime = 0;

  while (1)
  {
    qStatus = xQueueReceive(ledRedQueue, &valueReceived, xTicksToWait);
    if (qStatus != pdPASS)
    {
      continue;
    }

    if (lastState != valueReceived)
    {
      lastState = valueReceived;
      if (valueReceived == HIGH)
        lifeTime += 300;
    }

    Serial.println(lifeTime);

    digitalWrite(ledRedPin, lifeTime > 0 ? HIGH : LOW);

    if (lifeTime > 0)
      lifeTime -= 10;
      
    vTaskDelay(10);
  }
}

void vLedYellowTask(void *pvParameters)
{
  int32_t valueReceived;
  BaseType_t qStatus;
  const TickType_t xTicksToWait = pdMS_TO_TICKS(100);

  pinMode(ledYellowPin, OUTPUT);

  while (1)
  {
    xQueueReceive(ledYellowQueue, &valueReceived, xTicksToWait);
    digitalWrite(ledYellowPin, valueReceived);
    vTaskDelay(1);
  }
}

void vLedGreenTask(void *pvParameters)
{
  int32_t valueReceived;
  BaseType_t qStatus;
  const TickType_t xTicksToWait = pdMS_TO_TICKS(100);

  pinMode(ledGreenPin, OUTPUT);

  while (1)
  {
    xQueueReceive(ledGreenQueue, &valueReceived, xTicksToWait);
    digitalWrite(ledGreenPin, valueReceived);
    vTaskDelay(1);
  }
}

// void vReceiverTask(void *pvParameters)
// {
//   uint16_t *params = (uint16_t *)pvParameters;

//   int ledPin = params[0];
//   int swPin = params[1];

//   int32_t valueReceived;
//   BaseType_t qStatus;
//   const TickType_t xTicksToWait = pdMS_TO_TICKS(100);

//   pinMode(RED, OUTPUT);

//   while (1)
//   {
//     xQueueReceive(ledQueue, &valueReceived, xTicksToWait);
//     Serial.print("Received value : ");
//     Serial.println(valueReceived);
//     digitalWrite(RED, valueReceived);
//     vTaskDelay(1);
//   }
// }

void loop() {}