//
//  EffectExtra.h
//  Additional Plugin Code
//
//  This file is a workspace for developing new DSP objects or functions to use in your plugin.
//

#include <cmath>

class MyFilters
{
public:
	void setupFilters(float fLpfCutoff, float fSampleRate, float fBpfFrequency, float fBpfQ, float fHpfCutoff)
	{
		for (int ch = 0; ch < 2; ch++)
		{
			LPF[ch].set(fLpfCutoff, fSampleRate);
			BPF[ch].set(fBpfFrequency, fBpfQ, fSampleRate);
			HPF[ch].set(fHpfCutoff, fSampleRate);
		}
	}

	void processFilters(float fLpfOnOff, float fInput, int ch, float fLpfGain, float fBpfOnOff, float fBpfGain, float fHpfOnOff, float fHpfGain, float& fWet)
	{
		float fLpfBand = 0.0f;
		float fBpfBand = 0.0f;
		float fHpfBand = 0.0f;
		int iBandCount = 0;

		if (fLpfOnOff > 0.5f)
		{
			fLpfBand = fInput;
			LPF[ch].process(fLpfBand);
			fLpfBand *= fLpfGain;
			iBandCount++;
		}
		if (fBpfOnOff > 0.5f)
		{
			fBpfBand = fInput;
			BPF[ch].process(fBpfBand);
			fBpfBand *= fBpfGain;
			iBandCount++;
		}
		if (fHpfOnOff > 0.5f)
		{
			fHpfBand = fInput;
			HPF[ch].process(fHpfBand);
			fHpfBand *= fHpfGain;
			iBandCount++;
		}

		fWet = fLpfBand + fBpfBand + fHpfBand;
		if (iBandCount > 0) fWet /= (float)iBandCount;
		else fWet = fInput;
	}

	class MyLowPassFilter
	{
	public:

		class MyIirFilter
		{
		public:
			void set(float fCutoff, float sampleRate)
			{
				// Map normalised cutoff parameter to a musical frequency range.
				float fFrequency = mapFrequency(fCutoff, sampleRate);
				float fQ = 0.7071f;
				float fOmega = (2.0f * 3.14159265359f * fFrequency) / sampleRate;
				float fSinOmega = sinf(fOmega);
				float fCosOmega = cosf(fOmega);
				float fAlpha = fSinOmega / (2.0f * fQ);

				float b0 = (1.0f - fCosOmega) * 0.5f;
				float b1 = 1.0f - fCosOmega;
				float b2 = (1.0f - fCosOmega) * 0.5f;
				float a0 = 1.0f + fAlpha;
				float a1 = -2.0f * fCosOmega;
				float a2 = 1.0f - fAlpha;

				setTargets(b0, b1, b2, a0, a1, a2);
			}

			float process(float input)
			{
				// Smooth coefficients to avoid zipper noise when parameters move.
				smoothCoeffs();

				// Process filter sample here using the biquad difference equation.
				float fOutput = (fCurrentB0 * input) + (fCurrentB1 * fPreviousInput1) + (fCurrentB2 * fPreviousInput2)
					- (fCurrentA1 * fPreviousOutput1) - (fCurrentA2 * fPreviousOutput2);
				fPreviousInput2 = fPreviousInput1;
				fPreviousInput1 = input;
				fPreviousOutput2 = fPreviousOutput1;
				fPreviousOutput1 = fOutput;
				return fOutput;
			}

		private:
			// Declare your internal firstOrderFilter variables here

			float mapFrequency(float fCutoff, float sampleRate)
			{
				if (fCutoff < 0.0f) fCutoff = 0.0f;
				if (fCutoff > 1.0f) fCutoff = 1.0f;
				float fMinHz = 20.0f;
				float fMaxHz = sampleRate * 0.45f;
				if (fMaxHz < fMinHz) fMaxHz = fMinHz;
				return fMinHz * powf(fMaxHz / fMinHz, fCutoff);
			}

			void setTargets(float b0, float b1, float b2, float a0, float a1, float a2)
			{
				fTargetB0 = b0 / a0;
				fTargetB1 = b1 / a0;
				fTargetB2 = b2 / a0;
				fTargetA1 = a1 / a0;
				fTargetA2 = a2 / a0;

				if (!bHasInitialised)
				{
					fCurrentB0 = fTargetB0;
					fCurrentB1 = fTargetB1;
					fCurrentB2 = fTargetB2;
					fCurrentA1 = fTargetA1;
					fCurrentA2 = fTargetA2;
					bHasInitialised = true;
				}
			}

