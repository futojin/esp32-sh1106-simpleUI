#ifndef Futojin_SIMPLEUI_H
#define Futojin_SIMPLEUI_H

#include "SH1106Wire.h"
#include "internal.h"
#include "icon.h"
#include <vector>
#include <list>

// #define DEBUG_SIMPLEUI(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#ifndef DEBUG_SIMPLEUI
#define DEBUG_SIMPLEUI(...)
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
  friend class Page;

public:
  char *value;
  const char *m_label;

  Item(const char *label, void (*onValueChange)(Item *item, const Event *event));
  void (*onValueChange)(Item *item, const Event *event);
  bool isEnabled() const { return m_enabled; }
  void setEnabled(bool enabled) { m_enabled = enabled; }

protected:
  SH1106Wire *m_display;
  bool m_enabled;

  void onEvent(Event &event);
  virtual void draw(u_int16_t idx) = 0;
  virtual void drawHighlight(u_int16_t idx) = 0;
  virtual void drawValueHighlight(u_int16_t idx) = 0;
  void syncDisplay(SH1106Wire *display) { m_display = display; }
};

class PageItem : public Item
{
public:
  PageItem(const char *label, void (*onValueChange)(Item *item, const Event *event));

private:
  void draw(u_int16_t idx) override;
  void drawHighlight(u_int16_t idx) override;
  void drawValueHighlight(u_int16_t idx) override;
};

class HeroPageItem : public Item
{
public:
  HeroPageItem(const char *label, void (*onValueChange)(Item *item, const Event *event));

private:
  void draw(u_int16_t idx) override;
  void drawHighlight(u_int16_t idx) override;
  void drawValueHighlight(u_int16_t idx) override;
};

class Navbar
{
  friend class Container;

public:
private:
  SH1106Wire *m_display;
  std::vector<Page *> m_pages;
  CONTEXT m_context;
  Container *m_container;

  Navbar(SH1106Wire &display, Container &container);
  void addPage(Page &page);
  void draw(const Page &currentPage);
  void onEvent(Event &event);
};

class Page
{
  friend class Container;

public:
  Page(const unsigned char *icon);
  void enable(bool enabled) { m_enabled = enabled; }
  bool enabled() const { return m_enabled; }
  void enableSaveActions(void (*onSave)(), void (*onExit)());
  void disableSaveActions();

  const unsigned char *getIcon() const { return m_icon; }

protected:
  SH1106Wire *m_display;
  Container *m_container;
  const unsigned char *m_icon;
  CONTEXT m_context;
  bool m_enabled;
  bool m_enableSaveActions;

  virtual void drawItems() = 0;
  virtual void syncDisplay() = 0;
  virtual void start() = 0;
  virtual void reset() = 0;
  virtual void onPageEvent(Event &event) = 0;
  virtual void onItemEvent(Event &event) = 0;

  void drawSaveActions();
  void draw();
  void onEvent(Event &event);
  void onSaveEvent(Event &event);
  void onExitEvent(Event &event);

  void (*onSave)();
  void (*onExit)();

  // Helper functions to call Item's non-public methods from Page subclass without declaring them as friend.
  void item_onEvent(Item &item, Event &event) { item.onEvent(event); }
  void item_syncDisplay(Item &item) { item.syncDisplay(m_display); }
  void item_draw(Item &item, u_int16_t idx) { item.draw(idx); }
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
  void addItem(PageItem &pageItem);

private:
  std::list<PageItem *> m_pageItems;
  std::list<PageItem *>::iterator m_currentItemIt;

  bool nextItem();
  bool prevItem();
  void drawItems() override;
  void start() override;
  void onPageEvent(Event &event) override;
  void onItemEvent(Event &event) override;
  void syncDisplay() override;
  void reset() override;
};

class Container
{
  friend class Navbar;
  friend class Page;
  friend void onContainerRotaryEvent(ROTARY_EVENT rEvent);
  friend void onContainerSwitchEvent(u_int8_t pinState);

public:
  // Singleton
  static Container &getInstance(SH1106Wire &display);
  static Container &getInstance(SH1106Wire &display, u_int8_t tra, u_int8_t trb, u_int8_t psh);
  Container(const Container &) = delete;
  Container &operator=(const Container &) = delete;

  void initDisplay(bool flipVertical = true);
  void addPage(Page &childPage);
  void setCurrentPage(Page &newPage);
  void onEvent(Event &event);
  void enableScreenSaver(u_int8_t timeoutSec);
  void disableScreenSaver();
  void start();
  void flipDisplay(bool flipVertical);

private:
  struct WatchdogTaskParams
  {
    Container *container;
  };

  SH1106Wire *m_display;
  Page *m_currentPage;
  Navbar m_navbar;
  CONTEXT m_context;
  std::vector<Page *> m_pages;
  u_int8_t m_idx;
  TaskHandle_t m_watchdogTaskHandle;
  u_int8_t m_screenBrightness;
  u_int8_t m_screenSaverTimeoutSec;
  volatile unsigned long m_lastActivityMs;
  RotaryDebounce *m_rotaryDebounce;
  SwitchDebounce *m_switchDebounce;

  static WatchdogTaskParams s_watchdogTaskParams;
  static Container *s_containerInstance;

  void drawOverlay();
  void draw();
  void trackCurrentPage(ROTARY_EVENT rEvent);
  void onEventYield(Event &event);
  void createWatchdogTask();
  static void onWatchdogTask(void *parameter);
  void screenSaverTask(WatchdogTaskParams *params);
  u_int8_t nextEnabledPage();
  u_int8_t previousEnabledPage();

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
  RotaryDebounce(const u_int8_t tra, const u_int8_t trb, void (*onRotaryEvent)(ROTARY_EVENT event));
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

  void (*onRotaryEvent)(ROTARY_EVENT event);
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
  SwitchDebounce(u_int8_t pin, void (*onSwitchEvent)(u_int8_t pinState));
  ~SwitchDebounce();
  void start();

private:
  u_int8_t m_pin;
  int m_lastPinState;
  TimerHandle_t m_debounceTimer;
  void (*onSwitchEvent)(u_int8_t pinState);
};
#endif // Futojin_SIMPLEUI_H