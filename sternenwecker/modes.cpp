#include <Arduino.h>
#include "resources.h"
#include "clock.h"
#include "modes.h"

Mode::Mode() {
}

// --------- MOff ----------
MOff m_off = MOff();
void MOff::enter() {
  matrix.clear();
  matrix.show();
}

Mode* MOff::press() {
  return &m_menu;
}

// --------- MMenu ----------
MMenu m_menu = MMenu();

static const uint8_t PROGMEM 
  MENU_ICONS[MENU_COUNT][8] = {
    // { B01111111,
    //   B01111111,
    //   B00001111,
    //   B00011111,
    //   B00111011,
    //   B01110011,
    //   B11100011,
    //   B11000000 },
    { B00000000,
      B00000000,
      B00011100,
      B00001100,
      B00010100,
      B00100000,
      B00000000,
      B00000000 },
    { B00000000,
      B00000000,
      B00100000,
      B00010100,
      B00001100,
      B00011100,
      B00000000,
      B00000000 },
    { B00000000,
      B00000000,
      B00000000,
      B00111100,
      B00000000,
      B00111100,
      B00000000,
      B00000000 },
    { B00000000,
      B00011000,
      B00011000,
      B00011000,
      B00000000,
      B00011000,
      B00000000,
      B00000000 },
    };

void MMenu::update() {
  matrix.drawBitmap(0, 0, MENU_ICONS[current], 8, 8, MENU_COLOR);
  matrix.show();
}

void MMenu::enter() {
  current = 0;
  update();
}

Mode* MMenu::press() {
  switch (current) {
    case 0: return &m_menu;
    case 1: return &m_menu;
    case 2: return &m_torch;
    case 3: return &m_set;
    default: return &m_off;
  }
  // TODO
  return &m_menu;
}

Mode* MMenu::longpress() { return &m_off; }

Mode* MMenu::left_turn() {
  current = (current + MENU_COUNT - 1) % MENU_COUNT;
  update();
  return NULL;
}
Mode* MMenu::right_turn() {
  current = (current + 1) % MENU_COUNT;
  update();
  return NULL;
}

// --------- MTorch ----------
MTorch m_torch = MTorch();
void MTorch::update() {
  uint32_t color = matrix.hue_to_color((hue * 360.0)/TORCH_HUE_STEPS);
  matrix.setBrightness(BRIGHTNESS / (float)TORCH_BRIGHTNESS_STEPS * brightness);
  matrix.fillScreen(color);
  matrix.show();
}

void MTorch::enter() {
  hue = TORCH_HUE_START;
  brightness = TORCH_BRIGHTNESS_START;
  update();
}
void MTorch::leave() {
  // clean up and set to default brightness
  matrix.setBrightness(BRIGHTNESS);
}

Mode* MTorch::press() { return &m_off; }
Mode* MTorch::longpress() { return NULL; }
Mode* MTorch::button_hold() {
  hue = (hue + 1) % TORCH_HUE_STEPS;
  update();
  return NULL;
}

Mode* MTorch::left_turn() {
  if (brightness > 0) {
    brightness--;
  }
  update();
  return NULL;
}
Mode* MTorch::right_turn() {
  if (brightness < TORCH_BRIGHTNESS_STEPS) {
    brightness++;
  }
  update();
  return NULL;
}

// --------- MSet ----------
MSet m_set = MSet();
void MSet::update() {
  matrix.clear();
  uint8_t num_to_show = (state == SET_STATE_MINUTE) ? get_current_minute() : get_current_hour();
  uint8_t num_x = (state == SET_STATE_MINUTE) ? 2 : 0;
  uint8_t dots_x = (state == SET_STATE_MINUTE) ? 0 : 7;
  matrix.draw3x5Digit(num_to_show / 10, num_x,2, SET_COLOR);
  matrix.draw3x5Digit(num_to_show % 10, num_x+3,2, SET_COLOR);
  if (get_current_second() % 2 == 0) {
    matrix.drawPixel(dots_x, 3, SET_COLOR);
    matrix.drawPixel(dots_x, 5, SET_COLOR);
  }
  matrix.show();
}

void MSet::enter() {
  state = SET_STATE_HOUR;
  update();
}

Mode* MSet::loop() {
  update();
  return NULL;
}

Mode* MSet::press() {
  switch (state) {
  case SET_STATE_HOUR:
    state = SET_STATE_MINUTE;
    update();
    return NULL;
  case SET_STATE_MINUTE:
    return &m_off;
  }
}
Mode* MSet::longpress() { return &m_menu; }
Mode* MSet::left_turn() {
  if (state == SET_STATE_HOUR)
    substract_hour_from_offset();
  if (state == SET_STATE_MINUTE)
    substract_minute_from_offset();

  update();
  return NULL;
}
Mode* MSet::right_turn() {
  if (state == SET_STATE_HOUR)
    add_hour_to_offset();
  if (state == SET_STATE_MINUTE)
    add_minute_to_offset();

  update();
  return NULL;
}