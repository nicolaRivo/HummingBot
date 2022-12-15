/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HummingBotAudioProcessor::HummingBotAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
apvts(*this, nullptr, "ParamTreeIdentifier", {
    std::make_unique<juce::AudioParameterFloat>("bass_gain", "Bass Gain", 0.00001f, 1.0f, 0.7f),
    std::make_unique<juce::AudioParameterFloat>("chord_gain", "Chord Gain", 0.00001f, 1.0f, 0.7f)

})



{
      //===========//
     //Constructor//
    //===========//

    //Parameters//

    bassGainParam  = apvts.getRawParameterValue("bass_gain");
    chordGainParam = apvts.getRawParameterValue("chord_gain");

    
    //Synth Voice Allocation//


        for (int i = 0 ; i < testVoiceCount; i++)
        {
            synth.addVoice(new TestSynthVoice);
        }
      synth.addSound (new TestSynthSound);

}

HummingBotAudioProcessor::~HummingBotAudioProcessor()
{
}

//==============================================================================

/*
 ==================
||PREPARE TO PLAY ||
 ==================
*/

void HummingBotAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
        synth.setCurrentPlaybackSampleRate(sampleRate);
    
        for (int i = 0; i < testVoiceCount; i++)
        {
            TestSynthVoice* v = dynamic_cast<TestSynthVoice*>(synth.getVoice(i));
            v->init(sampleRate);
        }
    
    
    //DSP LOOP STUFF
    bassOsc.setSampleRate(sampleRate);
    bassOscEnvelope.setSampleRate(sampleRate);
    bassOscEnvelope.toggleRetrigger(true);
    
    chordOscEnvelope.setSampleRate(sampleRate);
    chordOscEnvelope.setParameters(0.3, 1, 0.6, 3, 3);
    chordOscEnvelope.toggleRetrigger(true);

    chordOsc1.setSampleRate(sampleRate);
    chordOsc2.setSampleRate(sampleRate);
    chordOsc3.setSampleRate(sampleRate);
    
    myOsc5.setSampleRate(sampleRate);
    myOsc6.setSampleRate(sampleRate);
    
    
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

/*
 ================
||PROCESS BLOCK ||
 ================
*/

void HummingBotAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    
    
    //    juce::ScopedNoDenormals noDenormals;
    int numSamples = buffer.getNumSamples();
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);

    
    
    buffer.clear();//------------------------*clear the MIDI  buffer of any potential MIDI  that remained there from previous cycle
 
    int arr[7];
    
    midiProcessor.process(midiMessages, arr);//---*process the incoming MIDI messages
    int sumArr=0;
    
    for(int i = 0; i<7;i++)
        sumArr += arr[i];
   
    
    if(sumArr<30 && sumArr>10)
    {
        std::cout<< "\n\n SumArray is "<< sumArr<<" \n\n";

        for(int i = 0; i<7;i++)
        {
            harmonyArray[i] = arr[i];
            std::cout<< "\n\n I am in the .cpp file "<< arr[i]<<" \n\n";
        }
        
        root = harmonyArray[0];
        fifth = harmonyArray[1];
        guideTones[0] = harmonyArray[2];
        guideTones[1] = harmonyArray[3];
        extensions[0] = harmonyArray[4];
        extensions[1] = harmonyArray[5];
        extensions[2] = harmonyArray[6];
        
    }

   
    //computerSynth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    //humanSynth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    
    
    
    

    rootFreq = midiToFrequency(root + octaves(4));
    fifthFreq = midiToFrequency(fifth + octaves(5));
    guideTonesFreq[0] = midiToFrequency(guideTones[0] + octaves(5));
    guideTonesFreq[1] = midiToFrequency(guideTones[1] + octaves(6));
    extensionsFreq[0] = midiToFrequency(extensions[0] + octaves(8));
    extensionsFreq[1] = midiToFrequency(extensions[1] + octaves(8));
    extensionsFreq[2] = midiToFrequency(extensions[2] + octaves(8));

    
    
    bassOsc.setFrequency(midiToFrequency(root + octaves(4)));
   // myOsc1.setFrequency(rootFreq);

    chordOsc1.setFrequency(midiToFrequency(root + octaves(5)));
    chordOsc2.setFrequency(midiToFrequency(fifth + octaves(5)));
    chordOsc3.setFrequency(midiToFrequency(guideTones[0] + octaves(5)));
    
    
    
    myOsc5.setFrequency(extensionsFreq[0]);
    myOsc6.setFrequency(extensionsFreq[1]);
    
    /*
     ===========
    ||DSP LOOP ||
     ===========
    */
    
    for (int i = 0; i < numSamples; i++)
    {
        
        // your sample-by-sample DSP code here!
        // An example white noise generater as a placeholder - replace with your own code
        
        
        
        float currentSample = 0.0f;//main sample. It will eventually hold together all the samples end be output to the audioBuffer
        
        float processedBassOsc = bassOsc.process();
        float processedBassOscEnvelope =bassOscEnvelope.process();
        processedBassOscEnvelope *= (*bassGainParam);
        
        std::cout << drt.nextStep("bass gain value is", *bassGainParam);
        
        

        float processedChordOsc1 = chordOsc1.process();
        float processedChordOsc2 = chordOsc2.process();
        float processedChordOsc3 = chordOsc3.process();

        float processedChordOscEnvelope = chordOscEnvelope.process();
        
        processedChordOscEnvelope *= (*chordGainParam);
        
        std::cout << drt.nextStep("chord gain value is", *chordGainParam);
        
        
        
        processedBassOsc *= processedBassOscEnvelope;
        
         processedChordOsc1 *= processedChordOscEnvelope;
         processedChordOsc2 *= processedChordOscEnvelope;
         processedChordOsc3 *= processedChordOscEnvelope;
        
        
       samples.push_back(processedBassOsc * 0.2);
        samples.push_back(processedChordOsc1);
        samples.push_back(processedChordOsc2);
        samples.push_back(processedChordOsc3);
        
        //samples.push_back(myOsc5.process());
        //samples.push_back(myOsc6.process());

        
        for (int i = 0; i < samples.size(); i++)
        {
            currentSample += samples[i];
        }

        currentSample /= samples.size();

        //currentSample=0.0f;
        
        samples.clear();
        
        leftChannel[i]  += currentSample;
        rightChannel[i] += currentSample;

    }

}





const juce::String HummingBotAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool HummingBotAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool HummingBotAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool HummingBotAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double HummingBotAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int HummingBotAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int HummingBotAudioProcessor::getCurrentProgram()
{
    return 0;
}

void HummingBotAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String HummingBotAudioProcessor::getProgramName (int index)
{
    return {};
}

void HummingBotAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================

void HummingBotAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool HummingBotAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif



//==============================================================================
bool HummingBotAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* HummingBotAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void HummingBotAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // getStateInformation
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void HummingBotAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // setStateInformation
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName (apvts.state.getType()))
        {
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HummingBotAudioProcessor();
}
