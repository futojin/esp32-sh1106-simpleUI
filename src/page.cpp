#include "simpleUI.h"
Event PAGE_YIELD = {EVENT_YIELD, PAGE};

Page::Page(const unsigned char *icon)
    : m_icon(icon),
      m_display(nullptr),
      m_context(NONE),
      m_enabled(true),
      m_enableSaveActions(false)
{
}

void Page::draw()
{
  DEBUG_SIMPLEUI("Page::draw\n");
  drawItems();
  drawSaveActions();
}

void Page::checkAndYield()
{
  if (m_context == NONE)
  {
    DEBUG_SIMPLEUI("Page::checkAndYield: YIELD()\n");
    reset();
    m_container->onEventYield(PAGE_YIELD);
  }
}

void Page::onEvent(Event &event)
{
  DEBUG_SIMPLEUI("Page::onEvent:Event: %d %lu\n", event.eventId, event.value);
  DEBUG_SIMPLEUI("Page::onEvent:Context: %d\n", m_context);
  if (m_context == PAGE && event.eventId == EVENT_ROT)
  {
    onPageEvent(event);
    DEBUG_SIMPLEUI("Page::onEvent:New-Context: %d\n", m_context);
    checkAndYield();
  }
  else if (m_context == ITEM && event.eventId == EVENT_ROT)
  {
    onItemEvent(event);
    DEBUG_SIMPLEUI("Page::onEvent:New-Context: %d\n", m_context);
    checkAndYield();
  }
  else if (m_context == SAVE && event.eventId == EVENT_ROT)
  {
    onSaveEvent(event);
    DEBUG_SIMPLEUI("Page::onEvent:New-Context: %d\n", m_context);
    checkAndYield();
  }
  else if (m_context == EXIT && event.eventId == EVENT_ROT)
  {
    onExitEvent(event);
    DEBUG_SIMPLEUI("Page::onEvent:New-Context: %d\n", m_context);
    checkAndYield();
  }
  else if (m_context == NONE && event.eventId == EVENT_ROT && event.value == ROTARY_EVENT_PUSH)
  {
    DEBUG_SIMPLEUI("Page::onPush: NONE -> PAGE\n");
    m_context = PAGE;
  }
}

void Page::onSaveEvent(Event &event)
{
  // inconsistent state, yield to container.
  if (!m_enableSaveActions)
  {
    DEBUG_SIMPLEUI("Page::onEvent: SAVE inconsistent YIELD()\n");
    m_context = NONE;
    return;
  }

  if (event.value == ROTARY_EVENT_CW)
  {
    DEBUG_SIMPLEUI("Page::onEvent: SAVE -> EXIT\n");
    m_context = EXIT;
  }
  else if (event.value == ROTARY_EVENT_CCW)
  {
    DEBUG_SIMPLEUI("Page::onEvent: SAVE -> PAGE\n");
    m_context = PAGE;
  }
  else if (event.value == ROTARY_EVENT_PUSH)
  {
    if (m_enableSaveActions && onSave)
    {
      DEBUG_SIMPLEUI("Page::onSaveEvent: onSave callback\n");
      onSave();
    }
    DEBUG_SIMPLEUI("Page::onPush YIELD(SAVE)\n");
    m_context = NONE;
  }
}

void Page::onExitEvent(Event &event)
{
  if (!m_enableSaveActions)
  {
    DEBUG_SIMPLEUI("Page::onEvent: SAVE inconsistent YIELD()\n");
    m_context = NONE;
    return;
  }

  if (event.value == ROTARY_EVENT_CCW || event.value == ROTARY_EVENT_CW)
  {
    DEBUG_SIMPLEUI("Page::onEvent: EXIT -> SAVE\n");
    m_context = SAVE;
  }
  else if (event.value == ROTARY_EVENT_PUSH)
  {
    if (m_enableSaveActions && onExit)
    {
      DEBUG_SIMPLEUI("Page::onExitEvent: onExit callback\n");
      onExit();
      start(); // re-get values
    }
    DEBUG_SIMPLEUI("Page::onPush YIELD(EXIT)\n");
    m_context = NONE;
  }
}

void Page::drawSaveActions()
{
  if (m_enableSaveActions && m_context != NONE)
  {
    DEBUG_SIMPLEUI("Page::drawSaveActions\n");
    int width = m_display->getWidth();
    int height = m_display->getHeight();

    // Draw on the bottom-right corner
    int16_t y = height - ICON_SIZE - 1; //-1 for border
    int16_t x_exit = width - ICON_SIZE - 1;
    int16_t x_save = x_exit - ICON_SIZE;
    if (m_context == SAVE) // Save icon selected
    {
      m_display->drawRect(x_save, y, ICON_SIZE, ICON_SIZE);
    }
    else if (m_context == EXIT) // Exit icon selected
    {
      m_display->drawRect(x_exit, y, ICON_SIZE, ICON_SIZE);
    }

    m_display->drawXbm(x_exit, y, ICON_SIZE, ICON_SIZE, icon_back);
    m_display->drawXbm(x_save, y, ICON_SIZE, ICON_SIZE, icon_save);
  }
}
void Page::enableSaveActions(void (*onSave)(), void (*onExit)())
{
  m_enableSaveActions = true;
  this->onSave = onSave;
  this->onExit = onExit;
}

void Page::disableSaveActions()
{
  m_enableSaveActions = false;
  this->onSave = nullptr;
  this->onExit = nullptr;
}