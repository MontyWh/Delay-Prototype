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
		currentACoeff = coeff;
		previousBCoeff = 1 - currentACoeff;
	}

	float getCutoff(float sampleRate)
	{
		float fOutput = acos(1 - (pow(currentACoeff, 2 / (2 * previousBCoeff))) * (sampleRate / (2 * M_PI))); // Calculate cutoff frequency based on currentACoeff and previousBCoeff
		printf("Cutoff: %f\n", fOutput);
		return fOutput;
	}

	float process(float input)
	{
		// Filter individual samples here - 𝑦0 = 𝑎𝑥0 + 𝑏𝑦-1
		float fOutput = (input * currentACoeff) + (previousOutput * previousBCoeff);
		previousOutput = fOutput;  // Store for next sample
		return fOutput;
	}

private:
	// Declare your internal filter variables here

	float currentACoeff;
	float previousBCoeff = 0;
	float previousOutput = 0;  // Stores y-1
};