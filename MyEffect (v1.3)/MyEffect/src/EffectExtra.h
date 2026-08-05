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
        pfCircularBuffer[0] = nullptr;
        pfCircularBuffer[1] = nullptr;

        iBufferSize = 0;
        iBufferWritePos = 0;
        iBufferReadPos = 0;
    }

    ~MyDelay()
    {
        for (int ch = 0; ch < 2; ch++) delete[] pfCircularBuffer[ch];
    }

    template <typename ParameterContainer>
    void getDelayParameters(const ParameterContainer& parameters)
    {
        fDelayTime = parameters[1];
        fFeedbackGain = parameters[2];
    }

    void initialiseBuffer(float fSampleRate)
    {
        iBufferSize = fSampleRate * 2; // 2 seconds of audio

        for (int ch = 0; ch < 2; ch++)
        {
            pfCircularBuffer[ch] = new float[iBufferSize]; // Allocate memory for the circular buffer

            for (int i = 0; i < iBufferSize; i++) pfCircularBuffer[ch][i] = 0.0f; // Initialise the circular buffer to zero
        }

        iBufferWritePos = 0; // Reset the write position to the start of the buffer
    }

    int readBufferPosition(float fSampleRate)
    {
        signed int iBufferReadPos;

        iBufferReadPos = iBufferWritePos - (fSampleRate * fDelayTime); // Use the delay time parameter
        if (iBufferReadPos < 0) iBufferReadPos += iBufferSize; // Wrap around if necessary

        return iBufferReadPos;
    }

    float process(float fIn, int ch)
    {
        // Add your effect processing here
        float fDelSig = pfCircularBuffer[ch][iBufferReadPos]; // Read the delayed sample from the circular buffer

        pfCircularBuffer[ch][iBufferWritePos] = fIn + (fDelSig * fFeedbackGain); // Write input + feedback to circular buffer

        return fIn + fDelSig;
    }

    void postProcess()
    {
        iBufferWritePos++; // Increment the write position
        if (iBufferWritePos >= iBufferSize) iBufferWritePos = 0; // Wrap around if necessary. Reset to 0 if the write position exceeds the buffer size
    }

    signed int iBufferReadPos;

protected:
    float fDelayTime;
    float fFeedbackGain;

private:
    float* pfCircularBuffer[2]; // Used to point to an arrary that acts as the circular buffer itself
    int iBufferSize, iBufferWritePos; // Used to store the size of the circular buffer, and the current write position (arrary index) in the buffer
};
