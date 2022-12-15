/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <map>          //needed to generate a map for key signature accidentals
#include "MidiProcessor.h"
#include "harmonyResolver.h"
#include "ComputerSynthesiser.h"
#include "TestSynthesiser.h"
#include "HumanSynthesiser.h"
#include "Oscillators.h"
#include "debugResolutionTool.h"

//==============================================================================
/**
*/
class HummingBotAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    HummingBotAudioProcessor();
    ~HummingBotAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    
    float midiToFrequency( int midiValue){
        //float floatMidiValue = (float) midiValue;
        float frequencyValue = 440.0f * pow(2.0f, ((midiValue - 69.0f) / 12.0f));   // MIDI to Hz conversion formula: 440 * 2^[(d - 69) / 12]
        return frequencyValue;
    };

    int octaves (int octNum)
    {
        return octNum * 12;
    }
    
    
private:
    
    
    
    juce::AudioProcessorValueTreeState generalParameters;
    
    juce::AudioProcessorValueTreeState synthParameters;
    
    std::atomic<float>* detuneParam;
    

    juce::Synthesiser synth;

    SineOsc mySineOsc;
    

    int testVoiceCount = 16;
    
    MidiProcessor midiProcessor;
    HarmonyResolver hr;
    std::vector<std::string> possibleHarmonies;

    //DSP LOOP STUFF
    bool playing = false;
    
    int root = 0;
    int fifth = 7;
    int guideTones[2] = {4,11};
    int extensions[3] = {2,5,9};
    
    int harmonyArray[7];
    
    float rootFreq;
    float fifthFreq;
    float guideTonesFreq[2];
    float extensionsFreq[3];

    std::vector<float> samples;

    /// a random object for use in our test noise function
    juce::Random random;
    
    Envelope bassOscEnvelope;
    
    TriOsc bassOsc;
    float bassGain = 1.0f;
    
    Envelope chordOscEnvelope;
    
    SineOsc chordOsc1;
    SineOsc chordOsc2;
    SineOsc chordOsc3;
    float chordGain = 1.0f;

    
    SineOsc myOsc5;
    SineOsc myOsc6;
    
    
    //parameters
    
    std::atomic<float>* bassGainParam;
    std::atomic<float>* chordGainParam;

    
    
    //debug resolution
    
    DebugResolutionTool drt;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HummingBotAudioProcessor)
};
