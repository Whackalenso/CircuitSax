#include "MIDIUSB.h"
// #include "PitchToNote.h"
#include <math.h>

const int sensorPin = A5;
// bool lastReading = false;
// bool blowState = false;
// unsigned long lastDebounceTime;
// const int debounceDelay = 2;
bool isBlowing = false;
const int threshold = 75;
const int modWheel = 1;

const int palmKeys[3] = {4, 5, 6};                                     // D, D#, F
const int sideKeys[3] = {9, 10, 11};                                   // top -> bot
const int LPKeys[4] = {A0, A1, A2, A3};                                // G#, C#, B, Bb
const int RPKeys[2] = {A4, MISO};                                      // D#, C
const int whiteKeys[8] = {3, 8, SCK, 13, 12, 1, RPKeys[1], LPKeys[2]}; // B, A, G, F, E, D, Low C, Low B
const int octaveKey = 0;
const int ffKey = 2;
const int bisKey = 7;

#define CSHARP(oct) 13 + oct * 12
const int baseOctave = 4;
int activeNote = CSHARP(baseOctave);
int currentNote = activeNote;
const int timeToChangeNote = 0;
int lastChangedNote = -1 * timeToChangeNote;

// int calibrationTime = 10000;
// int threshold;
// long sampleCount = 0;

// list of regular keys (6 pearl keys + low c) pressed, if break in keys then thats the note
// example (list of keys pressed):
// [1, 2, 4] -> 2 is the note
// [2] -> 0 is the note (check for accidental)
// [1, 2, 3] -> 3 is the note (check for accidental)
// then check for accidental

void setup()
{
  pinMode(sensorPin, INPUT);
  for (int i = 0; i <= 22; i++)
  {
    if ((i <= 15) || (i >= 18))
    {
      pinMode(i, INPUT_PULLUP);
    }
  }
}

void loop()
{
  int sensorReading = analogRead(sensorPin);
  int velocity = constrain(sensorReading - threshold, 0, 64) * 2;
  controlChange(0, modWheel, velocity);

  // bool blowStateChanged = false;

  // if (reading != lastReading) {
  //   lastDebounceTime = millis();
  // }

  // if ((millis() - lastDebounceTime) > debounceDelay) {
  //   if (blowState != reading) {
  //     blowState = reading;
  //     blowStateChanged = true;
  //   }
  // }

  // lastReading = reading;

  int oct = digitalRead(octaveKey) == LOW ? baseOctave + 1 : baseOctave;
  int note = CSHARP(oct) + 3;

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
      if (i == 0)
      {
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
      }
      if (i == 1)
      {
        if (digitalRead(bisKey) == LOW)
        {
          note -= 1;
        }
        else if (digitalRead(sideKeys[1]) == LOW)
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
      }
      if (i == 2)
      {
        if (digitalRead(sideKeys[2]) == LOW)
        {
          note += 1;
        }
      }
      if (i == 3)
      {
        if (digitalRead(LPKeys[0]) == LOW)
        { // G#
          note += 1;
        }
        else if (digitalRead(whiteKeys[4]) == LOW)
        { // Gb
          note -= 1;
        }
      }
      if (i == 6)
      {
        if (digitalRead(RPKeys[0]) == LOW)
        { // D#
          note += 1;
        }
      }
      if (i == 7)
      {
        if (digitalRead(LPKeys[1]) == LOW)
        {
          note += 1;
        }
      }
      if (i == 7)
      { // low bb and b
        if (digitalRead(LPKeys[3]) == LOW)
        { // low bb and c
          note -= 2;
        }
      }

      break;
    }
  }

  // for (int i = 0; i < 3; i++) {
  //   Serial.print(digitalRead(palmKeys[i]));
  // }
  // Serial.print("\n");

  if ((velocity > 0) != isBlowing || note != activeNote)
  {
    isBlowing = velocity > 0;
    noteOff(0, activeNote, 0);
    activeNote = note;
    noteOn(0, activeNote, isBlowing ? 127 : 0);
    MidiUSB.flush();
  }
  else if (!isBlowing)
  { // just to make sure notes turn off
    for (int i = 46; i <= 77; i++)
    { // update when adding altissimo
      noteOff(0, i, 0);
    }
    MidiUSB.flush();
  }
  // if (note != currentNote) {
  //   lastChangedNote = millis();
  //   currentNote = note;
  // }

  // if ((millis() - lastChangedNote >= timeToChangeNote) && (currentNote != activeNote)) {
  //   if (isBlowing) {
  //     noteOff(0, activeNote, 0);
  //     noteOn(0, currentNote, 127);
  //     MidiUSB.flush();
  //   }
  //   activeNote = currentNote;
  // }

  // if ((velocity > 0) != isBlowing) {
  //   isBlowing = velocity > 0;
  //   if (isBlowing) {
  //     noteOn(0, activeNote, 127);
  //   } else {
  //     noteOff(0, activeNote, 0);
  //   }
  // }
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value)
{

  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};

  MidiUSB.sendMIDI(event);
}

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity)
{

  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};

  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity)
{

  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};

  MidiUSB.sendMIDI(noteOff);
}
