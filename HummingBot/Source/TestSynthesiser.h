/*
  ==============================================================================

    TestSynthesiser.h
    Created: 7 Mar 2020 4:27:57pm
    Author:  Tom Mudd

  ==============================================================================
*/

#pragma once
#include "Oscillators.h"
#include "Envelope.h"
#include <vector>
#include "AddSynth.h"
#include "PluginProcessor.h"
#include "DelayLine.h"

// ===========================
// ===========================
// SOUND
class TestSynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote      (int) override      { return true; }
    //--------------------------------------------------------------------------
    bool appliesToChannel   (int) override      { return true; }
};




// =================================
// =================================
// Synthesiser Voice - your synth code goes in here

/*!
 @class TestSynthVoice
 @abstract struct defining the DSP associated with a specific voice.
 @discussion multiple TestSynthVoice objects will be created by the Synthesiser so that it can be played polyphicially
 
 @namespace none
 @updated 2019-06-18
 */
class TestSynthVoice : public juce::SynthesiserVoice
{
public:
    TestSynthVoice() {}
    //--------------------------------------------------------------------------
    /**
     What should be done when a note starts

     @param midiNoteNumber
     @param velocity
     @param SynthesiserSound unused variable
     @param / unused variable
     */
    
    
    void setAllowSynthNotes(bool _allowSynthNotes)
    {
     
        allowSynthNotes = _allowSynthNotes;
        
    }
    
    float midiToFrequency( int midiValue){
        //float floatMidiValue = (float) midiValue;
        float frequencyValue = 440.0f * pow(2.0f, ((midiValue - 69.0f) / 12.0f));   // MIDI to Hz conversion formula: 440 * 2^[(d - 69) / 12]
        return frequencyValue;
    };

    int octaves (int octNum)
    {
        return octNum * 12;
    }
    
    void init (float sampleRate)
    {
        sawToothOsc.setSampleRate(sampleRate);
        sawToothDetuneOsc.setSampleRate(sampleRate);

        sineOsc.setSampleRate(sampleRate);
        sineDetuneOsc.setSampleRate(sampleRate);
        
        squareOsc.setSampleRate(sampleRate);
        squareDetuneOsc.setSampleRate(sampleRate);
        
        triOsc.setSampleRate(sampleRate);
        triDetuneOsc.setSampleRate(sampleRate);
        
        delay.setSizeInSamples(sampleRate * 5);
        delay.setDelayTimeInSamples(delayTime);
        delay.setFeedbackAmt(delayFeedback);
        
        
        
        
        env.setSampleRate(sampleRate);
    }
    
    void setSynthVoiceGain(float _synthVoiceGain)
    {
        synthVoiceGain = _synthVoiceGain;
    }

    
    void setDetune (float detuneIn)
    {
        detuneAmount = detuneIn;
    }
    
    void setADSR (float _attack, float _decay, float _sustain, float _release)
    {
        attack = _attack;
        decay = _decay;
        sustain = _sustain;
        release = _release;
        
    }
    
    void setOscShape (int _oscShape)
    {
        oscShape = _oscShape;
    }
    
    void setDelay (float _delayTime, float _delayFeedback)
    {
        delayTime = _delayTime;
        delayFeedback = _delayFeedback;
    }
    
    
    
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {
        if(midiNoteNumber != 36 && allowSynthNotes)
        {
            playing = true;
            ending = false;
            freq = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
            
            envParams.attack = attack;
            envParams.decay = decay;
            envParams.sustain = sustain;
            envParams.release = release;

            env.setParameters(envParams);
            
            
            sawToothOsc.setFrequency(freq);
            sineOsc.setFrequency(freq);
            squareOsc.setFrequency(freq);
            triOsc.setFrequency(freq);
            
            
            env.reset();
            env.noteOn();
        }
    }
    //--------------------------------------------------------------------------
    /// Called when a MIDI noteOff message is received
    /**
     What should be done when a note stops

     @param / unused variable
     @param allowTailOff bool to decie if the should be any volume decay
     */
    void stopNote(float /*velocity*/, bool allowTailOff) override
    {
        env.noteOff();
        ending = true;
        allowSynthNotes = true;

    }
    
