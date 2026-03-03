#include "simpleUI.h"

#define DRAW_LABEL_HEIGHT 19
#define DRAW_VALUE_HEIGHT 28
#define DRAW_HIGHLIGHT_PADDING 2

HeroPageItem::HeroPageItem(const char *label, void (*valueChangeResponder)(Item *item, const Event *event))
    : Item(label, valueChangeResponder)
{
}

void HeroPageItem::draw(u_int16_t idx, uint16_t offsetX, uint16_t offsetY)
{
  m_display->setFont(ArialMT_Plain_16);
  m_display->setTextAlignment(TEXT_ALIGN_CENTER);
  m_display->drawString(m_display->getWidth() / 2, offsetY, m_label);

  m_display->setFont(ArialMT_Plain_24);
  m_display->drawString(m_display->getWidth() / 2, offsetY + DRAW_LABEL_HEIGHT, value);
}

void HeroPageItem::drawHighlight(u_int16_t idx, uint16_t offsetX, uint16_t offsetY)
{
  this->drawValueHighlight(idx, offsetX, offsetY); // For hero item, skip drawHighlight. Expect only 1 item per page.
}

void HeroPageItem::drawValueHighlight(u_int16_t idx, uint16_t offsetX, uint16_t offsetY)
{
  m_display->setFont(ArialMT_Plain_24);
  uint16_t textWidth = m_display->getStringWidth(value);
  int16_t x = (m_display->getWidth() / 2 - textWidth / 2) - DRAW_HIGHLIGHT_PADDING;
  int16_t y = offsetY + DRAW_LABEL_HEIGHT;
  m_display->drawRect(x, y, textWidth + 2 * DRAW_HIGHLIGHT_PADDING, DRAW_VALUE_HEIGHT);
}
