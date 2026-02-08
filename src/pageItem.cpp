#include "simpleUI.h"

#define DRAW_X_MARGIN 3
#define DRAW_ITEM_HEIGHT 13

PageItem::PageItem(const char *label, void (*valueChangeResponder)(Item *item, const Event *event))
    : Item(label, valueChangeResponder)
{
}

void PageItem::draw(u_int16_t idx)
{
  int16_t y = idx * DRAW_ITEM_HEIGHT;

  m_display->setFont(ArialMT_Plain_10);
  m_display->setTextAlignment(TEXT_ALIGN_LEFT);
  m_display->drawString(DRAW_X_MARGIN, y, m_label);

  m_display->setTextAlignment(TEXT_ALIGN_RIGHT);
  m_display->drawString(m_display->getWidth() - DRAW_X_MARGIN, y, value);
}

void PageItem::drawHighlight(u_int16_t idx)
{
  int16_t y = idx * DRAW_ITEM_HEIGHT + 1; // +1 for border
  int16_t x = 1;
  m_display->drawRect(x, y, m_display->getWidth() - 2, DRAW_ITEM_HEIGHT - 1); //-2 for border
}

void PageItem::drawValueHighlight(u_int16_t idx)
{
  int16_t y = idx * DRAW_ITEM_HEIGHT + 1; // +1 for border
  int16_t textWidth = m_display->getStringWidth(value);
  int16_t x = m_display->getWidth() - DRAW_X_MARGIN - textWidth - 2;             // -1 for border, -1 for padding
  m_display->drawRect(x, y, textWidth + DRAW_X_MARGIN, DRAW_ITEM_HEIGHT - 1); // -1 for border
}
