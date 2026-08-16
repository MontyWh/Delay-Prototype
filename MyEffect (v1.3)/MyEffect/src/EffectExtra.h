//
//  EffectExtra.h
//  Additional Plugin Code
//
//  This file is a workspace for developing new DSP objects or functions to use in your plugin.
//


class MyLowPassFilter
{
public:

	class MyIirFilter
	{
	public:
		void set(float coeff)
		{
			// Initialise your firstOrderFilter variables here
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
		// Declare your internal firstOrderFilter variables here

		float fCurrentACoeff;
		float fPreviousBCoeff = 0.0f;
		float fPreviousOutput = 0.0f;  // Stores y-1
	};

	class MyBiQuadFilter
	{
	public:

		void set(float fCutoff)
		{
			firstOrderFilter.set(fCutoff);
			secondOrderFilter.set(fCutoff);
		}

		void process(float& fWet)
		{
			fWet = firstOrderFilter.process(fWet); // shallow -6dB firstOrderFilter slope. Lets through significant frequency content above the cutoff.
			fWet = secondOrderFilter.process(fWet); // second order firstOrderFilter -12dB firstOrderFilter slope. Lets through less frequency content above the cutoff.
		}

	private:
		MyIirFilter firstOrderFilter;
		MyIirFilter secondOrderFilter;
	};

	class FilterStages
	{
	public:

		void set(float fCutoff)
		{
			for (int i = 0; i < 4; ++i) biQuadFilter[i].set(fCutoff);
		}

		void process(float& fWet)
		{
			for (int i = 0; i < 4; ++i) biQuadFilter[i].process(fWet); // shallow -6dB firstOrderFilter slope. Lets through significant frequency content above the cutoff.
		}

	private:
		MyBiQuadFilter biQuadFilter[4];
	};
};

class MyHighPassFilter
{
public:

	class MyIirFilter
	{
	public:
		void set(float coeff)
		{
			// Initialise your firstOrderFilter variables here
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
			fOutput = input - fOutput; // High-pass filter output is the input minus the low-pass filter output

			return fOutput;
		}

	private:
		// Declare your internal firstOrderFilter variables here

		float fCurrentACoeff;
		float fPreviousBCoeff = 0.0f;
		float fPreviousOutput = 0.0f;  // Stores y-1
	};

	class MyBiQuadFilter
	{
	public:

		void set(float fCutoff)
		{
			firstOrderFilter.set(fCutoff);
			secondOrderFilter.set(fCutoff);
		}

		void process(float& fWet)
		{
			fWet = firstOrderFilter.process(fWet); // shallow -6dB firstOrderFilter slope. Lets through significant frequency content above the cutoff.
			fWet = secondOrderFilter.process(fWet); // second order firstOrderFilter -12dB firstOrderFilter slope. Lets through less frequency content above the cutoff.
		}

	private:
		MyIirFilter firstOrderFilter;
		MyIirFilter secondOrderFilter;
	};

	class FilterStages
	{
	public:

		void set(float fCutoff)
		{
			for (int i = 0; i < 4; ++i) biQuadFilter[i].set(fCutoff);
		}

		void process(float& fWet)
		{
			for (int i = 0; i < 4; ++i) biQuadFilter[i].process(fWet); // shallow -6dB firstOrderFilter slope. Lets through significant frequency content above the cutoff.
		}

	private:
		MyBiQuadFilter biQuadFilter[4];
	};
};

class MyBandPassFilter
{
public:

	class MyBiQuadFilter
	{
	public:

		void set(float fFrequency, float fQ)
		{
			// Calculate band edges from centre frequency and Q value
			if (fQ < 0.001f) fQ = 0.001f;
			float fBandwidth = fFrequency / fQ;
			float fLowCutoff = fFrequency - (fBandwidth * 0.5f);
			float fHighCutoff = fFrequency + (fBandwidth * 0.5f);

			if (fLowCutoff < 0.0f) fLowCutoff = 0.0f;
			if (fHighCutoff > 1.0f) fHighCutoff = 1.0f;
			if (fHighCutoff < fLowCutoff) fHighCutoff = fLowCutoff;

			for (int i = 0; i < 4; ++i)
			{
				HPF[i].set(fLowCutoff);
				LPF[i].set(fHighCutoff);
			}
		}

		void process(float& fWet)
		{
			// High-pass first to remove low-frequency content below the band.
			for (int i = 0; i < 4; ++i) HPF[i].process(fWet);

			// Low-pass second to remove high-frequency content above the band.
			for (int i = 0; i < 4; ++i) LPF[i].process(fWet);
		}

	private:
		MyHighPassFilter::MyBiQuadFilter HPF[4];
		MyLowPassFilter::MyBiQuadFilter LPF[4];
	};

	class FilterStages
	{
	public:

		void set(float fFrequency, float fQ)
		{
			for (int i = 0; i < 4; ++i) biQuadFilter[i].set(fFrequency, fQ);
		}

		void process(float& fWet)
		{
			for (int i = 0; i < 4; ++i) biQuadFilter[i].process(fWet); // shallow -6dB firstOrderFilter slope. Lets through significant frequency content above the cutoff.
		}

	private:
		MyBiQuadFilter biQuadFilter[4];
	};
};