#include "simpleUI.h"

Status::Status()
    : m_display(nullptr), posX(0), posY(0) {}

void Status::setPosition(uint16_t posX, uint16_t posY)
{
  this->posX = posX;
  this->posY = posY;
}