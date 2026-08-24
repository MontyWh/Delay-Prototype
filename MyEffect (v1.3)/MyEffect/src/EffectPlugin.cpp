//
//  EffectPlugin.cpp
//  MyEffect Plugin Source Code
//
//  Used to define the bodies of functions used by the plugin, as declared in EffectPlugin.h.
//

#include "EffectPlugin.h"

////////////////////////////////////////////////////////////////////////////
// EFFECT - represents the whole effect plugin
////////////////////////////////////////////////////////////////////////////

// Called to create the effect (used to add your effect to the host plugin)
extern "C" {
    CREATE_FUNCTION createEffect(float sampleRate) {
        ::stk::Stk::setSampleRate(sampleRate);
        
        //==========================================================================
        // CONTROLS - Use this array to completely specify your UI
        // - tells the system what parameters you want, and how they are controlled
        // - add or remove parameters by adding or removing entries from the list
        // - each control should have an expressive label / caption
        // - controls can be of different types: ROTARY, BUTTON, TOGGLE, SLIDER, or MENU (see definitions)
        // - for rotary and linear sliders, you can set the range of values (make sure the initial value is inside the range)
        // - for menus, replace the three numeric values with a single array of option strings: e.g. { "one", "two", "three" }
        // - by default, the controls are laid out in a grid, but you can also move and size them manually
        //   i.e. replace AUTO_SIZE with { 50,50,100,100 } to place a 100x100 control at (50,50)
        
		const Parameters CONTROLS = {
			//  name,       type,              min, max, initial, size
			{   "Input Gain",  Parameter::ROTARY, 0.0, 1.0f, 1.0f, AUTO_SIZE  },

			{   "LPF Gain",  Parameter::ROTARY, 0.0f, 1.0f, 0.5f, AUTO_SIZE},
			{   "LPF Cutoff",  Parameter::ROTARY, 0.0, 1.0f, 0.25f, AUTO_SIZE  },
			{   "LPF On/Off",  Parameter::TOGGLE, 0.0, 1.0f, 1.0f, AUTO_SIZE  },

			{   "BPF Gain",  Parameter::ROTARY, 0.0f, 1.0f, 0.5f, AUTO_SIZE  },
			{   "BPF Q",  Parameter::ROTARY, 0.0, 1.0f, 0.51f, AUTO_SIZE },
			{   "BPF Frequency",  Parameter::ROTARY, 0.0, 1.0f, 0.75f, AUTO_SIZE },
			{   "BPF On/Off",  Parameter::TOGGLE, 0.0, 1.0f, 1.0f, AUTO_SIZE },

			{   "HPF Gain",  Parameter::ROTARY, 0.0f, 1.0f, 0.5f, AUTO_SIZE  },
			{   "HPF Cutoff",  Parameter::ROTARY, 0.0, 1.0f, 0.75f, AUTO_SIZE  },
			{   "HPF On/Off",  Parameter::TOGGLE, 0.0, 1.0f, 1.0f, AUTO_SIZE  },

			{   "Output Gain",  Parameter::ROTARY, 0.0, 1.0f, 1.0f, AUTO_SIZE  },
		};

		const Presets PRESETS = {
			{ "Preset 1", { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
			{ "Preset 2", { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
			{ "Preset 3", { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
		};

        return (APDI::Effect*)new MyEffect(CONTROLS, PRESETS);
    }
}

// Constructor: called when the effect is first created / loaded
MyEffect::MyEffect(const Parameters& parameters, const Presets& presets)
: Effect(parameters, presets)
{
    // Initialise member variables, etc.
}

// Destructor: called when the effect is terminated / unloaded
MyEffect::~MyEffect()
{
    // Put your own additional clean up code here (e.g. free memory)
}

// EVENT HANDLERS: handle different user input (button presses, preset selection, drop menus)

void MyEffect::presetLoaded(int iPresetNum, const char *sPresetName)
{
    // A preset has been loaded, so you could perform setup, such as retrieving parameter values
    // using getParameter and use them to set state variables in the plugin
}

void MyEffect::optionChanged(int iOptionMenu, int iItem)
{
    // An option menu, with index iOptionMenu, has been changed to the entry, iItem
}

void MyEffect::buttonPressed(int iButton)
{
    // A button, with index iButton, has been pressed
}

// Applies audio processing to a buffer of audio
// (inputBuffer contains the input audio, and processed samples should be stored in outputBuffer)
void MyEffect::process(const float** inputBuffers, float** outputBuffers, int numSamples)
{
    float fIn[2], fOut[2] = {0.0f, 0.0f};
    const float *pfInBuffer[2] = { inputBuffers[0], inputBuffers[1] };
    float *pfOutBuffer[2] = { outputBuffers[0], outputBuffers[1] };
    
	float fInGain = pow(parameters[0], 3.0f);

	float fLpfGain = pow(parameters[1], 3.0f);
	float fLpfCutoff = pow(parameters[2], 3.0f);
	float fLpfOnOff = parameters[3];

	float fBpfGain = pow(parameters[4], 3.0f);
	float fBpfQ = pow(parameters[5], 2.0f);
	float fBpfFrequency = pow(parameters[6], 3.0f);
	float fBpfOnOff = parameters[7];

	float fHpfGain = pow(1.0f - parameters[8], 3.0f);
	float fHpfCutoff = pow(parameters[9], 3.0f);
	float fHpfOnOff = parameters[10];

	float fOutGain = pow(parameters[11], 3.0f);

	for (int ch = 0; ch < 2; ch++)
	{
		LPF[ch].set(fLpfCutoff);
		BPF[ch].set(fBpfFrequency, fBpfQ);
		HPF[ch].set(fHpfCutoff);
	}

	for (int i = 0; i < numSamples; i++)
	{
		for (int ch = 0; ch < 2; ch++)
		{
			// Get sample from input
			fIn[ch] = *pfInBuffer[ch]++;

			// Add your effect processing here
			fIn[ch] *= fInGain; // Apply input gain
			float fWet = fIn[ch];
			float fLpfSig = fWet;
			float fBpfSig = fWet;
			float fHpfSig = fWet;

			/*if (fLpfOnOff <= 0.5f)
			{
				float fFiltered = LPF[ch].process(fLpfSig);

				if (fLpfGain < 0.5f)
				{
					// Filter mode: blend from filtered to dry
					float blend = fLpfGain * 2.0f;  // 0.0 to 1.0
					fLpfSig = fFiltered * (1.0f - blend) + fLpfSig * blend;
				}
				else
				{
					// Shelf mode: boost low frequencies
					float shelfGain = (fLpfGain - 0.5f) * 2.0f;  // 0.0 to 1.0
					fLpfSig = fWet + (fWet - fFiltered) * shelfGain;
				}
			}*/

			/*if (fHpfOnOff <= 0.5f)
			{
				float fFiltered = HPF[ch].process(fHpfSig);

				if (fHpfGain < 0.5f)
				{
					// Filter mode: blend from filtered to dry
					float blend = fHpfGain * 2.0f;  // 0.0 to 1.0
					fHpfSig = fFiltered * (1.0f - blend) + fHpfSig * blend;
				}
				else
				{
					// Shelf mode: boost low frequencies
					float shelfGain = (fHpfGain - 0.5f) * 2.0f;  // 0.0 to 1.0
					fHpfSig = fHpfSig + (fHpfSig - fFiltered) * shelfGain;
				}
			}*/

			if (fBpfOnOff <= 0.5f)
			{
				float fFiltered = BPF[ch].process(fWet);

				// Calculate blend factor ONCE, before using it on each sample
				float fBlend;
				if (fBpfGain <= 0.5f)
				{
					// 0.0 = full filter, 0.5 = unchanged (0 dB)
					fBlend = fBpfGain * 2.0f;
					fBpfSig = fFiltered * (1.0f - fBlend) + (fWet * fBlend);
				}
				else
				{
					// 0.5 = unchanged (0 dB), 1.0 = full shelf boost
					fBlend = (fBpfGain - 0.5f) * 2.0f;
					fBpfSig = fWet + (fFiltered * fBlend);
				}
			}

			fWet = /*fLpfSig * */fBpfSig/* * fHpfSig*/;

			fOut[ch] = fWet * fOutGain;

			// Copy result to output
			*pfOutBuffer[ch]++ = fOut[ch];
		}
	}
}
