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
            {   "Param 0",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },
            {   "Param 1",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },
            {   "Param 2",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },
            {   "Param 3",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },
            {   "Param 4",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },
            {   "Param 5",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },
            {   "Param 6",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },
            {   "Param 7",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },
            {   "Param 8",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },
            {   "Param 9",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },
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

	iBufferSize = fSampleRate * 2; // 2 seconds of audio

	for (int ch = 0; ch < 2; ch++)
    {
        pfCircularBuffer[ch] = new float[iBufferSize]; // Allocate memory for the circular buffer
        
        for (int i = 0; i < iBufferSize; i++) pfCircularBuffer[ch][i] = 0.0f; // Initialise the circular buffer to zero
    }

    iBufferWritePos = 0; // Reset the write position to the start of the buffer
}

// Destructor: called when the effect is terminated / unloaded
MyEffect::~MyEffect()
{
    // Put your own additional clean up code here (e.g. free memory)
    for (int ch = 0; ch < 2; ch++) delete[] pfCircularBuffer[ch];
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
   	float fWet[2], fIn[2], fOut[2] = { 0, 0, };
	const float* pfInBuffer[2] = { inputBuffers[0], inputBuffers[1] };
    float *pfOutBuffer[2] = { outputBuffers[0], outputBuffers[1] };
    
    fSampleRate = getSampleRate();
    
    signed int iBufferReadPos;
    
    while(numSamples--)
    {

        iBufferReadPos = iBufferWritePos - (fSampleRate * (1.0f / 4.0f)); // 0.25 seconds delay
        if (iBufferReadPos < 0) iBufferReadPos += iBufferSize; // Wrap around if necessary

        for (int ch = 0; ch < 2; ch++)
        {
            // Get sample from input
            fIn[ch] = *pfInBuffer[ch]++;

			// Add your effect processing here
			pfCircularBuffer[ch][iBufferWritePos] = fIn[ch]; // Write the input sample to the circular buffer

			float fDelSig = pfCircularBuffer[ch][iBufferReadPos]; // Read the delayed sample from the circular buffer
			fDelSig *= 0.2f; // Scale the delayed signal to 20% of its original amplitude
            fWet[ch] = fIn[ch] + fDelSig;

            fOut[ch] = fWet[ch];


            // Copy result to output
            *pfOutBuffer[ch]++ = fOut[ch];
        }

        iBufferWritePos++; // Increment the write position
        if (iBufferWritePos >= iBufferSize) iBufferWritePos = 0; // Wrap around if necessary. Reset to 0 if the write position exceeds the buffer size
    }
}
