//
//  EffectExtra.h
//  Additional Plugin Code
//
//  This file is a workspace for developing new DSP objects or functions to use in your plugin.
//


class MyFilterGainProcessor // Filter / shelf hybrid splitter - gain processor class
{
public:
	float filterShelfHybridSplitter(float input, float filtered, float gain)
	{
		// Calculate blend factor ONCE, before using it on each sample
		float fBlend;
		if (gain < 3.0f)
		{
			// 0.0 = full filter, 3.0 = unchanged (0 dB)
			fBlend = gain / 3.0f;
			return filtered * (1.0f - fBlend) + (input * fBlend);
		}
		else
		{
			// 3.0 = unchanged (0 dB), 6.0 = full shelf boost
			fBlend = (gain - 3.0f) / 3.0f;
			return input + (filtered * fBlend);
		}
	}
};

class MyIirFilter // A namespace for your Infinite Impulse Response (IIR) filter classes
{
public:

	class MyBiQuadFilter
	{
	public:

		class MyLowPassFilter
		{
		public:
			void reset()
			{
				fPreviousOutputFirst = fPreviousOutputSecond = 0.0f;
			}

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
				float fFirst = (input * fCurrentACoeff) + (fPreviousOutputFirst * fPreviousBCoeff);
				fPreviousOutputFirst = fFirst;  // Store for next sample

				float fSecond = (fFirst * fCurrentACoeff) + (fPreviousOutputSecond * fPreviousBCoeff);
				fPreviousOutputSecond = fSecond;  // Store for next sample

				return fSecond;
			}

		private:
			// Declare your internal filter stage variables here

			float fCurrentACoeff = 0.0f;
			float fPreviousBCoeff = 0.0f;
			float fPreviousOutputFirst = 0.0f;
			float fPreviousOutputSecond = 0.0f;
		};

		class MyHighPassFilter
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
				float fLowPassFirst = (input * fCurrentACoeff) + (fPreviousOutputFirst * fPreviousBCoeff);
				fPreviousOutputFirst = fLowPassFirst;  // Store for next sample
				float fFirst = input - fLowPassFirst;

				float fLowPassSecond = (fFirst * fCurrentACoeff) + (fPreviousOutputSecond * fPreviousBCoeff);
				fPreviousOutputSecond = fLowPassSecond;  // Store for next sample

				return fFirst - fLowPassSecond;
			}

		private:
			// Declare your internal filter stage variables here

			float fCurrentACoeff = 0.0f;
			float fPreviousBCoeff = 0.0f;
			float fPreviousOutputFirst = 0.0f;
			float fPreviousOutputSecond = 0.0f;
		};

		class MyBandPassFilter
		{
		public:
			void set(float frequency, float q)
			{
				float minQ = 0.1f;
				float maxQ = 100.0f;
				float mappedQ = minQ * pow(maxQ / minQ, q); // Map Q from 0.0-1.0 to minQ-maxQ range

				float bandwidth = frequency / mappedQ; // Calculate bandwidth based on frequency and Q

				// Calculate cutoff coefficients
				float fLowCutoff = frequency - (bandwidth * 0.5f);  // Below center
				float fHighCutoff = frequency + (bandwidth * 0.5f); // Above center

				// Clamp to valid coefficient range
				fLowCutoff = fmax(0.001f, fmin(fLowCutoff, 0.999f));
				fHighCutoff = fmax(0.001f, fmin(fHighCutoff, 0.999f));

				fTargetLowCutoff = fLowCutoff;
				fTargetHighCutoff = fHighCutoff;

				if (!bCutoffInitialised)
				{
					fCurrentLowCutoff = fTargetLowCutoff;
					fCurrentHighCutoff = fTargetHighCutoff;

					for (int i = 0; i < 4; ++i)
					{
						HPF[i].set(fCurrentLowCutoff);  // Remove frequencies below
						LPF[i].set(fCurrentHighCutoff); // Remove frequencies above
					}

					bCutoffInitialised = true;
				}
			}

			float process(float input)
			{
				fCurrentLowCutoff += (fTargetLowCutoff - fCurrentLowCutoff) * fCutoffSmoothingCoeff;
				fCurrentHighCutoff += (fTargetHighCutoff - fCurrentHighCutoff) * fCutoffSmoothingCoeff;

				for (int i = 0; i < 4; ++i)
				{
					HPF[i].set(fCurrentLowCutoff);
					LPF[i].set(fCurrentHighCutoff);
				}

				float output = input;
				for (int i = 0; i < 4; ++i)
				{
					output = HPF[i].process(output);
					output = LPF[i].process(output);
				}
				return output;
			}

		private:
			MyHighPassFilter HPF[4];
			MyLowPassFilter LPF[4];

			float fCurrentLowCutoff = 0.001f;
			float fCurrentHighCutoff = 0.999f;
			float fTargetLowCutoff = 0.001f;
			float fTargetHighCutoff = 0.999f;
			float fCutoffSmoothingCoeff = 0.0025f;
			bool bCutoffInitialised = false;
		};
	};
};

