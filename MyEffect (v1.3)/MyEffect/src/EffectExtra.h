//
//  EffectExtra.h
//  Additional Plugin Code
//
//  This file is a workspace for developing new DSP objects or functions to use in your plugin.
//

#pragma once

class MyNoiseGate
{
public:

	float noiseGate(float input, float threshold, float fReduction)
	{
		noiseGateFilter.tick(input);

		bool bMeterCounterCondition = false;


		// Define the input range (1 to 40)
		float fRangeMin = 1.0;
		float fRangeMax = 40.0;

		if (bMeterCounterCondition == true)
		{
			// test and cycle through fThresh
			fMeterCounter += 0.0000025; // count up
			threshold = fMeterCounter; // display the value
			if (fMeterCounter > 1) fMeterCounter = 0; // reset the value
		}

		float fAbsolute = fabsf(input);

		if (fPeak < fAbsolute) fPeak = fAbsolute; // did we see a louder peak?
		iMeasuredItems++;


		if (iMeasuredItems == iMeasuredLength)
		{
			//// Scale and offset the values of fMax0
			fPeak = fRangeMin + (fPeak * (fRangeMax - fRangeMin));

			// Use the log10() function to calculate log values, to give you values between log10(1) to log10(40), to then scale to the range of 0 to 1 by dividing by log10(4)
			fPeak = log10(fPeak) / log10(40);

			iMeasuredItems = fPeak = 0; // reset for next time
		}

		fGateTarget = (fPeak > threshold) ? 1 : fReduction; // should the gate open?
		if (fGateGain < fGateTarget) // the gate is opening - 'Attack'
		{
			fGateGain += 0.01;
			if (fGateGain > 1) fGateGain = 1;
		}
		if (fGateGain > fGateTarget) // the gate is closing - 'Release'
		{
			fGateGain -= 0.01;
			if (fGateGain < fReduction) fGateGain = fReduction;
		}

		return fGateGain;
	}

	LPF noiseGateFilter;

private:
	int iMeasuredLength = static_cast<int>(getSampleRate());
	int iMeasuredItems = 0;
	float fPeak = 0; // initially there is no peak value
	float fGateGain = 0; // initially the gate is closed
	float fGateTarget = 0; // the gate is opening/closing

	float fMeterCounter = 0;
};

class MyMultibandDistortion
{
public:
	MyMultibandDistortion()
	{
		subBass.setCutoff(60.0);
		bassUpper.setCutoff(250.0);
		bassLower.setCutoff(60.0);
		midUpper.setCutoff(2000.0);
		midLower.setCutoff(250.0);
		treble.setCutoff(2000.0);
	}

	class MyDistortionEffect
	{
	public:
		class MyDistortionTypes
		{
		public:
			// Function to rectify a float value
			float rectify(float value) {
				return (value < 0) ? -value : value;
			}

			float softClip(float input, float control)
			{
				float fOutput = (2 / M_PI) * atan(input * control);
				return fOutput;
			}

			float hardClip(float input, float control)
			{
				input *= control;
				if (input > 1) input = 1;
				else if (input < -1) input = -1;

				return input;
			}

			float quantisedDistortion(float input, float control)
			{
				// Adjust gain for MyDistortion based on threshold parameter
				float fAdjustedGain = std::pow(10, (control * 2.5) / 20);

				// Apply MyDistortion by scaling the input
				int iQuantised = static_cast<int>(std::round(input * fAdjustedGain));

				// Convert quantized integer back to floating-point
				float fOutput = iQuantised / fAdjustedGain;
				fOutput = hardClip(fOutput, 1.0f);

				// Return the distorted signal
				return fOutput;
			}

			float rectifiedDistortion(float input, float control)
			{

				float fOutput = rectify(input * control);

				return fOutput;
			}

			float foldingDistortion(float input, float control)
			{
				input *= control;

				while (input > 1.0f || input < -1.0f)
				{
					if (input > 1.0f) input = 2.0f - input;
					else if (input < -1.0f) input = -2.0f - input;
				}

				return input;
			}

			float asymmetricDistortion(float input, float control)
			{
				if (input < 0) input = softClip(input, control);
				else if (input > 0) input = hardClip(input, control);

				return input;
			}

			float parabolicDistortion(float input, float control)
			{
				if (input < 0) input = 0 - pow(hardClip(input, control), 2);
				else if (input > 0) input = pow(hardClip(input, control), 2);

				if (input < 0) input = input;
				else if (input > 0) input = input;
				else input = 0;

				return input;
			}

