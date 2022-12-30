/*
  ==============================================================================

    Envelope.h
    Created: 5 Nov 2022 9:41:23pm
    Author:  Nicola Rivosecchi

  ==============================================================================
*/

#pragma once
#include "DebugResolutionTool.h"

/**
 This class instantiates a ADHSR filter, with customizable parameters
 The envelope can be turned into a ramping filter (ADS) through toggleAsRiser()
 */
class Envelope {
    
public:
    
    /**
     sets a sample rate and calculates the duration of a sample
     */
    void setSampleRate (float sr)
    {
        seconds = 1.0f / float(sr);  //calculates how many seconds each sample lasts
        
        if(debug)std::cout << debugLine1.nextStep("Class Envelope Debug == seconds ", seconds);

        sampleRate = sr;
        nodeName = "waiting to start";

    }
    
    ///sets the name tag of the envelope for debugging and tracing
    void setNameTag(std::string nt)
    {
        nameTag = nt;
    }
    
    ///gets the name tag of the envelope for debugging and tracing
    std::string getNameTag()
    {
        return nameTag;
    }
    
    ///sets a name for the current node
    void setNodeName (std::string newNode)
    {
        nodeName = newNode;
    };
    
    ///returns the name of the current node
    std::string getNodeName()
    {
        return nodeName;
    };
    
    ///returns a trace of the passing time inside the encelope
    float getTimeFlow()
    {
        return timeFlow;
    }


    
    
    /**
     read the timer and decided wich node of the envelope we are in; process the envelope accordingly
     @return gain value
     */
    float process ()
    {
        if (gain > 1.0f){
            std::cout << debugLine1.nextStep("gain is ", gain);
            gain = 1.0;
        }
        
        //make sure that the gain is always smaller or equal to one
        assert(gain <= 1.0f);
   
        timeFlow += seconds;
        
        float envNode = 0.0f; //keeps track of the node of the envelope we are on adding the duration of each segment together

            
        envNode = attack; //sets the time to be spent on this initial node
        

        // while the time count is under the amount of time to be spent on this node, process this node
        if (timeFlow < envNode)
        {
            setNodeName("attack");
            if(debug) debugLine1.nextStep("Class Envelope Debug: stage--> " + nodeName + " timeFlow-->", timeFlow);
            return gainRamp(0.0f, 1.0f, attack); //get from 0 to 1 in the set ATTACK time
        }
        
        //updates the time treshold to enter this node of the function; if the threshold has been passed, this node will be skipped
        envNode +=decay;
        
        // while the time count is under the amount of time to be spent on this node + the prevoius ones, process this node
        if (timeFlow < envNode)
        {
            setNodeName("decay");
            if(debug) std::cout << debugLine1.nextStep("Class Envelope Debug: stage--> " + nodeName + " timeFlow-->", timeFlow);
            return gainRamp(1.0f, sustain, decay); //get from 1 to the SUSTAIN level in the set DECAY time
        }

        //updates the time treshold to enter this node of the function; if the threshold has been passed, this node will be skipped
        envNode +=hold;
        
        // while the time count is under the amount of time to be spent on this node + the prevoius ones, process this node
        if (timeFlow < envNode)
        {
            setNodeName("hold");
            if(debug) std::cout << debugLine1.nextStep("Class Envelope Debug: stage--> " + nodeName + " timeFlow-->", timeFlow);
            return gainRamp(sustain, sustain, hold); //keeps the sustain for the set HOLD time
        }
        
        //if the envelope is set as a riser, it will never get past this stage and it will always return the sustain gain value
        if(asRiser) return gain;
        
        
        //updates the time treshold to enter this node of the function; if the threshold has been passed, this node will be skipped
        envNode +=release;
        
        // while the time count is under the amount of time to be spent on this node + the prevoius ones, process this node
        if (timeFlow < envNode)
        {
            setNodeName("release");
            if(debug) std::cout << debugLine1.nextStep("Class Envelope Debug: stage--> " + nodeName + " timeFlow-->", timeFlow);
            return gainRamp(sustain, 0.0f, release); //keeps the sustain for the set HOLD time
        }


        
        // durng DIE node, the envelope in inactive but can't get retriggered
        setNodeName("die");
        envNode += die;
        if (timeFlow < envNode)
        {
            if(debug) std::cout << debugLine1.nextStep("Class Envelope Debug: stage--> " + nodeName + " timeFlow-->", timeFlow);
            return 0.0001f;
        }
        
        // once the program has reached this stage, the envelope is over and it can be retriggered
        gain = 0.0f;
        if (retrigger)
            trigger();
        
        setNodeName("ended");
        if(debug) std::cout << debugLine1.nextStep("Class Envelope Debug: stage--> " + nodeName + " timeFlow-->", timeFlow);
        return gain;
    }
    
