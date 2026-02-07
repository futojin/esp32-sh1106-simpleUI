#include "simpleUI.h"

Navbar::Navbar(SH1106Wire &display, Container &container)
    : m_display(&display),
      m_context(NAVBAR),
      m_container(&container)
{
}

void Navbar::draw(const Page &currentPage)
{
  DEBUG_SIMPLEUI("Navbar::draw\n");
  if (m_context != NAVBAR)
  {
    return;
  }

  u_int16_t iconIdx = 0;

  // Draw on the bottom
  int height = m_display->getHeight();
  int16_t y = height - ICON_SIZE - 1; //-1 for border

  for (const Page *thisPage : m_pages)
  {
    if (!thisPage->enabled())
    {
      continue;
    }

    int16_t x = iconIdx * ICON_SIZE + 1;
    if (thisPage == &currentPage)
    {
      m_display->drawRect(x, y, ICON_SIZE, ICON_SIZE);
    }
    m_display->drawXbm(x, y, ICON_SIZE, ICON_SIZE, thisPage->getIcon());
    iconIdx++;
  }
}

void Navbar::addPage(Page &page)
{
  m_pages.push_back(&page);
}

void Navbar::onEvent(Event &event)
{
  // Navbar doesn't track CW and CCW. currentPage tracking is done by Container
  // Navbar only handle push events
  if (event.eventId != EVENT_ROT || event.value != ROTARY_EVENT_PUSH)
  {
    return;
  }

  // Push event essentially toggle between NAVBAR and NONE (Page)
  if (m_context == NONE)
  {
    DEBUG_SIMPLEUI("Navbar::onPush: NONE -> NAVBAR\n");
    m_context = NAVBAR;
  }
  else if (m_context == NAVBAR)
  {
    m_context = NONE;
    DEBUG_SIMPLEUI("Navbar::onPush: NAVBAR -> NONE\n");
    Event yieldToContainer = {EVENT_YIELD, NAVBAR};
    m_container->onEventYield(yieldToContainer);
  }
}
