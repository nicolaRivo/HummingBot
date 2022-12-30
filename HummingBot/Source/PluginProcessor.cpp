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
    std::make_unique<juce::AudioParameterFloat>("bass_gain",                    "Arrangement : Bass Gain",          0.00001f, 1.0f, 0.7f),
    std::make_unique<juce::AudioParameterFloat>("chord_gain",                   "Arrangement : Chord Gain",         0.00001f, 1.0f, 0.7f),
    std::make_unique<juce::AudioParameterFloat>("extension_gain",               "Arrangement : Extensions Gain",    0.00001f, 1.0f, 0.7f),
    std::make_unique<juce::AudioParameterChoice>("prioritise_key_change",       "Arrangement : Prioritise Key Change", juce::StringArray({"No","Yes"}), 1),
    std::make_unique<juce::AudioParameterFloat>("reverb_amount",                     "Reverb : Amount",             0.00001f, 1.0f, 0.3f),
    std::make_unique<juce::AudioParameterFloat>("reverb_size",                       "Reverb : Size",               0.00001f, 1.0f, 0.3f),

//synthParameters
    
    std::make_unique<juce::AudioParameterChoice>("osc_shape",       "Voice : Osc Shape", juce::StringArray({"SawTooth","Square", "Sine", "Triangle"}), 1),
    
    std::make_unique<juce::AudioParameterFloat>("voice_gain",       "Voice  : Gain",     0.0f,  1.0f, 0.4f),
    std::make_unique<juce::AudioParameterFloat>("voice_detune",      "Voice : Detune",   0.0f, 20.0f, 2.0f),
    
    
    std::make_unique<juce::AudioParameterFloat>("amp_attack",           "AMP : Attack",     0.01f, 3.0f, 0.5f),
    std::make_unique<juce::AudioParameterFloat>("amp_decay",            "AMP : Decay",      0.01f, 3.0f, 0.1f),
    std::make_unique<juce::AudioParameterFloat>("amp_sustain",          "AMP : Sustain",    0.0001f, 1.00f, 0.5f),
    std::make_unique<juce::AudioParameterFloat>("amp_release",          "AMP : Release",    0.01f, 3.0f, 0.5f),
    std::make_unique<juce::AudioParameterChoice>("rev_include_synth", "REV : Apply to synth", juce::StringArray({"No","Yes"}), 1)

    
    
    
//    std::make_unique<juce::AudioParameterFloat>("delay_time",       "Delay  : Time",     0.0f,  1.0f, 0.4f),
//    std::make_unique<juce::AudioParameterFloat>("delay_feedback",   "Delay  : Feedback", 0.0f,  1.0f, 0.4f),
//
//    std::make_unique<juce::AudioParameterFloat>("syth_reverb_dry",   "Reverb  : Dry", 0.0f,  1.0f, 0.4f),
//    std::make_unique<juce::AudioParameterFloat>("syth_reverb_wet",   "Reverb  : Wet", 0.0f,  1.0f, 0.4f),
//    std::make_unique<juce::AudioParameterFloat>("syth_reverb_size",   "Reverb  : Size", 0.0f,  1.0f, 0.4f)

})




