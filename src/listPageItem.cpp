#include "simpleUI.h"

#define DRAW_X_MARGIN 1
#define DRAW_ITEM_HEIGHT 13

ListPageItem::ListPageItem(const char *label, void (*valueChangeResponder)(Item *item, const Event *event))
    : Item(label, valueChangeResponder)
{
}

void ListPageItem::draw(u_int16_t idx)
{
  int16_t y = m_offsetY + idx * DRAW_ITEM_HEIGHT;

  m_display->setFont(ArialMT_Plain_10);
  m_display->setTextAlignment(TEXT_ALIGN_LEFT);
  m_display->drawString(DRAW_X_MARGIN + m_offsetX, y, m_label);

  m_display->setTextAlignment(TEXT_ALIGN_RIGHT);
  m_display->drawString(m_display->getWidth() - DRAW_X_MARGIN, y, value);
}

void ListPageItem::drawHighlight(u_int16_t idx)
{
  int16_t y = m_offsetY + idx * DRAW_ITEM_HEIGHT;
  int16_t x = m_offsetX;
  m_display->drawRect(x, y, m_display->getWidth() - m_offsetX, DRAW_ITEM_HEIGHT);
}

void ListPageItem::drawValueHighlight(u_int16_t idx)
{
  int16_t y = m_offsetY + idx * DRAW_ITEM_HEIGHT;
  int16_t textWidth = m_display->getStringWidth(value);
  int16_t x = m_display->getWidth() - DRAW_X_MARGIN - textWidth - 2;
  m_display->drawRect(x, y, m_display->getWidth() - x, DRAW_ITEM_HEIGHT);
}
