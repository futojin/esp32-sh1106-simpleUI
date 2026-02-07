#include "simpleUI.h"

HeroPage::HeroPage(const unsigned char *icon)
    : Page(icon),
      m_currentItem(nullptr)
{
}

void HeroPage::addItem(HeroPageItem &pageItem)
{
  m_currentItem = &pageItem;
  item_syncDisplay(pageItem);
}

void HeroPage::start()
{
  Event initEvent = {EVENT_EMPTY, 0};
  item_onEvent(*m_currentItem, initEvent);
}

void HeroPage::reset()
{
}

void HeroPage::onPageEvent(Event &event)
{
  DEBUG_SIMPLEUI("HeroPage::onPageEvent:Event: %d %lu\n", event.eventId, event.value);
  m_context = ITEM;
  onItemEvent(event); // There is no rotary events in hero page
}

void HeroPage::onItemEvent(Event &event)
{
  DEBUG_SIMPLEUI("HeroPage::onItemEvent:Event: %d %lu\n", event.eventId, event.value);
  if (m_currentItem == nullptr)
  {
    DEBUG_SIMPLEUI("HeroPage::onPush ITEM -> NONE\n");
    m_context = NONE;
    return;
  }

  switch (event.value)
  {
  case ROTARY_EVENT_CW:
  case ROTARY_EVENT_CCW:
    item_onEvent(*m_currentItem, event);
    break;
  case ROTARY_EVENT_PUSH:
    if (m_enableSaveActions)
    {
      DEBUG_SIMPLEUI("HeroPage::onPush ITEM -> SAVE\n");
      m_context = SAVE;
    }
    else
    {
      DEBUG_SIMPLEUI("HeroPage::onPush ITEM -> NONE\n");
      m_context = NONE;
    }
    break;
  default:
    break;
  }
}

void HeroPage::drawItems()
{
  DEBUG_SIMPLEUI("HeroPage::drawItems\n");
  if (m_currentItem->isEnabled())
  {
    item_draw(*m_currentItem, 0); // HeroPageItem doesn't use idx

    if (m_context == PAGE || m_context == ITEM)
    {
      DEBUG_SIMPLEUI("HeroPage::drawItems: ValueHighlight %s\n", m_currentItem->m_label);
      item_drawValueHighlight(*m_currentItem, 0);
    }
  }
}

void HeroPage::syncDisplay()
{
  if (m_currentItem)
  {
    item_syncDisplay(*m_currentItem);
  }
}