{
      //=================//
     //~~~CONSTRUCTOR~~~//
    //=================//

    /*--GENERAL parameters--*/

    bassGainParam  = generalParameters.getRawParameterValue("bass_gain");
    chordGainParam = generalParameters.getRawParameterValue("chord_gain");
    extensionGainParam = generalParameters.getRawParameterValue("extension_gain");


    reverbAmountParam  = generalParameters.getRawParameterValue("reverb_amount");
    reverbSizeParam = generalParameters.getRawParameterValue("reverb_size");
    
    prioritiseKeyChangeParam = generalParameters.getRawParameterValue("prioritise_key_change");
    
    /*--SYNTH parameters--*/
    synthVoiceGainParam = generalParameters.getRawParameterValue("voice_gain");
    detuneParam = generalParameters.getRawParameterValue("voice_detune");

    
    ampAttackParam  =   generalParameters.getRawParameterValue("amp_attack");
    ampDecayParam   =   generalParameters.getRawParameterValue("amp_decay");
    ampSustainParam =   generalParameters.getRawParameterValue("amp_sustain");
    ampReleaseParam =   generalParameters.getRawParameterValue("amp_release");
    oscShapeParam   =   generalParameters.getRawParameterValue("osc_shape");

    includeSynthInReverbParam   =   generalParameters.getRawParameterValue("rev_include_synth");

//    delayTimeParam      = generalParameters.getRawParameterValue("delay_time");
//    delayFeedbackParam  = generalParameters.getRawParameterValue("delay_feedback");
//
//    synthReverbDryParam  = generalParameters.getRawParameterValue("syth_reverb_dry");
//    synthReverbWetParam  = generalParameters.getRawParameterValue("syth_reverb_wet");
//    synthReverbSizeParam  = generalParameters.getRawParameterValue("syth_reverb_size");

    
    
    

    
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
    
        sr = sampleRate; //set project sample rate in higher scope variable
    
        for (int i = 0; i < testVoiceCount; i++)
        {
            TestSynthVoice* v = dynamic_cast<TestSynthVoice*>(synth.getVoice(i));
            v->init(sampleRate);
        }
    
    
    
    
    //DSP LOOP INITIALIZATIONS
    
    
    /*--initialise HARMONY SWAP ENVELOPES--*/
    concludeOldHarmony.setSampleRate(sampleRate);
    concludeOldHarmony.setParameters(0.001f, 0.001f, 1.0f, 1.0f, harmonySwitchingTime);
    concludeOldHarmony.reset();
    
    beginNewHarmony.setSampleRate(sampleRate);
    beginNewHarmony.setParameters(harmonySwitchingTime, 0.001f, 1.0f, 0.0f, 0.0f);
    beginNewHarmony.reset();
    
    /*--initialise OSCILLATORS--*/
    bassOsc.setSampleRate(sampleRate);
    bassOscEnvelope.setSampleRate(sampleRate);
    bassOscEnvelope.toggleRetrigger(true);
    
    chordOscEnvelope.setSampleRate(sampleRate);
    chordOscEnvelope.setParameters(0.3, 1, 0.6, 3, 3);
    chordOscEnvelope.toggleRetrigger(true);
    
    extensionOneOscEnvelope.setSampleRate(sampleRate);
    extensionOneOscEnvelope.setParameters(0.5, 0.5, 0.6, 0.0, 0.6);
    
    extensionTwoOscEnvelope.setSampleRate(sampleRate);
    extensionTwoOscEnvelope.setParameters(0.5, 0.3, 0.8, 0.0, 0.7);

    chordOsc1.setSampleRate(sampleRate);
    chordOsc2.setSampleRate(sampleRate);
    chordOsc3.setSampleRate(sampleRate);
    chordOsc4.setSampleRate(sampleRate);
    
    extensionOsc1.setSampleRate(sampleRate);
    extensionOsc2.setSampleRate(sampleRate);


    /*--initialise all DELAYS--*/

    extensionOneDelay.setSizeInSamples(sampleRate * 5);
    extensionOneDelay.setDelayTimeInSamples(sampleRate * 0.6);
    extensionOneDelay.setFeedbackAmt(0.8f);
    
    extensionTwoDelay.setSizeInSamples(sampleRate * 5);
    extensionTwoDelay.setDelayTimeInSamples(sampleRate * 1);
    extensionTwoDelay.setFeedbackAmt(0.7f);
    
    /*--initialise all LFOs--*/

    extensionOneDelayPanLFO.setSampleRate(sampleRate);
    extensionOneDelayPanLFO.setFrequency(1.0f);
    extensionOneDelayPanLFO.mapLFO(0.9f);

    
    extensionTwoDelayPanLFO.setSampleRate(sampleRate);
    extensionTwoDelayPanLFO.setFrequency(1.2f);
    extensionTwoDelayPanLFO.mapLFO(0.9f);

    chordsFilterLFO.setSampleRate(sampleRate);
    chordsFilterLFO.setFrequency(0.4);
    chordsFilterLFO.mapLFO(500.0f, -500.0f);

    
    /*--SMOOTHING parameters--*/
    smoothBassGain.reset(sampleRate, smoothRampingTime);
    smoothBassGain.setCurrentAndTargetValue(0.0);
    
    smoothChordGain.reset(sampleRate, smoothRampingTime);
    smoothChordGain.setCurrentAndTargetValue(0.0);

    smoothExtensionGain.reset(sampleRate, smoothRampingTime);
    smoothExtensionGain.setCurrentAndTargetValue(0.0);
    
    smoothReverbAmount.reset(sampleRate, smoothRampingTime);
    smoothReverbAmount.setCurrentAndTargetValue(0.0);
    
    smoothReverbSize.reset(sampleRate, smoothRampingTime);
    smoothReverbSize.setCurrentAndTargetValue(0.0);
    
    smoothSynthVoiceGain.reset(sampleRate, smoothRampingTime);
    smoothSynthVoiceGain.setCurrentAndTargetValue(0.0);
    
    /*--initialise all Filters--*/

    chordsFilter.reset();
    
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
    smoothExtensionGain.setTargetValue(*extensionGainParam);

    
    smoothReverbAmount.setTargetValue(*reverbAmountParam);
    smoothReverbSize.setTargetValue(*reverbSizeParam);
    
    smoothSynthVoiceGain.setTargetValue(*synthVoiceGainParam);
 

    /*=====================
    ||MIDI PROCESSOR begin||
     ======================
     midiProcessor will grab the notes entered while holding down C#1 midi key, it will then find major scales that contains all those notes */
     
    int harmonyNotes[7];
    midiProcessor.setPrioritizeKeyChange(*prioritiseKeyChangeParam);
    midiProcessor.process(midiMessages, harmonyNotes, chordDegreePointer, allowSynthNotesPointer, beginHarmonySwitchPointer, harmonySwitchFirstCyclePointer);//---*process the incoming MIDI messages
    
    int sumArr=0;
    
    for(int i = 0; i<7;i++)
        sumArr += harmonyNotes[i];
   
    
    //if harmonyNotes array contains useful note numbers, and not just the initialization values, populate the harmonyArray[i]
    if (sumArr < 100 && sumArr > 0)
        {
            for(int i = 0; i<7;i++)
            {
                harmonyArray[i] = harmonyNotes[i];
            }
        }
    
    
    
    

    /*===============================
    ||HARMONY SWITCH ALGORYTHM begin||
     ================================
    
    This section is to smoothly switch between harmonies.To do so, it uses two envelopes -concludeOldHarmony and -beginNewHarmony.
     
    A boolean called beginHarmonySwitch is used to know when to initiate this process and this is produced from within the midiProcess function.
     
    Once this process is running, two boolean checks are used to flag the program wich step of this harmony swap we are on: -concludingOldHarmony and -beginningNewHarmony (I am aware ofthe fact that the two names are too similar for pointing radically different kinds of objects, this issue will be addressed in a future update of the code).
     ===================================================================================================================================================================*/
    
    //if we are on the first cycle of the hamrony switch, initialise correctly the node names of concludeOldHarmony and beginNewHarmony.
    if(beginHarmonySwitch && harmonySwitchFirstCycle)
    {
    
        concludeOldHarmony.setNodeName("waiting");
        beginNewHarmony.setNodeName("finished");
        harmonySwitchFirstCycle = false;
    }
    
    
    
    //inform the program of wich stage of switching harmony we are on
    if(beginHarmonySwitch)
    {
        if(concludeOldHarmony.getNodeName()=="waiting")
            concludingOldHarmony=true;
        else if(concludeOldHarmony.getNodeName()=="finished")
            concludingOldHarmony=false;
            
        if(beginNewHarmony.getNodeName()=="waiting")
            beginningNewHarmony = true;
        else if(beginNewHarmony.getNodeName()=="finished")
            beginningNewHarmony = false;
    }
    
    
    
    //if we are switching harmony and the old harmony has been faded out, go ahead and switch harmonies; once thi is done, beginHarmonySwitch will be set to false and this whole chunk of code will be skipped until a new harmony is set
    if (beginHarmonySwitch)
    {
        if (concludeOldHarmony.getNodeName()=="finished")
        {
            concludingOldHarmony = false;
            beginningNewHarmony = true;
            beginNewHarmony.reset();

            if(beginHarmonySwitch){
                //SWITCH THE HARMONY
                bassFirstCycle = true;
                                
                root = harmonyArray[0];
                fifth = harmonyArray[1];
                guideTones[0] = harmonyArray[2];//3rd
                guideTones[1] = harmonyArray[3];//7th
                extensions[0] = harmonyArray[4];//9th
                
                if (chordDegree == 1 || chordDegree == 5 )
                {
                    extensions[1] = harmonyArray[5];//if chord is minor, second extension will be 11th
                } else if (chordDegree == 0 || chordDegree == 4 )
                {
                    extensions[1] = harmonyArray[6];//if chord is minor, second extension will be 13th
                } else if (chordDegree == 2 )
                {
                    extensions[0] += 1; //if we are dealing with the prhigian chord, avoid using the minor 9th as an extension, chose the major 9th instead
                    extensions[1] = harmonyArray[5] ;
                } else if (chordDegree == 3 )
                {
                    extensions[1] = 2; //if we are dealing with the lydian chord, use the #11 as the second extension
                }
                
                beginHarmonySwitch = false;
            }
        }
    }

    
    //update all the variables that contain the frequency corresponding to each tone of the current harmony.
    //there is no need to do this for each block, in future update will be moved inside the previus   --if (beginHarmonySwitch)--  so that will happen only once the harmonyArray has been repopulated with the new harmony.
    

    
    rootFreq = juce::MidiMessage::getMidiNoteInHertz(root + octaves(4));
    fifthFreq = juce::MidiMessage::getMidiNoteInHertz(fifth + octaves(5));
    guideTonesFreq[0] = juce::MidiMessage::getMidiNoteInHertz(guideTones[0] + octaves(5));
    guideTonesFreq[1] = juce::MidiMessage::getMidiNoteInHertz(guideTones[1] + octaves(6));
    extensionsFreq[0] = juce::MidiMessage::getMidiNoteInHertz(extensions[0] + octaves(7));
    extensionsFreq[1] = juce::MidiMessage::getMidiNoteInHertz(extensions[1] + octaves(7));

  
    
    /*=============================
    ||HARMONY SWITCH ALGORYTHM end||
     =============================*/
    
    
    
    //=======================================//
   //~~~SET SYNTH PARAMETER FOR EACH VOICE~~//
  //=======================================//
    //call all the function that will set the synth parameters for each new voice
    
    for (int i = 0; i < testVoiceCount; i++)
    {
        TestSynthVoice* v = dynamic_cast<TestSynthVoice*>(synth.getVoice(i));
        v->setOscShape(*oscShapeParam);
    }

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
    
    
    for (int i = 0; i < testVoiceCount; i++)
    {
        TestSynthVoice* v = dynamic_cast<TestSynthVoice*>(synth.getVoice(i));
        v->setAllowSynthNotes(allowSynthNotes);
    }
    
    
//    for (int i = 0; i < testVoiceCount; i++)
//    {
//        TestSynthVoice* v = dynamic_cast<TestSynthVoice*>(synth.getVoice(i));
//        v->setDelay(*delayTimeParam, *delayFeedbackParam);
//    }
    
    //these are to randomly trigger the extension oscillators
    int extensionOneRandomInt = random.nextInt(1000);
    int extensionTwoRandomInt = random.nextInt(1000);

    
    //these are to figure out wether the delay has been silent for a whole block, in that case I will assume that it completed its cycle and will be allowd to change it's value without strange glitches
    float delayedExtensionOneBlock = 0.0f;
    float delayedExtensionTwoBlock = 0.0f;

    
    /* ===============
    ||DSP LOOP start||
     ===============*/
    
    for (int i = 0; i < numSamples; i++)
    {
        
        //process all LFOs
        float processedExtensionOneDelayPanLFO = extensionOneDelayPanLFO.process();
        float processedExtensionTwoDelayPanLFO = extensionTwoDelayPanLFO.process();
        float processedChordsFilterLFO = chordsFilterLFO.process();
        
        
        //process all LFOs Filters
        
        float chordsFilter_cutoff = 1000.0f + (processedChordsFilterLFO);
        
        
        float chordsFilter_resonance = 1.0f;
        
        chordsFilter.setCoefficients(juce::IIRCoefficients::makeLowPass(sr, chordsFilter_cutoff, chordsFilter_resonance)); //set the filter
        
        /* ============================
        ||GENERATIVE ARRANGEMENT start||
         =============================
         This section is to generate an ever changing arrangement for the "accomping" part of the program.
         A series of random number based switch will make sure that each part of the arrangement (Bass, Chords, Upper Extenions) will be different each time their own envelope restarts
         */
        
        //START OF Bass randomization
        //the bass will switch between root (making sure that this one happens fo the first time for each harmony change, and it will happen every other envelope cycle, in order not to loose the sense of the root of the harmony), fifth and octave.
        if(bassFirstCycle)
            bassOsc.setFrequency(midiToFrequency(root + octaves(4)));

        if(processedBassOscEnvelope<0.03f && bassWasAudible && bassIsRoot == true)
        {
            bassFirstCycle = false;
            bassWasAudible = false;
            if( random.nextInt(100)<90 )
            {
                if(random.nextInt(100)<50)
                {
                    bassOsc.setFrequency(fifthFreq);
                    bassIsRoot = false;
                } else
                {
                    bassOsc.setFrequency(rootFreq * 2);
                    bassIsRoot = false;
                }
            }
        }
        
        
        if(processedBassOscEnvelope<0.03f && bassWasAudible && bassIsRoot == false)
        {
            bassWasAudible=false;
            bassOsc.setFrequency(rootFreq);
            bassIsRoot = true;
        }
        
        
        if(processedBassOscEnvelope>0.03f)
            bassWasAudible=true;
        
        //END OF Bass randomization

        
        
        //START OF Chord randomization
        //chordWasAudible make sure that this loop is not entered for each DSP cycle where processedChordOscEnvelope, but only one time per envelope cycle.
        if(processedChordOscEnvelope<0.001f && chordWasAudible)
        {
            
            chordWasAudible=false;
            int randomChordArrangemet = random.nextInt(100);
            if( randomChordArrangemet>0 && randomChordArrangemet<25 )
            {
                //root position
                chordOsc1.setFrequency(juce::MidiMessage::getMidiNoteInHertz(root + octaves(5)));
                chordOsc2.setFrequency(juce::MidiMessage::getMidiNoteInHertz(fifth + octaves(5)));
                chordOsc3.setFrequency(juce::MidiMessage::getMidiNoteInHertz(guideTones[0] + octaves(5)));
                chordOsc4.setFrequency(juce::MidiMessage::getMidiNoteInHertz(guideTones[1] + octaves(5)));
            } else if (randomChordArrangemet>25 && randomChordArrangemet<50 )
            {
                //first inversion
                chordOsc1.setFrequency(juce::MidiMessage::getMidiNoteInHertz(root + octaves(6)));
                chordOsc2.setFrequency(juce::MidiMessage::getMidiNoteInHertz(fifth + octaves(5)));
                chordOsc3.setFrequency(juce::MidiMessage::getMidiNoteInHertz(guideTones[0] + octaves(5)));
                chordOsc4.setFrequency(juce::MidiMessage::getMidiNoteInHertz(guideTones[1] + octaves(5)));
            } else if (randomChordArrangemet>50 && randomChordArrangemet<75 )
            {
                //second inversion
                chordOsc1.setFrequency(juce::MidiMessage::getMidiNoteInHertz(root + octaves(6)));
                chordOsc2.setFrequency(juce::MidiMessage::getMidiNoteInHertz(fifth + octaves(6)));
                chordOsc3.setFrequency(juce::MidiMessage::getMidiNoteInHertz(guideTones[0] + octaves(5)));
                chordOsc4.setFrequency(juce::MidiMessage::getMidiNoteInHertz(guideTones[1] + octaves(5)));
            } else if (randomChordArrangemet>75 && randomChordArrangemet<100 )
            {
                //third inversion
                chordOsc1.setFrequency(juce::MidiMessage::getMidiNoteInHertz(root + octaves(6)));
                chordOsc2.setFrequency(juce::MidiMessage::getMidiNoteInHertz(fifth + octaves(6)));
                chordOsc3.setFrequency(juce::MidiMessage::getMidiNoteInHertz(guideTones[0] + octaves(6)));
                chordOsc4.setFrequency(juce::MidiMessage::getMidiNoteInHertz(guideTones[1] + octaves(5)));
            }
        }
        
        if(processedChordOscEnvelope>0.03f)
            chordWasAudible=true;
        
        //END OF Chord randomization

        
        
        //START OF ExtensionOne randomization
        if(processedExtensionOneOscEnvelope == 0.0f)
        {
            //std::cout<< "processedExtensionOneOscEnvelope --> "<<processedExtensionOneOscEnvelope<<"\n\n";
            
            
            if( extensionOneRandomInt == 1 )
            {

                if (delayedExtensionOneIsOver == 1)
                    extensionOneDelay.setDelayTimeInSamples(sr * processedExtensionOneDelayPanLFO);
                
                extensionOneOscEnvelope.trigger();
                extensionOneWasAudible=false;

                if(random.nextInt(100)<50)
                {
                    if(random.nextInt(100)<50)
                    {
                        extensionOsc1.setFrequency(extensionsFreq[0]);
                        
                    } else
                    {
                        extensionOsc1.setFrequency(extensionsFreq[1]);
                    }
                } else
                {
                    if(random.nextInt(100)<50)
                    {
                        extensionOsc1.setFrequency(rootFreq*4);
                        
                    } else
                    {
                        extensionOsc1.setFrequency(fifthFreq*4);
                    }
                }
            }
        }
        
        //END OF ExtensionOne randomization
        
        
        
        
        //START OF ExtensionTwo randomization
        
        //if(processedExtensionTwoOscEnvelope == 0.0f && extensionTwoWasAudible)
        if(processedExtensionTwoOscEnvelope == 0.0f)
        {

            //std::cout<<"processedExtensionTwoOscEnvelope --> " <<processedExtensionTwoOscEnvelope <<"\n\n";


            if( extensionTwoRandomInt == 1 )
            {
                if (delayedExtensionTwoIsOver == 1)
                    extensionTwoDelay.setDelayTimeInSamples(sr * processedExtensionTwoDelayPanLFO);
                
                extensionTwoOscEnvelope.trigger();
                extensionTwoWasAudible=false;
                if(random.nextInt(100)<50)
                {
                    if(random.nextInt(100)<50)
                    {
                        extensionOsc2.setFrequency(extensionsFreq[0]);
                        
                    } else
                    {
                        extensionOsc2.setFrequency(extensionsFreq[1]);
                    }
                } else
                {
                    if(random.nextInt(100)<50)
                    {
                        extensionOsc2.setFrequency(rootFreq*4);
                        
                    } else
                    {
                        extensionOsc2.setFrequency(fifthFreq*4);
                    }
                }
            }
        }
        
        
        //END OF Extension randomization

        
        /*--PROCESS smooth parameters--*/

        float bassGainSmoothedVal = smoothBassGain.getNextValue();
        float chordGainSmoothedVal = smoothChordGain.getNextValue();
        float extensionGainSmoothedVal = smoothExtensionGain.getNextValue();

        
        float reverbAmountSmoothedVal = smoothReverbAmount.getNextValue();
        float reverbSizeSmoothedVal = smoothReverbSize.getNextValue();
        
        float synthVoiceGainSmoothVal = smoothSynthVoiceGain.getNextValue();
        
        
        //set global reverb
        
        float revCurrentLevels = (reverbAmountSmoothedVal * reverbSizeSmoothedVal); // OPTIMIZATION, MIGHT NOT WORK YET: initialize value to be check in order to update the reverb parameters
        
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
        float leftOnlySample = 0.0f;//left channel sample. It will eventually hold together all the samples end be output to the left channel of the audioBuffer;
        float rightOnlySample = 0.0f;//right channel samplee. It will eventually hold together all the samples end be output to the right channel of the audioBuffer;

        rightSamples.push_back(0.0f);
        leftSamples.push_back(0.0f);
        
        float processedBassOsc = bassOsc.process();
        processedBassOscEnvelope = bassOscEnvelope.process();
        processedBassOscEnvelope *= bassGainSmoothedVal;
        
        

        float processedChordOsc1 = chordOsc1.process();
        float processedChordOsc2 = chordOsc2.process();
        float processedChordOsc3 = chordOsc3.process();
        float processedChordOsc4 = chordOsc4.process();

        processedChordOscEnvelope = chordOscEnvelope.process();
        processedChordOscEnvelope *= chordGainSmoothedVal;
        
        
        float processedExtensionOsc1 = extensionOsc1.process();
        float processedExtensionOsc2 = extensionOsc2.process();

        processedExtensionOneOscEnvelope = extensionOneOscEnvelope.process();
       // std::cout<<drt.nextStep("processedExtensionOneOscEnvelope outside arrangement  --> ", processedExtensionOneOscEnvelope);

        processedExtensionOneOscEnvelope *= extensionGainSmoothedVal;
        
        processedExtensionTwoOscEnvelope = extensionTwoOscEnvelope.process();
        //std::cout<<drt.nextStep("processedExtensionTwoOscEnvelope outside arrangement --> ", processedExtensionTwoOscEnvelope);

        processedExtensionTwoOscEnvelope *= extensionGainSmoothedVal;
        
        
        processedBassOsc *= processedBassOscEnvelope;
    
        processedExtensionOsc1 *= processedExtensionOneOscEnvelope;
        processedExtensionOsc2 *= processedExtensionTwoOscEnvelope;

        
        //process and apply delays

        float delayedExtensionOne = extensionOneDelay.process(processedExtensionOsc1);
        
        float delayedExtensionTwo = extensionTwoDelay.process(processedExtensionOsc2);

        
        samples.push_back(processedBassOsc);
        
        float processedChordOscTotal = (processedChordOsc1 + processedChordOsc2 + processedChordOsc3 + processedChordOsc4)/4;
        processedChordOscTotal =  chordsFilter.processSingleSampleRaw(processedChordOscTotal);
        processedChordOscTotal *= processedChordOscEnvelope;
        
        
        samples.push_back(processedChordOscTotal);

        

        leftSamples.push_back(processedExtensionOsc1 * 0.8f);
        rightSamples.push_back(processedExtensionOsc1 * 0.2f);


        leftSamples.push_back(processedExtensionOsc2 * 0.2f);
        rightSamples.push_back(processedExtensionOsc2 * 0.8f);
    

        
        //
        
        /*  Sometimes this delayLine doesn't work as expected and returns wildly loud values that elude juce gain level checks.
            I had therfore to exclude those from the final sample as it is happening quite frequently.
            I suspect this has to do with the way I raondmly avoid to process the extensions in the generativ arrangement section.
            Didn't have the time to figure out a better solution for this delay to still happen in the time I had to finalize this version so I will just exclude it for now.
        */
        
        
        delayedExtensionOneBlock += delayedExtensionOne;
        delayedExtensionTwoBlock += delayedExtensionTwo;

        
        if(delayedExtensionOne < 1.0f)
        {
            leftSamples.push_back(delayedExtensionOne * 0.8f * processedExtensionOneDelayPanLFO);
            rightSamples.push_back(delayedExtensionOne * 0.2f * (1.0f - processedExtensionOneDelayPanLFO));
        }else{
            int weGotAProblem;
        }
        
        
        if(delayedExtensionTwo < 1.0f)
        {
            leftSamples.push_back(delayedExtensionTwo * 0.2f * processedExtensionTwoDelayPanLFO);
            rightSamples.push_back(delayedExtensionTwo * 0.8f * (1.0f - processedExtensionTwoDelayPanLFO));
        }else {
            int weGotAProblem;
        }
        

        
        for (int i = 0; i < samples.size(); i++)
        {
            currentSample += samples[i] ;
        }
        
        for (int i = 0; i < leftSamples.size(); i++)
        {
            leftOnlySample += leftSamples[i] ;
        }
        
        for (int i = 0; i < rightSamples.size(); i++)
        {
            rightOnlySample += rightSamples[i] ;
        }
        

        currentSample /= samples.size();
        leftOnlySample /= leftSamples.size();
        rightOnlySample /= rightSamples.size();

        rightOnlySample = (rightOnlySample + currentSample)/2;
        leftOnlySample = (leftOnlySample + currentSample)/2;
        
        rightOnlySample *= genProtectionGain;
        leftOnlySample *= genProtectionGain;

        
        //SPEAKER PROTECTION
        //sometimes the delayLine doesn't work as expected and returns wildly loud values that elude juce gain level checks, this control should prevent it from happen
        //I have removed the delay on the extensions so this should avoid to happen. still good to have the asserts to protect speakers and ears though.
        
        jassert(rightOnlySample < 1.0f);

        if (rightOnlySample > 1.0f)
        {
            //this is useful in case i want to comment out the previous jassert to try to catch the reason why the gain is skyrocketing
            rightOnlySample = 1.0f;
        }
        
        jassert(leftOnlySample < 1.0f);

        if (leftOnlySample > 1.0f)
        {
            //this is useful in case i want to comment out the previous jassert to try to catch the reason why the gain is skyrocketing
            leftOnlySample = 1.0f;
        }
        
        samples.clear();
        leftSamples.clear();
        rightSamples.clear();
        
        //fade out the old harmony
        if (concludingOldHarmony)
        {
            processedConcludeOldHarmony = concludeOldHarmony.process();
            
            
            //some times ending of the envelope attack turns out being negative, this partially solves the problem
            if (processedConcludeOldHarmony < 0.001)
                processedConcludeOldHarmony = 0.001;

            rightOnlySample *= processedConcludeOldHarmony;
            leftOnlySample  *= processedConcludeOldHarmony;
            
            if (debug)std::cout <<drt.nextStep("processedConcludeOldHarmony is ", processedConcludeOldHarmony) ;

        }
        
        
        //concludingNewHarmony has finished its cycle, setting the variables to be ready for next harmony swap and resetting the concludeOldHarmony envelope and setting the beginNewHArmony to be ready to be processed
        if(concludeOldHarmony.getNodeName() == "ended")
        {
            concludingOldHarmony = false;
            concludeOldHarmony.reset(0.999999f);
            concludeOldHarmony.setNodeName("finished");
            beginNewHarmony.setNodeName("waiting");
        }
        

        //fade in the new harmony
        if( beginningNewHarmony)
        {
            processedBeginNewHarmony = beginNewHarmony.process();
            rightOnlySample *= processedBeginNewHarmony;
            leftOnlySample  *= processedBeginNewHarmony;
            if (debug) std::cout <<drt.nextStep("processedBeginNewHarmony is ", processedBeginNewHarmony) ;
        }


        if (processedBeginNewHarmony>0.999999f && beginNewHarmony.getNodeName() == "attack")
        {
            processedBeginNewHarmony = 0.00001f;
            beginningNewHarmony=false;
            beginNewHarmony.reset(0.00001f);
        }

        
        leftChannel[i]  += leftOnlySample;
        rightChannel[i] += rightOnlySample;

    }
    
    
    /* =============
    ||DSP LOOP end||
     =============*/
    
    if (delayedExtensionOneBlock== 0.0f)
        delayedExtensionOneIsOver = 1;
     
    if (delayedExtensionTwoBlock== 0.0f)
        delayedExtensionTwoIsOver = 1;
        
    
    //apply global reverb
    if(!*includeSynthInReverbParam)
        generalReverb.processStereo(leftChannel, rightChannel, numSamples);
    
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    
    if(*includeSynthInReverbParam)
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
