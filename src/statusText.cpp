#include "simpleUI.h"

StatusText::StatusText(const char *text, OLEDDISPLAY_TEXT_ALIGNMENT alignment, uint16_t minWidth)
    : Status(alignment, minWidth), m_text(text) {}

uint16_t StatusText::draw()
{
  m_display->setFont(ArialMT_Plain_10);
  m_display->setTextAlignment(m_alignment);
  m_display->drawString(m_posX, m_posY, m_text);
  return max(m_display->getStringWidth(m_text), m_minWidth);
}