			float quarterCircleDistortion(float input, float control)
			{
				input *= control;
				float fRadius = 1.0f;
				float fInside = fRadius - pow(input, 2.0f);
				if (fInside < 0.0f) fInside = 0.0f;

				float fOutput = sqrt(fInside);

				return fOutput;
			}


			// Lo-Fi MyDistortion types

			float tangentDistortion(float input, float control)
			{
				// Use a non-linear function like tanh for soft clipping
				float fOutput = std::tanh(input * control);


				return fOutput;
			}

			float aliasingDistortion(float input, float control, int numOfSamples, float fSR)
			{
				// "Time-quantising" aka 'aliasing'

				input *= control;

				static int iSampleCounter = 0; // Keeps track of the sample count
				static float fLastOutput = 0.0; // Holds the last fOutput sample for aliasing effect

				// Calculate the downsampling factor based on the threshold parameter
				int iDownsampleFactor = 1 + static_cast<int>(pow(control, 2.0f) * 60.0f);

				// Downsample by only updating fOutput on every Nth sample, according to iDownsampleFactor
				if (iSampleCounter % iDownsampleFactor == 0) {
					fLastOutput = input;
				}

				// Increment sample counter and wrap it around to prevent overflow
				iSampleCounter++;
				if (iSampleCounter >= iDownsampleFactor) iSampleCounter = 0;

				return fLastOutput;
			}

			float phaseDistortion(float input, float control, int numOfSamples, float sampleRate)
			{
				// Apply phase Distortion by modifying the phase of the input
				float fPhaseShift = 20.0f + (control * 1980.0f); // Adjust the scaling factor as needed

				// Calculate the fPhase increment per sample
				float fPhaseIncrement = 2.0f * M_PI * fPhaseShift / sampleRate;

				static float fPhase = 0.0f;
				fPhase += fPhaseIncrement;
				if (fPhase > 2.0f * M_PI) fPhase -= 2.0f * M_PI;

				float fOutput = input * std::cos(fPhase);

				return fOutput;
			}

			float alterBitDepth(float input, float control)
			{
				// Number of bits to reduce by
				float fMaxDepth = 24 * (1 - std::pow(control, 1.0 / 4.0));

				// Calculate the altered sample value
				float fAlteredSample = round((input + 1.0) * fMaxDepth) / (fMaxDepth + 1.0);

				return fAlteredSample;
			}
		};

		float processDistortion(float input, int type, float control)
		{
			float fInputGain = 1.0 + (pow(control, 3.0) * (25.0 - 1.0));

			// Distortion types

			if (type == 1)
			{
				input = DistortionTypes.softClip(input, fInputGain);
			}

			else if (type == 2)
			{
				input = DistortionTypes.hardClip(input, fInputGain);
			}

			else if (type == 3)
			{
				input = DistortionTypes.quantisedDistortion(input, fInputGain);
			}

			else if (type == 4)
			{
				input = DistortionTypes.rectifiedDistortion(input, fInputGain);
			}

			else if (type == 5)
			{
				input = DistortionTypes.foldingDistortion(input, fInputGain);
			}

			else if (type == 6)
			{
				input = DistortionTypes.asymmetricDistortion(input, fInputGain);
			}

			else if (type == 7)
			{
				input = DistortionTypes.parabolicDistortion(input, fInputGain);
			}

			else if (type == 8)
			{
				input = DistortionTypes.quarterCircleDistortion(input, fInputGain);
			}

			return input;
		}

		MyDistortionTypes DistortionTypes;
	};

	float process(float input, float typeDistortionAmount[9], int tonalDistortionType[4])
	{
		float fBand[4];
		float fBandClean[4];
		float fTonalDistortion[4];

		// Initialise bands for this channel
		fBand[0] = subBass.tick(input);
		fBand[1] = (bassUpper.tick(input) + bassLower.tick(input));
		fBand[2] = (midUpper.tick(input) + midLower.tick(input));
		fBand[3] = treble.tick(input);

		// Process each band if required
		for (int j = 0; j <= 3; j++)
		{
			fBandClean[j] = fBand[j];

			int iCurrentType = tonalDistortionType[j];
			if (iCurrentType < 0) iCurrentType = 0;
			else if (iCurrentType > 8) iCurrentType = 8;

			float fCurrentDistortionAmount = typeDistortionAmount[iCurrentType];
			float fDistortedBand = DistortionEffect.processDistortion(fBand[j], iCurrentType, fCurrentDistortionAmount);

			fTonalDistortion[j] = (fDistortedBand * fCurrentDistortionAmount) + ((1.0f - fCurrentDistortionAmount) * fBandClean[j]);
		}

		// Sum the processed bands
		input = fTonalDistortion[0] + fTonalDistortion[1] + fTonalDistortion[2] + fTonalDistortion[3];

		return input;
	}

private:
	LPF subBass;
	LPF bassLower;
	HPF bassUpper;
	LPF midLower;
	HPF midUpper;
	HPF treble;

