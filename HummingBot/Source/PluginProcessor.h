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
#include "DelayLine.h";

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
    
    /*--FUNDAMENTAL variables--*/
    
    juce::Synthesiser synth;
    int testVoiceCount = 16;
    MidiProcessor midiProcessor;
    HarmonyResolver hr;
    
    bool allowSynthNotes = true;
    bool * allowSynthNotesPointer = &allowSynthNotes;
    
    std::vector<float> samples;
    std::vector<std::string> possibleHarmonies;
    
    Envelope concludeOldHarmony;
    float processedConcludeOldHarmony = 0.2f;
    bool concludingOldHarmony = false;
    
    Envelope beginNewHarmony;
    float processedBeginNewHarmony;
    bool beginningNewHarmony = false;
    
    
    bool beginHarmonySwitch;
    bool* beginHarmonySwitchPointer = &beginHarmonySwitch;
    
    
    /*--HARMONY variables--*/
    int harmonyArray[7];

    int root = 0;
    int fifth = 7;
    int guideTones[2] = {4,11};
    int extensions[2] = {2,10};
    
    float rootFreq;
    float fifthFreq;
    float guideTonesFreq[2];
    float extensionsFreq[2];

    
    int chordDegree;
    int * chordDegreePointer = &chordDegree;
    
    bool prioritiseKeyChange;

    bool bassIsRoot = true;
    bool bassWasAudible = false;
    bool bassFirstCycle = true;
    
    bool chordWasAudible = false;
    bool chordFirstCycle = true;
    
    bool extensionOneWasAudible = false;
    bool extensionOneFirstCycle = true;
    bool playExtensionOne = true;
    
    bool extensionTwoWasAudible = false;
    bool extensionTwoFirstCycle = true;
    bool playExtensionTwo = true;


    
    float processedBassOscEnvelope;
    float processedChordOscEnvelope;
    float processedExtensionOneOscEnvelope;
    float processedExtensionTwoOscEnvelope;

    
    
    //chordDegreePointer = &chordDegree;

    
    /*--GENERAL oscillators--*/

    TriOsc bassOsc;
    SineOsc chordOsc1, chordOsc2, chordOsc3, chordOsc4;
    SineOsc extensionOsc1, extensionOsc2;


    /*--GENERAL envelopes--*/
    
    Envelope chordOscEnvelope;
    Envelope bassOscEnvelope;
    Envelope extensionOneOscEnvelope;
    Envelope extensionTwoOscEnvelope;


    
    /*--VRBs & DLYs--*/
    juce::Reverb generalReverb;
    juce::Reverb::Parameters reverbParams;
    float revCheckLevels; // OPTIMIZATION, MIGHT NOT WORK YET: initialize value to be check in order to update the reverb parameters

    DelayLine extensionDelay;
    


      //==============//
     //~~PARAMETERS~~//
    //==============//
    
     /*--GENERAL parameters--*/
    
    juce::AudioProcessorValueTreeState generalParameters;
    
    std::atomic<float>* bassGainParam;
    juce::SmoothedValue<float> smoothBassGain;
    
    std::atomic<float>* chordGainParam;
    juce::SmoothedValue<float> smoothChordGain;
    
    std::atomic<float>* extensionGainParam;
    juce::SmoothedValue<float> smoothExtensionGain;
    
    
    std::atomic<float>* reverbAmountParam;
    juce::SmoothedValue<float> smoothReverbAmount;
    
    std::atomic<float>* reverbSizeParam;
    juce::SmoothedValue<float> smoothReverbSize;
    
    
    std::atomic<float>* prioritiseKeyChangeParam;


    /*--SYNTH parameters--*/
    
    juce::AudioProcessorValueTreeState synthParameters;
    
    std::atomic<float>* detuneParam;
    
    std::atomic<float>* synthVoiceGainParam;

    
    std::atomic<float>* ampAttackParam;
    std::atomic<float>* ampDecayParam;
    std::atomic<float>* ampSustainParam;
    std::atomic<float>* ampReleaseParam;

    
    

    /*--MISC--*/
    double smoothRampingTime = 0.1;//------>time ramp for smothed values
    DebugResolutionTool drt;//------------->generic debug tool
    float sr;//---------------------------->project sample rate
    float genProtectionGain = 0.3f;//------>protection gain applied at the and of the general DSP loop
    juce::Random random;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HummingBotAudioProcessor)
};