    ///restart the envelope cycle, but only if the envelope has completed its cycle
    void trigger(){
        if (gain == 0.0f)
        {
            timeFlow = 0.0f;//-----Resets the count of seconds, retriggering the envelope
            gain = 0.0f;//---------Makes sure that we are starting from a gain of 0
            nodeName = "waiting to start";

        }
    }
    
    /**restart the envelope cycle, at any moment
     @param gain the gain can be reset to a specific value if needed, it might be useful if you need an enveloope that always starts from the sustain node.
     */
    void reset(float _gain = 0.000001f)
    {
        timeFlow = 0.0f;//-----Resets the count of seconds, retriggering the envelope
        gain = _gain;//---------Makes sure that we are starting from a gain of 0 (or a custom valule)
        nodeName = "waiting to start";
    }
    /**
     sets all the needed parameters for the envelope
     @param _attack float attack time in seconds
     @param _decay Decay time in seconds
     @param _sustain Gain applied to the sustain
     @param _hold Time the sustain is hold in seconds
     @param _release Release time in seconds
     */
    void setParameters (float _attack,//---------------------------Attack time in seconds
                        float _decay,//----------------------------Decay time in seconds
                        float _sustain,//--------------------------Gain applied to the sustain
                        float _hold,//-----------------------------Time the sustain gain is held in seconds
                        float _release)//--------------------------release time in seconds
    {
        attack = _attack;
        decay = _decay;
        sustain= _sustain;
        hold = _hold;
        release = _release;
    }
    
    ///set die time in seconds
    void setDie(float d)
    {
        die = d;
    }
    
    ///generate a random set of envelope parameters
    void randomizeParameters()
    {
        attack = random.nextFloat() * 5;
        decay = random.nextFloat() * 5;
        sustain=random.nextFloat();
        hold = random.nextFloat() * 10;
        release = random.nextFloat() * 5;
    }
    /**
     allows the envelope to automatically retrigger once its cycle is over; By default this is set as False
     @param t enable/disable retrigger function
     @param dieTime sets how long it will wait before retriggering
     */
    void toggleRetrigger (bool t, float dieTime = 0.0f)
    {
        retrigger = t;
        die = dieTime;
    }
    
    ///if set true turns the envelope into a riser that will only reach the sustain stage and then always return the sustain gain value
    void toggleAsRiser (bool t)
    {
        asRiser = t;
    }
    
private:
    
    /**
     calculates the size of the increments to get from startGain to endGain in the time defined in nodeTime
     @param startGain gain value to start from
     @param endGain gain value to end at
     @param nodeTime time to spend to do this ptocess
     */
    float gainRamp (float startGain = 0.0f, float endGain = 1.0f, float nodeTime = 1.0f)
    {
        //make sure that both endGain and startGain are values between 0 and 1
        assert (1.0f >= endGain  && endGain >= 0.0f && 1.0f >= startGain && startGain >= 0.0f);
        
        //If startGain == endGain we are probably in a sustain node, so just return whatever the gain was previously without engaging in other calculations
        if (startGain == endGain)
            return gain;
        
        float gainDifference = (endGain - startGain); //-----------------Calculates the size of the gain difference
        float rampStep = gainDifference / (nodeTime * sampleRate); //----Size of each gain increase/decrease for process cycle
        gain += rampStep;//----------------------------------------------Updates the gain ramp
        return gain;
    }
    
    
    
    bool debug = 0;
    DebugResolutionTool debugLine1;


    juce::Random random;
    
    //Envelope engine
    float gain = 0.0f;//-----------------------------Gain that each step of the envelope will carry
    float sampleRate;//------------------------------Sample rate
    double seconds; // ------------------------------Lapse of time each sample is worth in seconds----(s)
    float timeFlow = 0.0f;//-------------------------Seconds passed since triggered
    std::string nodeName = "uninitialized";//--------Name to be assigned to each stage of the envelope, for tracing ad debug reason
    std::string nameTag = "default Envelope";//------Name to be assigned to each stage of the envelope, for tracing ad debug reason
    bool retrigger = 0;//----------------------------If true, once the envelope is over it will automatically retrigger
    bool asRiser = 0;//------------------------------If true, the envelope will only reach the sustain stage and then always return the sustain gain value
    
    //Envelope Parameters
    float attack = 1.0f;//---------------------------Attack time in seconds
    float decay = 1.0f;//----------------------------Decay time in seconds
    float sustain = 0.5f;//--------------------------Gain applied to the sustain
    float hold = 1.0f;//-----------------------------Time the sustain is hold in seconds
    float release = 2.5f;//--------------------------Attack time in seconds
    float die = 0.0f;//-------------------------------if retrigger is enabled, it will wait this time before retriggering
};