    //--------------------------------------------------------------------------
    /**
     The Main DSP Block: Put your DSP code in here
     
     If the sound that the voice is playing finishes during the course of this rendered block, it must call clearCurrentNote(), to tell the synthesiser that it has finished

     @param outputBuffer pointer to output
     @param startSample position of first sample in buffer
     @param numSamples number of smaples in output buffer
     */
    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override
    {
        if (playing) // check to see if this voice should be playing
        {
            
            sawToothDetuneOsc.setFrequency(freq - detuneAmount);
            sineDetuneOsc.setFrequency(freq - detuneAmount);
            squareDetuneOsc.setFrequency(freq - detuneAmount);
            triDetuneOsc.setFrequency(freq - detuneAmount);


            
            
            // iterate through the necessary number of samples (from startSample up to startSample + numSamples)
            for (int sampleIndex = startSample;   sampleIndex < (startSample+numSamples);   sampleIndex++)
            {
//                // your sample-by-sample DSP code here!
//                // An example white noise generater as a placeholder - replace with your own code
//
                float envVal = env.getNextSample();
                std::vector<float> currentSample;
                float finalSample = 0.0f;//main sample. It will eventually hold together all the samples end be output to the audioBuffer
            

            
                switch (oscShape)
                {
                    case 0:
                        currentSample.push_back((sawToothOsc.process() + sawToothDetuneOsc.process()) / 2.0f * envVal);
                        break;
                    case 1:
                        currentSample.push_back((squareOsc.process() + squareDetuneOsc.process()) / 2.0f * envVal);
                        break;
                    case 2:
                        currentSample.push_back((sineOsc.process() + sineDetuneOsc.process()) / 2.0f * envVal);
                        break;
                    case 3:
                        currentSample.push_back((triOsc.process() + triDetuneOsc.process()) / 2.0f * envVal);
                        break;
                }
                    
                
                //if delay time is not zero, process the delay line as well

                
                for(int i = 0; i < currentSample.size(); i++)
                {
                    finalSample += currentSample[i];
                }
                
                finalSample /= currentSample.size();
                
                finalSample *= synthVoiceGain;
                
                
                
//                if (delayTime != 0)
//                {
//                    finalSample +=delay.process(finalSample);
//                    finalSample /=2;
//                }
                
                
                
                // for each channel, write the currentSample float to the output
                for (int chan = 0; chan<outputBuffer.getNumChannels(); chan++)
                {
                    // The output sample is scaled by synthProtectionGain so that it is not too loud by default
                    outputBuffer.addSample (chan, sampleIndex, finalSample * synthProtectionGain);
                }
                
                if (ending)
                {
                    if (envVal <0.00001f)
                    {
                        clearCurrentNote();
                        playing = false;
                    }
                }
            }
        }
    }
    //--------------------------------------------------------------------------
    void pitchWheelMoved(int) override {}
    //--------------------------------------------------------------------------
    void controllerMoved(int, int) override {}
    //--------------------------------------------------------------------------
    /**
     Can this voice play a sound. I wouldn't worry about this for the time being

     @param sound a juce::SynthesiserSound* base class pointer
     @return sound cast as a pointer to an instance of TestSynthSound
     */
    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<TestSynthSound*> (sound) != nullptr;
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    // Set up any necessary variables here
    /// Should the voice be playing?
    
    bool playing = false;
    bool ending = false;
    
    
    
    SawToothOsc sawToothOsc, sawToothDetuneOsc;
    SquareOsc squareOsc, squareDetuneOsc;
    SineOsc sineOsc, sineDetuneOsc;
    TriOsc triOsc, triDetuneOsc;
    int oscShape = 1;
    
    
    float freq ;//---------------the frequecy value that will be used to generate the synthesiser note
    
    
    float synthVoiceGain = 0.4f;
    float detuneAmount = 2.0f;
    
    juce::ADSR::Parameters envParams;
    float attack;
    float decay;
    float sustain;
    float release;
    
    DelayLine delay;
    float delayTime = 0.0f;
    float delayFeedback = 0.0f;
    
    
    bool allowSynthNotes = true;
    
    juce::ADSR env;
    
    float synthProtectionGain = 0.3f;

};
