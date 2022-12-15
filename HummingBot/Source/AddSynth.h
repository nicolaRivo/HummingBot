/*
  ==============================================================================

    AddSynth.h
    Created: 7 Nov 2022 9:53:47am
    Author:  Nicola Rivosecchi

  ==============================================================================
*/

#pragma once
#include "Oscillators.h"
#include "debugResolutionTool.h"


/**
 Additive synth. it can generate harmonic or inharmonic partials.
 if run in complex mode, all the partial frequencies will be modulated around their central frequency resulting in something that feel a bit more like a collection of sinewaves rather than a unitary additive synthesis tone
 */
class AddSynth
{
public:
    
    ///sets sample rate and gives a name to the LFO for tracing reasons
    void setSampleRate(float sr)
    {
        sampleRate = sr;
        partialBlockModulationLFO.setCaller("AddSynth - " + nameTag + " partialModulationLFO");
    }
    
    ///sets frequency of fundamental harmonic
    void setFundFreq(float f)
    {
        fundamentalFreq = f;
    }
    
    ///sets number of partial harmonics
    void setPartials(int p)
    {
        partials = p;
    }
    
    /// sets the frequency for the the partial modulation LFO (has to be lower than 20Hz)
    void setPartialModulationLFOFrequency(float f)
    {
        partialBlockModulationLFO.setFrequency(f);
    }
    
    ///initialises all the oscilators needed to generate the additive synthesis tone (fundamental + harmonic partials)
    void initialiseHarmonics()
    {
        //initialise the fundamental frequency oscillator and the Partial modulation LFO
        fundamentalOsc.setFrequency(fundamentalFreq);
        fundamentalOsc.setSampleRate(sampleRate);
        partialBlockModulationLFO.setSampleRate(sampleRate);
        addOsc.clear();//make sure that the additive synthesis vector (fundamental + partials) is empty

        //initialise all the required partials and stores them in the addOsc vector
        addOsc.push_back(fundamentalOsc);
        for (int i = 0; i < partials; i++)
        {
            SineOsc partialOsc;//------------------------------------Generates a new oscillator for the new partial
            partialOsc.setSampleRate(sampleRate);//------------------Initializes the sample rate of the new partial
            partialOsc.setFrequency(fundamentalFreq * (i+2));//------Sets the oscillator frequency to the next harmonic partial
            addOsc.push_back(partialOsc);//--------------------------Pushes the new partial in the additive synthesis vector (fundamental + partials)
        }
    }
    
    
    ///initialises all the oscilators needed to generate the additive synthesis tone (fundamental + inharmonic partials). The frequencies of the inharmonics are random but progressive
    void initialiseInharmonics()
    {
        //initialise the fundamental frequency oscillator and the Partial modulation LFO
        fundamentalOsc.setFrequency(fundamentalFreq);
        fundamentalOsc.setSampleRate(sampleRate);
        partialBlockModulationLFO.setSampleRate(sampleRate);
        addOsc.clear();//make sure that the additive synthesis vector (fundamental + partials) is empty
        
        std::vector <float> frequencies;//------------Generates a vector to store all the frequencies - useful as a reference to generate the inharmonic frequencies
        frequencies.push_back(fundamentalFreq);//-----Add the fundamental frequency to the vector of frequencies
        addOsc.push_back(fundamentalOsc);//-----------Add the fundamental oscillator to the vector of oscillator

        
        //initialise all the required partials and stores them in the addOsc vector
        for (int i = 0; i < partials; i++)
        {
            float newPartialIharmonicFreq = random.nextFloat() * frequencies[i]  + frequencies[i] ; //----Generates a random number between the previou partial and its double
            SineOsc partialOsc;//-------------------------------------------------------------------------Generates a new oscillator for the new partial
            partialOsc.setSampleRate(sampleRate);//-------------------------------------------------------Initializes the sample rate of the new partial
            partialOsc.setFrequency(newPartialIharmonicFreq);//-------------------------------------------Sets the oscillator frequency to the newly generated partial
            addOsc.push_back(partialOsc);//---------------------------------------------------------------Pushes the new partial in the additive synthesis vector (fundamental + partials)
        }
    }
    
