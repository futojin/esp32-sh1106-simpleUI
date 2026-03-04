#include "simpleUI.h"

#define PADDING 2

HeroPageItem::HeroPageItem(const char *label, void (*valueChangeResponder)(Item *item, const Event *event))
    : Item(label, valueChangeResponder)
{
  m_smallFont = false;
}

void HeroPageItem::draw(u_int16_t idx, uint16_t offsetX, uint16_t offsetY)
{
  const uint8_t *label_font = m_smallFont ? ArialMT_Plain_10 : ArialMT_Plain_16;
  const uint8_t *value_font = m_smallFont ? ArialMT_Plain_16 : ArialMT_Plain_24;

  m_display->setFont(label_font);
  m_display->setTextAlignment(TEXT_ALIGN_CENTER);
  m_display->drawString(m_display->getWidth() / 2, offsetY, m_label);

  m_display->setFont(value_font);
  m_display->drawString(m_display->getWidth() / 2, offsetY + label_font[1], value);
}

void HeroPageItem::drawHighlight(u_int16_t idx, uint16_t offsetX, uint16_t offsetY)
{
  this->drawValueHighlight(idx, offsetX, offsetY); // For hero item, skip drawHighlight. Expect only 1 item per page.
}

void HeroPageItem::drawValueHighlight(u_int16_t idx, uint16_t offsetX, uint16_t offsetY)
{
  const uint8_t *label_font = m_smallFont ? ArialMT_Plain_10 : ArialMT_Plain_16;
  const uint8_t *value_font = m_smallFont ? ArialMT_Plain_16 : ArialMT_Plain_24;

  m_display->setFont(value_font);
  uint16_t textWidth = m_display->getStringWidth(value);
  int16_t x = (m_display->getWidth() / 2 - textWidth / 2) - PADDING;
  int16_t y = offsetY + label_font[1];
  m_display->drawRect(x, y, textWidth + 2 * PADDING, value_font[1]);
}
