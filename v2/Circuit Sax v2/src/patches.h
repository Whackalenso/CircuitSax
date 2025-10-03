#include <Audio.h>
#include <waveforms.h>

struct Patch
{
    int waveform;
    int16_t *arbitraryWaveform;
    int detuneWaveform;
    float detuneAmounts[4];
    bool detune;
    float filterMin;
    float filterMax;
    float filterQ;
};

const Patch patches[] = {
    {WAVEFORM_SAWTOOTH, NULL, WAVEFORM_SAWTOOTH, {0, 0, 0, 0}, false, 10, 15000, 1},
    {WAVEFORM_SAWTOOTH, NULL, WAVEFORM_SQUARE, {-0.06, 0, 0, 0.06}, true, 10, 15000, 1}};

const int numPatches = 2;