	MyDistortionEffect DistortionEffect;
};

class MySaturation
{
public:
	float vinylCrackle(float input, int numOfSamples, float loFiBlend, int counter, float sampleRate)
	{
		int randomNumber = rand() % 200 + 1;     // in the range 1 to fRangeMin

		float fInputDip = input * 0.5;

		if (numOfSamples == randomNumber)
		{
			if (randomNumber == 0)
				input = 0 + (randomNumber / 100);

			else if (randomNumber == 1)
				input = DistortionTypes.hardClip(input * (1.0 + loFiBlend), 1.0);

			else if (randomNumber == 2)
			{
				input = DistortionTypes.alterBitDepth(fInputDip, loFiBlend);
			}
			else if (randomNumber == 3)
			{
				input = DistortionTypes.aliasingDistortion(fInputDip, loFiBlend, numOfSamples, sampleRate);
			}
			else if (randomNumber == 4)
			{
				input = DistortionTypes.phaseDistortion(fInputDip, loFiBlend, numOfSamples, sampleRate);
			}
		}
		input = DistortionTypes.alterBitDepth(input, 0.0625 * loFiBlend);

		return input;
	}


	float loFiEffects(float input, int type, float loFiBlend, int numOfSamples, float sampleRate)
	{
		float output = input;
		if (type == 1)
		{
			output = DistortionTypes.tangentDistortion(input, loFiBlend);
		}
		else if (type == 2)
		{
			output = DistortionTypes.aliasingDistortion(input, loFiBlend, numOfSamples, sampleRate);
		}
		else if (type == 3)
		{
			output = DistortionTypes.phaseDistortion(input, loFiBlend, numOfSamples, sampleRate);
		}
		else if (type == 4)
		{
			output = DistortionTypes.alterBitDepth(input, loFiBlend);
		}
		else if (type == 5)
		{
			output = vinylCrackle(input, numOfSamples, loFiBlend, vinylCounter, sampleRate);
		}

		output = (output * loFiBlend) + ((1.0f - loFiBlend) * input);

		return output;
	}

	float vinylCounter = 0;
	MyMultibandDistortion::MyDistortionEffect::MyDistortionTypes DistortionTypes;
};

class MyModulator
{
public:
	MyModulator()
	{

	}

	~MyModulator()
	{
		destroy();
	}

	void initialise(float sampleRate)
	{
		destroy();
		fSampleRate = sampleRate;
		iBufferSize = static_cast<int>(sampleRate * 2.0f);
		if (iBufferSize < 1)
			iBufferSize = 1;

		pfCircularBuffer = new float[iBufferSize];
		for (int i = 0; i < iBufferSize; ++i)
			pfCircularBuffer[i] = 0.0f;

		iBufferWritePos = 0;
		iBufferReadPos = 0;
		fDel = 0.0f;
		fPhasePos = 0.0f;
		fPhaseInc = 0.0f;
	}

	void setupValues(float rate)
	{
		if (fSampleRate <= 0.0f)
			return;

		fPhaseInc = (2.0f * static_cast<float>(M_PI) * rate) / fSampleRate;
	}

	void destroy()
	{
		delete[] pfCircularBuffer;
		pfCircularBuffer = nullptr;
		fSampleRate = 0.0f;
		iBufferWritePos = 0;
		iBufferReadPos = 0;
		iBufferSize = 0;
		fDel = 0.0f;
		fPhasePos = 0.0f;
		fPhaseInc = 0.0f;
	}

