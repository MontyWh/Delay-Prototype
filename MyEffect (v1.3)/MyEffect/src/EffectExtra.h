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
			void set(float cutoff, float sampleRate)
			{
				// Map normalised cutoff parameter to a musical f range.
				float fMinHz = 20.0f;
				float fMaxHz = sampleRate * 0.45f;
				if (fMaxHz < fMinHz) fMaxHz = fMinHz;
				float fFrequency = fMinHz * powf(fMaxHz / fMinHz, cutoff);

				float fQ = 0.7071f;
				float fOmega = (2.0f * M_PI * fFrequency) / sampleRate;
				float fSinOmega = sinf(fOmega);
				float fCosOmega = cosf(fOmega);
				float fAlpha = fSinOmega / (2.0f * fQ);

				float fB0 = (1.0f - fCosOmega) * 0.5f;
				float fB1 = 1.0f - fCosOmega;
				float fB2 = (1.0f - fCosOmega) * 0.5f;
				float fA0 = 1.0f + fAlpha;
				float fA1 = -2.0f * fCosOmega;
				float fA2 = 1.0f - fAlpha;

				setCoeffTargets(fB0, fB1, fB2, fA0, fA1, fA2);
			}

			float process(float input)
			{
				// Smooth coefficients to avoid zipper noise when parameters move.
				smoothCoefficientTargets();

				// Process filter sample here using the biquad difference equation.
				float fOutput = (fCurrentB0Coeff * input) + (fCurrentB1Coeff * fPreviousInput1) + (fCurrentB2Coeff * fPreviousInput2)
					- (fCurrentA1Coeff * fPreviousOutput1) - (fCurrentA2Coeff * fPreviousOutput2);
				fPreviousInput2 = fPreviousInput1;
				fPreviousInput1 = input;
				fPreviousOutput2 = fPreviousOutput1;
				fPreviousOutput1 = fOutput;
				return fOutput;
			}

		private:
			// Declare your internal firstOrderFilter variables here

			void setCoeffTargets(float b0, float b1, float b2, float a0, float a1, float a2)
			{
				fTargetB0Coeff = b0 / a0;
				fTargetB1Coeff = b1 / a0;
				fTargetB2Coeff = b2 / a0;
				fTargetA1Coeff = a1 / a0;
				fTargetA2Coeff = a2 / a0;

				if (!bHasInitialisedCoeffs)
				{
					fCurrentB0Coeff = fTargetB0Coeff;
					fCurrentB1Coeff = fTargetB1Coeff;
					fCurrentB2Coeff = fTargetB2Coeff;
					fCurrentA1Coeff = fTargetA1Coeff;
					fCurrentA2Coeff = fTargetA2Coeff;
					bHasInitialisedCoeffs = true;
				}
			}

			void smoothCoefficientTargets()
			{
				fCurrentB0Coeff += (fTargetB0Coeff - fCurrentB0Coeff) * fCoeffSmoothing;
				fCurrentB1Coeff += (fTargetB1Coeff - fCurrentB1Coeff) * fCoeffSmoothing;
				fCurrentB2Coeff += (fTargetB2Coeff - fCurrentB2Coeff) * fCoeffSmoothing;
				fCurrentA1Coeff += (fTargetA1Coeff - fCurrentA1Coeff) * fCoeffSmoothing;
				fCurrentA2Coeff += (fTargetA2Coeff - fCurrentA2Coeff) * fCoeffSmoothing;
			}

			bool bHasInitialisedCoeffs = false;
			float fCoeffSmoothing = 0.0025f;

			float fCurrentB0Coeff = 1.0f;
			float fCurrentB1Coeff = 0.0f;
			float fCurrentB2Coeff = 0.0f;
			float fCurrentA1Coeff = 0.0f;
			float fCurrentA2Coeff = 0.0f;

			float fTargetB0Coeff = 1.0f;
			float fTargetB1Coeff = 0.0f;
			float fTargetB2Coeff = 0.0f;
			float fTargetA1Coeff = 0.0f;
			float fTargetA2Coeff = 0.0f;

			float fPreviousInput1 = 0.0f;
			float fPreviousInput2 = 0.0f;
			float fPreviousOutput1 = 0.0f;
			float fPreviousOutput2 = 0.0f;
		};

		class MyBiQuadFilter
		{
		public:

			void set(float cutoff, float sampleRate)
			{
				firstOrderFilter.set(cutoff, sampleRate);
				secondOrderFilter.set(cutoff, sampleRate);
			}

			void process(float& wet)
			{
				wet = firstOrderFilter.process(wet); // First low-pass biquad stage.
				wet = secondOrderFilter.process(wet); // Second low-pass biquad stage.
			}

		private:
			MyIirFilter firstOrderFilter;
			MyIirFilter secondOrderFilter;
		};

		class FilterStages
		{
		public:

			void set(float cutoff, float sampleRate)
			{
				for (int i = 0; i < 2; ++i) biQuadFilter[i].set(cutoff, sampleRate);
			}

			void process(float& wet)
			{
				for (int i = 0; i < 2; ++i) biQuadFilter[i].process(wet); // 2-stage low-pass cascade.
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
			void set(float cutoff, float sampleRate)
			{
				// Map normalised cutoff parameter to a musical f range.
				float fFrequency = mapFrequencyFromNormalised(cutoff, sampleRate);
				float fQ = 0.7071f;
				float fOmega = (2.0f * 3.14159265359f * fFrequency) / sampleRate;
				float fSinOmega = sinf(fOmega);
				float fCosOmega = cosf(fOmega);
				float fAlpha = fSinOmega / (2.0f * fQ);

				float fB0 = (1.0f + fCosOmega) * 0.5f;
				float fB1 = -(1.0f + fCosOmega);
				float fB2 = (1.0f + fCosOmega) * 0.5f;
				float fA0 = 1.0f + fAlpha;
				float fA1 = -2.0f * fCosOmega;
				float fA2 = 1.0f - fAlpha;

				setCoeffTargets(fB0, fB1, fB2, fA0, fA1, fA2);
			}

			float process(float input)
			{
				// Smooth coefficients to avoid zipper noise when parameters move.
				smoothCoefficientTargets();

				// Process filter sample here using the biquad difference equation.
				float fOutput = (fCurrentB0Coeff * input) + (fCurrentB1Coeff * fPreviousInput1) + (fCurrentB2Coeff * fPreviousInput2)
					- (fCurrentA1Coeff * fPreviousOutput1) - (fCurrentA2Coeff * fPreviousOutput2);
				fPreviousInput2 = fPreviousInput1;
				fPreviousInput1 = input;
				fPreviousOutput2 = fPreviousOutput1;
				fPreviousOutput1 = fOutput;
				return fOutput;
			}

		private:
			// Declare your internal firstOrderFilter variables here

			float mapFrequencyFromNormalised(float fCutoff, float fSampleRate)
			{
				if (fCutoff < 0.0f) fCutoff = 0.0f;
				if (fCutoff > 1.0f) fCutoff = 1.0f;
				float fMinHz = 20.0f;
				float fMaxHz = fSampleRate * 0.45f;
				if (fMaxHz < fMinHz) fMaxHz = fMinHz;
				return fMinHz * powf(fMaxHz / fMinHz, fCutoff);
			}

			void setCoeffTargets(float b0, float b1, float b2, float a0, float a1, float a2)
			{
				fTargetB0Coeff = b0 / a0;
				fTargetB1Coeff = b1 / a0;
				fTargetB2Coeff = b2 / a0;
				fTargetA1Coeff = a1 / a0;
				fTargetA2Coeff = a2 / a0;

				if (!bHasInitialisedCoeffs)
				{
					fCurrentB0Coeff = fTargetB0Coeff;
					fCurrentB1Coeff = fTargetB1Coeff;
					fCurrentB2Coeff = fTargetB2Coeff;
					fCurrentA1Coeff = fTargetA1Coeff;
					fCurrentA2Coeff = fTargetA2Coeff;
					bHasInitialisedCoeffs = true;
				}
			}

			void smoothCoefficientTargets()
			{
				fCurrentB0Coeff += (fTargetB0Coeff - fCurrentB0Coeff) * fCoeffSmoothing;
				fCurrentB1Coeff += (fTargetB1Coeff - fCurrentB1Coeff) * fCoeffSmoothing;
				fCurrentB2Coeff += (fTargetB2Coeff - fCurrentB2Coeff) * fCoeffSmoothing;
				fCurrentA1Coeff += (fTargetA1Coeff - fCurrentA1Coeff) * fCoeffSmoothing;
				fCurrentA2Coeff += (fTargetA2Coeff - fCurrentA2Coeff) * fCoeffSmoothing;
			}

			bool bHasInitialisedCoeffs = false;
			float fCoeffSmoothing = 0.0025f;

			float fCurrentB0Coeff = 1.0f;
			float fCurrentB1Coeff = 0.0f;
			float fCurrentB2Coeff = 0.0f;
			float fCurrentA1Coeff = 0.0f;
			float fCurrentA2Coeff = 0.0f;

			float fTargetB0Coeff = 1.0f;
			float fTargetB1Coeff = 0.0f;
			float fTargetB2Coeff = 0.0f;
			float fTargetA1Coeff = 0.0f;
			float fTargetA2Coeff = 0.0f;

			float fPreviousInput1 = 0.0f;
			float fPreviousInput2 = 0.0f;
			float fPreviousOutput1 = 0.0f;
			float fPreviousOutput2 = 0.0f;
		};

		class MyBiQuadFilter
		{
		public:

			void set(float cutoff, float sampleRate)
			{
				firstOrderFilter.set(cutoff, sampleRate);
				secondOrderFilter.set(cutoff, sampleRate);
			}

			void process(float& wet)
			{
				wet = firstOrderFilter.process(wet); // First high-pass biquad stage.
				wet = secondOrderFilter.process(wet); // Second high-pass biquad stage.
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
			void set(float f, float q, float sampleRate)
			{
				// Use f directly in Hz and map q parameter to musical range.
				float fFrequencyHz = clampFrequencyHz(f, sampleRate);
				float fQValue = mapQFromNormalised(q);
				float fOmega = (2.0f * 3.14159265359f * fFrequencyHz) / sampleRate;
				float fSinOmega = sinf(fOmega);
				float fCosOmega = cosf(fOmega);
				float fAlpha = fSinOmega / (2.0f * fQValue);

				float fB0 = fAlpha;
				float fB1 = 0.0f;
				float fB2 = -fAlpha;
				float fA0 = 1.0f + fAlpha;
				float fA1 = -2.0f * fCosOmega;
				float fA2 = 1.0f - fAlpha;

				setCoeffTargets(fB0, fB1, fB2, fA0, fA1, fA2);
			}

			float process(float input)
			{
				// Smooth coefficients to avoid zipper noise when parameters move.
				smoothCoefficientTargets();

				// Process filter sample here using the biquad difference equation.
				float fOutput = (fCurrentB0Coeff * input) + (fCurrentB1Coeff * fPreviousInput1) + (fCurrentB2Coeff * fPreviousInput2)
					- (fCurrentA1Coeff * fPreviousOutput1) - (fCurrentA2Coeff * fPreviousOutput2);
				fPreviousInput2 = fPreviousInput1;
				fPreviousInput1 = input;
				fPreviousOutput2 = fPreviousOutput1;
				fPreviousOutput1 = fOutput;
				return fOutput;
			}

		private:
			// Declare your internal firstOrderFilter variables here

			float clampFrequencyHz(float f, float sampleRate)
			{
				float fMinHz = 20.0f;
				float fMaxHz = sampleRate * 0.45f;
				if (fMaxHz < fMinHz) fMaxHz = fMinHz;
				if (f < fMinHz) f = fMinHz;
				if (f > fMaxHz) f = fMaxHz;
				return f;
			}

			float mapQFromNormalised(float q)
			{
				if (q < 0.0f) q = 0.0f;
				if (q > 1.0f) q = 1.0f;
				return 0.3f + (q * (12.0f - 0.3f));
			}

			void setCoeffTargets(float b0, float b1, float b2, float A0, float A1, float A2)
			{
				fTargetB0Coeff = b0 / A0;
				fTargetB1Coeff = b1 / A0;
				fTargetB2Coeff = b2 / A0;
				fTargetA1Coeff = A1 / A0;
				fTargetA2Coeff = A2 / A0;

				if (!bHasInitialisedCoeffs)
				{
					fCurrentB0Coeff = fTargetB0Coeff;
					fCurrentB1Coeff = fTargetB1Coeff;
					fCurrentB2Coeff = fTargetB2Coeff;
					fCurrentA1Coeff = fTargetA1Coeff;
					fCurrentA2Coeff = fTargetA2Coeff;
					bHasInitialisedCoeffs = true;
				}
			}

			void smoothCoefficientTargets()
			{
				fCurrentB0Coeff += (fTargetB0Coeff - fCurrentB0Coeff) * fCoeffSmoothing;
				fCurrentB1Coeff += (fTargetB1Coeff - fCurrentB1Coeff) * fCoeffSmoothing;
				fCurrentB2Coeff += (fTargetB2Coeff - fCurrentB2Coeff) * fCoeffSmoothing;
				fCurrentA1Coeff += (fTargetA1Coeff - fCurrentA1Coeff) * fCoeffSmoothing;
				fCurrentA2Coeff += (fTargetA2Coeff - fCurrentA2Coeff) * fCoeffSmoothing;
			}

			bool bHasInitialisedCoeffs = false;
			float fCoeffSmoothing = 0.0025f;

			float fCurrentB0Coeff = 1.0f;
			float fCurrentB1Coeff = 0.0f;
			float fCurrentB2Coeff = 0.0f;
			float fCurrentA1Coeff = 0.0f;
			float fCurrentA2Coeff = 0.0f;

			float fTargetB0Coeff = 1.0f;
			float fTargetB1Coeff = 0.0f;
			float fTargetB2Coeff = 0.0f;
			float fTargetA1Coeff = 0.0f;
			float fTargetA2Coeff = 0.0f;

			float fPreviousInput1 = 0.0f;
			float fPreviousInput2 = 0.0f;
			float fPreviousOutput1 = 0.0f;
			float fPreviousOutput2 = 0.0f;
		};

		class MyBiQuadFilter
		{
		public:

			void set(float f, float q, float sampleRate)
			{
				firstOrderFilter.set(f, q, sampleRate);
				secondOrderFilter.set(f, q, sampleRate);
			}

			void process(float& wet)
			{
				wet = firstOrderFilter.process(wet); // First band-pass biquad stage.
				wet = secondOrderFilter.process(wet); // Second band-pass biquad stage.
			}

		private:
			MyIirFilter firstOrderFilter;
			MyIirFilter secondOrderFilter;
		};

		class FilterStages
		{
		public:

			void set(float f, float q, float sampleRate)
			{
				for (int i = 0; i < 2; ++i) biQuadFilter[i].set(f, q, sampleRate);
			}

			void process(float& wet)
			{
				for (int i = 0; i < 2; ++i) biQuadFilter[i].process(wet); // 2-stage band-pass cascade.
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