
#include "simpleUI.h"
#include <Arduino.h>

#define MAX_ROTARY_STATE_TRANSITION_MS 500
#define QUEUE_LENGTH 10

static QueueHandle_t rotaryDebounceIsrQueue = nullptr;
static QueueHandle_t callbackQueue = nullptr;

static TaskHandle_t rotaryDebounceIsrHandler = nullptr;
static TaskHandle_t rotaryDebounceCallbackHandler = nullptr;

void IRAM_ATTR rotary_isr(void *arg)
{
  if (arg == nullptr)
  {
    return;
  }
  RotaryDebounce *debounceInstance = (RotaryDebounce *)arg;

  RotaryDebounce::IsrTaskParams params;
  params.interruptMs = millis();
  params.debounceInstance = debounceInstance;

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(rotaryDebounceIsrQueue, &params, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken)
  {
    portYIELD_FROM_ISR();
  }
}

void handleRotaryDebounceQueueTask(void *parameter)
{
  for (;;)
  {
    RotaryDebounce::IsrTaskParams params;
    if (xQueueReceive(rotaryDebounceIsrQueue, &params, portMAX_DELAY))
    {
      params.debounceInstance->abInterrupt(params.interruptMs);
    }
  }
}

void handleRotaryCallbackTask(void *parameter)
{
  for (;;)
  {
    RotaryDebounce::CallbackTaskParams params;
    if (xQueueReceive(callbackQueue, &params, portMAX_DELAY))
    {
      DEBUG_SIMPLEUI("RotaryDebounce Event: %d\n", params.event);
      DEBUG_SIMPLEUI("--------------------------------------------\n");
      params.debounceInstance->onRotaryEvent(params.event);
    }
  }
}

RotaryDebounce::RotaryDebounce(const u_int8_t pinA, const u_int8_t pinB, void (*rotaryEventResponder)(ROTARY_EVENT event))
    : m_pinA(pinA),
      m_pinB(pinB),
      onRotaryEvent(rotaryEventResponder)
{
  if (rotaryDebounceIsrQueue == nullptr)
  {
    rotaryDebounceIsrQueue = xQueueCreate(QUEUE_LENGTH, sizeof(IsrTaskParams));
  }
  if (callbackQueue == nullptr)
  {
    callbackQueue = xQueueCreate(QUEUE_LENGTH, sizeof(CallbackTaskParams));
  }

  configureTask();
}

RotaryDebounce::~RotaryDebounce()
{
  detachInterrupt(digitalPinToInterrupt(m_pinA));
  detachInterrupt(digitalPinToInterrupt(m_pinB));
}

void RotaryDebounce::start()
{
  DEBUG_SIMPLEUI("Starting RotaryDebounce on pins A(%d), B(%d)\n", m_pinA, m_pinB);
  attachInterruptArg(digitalPinToInterrupt(m_pinA), rotary_isr, (void *)this, CHANGE);
  attachInterruptArg(digitalPinToInterrupt(m_pinB), rotary_isr, (void *)this, CHANGE);
}

void RotaryDebounce::configureTask()
{
  if (rotaryDebounceIsrHandler == nullptr)
  {
    xTaskCreate(
        handleRotaryDebounceQueueTask, // Function that implements the task.
        "Debounce Queue Task",         // Text name for the task.
        2048,                          // Stack size in bytes.
        nullptr,                       // Task input parameter.
        3 | portPRIVILEGE_BIT,         // Priority at which the task is created. (0=lowest)
        &rotaryDebounceIsrHandler);    // Used to pass out the created task's handle.
  }

  if (rotaryDebounceCallbackHandler == nullptr)
  {
    xTaskCreate(
        handleRotaryCallbackTask,
        "Rotary Callback Task",
        4096,
        nullptr,
        2 | portPRIVILEGE_BIT,
        &rotaryDebounceCallbackHandler);
  }
}

