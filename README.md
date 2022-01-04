# AudioScope

Audio X/Y visualizer code for the Scopeclock from Cathode Corner 

https://scopeclock.com/

This makes use of the excellent [Teensy Audio Library](https://www.pjrc.com/teensy/td_libs_Audio.html) to expose the Teensy in the Scopeclock as a USB audio interface and display the incoming audio as X/Y for the L and R channels respectively. 

The X and Y position knobs work as expected, the rotary encoder adjusts gain(size), pressing it resets gain to the default of 1. 

The beam is blanked after 5 seconds of little movement to avoid hitting the screen too long in the center but without preventing some effects in common scope music. 

This demo shows [Jerobeam Fenderson's](https://oscilloscopemusic.com/) Kickstarter [video](https://www.youtube.com/watch?v=qnL40CbuodU) from 2015. The compression is visible, for example the Death Star is all mushy while it looks perfect when playing the WAV version instead of the video audio.

[![XY Demo](https://yt-embed.herokuapp.com/embed?v=GAK96sah6cw)](https://www.youtube.com/watch?v=GAK96sah6cw)

Other demo, close-up:
https://www.youtube.com/watch?v=C2VlYvGJmeg

### New 22.01.03: Now supports time-based display!

A long press of the rotary encoder will toggle between X-Y and Y-T modes. In Y-T mode the left pot adjusts signal amplitude (only the right channel signal is displayed), the right pot adjusts sweep frequency and the encoder adjusts vertical position, push to reset.

[![Waveform Demo](https://yt-embed.herokuapp.com/embed?v=tQQd2wUsB_g)](https://www.youtube.com/watch?v=tQQd2wUsB_g)

## Building/installing

- Install Arduino and the Teensyduino addon as per https://www.pjrc.com/teensy/td_download.html
- Power up your clock and connect it via USB to the computer (using the micro USB port, not the GPS port)
- Make sure you can build/flash the original code at https://github.com/nixiebunny/SCTVcode or that of amalmin at https://github.com/amalmin/SCTVcode that has nice additions to return your clock to normal function. This should only require opening SCTVCode.ino, selecting Tools -> Board Type -> Teensyduino -> Teensy 3.6 in the Arduino IDE, selecting the Port that should show up as Teensy 3.6, and hitting "Verify". If there are no errors you can Upload.
- Open the AudioScope sketch, but in addition to the above set Tools -> USB Type to "Audio" before uploading. 

Once the sketch is loaded the beam should show a dot for 5 seconds and turn off. At this point you can select your audio output on the computer to the newly appeared Teensy Audio interface, and now anything that plays will be shown on the clock! Brightness will need adjustment depending on content. 

Pro tip, on Windows use the [Voicemeeter Banana](https://vb-audio.com/Voicemeeter/banana.htm) software audio mixer to duplicate your main speaker output to allow listening while watching. Select your usual output as A1, the Scopeclock as A2. On the first Virtual Input strip click the A2 button so that it gets sent to both outputs. In Windows set your default output to Voicemeeter Input.

## Optional hardware mod

If you install a [PT8211 Audio Kit for Teensy 3.x](https://www.pjrc.com/store/pt8211_kit.html) on your Scopeclock's Teensy it will also output the audio there, with the level controlled by the PC volume independently of the scope display size. That's what's actually used in the demo above, with the JBL speaker as output.

![IMG_20220102_120553](https://user-images.githubusercontent.com/6065069/147874796-16991ea7-24d3-4f9f-9480-8bc852ecb849.jpg)


