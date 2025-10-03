#include <MPU6050_tockn.h>
#include <patches.h>
#include <Bounce2.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform waveform1;     // xy=1435.5,776.5
AudioSynthWaveform waveform3;     // xy=1444,871
AudioSynthWaveform waveform2;     // xy=1447,818
AudioSynthWaveform waveform4;     // xy=1450,927
AudioMixer4 waveformMixer;        // xy=1691,798
AudioFilterStateVariable filter1; // xy=1888.9999694824219,784.9999885559082
AudioMixer4 wetMixer;             // xy=2121.5,775
AudioOutputI2S i2s1;              // xy=2321,776
AudioOutputUSB usb1;              // for USB audio out
AudioConnection patchCord1(waveform1, 0, waveformMixer, 0);
AudioConnection patchCord2(waveform3, 0, waveformMixer, 2);
AudioConnection patchCord3(waveform2, 0, waveformMixer, 1);
AudioConnection patchCord4(waveform4, 0, waveformMixer, 3);
AudioConnection patchCord5(waveformMixer, 0, filter1, 0);
AudioConnection patchCord6(filter1, 0, wetMixer, 0);
AudioConnection patchCord7(wetMixer, 0, i2s1, 0);
AudioConnection patchCord8(wetMixer, 0, i2s1, 1);
AudioConnection patchCord9(wetMixer, 0, usb1, 0);
AudioControlSGTL5000 sgtl5000_1; // xy=2347.5000076293945,872.5000019073486
// GUItool: end automatically generated code

MPU6050 mpu6050(Wire2); // sda scl: 25, 24

Patch patch = patches[0];

//    -----------------       DONT CONFIGURE 22         !!!!!!!!!!!!!

// Breath
int breathPin = 26;
bool isBlowing = false;
float breathMin = 80;
float breathMax = 650;
const int timeToChangeBreath = 10;
int lastChangedBreath = -1 * timeToChangeBreath;

// Control Change
const int modWheelCC = 1;
const int breathCC = 2;
const int ccSendTime = 1;
int lastSentCC = -1 * ccSendTime;

// Button pins
const int palmKeys[3] = {41, 40, 39};                                    // D, D#, F
const int sideKeys[3] = {36, 35, 34};                                    // top -> bot
const int LPKeys[4] = {28, 29, 30, 31};                                  // G#, C#, B, Bb
const int RPKeys[2] = {5, 4};                                            // D#, C
const int whiteKeys[8] = {10, 37, 32, 27, 33, 12, RPKeys[1], LPKeys[2]}; // B, A, G, F, E, D, Low C, Low B
const int octKey = 6;
const int ffKey = 9;
const int bisKey = 38;
const int configBtn = 11;

// Note
#define CSHARP(oct) 13 + oct * 12
int baseOct = 4;
int transposition = 3;
int activeNote = CSHARP(baseOct); // Note currently played
int pressedNote = activeNote;
const int pressNoteTime = 1;
int lastPressedNote = -1 * pressNoteTime; // when pressedNote was last changed
int pitchOffset = 0.21;

// FF Key octave jumping
int octChange = 0;
Button ffKeyBtn = Button();

// Portamento
const int portamentoDuration = 20; // ms
int startPitch = -1;               // st
int targetPitch = -1;              // st
float currentPitch = -1;           // st
int lastChangedTargetPitch;

// Pitch Bend
const int pitchBendAmt = 1;

// Detuning
AudioSynthWaveform *waveforms[4] = {&waveform1, &waveform2, &waveform3, &waveform4};

// Utility functions

float map(float value, float inMin, float inMax, float outMin, float outMax)
{
  float scale = (value - inMin) / (inMax - inMin);
  return outMin + scale * (outMax - outMin);
}

float mapLog(float value, float inMin, float inMax, float outMin, float outMax)
{
  float result = map(value, inMin, inMax, log10(outMin), log10(outMax));
  return pow(10, result);
}

float mapWithCurve(float value, float inMin, float inMax, float outMin, float outMax, float (*curve)(float))
{
  float normalized = map(value, inMin, inMax, 0, 1);
  return map(curve(normalized), 0, 1, outMin, outMax);
}

float volumeCurve(float x)
{
  return 1 - pow(1 - x, 1);
}

float noteToFreq(float midi)
{
  return 440 * pow(2.0, (midi - 69) / 12.0);
}

float freqToNote(float freq)
{
  return 12.0 * log2(freq / 440.0);
}

int getModWheel(int velocity)
{
  int steepness = 10;
  int midpoint = 120;

  int scaledInput = (velocity - midpoint) / steepness;
  return 127 / (1 + pow(2.71828, -scaledInput));
}

