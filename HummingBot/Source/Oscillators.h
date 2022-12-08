//
//  Oscillators.h
//  Challange 4
//
//  Created by Nicola Rivosecchi on 24/10/2022.
//

#ifndef Oscillators_h
#define Oscillators_h
#include "debugResolutionTool.h"


class Phasor
{
#include <cmath>                // for the sin() and tan() function
    
public:
    
    float process()
    {
        float processedPhase;
        phase += phaseDelta;
        
        if(phase > 1.0f)
            phase =-1;
        
        processedPhase = output(phase);
        return processedPhase;
    };
    
    float modulatePhase ( float modPhase, float LFO = 1, float AMT = 1)
    {
        float processed = process();
        processed += modPhase * LFO * AMT;
        return processed;
    };
    
    virtual float output (float outputValue){
        return outputValue;
    }
    /**
     modulates the phasor phase by multiplying it by an input value
     */

    void setSampleRate(float SR)
    {
        sampleRate = SR;
    }
    
    float getSampleRate()
    {
        return sampleRate;
    }
    
    virtual void setFrequency(float freq)
    {
        frequency = freq;
        phaseDelta = frequency  / float(sampleRate);
    }
    
    void setOscKind(std::string Name)
    {
        oscKind = Name;
    }
    
    int type = 3 ;
    
    
    float getPhase()
    {
        return phase;
    };
    
    
    float getFrequency()
    {
        return frequency;
    };
    
    
private:
    std::string oscKind = "" ;
    int sampleRate = 44100;
    float frequency = 440;              // the frequency of the tone to generate
    float phase = 0.0f;                 // keep track of the phase of our oscillator
    float phaseDelta = frequency / sampleRate;                   // phaseDelta - how much increment the phase by for each sample

};

 
// CHILD Class
class SineOsc : public Phasor
{
public:
    //Builder Function assigns the correct OscKind to the instance
    SineOsc()
    {
        setOscKind("Sine Wave");
    }
    
    float output (float phase) override
    {
        float outputValue = sin(phase * 2 * 3.141593);
        return outputValue;
    }
    
    
};

// CHILD Class
class TriOsc : public Phasor
{
public:
    //Builder Function assigns the correct OscKind to the instance
    TriOsc()
    {
        setOscKind("Triangle Wave");
    }
    
    float output (float phase) override
    {
        return fabsf(phase - 0.5f) -0.5f;
    }
};

// CHILD Class
class SquareOsc : public Phasor
{
public:
    //Builder Function assigns the correct OscKind to the instance
    SquareOsc()
    {
        setOscKind("Square Wave");
    }
    
    float output (float phase) override
    {
        float outputValue = 0.5f;
        if (phase < pulseWidth)
            outputValue = -0.5f;
        return outputValue;
    }
    
    void setPulseWidth(float pw)
    {
         pulseWidth = pw;
    }
private:
    float pulseWidth = 0.5f;
};


// CHILD Class
class SmoothSquareOsc : public Phasor
{
public:
    //Builder Function assigns the correct OscKind to the instance
    SmoothSquareOsc()
    {
        setOscKind("Smooth Square Wave");
    }
    
    float output (float phase) override
    {
        float outputValue = 0.5f;
        if (phase < pulseWidth)
            outputValue = -0.5f;
        
        
        return tan(outputValue);
    }
    
    void setPulseWidth(float pw)
    {
         pulseWidth = pw;
    }
private:
    float pulseWidth = 0.5f;
};



class LFO : public Phasor
{
public:

    float output (float phase) override
    {
        
        float outputValue = (sin(phase * 2 * 3.141593) + 1) / 2;
        
        //assert((getFrequency() < 20.0f)); // frequency i
        
        
        
        //scale the output value between the desired min and max
        //
        //        (b-a)(x - min)
        // f(x) = --------------  + a
        //           max - min
        // where a and b are the normal bounduaries of the lfo output ±1
        
        if (minValue < 0 && enableNegatives)
        {
            //if the mapping range has a negative area and negative values are alloed, move it into the positive area first
            float tempMinValue = 0;
            float tempMaxValue = maxValue - minValue;
            
            //then do the mapping
            outputValue = (tempMaxValue - tempMinValue) * (outputValue);
            outputValue += tempMinValue;
            
            //finally shift back the result in the desired range
            outputValue +=minValue;
            
        }else
        {   outputValue = (maxValue - minValue) * (outputValue);
            outputValue += minValue;
        }

        

        if (enableDebug) std::cout << debugPhaseOutput.nextStep("the phase of the lfo, called by " + caller + " once mapped is", outputValue);

        

        //multiply output value by the scaleIndex
        outputValue *= scaleIndex;
        
        assert((minValue < maxValue)); //catch potential troublesome condition fo min being larger than max
        
        
        //truncate the output if overtakes set min/max bounduaries
        if (outputValue < minValue)
            outputValue=minValue;
        
        if (outputValue > maxValue)
            outputValue=maxValue;
        
        return outputValue;
    }
    
    /**
     this function sets the mapping parameters. you can only map to a range of positive numbers  than zero
     @param max float, maximum value reached by the LFO
     @param min float, minimum value reached by the LFO
     @param scaleIndex float, multiplies the lfo values, resulting in a magnified and clipped LFO sinewave if multiplied by a number larger than 1 and a shrunk LFO sinewave if multiplied by a number between 0 and 1
     */
    void mapLFO(float max, float min = 0.00001f, float scale = 1.0f)
    {
        maxValue = max;
        minValue = min;
        scaleIndex = scale;
    };
    
    
    ///sets caller ID so that its easier to debug
    void setCaller (std::string c)
    {
        caller = c;
    }
    
    ///allows the LFO to assume negative values
    void toggleNegativeValues(bool t){
        enableNegatives = t;
    }
    
    ///allows debug messages to come through
    void toggleDebug (bool t){
        enableDebug = t;
    }
    
    
private:
    std::string caller = "-Caller Name Not Assigned-";
    bool enableDebug = 0; //toggle debug mesages
    bool enableNegatives = 0; //enables the LFO to oscillate between positive and negative numbers
    float maxValue = 1.0f;
    float minValue = 0.000001f;
    float scaleIndex = 1.0f;
    DebugResolutionTool debugPhaseOutput;
    
    
};

#endif /* Oscillators_h */
