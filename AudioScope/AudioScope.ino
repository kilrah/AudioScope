// https://github.com/kilrah/AudioScope
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

// Audio connections from the Audio System Design Tool for Teensy Audio Library (https://www.pjrc.com/teensy/gui/index.html)
// GUItool: begin automatically generated code
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include "driver_encoder.h"

AudioInputUSB            usb1; //xy=369,421
AudioSynthWaveformDc     dcX;            //xy=574,376
AudioSynthWaveformDc     dcY;            //xy=575,458
AudioAmplifier           ampL;           //xy=577,548
AudioAmplifier           ampR;           //xy=577,606
AudioMixer4              mixerX;         //xy=723,347
AudioMixer4              mixerY;         //xy=730,436
AudioOutputPT8211        pt821_1; //xy=755,578
AudioOutputAnalogStereo  dacs1; //xy=915,389
AudioMixer4              mixerRMS;         //xy=928,506
AudioFilterStateVariable filterRMS;        //xy=1101,496
AudioAnalyzeRMS          rms1;           //xy=1274,497
AudioConnection          patchCord1(usb1, 0, mixerX, 0);
AudioConnection          patchCord2(usb1, 0, ampL, 0);
AudioConnection          patchCord3(usb1, 1, mixerY, 0);
AudioConnection          patchCord4(usb1, 1, ampR, 0);
AudioConnection          patchCord5(dcX, 0, mixerX, 1);
AudioConnection          patchCord6(dcY, 0, mixerY, 1);
AudioConnection          patchCord7(ampL, 0, pt821_1, 0);
AudioConnection          patchCord8(ampR, 0, pt821_1, 1);
AudioConnection          patchCord9(mixerX, 0, dacs1, 0);
AudioConnection          patchCord10(mixerX, 0, mixerRMS, 0);
AudioConnection          patchCord11(mixerY, 0, dacs1, 1);
AudioConnection          patchCord12(mixerY, 0, mixerRMS, 1);
AudioConnection          patchCord13(mixerRMS, 0, filterRMS, 0);
AudioConnection          patchCord14(filterRMS, 2, rms1, 0);
// GUItool: end automatically generated code

// SCTVClock pins
const int BlankPin   =  2;   // Low blanks the display
const int encButPin  = 14;   // encoder button, 0 = pressed
const int encBPin    = 15;   // encoder quadrature
const int encAPin    = 16; 
const int XPosPin    = A15;  // horizontal centering knob
#ifdef SCTVA
  const int YPosPin  = A18;  // vertical centering knob on SCTVA is in odd place
#else
  const int YPosPin  = A16;  // vertical centering knob
#endif

// Rotary encoder
Encoder rotenc;

elapsedMillis controls = 0;
elapsedMillis rmsBlank = 0;

void setup() {
  //Serial.begin(115200);
  AudioMemory(10);
  pinMode(BlankPin, OUTPUT);
  dacs1.analogReference(EXTERNAL);
  mixerRMS.gain(0, 1);
  mixerRMS.gain(1, 1);
  filterRMS.frequency(100);
  filterRMS.resonance(0.7);
  rotenc.begin(encAPin, encBPin, encButPin);
  rotenc.setLimits(-20, 40);
  digitalWrite(BlankPin, HIGH);       // Enable beam
}

void loop() {
  rotenc.update();

  // Update controls every 10ms
  if(controls > 10) {
    float gain;
    controls = 0;

    gain = ((float)map(rotenc.getPosition(), -20, 10, 0, 150))/100;
    if(rotenc.isPressed())
      rotenc.setPosition(0);

    // Set scope gain from rotary encoder, press to reset to default of 1
    mixerX.gain(0, gain);
    mixerY.gain(0, gain);

    // Set audio out gain from PC volume
    ampL.gain(usb1.volume());
    ampR.gain(usb1.volume());

    // Set X/Y positions from pots
    dcX.amplitude((float)map(analogRead(XPosPin), 0, 1024, -100, 100)/100);
    dcY.amplitude((float)map(analogRead(YPosPin), 0, 1024, -100, 100)/100);

    // Blank beam if amplitude is low for 5 secs
    if(rms1.available()) {
      if(rms1.read() > 0.05)
        rmsBlank = 0;
    }
    if(rmsBlank > 5000)
      digitalWrite(BlankPin, LOW);
    else
      digitalWrite(BlankPin, HIGH);
  }
}
