#include <Arduino.h>
#include "simpleUI.h"

// #define DEBUG_(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#ifndef DEBUG_
#define DEBUG_(...)
#endif

// Rotary Encoder
#define ROT_INPUT_PSH GPIO_NUM_0 // Rotary Push button
#define ROT_INPUT_A GPIO_NUM_20  // Rotary Encoder A
#define ROT_INPUT_B GPIO_NUM_21  // Rotary Encoder B

// I2C Pins
#define I2C_SDA_PIN GPIO_NUM_8
#define I2C_SCL_PIN GPIO_NUM_10

struct DisplayData
{
  uint8_t current_duty = 0; // LED brightness duty cycle (0-255)
  bool flipDisplay = false; // Whether the display is flipped vertically
} displayData;

// ##############################
// Menus intialization and event handling
// ##############################
SH1106Wire display(0x3C, I2C_SDA_PIN, I2C_SCL_PIN);
Container &container = Container::getInstance(display, ROT_INPUT_A, ROT_INPUT_B, ROT_INPUT_PSH);

void onHandleBrightnessChange(Item *item, const Event *event)
{
  static char brightnessValue[4]; // 4 characters long max
  DEBUG_("onHandleBrightnessChange: got event: %d, %d\n", event->eventId, event->value);
  DEBUG_("onHandleBrightnessChange: current duty: %d\n", displayData.current_duty);
  if (event->eventId == EVENT_ROT)
  {
    Event msg = *event;
    {
      if (msg.value == ROTARY_EVENT_CW)
      {
        // Clockwise: increase brightness
        if (displayData.current_duty < 255)
        {
          displayData.current_duty += 1;
        }
      }
      else if (msg.value == ROTARY_EVENT_CCW)
      {
        // Counter-clockwise: decrease brightness
        if (displayData.current_duty > 0)
        {
          displayData.current_duty -= 1;
        }
      }
    }
  }
  DEBUG_("onHandleBrightnessChange: brightness changed to: %d\n", displayData.current_duty);
  sprintf(brightnessValue, "%d", displayData.current_duty); // Convert to string to update display
  item->value = brightnessValue;
}

HeroPageItem brightnessHeroItem("Brightness", onHandleBrightnessChange);
PageItem brightnessItem("LED Brightness", onHandleBrightnessChange);
PageItem flipDisplayItem("Flip Display", [](Item *item, const Event *event)
                         {
                           static char flipValue[4];
                           if (event->eventId == EVENT_ROT)
                           {
                             if (event->value == ROTARY_EVENT_CW || event->value == ROTARY_EVENT_CCW)
                             {
                               displayData.flipDisplay = !displayData.flipDisplay; // Toggle flip state
                               container.flipDisplay(displayData.flipDisplay);     // Apply
                             }
                           }
                           DEBUG_("Flip Display toggled to: %s\n", displayData.flipDisplay ? "ON" : "OFF");
                           sprintf(flipValue, "%s", displayData.flipDisplay ? "ON" : "OFF"); // Convert to string to update display
                           item->value = flipValue;                                          // Update display value
                         });
PageItem dummyItem("Test 1", onHandleBrightnessChange);
PageItem dummyItem2("Test 2", onHandleBrightnessChange);

HeroPage mainPage(icon_bulb);
ListPage settingsPage(icon_settings);

StatusText text1("00:00");
StatusText text2("World");

void setup()
{
  // Container does not define pinMode directly.
  pinMode(ROT_INPUT_A, INPUT);
  pinMode(ROT_INPUT_B, INPUT);
  pinMode(ROT_INPUT_PSH, INPUT);

  Serial.begin(115200);
  container.initDisplay(); // Getting ready and to allow an early visual cue we are starting up...

  container.addPage(mainPage);
  container.addPage(settingsPage);

  mainPage.addItem(brightnessHeroItem);
  settingsPage.addItem(brightnessItem);
  settingsPage.addItem(flipDisplayItem);
  settingsPage.addItem(dummyItem);
  settingsPage.addItem(dummyItem2);

  container.addStatus(text1);
  container.addStatus(text2);

  container.enableScreenSaver(60);
  container.enableOverlay(false);

  container.setCurrentPage(mainPage);
  container.start();
}

void loop()
{
  static unsigned long lastUpdate = 0;
  static char timeStr[6]; // "mm:ss\0"

  unsigned long now = millis();
  if (now - lastUpdate >= 1000 && lastUpdate != now)
  {
    lastUpdate = now;
    unsigned long totalSeconds = now / 1000;
    unsigned long minutes = (totalSeconds / 60) % 60;
    unsigned long seconds = totalSeconds % 60;
    sprintf(timeStr, "%02lu:%02lu", minutes, seconds);
    text1.setText(timeStr);
    container.draw();
  }
}
