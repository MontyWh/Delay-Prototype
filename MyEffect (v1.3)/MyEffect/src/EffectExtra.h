//
//  EffectExtra.h
//  Additional Plugin Code
//
//  This file is a workspace for developing new DSP objects or functions to use in your plugin.
//

#pragma once
#include <cmath>

class MyFilters
{
public:
	class MySvfFilter // State Variable Filter
	{
	public:
		class MyIirFilter
		{
		public:
			void setSampleRate(float sampleRate)
			{
				fSampleRate = sampleRate;
				updateCoefficients();
			}

			void set(float cutoffFrequency, float qualityFactor)
			{
				// Initialise your filter variables here
				fFrequencyNorm = cutoffFrequency;
				fQValue = qualityFactor;
				updateCoefficients();
			}

			float processLowPass(float input)
			{
				float fLowPass, fBandPass, fHighPass;
				processCore(input, fLowPass, fBandPass, fHighPass);
				return fLowPass;
			}

			float processHighPass(float input)
			{
				float fLowPass, fBandPass, fHighPass;
				processCore(input, fLowPass, fBandPass, fHighPass);
				return fHighPass;
			}

			float processBandPass(float input)
			{
				float fLowPass, fBandPass, fHighPass;
				processCore(input, fLowPass, fBandPass, fHighPass);
				return fBandPass;
			}

		private:
			void updateCoefficients()
			{
				const float fPi = 3.14159265359f;
				const float fMinHz = 20.0f;
				const float fMaxHz = 0.45f * fSampleRate;
				const float fRatio = fMaxHz / fMinHz;

				const float fHz = fMinHz * std::pow(fRatio, fFrequencyNorm);

				fG = std::tan(fPi * fHz / fSampleRate);
				fK = 1.0f / fQValue;

				fA1 = 1.0f / (1.0f + fG * (fG + fK));
				fA2 = fG * fA1;
				fA3 = fG * fA2;
			}

			void processCore(float input, float& fLowPass, float& fBandPass, float& fHighPass)
			{
				const float v3 = input - fIc2Eq;
				const float v1 = (fA1 * fIc1Eq) + (fA2 * v3);
				const float v2 = fIc2Eq + (fA2 * fIc1Eq) + (fA3 * v3);

				fIc1Eq = (2.0f * v1) - fIc1Eq;
				fIc2Eq = (2.0f * v2) - fIc2Eq;

				fLowPass = v2;
				fBandPass = v1;
				fHighPass = input - (fK * fBandPass) - fLowPass;
			}

			float fSampleRate = 48000.0f;
			float fFrequencyNorm = 0.5f;
			float fQValue = 0.707f;

			float fG = 0.0f;
			float fK = 1.0f;
			float fA1 = 1.0f;
			float fA2 = 0.0f;
			float fA3 = 0.0f;

			float fIc1Eq = 0.0f;
			float fIc2Eq = 0.0f;
		};

		class MyLowPassFilter
		{
		public:
			class MyBiQuadFilter
			{
			public:
				void setSampleRate(float sampleRate)
				{
					filter.setSampleRate(sampleRate);
				}

				void set(float fCutoff)
				{
					filter.set(fCutoff, 0.707f);
				}

				void process(float& fWet)
				{
					fWet = filter.processLowPass(fWet); // second-order low-pass filter response.
				}

			private:
				MyIirFilter filter;
			};

			class FilterStages
			{
			public:

				void setSampleRate(float sampleRate)
				{
					for (int i = 0; i < 4; ++i) biQuadFilter[i].setSampleRate(sampleRate);
				}

				void set(float fCutoff)
				{
					for (int i = 0; i < 4; ++i) biQuadFilter[i].set(fCutoff);
				}

				void process(float& fWet)
				{
					for (int i = 0; i < 4; ++i) biQuadFilter[i].process(fWet);
				}

			private:
				MyBiQuadFilter biQuadFilter[4];
			};
		};

		class MyHighPassFilter
		{
		public:
			class MyBiQuadFilter
			{
			public:
				void setSampleRate(float sampleRate)
				{
					filter.setSampleRate(sampleRate);
				}

				void set(float fCutoff)
				{
					filter.set(fCutoff, 0.707f);
				}

				void process(float& fWet)
				{
					fWet = filter.processHighPass(fWet); // second-order high-pass filter response.
				}

			private:
				MyIirFilter filter;
			};

			class FilterStages
			{
			public:

				void setSampleRate(float sampleRate)
				{
					for (int i = 0; i < 4; ++i) biQuadFilter[i].setSampleRate(sampleRate);
				}

				void set(float fCutoff)
				{
					for (int i = 0; i < 4; ++i) biQuadFilter[i].set(fCutoff);
				}

				void process(float& fWet)
				{
					for (int i = 0; i < 4; ++i) biQuadFilter[i].process(fWet);
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
				void setSampleRate(float sampleRate)
				{
					filter.setSampleRate(sampleRate);
				}

				void set(float fFrequency, float fQ, float fBandwidth)
				{
					float fSafeBandwidth = (fBandwidth > 0.0f) ? fBandwidth : 0.0001f;
					float fEffectiveQ = fQ / fSafeBandwidth;
					filter.set(fFrequency, fEffectiveQ);
				}

				void process(float& fWet)
				{
					fWet = filter.processBandPass(fWet); // second-order band-pass filter response.
				}

			private:
				MyIirFilter filter;
			};

			class FilterStages
			{
			public:
				void setSampleRate(float sampleRate)
				{
					for (int i = 0; i < 4; ++i) biQuadFilter[i].setSampleRate(sampleRate);
				}

				void set(float fFrequency, float fQ, float fBandwidth)
				{
					for (int i = 0; i < 4; ++i) biQuadFilter[i].set(fFrequency, fQ, fBandwidth);
				}

				void process(float& fWet)
				{
					for (int i = 0; i < 4; ++i) biQuadFilter[i].process(fWet);
				}

			private:
				MyBiQuadFilter biQuadFilter[4];
			};
		};
	};

	using MyLowPassFilter = MySvfFilter::MyLowPassFilter;
	using MyBandPassFilter = MySvfFilter::MyBandPassFilter;
	using MyHighPassFilter = MySvfFilter::MyHighPassFilter;
};