			void smoothCoeffs()
			{
				fCurrentB0 += (fTargetB0 - fCurrentB0) * fCoeffSmoothing;
				fCurrentB1 += (fTargetB1 - fCurrentB1) * fCoeffSmoothing;
				fCurrentB2 += (fTargetB2 - fCurrentB2) * fCoeffSmoothing;
				fCurrentA1 += (fTargetA1 - fCurrentA1) * fCoeffSmoothing;
				fCurrentA2 += (fTargetA2 - fCurrentA2) * fCoeffSmoothing;
			}

			bool bHasInitialised = false;
			float fCoeffSmoothing = 0.0025f;

			float fCurrentB0 = 1.0f;
			float fCurrentB1 = 0.0f;
			float fCurrentB2 = 0.0f;
			float fCurrentA1 = 0.0f;
			float fCurrentA2 = 0.0f;

			float fTargetB0 = 1.0f;
			float fTargetB1 = 0.0f;
			float fTargetB2 = 0.0f;
			float fTargetA1 = 0.0f;
			float fTargetA2 = 0.0f;

			float fPreviousInput1 = 0.0f;
			float fPreviousInput2 = 0.0f;
			float fPreviousOutput1 = 0.0f;
			float fPreviousOutput2 = 0.0f;
		};

		class MyBiQuadFilter
		{
		public:

			void set(float fCutoff, float sampleRate)
			{
				firstOrderFilter.set(fCutoff, sampleRate);
				secondOrderFilter.set(fCutoff, sampleRate);
			}

			void process(float& fWet)
			{
				fWet = firstOrderFilter.process(fWet); // First low-pass biquad stage.
				fWet = secondOrderFilter.process(fWet); // Second low-pass biquad stage.
			}

		private:
			MyIirFilter firstOrderFilter;
			MyIirFilter secondOrderFilter;
		};

		class FilterStages
		{
		public:

			void set(float fCutoff, float sampleRate)
			{
				for (int i = 0; i < 2; ++i) biQuadFilter[i].set(fCutoff, sampleRate);
			}

			void process(float& fWet)
			{
				for (int i = 0; i < 2; ++i) biQuadFilter[i].process(fWet); // 2-stage low-pass cascade.
			}

		private:
			MyBiQuadFilter biQuadFilter[2];
		};
	};

	class MyHighPassFilter
	{
	public:

		class MyIirFilter
		{
		public:
			void set(float fCutoff, float sampleRate)
			{
				// Map normalised cutoff parameter to a musical frequency range.
				float fFrequency = mapFrequency(fCutoff, sampleRate);
				float fQ = 0.7071f;
				float fOmega = (2.0f * 3.14159265359f * fFrequency) / sampleRate;
				float fSinOmega = sinf(fOmega);
				float fCosOmega = cosf(fOmega);
				float fAlpha = fSinOmega / (2.0f * fQ);

				float b0 = (1.0f + fCosOmega) * 0.5f;
				float b1 = -(1.0f + fCosOmega);
				float b2 = (1.0f + fCosOmega) * 0.5f;
				float a0 = 1.0f + fAlpha;
				float a1 = -2.0f * fCosOmega;
				float a2 = 1.0f - fAlpha;

				setTargets(b0, b1, b2, a0, a1, a2);
			}

			float process(float input)
			{
				// Smooth coefficients to avoid zipper noise when parameters move.
				smoothCoeffs();

				// Process filter sample here using the biquad difference equation.
				float fOutput = (fCurrentB0 * input) + (fCurrentB1 * fPreviousInput1) + (fCurrentB2 * fPreviousInput2)
					- (fCurrentA1 * fPreviousOutput1) - (fCurrentA2 * fPreviousOutput2);
				fPreviousInput2 = fPreviousInput1;
				fPreviousInput1 = input;
				fPreviousOutput2 = fPreviousOutput1;
				fPreviousOutput1 = fOutput;
				return fOutput;
			}

		private:
			// Declare your internal firstOrderFilter variables here

			float mapFrequency(float fCutoff, float sampleRate)
			{
				if (fCutoff < 0.0f) fCutoff = 0.0f;
				if (fCutoff > 1.0f) fCutoff = 1.0f;
				float fMinHz = 20.0f;
				float fMaxHz = sampleRate * 0.45f;
				if (fMaxHz < fMinHz) fMaxHz = fMinHz;
				return fMinHz * powf(fMaxHz / fMinHz, fCutoff);
			}

