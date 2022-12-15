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
        myOsc.setSampleRate(sampleRate);
        detuneOsc.setSampleRate(sampleRate);
        env.setSampleRate(sampleRate);
        juce::ADSR::Parameters envParams;
        
        
        envParams.attack = 1.0f;
        envParams.decay = 1.0f;
        envParams.sustain = 0.5f;
        envParams.release = 1.0f;

        env.setParameters(envParams);

    }
    
    
    void setDetune (float detuneIn)
    {
        detuneAmount = detuneIn;
    }
    
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {
        if(midiNoteNumber != 36){
            playing = true;
            ending = false;
            freq = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
            
            myOsc.setFrequency(freq);

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
            
            detuneOsc.setFrequency(freq - detuneAmount);

            
            
            // iterate through the necessary number of samples (from startSample up to startSample + numSamples)
            for (int sampleIndex = startSample;   sampleIndex < (startSample+numSamples);   sampleIndex++)
            {
//                // your sample-by-sample DSP code here!
//                // An example white noise generater as a placeholder - replace with your own code
//
                float envVal = env.getNextSample();
                float currentSample = 0.0f;//main sample. It will eventually hold together all the samples end be output to the audioBuffer
                currentSample = (myOsc.process() + detuneOsc.process()) / 2.0f * envVal;

                
                // for each channel, write the currentSample float to the output
                for (int chan = 0; chan<outputBuffer.getNumChannels(); chan++)
                {
                    // The output sample is scaled by 0.2 so that it is not too loud by default
                    outputBuffer.addSample (chan, sampleIndex, currentSample * 0.2);
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
    SineOsc myOsc, detuneOsc;
    
    float freq ;//---------------the frequecy value that will be used to generate the synthesiser note
    
    float detuneAmount = 2.0f;

    
    
    juce::ADSR env;

};
