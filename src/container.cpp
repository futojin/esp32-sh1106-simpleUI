#include "simpleUI.h"

#define DRAW_MARGIN 2
#define DRAW_SPACING 2
#define MAX_DISPLAY_BRIGHTNESS 128
#define MIN_DISPLAY_BRIGHTNESS 15
#define MIN_SCREEN_SAVER_TIMEOUT_SEC 5

// Define static members
Container::WatchdogTaskParams Container::s_watchdogTaskParams;
Container *Container::s_containerInstance = nullptr;

// Singleton factory methods
Container &Container::getInstance(SH1106Wire &display)
{
  if (s_containerInstance == nullptr)
  {
    s_containerInstance = new Container(display);
  }
  return *s_containerInstance;
}

Container &Container::getInstance(SH1106Wire &display, u_int8_t tra, u_int8_t trb, u_int8_t psh)
{
  if (s_containerInstance == nullptr)
  {
    s_containerInstance = new Container(display, tra, trb, psh);
  }
  return *s_containerInstance;
}

void onContainerRotaryEvent(ROTARY_EVENT rEvent);
void onContainerSwitchEvent(u_int8_t pinState);

Container::Container(SH1106Wire &display)
    : m_display(&display),
      m_currentPage(nullptr),
      m_navbar(display, *this),
      m_context(NAVBAR),
      m_idx(0),
      m_watchdogTaskHandle(nullptr),
      m_screenBrightness(MAX_DISPLAY_BRIGHTNESS), // Initialize to full brightness
      m_screenSaverTimeoutSec(0),                 // Initialize screen saver timeout to 0 (disabled)
      m_lastActivityMs(millis())
{
  m_rotaryDebounce = nullptr;
  m_switchDebounce = nullptr;
}

Container::Container(SH1106Wire &display, u_int8_t tra, u_int8_t trb, u_int8_t psh)
    : Container(display)
{
  m_rotaryDebounce = new RotaryDebounce(tra, trb, onContainerRotaryEvent);
  m_switchDebounce = new SwitchDebounce(psh, onContainerSwitchEvent);
}

Container::~Container()
{
  // Clean up FreeRTOS task
  if (m_watchdogTaskHandle)
  {
    vTaskDelete(m_watchdogTaskHandle);
    m_watchdogTaskHandle = nullptr;
  }

  // Clean up heap-allocated objects
  if (m_rotaryDebounce)
  {
    delete m_rotaryDebounce;
    m_rotaryDebounce = nullptr;
  }

  if (m_switchDebounce)
  {
    delete m_switchDebounce;
    m_switchDebounce = nullptr;
  }
}

void onContainerRotaryEvent(ROTARY_EVENT rEvent)
{
  Event event;
  event.eventId = EVENT_ROT;
  event.value = rEvent;
  DEBUG_SIMPLEUI("Got Rotary Event: %s\n", (rEvent == ROTARY_EVENT_CW) ? "CW" : "CCW");
  Container::s_containerInstance->onEvent(event);
}

void onContainerSwitchEvent(u_int8_t pinState)
{
  if (pinState == LOW)
  {
    Event event;
    event.eventId = EVENT_ROT;
    event.value = ROTARY_EVENT_PUSH;
    DEBUG_SIMPLEUI("Got Push Event: %s\n", (pinState == HIGH) ? "HIGH" : "LOW");
    Container::s_containerInstance->onEvent(event);
  }
}

void Container::initDisplay(bool flipVertical)
{
  m_display->init();
  m_display->clear();
  flipDisplay(flipVertical);
  this->drawOverlay();

  m_display->display();
}

void Container::addPage(Page &childPage)
{
  childPage.m_display = this->m_display;
  childPage.syncDisplay();
  m_pages.push_back(&childPage);
  m_navbar.addPage(childPage);
  childPage.m_container = this;

  if (m_currentPage == nullptr) // If this is the first page, assume current
  {
    m_currentPage = &childPage;
    m_idx = 0;
  }
}

void Container::setCurrentPage(Page &newPage)
{
  // Check if the mainPage is in m_pages vector
  for (size_t i = 0; i < m_pages.size(); i++)
  {
    Page *page = m_pages[i];
    if (page == &newPage)
    {
      m_currentPage = &newPage;
      m_idx = i;
      return;
    }
  }
  // Page not found in m_pages, do nothing.
}

void Container::draw()
{
  DEBUG_SIMPLEUI("Container::draw\n");
  m_display->clear();
  drawOverlay();
  m_navbar.draw(*m_currentPage);
  if (m_currentPage)
  {
    m_currentPage->draw();
  }
  m_display->display();
}

void Container::drawOverlay()
{
  int width = m_display->getWidth();
  int height = m_display->getHeight();
  m_display->drawRect(0, 0, width, height);
}

