/*
  ==============================================================================

    Harmonizer.h
    Created: 8 Nov 2022 10:15:01pm
    Author:  Nicola Rivosecchi

  ==============================================================================
*/
#pragma once
#include "Oscillators.h"
#include <vector>


/**
 Generates a vector of triangle waves in that spell out a chord.
 You can apply a vibrato effect, and the modulation of volume can be motulated to get more interesting results (even though I am not sure that is behaving the way I intended it)
 */
class Harmonizer
{
public:
    ///sets the ample rate to initialize all needed oscillators, intializes vibrato LFO and MTM LFO
    void setSampleRate(float sr)
    {
        sampleRate = sr;
        vibratoLFO.setSampleRate(sr);
        vibratoLFO.setFrequency(vibratoSpeed);
        vibratoLFO.mapLFO(0.7f, 0.3f);// set the lfo to moderately oscillate around a center of 0.5
        mtmLFO.setSampleRate(sr);
        mtmLFO.setFrequency(mtmSpeed);
        mtmLFO.mapLFO(0.9f, 0.1f);// set the mtm lfo to dramatically oscillate around a center of 0.5

    }
    
    void setNameTag(std::string nt)
    {
        nameTag = nt;
    }
    
    /**generates a chord starting from the root with desired number of voices (between 3 and 6) in the following voicing: r, 3, 7, extension1, extension2, extension3.
     @param root root note in MIDI
     @param voices desidered number of voices (between 3 and 6)
     @param chordKind 1 = Maj7 ; 2 = Min7 ; 3 = Dom7 ; 4 = Alt7
    */
    float processChord(float root, int voices = 4, int chordKind = 1)
    {
        if (voices < 3)
            voices = 3;
        
        if (voices > 6)
            voices = 6;
        
        std::vector<int> intervals ;
        
        
        switch (chordKind){
            case 1:{
                // Maj7
                intervals.push_back(0); //root
                intervals.push_back(4); //3
                intervals.push_back(11);//7
                intervals.push_back(14);//9
                intervals.push_back(21);//13
                intervals.push_back(30);//#11

                break;
            }
            case 2:{
                // Min7
                intervals.push_back(0); //root
                intervals.push_back(3); //b3
                intervals.push_back(10);//b7
                intervals.push_back(14);//9
                intervals.push_back(19);//5
                intervals.push_back(29);//11
                break;
            }
            case 3:{
                // Dom7
                intervals.push_back(0); //root
                intervals.push_back(4); //3
                intervals.push_back(10);//b7
                intervals.push_back(14);//9
                intervals.push_back(19);//5
                intervals.push_back(30);//#11
                break;
            }
            case 4:{
                // Alt7
                intervals.push_back(0); //root
                intervals.push_back(4); //3
                intervals.push_back(10);//b7
                intervals.push_back(7);//5
                intervals.push_back(15);//#9
                intervals.push_back(18);//b5
                break;
            }
        }
        
        float voicesMix = 0; //initialise the mixdown of all the voices
        
        
        //if the voicesCollection vector is empty, populate it
        if (voicesCollection.size() == 0)
        {
            for (int i = 0; i < voices; i++)
            {
                TriOsc voice;
                voicesCollection.push_back(voice);
                voice.setSampleRate(sampleRate);
                voicesCollection[i].setFrequency(midiToFrequency(root + intervals[i]));
            }
        }
        
        for (int i = 0; i < voices; i++)
        {
            voicesMix += voicesCollection[i].process() * i / voices;
        }

        voicesMix /= voicesCollection.size();

        //apply Vibrato LFO; <--- this can eventually be moved as a private function
        if(allowVibrato){
            float processedVibratoLFO = vibratoLFO.process();
            
            if(allowMtm){
                //applies the modulation of the modulator
                float lfoMix = (processedVibratoLFO + mtmLFO.process())/2;
                voicesMix *= lfoMix * vibratoAmount;
            }else{
                voicesMix *= processedVibratoLFO * vibratoAmount;
            }
        }
        return voicesMix;
    }
    
    ///enables a vibrato effect on the whole block of voices
    void toggleVibrato (bool t, float amount = 0.1f, float speed = 5.0f)
    {
        allowVibrato = t;
        
        //sets the allowed bounduaries for the amount variable
        if (amount < 0.01)
            amount = 0.01;
            
        if (amount > 1.0f)
            amount = 1.0f;
                
        vibratoAmount = amount;
            
            
        //sets the allowed bounduaries for the speed variable
        if (speed < 0.01)
            speed = 0.01;
            
        if (speed > 10.0f)
            speed = 10.0f;
            
        vibratoSpeed = speed;
        vibratoLFO.setFrequency(vibratoSpeed);
    }
    
    ///enables a second LFO that modulates the vibrato modulator to generate a more interesting wave
    void toggleModulateModulator (bool t, float amount = 0.1f, float speed = 5.0f)
    {
        allowMtm = t;
        
        //sets the allowed bounduaries for the speed variable
        if (speed < 0.01)
            speed = 0.01;
            
        if (speed > 10.0f)
            speed = 10.0f;
        
        mtmSpeed = speed;
        mtmLFO.setFrequency(vibratoSpeed);
    }
    
    
private:
    
    /**turns an int MIDI note value into a frequency(Hz) value
     @param midiValue int MIDI note value in the range 0-127
     @return the converted frequency(Hz) value
    */
    float midiToFrequency( float midiValue){
        float frequencyValue = 440 * pow(2, ((midiValue - 69) / 12));   // MIDI to Hz conversion formula: 440 * 2^[(d - 69) / 12]
        return frequencyValue;
    };
    
    //vibrato effect default variables
    LFO vibratoLFO;
    bool allowVibrato = 1;
    float vibratoSpeed = 1.0f;
    float vibratoAmount = 0.9f;
    
    //modulate the modulator default variables
    LFO mtmLFO;
    bool allowMtm = 1;
    float mtmSpeed = 0.0152f;
    
    std::string nameTag = "default Harmonizer";
    float sampleRate = 44100.0f;
    std::vector<TriOsc> voicesCollection;

};