			void setTargets(float b0, float b1, float b2, float a0, float a1, float a2)
			{
				fTargetB0 = b0 / a0;
				fTargetB1 = b1 / a0;
				fTargetB2 = b2 / a0;
				fTargetA1 = a1 / a0;
				fTargetA2 = a2 / a0;

				if (!bHasInitialised)
				{
					fCurrentB0 = fTargetB0;
					fCurrentB1 = fTargetB1;
					fCurrentB2 = fTargetB2;
					fCurrentA1 = fTargetA1;
					fCurrentA2 = fTargetA2;
					bHasInitialised = true;
				}
			}

			void smoothCoeffs()
			{
				fCurrentB0 += (fTargetB0 - fCurrentB0) * fCoeffSmoothing;
				fCurrentB1 += (fTargetB1 - fCurrentB1) * fCoeffSmoothing;
				fCurrentB2 += (fTargetB2 - fCurrentB2) * fCoeffSmoothing;
				fCurrentA1 += (fTargetA1 - fCurrentA1) * fCoeffSmoothing;
				fCurrentA2 += (fTargetA2 - fCurrentA2) * fCoeffSmoothing;
			}

			bool bHasInitialised = false;
			float fCoeffSmoothing = 0.0025f;

			float fCurrentB0 = 1.0f;
			float fCurrentB1 = 0.0f;
			float fCurrentB2 = 0.0f;
			float fCurrentA1 = 0.0f;
			float fCurrentA2 = 0.0f;

			float fTargetB0 = 1.0f;
			float fTargetB1 = 0.0f;
			float fTargetB2 = 0.0f;
			float fTargetA1 = 0.0f;
			float fTargetA2 = 0.0f;

			float fPreviousInput1 = 0.0f;
			float fPreviousInput2 = 0.0f;
			float fPreviousOutput1 = 0.0f;
			float fPreviousOutput2 = 0.0f;
		};

		class MyBiQuadFilter
		{
		public:

			void set(float fCutoff, float sampleRate)
			{
				firstOrderFilter.set(fCutoff, sampleRate);
				secondOrderFilter.set(fCutoff, sampleRate);
			}

			void process(float& fWet)
			{
				fWet = firstOrderFilter.process(fWet); // First high-pass biquad stage.
				fWet = secondOrderFilter.process(fWet); // Second high-pass biquad stage.
			}

		private:
			MyIirFilter firstOrderFilter;
			MyIirFilter secondOrderFilter;
		};

		class FilterStages
		{
		public:

			void set(float fCutoff, float sampleRate)
			{
				for (int i = 0; i < 2; ++i) biQuadFilter[i].set(fCutoff, sampleRate);
			}

			void process(float& fWet)
			{
				for (int i = 0; i < 2; ++i) biQuadFilter[i].process(fWet); // 2-stage high-pass cascade.
			}

		private:
			MyBiQuadFilter biQuadFilter[2];
		};
	};

	class MyBandPassFilter
	{
	public:

		class MyIirFilter
		{
		public:
			void set(float fFrequency, float fQ, float sampleRate)
			{
				// Map normalised frequency and Q parameters to musical ranges.
				float fFrequencyHz = mapFrequency(fFrequency, sampleRate);
				float fQValue = mapQ(fQ);
				float fOmega = (2.0f * 3.14159265359f * fFrequencyHz) / sampleRate;
				float fSinOmega = sinf(fOmega);
				float fCosOmega = cosf(fOmega);
				float fAlpha = fSinOmega / (2.0f * fQValue);

				float b0 = fAlpha;
				float b1 = 0.0f;
				float b2 = -fAlpha;
				float a0 = 1.0f + fAlpha;
				float a1 = -2.0f * fCosOmega;
				float a2 = 1.0f - fAlpha;

				setTargets(b0, b1, b2, a0, a1, a2);
			}

			float process(float input)
			{
				// Smooth coefficients to avoid zipper noise when parameters move.
				smoothCoeffs();

				// Process filter sample here using the biquad difference equation.
				float fOutput = (fCurrentB0 * input) + (fCurrentB1 * fPreviousInput1) + (fCurrentB2 * fPreviousInput2)
					- (fCurrentA1 * fPreviousOutput1) - (fCurrentA2 * fPreviousOutput2);
				fPreviousInput2 = fPreviousInput1;
				fPreviousInput1 = input;
				fPreviousOutput2 = fPreviousOutput1;
				fPreviousOutput1 = fOutput;
				return fOutput;
			}

