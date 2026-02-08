# ESP32 SH1116 OLED Module with EC11 Rotary Encoder Simple UI

This is a simple user interface library targeted for SH1106 OLED displays with EC11 rotary encoders, commonly sold in AliExpress listings.

It often can be bought USD$3-5 each (2026). Especially on bundle deals.

![alt text](docs/module-front.jpg) ![alt text](docs/module-iso.jpg)

## Features
* Modular menu system. You deal with high level concepts like "Pages" and "Items" rather than line and pixel coordinates.
* Interrupt based rotary encoder and button handling. Keep your loop() free for other tasks.
* Internally context and state aware. You don't need to track where you are in the menu system. Provide a callback to get notified of your own event.
* Easy to extend with your custom pages and items.
* Uses only the encoder push button to navigate through the menu system.
* Advanced debouncing algorithm for rotary encoders and buttons.
* Built-in screen timeout. Prolong those cheap OLED displays life.

![alt text](docs/menu-hero.jpeg)
![alt text](docs/menu-list.jpeg)

## Usage

## Extensions

## Credits
Built on top of the popular [SH1106Wire](https://github.com/ThingPulse/esp8266-oled-ssd1306/blob/master/README.md) library.

