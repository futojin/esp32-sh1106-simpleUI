#include "simpleUI.h"

#define DRAW_X_MARGIN 3
#define DRAW_FONT_10_HEIGHT 13
#define DRAW_FONT_16_HEIGHT 19
#define DRAW_FONT_24_HEIGHT 28

Item::Item(const char *label, void (*valueChangeResponder)(Item *item, const Event *event))
    : m_label(label),
      m_display(nullptr),
      onValueChange(valueChangeResponder),
      m_enabled(true)
{
}

void Item::onEvent(Event &event)
{
  // We can't be picky on event types here, just forward events to responder.
  if (onValueChange)
  {
    DEBUG_SIMPLEUI("Item::onEvent: Calling onValueChange: %d %lu\n", event.eventId, event.value);
    onValueChange(this, &event);
  }
}

PageItem::PageItem(const char *label, void (*valueChangeResponder)(Item *item, const Event *event))
    : Item(label, valueChangeResponder)
{
}

HeroPageItem::HeroPageItem(const char *label, void (*valueChangeResponder)(Item *item, const Event *event))
    : Item(label, valueChangeResponder)
{
}

void PageItem::draw(u_int16_t idx)
{
  int16_t y = idx * DRAW_FONT_10_HEIGHT;

  m_display->setFont(ArialMT_Plain_10);
  m_display->setTextAlignment(TEXT_ALIGN_LEFT);
  m_display->drawString(DRAW_X_MARGIN, y, m_label);

  m_display->setTextAlignment(TEXT_ALIGN_RIGHT);
  m_display->drawString(m_display->getWidth() - DRAW_X_MARGIN, y, value);
}

void PageItem::drawHighlight(u_int16_t idx)
{
  int16_t y = idx * DRAW_FONT_10_HEIGHT + 1; // +1 for border
  int16_t x = 1;
  m_display->drawRect(x, y, m_display->getWidth() - 2, DRAW_FONT_10_HEIGHT - 1); //-2 for border
}

void PageItem::drawValueHighlight(u_int16_t idx)
{
  int16_t y = idx * DRAW_FONT_10_HEIGHT + 1; // +1 for border
  int16_t textWidth = m_display->getStringWidth(value);
  int16_t x = m_display->getWidth() - DRAW_X_MARGIN - textWidth - 2;             // -1 for border, -1 for padding
  m_display->drawRect(x, y, textWidth + DRAW_X_MARGIN, DRAW_FONT_10_HEIGHT - 1); // -1 for border
}

void HeroPageItem::draw(u_int16_t idx)
{
  m_display->setFont(ArialMT_Plain_16);
  m_display->setTextAlignment(TEXT_ALIGN_CENTER);
  m_display->drawString(m_display->getWidth() / 2, 0, m_label);

  m_display->setFont(ArialMT_Plain_24);
  m_display->drawString(m_display->getWidth() / 2, DRAW_FONT_16_HEIGHT, value);
}

void HeroPageItem::drawHighlight(u_int16_t idx)
{
  this->drawValueHighlight(idx); // For hero item, skip drawHighlight. Expect only 1 item per page.
}

void HeroPageItem::drawValueHighlight(u_int16_t idx)
{
  m_display->setFont(ArialMT_Plain_24);
  uint16_t textWidth = m_display->getStringWidth(value);
  int16_t x = (m_display->getWidth() / 2 - textWidth / 2) - 2; // -2 padding
  int16_t y = DRAW_FONT_16_HEIGHT;
  m_display->drawRect(x, y, textWidth + 4, DRAW_FONT_24_HEIGHT); // +2 padding
}