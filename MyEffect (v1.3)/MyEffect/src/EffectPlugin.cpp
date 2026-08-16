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
            {   "Input Gain",  Parameter::ROTARY, 0.0, 1.0, 0.5, AUTO_SIZE  },
            {   "Filter Cutoff",  Parameter::ROTARY, 0.0, 1.0, 0.5, AUTO_SIZE  },
            {   "Output Gain",  Parameter::ROTARY, 0.0, 1.0, 0.5, AUTO_SIZE  },
        };

        const Presets PRESETS = {
            { "Preset 1", { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
            { "Preset 2", { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
            { "Preset 3", { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
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
    
    float fInGain = parameters[0];
	float fCutoff = parameters[1];
	float fOutGain = parameters[2];

    for (int ch = 0; ch < 2; ch++)
    {
        filter[ch].set(fCutoff);
        filter[ch].getCutoff(getSampleRate());
        secondOrderFilter[ch].set(fCutoff);
    }

	for (int i = 0; i < numSamples; i++)
	{
		for (int ch = 0; ch < 2; ch++)
		{
			// Get sample from input
			fIn[ch] = *pfInBuffer[ch]++;

			// Add your effect processing here#
			float fWet = fIn[ch] * fInGain;
			fWet = filter[ch].process(fWet); // shallow -6dB filter slope. Lets through significant frequency content above the cutoff.
			fWet = secondOrderFilter[ch].process(fWet); // second order filter -12dB filter slope. Lets through less frequency content above the cutoff.

			fOut[ch] = fWet * fOutGain;

			// Copy result to output
			*pfOutBuffer[ch]++ = fOut[ch];
		}
	}
}
