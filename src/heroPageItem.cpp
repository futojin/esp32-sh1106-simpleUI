#include "simpleUI.h"

#define DRAW_LABEL_HEIGHT 19
#define DRAW_VALUE_HEIGHT 28

HeroPageItem::HeroPageItem(const char *label, void (*valueChangeResponder)(Item *item, const Event *event))
    : Item(label, valueChangeResponder)
{
}

void HeroPageItem::draw(u_int16_t idx)
{
  m_display->setFont(ArialMT_Plain_16);
  m_display->setTextAlignment(TEXT_ALIGN_CENTER);
  m_display->drawString(m_display->getWidth() / 2, 0, m_label);

  m_display->setFont(ArialMT_Plain_24);
  m_display->drawString(m_display->getWidth() / 2, DRAW_LABEL_HEIGHT, value);
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
  int16_t y = DRAW_LABEL_HEIGHT;
  m_display->drawRect(x, y, textWidth + 4, DRAW_VALUE_HEIGHT); // +2 padding
}
