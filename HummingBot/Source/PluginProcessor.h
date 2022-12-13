/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <map>
#include "MidiProcessor.h"
#include "harmonyResolver.h"
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

    std::string findNearestHarmony(std::string currentHarmony, std::vector<std::string> possibleHarmonies, std::string mode = "nearest")
    {
        std::vector<int> harmonyDistances;//this vector will collect all the result of the harmonic distances calculations
        int currentHarmonyAccidentals = accidentals[currentHarmony]; //how many accidentals does my current harmony have?
        
        std::cout << "my current harmony is " << currentHarmony <<"\n\n";
        
        for (int i = 0; i < possibleHarmonies.size(); i++)
        {
            std::cout << "checking distance with " << possibleHarmonies[i] <<"\n\n";

            std::string checkingHarmony = possibleHarmonies[i]; //select a harmony from the vector of possible harmonies
            int checkingHarmonyAccidentals = accidentals[checkingHarmony];//how many accidentals does the harmony I am currently chacking have?
            
            //check distance from current harmony and checking harmony going flat
            bool foundFlatDistance = false;
            int checkFlatDistance = currentHarmonyAccidentals;
            int flatDistance = 0;

            while(foundFlatDistance == false)
            {
                if(checkFlatDistance == -7)
                    checkFlatDistance = 5;
                
                if(checkingHarmonyAccidentals == checkFlatDistance)
                    foundFlatDistance = true;
                else
                {
                    flatDistance++;
                    checkFlatDistance--;
                }
            }
            
            
            //check distance from current harmony and checking harmony going sharp
            bool foundSharpDistance = false;
            int checkSharpDistance = currentHarmonyAccidentals;
            int sharpDistance = 0;

            while(foundSharpDistance == false)
            {
                if(checkSharpDistance == 6)
                    checkSharpDistance = -6;
                
                if(checkingHarmonyAccidentals == checkSharpDistance)
                    foundSharpDistance = true;
                else
                {
                    sharpDistance++;
                    checkSharpDistance++;
                }
            }
            
            //wich one of the two was shorter?
            if (sharpDistance<flatDistance)
                harmonyDistances.push_back(sharpDistance); //sharp distance was smaller, so we will record this value
            else
                harmonyDistances.push_back(flatDistance); //flat distance was smaller, so we will record this value

            


            
        }
        
        
    }
    
private:
    
    
    MidiProcessor midiProcessor;
    HarmonyResolver hr;
    
    std::map<std::string, int> accidentals = {
        {"C",0}, {"F",-1}, {"G",1}, {"Bb",-2}, {"D",2}, {"Eb",-3}, {"A",3}, {"Ab",-4}, {"E",4}, {"Db",-5}, {"B",5}, {"Gb",-6}
    };
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HummingBotAudioProcessor)
};
