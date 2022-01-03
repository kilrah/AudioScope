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

// GUItool: begin automatically generated code
AudioInputUSB            usb1;           //xy=377,576
AudioSynthWaveform       waveform1;      //xy=530,413
AudioSynthWaveformDc     dcX;            //xy=582,531
AudioSynthWaveformDc     dcY;            //xy=583,613
AudioAmplifier           ampL;           //xy=585,703
AudioAmplifier           ampR;           //xy=585,761
AudioMixer4              mixerX;         //xy=731,502
AudioMixer4              mixerY;         //xy=738,591
AudioOutputPT8211        pt821_1;        //xy=763,733
AudioOutputAnalogStereo  dacs1;          //xy=923,544
AudioMixer4              mixerRMS;       //xy=936,661
AudioFilterStateVariable filterRMS;      //xy=1109,651
AudioAnalyzeRMS          rms1;           //xy=1282,652
AudioConnection          patchCord1(usb1, 0, mixerX, 0);
AudioConnection          patchCord2(usb1, 0, ampL, 0);
AudioConnection          patchCord3(usb1, 1, mixerY, 0);
AudioConnection          patchCord4(usb1, 1, ampR, 0);
AudioConnection          patchCord5(waveform1, 0, mixerX, 2);
AudioConnection          patchCord6(dcX, 0, mixerX, 1);
AudioConnection          patchCord7(dcY, 0, mixerY, 1);
AudioConnection          patchCord8(ampL, 0, pt821_1, 0);
AudioConnection          patchCord9(ampR, 0, pt821_1, 1);
AudioConnection          patchCord10(mixerX, 0, dacs1, 0);
AudioConnection          patchCord11(mixerX, 0, mixerRMS, 0);
AudioConnection          patchCord12(mixerY, 0, dacs1, 1);
AudioConnection          patchCord13(mixerY, 0, mixerRMS, 1);
AudioConnection          patchCord14(mixerRMS, 0, filterRMS, 0);
AudioConnection          patchCord15(filterRMS, 2, rms1, 0);
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

int mode = 0;

void setup() {
  //Serial.begin(115200);
  AudioMemory(10);
  pinMode(BlankPin, OUTPUT);
  dacs1.analogReference(EXTERNAL);
  filterRMS.frequency(100);
  filterRMS.resonance(0.7);
  waveform1.begin(WAVEFORM_SAWTOOTH);
  waveform1.amplitude(0.8);
  waveform1.offset(0.1);
  rotenc.begin(encAPin, encBPin, encButPin);
  rotenc.setLimits(-20, 40);
  digitalWrite(BlankPin, HIGH);       // Enable beam
}

void loop() {
  rotenc.update();

  // Update controls every 10ms
  if(controls > 10) {
    controls = 0;

    // Mode toggle
    if(rotenc.isLongPressed()) {
      if(mode == 0)
        mode = 1;
      else
        mode = 0;
      while(rotenc.isPressed())
        rotenc.update();
    }

    if(mode == 0) {  // XY
      // Set scope gain from rotary encoder, press to reset to default of 1
      if(rotenc.isPressed())
        rotenc.setPosition(0);
      float gain = ((float)map(rotenc.getPosition(), -20, 10, 0, 150))/100;
      
      // Disable time waveform, ensure DC offset is used, set gain as per above for both channels
      mixerX.gain(0, gain); // Sound
      mixerX.gain(1, 1);    // DC
      mixerX.gain(2, 0);    // Sawtooth
      
      mixerY.gain(0, gain); // Sound
      mixerY.gain(1, 1);    // DC
      
      // Blanking reacts to both axes
      mixerRMS.gain(0, 1);
 
      // Set X/Y positions from pots
      dcX.amplitude((float)map(analogRead(XPosPin), 0, 1024, -100, 100)/100);
      dcY.amplitude((float)map(analogRead(YPosPin), 0, 1024, -100, 100)/100);
    }
    else { // YT
      // Disable audio input and DC offset for X axis, enable sawtooth waveform
      mixerX.gain(0, 0);  // Sound
      mixerX.gain(1, 0);  // DC
      mixerX.gain(2, 1);  // Sawtooth
      
      // Set audio gain from left pot and timebase from right pot, disable DC offset
      mixerY.gain(0, (float)map(analogRead(XPosPin), 0, 1024, 0, 300)/100);
      mixerY.gain(1, 0);
      waveform1.frequency(map(analogRead(YPosPin), 0, 1024, 5, 200));

      // Blanking reacts to sound only
      mixerRMS.gain(0, 0);
    }

    // Set audio out gain from PC volume
    ampL.gain(usb1.volume());
    ampR.gain(usb1.volume());

    // Blank beam if amplitude is low for 5 secs
    if(rms1.available()) {
      if(rms1.read() > 0.02)
        rmsBlank = 0;
    }
    if(rmsBlank > 5000)
      digitalWrite(BlankPin, LOW);
    else
      digitalWrite(BlankPin, HIGH);
  }
}
