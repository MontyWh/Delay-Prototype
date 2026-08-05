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

		previousBCoeff = 1 - currentACoeff;
	}

	float process(float input)
	{
		// Filter individual samples here
	}

private:
	// Declare your internal filter variables here

	float currentACoeff;
	float previousBCoeff;
};