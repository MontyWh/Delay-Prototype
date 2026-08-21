//
//  EffectExtra.h
//  Additional Plugin Code
//
//  This file is a workspace for developing new DSP objects or functions to use in your plugin.
//


class MyIirFilter
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

			float process(float output)
			{
				// Filter individual samples here - 𝑦0 = 𝑎𝑥0 + 𝑏𝑦-1
				float fFirst = (output * fCurrentACoeff) + (fPreviousOutputFirst * fPreviousBCoeff);
				fPreviousOutputFirst = fFirst;  // Store for next sample

				float fSecond = (fFirst * fCurrentACoeff) + (fPreviousOutputSecond * fPreviousBCoeff);
				fPreviousOutputSecond = fSecond;  // Store for next sample

				output = fSecond;

				return output;
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

			float process(float& fWet)
			{
				// Filter individual samples here - 𝑦0 = 𝑎𝑥0 + 𝑏𝑦-1
				float fLowPassFirst = (fWet * fCurrentACoeff) + (fPreviousOutputFirst * fPreviousBCoeff);
				fPreviousOutputFirst = fLowPassFirst;  // Store for next sample
				float fFirst = fWet - fLowPassFirst;

				float fLowPassSecond = (fFirst * fCurrentACoeff) + (fPreviousOutputSecond * fPreviousBCoeff);
				fPreviousOutputSecond = fLowPassSecond;  // Store for next sample

				fWet = fFirst - fLowPassSecond;
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
			void set(float fFrequency, float fQ)
			{
				// Calculate band edges from centre frequency and Q value
				if (fQ < 0.001f) fQ = 0.001f;
				float fBandwidth = fFrequency / fQ;
				float fLowCutoff = fFrequency - (fBandwidth * 0.5f);
				float fHighCutoff = fFrequency + (fBandwidth * 0.5f);

				if (fLowCutoff < 0.0f) fLowCutoff = 0.0f;
				if (fHighCutoff > 1.0f) fHighCutoff = 1.0f;
				if (fHighCutoff < fLowCutoff) fHighCutoff = fLowCutoff; // Ens

				for (int i = 0; i < 4; ++i)
				{
					HPF[i].set(fLowCutoff);
					LPF[i].set(fHighCutoff);
				}
			}

			float process(float& input)
			{
				for (int i = 0; i < 4; ++i)
				{
					HPF[i].process(input); // Apply high-pass filter
					LPF[i].process(input); // Then apply low-pass filter

					return input;
				}
			}

		private:
			MyHighPassFilter HPF[4];
			MyLowPassFilter LPF[4];
		};
	};
};