int getPressedNote()
{
  int oct = digitalRead(octKey) == LOW ? baseOct + 1 : baseOct;
  int note = CSHARP(oct) + transposition;

  for (int i = 0; i < 8; i++)
  {
    if (digitalRead(whiteKeys[i]) == LOW)
    {
      note -= (i == 4 || i == 7) ? 1 : 2; // 4 and 7 are e and low b
      if (i == 7 && digitalRead(LPKeys[3]) == LOW)
      { // low bb and b
        note -= 1;
      }
    }
    else
    {
      // accidentals
      switch (i)
      {
      case 0:
        if (digitalRead(whiteKeys[1]) == LOW)
        { // C
          note -= 1;
        }
        else if (digitalRead(palmKeys[0]) == LOW)
        {
          note += 1;
          if (digitalRead(palmKeys[1]) == LOW)
          {
            note += 1;
            if ((digitalRead(sideKeys[0]) == LOW) && (digitalRead(palmKeys[2]) == HIGH))
            {
              note += 1;
            }
            else if (digitalRead(palmKeys[2]) == LOW)
            {
              note += 2;
            }
          }
        }
        break;

      case 1:
        if (digitalRead(bisKey) == LOW)
        {
          note -= 1;
        }
        if (digitalRead(sideKeys[1]) == LOW)
        {
          note += 1;
        }
        else
        {
          for (int j = 3; j <= 5; j++)
          { // Bb
            if (digitalRead(whiteKeys[j]) == LOW)
            {
              note -= 1;
              break;
            }
          }
        }
        break;

      case 2:
        if (digitalRead(sideKeys[2]) == LOW)
        {
          note += 1;
        }
        break;

      case 3:
        if (digitalRead(LPKeys[0]) == LOW)
        { // G#
          note += 1;
        }
        else if (digitalRead(whiteKeys[4]) == LOW)
        { // Gb
          note -= 1;
        }
        break;

      case 6:
        if (digitalRead(RPKeys[0]) == LOW)
        { // D#
          note += 1;
        }
        break;

      case 7:
        if (digitalRead(LPKeys[1]) == LOW)
        {
          note += 1;
        }

        // low bb and b
        if (digitalRead(LPKeys[3]) == LOW)
        { // low bb and c
          note -= 2;
        }
        break;
      }
      break;
    }
  }

  return note;
}

void setPatch(int index)
{
  patch = patches[index];
  for (int i = 0; i < 4; i++)
  {
    int waveform = patch.waveform;
    if (i == 0 || i == 3)
    {
      waveform = patch.detuneWaveform;
    }
    waveforms[i]->begin(waveform);
    if (patch.arbitraryWaveform)
    {
      waveforms[i]->arbitraryWaveform(patch.arbitraryWaveform, 12543.85);
    }
  }

  filter1.resonance(patch.filterQ);
}

//    -----------------       DONT CONFIGURE 22         !!!!!!!!!!!!!

void setup()
{
  Serial.begin(9600);

  for (int pin : palmKeys)
  {
    pinMode(pin, INPUT_PULLUP);
  }
  for (int pin : sideKeys)
  {
    pinMode(pin, INPUT_PULLUP);
  }
  for (int pin : LPKeys)
  {
    pinMode(pin, INPUT_PULLUP);
  }
  for (int pin : RPKeys)
  {
    pinMode(pin, INPUT_PULLUP);
  }
  for (int pin : whiteKeys)
  {
    pinMode(pin, INPUT_PULLUP);
  }
  pinMode(octKey, INPUT_PULLUP);
  pinMode(ffKey, INPUT_PULLUP);
  pinMode(bisKey, INPUT_PULLUP);
  pinMode(configBtn, INPUT_PULLUP);

  ffKeyBtn.attach(ffKey, INPUT_PULLUP);
  ffKeyBtn.interval(5);
  ffKeyBtn.setPressedState(LOW);

  AudioMemory(8);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5); // over 0.5 makes amp not work

  for (AudioSynthWaveform *w : waveforms)
  {
    w->amplitude(1);
  }

  filter1.octaveControl(1);
  // reverb1.reverbTime(0.3);
  // dryMixer.gain(0, 1);
  // dryMixer.gain(1, 0);
  // wetMixer.gain(1, 0);

  setPatch(0);

  Wire2.begin();
  mpu6050.begin();
}

