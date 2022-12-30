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
#include "TestSynthesiser.h"
#include "Oscillators.h"
#include "debugResolutionTool.h"
#include "DelayLine.h"

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
    bool debug = 0;
    
    bool allowSynthNotes = true;
    bool * allowSynthNotesPointer = &allowSynthNotes;
    
    std::vector<float> samples;//Central samples
    std::vector<float> leftSamples; //Right samples
    std::vector<float> rightSamples; //Left samples

    std::vector<std::string> possibleHarmonies;
    

    bool delayedExtensionOneIsOver = 1;
    bool delayedExtensionTwoIsOver = 1;
    
    //HARMONY SWAP VARIABLES
    bool harmonySwitchFirstCycle = false;
    bool *harmonySwitchFirstCyclePointer = &harmonySwitchFirstCycle;
    
    float harmonySwitchingTime = 1.0f;
    
    Envelope concludeOldHarmony;
    float processedConcludeOldHarmony = 0.2f;
    bool concludingOldHarmony = false;
    
    Envelope beginNewHarmony;
    float processedBeginNewHarmony;
    bool beginningNewHarmony = false;
    
    
    bool beginHarmonySwitch = false;
    bool* beginHarmonySwitchPointer = &beginHarmonySwitch;
    
    
    /*--HARMONY variables--*/
    int harmonyArray[7];

    int root = 0;
    int fifth = 7;
    int guideTones[2] = {4,11};
    int extensions[2] = {2,9};
    
    float rootFreq;
    float fifthFreq;
    float guideTonesFreq[2];
    float extensionsFreq[2];

    
    int chordDegree;
    int * chordDegreePointer = &chordDegree;
    
    bool prioritiseKeyChange;

    
    /*--HARMONY SWAP variables--*/

    bool bassIsRoot = true;
    bool bassWasAudible = false;
    bool bassFirstCycle = true;
    
    bool chordWasAudible = true;
    
    bool extensionOneWasAudible = true;
    
    bool extensionTwoWasAudible = true;
    bool extensionTwoFirstCycle = true;
    


    /*--HARMONY Processed envelopes--*/

    float processedBassOscEnvelope;
    float processedChordOscEnvelope;
    float processedExtensionOneOscEnvelope;
    float processedExtensionTwoOscEnvelope;

    

    
    /*--GENERAL oscillators--*/

    TriOsc bassOsc;
    SineOsc chordOsc1, chordOsc2, chordOsc3, chordOsc4;
    SineOsc extensionOsc1, extensionOsc2;


    /*--GENERAL envelopes--*/
    //these envelopes bring different parts of the arrangement on and off slowly overtime to contribute to the generative nature of the accompainment music
    
    Envelope bassOscEnvelope;
    Envelope chordOscEnvelope;
    Envelope extensionOneOscEnvelope;
    Envelope extensionTwoOscEnvelope;

    
    /*--GENERAL LFOs--*/

    LFO extensionOneDelayPanLFO;
    LFO extensionTwoDelayPanLFO;
    LFO chordsFilterLFO;
    
    
    /*--GENERAL Filters--*/
    
    juce::IIRFilter chordsFilter;

    
    /*--GENERAL VRBs & DLYs--*/
    juce::Reverb generalReverb;
    juce::Reverb::Parameters reverbParams;
    bool includeSynthInReverb = 1;
    float revCheckLevels; // this is a useful optimization variable, eventually it could be extended to all the parameters. Is used in the PluginProcessor.cpp to hold the value of all the reverb parameters multipiled toghether. This is then compared to another similar snapshot of all the reverb parameters and if the two values are identical, it means that the system doesn't have to update the parameters each cycle of the DSP loop
    
    juce::Reverb synthReverb;
    juce::Reverb::Parameters synthReverbParams;
    
    DelayLine extensionOneDelay;
    DelayLine extensionTwoDelay;

    


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
    
    //juce::AudioProcessorValueTreeState synthParameters;
    
    std::atomic<float>* oscShapeParam;
    
    std::atomic<float>* detuneParam;
    
    std::atomic<float>* synthVoiceGainParam;
    juce::SmoothedValue<float> smoothSynthVoiceGain;

    
    std::atomic<float>* ampAttackParam;
    std::atomic<float>* ampDecayParam;
    std::atomic<float>* ampSustainParam;
    std::atomic<float>* ampReleaseParam;
    std::atomic<float>* includeSynthInReverbParam;

    
//    std::atomic<float>* synthReverbDryParam;
//    std::atomic<float>* synthReverbWetParam;
//    std::atomic<float>* synthReverbSizeParam;
//
//    std::atomic<float>* delayTimeParam;
//    std::atomic<float>* delayFeedbackParam;
//

    /*--MISC--*/
    double smoothRampingTime = 0.1;//------>time ramp for smothed values
    DebugResolutionTool drt;//------------->generic debug tool
    float sr;//---------------------------->project sample rate
    float genProtectionGain = 0.3f;//------>protection gain applied at the and of the general DSP loop
    juce::Random random;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HummingBotAudioProcessor)
};
