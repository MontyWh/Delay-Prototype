//
//  EffectExtra.h
//  Additional Plugin Code
//
//  This file is a workspace for developing new DSP objects or functions to use in your plugin.
//

#pragma once

class MyDelay
{
public:
    MyDelay()
    {
        pfCircularBuffer = nullptr;

        iBufferSize = 0;
        iBufferWritePos = 0;

        fDelayTimes[0] = 0.0f;
        fDelayTimes[1] = 0.0f;
        fDelayTimes[2] = 0.0f;
        fFeedbackGain = 0.0f;
    }

    ~MyDelay()
    {
        delete[] pfCircularBuffer;
    }

    void setDelayTimes(float* delayTimes, float feedbackGain)
    {
        fDelayTimes[0] = delayTimes[0];
        fDelayTimes[1] = delayTimes[1];
        fDelayTimes[2] = delayTimes[2];
        fFeedbackGain = feedbackGain;
    }

    void initialiseBuffer(float fSampleRate)
    {
        iBufferSize = fSampleRate * 2; // 2 seconds of audio

        pfCircularBuffer = new float[iBufferSize]; // Allocate memory for the circular buffer

        for (int i = 0; i < iBufferSize; i++) pfCircularBuffer[i] = 0.0f; // Initialise the circular buffer to zero

        iBufferWritePos = 0; // Reset the write position to the start of the buffer
    }

    float process(float fIn, float fSampleRate)
    {
        // Sum all 3 taps
        float fSummedTaps = 0.0f;
        for (int d = 0; d < 3; d++)
        {
            int readPos = iBufferWritePos - (int)(fSampleRate * fDelayTimes[d]);
            if (readPos < 0) readPos += iBufferSize;

            fSummedTaps += pfCircularBuffer[readPos];
        }

        // Apply feedback to summed taps BEFORE writing
        pfCircularBuffer[iBufferWritePos] = fIn + (fSummedTaps * fFeedbackGain);

        return fIn + fSummedTaps;
    }

    void postProcess()
    {
        iBufferWritePos++; // Increment the write position
        if (iBufferWritePos >= iBufferSize) iBufferWritePos = 0; // Wrap around if necessary
    }

protected:
    float fDelayTimes[3];
    float fFeedbackGain;

private:
    float* pfCircularBuffer;
    int iBufferSize, iBufferWritePos;
};