	float process(float input, float depth, float wet, float outputGain, int waveType)
	{
		if (!pfCircularBuffer || iBufferSize <= 1) // Check if the circular buffer is valid
			return input;

		pfCircularBuffer[iBufferWritePos] = input;

		const int iFixedDelSamples = static_cast<int>(0.25f * fSampleRate); // 0.25 seconds delay
		iBufferReadPos = iBufferWritePos - iFixedDelSamples; // Read position is behind write position by fixed delay

		while (iBufferReadPos < 0)
			iBufferReadPos += iBufferSize;
		while (iBufferReadPos >= iBufferSize)
			iBufferReadPos -= iBufferSize;
		fDel = pfCircularBuffer[iBufferReadPos];

		float fMod = 0.0f;
		switch (waveType)
		{
			case 0: fMod = WaveTypes::generateSine(fPhasePos); break;
			case 1: fMod = WaveTypes::generateTriangle(fPhasePos); break;
			case 2: fMod = WaveTypes::generateSawtooth(fPhasePos); break;
			case 3: fMod = WaveTypes::generatePulse(fPhasePos, 0.5f); break;
			case 4: fMod = WaveTypes::generateSquare(fPhasePos); break;
			default: fMod = WaveTypes::generateSine(fPhasePos); break;
		}
		const float fOsc = ((fMod + 1.0f) * 0.5f) * depth;
		const float fTremGain = (1.0f - depth) + fOsc;
		const float fWet = input * fTremGain;

		return ((1.0f - wet) * input + (wet * fWet)) * outputGain;
	}

	float processOffset(float depth, int waveType)
	{
		if (!pfCircularBuffer || iBufferSize <= 1) // Check if the circular buffer is valid
			return 0.0f;

		float fMod = 0.0f;
		switch (waveType)
		{
			case 0: fMod = WaveTypes::generateSine(fPhasePos); break;
			case 1: fMod = WaveTypes::generateTriangle(fPhasePos); break;
			case 2: fMod = WaveTypes::generateSawtooth(fPhasePos); break;
			case 3: fMod = WaveTypes::generatePulse(fPhasePos, 0.5f); break;
			case 4: fMod = WaveTypes::generateSquare(fPhasePos); break;
			default: fMod = WaveTypes::generateSine(fPhasePos); break;
		}

		return depth * fMod;
	}

	void postProcess()
	{
		if (!pfCircularBuffer || iBufferSize <= 1) // Check if the circular buffer is valid
			return;

		iBufferWritePos++;
		if (iBufferWritePos >= iBufferSize)
			iBufferWritePos = 0;

		fPhasePos += fPhaseInc;
		if (fPhasePos >= (M_PI * 2.0f))
			fPhasePos -= (M_PI * 2.0f);
	}

	class WaveTypes
	{
	public:
		// Waveform generation functions
		// All functions output in range [-1, 1]
		static float generateSine(float phase)
		{
			return std::sin(phase);
		}

		static float generateTriangle(float phase)
		{
			// Convert phase to triangle wave
			const float fTwoPI = 2.0f * M_PI;
			float fNormalised = phase / fTwoPI; // 0 to 1
			if (fNormalised < 0.25f)
				return fNormalised * 4.0f; // 0 to 1
			else if (fNormalised < 0.75f)
				return 1.0f - ((fNormalised - 0.25f) * 4.0f); // 1 to -1
			else
				return -1.0f + ((fNormalised - 0.75f) * 4.0f); // -1 to 0
		}

		static float generateSawtooth(float phase)
		{
			const float fTwoPI = 2.0f * M_PI;
			return (phase / fTwoPI) * 2.0f - 1.0f;
		}

		static float generatePulse(float phase, float pulseWidth)
		{
			const float fTwoPI = 2.0f * M_PI;
			float fNormalised = phase / fTwoPI; // 0 to 1
			return (fNormalised < pulseWidth) ? 1.0f : -1.0f;
		}

		static float generateSquare(float phase)
		{
			return generatePulse(phase, 0.5f);
		}

		// Generate waveform based on type index (0-4)
		static float generateWaveform(int waveType, float phase, float pulseWidth = 0.5f)
		{
			switch (waveType)
			{
			case 0: return generateSine(phase);
			case 1: return generateTriangle(phase);
			case 2: return generateSawtooth(phase);
			case 3: return generatePulse(phase, pulseWidth);
			case 4: return generateSquare(phase);
			default: return generateSine(phase);
			}
		}
	};

private:
	float fSampleRate = 0.0f;
	float fPhasePos = 0.0f;
	float fPhaseInc = 0.0f;
	float* pfCircularBuffer = nullptr;
	int iBufferWritePos = 0;
	int iBufferReadPos = 0;
	int iBufferSize = 0;
	float fDel = 0.0f;
};

class MyFilters
{
public:

	class MyFilterGainProcessor // Filter / shelf hybrid splitter - gain processor class
	{
	public:
		float filterShelfHybridSplitter(float input, float filtered, float gain)
		{
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
					// Filter individual tapCount here - 𝑦0 = 𝑎𝑥0 + 𝑏𝑦-1
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
					// Filter individual tapCount here - 𝑦0 = 𝑎𝑥0 + 𝑏𝑦-1
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
};

class MyEcho
{
public:

