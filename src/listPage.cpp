#include "simpleUI.h"

#define DRAW_LIST_SIZE 3 // TODO: items size is hardcoded to 3. Use screen size.

void ListPage::addItem(PageItem &item)
{
  m_pageItems.push_back(&item);
  item_syncDisplay(item);

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
    item_draw(*item, drawIdx);

    if (item == *m_currentItemIt)
    {
      if (m_context == PAGE)
      {
        DEBUG_SIMPLEUI("Page::drawItem: *%s\n", item->m_label);
        item_drawHighlight(*item, drawIdx);
      }
      else if (m_context == ITEM)
      {
        DEBUG_SIMPLEUI("Page::drawItem: **%s\n", item->m_label);
        item_drawValueHighlight(*item, drawIdx);
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
    }
    else if (m_enableSaveActions) // we don't have previous item, but we can overflow to save actions
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
    item_onEvent(**m_currentItemIt, event);
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
    item_syncDisplay(*item);
  }
}

void ListPage::start()
{
  for (Item *item : m_pageItems)
  {
    Event initEvent = {EVENT_EMPTY, 0};
    item_onEvent(*item, initEvent);
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
