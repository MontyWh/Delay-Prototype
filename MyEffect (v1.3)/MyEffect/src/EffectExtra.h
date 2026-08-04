//
//  EffectExtra.h
//  Additional Plugin Code
//
//  This file is a workspace for developing new DSP objects or functions to use in your plugin.
//


class MyDelay
{
public:

    void ReadPosition()
    {
        iBufferReadPos = iBufferWritePos - (fSampleRate * (1.0f / 4.0f)); // 0.25 seconds delay
        if (iBufferReadPos < 0) iBufferReadPos += iBufferSize; // Wrap around if necessary
    }

    void process(int ch, float  fIn[2], float fFeedbackGain, float fDelayTime, float  fWet[2])
    {
        // Add your effect processing here
        pfCircularBuffer[ch][iBufferWritePos] = fIn[ch]; // Write the input sample to the circular buffer

        float fDelSig = fIn[ch] + (fDelSig * fFeedbackGain); // Read the delayed sample from the circular buffer
        fDelSig *= fDelayTime;

        fWet[ch] = fIn[ch] + fDelSig;
    }

    void postProcess()
    {
        iBufferWritePos++; // Increment the write position
        if (iBufferWritePos >= iBufferSize) iBufferWritePos = 0; // Wrap around if necessary. Reset to 0 if the write position exceeds the buffer size
    }

    float fSampleRate;
    signed int iBufferReadPos;

private:
    float* pfCircularBuffer[2]; // Used to point to an arrary that acts as the circular buffer itself
    int iBufferSize, iBufferWritePos; // Used to store the size of the circular buffer, and the current write position (arrary index) in the buffer
};