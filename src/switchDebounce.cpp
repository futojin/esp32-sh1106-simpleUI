#include "simpleUI.h"
#include <Arduino.h>

#define QUEUE_LENGTH 10
#define DEBOUNCE_TIME_MS 20

static QueueHandle_t switchDebounceCallbackQueue = nullptr;
static TaskHandle_t switchDebounceCallbackHandler = nullptr;

void IRAM_ATTR switchDebounce_isr(void *arg)
{
  if (arg == nullptr)
  {
    return;
  }

  SwitchDebounce *debounceInstance = (SwitchDebounce *)arg;

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xTimerResetFromISR(debounceInstance->m_debounceTimer, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken == pdTRUE)
  {
    portYIELD_FROM_ISR();
  }
}

void switchDebounce_timerCallback(TimerHandle_t xTimer)
{
  SwitchDebounce *debounceInstance = (SwitchDebounce *)pvTimerGetTimerID(xTimer);

  if (debounceInstance == nullptr)
  {
    return;
  }

  int pinState = digitalRead(debounceInstance->m_pin);
  if (pinState == debounceInstance->m_lastPinState)
  {
    return;
  }
  debounceInstance->m_lastPinState = pinState;
  xQueueSend(switchDebounceCallbackQueue, &debounceInstance, 0);
}

void handleSwitchDebounceCallbackTask(void *param)
{
  for (;;)
  {
    SwitchDebounce *debounceInstance;
    if (xQueueReceive(switchDebounceCallbackQueue, &debounceInstance, portMAX_DELAY))
    {
      DEBUG_SIMPLEUI("SwitchDebounce Callback: pin %d state %d\n", debounceInstance->m_pin, debounceInstance->m_lastPinState);
      DEBUG_SIMPLEUI("--------------------------------------------\n");
      // Call the user-defined callback
      debounceInstance->onSwitchEvent(debounceInstance->m_lastPinState);
    }
  }
}

SwitchDebounce::SwitchDebounce(u_int8_t pin, void (*onSwitchEvent)(u_int8_t pinState))
    : m_pin(pin),
      m_debounceTimer(nullptr),
      onSwitchEvent(onSwitchEvent)
{
  if (switchDebounceCallbackQueue == nullptr)
  {
    switchDebounceCallbackQueue = xQueueCreate(QUEUE_LENGTH, sizeof(SwitchDebounce *));
  }

  if (switchDebounceCallbackHandler == nullptr)
  {
    xTaskCreate(
        handleSwitchDebounceCallbackTask, // Function that implements the task.
        "Switch Debounce Callback Task",  // Text name for the task.
        4096,                             // Stack size in bytes.
        nullptr,                          // Parameter passed into the task.
        2 | portPRIVILEGE_BIT,            // Priority at which the task is created. (0=lowest)
        &switchDebounceCallbackHandler);  // Used to pass out the created task's handle.
  }
}

SwitchDebounce::~SwitchDebounce()
{
  detachInterrupt(digitalPinToInterrupt(m_pin));

  if (m_debounceTimer != nullptr)
  {
    xTimerStop(m_debounceTimer, 0);
    xTimerDelete(m_debounceTimer, 0);
    m_debounceTimer = nullptr;
  }
}

void SwitchDebounce::start()
{
  DEBUG_SIMPLEUI("SwitchDebounce::start on pin %d\n", m_pin);

  m_lastPinState = digitalRead(m_pin);

  if (m_debounceTimer != nullptr)
  {
    return;
  }

  m_debounceTimer = xTimerCreate(
      "Switch Debounce Timer",
      pdMS_TO_TICKS(DEBOUNCE_TIME_MS),
      pdFALSE, // one-shot timer
      (void *)this,
      switchDebounce_timerCallback);

  attachInterruptArg(
      digitalPinToInterrupt(m_pin),
      switchDebounce_isr,
      (void *)this,
      CHANGE);
}
