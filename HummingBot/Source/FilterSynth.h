/*
  ==============================================================================

    FilterSynth.h
    Created: 25 Oct 2022 2:20:58pm
    Author:  Nicola Rivosecchi

  ==============================================================================
*/
#pragma once
#include "Oscillators.h"

/**
 its a square wave with a Low Pass Filter. the cutoff is modulated by an LFO. Uou can get quite interesting synthesis results by applying dramatic LFO values
 */
class FilterSynth
{
public:
    
    ///set the sample rate to the lfo and to the squareWave oscillator. It also initializes the lfo by
    ///assigning it a caller Id and mapping it to max and min values
    void setSampleRate(float sr)
    {
        sampleRate = sr; //-----------------Set the class private variable sampleRate so that the filter can easily access it
        filter.reset();//-------------------Reset the filter
        squareOsc.setSampleRate(sr);//------Set the sample rate to the oscillator
        lfo.setSampleRate(sr);//------------Set the sample rate to the LFO
        lfo.setCaller(nameTag);//-----------Set the caller to the LFO, for easier debug
        lfo.mapLFO(maxCutoff,minCutoff);//--Map the LFO to the maxCutoff and minCutoff values
        //lfo.toggleDebug(1);//-------------This is a good place to toggle the LFO debugger if needed
    }
    
    ///set the LFO frequency
    void setLfoFrequency(float f)
    {
        lfo.setFrequency(f);
    }
    
    ///set the square wave oscillator frequency
    void setOscFrequency(float f)
    {
        squareOsc.setFrequency(f);
    }
    
    ///set the resonance for the filter
    void setResonance(float r)
    {
        resonance = r;
    }
    
    ///set the minimum cutoff point for the oscillating filter
    void setMinCutoff(float mco)
    {
        minCutoff = mco;
    }

    ///set the maximum cutoff point for the oscillating filter
    void setMaxCutoff(float mco)
    {
        maxCutoff = mco;
    }
    
    ///set the gain for the filter
    void setGain(float g)
    {
        gain = g;
    }
     
    ///sets a new nameTag to the object
    void setNameTag (std::string nt)
    {
        nameTag = nt;
    }
    
    ///randomize all the parameters
    void randomizeParams()
    {
        setOscFrequency (random.nextFloat() * 2000.0f);//-----Randomize the square wave oscillator frequency
        setLfoFrequency (random.nextFloat() * 2.0f);//--------Randomize LFO frequency
        setResonance (random.nextFloat() * 10.0f);//----------Randomize filter resonance
        setMaxCutoff (random.nextFloat() * 20000.0f);//-------Randomize max cutoff the LFO can reach
        setMinCutoff (random.nextFloat() * maxCutoff);//------Randomize min cutoff the LFO can reach
        lfo.mapLFO (maxCutoff,minCutoff);//-------------------Map the LFO to the new bounduaries
        setGain (random.nextFloat() * 0.9f);//----------------Randomize the gain applied before the filter
    }
    
    ///generate a filtered square wave sample
    float process ()
    {
        float cutoff =lfo.process();//------------------------------------------------------------------Lfo oscillating between maxCutoff and minCutoff
        filter.setCoefficients(juce::IIRCoefficients::makeLowPass(sampleRate, cutoff, resonance));//----Set the filter coefficients
        float rawSquareWave = squareOsc.process() * gain ;//--------------------------------------------Generate the required square wave sample
                                                                                                      //and applies gain
        float filteredSquare = filter.processSingleSampleRaw(rawSquareWave);//--------------------------Apply the filter to the square wave
        return filteredSquare;
        

    }
    
private:
    // initialise oscillator, LFO, filter and Random Function
    SquareOsc squareOsc;
    LFO lfo;
    juce::IIRFilter filter;
    juce::Random random;

    
    std::string nameTag = "default FilterSynth";
    float sampleRate;
    float oscFrequency = 500.0f;//------Fundamental Frequency of the oscillator
    float lfoFrequency = 0.4f;//--------Frequency for the LFO that will modulate the cutoff of the filter
    float maxCutoff = 1900.0f;//--------Maximum Cutoff value the LFO can reach
    float minCutoff = 0.1f;//-----------Minimum Cutoff value the LFO can reach
    float resonance = 1.0f;//-----------Resonance value applied to the filter
    float gain = 0.5f;//----------------Gain applied before the filter
    
};