	MyEcho() {}

	void initialise(float sampleRate)
	{
		Delay.initialiseBuffer(sampleRate);
		Reverb.initialiseBuffer(sampleRate);
	}

	void setupParameters(float* fDelayEffectTimes, float fReverbPatterns[][4], float fFeedbackGain, float fLpfCutoff, float fDrive, float fDiffusion, int iNumberOfDelays)
	{
		Delay.set(fDelayEffectTimes, fFeedbackGain, fLpfCutoff, fDrive, iNumberOfDelays);
		Reverb.set(fReverbPatterns, fFeedbackGain, fLpfCutoff, fDrive, fDiffusion, iNumberOfDelays);
	}

	float process(float input, float sampleRate, int bypassDelay, int bypassDelayMod, int bypassReverb, float modRate, float modDepth, float modDelayTime, float drive)
	{
		float fDelay = 0.0f;
		float fReverb = 0.0f;
		if (bypassDelay == 0) fDelay = Delay.process(input, sampleRate, bypassDelayMod, modRate, modDepth, modDelayTime, drive);
		if (bypassReverb == 0) fReverb = Reverb.process(input + fDelay, sampleRate, drive) * 0.75f;

		if (bypassDelay == 1 && bypassReverb == 1) return input;
		return fDelay + fReverb;
	}

	void postProcess()
	{
		Delay.tapTempoPost();
		Delay.postProcess();
		Reverb.postProcess();
	}

	class MyMultiLineDelay
	{
	public:

		MyMultiLineDelay()
		{
			fFeedbackGain = 0.0f;
			fDrive = 1.0f;
			fDriveEnvelope = 0.0f;
			fDriveDecayPerSample = 1.0f;
			iNumberOfDelays = 1;
		}

		void set(float *delayTimes, float feedbackGain, float lpfCutoff, float drive, int numDelays)
		{
			iNumberOfDelays = numDelays;
			for (int i = 0; i < iNumberOfDelays; i++)
			{
				MultipleDelays[i].set(delayTimes[i]);
				MultipleDelays[i].setLowPassCutoff(lpfCutoff);
			}
			fFeedbackGain = feedbackGain;
			fDrive = drive;
		}

		void setTapTempo(float sampleRate)
		{
			if (MultipleDelays[0].setTapTempo(sampleRate))
			{
				float fTapTime = MultipleDelays[0].fDelayTime;
				for (int i = 1; i < iNumberOfDelays; i++) MultipleDelays[i].setTappedDelayTime(fTapTime * pow(0.5f, (float)i));
			}
		}

		void initialiseBuffer(float sampleRate)
		{
			for (int i = 0; i < 3; i++) MultipleDelays[i].initialiseBuffer(sampleRate);

			fDriveEnvelope = 0.0f;
			fDriveDecayPerSample = (sampleRate > 0.0f) ? pow(0.001f, 1.0f / (sampleRate * 1.5f)) : 1.0f; // Decay over 1.5 seconds
		}

		float process(float input, float sampleRate, int bypassDelayMod, float modRate, float modDepth, float modDelayTime, float drive)
		{
			fDriveEnvelope *= fDriveDecayPerSample;
			if (fabsf(input) > 0.001f && fDriveEnvelope < 1.0f) fDriveEnvelope = 1.0f;
			float fDriveOverTime = 1.0f + ((drive - 1.0f) * fDriveEnvelope);

			float fSummedFilteredTaps = 0.0f;
			for (int i = 0; i < iNumberOfDelays; i++)
			{
				fSummedFilteredTaps += MultipleDelays[i].read(sampleRate, bypassDelayMod, modRate, modDepth, modDelayTime, fDriveOverTime); // Sum the outputs of all delay taps
			}

			float fFeedbackValue = fSummedFilteredTaps * fFeedbackGain;
			for (int i = 0; i < iNumberOfDelays; i++) MultipleDelays[i].write(input, fFeedbackValue, fDriveOverTime); // Keep input clean, distort repeats in feedback

			return fSummedFilteredTaps;
		}

		void tapTempoPost()
		{
			for (int i = 0; i < iNumberOfDelays; i++) MultipleDelays[i].tapTempoPost();
		}

		void postProcess()
		{
			for (int i = 0; i < iNumberOfDelays; i++) MultipleDelays[i].postProcess();
		}

