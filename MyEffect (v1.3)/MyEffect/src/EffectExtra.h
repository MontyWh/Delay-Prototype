//
//  EffectExtra.h
//  Additional Plugin Code
//
//  This file is a workspace for developing new DSP objects or functions to use in your plugin.
//

#pragma once

class MyFilters
{
public:

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

				iTapCount = iTapState = 0;
			}

			~MyDelay()
			{
				delete[] pfCircularBuffer;
			}

			void set(float delayTime)
			{
				fDelayTime = delayTime;
			}

			void initialiseBuffer(float sampleRate)
			{
				iBufferSize = sampleRate * 2; // 2 seconds of audio

				delete[] pfCircularBuffer;
				pfCircularBuffer = new float[iBufferSize]; // Allocate memory for the circular buffer

				for (int i = 0; i < iBufferSize; i++) pfCircularBuffer[i] = 0.0f; // Initialise the circular buffer to zero

				iBufferWritePos = 0; // Reset the write position to the start of the buffer
			}

			void setTapTempo(int tapState)
			{
				if (tapState == 1)
				{
					if (iTapState == 0)
					{
						iTapState = 1;
						iTapCount = 0;
					}
					else
					{
						iTapState = 0;
					}
				}
			}

			float read(float sampleRate)
			{
				int readPos = iBufferWritePos - (int)(sampleRate * fDelayTime); // Calculate the read position based on the delay time
				if (readPos < 0) readPos += iBufferSize;

				return pfCircularBuffer[readPos];
			}

			void write(float input)
			{
				pfCircularBuffer[iBufferWritePos] = input;
			}

			float samplesToTimeToFrequency(int tapCount, float sampleRate)
			{
				float fTime = tapCount / sampleRate; // Convert tapCount to time in seconds
				float fFrequency = 1.0f / fTime; // Convert time to frequency

				return fFrequency;
			}

			void tapTempoPost(float sampleRate)
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
			float* pfCircularBuffer;

			int iTapState;
			int iTapCount;
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

		void set(float delayTimes[3][4], float feedbackGain, float lpfCutoff, int numDelays)
		{
			iNumberOfDelayGroups = numDelays;
			for (int i = 0; i < iNumberOfDelayGroups; i++)
				for (int j = 0; j < 4; j++) Delays[i][j].set(&delayTimes[i][j], feedbackGain, lpfCutoff, 1);
		}

		int tapPos(int delayIndex, float time, float sampleRate)
		{
			int i = delayIndex / 4;
			int j = delayIndex % 4;
			int iBufferReadPos = Delays[i][j].MultipleDelays[0].iBufferWritePos - (time * sampleRate);
			if (iBufferReadPos < 0) iBufferReadPos += Delays[i][j].MultipleDelays[0].iBufferSize;
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
};
