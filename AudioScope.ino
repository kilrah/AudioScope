// https://github.com/kilrah/AudioScope
// Audio X/Y visualizer code for the Scopeclock from Cathode Corner
// https://scopeclock.com/

// Audio connections from the Audio System Design Tool for Teensy Audio Library (https://www.pjrc.com/teensy/gui/index.html)
// GUItool: begin automatically generated code
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>

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
int BlankPin   =  2;   // Low blanks the display
int encButPin  = 14;   // encoder button, 0 = pressed
int encBPin    = 15;   // encoder quadrature
int encAPin    = 16; 
int XPosPin    = A15;  // horizontal centering knob
#ifdef SCTVA
  int YPosPin  = A18;  // vertical centering knob on SCTVA is in odd place
#else
  int YPosPin  = A16;  // vertical centering knob
#endif

// Rotary encoder
int EncDir = 0;      // initial position
int ButHist = 0;     // button pressing history
int LastEnc;         // encoder previous state
bool pushed = false; // 1 if button was pushed this frame

elapsedMillis controls = 0;
elapsedMillis rmsBlank = 0;

void setup() {
  //Serial.begin(115200);
  AudioMemory(10);
  pinMode(encButPin, INPUT_PULLUP);   // encoder pushbutton
  pinMode(encAPin,   INPUT_PULLUP);   // encoder quadrature signals
  pinMode(encBPin,   INPUT_PULLUP);
  pinMode(BlankPin, OUTPUT);
  dacs1.analogReference(EXTERNAL);
  mixerRMS.gain(0, 1);
  mixerRMS.gain(1, 1);
  filterRMS.frequency(100);
  filterRMS.resonance(0.7);
  InitEnc();
  digitalWrite(BlankPin, HIGH);       // Enable beam
}

void loop() {
  if(controls > 10) {
    float gain;
    controls = 0;

    // Update controls every 10ms
    DoEnc();
    DoButt();
    EncDir = constrain(EncDir, -20, 40);
    gain = ((float)map(EncDir, -20, 10, 0, 150))/100;
    if(pushed)
      EncDir = 0;

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


// --- Code from https://github.com/nixiebunny/SCTVcode ----
// ---------------------- Rotary encoder -------------------

// We save a copy of the previous reading in bits 1:0 of LastEnc.
// Then we read the current encoder to bits 3:2 of LastEnc and look up the
// direction in the nifty table below.

// The result is in EncDir - 0 for no motion, 1+ for right, -1- for left.
// The code that uses the encoder motion has to clear EncDir!

// Encoder quadrature lookup table
int incr = 1;  // encoder turned clockwise
int decr = -1;  // encoder turned counter-clockwise
int none = 0; // encoder didn't move
int impos = 0;  // not possible - glitch detected

// PEC11R encoder: Two detents per cycle, only change on bit 0 moving
//         old:   00  01   10    11       new
int EncTab[] = {none,decr,none,impos,  // 00
                incr,none,impos,none,  // 01
                none,impos,none,incr,  // 10
                impos,none,decr,none}; // 11

// This works best if called once per MainLp loop

// Initialize the encoder history to match its position
void InitEnc()
{
  LastEnc = digitalRead(encBPin) << 1 | digitalRead(encAPin); 
  LastEnc = LastEnc * 5;  // copy of old and new next to each other
}

// Read encoder, update history, look for motion, update into EncDir
// This must be called exactly once per MainLp loop
// Returns # of detents in EncDir, -=CCW, +=CW
void DoEnc() {
  LastEnc = LastEnc >> 2 | digitalRead(encBPin) << 3 | digitalRead(encAPin) << 2; 
  EncDir = EncDir + EncTab[LastEnc];   // bump encoder value by motion via lookup table
}

// DoButt detects if the encoder button is pushed, and debounces.
// If it's time for action, it returns Button bit set in Flags
// This must be called exactly once per MainLp loop
void DoButt() {
  if (digitalRead(encButPin) == 0) {  // zero is pressed
    ButHist++;
    pushed = (ButHist == 3);
  }
  else {
    ButHist = 0;
    pushed = false;
  }
}
