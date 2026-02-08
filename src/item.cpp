#include "simpleUI.h"

Item::Item(const char *label, void (*valueChangeResponder)(Item *item, const Event *event))
    : m_label(label),
      m_display(nullptr),
      onValueChange(valueChangeResponder),
      m_enabled(true)
{
}

void Item::onEvent(Event &event)
{
  // We can't be picky on event types here, just forward events to responder.
  if (onValueChange)
  {
    DEBUG_SIMPLEUI("Item::onEvent: Calling onValueChange: %d %lu\n", event.eventId, event.value);
    onValueChange(this, &event);
  }
}
