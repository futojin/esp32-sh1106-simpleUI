#include "simpleUI.h"

#define DRAW_LIST_SIZE 3 // TODO: items size is hardcoded to 3. Use screen size.

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

void ListPage::start()
{
  for (Item *item : m_pageItems)
  {
    Event initEvent = {EVENT_EMPTY, 0};
    item->onEvent(initEvent);
  }
}

bool ListPage::nextItem()
{
  if (m_pageItems.empty())
  {
    return false;
  }

  // Current iterator is already at end
  if (m_currentItemIt == m_pageItems.end())
  {
    return false;
  }

  std::list<PageItem *>::iterator nextIt = m_currentItemIt;
  do
  {
    ++nextIt;
    if (nextIt != m_pageItems.end() && (*nextIt)->isEnabled())
    {
      m_currentItemIt = nextIt;
      return true;
    }
  } while (nextIt != m_pageItems.end());

  // No enabled item found after current position
  return false;
}

bool ListPage::prevItem()
{
  if (m_pageItems.empty())
  {
    return false;
  }

  if (m_currentItemIt == m_pageItems.begin())
  {
    return false; // Already at beginning
  }

  std::list<PageItem *>::iterator prevIt = m_currentItemIt;
  do
  {
    --prevIt;
    if ((*prevIt)->isEnabled())
    {
      m_currentItemIt = prevIt;
      return true;
    }
  } while (prevIt != m_pageItems.begin());

  // No enabled item found before current position
  return false;
}

void ListPage::reset()
{
  m_currentItemIt = m_pageItems.begin();
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

void ListPage::addItem(PageItem &item)
{
  m_pageItems.push_back(&item);
  item.m_display = this->m_display;

  if (m_pageItems.size() == 1)
  {
    m_currentItemIt = m_pageItems.begin();
  }
}

void ListPage::drawItems()
{
  if (m_pageItems.empty())
  {
    return; // Nothing to draw
  }
  DEBUG_SIMPLEUI("Page::drawItems\n");

  // Collect enabled items to draw
  std::vector<Item *> enabledItems;

  // First, collect previous enabled items with aim to have the currentItem in the middle of the screen, if possible
  auto prevIt = m_currentItemIt;
  while (prevIt != m_pageItems.begin() && enabledItems.size() < (DRAW_LIST_SIZE / 2))
  {
    --prevIt;
    if ((*prevIt)->isEnabled())
    {
      enabledItems.insert(enabledItems.begin(), *prevIt);
    }
  }

  // Add current item if enabled
  if ((*m_currentItemIt)->isEnabled())
  {
    enabledItems.push_back(*m_currentItemIt);
  }

  // Find next enabled items if we have space
  auto nextIt = m_currentItemIt;
  while (nextIt != m_pageItems.end() && enabledItems.size() < DRAW_LIST_SIZE)
  {
    ++nextIt;
    if (nextIt != m_pageItems.end() && (*nextIt)->isEnabled())
    {
      enabledItems.push_back(*nextIt);
    }
  }

  // Draw the enabled items
  int drawIdx = 0;
  for (Item *item : enabledItems)
  {
    DEBUG_SIMPLEUI("Page::drawItem: %s\n", item->m_label);
    item->draw(drawIdx);

    if (item == *m_currentItemIt)
    {
      if (m_context == PAGE)
      {
        DEBUG_SIMPLEUI("Page::drawItem: *%s\n", item->m_label);
        item->drawHighlight(drawIdx);
      }
      else if (m_context == ITEM)
      {
        DEBUG_SIMPLEUI("Page::drawItem: **%s\n", item->m_label);
        item->drawValueHighlight(drawIdx);
      }
    }

    drawIdx++;
  }
}

void ListPage::onPageEvent(Event &event)
{
  if (event.value == ROTARY_EVENT_CW)
  {
    bool next = nextItem();
    if (next)
    {
      DEBUG_SIMPLEUI("Page::onEvent: PAGE CW m_currentIdx: %s\n", (*m_currentItemIt)->m_label);
    }
    else if (m_enableSaveActions) // we don't have next item, but we can overflow to save actions
    {
      DEBUG_SIMPLEUI("Page::onEvent: PAGE CW SAVE\n");
      m_context = SAVE;
    }
    else // we don't have next item and can't overflow. Yield control back to container
    {
      DEBUG_SIMPLEUI("Page::onEvent: PAGE CW YIELD()\n");
      m_context = NONE;
    }
  }
  else if (event.value == ROTARY_EVENT_CCW)
  {
    bool prev = prevItem();
    if (prev)
    {
      DEBUG_SIMPLEUI("Page::onEvent: PAGE CCW m_currentIdx: %s\n", (*m_currentItemIt)->m_label);
    } else if (m_enableSaveActions) // we don't have previous item, but we can overflow to save actions
    {
      DEBUG_SIMPLEUI("Page::onEvent: PAGE CCW SAVE\n");
      m_context = SAVE;
    }
  }
  else if (event.value == ROTARY_EVENT_PUSH)
  {
    if (!m_pageItems.empty())
    {
      DEBUG_SIMPLEUI("Page::onPush PAGE -> ITEM\n");
      m_context = ITEM;
    }
  }
}

void ListPage::onItemEvent(Event &event)
{
  switch (event.value)
  {
  case ROTARY_EVENT_CW:
  case ROTARY_EVENT_CCW:
    (*m_currentItemIt)->onEvent(event);
    break;
  case ROTARY_EVENT_PUSH:
    DEBUG_SIMPLEUI("Page::onPush ITEM -> PAGE\n");
    m_context = PAGE;
    break;
  default:
    break;
  }
}

void ListPage::syncDisplay()
{
  for (Item *item : m_pageItems)
  {
    item->m_display = this->m_display;
  }
}

HeroPage::HeroPage(const unsigned char *icon)
    : Page(icon),
      m_currentItem(nullptr)
{
}

void HeroPage::addItem(HeroPageItem &pageItem)
{
  m_currentItem = &pageItem;
  pageItem.m_display = this->m_display;
}

void HeroPage::start()
{
  Event initEvent = {EVENT_EMPTY, 0};
  m_currentItem->onEvent(initEvent);
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
    m_currentItem->onEvent(event);
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
    m_currentItem->draw(0); // HeroPageItem doesn't use idx

    if (m_context == PAGE || m_context == ITEM)
    {
      DEBUG_SIMPLEUI("HeroPage::drawItems: ValueHighlight %s\n", m_currentItem->m_label);
      m_currentItem->drawValueHighlight(0);
    }
  }
}

void HeroPage::syncDisplay()
{
  if (m_currentItem)
  {
    m_currentItem->m_display = m_display;
  }
}