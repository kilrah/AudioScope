// --------------- Scope clock rotary encoder driver ---------------
// --- Based on code from https://github.com/nixiebunny/SCTVcode ---

// Part of https://github.com/kilrah/AudioScope
// Audio X/Y visualizer code for the Scopeclock from Cathode Corner
// https://scopeclock.com/

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#include <Arduino.h>
#include <Bounce2.h>

const uint32_t butBounceTime  = 10;   //ms
const uint32_t encBounceTime  = 1;    //ms
const uint32_t longPressDeflt = 1000; //ms

// Encoder quadrature lookup table
const int incr = 1;  // encoder turned clockwise
const int decr = -1;  // encoder turned counter-clockwise
const int none = 0; // encoder didn't move
const int impos = 0;  // not possible - glitch detected

// PEC11R encoder: Two detents per cycle, only change on bit 0 moving
const int EncTab[] = {none,decr,none,impos, // 00
                     incr,none,impos,none,  // 01
                     none,impos,none,incr,  // 10
                     impos,none,decr,none}; // 11

class Encoder {
  public:
    Encoder() {};
    void begin(const int pinA, const int pinB, const int pinPress);
    void update();    // Call this in main loop

    int  getPosition()                     { return _position; }
    void setPosition(int pos)              { _position = pos; }
    bool isPressed()                       { return _button.isPressed(); }
    bool isLongPressed()                   { return _longPress > _longPressDelay; }
    void setLimits(int min, int max)       { if(min < max) { _min = min; _max = max; }}
    void setLongPressDelay(uint32_t delay) { _longPressDelay = delay; };

  private:
    Button        _button;
    elapsedMillis _longPress;
    uint32_t      _longPressDelay = longPressDeflt;
    Button        _pinA;
    Button        _pinB;
    int           _min = -INT_MAX;
    int           _max = INT_MAX;
    int           _position;
    int           _lastPosition;
};

void Encoder::begin(const int pinA, const int pinB, const int pinPress)
{ 
  // Init pushbutton
  _button.interval(butBounceTime);
  _button.attach(pinPress, INPUT_PULLUP);
  _button.setPressedState(LOW);

  _position = 0;
  _longPress = 0;

  // Init encoder
  _pinA.attach(pinA, INPUT_PULLUP);
  _pinB.attach(pinB, INPUT_PULLUP);
  _pinA.interval(encBounceTime);
  _pinB.interval(encBounceTime);
  _pinA.setPressedState(HIGH);
  _pinB.setPressedState(HIGH);

  _lastPosition = _pinB.isPressed() << 1 | _pinA.isPressed();
  _lastPosition *= 5; // copy of old and new next to each other
}

void Encoder::update()
{
  // Update all pins
  _button.update();
  _pinA.update();
  _pinB.update();

  // Update long press handler
  if(!_button.isPressed())
    _longPress = 0;

  // Update encoder
  _lastPosition = _lastPosition >> 2 | _pinB.isPressed() << 3 | _pinA.isPressed() << 2;
  _position = _position + EncTab[_lastPosition];   // bump encoder value by motion via lookup table

  // Apply limits
  _position = constrain(_position, _min, _max);
}
