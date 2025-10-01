#include "MIDIUSB.h"
#include <math.h>

const int sensorPin = A5;
bool isBlowing = false;
const int min_air = 90;
const int max_air = 400;
const int modWheel = 1;

const int palmKeys[3] = {4, 5, 6};                                     // D, D#, F
const int sideKeys[3] = {9, 10, 11};                                   // top -> bot
const int LPKeys[4] = {A0, A1, A2, A3};                                // G#, C#, B, Bb
const int RPKeys[2] = {A4, MISO};                                      // D#, C
const int whiteKeys[8] = {3, 8, SCK, 13, 12, 1, RPKeys[1], LPKeys[2]}; // B, A, G, F, E, D, Low C, Low B
const int octaveKey = 0;
const int ffKey = 2;
const int bisKey = 7;

//const int altissimo = [[8, SCK], [8], [8, SCK, 12, 5]];

#define CSHARP(oct) 13 + oct * 12
const int baseOctave = 4;
int activeNote = CSHARP(baseOctave);
int prevFingers = 0;
int currentNote = activeNote;
const int timeToChangeNote = 1;
int lastChangedNote = -1 * timeToChangeNote;

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

//Serial.begin(9600);
}

int getVelocity(int sensorReading, float x2, float y2)
{

  return constrain(0.65 * (sensorReading - min_air), 0, 127);

  // float x1 = min_air;
  // float y1 = 0;
  // float x3 = max_air;
  // float y3 = 127;

  // float a = ((y1 * (x2 - x3)) + (y2 * (x3 - x1)) + (y3 * (x1 - x2))) /
  //           ((x1 * (x2 - x3) * (x2 - x3)) + (x2 * (x3 - x1) * (x3 - x1)) + (x3 * (x1 - x2) * (x1 - x2)));

  // float b = ((y1 - y2) - a * (x1 * x1 - x2 * x2)) / (x1 - x2);

  // float c = y1 - a * x1 * x1 - b * x1;

  // return constrain(a * pow(sensorReading, 2) + b * sensorReading + c, 0, 127);
}

int getModWheel(int velocity) {
  int steepness = 10;
  int midpoint = 120;
  
  int scaled_input = (velocity - midpoint) / steepness;
  return 127 / (1 + pow(2.71828, -scaled_input)) ;
}

void loop()
{
//  for (int i = 0; i <= 22; i++)
//  {
//    if ((i <= 15) || (i >= 18))
//    {
//      if (digitalRead(i) == LOW) {
//        Serial.println(i);
//      }
//    }
//  }
  
  int sensorReading = analogRead(sensorPin);
  int velocity = getVelocity(sensorReading, 200, 100); // constrain(0.5 * (sensorReading - min_air), 0, 127);
  controlChange(0, modWheel, getModWheel(velocity));
  controlChange(0, 2, velocity);
  // Serial.println(String(velocity) + " " + String(sensorReading));

  int oct = digitalRead(octaveKey) == LOW ? baseOctave + 1 : baseOctave;
  int note = CSHARP(oct) + 3;

//  if (digitalRead(ffKey) == LOW) {
//    for (int i = 0; i < 3; i++) {
//      bool correctFingering = true;
//      for (int i = 0; i <= 22; i++)
//      {
//        if ((i <= 15) || (i >= 18))
//        {
//          if (
//        }
//      }
//    }
//  }
  
  int i;
  for (i = 0; i < 8; i++)
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
      switch (i) {
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

  // Phone
//   if ((velocity > 0) != isBlowing || note != activeNote)
//   {
//     isBlowing = velocity > 0;
//     noteOff(0, activeNote, 0);
//     activeNote = note;
//     noteOn(0, activeNote, isBlowing ? 127 : 0);
//     MidiUSB.flush();
//   }
//   else if (!isBlowing)
//   { // just to make sure notes turn off
//     for (int i = 46; i <= 77; i++)
//     { // update when adding altissimo
//       noteOff(0, i, 0);
//     }
//     MidiUSB.flush();
//   }

   // Computer
  if (note != currentNote)
  {
    lastChangedNote = millis();
    currentNote = note;
  }

  if ((millis() - lastChangedNote >= timeToChangeNote) && (currentNote != activeNote))
  {
//    Serial.println(prevFingers > i);
    if (isBlowing)
    {
      noteOff(0, activeNote, 0);
      noteOn(0, currentNote, velocity);
      MidiUSB.flush();
    }
    activeNote = currentNote;
    prevFingers = i;
  }

  // add debounce 
  if ((velocity > 0) != isBlowing)
  {
    isBlowing = velocity > 0;
    if (isBlowing)
    {
      noteOn(0, activeNote, velocity);
    }
    else
    {
      noteOff(0, activeNote, 0);
    }
  }
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
