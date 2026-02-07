#ifndef Futojin_SIMPLEUI_H
#define Futojin_SIMPLEUI_H

#include "SH1106Wire.h"
#include "icon.h"
#include <vector>
#include <list>

// #define DEBUG_SIMPLEUI(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#ifndef DEBUG_SIMPLEUI
#define DEBUG_SIMPLEUI(...)
#endif

enum CONTEXT
{
  NONE,
  NAVBAR,
  PAGE,
  ITEM,
  SAVE,
  EXIT
};

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
  EVENT_YIELD // TODO: internal event
};

class Page;
class Container;

struct Event
{
  Event_ID eventId;
  unsigned long value;
};

class Item
{
  friend class Page;
  friend class HeroPage;
  friend class ListPage;

protected:
  SH1106Wire *m_display;
  bool m_enabled;

  void onEvent(Event &event);
  virtual void draw(u_int16_t idx) = 0;
  virtual void drawHighlight(u_int16_t idx) = 0;
  virtual void drawValueHighlight(u_int16_t idx) = 0;

public:
  char *value;
  const char *m_label;

  Item(const char *label, void (*onValueChange)(Item *item, const Event *event));
  void (*onValueChange)(Item *item, const Event *event);
  bool isEnabled() const { return m_enabled; }
  void setEnabled(bool enabled) { m_enabled = enabled; }
};

class PageItem : public Item
{
private:
  void draw(u_int16_t idx) override;
  void drawHighlight(u_int16_t idx) override;
  void drawValueHighlight(u_int16_t idx) override;

public:
  PageItem(const char *label, void (*onValueChange)(Item *item, const Event *event));
};

class HeroPageItem : public Item
{
  friend class HeroPage;

private:
  void draw(u_int16_t idx) override;
  void drawHighlight(u_int16_t idx) override;
  void drawValueHighlight(u_int16_t idx) override;

public:
  HeroPageItem(const char *label, void (*onValueChange)(Item *item, const Event *event));
};

class Navbar
{
  friend class Container;

private:
  SH1106Wire *m_display;
  std::vector<Page *> m_pages;
  CONTEXT m_context;
  Container *m_container;

  Navbar(SH1106Wire &display, Container &container);
  void addPage(Page &page);
  void draw(const Page &currentPage);
  void onEvent(Event &event);

public:
};

class Page
{
  friend class Container;

private:
  void checkAndYield();

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

public:
  Page(const unsigned char *icon);
  void enable(bool enabled) { m_enabled = enabled; }
  bool enabled() const { return m_enabled; }
  void enableSaveActions(void (*onSave)(), void (*onExit)());
  void disableSaveActions();

  const unsigned char *getIcon() const { return m_icon; }
};

class HeroPage : public Page
{
private:
  HeroPageItem *m_currentItem;

  void drawItems() override;
  void start() override;
  void syncDisplay() override;
  void reset() override;
  void onPageEvent(Event &event) override;
  void onItemEvent(Event &event) override;

public:
  HeroPage(const unsigned char *icon);
  void addItem(HeroPageItem &pageItem);
};

class ListPage : public Page
{
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

public:
  ListPage(const unsigned char *icon) : Page(icon) {}
  void addItem(PageItem &pageItem);
};

class RotaryDebounce;
class SwitchDebounce;

class Container
{
  friend class Navbar;
  friend class Page;
  friend class ListPage;
  friend class HeroPage;
  friend void onContainerRotaryEvent(ROTARY_EVENT rEvent);
  friend void onContainerSwitchEvent(u_int8_t pinState);

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

public:
  // Singleton factory methods
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