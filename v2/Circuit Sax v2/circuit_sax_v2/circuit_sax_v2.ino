#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <MPU6050.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=2165,705
AudioFilterStateVariable filter1;        //xy=2335,712
AudioOutputI2S           i2s1;           //xy=2518,695.5
AudioConnection          patchCord1(waveform1, 0, filter1, 0);
AudioConnection          patchCord2(filter1, 0, i2s1, 0);
AudioConnection          patchCord3(filter1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=2365,882
// GUItool: end automatically generated code


MPU6050 mpu(0x68, &Wire2);

//    -----------------       DONT CONFIGURE 22         !!!!!!!!!!!!!

int breathPin = 26;
bool isBlowing = false;
float breathMin = 70;
float breathMax = 600;

const int modWheelCC = 1;
const int breathCC = 2;

const int palmKeys[3] = {41, 40, 39};                                     // D, D#, F
const int sideKeys[3] = {36, 35, 34};                                   // top -> bot
const int LPKeys[4] = {28, 30, 29, 31};                                // G#, C#, B, Bb
const int RPKeys[2] = {5, 4};                                      // D#, C
const int whiteKeys[8] = {10, 37, 32, 27, 33, 12, RPKeys[1], LPKeys[2]}; // B, A, G, F, E, D, Low C, Low B
const int octKey = 6;
const int ffKey = 9;
const int bisKey = 38;

#define CSHARP(oct) 13 + oct * 12
const int baseOct = 4;
const int transposition = 3;
int activeNote = CSHARP(baseOct);
int currentNote = activeNote;
const int timeToChangeNote = 1;
int lastChangedNote = -1 * timeToChangeNote;

float map(float value, float inMin, float inMax, float outMin, float outMax) {
  float scale = (value - inMin) / (inMax - inMin);
  return outMin + scale * (outMax - outMin);
}

float mapLog(float value, float inMin, float inMax, float outMin, float outMax) {
  int result = map(value, inMin, inMax, log10(outMin), log10(outMax));
  return pow(10, result);
}

// float mapLog(float value, float in_min, float in_max, float out_min, float out_max) {
//   float log_out_min = log10(out_min);
//   float log_out_max = log10(out_max);
//   float scale = (value - in_min) / (in_max - in_min);
//   float log_result = log_out_min + scale * (log_out_max - log_out_min);
//   return pow(10, log_result);
// }

float noteToFreq(int midi) {
  return 440 * pow(2.0, (midi - 69) / 12.0);
}

int getModWheel(int velocity) {
  int steepness = 10;
  int midpoint = 120;
  
  int scaledInput = (velocity - midpoint) / steepness;
  return 127 / (1 + pow(2.71828, -scaledInput)) ;
}

int getNewNote() {
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

  return note;
}

//    -----------------       DONT CONFIGURE 22         !!!!!!!!!!!!!

void setup() {
  Serial.begin(9600);

  for (int pin : palmKeys) { pinMode(pin, INPUT_PULLUP); }
  for (int pin : sideKeys) { pinMode(pin, INPUT_PULLUP); }
  for (int pin : LPKeys) { pinMode(pin, INPUT_PULLUP); }
  for (int pin : RPKeys) { pinMode(pin, INPUT_PULLUP); }
  for (int pin : whiteKeys) { pinMode(pin, INPUT_PULLUP); }
  pinMode(octKey, INPUT_PULLUP);
  pinMode(ffKey, INPUT_PULLUP);
  pinMode(bisKey, INPUT_PULLUP);

  AudioMemory(8);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);

  waveform1.begin(WAVEFORM_SAWTOOTH);
}

void loop() {

  int rawBreath = analogRead(breathPin);
  int velocity = constrain(map(rawBreath, breathMin, breathMax, 0, 255), 0, 255);
  usbMIDI.sendControlChange(modWheelCC, getModWheel(velocity), 0);
  usbMIDI.sendControlChange(breathCC, velocity, 0);
  waveform1.amplitude((rawBreath > 80) ? 0.5 : 0);

  int newNote = getNewNote();

  if (newNote != currentNote)
  {
    lastChangedNote = millis();
    currentNote = newNote;
  }

  if ((millis() - lastChangedNote >= timeToChangeNote) && (currentNote != activeNote))
  {
    if (isBlowing)
    {
      usbMIDI.sendNoteOff(activeNote, 0, 0);
      usbMIDI.sendNoteOn(currentNote, velocity, 0);

      waveform1.frequency(noteToFreq(currentNote));
    }
    activeNote = currentNote;
  }

  // add debounce 
  if ((velocity > 0) != isBlowing)
  {
    isBlowing = velocity > 0;
    if (isBlowing)
    {
      usbMIDI.sendNoteOn(activeNote, velocity, 0);
      waveform1.frequency(noteToFreq(activeNote));
    }
    else
    {
      usbMIDI.sendNoteOff(activeNote, 0, 0);
    }
  }


  // float filter_freq = mapLog(breath, breath_min, breath_max, filter_min, filter_max);
  // filter_freq = constrain(filter_freq, filter_min, filter_max);
  // // Serial.print(breath);
  // // Serial.print(" ");
  // // Serial.println(filter_freq);
  // filter1.frequency(filter_freq);
}




