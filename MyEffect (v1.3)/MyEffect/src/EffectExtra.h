//
//  EffectExtra.h
//  Additional Plugin Code
//
//  This file is a workspace for developing new DSP objects or functions to use in your plugin.
//


class MyFilter
{
public:
	void set(float coeff)
	{
		// Initialise your filter variables here
		fCurrentACoeff = coeff;
		fPreviousBCoeff = 1.0f - fCurrentACoeff;
	}

	float getCutoff(float sampleRate)
	{
		float fOutput = acos(1 - (pow(fCurrentACoeff, 2.0f / (2.0f * fPreviousBCoeff))) * (sampleRate / (2.0f * M_PI))); // Calculate cutoff frequency based on fCurrentACoeff and fPreviousBCoeff
		printf("Cutoff: %f\n", fOutput);
		return fOutput;
	}

	float process(float input)
	{
		// Filter individual samples here - 𝑦0 = 𝑎𝑥0 + 𝑏𝑦-1
		// Y is the output, X is the input, a is the fCurrentACoeff, b is the fPreviousBCoeff, and y-1 is the previous output sample
		float fOutput = (input * fCurrentACoeff) + (fPreviousOutput * fPreviousBCoeff);
		fPreviousOutput = fOutput;  // Store for next sample
		return fOutput;
	}

private:
	// Declare your internal filter variables here

	float fCurrentACoeff;
	float fPreviousBCoeff = 0.0f;
	float fPreviousOutput = 0.0f;  // Stores y-1
};