		private:
			// Declare your internal firstOrderFilter variables here

			float mapFrequency(float fFrequency, float sampleRate)
			{
				if (fFrequency < 0.0f) fFrequency = 0.0f;
				if (fFrequency > 1.0f) fFrequency = 1.0f;
				float fMinHz = 20.0f;
				float fMaxHz = sampleRate * 0.45f;
				if (fMaxHz < fMinHz) fMaxHz = fMinHz;
				return fMinHz * powf(fMaxHz / fMinHz, fFrequency);
			}

			float mapQ(float fQ)
			{
				if (fQ < 0.0f) fQ = 0.0f;
				if (fQ > 1.0f) fQ = 1.0f;
				return 0.3f + (fQ * (12.0f - 0.3f));
			}

			void setTargets(float b0, float b1, float b2, float a0, float a1, float a2)
			{
				fTargetB0 = b0 / a0;
				fTargetB1 = b1 / a0;
				fTargetB2 = b2 / a0;
				fTargetA1 = a1 / a0;
				fTargetA2 = a2 / a0;

				if (!bHasInitialised)
				{
					fCurrentB0 = fTargetB0;
					fCurrentB1 = fTargetB1;
					fCurrentB2 = fTargetB2;
					fCurrentA1 = fTargetA1;
					fCurrentA2 = fTargetA2;
					bHasInitialised = true;
				}
			}

			void smoothCoeffs()
			{
				fCurrentB0 += (fTargetB0 - fCurrentB0) * fCoeffSmoothing;
				fCurrentB1 += (fTargetB1 - fCurrentB1) * fCoeffSmoothing;
				fCurrentB2 += (fTargetB2 - fCurrentB2) * fCoeffSmoothing;
				fCurrentA1 += (fTargetA1 - fCurrentA1) * fCoeffSmoothing;
				fCurrentA2 += (fTargetA2 - fCurrentA2) * fCoeffSmoothing;
			}

			bool bHasInitialised = false;
			float fCoeffSmoothing = 0.0025f;

			float fCurrentB0 = 1.0f;
			float fCurrentB1 = 0.0f;
			float fCurrentB2 = 0.0f;
			float fCurrentA1 = 0.0f;
			float fCurrentA2 = 0.0f;

			float fTargetB0 = 1.0f;
			float fTargetB1 = 0.0f;
			float fTargetB2 = 0.0f;
			float fTargetA1 = 0.0f;
			float fTargetA2 = 0.0f;

			float fPreviousInput1 = 0.0f;
			float fPreviousInput2 = 0.0f;
			float fPreviousOutput1 = 0.0f;
			float fPreviousOutput2 = 0.0f;
		};

		class MyBiQuadFilter
		{
		public:

			void set(float fFrequency, float fQ, float sampleRate)
			{
				firstOrderFilter.set(fFrequency, fQ, sampleRate);
				secondOrderFilter.set(fFrequency, fQ, sampleRate);
			}

			void process(float& fWet)
			{
				fWet = firstOrderFilter.process(fWet); // First band-pass biquad stage.
				fWet = secondOrderFilter.process(fWet); // Second band-pass biquad stage.
			}

		private:
			MyIirFilter firstOrderFilter;
			MyIirFilter secondOrderFilter;
		};

		class FilterStages
		{
		public:

			void set(float fFrequency, float fQ, float sampleRate)
			{
				for (int i = 0; i < 2; ++i) biQuadFilter[i].set(fFrequency, fQ, sampleRate);
			}

			void process(float& fWet)
			{
				for (int i = 0; i < 2; ++i) biQuadFilter[i].process(fWet); // 2-stage band-pass cascade.
			}

		private:
			MyBiQuadFilter biQuadFilter[2];
		};
	};

	private:

	MyLowPassFilter::FilterStages LPF[2]; // Stereo lowpass filter stages (2 channels)
	MyBandPassFilter::FilterStages BPF[2]; // Stereo bandpass filter stages (2 channels)
	MyHighPassFilter::FilterStages HPF[2]; // Stereo highpass filter stages (2 channels)
};