/*
  ==============================================================================

    FmSineOsc.h
    Created: 5 Nov 2022 9:07:56pm
    Author:  Nicola Rivosecchi

  ==============================================================================
*/


/**
 Frequency modulation sine oscillator
 */
class FmSineOsc
{
public:
  
    /**
     initializes the FM sine oscillator by setting the sample rate to all the elements (carrier, modulator and modulator LFO). it also sets a caller name to the LFO for debug reasons
     @param sampleRate float, the desired sample rate
     */
    void setSampleRate (float sampleRate)
    {
        carrier.setSampleRate(sampleRate);
        modulator.setSampleRate(sampleRate);
        modulatorLfo.setSampleRate(sampleRate);
        modulatorLfo.setCaller("fmSineOsc - " + nameTag);
    }
    
    
    /**
     sets a personalized name tag to the instance of the oscillator, useful to trace it in a debug situation
     @param nt string, personalized name tag
     */
    void setNameTag (std::string nt)
    {
        nameTag = nt;
    }
    
    /**
     set frequences for all the oscillators,
     @param carrierFreq float frequency to set the carrier to (Hz)
     @param modulatorFreq float frequency to set the carrier to (Hz); default value is carrierFreq + 1
     @param lfoFreq float frequency to set the lfo modulating the modulator's frequency to (Hz); as for any lfo, the max allowed is 20.0f.
     */
    void setAllFrequencies (float carrierFreq, float lfoFreq = 0.1f, float modulatorFreq = 0.0f)
    {
        //if the modulatorFreq has not been set or equals to 0, set it as carrierFreq + the defined offset (defaulted on 1Hz)
        if (modulatorFreq == 0.0f)
            modulatorFreq = carrierFreq + offset;
                    
        carrierCentralFrequency = carrierFreq;
        
        modulatorCentralFrequency = modulatorFreq;
        
        modulatorLfo.setFrequency(lfoFreq);
    }
    
    /**
    set exclusively the carrier's frequency (Hz)
    @param f float, desired carrier frequency
     */
    void setCarrierFrequency (float f)
    {
        carrier.setFrequency(f);
    }
    
    
    ///set exclusively the carrier's frequency
    void setModulatorFrequency (float f)
    {
        modulator.setFrequency(f);
    }
    
    /**
    set exclusively the LFO's frequency (Hz)
    @param f float, desired LFO frequency
     */
    void setModulatorLfoFrequency(float f)
    {
        modulatorLfo.setFrequency(f);
    }
    
    /**
    set exclusively the carrier's frequency (Hz)
    @param f float, desired carrier frequency
     */
    void setModulatorrCentralFrequency (float f)
    {
        modulatorCentralFrequency = f;
    }
    
    void setModulatorLfoIntensity(float i)
    {
        lfoModulationIntensity = i;
    }
    
    
    void setFmIntensity(float i)
    {
        fmIntensity = i;
    }
    
    void setSineOffset(float o)
    {
        offset = o;
    };
    
    void toggleModulatorLfo(bool toggle)
    {
        modulateTheModulator = toggle;
    }
    
    
    /**
     maps the modulator LFO to a desired range
     @param max float, max value that will be added to the modulator frequency
     @param min  float, min value that will be added to the modulator frequency
     @param scale float, multiplying factor for the LFO oscillation. Keep in mind that the LFO will clip values that exceed the max/min range.
     */
    void mapModulatorLfo(float max, float min, float scale = 1.0f)
    {
        modulatorLfo.mapLFO(max, min, scale);
    }

    
    /**
     
     @return processed step of the FMoscillator
     */
    float process()
    {
        //if modulateTheModulator, the modulatro wave frequency will get modulated as well
        if (modulateTheModulator)
        {
            //process the LFO that moduates the frequency of the modulator and store it in processedModulatorLfo
            float processedModulatorLfo = modulatorLfo.process();
            
            //store the modulator frequency, modulated by adding the lfo value
            float modulatorFrequency = modulatorCentralFrequency + processedModulatorLfo * lfoModulationIntensity;

            //update the modulator frequency
            modulator.setFrequency(modulatorFrequency);
        }
        
        //process the modulator and store it in processedModulator
        float processedModulator = modulator.process();
        float intensifiedModulator = processedModulator * fmIntensity;
        
        //store the carrier modulated frequency
        float carrierFrequency = carrierCentralFrequency * intensifiedModulator;

        //update the carrier frequency
        carrier.setFrequency(carrierFrequency);
        
        //finally process the output
        float output = carrier.process();
        
        return output;
    }
    
private:
    
    std::string nameTag = "Default FmSineOsc";
    bool modulateTheModulator = 1;
    float modulatorCentralFrequency;
    float carrierCentralFrequency;
    float fmIntensity = 1.0f;
    float lfoModulationIntensity = 1.0f ;
    float offset = 1.0; //-----------------------difference between carrier and modulatro in hz
    SineOsc carrier;
    SineOsc modulator;
    LFO modulatorLfo;
    
};


#pragma once