		class MyDelay
		{
		public:

			MyDelay()
			{
				pfCircularBuffer = nullptr;

				iBufferSize = 0;
				iBufferWritePos = 0;

				fDelayTime = 0.0f;
				fOutputDelayTime = 0.0f;

				iTapState[0] = 0;
				iTapState[1] = 0;
				iTapCount = 0;
				iTapTimingActive = 0;
				iTapGroupCount = 0;
				iTapInactivityCount = 0;
				fTapCountAccum = 0.0f;
				fTapResetSamples = 0.0f;
			}

			~MyDelay()
			{
				delete[] pfCircularBuffer;
			}

			void set(float delayTime)
			{
				float fDelayTimeDiff = delayTime - fDelayTime;
				if (fDelayTimeDiff < -0.000001f || fDelayTimeDiff > 0.000001f)
				{
					fDelayTime = delayTime;
				}
			}

			void initialiseBuffer(float sampleRate)
			{
				iBufferSize = sampleRate * 2; // 2 seconds of audio

				delete[] pfCircularBuffer;
				pfCircularBuffer = new float[iBufferSize]; // Allocate memory for the circular buffer

				for (int i = 0; i < iBufferSize; i++) pfCircularBuffer[i] = 0.0f; // Initialise the circular buffer to zero

				iBufferWritePos = 0; // Reset the write position to the start of the buffer
				fTapResetSamples = sampleRate * 3.0f;
				Mod.initialise(sampleRate);
			}

			bool setTapTempo(float sampleRate)
			{
				iTapState[0] = iTapState[1];
				iTapState[1] = 1;

				if (iTapState[0] == 0 && iTapState[1] == 1)
				{
					if (iTapTimingActive == 0)
					{
						iTapTimingActive = 1;
						iTapGroupCount = 1;
						iTapCount = 0;
						iTapInactivityCount = 0;
						fTapCountAccum = 0.0f;
						return false;
					}

					if (iTapCount > 0)
					{
						fTapCountAccum += (float)iTapCount;
						iTapGroupCount++;
						iTapInactivityCount = 0;

						float fTapCountAverage = fTapCountAccum / (float)(iTapGroupCount - 1);
						float fFrequency = samplesToTimeToFrequency((int)fTapCountAverage, sampleRate);
						iTapCount = 0;

						if (fFrequency > 0.0f)
						{
							setTappedDelayTime(1.0f / (fFrequency * fTapPulseMultiplier));
						}

						if (iTapGroupCount >= 4)
						{
							iTapGroupCount = 1;
							fTapCountAccum = 0.0f;
						}

						return (fFrequency > 0.0f);
					}
				}

				return false;
			}

			float read(float sampleRate, int bypassDelayMod, float modRate, float modDepth, float modDelayTime, float drive)
			{
				if (bypassDelayMod == 0)
				{
					Mod.setupValues(modRate);
					float fDelayOffset = Mod.processOffset(modDepth, 1); // Get the modulation offset for the delay time
					modDelayTime = modDelayTime + fDelayOffset;
					Mod.postProcess();
				}
				fOutputDelayTime = fDelayTime + modDelayTime;
				if (fOutputDelayTime < 0.001f) fOutputDelayTime = 0.001f;
				if (fOutputDelayTime > 2.0f) fOutputDelayTime = 2.0f;
				int readPos = iBufferWritePos - (int)(sampleRate * fOutputDelayTime); // Calculate the read position based on the delay time
				if (readPos < 0) readPos += iBufferSize;

				return Distortion.softClip(LPF.process(pfCircularBuffer[readPos]), drive);
			}

			void setTappedDelayTime(float delayTime)
			{
				fDelayTime = delayTime;
				if (fDelayTime < 0.001f) fDelayTime = 0.001f;
				if (fDelayTime > 2.0f) fDelayTime = 2.0f;
			}

			void setLowPassCutoff(float cutoff)
			{
				LPF.set(cutoff);
			}

			void write(float input, float feedback, float drive)
			{
				float fDistortedFeedback = Distortion.softClip(feedback, drive);
				pfCircularBuffer[iBufferWritePos] = input + fDistortedFeedback;
			}

			float samplesToTimeToFrequency(int tapCount, float sampleRate)
			{
				if (tapCount <= 0 || sampleRate <= 0.0f) return 0.0f;

				float fTime = ((float)tapCount / sampleRate) * fTapIntervalMultiplier;
				if (fTime <= 0.0f) return 0.0f;

				float fFrequency = 1.0f / fTime; // Convert time to frequency
				return fFrequency;
			}

