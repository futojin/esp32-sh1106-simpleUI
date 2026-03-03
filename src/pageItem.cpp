#include "simpleUI.h"

#define DRAW_X_MARGIN 1
#define DRAW_ITEM_HEIGHT 13

PageItem::PageItem(const char *label, void (*valueChangeResponder)(Item *item, const Event *event))
    : Item(label, valueChangeResponder)
{
}

void PageItem::draw(u_int16_t idx, uint16_t offsetX, uint16_t offsetY)
{
  int16_t y = offsetY + idx * DRAW_ITEM_HEIGHT;

  m_display->setFont(ArialMT_Plain_10);
  m_display->setTextAlignment(TEXT_ALIGN_LEFT);
  m_display->drawString(DRAW_X_MARGIN + offsetX, y, m_label);

  m_display->setTextAlignment(TEXT_ALIGN_RIGHT);
  m_display->drawString(m_display->getWidth() - DRAW_X_MARGIN, y, value);
}

void PageItem::drawHighlight(u_int16_t idx, uint16_t offsetX, uint16_t offsetY)
{
  int16_t y = offsetY + idx * DRAW_ITEM_HEIGHT;
  int16_t x = offsetX;
  m_display->drawRect(x, y, m_display->getWidth() - offsetX, DRAW_ITEM_HEIGHT);
}

void PageItem::drawValueHighlight(u_int16_t idx, uint16_t offsetX, uint16_t offsetY)
{
  int16_t y = offsetY + idx * DRAW_ITEM_HEIGHT;
  int16_t textWidth = m_display->getStringWidth(value);
  int16_t x = m_display->getWidth() - DRAW_X_MARGIN - textWidth - 2;
  m_display->drawRect(x, y, m_display->getWidth() - x, DRAW_ITEM_HEIGHT);
}
