/*
  ==============================================================================

    DelayLine.h
    Created: 5 Nov 2022 5:03:23pm
    Author:  Nicola Rivosecchi

  ==============================================================================
*/

#pragma once


/**
 generates a delay from an incoming signal, with the possibility of feeding this delay back into it
 */
class DelayLine
{
public:
    
    ///destructor function, frees up memory where delay line goes out of scope
    ~DelayLine()
    {
        if(data != nullptr)
            delete[] data;
    }
    
    
    ///set the maximum size of the delay line
    void setSizeInSamples (int newSize)
    {
        size = newSize; //store the new size
        
        //free up existing data
        if (data != nullptr) 
            delete[] data;
        
        data = new float[size]; //initialize our array
        
        
        //make sure that our new data array is empty
        for (int i = 0; i < size; i++)
        {
            data[i] = 0.0f;
        }
    }
    
    ///set the delay time
    void setDelayTimeInSamples (float newDelayTime)
    {
        delayTime = newDelayTime;
        
        // set the read time to be behind the write index
        readIndex = writeIndex - delayTime;
        
        if (readIndex < 0)
            readIndex += size;
        
    }
    ///sets the amount of feedback to be applied to the delay. It must be
    void setFeedbackAmt(float fb)
    {
        //make sure the feedback is in a safe range
        if(fb > 1.0f)
            fb = 1.0f;
        
        if(fb < 0.0f)
            fb = 0.0f;
        
        feedback = fb;
    }
    
    
    float linearInterpolation ()
    {
        //get the two indexes around our read index
        int indexA = int(readIndex);
        int indexB = indexA + 1;
        
        //wrap around if we exceed the size
        if (indexB > size)
            indexB -= size;
            
        float valA = data[indexA];
        float valB = data[indexB];

        //store the remainder
        float remainder = readIndex - indexA;
        
        //actual interpolation
        float interpolatedSample = (1-remainder) * valA + remainder * valB;
            
        return interpolatedSample;
    }
    
    ///call every sample to
    ///-- store the new sample
    ///-- advance play and write index
    ///-- return the value at play index
    float process(float inputSample)
    {
        
        float outputSample = linearInterpolation();

        //store value at write index
        data[writeIndex] = inputSample + outputSample * feedback;
        
        //get value  at read index
        readIndex++;
        
        //wraps around the read index
        if (readIndex >= size)
            readIndex -= size;
        
        //wraps around the write index
        writeIndex++;
        if (writeIndex >= size)
            writeIndex -= size;
        
        return outputSample;
    }


    
    
private:
    float readIndex = 0;//--------Read position as an index
    int writeIndex = 0;//---------Write position as an index
    int size;//-------------------Maximum possible delay time
    float* data = nullptr;//------Store input audio data
    float feedback = 0.3f;//------Feedback amount
    int delayTime;//--------------Delay time in samples
};
