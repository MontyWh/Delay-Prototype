//
//  EffectPlugin.h
//  MyEffect Plugin Header File
//
//  Used to declare objects and data structures used by the plugin.
//

#pragma once

#include "apdi/Plugin.h"
#include "apdi/Helpers.h"
using namespace APDI;

#include "EffectExtra.h"

class MyEffect : public APDI::Effect
{
public:
    MyEffect(const Parameters& parameters, const Presets& presets); // constructor (initialise variables, etc.)
    ~MyEffect();                                                    // destructor (clean up, free memory, etc.)

    void setSampleRate(float sampleRate){ stk::Stk::setSampleRate(sampleRate); }
    float getSampleRate() const { return stk::Stk::sampleRate(); };

    void setupFilters(float fLpfCutoff, float fBpfFrequency, float fBpfQ, float fBpfBandwidth, float fHpfCutoff);
    void processFilters(float fLpfOnOff, int ch, float& fWet, float fLpfGain, float fBpfOnOff, float fBpfGain, float fHpfOnOff, float fHpfGain);

    void process(const float** inputBuffers, float** outputBuffers, int numSamples);
    
    void presetLoaded(int iPresetNum, const char *sPresetName);
    void optionChanged(int iOptionMenu, int iItem);
    void buttonPressed(int iButton);

private:
    // Declare shared member variables here

	MyFilters::MyLowPassFilter::MyBiQuadFilter LPF[2]; // Stereo 4-pole lowpass filter (2 channels, 4 filters per channel)
	MyFilters::MyBandPassFilter::MyBiQuadFilter BPF[2]; // Stereo 4-pole bandpass filter (2 channels, 4 filters per channel)
	MyFilters::MyHighPassFilter::MyBiQuadFilter HPF[2]; // Stereo 4-pole highpass filter (2 channels, 4 filters per channel)
};
