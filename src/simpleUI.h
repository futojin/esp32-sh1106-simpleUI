#ifndef Futojin_SIMPLEUI_H
#define Futojin_SIMPLEUI_H

#include "SH1106Wire.h"
#include "internal.h"
#include "icon.h"
#include <vector>
#include <list>

#define DEBUG_SIMPLEUI(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#ifndef DEBUG_SIMPLEUI
#define DEBUG_SIMPLEUI(...)
#endif

// #define DEBUG_DEBOUNCE(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#ifndef DEBUG_DEBOUNCE
#define DEBUG_DEBOUNCE(...)
#endif

enum ROTARY_EVENT
{
  ROTARY_EVENT_CW,
  ROTARY_EVENT_CCW,
  ROTARY_EVENT_PUSH
};

enum Event_ID
{
  EVENT_EMPTY,
  EVENT_PIR,
  EVENT_TIM,
  EVENT_ROT,
  EVENT_YIELD // internal event, do not use.
};

struct Event
{
  Event_ID eventId;
  unsigned long value;
};

class Item
{
public:
  char *value;
  const char *m_label;

  Item(const char *label, void (*onValueChange)(Item *item, const Event *event));
  void (*onValueChange)(Item *item, const Event *event);
  bool isEnabled() const { return m_enabled; }
  void setEnabled(bool enabled) { m_enabled = enabled; }
  void setOffset(uint16_t offsetX, uint16_t offsetY)
  {
    m_offsetX = offsetX;
    m_offsetY = offsetY;
  }
  void onEvent(Event &event);
  virtual void draw(u_int16_t idx) = 0;
  virtual void drawHighlight(u_int16_t idx) = 0;
  virtual void drawValueHighlight(u_int16_t idx) = 0;
  void syncDisplay(SH1106Wire *display) { m_display = display; }

protected:
  SH1106Wire *m_display;
  bool m_enabled;
  uint16_t m_offsetX;
  uint16_t m_offsetY;
};

class ListPageItem : public Item
{
public:
  ListPageItem(const char *label, void (*onValueChange)(Item *item, const Event *event));

  void draw(u_int16_t idx) override;
  void drawHighlight(u_int16_t idx) override;
  void drawValueHighlight(u_int16_t idx) override;
};

class HeroPageItem : public Item
{
public:
  HeroPageItem(const char *label, void (*onValueChange)(Item *item, const Event *event));
  void draw(u_int16_t idx) override;
  void drawHighlight(u_int16_t idx) override;
  void drawValueHighlight(u_int16_t idx) override;
  void useSmallFont(bool smallFont) { m_smallFont = smallFont; }

private:
  bool m_smallFont;
};

class Navbar
{
public:
  Navbar(SH1106Wire &display, Container &container);
  void addPage(Page &page);
  void draw(const Page &currentPage);
  void onEvent(Event &event);

private:
  SH1106Wire *m_display;
  std::vector<Page *> m_pages;
  CONTEXT m_context;
  Container *m_container;
};

class Page
{
public:
  Page(const unsigned char *icon);
  void enable(bool enabled) { m_enabled = enabled; }
  bool enabled() const { return m_enabled; }
  void enableSaveActions(void (*onSave)(), void (*onExit)());
  void disableSaveActions();
  void setOffset(uint16_t offsetX, uint16_t offsetY);
  const unsigned char *getIcon() const { return m_icon; }
  void setContainer(Container &container) { m_container = &container; }
  void setDisplay(SH1106Wire &display) { m_display = &display; }
  void draw();
  void onEvent(Event &event);
  virtual void start() = 0;
  virtual void syncDisplay() = 0;
  virtual void reset() = 0;

protected:
  SH1106Wire *m_display;
  Container *m_container;
  const unsigned char *m_icon;
  CONTEXT m_context;
  bool m_enabled;
  bool m_enableSaveActions;
  uint16_t m_offsetX;
  uint16_t m_offsetY;

  virtual void drawItems() = 0;
  virtual void onPageEvent(Event &event) = 0;
  virtual void onItemEvent(Event &event) = 0;
  void drawSaveActions();
  void onSaveEvent(Event &event);
  void onExitEvent(Event &event);

  void (*onSave)();
  void (*onExit)();

  // Helper functions to call Item's methods from Page subclass without explicit reference to an Item subclass.
  void item_onEvent(Item &item, Event &event) { item.onEvent(event); }
  void item_syncDisplay(Item &item) { item.syncDisplay(m_display); }
  void item_draw(Item &item, u_int16_t idx);
  void item_drawHighlight(Item &item, u_int16_t idx) { item.drawHighlight(idx); }
  void item_drawValueHighlight(Item &item, u_int16_t idx) { item.drawValueHighlight(idx); }

private:
  void checkAndYield();
};

class HeroPage : public Page
{
public:
  HeroPage(const unsigned char *icon);
  void addItem(HeroPageItem &pageItem);

private:
  HeroPageItem *m_currentItem;

  void drawItems() override;
  void start() override;
  void syncDisplay() override;
  void reset() override;
  void onPageEvent(Event &event) override;
  void onItemEvent(Event &event) override;
};

class ListPage : public Page
{
public:
  ListPage(const unsigned char *icon) : Page(icon) {}
  void addItem(ListPageItem &pageItem);

private:
  std::list<ListPageItem *> m_pageItems;
  std::list<ListPageItem *>::iterator m_currentItemIt;

  bool nextItem();
  bool prevItem();
  void drawItems() override;
  void start() override;
  void onPageEvent(Event &event) override;
  void onItemEvent(Event &event) override;
  void syncDisplay() override;
  void reset() override;
};

