/*
  ==============================================================================

    Chorus.h
    Created: 9 Nov 2022 5:24:04am
    Author:  Nicola Rivosecchi
 
//  I have started setting this up then i decided to give up due to time limitation
  ==============================================================================
*/
class Chorus
{
public:
    void chorus initialize (float sr){
        delay1.setSizeInSamples = 100000;
        delay2.setSizeInSamples = 100000;
        //initialize LFOs
        lfo1.setSampleRate(sr);
        lfo2.setSampleRate(sr);
        lfo1.setFrequency(lfoFreq);
        lfo2.setFrequency(lfoFreq);
        
        //shift phase of lfo2
        for(int i = 0; i < phaseOffset; i++){
            lfo2.process();
        }
    }
    
    //sets the feedback amout of both the delays
    void setFeedback(float fb)
    {
        delay1.setFeedback(fb);
        delay2.setFeedback(fb);
    }
    
    
private:
    DelayLine delay1;
    DelayLine delay2;
    LFO lfo1;
    LFO lfo2;
    float phaseOffset = 500.0f; //offset of phase between LFOs in  number of samples
    float lfoFreq = 1.0f;
        
}
#pragma once