void loop()
{
  mpu6050.update();

  // Breath
  int rawBreath = analogRead(breathPin);
  int velocity = constrain(map(rawBreath, breathMin, breathMax, 0, 127), 0, 127);

  // if (millis() - lastSentCC >= ccSendTime)
  // {
  //   usbMIDI.sendControlChange(modWheelCC, velocity, 0); // getModWheel(velocity)
  //   usbMIDI.sendControlChange(breathCC, velocity, 0);
  //   lastSentCC = millis();
  // }

  int newNote = getPressedNote();

  // Once new note is pressed, starts timer to actually send midi signal
  if (newNote != pressedNote)
  {
    lastPressedNote = millis();
    pressedNote = newNote;
  }

  // Change pitch
  if ((millis() - lastPressedNote >= pressNoteTime) && (pressedNote != activeNote))
  {
    // if (isBlowing)
    // {
    //   // usbMIDI.sendNoteOff(activeNote, 0, 0);
    //   // usbMIDI.sendNoteOn(pressedNote, velocity, 0);

    //   waveform1.frequency(noteToFreq(pressedNote));
    // }

    baseOct += octChange;
    pressedNote += octChange * 12;
    octChange = 0;

    activeNote = pressedNote;
    targetPitch = activeNote;
    startPitch = currentPitch != -1 ? currentPitch : targetPitch;
    lastChangedTargetPitch = millis();
  }
  // Portamento
  if (startPitch != -1)
  {
    float elapsedTime = millis() - lastChangedTargetPitch;
    if (elapsedTime <= portamentoDuration)
    {
      currentPitch = map(elapsedTime / portamentoDuration, 0, 1, startPitch, targetPitch);
    }
    else // Pitch bend
    {
      float pitchBend = mpu6050.getAccX() * -1;
      if (abs(pitchBend) < 0.15)
      {
        pitchBend = 0;
      }
      Serial.println(mpu6050.getAccX());
      currentPitch = targetPitch + pitchBend;
    }
  }
  for (int i = 0; i < 4; i++)
  {
    waveforms[i]
        ->frequency(noteToFreq(currentPitch + patch.detuneAmounts[i]));
  }

  // Switch breath state
  if ((millis() - lastChangedBreath >= timeToChangeBreath) && ((velocity > 0) != isBlowing))
  {
    isBlowing = velocity > 0;
    lastChangedBreath = millis();

    baseOct += octChange;
    octChange = 0;

    // if (isBlowing)
    // {
    //   // usbMIDI.sendNoteOn(activeNote, velocity, 0);
    //   waveform1.frequency(noteToFreq(activeNote));
    // }
    // else
    // {
    //   // usbMIDI.sendNoteOff(activeNote, 0, 0);
    // }
  }

  // Filter1
  float filterFreq = mapLog(velocity, 0, 127, patch.filterMin, patch.filterMax);
  filter1.frequency(filterFreq);

  // Volume
  float volume = mapWithCurve(velocity * 1.0, 0, 127, 0, 1, volumeCurve);
  if (patch.detune)
  {
    for (int i = 0; i < 4; i++)
    {
      waveformMixer.gain(i, volume / 4);
    }
  }
  else
  {
    waveformMixer.gain(0, volume);
    for (int i = 1; i < 4; i++)
    {
      waveformMixer.gain(i, 0);
    }
  }

  // Config
  if (digitalRead(configBtn) == LOW)
  {
    // Select patch
    for (int i = 0; i < numPatches; i++)
    {
      if (digitalRead(RPKeys[i]) == LOW)
      {
        setPatch(i);
        break;
      }
    }

    if (digitalRead(palmKeys[0]) == LOW)
    {
      sgtl5000_1.volume(0.5);
    }
    if (digitalRead(palmKeys[1]) == LOW)
    {
      sgtl5000_1.volume(0.6);
    }
    if (digitalRead(palmKeys[2]) == LOW)
    {
      sgtl5000_1.volume(1);
    }

    // Octave Reset
    if (digitalRead(ffKey) == LOW)
    {
      baseOct = 4;
      octChange = 0;
    }

    if (digitalRead(whiteKeys[3]) == LOW)
    {
      transposition = 3;
    }
    if (digitalRead(whiteKeys[4]) == LOW)
    {
      transposition = -2;
    }
    if (digitalRead(whiteKeys[5]) == LOW)
    {
      transposition = 0;
    }
  }

  // FF Key Octave Jumping
  ffKeyBtn.update();
  if (ffKeyBtn.pressed())
  {
    if (octChange == 0)
    {
      if (velocity == 0)
      {
        baseOct += (digitalRead(octKey) == LOW) ? 1 : -1;
      }
      else
      {
        octChange = (digitalRead(octKey) == LOW) ? 1 : -1;
      }
    }
  }
}