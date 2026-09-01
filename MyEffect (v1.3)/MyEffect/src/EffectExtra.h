//
//  EffectExtra.h
//  Additional Plugin Code
//
//  This file is a workspace for developing new DSP objects or functions to use in your plugin.
//

#pragma once

class MyTremolo
{
public:
	MyTremolo()
	{

	}

	~MyTremolo()
	{

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

	float process(float input, float depth, float wet, float outputGain)
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

		const float fMod = std::sin(fPhasePos);
		const float fOsc = ((fMod + 1.0f) * 0.5f) * depth;
		const float fTremGain = (1.0f - depth) + fOsc;
		const float fWet = input * fTremGain;

		return ((1.0f - wet) * input + (wet * fWet)) * outputGain;
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

	void setDelayTapTempo(float sampleRate)
	{
		Delay.setTapTempo(sampleRate);
	}

	void setupParameters(float* fDelayEffectTimes, float fReverbPatterns[][4], float fFeedbackGain, float fLpfCutoff, int iNumberOfDelays)
	{
		Delay.set(fDelayEffectTimes, fFeedbackGain, fLpfCutoff, iNumberOfDelays);
		Reverb.set(fReverbPatterns, fFeedbackGain, fLpfCutoff, iNumberOfDelays);
	}

	float process(float input, float sampleRate, int bypassDelay, int bypassReverb)
	{
		if (bypassDelay == 0) input = Delay.process(input, sampleRate);
		if (bypassReverb == 0) input = Reverb.process(input, sampleRate);

		return input;
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
			iNumberOfDelays = 1;
		}

		void set(float *delayTimes, float feedbackGain, float lpfCutoff, int numDelays)
		{
			iNumberOfDelays = numDelays;
			for (int i = 0; i < iNumberOfDelays; i++) MultipleDelays[i].set(delayTimes[i]);
			fFeedbackGain = feedbackGain;

			LPF.set(lpfCutoff);
		}

		void setTapTempo(float sampleRate)
		{
			if (MultipleDelays[0].setTapTempo(sampleRate))
			{
				float fTapTime = MultipleDelays[0].getDelayTime();
				for (int i = 1; i < iNumberOfDelays; i++) MultipleDelays[i].setTappedDelayTime(fTapTime / pow(2.0f, (float)i));
			}
		}

		void initialiseBuffer(float sampleRate)
		{
			for (int i = 0; i < 3; i++) MultipleDelays[i].initialiseBuffer(sampleRate);
		}

		float process(float input, float sampleRate)
		{
			float fSummedTaps = 0.0f;
			for (int i = 0; i < iNumberOfDelays; i++) fSummedTaps += MultipleDelays[i].read(sampleRate); // Sum the outputs of all delay taps

			float fWriteValue = input + LPF.process(fSummedTaps * fFeedbackGain);
			for (int i = 0; i < iNumberOfDelays; i++) MultipleDelays[i].write(fWriteValue); // Write the same value to all delay taps

			return fSummedTaps;
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
				fManualDelayTime = 0.0f;

				iTapCount = iTapState = 0;
			}

			~MyDelay()
			{
				delete[] pfCircularBuffer;
			}

			void set(float delayTime)
			{
				float fDelayTimeDiff = delayTime - fManualDelayTime;
				if (fDelayTimeDiff < -0.000001f || fDelayTimeDiff > 0.000001f)
				{
					fManualDelayTime = delayTime;
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
			}

			bool setTapTempo(float sampleRate)
			{
				if (iTapState == 0)
				{
					iTapState = 1;
					iTapCount = 0;
					return false;
				}
				else
				{
					iTapState = 0;

					float fFrequency = samplesToTimeToFrequency(iTapCount, sampleRate);
					if (fFrequency > 0.0f)
					{
						setTappedDelayTime(1.0f / (fFrequency * fTapPulseMultiplier));
						return true;
					}
				}

				return false;
			}

			float read(float sampleRate)
			{
				int readPos = iBufferWritePos - (int)(sampleRate * fDelayTime); // Calculate the read position based on the delay time
				if (readPos < 0) readPos += iBufferSize;

				return pfCircularBuffer[readPos];
			}

			float getDelayTime()
			{
				return fDelayTime;
			}

			void setTappedDelayTime(float delayTime)
			{
				fDelayTime = delayTime;
				if (fDelayTime < 0.001f) fDelayTime = 0.001f;
				if (fDelayTime > 2.0f) fDelayTime = 2.0f;
			}

			void write(float input)
			{
				pfCircularBuffer[iBufferWritePos] = input;
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
				if (iTapState == 1) iTapCount++;
			}

			void postProcess()
			{
				iBufferWritePos++; // Increment the write position
				if (iBufferWritePos >= iBufferSize) iBufferWritePos = 0; // Wrap around if necessary
			}

			int iBufferWritePos;
			int iBufferSize;

		private:
			float fDelayTime;
			float fManualDelayTime;
			float* pfCircularBuffer;

			int iTapState;
			int iTapCount;
			float fTapIntervalMultiplier = 1.0f;
			float fTapPulseMultiplier = 1.0f;
		};

		MyDelay MultipleDelays[3];

	private:
		int iNumberOfDelays;
		float fFeedbackGain;

		MyFilters::MyIirFilter::MyBiQuadFilter::MyLowPassFilter LPF;
	};

	class MyReverb
	{
	public:
		void initialiseBuffer(float sampleRate)
		{
			for (int i = 0; i < 3; i++)
				for (int j = 0; j < 4; j++) Delays[i][j].initialiseBuffer(sampleRate);
		}

		void set(float delayTimes[][4], float feedbackGain, float lpfCutoff, int numDelays)
		{
			iNumberOfDelayGroups = numDelays;
			for (int i = 0; i < iNumberOfDelayGroups; i++)
				for (int j = 0; j < 4; j++) Delays[i][j].set(&delayTimes[i][j], feedbackGain, lpfCutoff, 1);
		}

		int tapPos(int delayIndex, float time, float sampleRate)
		{
			int iDelayGroup = delayIndex / 4;
			int iDelayGroupTarget = delayIndex % 4;
			int iBufferReadPos = Delays[iDelayGroup][iDelayGroupTarget].MultipleDelays[0].iBufferWritePos - (time * sampleRate); // Calculate the read position based on the delay time
			if (iBufferReadPos < 0) iBufferReadPos += Delays[iDelayGroup][iDelayGroupTarget].MultipleDelays[0].iBufferSize; // Wrap around if necessary
			return iBufferReadPos;
		}

		float process(float input, float sampleRate)
		{
			float fSummedTaps = 0.0f;
			for (int i = 0; i < iNumberOfDelayGroups; i++)
				for (int j = 0; j < 4; j++) fSummedTaps += Delays[i][j].process(input, sampleRate);
			return fSummedTaps;
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

	private:
		MyEcho::MyMultiLineDelay Delay;
		MyEcho::MyReverb Reverb;
};