    /**sets a name tag to the oscillator, useful to trace it back in debug situations
     @param nt strung, desired name tag
     */
    void setNameTag(std::string nt)
    {
        nameTag = nt;
    }
    
    
    /**
     process the  additive synth by adding together the  partials and the fundamental. Each partial is half the volume of the previous one.
     if enabled, an LFO will modulate the gain of all the partials as a block.
     @return next step of the processed add sinth
     */
    float processSimple()
    {
        float processPhase = 0;
        float processedPartialModulationLFO = partialBlockModulationLFO.process();
        //std::cout << checkLFOstatus.nextStep("LFO value at this stage is", processedPartialModulationLFO);
        
        for (int i = 0; i < addOsc.size(); i++ )
        {
            //processes the partial and adapts the gain;
            float processsedPartial = addOsc[i].process() / (i+1);
            
            //if the block modulation is requred, proces the gain of the partials accordingly
            if (allowPartialBlockModulationLFO  && i != 0 )
                processsedPartial *= processedPartialModulationLFO;
            
            processPhase += processsedPartial;
        }
        
        //normalize gain
        return processPhase / addOsc.size();
    }
    
    
    
    
    ///enables the LFO to modulate the gain of all the partials by the same amount
    void togglePartialBlockModulationLFO(bool t)
    {
        allowPartialBlockModulationLFO = t;
    }
    
    
    /**This function will process the fundamental and will apply individual oscillators to each partial, modulating their frequency
     @param randomFrequencyMax is the maximum value of the LFO random frequence that is individually assigned to each partial
     @param partialGainAsBlock bool, 1 will apply a single gain LFO to all the partial frequencies, 0 will apply one gain LFO to each partial frequency
     */
    float processComplex(float randomFrequencyMax = 0.1f, bool partialGainAsBlock = 0)
    {
        float processPhase = 0;
        float processedPartialModulationLFO = partialBlockModulationLFO.process(); //this will process LFO that will modulate each partial gain as a block

        
        //if the lfo vector hasnt been populated yet, it will be done now
        if (partialLFOs.size() == 0)
        {
            for (int i = 0; i < addOsc.size() + 1; i++)
            {
                LFO partialLFO;  //---------------------------------------------------Define a new LFO for the current partial
                partialLFO.setCaller("Lfo for partial n" + std::to_string(i));//------Assign a caller ID to the new LFO for debug purposes
                partialLFO.setSampleRate(sampleRate);//-------------------------------Set Sample Rate to the new LFO
                partialLFO.setFrequency(random.nextFloat() * randomFrequencyMax);//---Assign a random Frequency
                partialLFOs.push_back(partialLFO);//----------------------------------Add the new LFO to the vector
            }
        }
        
        std::vector <float> processedPartialLFOs;// New vector to store the values of each partialLFO once processed
        
        //process all the LFOs and store the result in the partialLFOs vector; I am creating one LFO more than the partials so that i can get more variety by using the ollowing LFO value (processedPartialLFOs[i+1]) to modulate the volume of the single partials
        for (int i = 0; i < partialLFOs.size(); i++)
        {
            float processedLFO = partialLFOs[i].process();
            processedPartialLFOs.push_back(processedLFO);
        }
        
        //processes fundamental;
        float processsedFundamental = addOsc[0].process();
        processPhase +=processsedFundamental;
        
        //processes the partials
        for (int i = 1; i < addOsc.size(); i++ )
        {
            //applies an lfo to modulate the frequency for each partial and the modulator
            addOsc[i].setFrequency(fundamentalFreq * (i+1) * processedPartialLFOs[i]);
            
            //float processsedPartial = addOsc[i].process();
            float processsedPartial = addOsc[i].process() / (i+1);

            if(partialGainAsBlock)
                processsedPartial *= processedPartialModulationLFO;//-----process all the partials volume as a block
            else
                processsedPartial *= processedPartialLFOs[i+1];//---------process the partials volumes individually

    
            processPhase += processsedPartial;
        }
        
        //normalize gain
        return processPhase / addOsc.size();
    }
    
    
private:
    
    //initialize
    DebugResolutionTool checkLFOstatus;//--Debug tool to check the status of the LFOs
    SineOsc fundamentalOsc;//--------------Fundamental oscillator that will generate al the partials
    std::vector<SineOsc> addOsc;//---------This vector of sinewaves will contain all the partials
    std::vector <LFO> partialLFOs;//-------This vector of LFOs can be used to modulate individual partial independedntly from the others
    LFO partialBlockModulationLFO;//-------This LFO can be used to modulate all the partials in the same way, as a block
    juce::Random random;//-----------------Useful random funcrion
    float sampleRate;

    
    
    bool allowPartialBlockModulationLFO = 1;
    std::string nameTag = "Default addOsc";
    int partials = 8;
    float fundamentalFreq = 440;

    
 
};