class Status
{
public:
  Status(OLEDDISPLAY_TEXT_ALIGNMENT alignment = TEXT_ALIGN_LEFT, uint16_t minWidth = 0);

  void setDisplay(SH1106Wire &display) { m_display = &display; }
  void setPosition(uint16_t posX, uint16_t posY);
  OLEDDISPLAY_TEXT_ALIGNMENT getAlignment() { return m_alignment; }
  virtual uint16_t draw() = 0;

protected:
  uint16_t m_posX;
  uint16_t m_posY;
  uint16_t m_minWidth;
  SH1106Wire *m_display;
  OLEDDISPLAY_TEXT_ALIGNMENT m_alignment;
};

class StatusBar
{
public:
  StatusBar(SH1106Wire &display);

  void addStatus(Status &status);
  void draw();

private:
  SH1106Wire *m_display;
  std::vector<Status *> m_l_statuses;
  std::vector<Status *> m_r_statuses;
};

class StatusText : public Status
{
public:
  StatusText(const char *text, OLEDDISPLAY_TEXT_ALIGNMENT alignment = TEXT_ALIGN_LEFT, uint16_t minWidth = 0);

  void setText(const String &text) { m_text = text.c_str(); }
  void setText(const char *text) { m_text = text; }
  const char *getText() const { return m_text; }
  uint16_t draw() override;

private:
  const char *m_text;
};

class Container
{
  friend void onContainerRotaryEvent(ROTARY_EVENT rEvent);
  friend void onContainerSwitchEvent(u_int8_t pinState);

public:
  // Singleton
  static Container &getInstance(SH1106Wire &display);
  static Container &getInstance(SH1106Wire &display, u_int8_t tra, u_int8_t trb, u_int8_t psh);
  Container(const Container &) = delete;
  Container &operator=(const Container &) = delete;

  void initDisplay(bool flipVertical = true);
  void draw();
  void addPage(Page &childPage);
  void setCurrentPage(Page &newPage);
  void onEvent(Event &event);
  void enableScreenSaver(u_int8_t timeoutSec);
  void disableScreenSaver();
  void enableOverlay(bool enabled);
  void start();
  void flipDisplay(bool flipVertical);
  void addStatus(Status &status);
  bool isStatusBarEnabled() const { return m_statusBarEnabled; }
  void onEventYield(Event &event);

private:
  struct WatchdogTaskParams
  {
    Container *container;
  };

  SH1106Wire *m_display;
  Page *m_currentPage;
  Navbar m_navbar;
  StatusBar m_statusBar;
  CONTEXT m_context;
  std::vector<Page *> m_pages;
  u_int8_t m_idx;
  TaskHandle_t m_watchdogTaskHandle;
  u_int8_t m_screenSaverTimeoutSec;
  volatile unsigned long m_lastActivityMs;
  RotaryDebounce *m_rotaryDebounce;
  SwitchDebounce *m_switchDebounce;
  bool m_statusBarEnabled;
  bool m_screenSaverActive;
  bool m_overlayEnabled;

  static WatchdogTaskParams s_watchdogTaskParams;
  static Container *s_containerInstance;

  void drawOverlay();
  void trackCurrentPage(ROTARY_EVENT rEvent);

  void createWatchdogTask();
  static void onWatchdogTask(void *parameter);
  void screenSaverTask(WatchdogTaskParams *params);
  u_int8_t nextEnabledPage();
  u_int8_t previousEnabledPage();
  void updatePageOffset();

  Container(SH1106Wire &display);
  Container(SH1106Wire &display, u_int8_t tra, u_int8_t trb, u_int8_t psh);
  ~Container();
};

class RotaryDebounce
{
  friend void IRAM_ATTR rotary_isr(void *arg);
  friend void handleRotaryDebounceQueueTask(void *parameter);
  friend void handleRotaryCallbackTask(void *parameter);

public:
  /**
   * Note: Rotary encoder pins are assumed to be pulled up HIGH.
   */
  RotaryDebounce(const u_int8_t tra, const u_int8_t trb, void (*onRotaryEvent)(const ROTARY_EVENT event));
  ~RotaryDebounce();
  void start();

private:
  u_int8_t m_pinA;
  u_int8_t m_pinB;

  enum RotaryPhase
  {
    RESET,
    S1,
    S2,
    S3,
    S4
  };

  struct IsrTaskParams
  {
    unsigned long interruptMs;
    RotaryDebounce *debounceInstance;
  };

  struct CallbackTaskParams
  {
    ROTARY_EVENT event;
    RotaryDebounce *debounceInstance;
  };

  volatile struct RotaryState
  {
    ROTARY_EVENT direction;
    RotaryPhase phase;
    unsigned long startMs;
  } m_rotaryState;

  void (*onRotaryEvent)(const ROTARY_EVENT event);
  void abInterrupt(unsigned long interruptMs);
  void resetState();
  void cw(int pinAState, int pinBState);
  void ccw(int pinAState, int pinBState);
  void configureTask();
};

class SwitchDebounce
{
  friend void IRAM_ATTR switchDebounce_isr(void *arg);
  friend void handleSwitchDebounceIsrQueueTask(void *param);
  friend void handleSwitchDebounceCallbackTask(void *param);
  friend void switchDebounce_timerCallback(TimerHandle_t xTimer);

public:
  SwitchDebounce(u_int8_t pin, void (*onSwitchEvent)(const u_int8_t pinState));
  ~SwitchDebounce();
  void start();

private:
  u_int8_t m_pin;
  int m_lastPinState;
  TimerHandle_t m_debounceTimer;
  void (*onSwitchEvent)(const u_int8_t pinState);
};
#endif // Futojin_SIMPLEUI_H