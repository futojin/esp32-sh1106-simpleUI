#include "simpleUI.h"

StatusText::StatusText(const char *text)
    : Status(), m_text(text) {}

void StatusText::draw()
{
  m_display->setFont(ArialMT_Plain_10);
  m_display->setTextAlignment(TEXT_ALIGN_LEFT);
  m_display->drawString(posX, posY, m_text);
}