			void tapTempoPost()
			{
				iTapState[0] = iTapState[1];
				iTapState[1] = 0;
				if (iTapTimingActive == 1)
				{
					iTapCount++;
					iTapInactivityCount++;
					if ((float)iTapInactivityCount >= fTapResetSamples)
					{
						iTapTimingActive = 0;
						iTapGroupCount = 0;
						iTapCount = 0;
						iTapInactivityCount = 0;
						fTapCountAccum = 0.0f;
					}
				}
			}

			void postProcess()
			{
				iBufferWritePos++; // Increment the write position
				if (iBufferWritePos >= iBufferSize) iBufferWritePos = 0; // Wrap around if necessary
			}

			int iBufferWritePos;
			int iBufferSize;
			float fDelayTime;

			float fOutputDelayTime;

		private:
			float* pfCircularBuffer;
			MyFilters::MyIirFilter::MyBiQuadFilter::MyLowPassFilter LPF;

			int iTapState[2];
			int iTapCount;
			int iTapTimingActive;
			int iTapGroupCount;
			int iTapInactivityCount;
			float fTapCountAccum;
			float fTapResetSamples;
			float fTapIntervalMultiplier = 1.0f;
			float fTapPulseMultiplier = 1.0f;

			MyModulator Mod;

			MyMultibandDistortion::MyDistortionEffect::MyDistortionTypes Distortion;
		};

		MyDelay MultipleDelays[3];

	private:
		int iNumberOfDelays;
		float fFeedbackGain;
		float fDrive;
		float fDriveEnvelope;
		float fDriveDecayPerSample;
	};

	class MyReverb
	{
	public:
		MyReverb()
		{
			for (int i = 0; i < 4; i++)
			{
				pfEchoBlockBuffers[i] = nullptr;
				iEchoBlockWritePos[i] = 0;
				iEchoBlockPreDelaySamples[i] = 1;
			}
		}

		~MyReverb()
		{
			for (int i = 0; i < 4; i++) delete[] pfEchoBlockBuffers[i];
		}

		class MyEchoBlock
		{
		public:

			void initialiseBuffer(float sampleRate)
			{
				for (int i = 0; i < 3; i++)
					for (int j = 0; j < 4; j++) Delays[i][j].initialiseBuffer(sampleRate);
			}

			void set(float delayTimes[][4], float feedbackGain, float lpfCutoff, float drive, int numDelays)
			{
				iNumberOfDelayGroups = numDelays;
				if (iNumberOfDelayGroups < 1) iNumberOfDelayGroups = 1;
				if (iNumberOfDelayGroups > 3) iNumberOfDelayGroups = 3;

				for (int i = 0; i < iNumberOfDelayGroups; i++)
				{
					for (int j = 0; j < 4; j++)
					{
						float fDelayLineTimes[3];
						for (int k = 0; k < 3; k++)
						{
							int iTapIndex = (j + k) % 4;
							fDelayLineTimes[k] = delayTimes[i][iTapIndex];
						}
						Delays[i][j].set(fDelayLineTimes, feedbackGain, lpfCutoff, drive, 3);
					}
				}
			}

			int tapPos(int delayIndex, float time, float sampleRate)
			{
				int iDelayGroup = delayIndex / 4;
				int iDelayGroupTarget = delayIndex % 4;
				int iBufferReadPos = Delays[iDelayGroup][iDelayGroupTarget].MultipleDelays[0].iBufferWritePos - (time * sampleRate); // Calculate the read position based on the delay time
				if (iBufferReadPos < 0) iBufferReadPos += Delays[iDelayGroup][iDelayGroupTarget].MultipleDelays[0].iBufferSize; // Wrap around if necessary
				return iBufferReadPos;
			}

			float process(float input, float sampleRate, float drive)
			{
				float fSummedTaps = 0.0f;
				for (int i = 0; i < iNumberOfDelayGroups; i++)
					for (int j = 0; j < 4; j++) fSummedTaps += Delays[i][j].process(input, sampleRate, 0, 0.0f, 0.0f, 0.0f, drive);
				return fSummedTaps * 0.25f;
			}

			void postProcess()
			{
				for (int i = 0; i < iNumberOfDelayGroups; i++)
					for (int j = 0; j < 4; j++) Delays[i][j].postProcess();
			}

			MyMultiLineDelay Delays[3][4];

		private:
			int iNumberOfDelayGroups = 0;
		};

