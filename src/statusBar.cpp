#include "simpleUI.h"

StatusBar::StatusBar(SH1106Wire &display)
    : m_display(&display)
{
}

void StatusBar::addStatus(Status &status)
{
  status.setDisplay(*m_display);
  m_statuses.push_back(&status);

  uint32_t count = m_statuses.size();
  uint32_t slotWidth = m_display->getWidth() / count;

  for (uint32_t i = 0; i < count; i++)
  {
    m_statuses[i]->setPosition(i * slotWidth, 0);
  }
}

void StatusBar::draw()
{
  for (Status *status : m_statuses)
  {
    status->draw();
  }
}
