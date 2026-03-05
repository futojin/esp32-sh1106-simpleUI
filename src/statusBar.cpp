#include "simpleUI.h"

#define SEPARATOR_WIDTH 3

StatusBar::StatusBar(SH1106Wire &display)
    : m_display(&display)
{
}

void StatusBar::addStatus(Status &status)
{
  status.setDisplay(*m_display);
  if (status.getAlignment() == TEXT_ALIGN_LEFT)
  {
    m_l_statuses.push_back(&status);
  }
  else
  {
    m_r_statuses.push_back(&status);
  }
}

void StatusBar::draw()
{
  m_display->drawLine(0, ArialMT_Plain_10[1], m_display->getWidth(), ArialMT_Plain_10[1]);

  uint16_t offsetX = 0;
  for (Status *status : m_l_statuses)
  {
    status->setPosition(offsetX, 0);
    offsetX += status->draw() + SEPARATOR_WIDTH;
  }

  offsetX = m_display->getWidth();
  for (Status *status : m_r_statuses)
  {
    status->setPosition(offsetX, 0);
    offsetX -= status->draw() + SEPARATOR_WIDTH;
  }
}