		void initialiseBuffer(float sampleRate)
		{
			for (int i = 0; i < 4; i++) EchoBlocks[i].initialiseBuffer(sampleRate);

			iEchoBlockBufferSize = (int)(sampleRate * 2.0f);
			if (iEchoBlockBufferSize < 1) iEchoBlockBufferSize = 1;

			for (int i = 0; i < 4; i++)
			{
				delete[] pfEchoBlockBuffers[i];
				pfEchoBlockBuffers[i] = new float[iEchoBlockBufferSize];
				for (int j = 0; j < iEchoBlockBufferSize; j++) pfEchoBlockBuffers[i][j] = 0.0f;

				iEchoBlockWritePos[i] = 0;
				iEchoBlockPreDelaySamples[i] = (int)(sampleRate * (0.004f * (float)(i + 1))); // 4,8,12,16ms
				if (iEchoBlockPreDelaySamples[i] < 1) iEchoBlockPreDelaySamples[i] = 1;
				if (iEchoBlockPreDelaySamples[i] >= iEchoBlockBufferSize) iEchoBlockPreDelaySamples[i] = iEchoBlockBufferSize - 1;
			}
		}

		void set(float reverbPatterns[][4], float feedbackGain, float lpfCutoff, float drive, float diffusion, int numDelays)
		{
			if (diffusion < 0.0f) diffusion = 0.0f;
			if (diffusion > 1.0f) diffusion = 1.0f;

			const float fBlockMultipliers[4] = { 1.00f, 1.13f, 0.91f, 1.27f };
			for (int i = 0; i < 4; i++)
			{
				float fOffsetPatterns[3][4];
				for (int j = 0; j < 3; j++)
				{
					for (int k = 0; k < 4; k++)
					{
						float fJitterAmount = 0.0001f + (0.0014f * diffusion);
						float fTapJitter = fJitterAmount * (float)(k + 1);
						if (i % 2 == 1) fTapJitter = -fTapJitter;

						float fScaledMultiplier = 1.0f + ((fBlockMultipliers[i] - 1.0f) * diffusion);
						float fOffsetTime = (reverbPatterns[j][k] * fScaledMultiplier) + fTapJitter;
						if (fOffsetTime < 0.001f) fOffsetTime = 0.001f;
						if (fOffsetTime > 2.0f) fOffsetTime = 2.0f;
						fOffsetPatterns[j][k] = fOffsetTime;
					}
				}
				EchoBlocks[i].set(fOffsetPatterns, feedbackGain, lpfCutoff, drive, numDelays);
			}
		}

		float process(float input, float sampleRate, float drive)
		{
			float fSummedEchoBlocks = 0.0f;
			for (int i = 0; i < 4; i++)
			{
				float fBlockInput = input;
				float* pfBuffer = pfEchoBlockBuffers[i];
				if (pfBuffer != nullptr && iEchoBlockBufferSize > 1)
				{
					if (iEchoBlockWritePos[i] < 0 || iEchoBlockWritePos[i] >= iEchoBlockBufferSize) iEchoBlockWritePos[i] = 0;

					int iPreDelay = iEchoBlockPreDelaySamples[i];
					if (iPreDelay < 1) iPreDelay = 1;
					if (iPreDelay >= iEchoBlockBufferSize) iPreDelay = iEchoBlockBufferSize - 1;

					int iReadPos = iEchoBlockWritePos[i] - iPreDelay;
					while (iReadPos < 0) iReadPos += iEchoBlockBufferSize;
					if (iReadPos >= iEchoBlockBufferSize) iReadPos = iReadPos % iEchoBlockBufferSize;

					fBlockInput = pfBuffer[iReadPos];
					pfBuffer[iEchoBlockWritePos[i]] = input;

					iEchoBlockWritePos[i]++;
					if (iEchoBlockWritePos[i] >= iEchoBlockBufferSize) iEchoBlockWritePos[i] = 0;
				}

				fSummedEchoBlocks += EchoBlocks[i].process(fBlockInput, sampleRate, drive);
			}
			return fSummedEchoBlocks;
		}

		void postProcess()
		{
			for (int i = 0; i < 4; i++) EchoBlocks[i].postProcess();
		}

	private:
		MyEchoBlock EchoBlocks[4];
		float* pfEchoBlockBuffers[4];
		int iEchoBlockBufferSize = 0;
		int iEchoBlockWritePos[4];
		int iEchoBlockPreDelaySamples[4];
	};

	MyEcho::MyMultiLineDelay Delay;
	MyEcho::MyReverb Reverb;
};
