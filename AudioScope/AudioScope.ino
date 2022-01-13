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

const int MAX_MODES = 3;

// GUItool: begin automatically generated code
AudioInputUSB            usb1;           //xy=343,541
AudioAnalyzeFFT256       fft256_R;       //xy=403,758
AudioAnalyzeFFT256       fft256L;       //xy=432,333
AudioSynthWaveform       waveform1;      //xy=543,402
AudioSynthWaveformDc     dcX;            //xy=548,496
AudioSynthWaveformDc     dcY;            //xy=549,578
AudioAmplifier           ampL;           //xy=551,668
AudioAmplifier           ampR;           //xy=551,726
AudioPlayQueue           queueFFT;         //xy=562,622
AudioMixer4              mixerX;         //xy=697,467
AudioMixer4              mixerY;         //xy=704,556
AudioOutputPT8211        pt821_1;        //xy=729,698
AudioOutputAnalogStereo  dacs1;          //xy=889,509
AudioMixer4              mixerRMS;       //xy=902,626
AudioFilterStateVariable filterRMS;      //xy=1075,616
AudioAnalyzeRMS          rms1;           //xy=1248,617
AudioConnection          patchCord1(usb1, 0, mixerX, 0);
AudioConnection          patchCord2(usb1, 0, ampL, 0);
AudioConnection          patchCord3(usb1, 0, fft256L, 0);
AudioConnection          patchCord4(usb1, 1, mixerY, 0);
AudioConnection          patchCord5(usb1, 1, ampR, 0);
AudioConnection          patchCord6(usb1, 1, fft256_R, 0);
AudioConnection          patchCord7(waveform1, 0, mixerX, 2);
AudioConnection          patchCord8(dcX, 0, mixerX, 1);
AudioConnection          patchCord9(dcY, 0, mixerY, 1);
AudioConnection          patchCord10(ampL, 0, pt821_1, 0);
AudioConnection          patchCord11(ampR, 0, pt821_1, 1);
AudioConnection          patchCord12(queueFFT, 0, mixerY, 3);
AudioConnection          patchCord13(mixerX, 0, dacs1, 0);
AudioConnection          patchCord14(mixerX, 0, mixerRMS, 0);
AudioConnection          patchCord15(mixerY, 0, dacs1, 1);
AudioConnection          patchCord16(mixerY, 0, mixerRMS, 1);
AudioConnection          patchCord17(mixerRMS, 0, filterRMS, 0);
AudioConnection          patchCord18(filterRMS, 2, rms1, 0);
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
int16_t *queueYBuffer = NULL;

void setup() {
  Serial.begin(115200);
  AudioMemory(20);
  pinMode(BlankPin, OUTPUT);
  dacs1.analogReference(EXTERNAL);
  filterRMS.frequency(100);
  filterRMS.resonance(0.7);
  waveform1.begin(WAVEFORM_SAWTOOTH);
  waveform1.amplitude(0.8);
  waveform1.offset(0.1);
  rotenc.begin(encAPin, encBPin, encButPin);
  rotenc.setLimits(-20, 40);
  fft256L.averageTogether(4);
  queueYBuffer = queueFFT.getBuffer();
  for(int i = 0; i<128; i++)
    queueYBuffer[i] = 0;  
  digitalWrite(BlankPin, HIGH);       // Enable beam
  //mode = 2;
}

void loop() {
  rotenc.update();

  // Update controls every 10ms
  if(controls > 10) {
    controls = 0;

    // Mode toggle
    if(rotenc.isLongPressed()) {
      ++mode %= MAX_MODES;
      Serial.println(mode);
      if(mode == 0) {
        rotenc.setLimits(-50, 50);
      }
      else {
        rotenc.setLimits(-20, 40);
      }
      while(rotenc.isPressed())
        rotenc.update();
      rotenc.setPosition(0);
    }

    // Pressing encoder resets position
    if(rotenc.isPressed())
      rotenc.setPosition(0);

    if(mode == 0) {  // XY
      // Set scope gain from rotary encoder
      float gain = ((float)map(rotenc.getPosition(), -20, 10, 0, 150))/100;
      
      // Disable time waveform, ensure DC offset is used, set gain as per above for both channels
      mixerX.gain(0, gain); // Sound
      mixerX.gain(1, 1);    // DC
      mixerX.gain(2, 0);    // Sawtooth
      mixerX.gain(3, 0);
      
      mixerY.gain(0, gain); // Sound
      mixerY.gain(3, 0);
      
      // Blanking reacts to both axes
      mixerRMS.gain(0, 1);
 
      // Set X/Y positions from pots
      dcX.amplitude((float)map(analogRead(XPosPin), 0, 1024, -100, 100)/100);
      dcY.amplitude((float)map(analogRead(YPosPin), 0, 1024, -100, 100)/100);
    }
    else if(mode == 1) { // YT
      // Disable audio input and DC offset for X axis, enable sawtooth waveform
      mixerX.gain(0, 0);  // Sound
      mixerX.gain(1, 0);  // DC
      mixerX.gain(2, 1);  // Sawtooth
      mixerX.gain(3, 0);

      // Set Y position from rotary encoder
      dcY.amplitude((float)rotenc.getPosition()/100);
      
      // Set audio gain from left pot and timebase from right pot
      mixerY.gain(3, 0);
      mixerY.gain(0, (float)map(analogRead(XPosPin), 0, 1024, 0, 300)/100);
      waveform1.frequency(map(analogRead(YPosPin), 0, 1024, 5, 200));

      // Blanking reacts to sound only
      mixerRMS.gain(0, 0);
    } 
    else if(mode == 2) { // Spectrum
      
      mixerX.gain(0, 0);  // Sound
      mixerX.gain(1, 0);  // DC
      mixerX.gain(2, 1);  // Sawtooth

      waveform1.frequency(172);

      mixerY.gain(0, 0);
      mixerY.gain(1, 0);
      mixerY.gain(3, 1);
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
  if(mode == 2) {
    if(fft256L.available()) {
      for(int i = 10; i<128; i++)
        queueYBuffer[i] = (float)fft256L.output[i]*map(analogRead(XPosPin), 0, 1024, 1, 200);
      //AudioNoInterrupts();
      queueFFT.playBuffer();
    }
  }
}
