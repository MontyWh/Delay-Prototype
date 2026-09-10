//
//  EffectPlugin.h
//  MyEffect Plugin Header File
//
//  Used to declare objects and data structures used by the plugin.
//

#pragma once

#include <algorithm>

#include "apdi/Plugin.h"
#include "apdi/Helpers.h"
using namespace APDI;
#include <cmath>

#include "EffectExtra.h"

class MyEffect : public APDI::Effect
{
public:
	class MyStereoProcessor
	{
	public:

		MyStereoProcessor() {}

		void initialise(float sampleRate)
		{
			Echo[0].initialise(sampleRate);
			Echo[1].initialise(sampleRate);
		}

		void configure(float* fDelayEffectTimes, float fReverbPatterns[][4], float fFeedbackGain, float fLpfCutoff, float fDrive, float fDiffusion, int iNumberOfDelays)
		{
			Echo[0].setupParameters(fDelayEffectTimes, fReverbPatterns, fFeedbackGain, fLpfCutoff, fDrive, fDiffusion, iNumberOfDelays);
			Echo[1].setupParameters(fDelayEffectTimes, fReverbPatterns, fFeedbackGain, fLpfCutoff, fDrive, fDiffusion, iNumberOfDelays);
		}

		void process(float inputLeft, float inputRight, float sampleRate, int bypassDelay, int bypassDelayMod, int bypassReverb, float modRate, float modDepth, float modDelayTime, float drive, float stereoWidth, float& outputLeft, float& outputRight)
		{
			float fWetLeft = Echo[0].process(inputLeft, sampleRate, bypassDelay, bypassDelayMod, bypassReverb, modRate, modDepth, modDelayTime, drive);
			float fWetRight = Echo[1].process(inputRight, sampleRate, bypassDelay, bypassDelayMod, bypassReverb, modRate, modDepth, modDelayTime, drive);

			float fMid = (fWetLeft + fWetRight) * 0.5f;
			float fSide = (fWetLeft - fWetRight) * 0.5f;
			if (stereoWidth < 0.0f) stereoWidth = 0.0f;
			if (stereoWidth > 1.0f) stereoWidth = 1.0f;
			fSide *= stereoWidth;
			outputLeft = fMid + fSide;
			outputRight = fMid - fSide;
		}

		void postProcess()
		{
			Echo[0].postProcess();
			Echo[1].postProcess();
		}

	public:
		MyEcho Echo[2];
	};

	MyEffect(const Parameters& parameters, const Presets& presets); // constructor (initialise variables, etc.)
	~MyEffect();                                                    // destructor (clean up, free memory, etc.)

	void setSampleRate(float sampleRate){ stk::Stk::setSampleRate(sampleRate); }
	float getSampleRate() const { return stk::Stk::sampleRate(); };

	void wetDryBlend(float& output, float wet, float mix, float dry);

	void process(const float** inputBuffers, float** outputBuffers, int numSamples);

	void presetLoaded(int iPresetNum, const char *sPresetName);
	void optionChanged(int iOptionMenu, int iItem);
	void buttonPressed(int iButton);

private:
	// Declare shared member variables here

	float updateTempoDivisions(int tempoBpm, float tempoOrTime, float delayTime);

	MyStereoProcessor Stereo;
	float fDelayEffectTimes[3];

	float fSampleRate;

	float fCrotchet, fQuaver, fSemiQuaver;
};