void RotaryDebounce::abInterrupt(unsigned long currentMs)
{
  int pinAState = digitalRead(m_pinA);
  int pinBState = digitalRead(m_pinB);
  if (m_rotaryState.phase != RESET && currentMs - m_rotaryState.startMs > MAX_ROTARY_STATE_TRANSITION_MS)
  {
    DEBUG_SIMPLEUI("Timeout exceeded. (%lu ms)\n", currentMs - m_rotaryState.startMs);
    resetState();
  }

  DEBUG_SIMPLEUI("abInterrupt: A=%d, B=%d\n", pinAState, pinBState);
  DEBUG_SIMPLEUI("Current Phase: %d\n", m_rotaryState.phase);

  switch (m_rotaryState.phase)
  {
  case RESET: // RESET -> S1
    if (pinAState == LOW && pinBState == HIGH)
    {
      m_rotaryState.phase = S1;
      m_rotaryState.direction = ROTARY_EVENT_CW;
      m_rotaryState.startMs = currentMs;
      break;
    }
    if (pinAState == HIGH && pinBState == LOW)
    {
      m_rotaryState.phase = S1;
      m_rotaryState.direction = ROTARY_EVENT_CCW;
      m_rotaryState.startMs = currentMs;
      break;
    }
    // all other transitions are invalid
    resetState();
    break;

  case S1:
  case S2:
  case S3:
    if (m_rotaryState.direction == ROTARY_EVENT_CW)
    {
      cw(pinAState, pinBState);
      break;
    }
    if (m_rotaryState.direction == ROTARY_EVENT_CCW)
    {
      ccw(pinAState, pinBState);
      break;
    }
    // all other transitions are invalid
    resetState();
    break;

  default:
    resetState();
    break;
  }

  DEBUG_SIMPLEUI("New Phase: %d, direction: %d\n", m_rotaryState.phase, m_rotaryState.direction);
  if (m_rotaryState.phase == S4)
  {
    ROTARY_EVENT event = m_rotaryState.direction;
    resetState();

    RotaryDebounce::CallbackTaskParams params;
    params.debounceInstance = this;
    params.event = event;
    xQueueSend(callbackQueue, &params, 0);
  }
}

void RotaryDebounce::resetState()
{
  m_rotaryState.phase = RESET;
  m_rotaryState.direction = ROTARY_EVENT_CW;
  m_rotaryState.startMs = 0;
}

void RotaryDebounce::cw(int pinAState, int pinBState)
{
  switch (m_rotaryState.phase)
  {
  case S1:
    // S1 -> S2
    if (pinAState == LOW && pinBState == LOW)
    {
      m_rotaryState.phase = S2;
      break;
    }
    // S1 -> S1 (B bouncing)
    if (pinAState == LOW && pinBState == HIGH)
    {
      break;
    }
    // all other conditions are invalid
    resetState();
    break;

  case S2:
    // S2 -> S3
    if (pinAState == HIGH && pinBState == LOW)
    {
      m_rotaryState.phase = S3;
      break;
    }
    // S2 -> S2 (A bouncing)
    if (pinAState == LOW && pinBState == LOW)
    {
      break;
    }
    // all other conditions are invalid
    resetState();
    break;

  case S3:
    // S3 -> S4 (complete)
    if (pinAState == HIGH && pinBState == HIGH)
    {
      m_rotaryState.phase = S4;
      break;
    }
    // S3 -> S3 (B bouncing)
    if (pinAState == HIGH && pinBState == LOW)
    {
      break;
    }
    // all other conditions are invalid
    resetState();
    break;

  default:
    resetState();
    break;
  }
}

void RotaryDebounce::ccw(int pinAState, int pinBState)
{
  switch (m_rotaryState.phase)
  {
  case S1:
    // S1 -> S2
    if (pinAState == LOW && pinBState == LOW)
    {
      m_rotaryState.phase = S2;
      break;
    }
    // S1 -> S1 (A bouncing)
    if (pinAState == HIGH && pinBState == LOW)
    {
      break;
    }
    // all other conditions are invalid
    resetState();
    break;

  case S2:
    // S2 -> S3
    if (pinAState == LOW && pinBState == HIGH)
    {
      m_rotaryState.phase = S3;
      break;
    }
    // S2 -> S2 (B bouncing)
    if (pinAState == LOW && pinBState == LOW)
    {
      break;
    }
    // all other conditions are invalid
    resetState();
    break;

  case S3:
    // S3 -> S4 (complete)
    if (pinAState == HIGH && pinBState == HIGH)
    {
      m_rotaryState.phase = S4;
      break;
    }
    // S3 -> S3 (A bouncing)
    if (pinAState == LOW && pinBState == HIGH)
    {
      break;
    }
    // all other conditions are invalid
    resetState();
    break;

  default:
    resetState();
    break;
  }
}