void Container::onEvent(Event &event)
{
  unsigned long id = millis() % 1000;
  DEBUG_SIMPLEUI("-- %lu Container::onEvent %d %lu----\n", id, event.eventId, event.value);
  DEBUG_SIMPLEUI("Container::onEvent:m_context %d\n", m_context);
  DEBUG_SIMPLEUI("Container::onEvent:m_idx %d\n", m_idx);

  // Reset activity time and brightness on any user interaction
  m_lastActivityMs = millis();

  if (m_screenBrightness < MAX_DISPLAY_BRIGHTNESS) // Don't unnecessarily change brightness as it make the display flicker
  {
    m_screenBrightness = MAX_DISPLAY_BRIGHTNESS;
    m_display->setBrightness(MAX_DISPLAY_BRIGHTNESS);
    draw();
    DEBUG_SIMPLEUI("Container::onEvent: Woke from screen saver, ignoring event\n");
    return; // Don't process 1st interaction after waking from screen saver
  }

  if (event.eventId == EVENT_ROT)
  {
    if (m_context == NAVBAR)
    {
      DEBUG_SIMPLEUI("Container::onEvent NAVBAR:%d\n", event.value);
      trackCurrentPage((ROTARY_EVENT)event.value);
      m_navbar.onEvent(event);
    }
    else if (m_context == PAGE)
    {
      DEBUG_SIMPLEUI("Container::onEvent PAGE:%d\n", event.value);
      m_currentPage->onEvent(event);
    }
  }

  draw();
  DEBUG_SIMPLEUI("-- %lu ------------\n", id);
}

void Container::onEventYield(Event &event)
{
  DEBUG_SIMPLEUI("Container::onEventYield\n");
  if (event.eventId == EVENT_YIELD)
  {
    CONTEXT who = (CONTEXT)event.value;
    Event pushEvent = {EVENT_ROT, ROTARY_EVENT_PUSH};
    if (who == NAVBAR)
    {
      m_context = PAGE;
      DEBUG_SIMPLEUI("Container::onEventYield: EVENT_YIELD: NAVBAR -> PAGE\n");
      m_currentPage->onEvent(pushEvent);
    }
    else if (who == PAGE)
    {
      m_context = NAVBAR;
      DEBUG_SIMPLEUI("Container::onEventYield: EVENT_YIELD: PAGE -> NAVBAR\n");
      m_navbar.onEvent(pushEvent);
    }
  }
}

void Container::trackCurrentPage(ROTARY_EVENT rEvent)
{
  DEBUG_SIMPLEUI("Container::trackCurrentPage: %d\n", rEvent);
  if (rEvent == ROTARY_EVENT_CW && m_idx < m_pages.size() - 1)
  {
    u_int8_t nextIdx = nextEnabledPage();
    if (nextIdx != m_idx)
    {
      m_idx = nextIdx;
      m_currentPage = m_pages[m_idx];
    }
  }
  else if (rEvent == ROTARY_EVENT_CCW && m_idx > 0)
  {
    u_int8_t prevIdx = previousEnabledPage();
    if (prevIdx != m_idx)
    {
      m_idx = prevIdx;
      m_currentPage = m_pages[m_idx];
    }
  }
}

void Container::start()
{
  for (Page *page : m_pages)
  {
    page->start();
  }
  m_lastActivityMs = millis();
  draw();
  if (m_rotaryDebounce)
  {
    m_rotaryDebounce->start();
  }
  if (m_switchDebounce)
  {
    m_switchDebounce->start();
  }
}

void Container::createWatchdogTask()
{
  if (m_watchdogTaskHandle == nullptr)
  {
    xTaskCreate(
        onWatchdogTask,
        "watchdog Task",
        1024,
        &s_watchdogTaskParams,
        1 | portPRIVILEGE_BIT,
        &m_watchdogTaskHandle);

    s_watchdogTaskParams.container = this;
  }
}

void Container::enableScreenSaver(u_int8_t timeoutSec)
{
  createWatchdogTask();

  m_screenSaverTimeoutSec = timeoutSec < MIN_SCREEN_SAVER_TIMEOUT_SEC ? MIN_SCREEN_SAVER_TIMEOUT_SEC : timeoutSec;
  m_lastActivityMs = millis();
}

void Container::disableScreenSaver()
{
  m_screenSaverTimeoutSec = 0;
}

void Container::onWatchdogTask(void *parameter)
{
  WatchdogTaskParams *params = (WatchdogTaskParams *)parameter;
  Container *container = params->container;

  for (;;)
  {
    // Check every second
    vTaskDelay(pdMS_TO_TICKS(1000));

    if (container->m_screenSaverTimeoutSec > 0)
    {
      container->screenSaverTask(params);
    }
  }
}

void Container::screenSaverTask(WatchdogTaskParams *params)
{
  u_int32_t timeoutMs = m_screenSaverTimeoutSec * 1000;
  if (timeoutMs < MIN_SCREEN_SAVER_TIMEOUT_SEC * 1000)
  {
    return; // Last check before proceeding for thread safety.
  }

  unsigned long lastActivity = m_lastActivityMs;
  u_int32_t currentTime = millis();
  u_int32_t elapsedMs = currentTime - lastActivity;
  if (elapsedMs >= timeoutMs)
  {
    if (m_screenBrightness > MIN_DISPLAY_BRIGHTNESS)
    {
      m_screenBrightness = MIN_DISPLAY_BRIGHTNESS;

      m_display->setBrightness(MIN_DISPLAY_BRIGHTNESS);
      m_display->clear();
      drawOverlay();
      m_display->display();
    }
  }
}

u_int8_t Container::nextEnabledPage()
{
  for (u_int8_t i = m_idx + 1; i < m_pages.size(); i++)
  {
    if (m_pages[i]->enabled())
    {
      return i;
    }
  }
  return m_idx;
}

u_int8_t Container::previousEnabledPage()
{
  for (int i = m_idx - 1; i >= 0; i--)
  {
    if (m_pages[i]->enabled())
    {
      return i;
    }
  }
  return m_idx;
}

void Container::flipDisplay(bool flipVertical)
{
  if (flipVertical)
  {
    m_display->flipScreenVertically();
  }
  else
  {
    m_display->resetOrientation();
  }
  draw();
}