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
generalParameters(*this, nullptr, "GeneralParameters", {
    std::make_unique<juce::AudioParameterFloat>("bass_gain",   "Arrangement : Bass Gain",   0.00001f, 1.0f, 0.7f),
    std::make_unique<juce::AudioParameterFloat>("chord_gain",  "Arrangement : Chords Gain", 0.00001f, 1.0f, 0.7f),
    std::make_unique<juce::AudioParameterFloat>("reverb_amount",    "Reverb : Amount",      0.00001f, 1.0f, 0.3f),
    std::make_unique<juce::AudioParameterFloat>("reverb_size",      "Reverb : Size",        0.00001f, 1.0f, 0.3f),


}),
synthParameters(*this, nullptr, "SynthParameters", {
    
    std::make_unique<juce::AudioParameterFloat>("voice_gain",       "Voice  : Gain",     0.0f,  1.0f, 0.4f),
    std::make_unique<juce::AudioParameterFloat>("voice_detune",      "Voice : Detune",   0.0f, 20.0f, 2.0f),
    
    
    std::make_unique<juce::AudioParameterFloat>("amp_attack",   "AMP : Attack",     0.0001f, 3.0f, 0.5f),
    std::make_unique<juce::AudioParameterFloat>("amp_decay",    "AMP : Decay",      0.0001f, 3.0f, 0.1f),
    std::make_unique<juce::AudioParameterFloat>("amp_sustain",  "AMP : Sustain",    0.0001f, 1.00f, 0.5f),
    std::make_unique<juce::AudioParameterFloat>("amp_release",  "AMP : Release",    0.0001f, 3.0f, 0.5f)

})




{
      //=================//
     //~~~CONSTRUCTOR~~~//
    //=================//

    /*--GENERAL parameters--*/

    bassGainParam  = generalParameters.getRawParameterValue("bass_gain");
    chordGainParam = generalParameters.getRawParameterValue("chord_gain");

    reverbAmountParam  = generalParameters.getRawParameterValue("reverb_amount");
    reverbSizeParam = generalParameters.getRawParameterValue("reverb_size");
    
    /*--SYNTH parameters--*/

    synthVoiceGainParam = synthParameters.getRawParameterValue("voice_gain");
    detuneParam = synthParameters.getRawParameterValue("voice_detune");

    
    ampAttackParam  =   synthParameters.getRawParameterValue("amp_attack");
    ampDecayParam   =   synthParameters.getRawParameterValue("amp_decay");
    ampSustainParam =   synthParameters.getRawParameterValue("amp_sustain");
    ampReleaseParam =   synthParameters.getRawParameterValue("amp_release");


    
    
    

    
       //============================//
      //~~~SYNTH VOICE ALLOCATION~~~//
     //============================//
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
    
    extensionOsc1.setSampleRate(sampleRate);
    extensionOsc2.setSampleRate(sampleRate);
    
    
    /*--SMOOTHING parameters--*/
    smoothBassGain.reset(sampleRate, smoothRampingTime);
    smoothBassGain.setCurrentAndTargetValue(0.0);
    
    smoothChordGain.reset(sampleRate, smoothRampingTime);
    smoothChordGain.setCurrentAndTargetValue(0.0);

    smoothReverbAmount.reset(sampleRate, smoothRampingTime);
    smoothReverbAmount.setCurrentAndTargetValue(0.0);
    
    smoothReverbSize.reset(sampleRate, smoothRampingTime);
    smoothReverbSize.setCurrentAndTargetValue(0.0);
    
    
    
    //General Reverb
    generalReverb.reset();
}

/*
 ================
||PROCESS BLOCK ||
 ================
*/

void HummingBotAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    
    
    //    juce::ScopedNoDenormals noDenormals;
    int numSamples = buffer.getNumSamples();//----------->get the number of samples for each block
    float* leftChannel = buffer.getWritePointer(0);//---->get pointer to the left channel
    float* rightChannel = buffer.getWritePointer(1);//--->get pointer to the right channel
    buffer.clear();//------------------------------------>clear the MIDI  buffer of any potential MIDI  that remained there from previous cycle

    
    
    
    /*--SMOOTHING parameters--*/
    
    smoothBassGain.setTargetValue(*bassGainParam);
    smoothChordGain.setTargetValue(*chordGainParam);
    
    smoothReverbAmount.setTargetValue(*reverbAmountParam);
    smoothReverbSize.setTargetValue(*reverbSizeParam);
    
    
 
    int harmonyNotes[7];
    
    
    midiProcessor.setPrioritizeKeyChange(prioritiseKeyChange);
    
    midiProcessor.process(midiMessages, harmonyNotes, chordDegreePointer);//---*process the incoming MIDI messages
    int sumArr=0;
    
    for(int i = 0; i<7;i++)
        sumArr += harmonyNotes[i];
   
    
    if(sumArr<100 && sumArr>0)
    {
        std::cout<< "\n\n I am in the .cpp file, new chord degree is "<< chordDegree <<" \n\n";

        for(int i = 0; i<7;i++)
        {
            harmonyArray[i] = harmonyNotes[i];
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
    
    for (int i = 0; i < testVoiceCount; i++)
    {
        TestSynthVoice* v = dynamic_cast<TestSynthVoice*>(synth.getVoice(i));
        v->setDetune(*detuneParam);
    }
    
    for (int i = 0; i < testVoiceCount; i++)
    {
        TestSynthVoice* v = dynamic_cast<TestSynthVoice*>(synth.getVoice(i));
        v->setSynthVoiceGain(*synthVoiceGainParam);
    }
    
    
    for (int i = 0; i < testVoiceCount; i++)
    {
        TestSynthVoice* v = dynamic_cast<TestSynthVoice*>(synth.getVoice(i));
        v->setADSR(*ampAttackParam, *ampDecayParam, *ampSustainParam, *ampReleaseParam);
    }
    
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
    
    
    
    extensionOsc1.setFrequency(extensionsFreq[0]);
    extensionOsc2.setFrequency(extensionsFreq[1]);
    
    /* ===============
    ||DSP LOOP start||
     ===============*/
    
    for (int i = 0; i < numSamples; i++)
    {
        
        // your sample-by-sample DSP code here!
        // An example white noise generater as a placeholder - replace with your own code
        
        
        /*--PROCESS smooth parameters--*/

        float bassGainSmoothedVal = smoothBassGain.getNextValue();
        float chordGainSmoothedVal = smoothChordGain.getNextValue();
        float reverbAmountSmoothedVal = smoothReverbAmount.getNextValue();
        float reverbSizeSmoothedVal = smoothReverbSize.getNextValue();
        
        
        //set global reverb
        
        int revCurrentLevels = (reverbParams.wetLevel * reverbParams.roomSize); // OPTIMIZATION, MIGHT NOT WORK YET: initialize value to be check in order to update the reverb parameters
        
        if(revCurrentLevels != revCheckLevels)
        {
            reverbParams.dryLevel = 1.0f - reverbAmountSmoothedVal/2.0f;
            reverbParams.wetLevel = reverbAmountSmoothedVal;
            reverbParams.roomSize = reverbSizeSmoothedVal;
            generalReverb.setParameters(reverbParams);
            revCheckLevels = (reverbParams.wetLevel * reverbParams.roomSize); // OPTIMIZATION, MIGHT NOT WORK YET: initialize value to be check in order to update the
        }
        
        
        /*========================================
        ||Start adding data to the current sample||
         ========================================*/
        
        float currentSample = 0.0f;//main sample. It will eventually hold together all the samples end be output to the audioBuffer
        
        float processedBassOsc = bassOsc.process();
        float processedBassOscEnvelope =bassOscEnvelope.process();
        processedBassOscEnvelope *= bassGainSmoothedVal;
        
        

        float processedChordOsc1 = chordOsc1.process();
        float processedChordOsc2 = chordOsc2.process();
        float processedChordOsc3 = chordOsc3.process();

        float processedChordOscEnvelope = chordOscEnvelope.process();
        
        processedChordOscEnvelope *= chordGainSmoothedVal;
        
        
        
        
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

        currentSample *= genProtectionGain;
        
        samples.clear();
        
        leftChannel[i]  += currentSample;
        rightChannel[i] += currentSample;

    }
    
    
    /* =============
    ||DSP LOOP end||
     =============*/
    
    
    //apply global reverb
       generalReverb.processStereo(leftChannel, rightChannel, numSamples);

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
    auto state = generalParameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void HummingBotAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // setStateInformation
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName (generalParameters.state.getType()))
        {
            generalParameters.replaceState (juce::ValueTree::fromXml (*xmlState));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HummingBotAudioProcessor();
}
