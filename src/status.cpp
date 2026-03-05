#include "simpleUI.h"

Status::Status(OLEDDISPLAY_TEXT_ALIGNMENT alignment, uint16_t minWidth)
    : m_display(nullptr), m_posX(0), m_posY(0), m_alignment(alignment), m_minWidth(minWidth) {}

void Status::setPosition(uint16_t posX, uint16_t posY)
{
  this->m_posX = posX;
  this->m_posY = posY;
}