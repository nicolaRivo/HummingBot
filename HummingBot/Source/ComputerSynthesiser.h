/*
  ==============================================================================

    ComputerSynthesiser.h
    Created: 7 Mar 2020 4:27:57pm
    Author:  Tom Mudd

  ==============================================================================
*/

#pragma once
#include "Oscillators.h"
#include "MidiProcessor.h"

// ===========================
// ===========================
// SOUND
class ComputerSynthSound : public juce::SynthesiserSound
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
 @class ComputerSynthVoice
 @abstract struct defining the DSP associated with a specific voice.
 @discussion multiple ComputerSynthVoice objects will be created by the Synthesiser so that it can be played polyphicially
 
 @namespace none
 @updated 2019-06-18
 */
class ComputerSynthVoice : public juce::SynthesiserVoice
{
public:
    ComputerSynthVoice() {}
    
    void init (float sampleRate)
    {
        myTriOsc1.setSampleRate(sampleRate);
        myTriOsc2.setSampleRate(sampleRate);
        myTriOsc3.setSampleRate(sampleRate);
        myTriOsc4.setSampleRate(sampleRate);
    }
    
    
    void operateMidiProcess(juce::MidiBuffer& midiMessages)
    {
        
        
    }
    
    int octaves (int octNum)
    {
        return octNum * 12;
        
    }
    
    
    /**turns an int MIDI note value into a frequency(Hz) value
         @param midiValue int MIDI note value in the range 0-127
         @return the converted frequency(Hz) value
        */
        float midiToFrequency( int midiValue){
            float frequencyValue = 440.0f * pow(2.0f, ((midiValue - 69.0f) / 12.0f));   // MIDI to Hz conversion formula: 440 * 2^[(d - 69) / 12]
            return frequencyValue;
        };
    
    
    //--------------------------------------------------------------------------
    /**
     What should be done when a note starts

     @param midiNoteNumber
     @param velocity
     @param SynthesiserSound unused variable
     @param / unused variable
     */
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {
        playing = true;
        
        std::cout<< "midiNote is " <<midiNoteNumber<<"\n\n";
        //midiProcessor.process(midiNoteNumber,velocity);//---*process the incoming MIDI messages
        
        float rootFreq = midiToFrequency(root + octaves(1));
        float fifthFreq = midiToFrequency(fifth + octaves(1));
        float guideTonesFreq[2] = {midiToFrequency(guideTones[0] + octaves(2)),midiToFrequency(guideTones[1] + octaves(2))};
        float extensionsFreq[2] = {midiToFrequency(extensions[0] + octaves(4)),midiToFrequency(extensions[2] + octaves(4))};

        myTriOsc1.setFrequency(rootFreq);
        myTriOsc2.setFrequency(fifthFreq);
        myTriOsc3.setFrequency(guideTonesFreq[0]);
        myTriOsc4.setFrequency(guideTonesFreq[1]);
                
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
        clearCurrentNote();
        playing = false;
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
            // iterate through the necessary number of samples (from startSample up to startSample + numSamples)
            for (int sampleIndex = startSample;   sampleIndex < (startSample+numSamples);   sampleIndex++)
            {
                // your sample-by-sample DSP code here!
                // An example white noise generater as a placeholder - replace with your own code
                float currentSample = 0;
                
                voices.push_back(myTriOsc1.process());
                voices.push_back(myTriOsc2.process());
                voices.push_back(myTriOsc3.process());
                voices.push_back(myTriOsc4.process());
                
                
                for(int i = 0 ; i < voices.size(); i++)
                {
                    currentSample+=voices[i];
                }
                
                currentSample /= voices.size();
                currentSample *=protectionGain;
                
                // for each channel, write the currentSample float to the output
                for (int chan = 0; chan<outputBuffer.getNumChannels(); chan++)
                {
                    // The output sample is scaled by 0.2 so that it is not too loud by default
                    outputBuffer.addSample (chan, sampleIndex, currentSample * 0.2);
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
     @return sound cast as a pointer to an instance of ComputerSynthSound
     */
    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<ComputerSynthSound*> (sound) != nullptr;
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    // Set up any necessary variables here
    /// Should the voice be playing?
    int root = 0;
    
    int fifth = 7;
    
    int guideTones[2] = {4,11};
    
    int extensions[3] = {2,5,9};
    
    float protectionGain = 0.9;
    
    bool playing = false;
    MidiProcessor midiProcessor;
    /// a random object for use in our test noise function
    juce::Random random;
    
    TriOsc myTriOsc1;
    TriOsc myTriOsc2;
    TriOsc myTriOsc3;
    TriOsc myTriOsc4;

    std::vector<float> voices;

    
    
    
    
    

};
