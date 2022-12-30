/*
  ==============================================================================
    MidiProcessor.h
  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include <vector>
#include "harmonyResolver.h"
#include "utilities.h"

class MidiProcessor
{
public:
    
    void process(juce::MidiBuffer& midiMessages, int* currentHarmony, int* chordDegree, bool* allowSynthNotes, bool* beginHarmonySwitch, bool* harmonySwitchFirstCycle)
    {
        juce::MidiBuffer::Iterator it(midiMessages);
        juce::MidiMessage currentMessage;
        int samplePos;
        std::vector<std::string> possibleMajorScales;
        
        while (it.getNextEvent(currentMessage, samplePos))
        {
            hr.setPrioritizeKeyChange(prioritizeKeyChange);
            
            //if the note associated with enterHarmonyMode is pressed, we enter the harmony mode
            if (currentMessage.getNoteNumber() == 36 && currentMessage.getFloatVelocity()!=0 )
            {
                enterHarmonyMode = 1;
                *allowSynthNotes = false;
                 if(debug) std::cout << "enterHarmonyMode = " << enterHarmonyMode<< "\n";
            }
            
            //if the note associated with enterHarmonyMode is released, we exit the harmony mode
            if (currentMessage.getNoteNumber() == 36 && currentMessage.getFloatVelocity()==0 )
            {
                enterHarmonyMode = 0;
                *allowSynthNotes = true;

                 if(debug) std::cout << "enterHarmonyMode = " << enterHarmonyMode<< "\n";
                
            }
            
            
//            juce::KeyPress::KeyPress('c',  juce::ModifierKeys::shiftModifier,0);
            
            //if enterHarmonyMode is true the midi processor will collect the pressed keys until it gets a note release message
            if (enterHarmonyMode){
                
                if(currentMessage.getFloatVelocity())
                {
                    //if we enter a note that is different from the note that enables the harmony mode, such note will be collected.
                    if(currentMessage.getNoteNumber()!=36)
                    {
                        bool isNewNote = 1;
                        int newNote = currentMessage.getNoteNumber() % 12;
                
                        //check wether the newly received note already exists within the newNotesCollection
                        if(newNotesCollection.size()>0)
                        {
                            for (int i=0; i< newNotesCollection.size(); i++)
                            {
                                if (newNotesCollection[i] == newNote)
                                    isNewNote = 0;
                            }
                        }

                        //if the new note already exists within the notesCollection, it will be ignored

                        if(isNewNote)
                            newNotesCollection.push_back(newNote);
                        
                         if(debug) std::cout << "Vector length = " << newNotesCollection.size() << "\n";
                        
                        if ( newNotesCollection.size() > maxHarmonyInput )
                            newNotesCollection.erase(newNotesCollection.begin() );
                    }
                    
                    
                     if(debug) std::cout << "Vector length = " << newNotesCollection.size() << "\n";
                }else
                {
                    //if there are notes in the notes collection vector, and one of these is released, the whole vector is cleared and the notes will be sent out to update the harmonic content of the music generator.
                    if(newNotesCollection.size() !=0)
                    {
                         if(debug) std::cout << "note content is ";
                        
                        for(int i = 0; i < newNotesCollection.size() ; i++)
                        {
                             if(debug) std::cout <<newNotesCollection[i] << " ";
                        }
                        
                         if(debug) std::cout << "\n";
                        storedNotesCollection = newNotesCollection;
                        
                        possibleMajorScales = hr.findNewPossibleMajorScales(storedNotesCollection);
                        
                        
                        

                        //the first element in the possibleMajorScales vector is a declaration of the result of the operation: if it's 0 it means that no possible major scales could be found via the input
                        
                        if(possibleMajorScales[1]!="0")
                        {
                            std::string nextKeyCentre = hr.findNearestKeyCentre(currentKeyCentre, possibleMajorScales);

                            if(chat) std::cout << "MidiProcessor - Old key Centre was " << currentKeyCentre << "\n";
                            if(chat)  std::cout << "Next key Centre is " << nextKeyCentre << "\n";
                            currentKeyCentre = nextKeyCentre;
                            
                            if(chat) std::cout << "New Key Centre is " << currentKeyCentre << "\n\n";
                            hr.setCurrentMajorScale(currentKeyCentre);
                            
                            
                            
                            
                            std::vector<std::string> possibleNewChordDegrees = hr.findNewPossibleChordDegrees(newNotesCollection, currentKeyCentre);
                            
                            
                            if(chat)  std::cout << "MidiProcessor - possible new chords degrees are:\n";
                            for (int i = 0; i< possibleNewChordDegrees.size(); i++)
                            {
                                if(chat) std::cout << possibleNewChordDegrees[i] <<"\n";
                            }
                            if(chat) std::cout << "\n";

                            std::string currentScaleDegree; //this will hold the new scale degree
                            
                            bool noPossibleChord=0; //this will become true if no chord was found in the harmony and will skip the rest of the code without returning anything
                            
                            //if you have multiple chord options, randomly choose one chord out of the ones you have
                            if(possibleNewChordDegrees.size()>1)
                                currentScaleDegree = possibleNewChordDegrees[random.nextInt(possibleNewChordDegrees.size())];
                            else if(possibleNewChordDegrees.size()==1)
                                 currentScaleDegree = possibleNewChordDegrees[0];
                            else if(possibleNewChordDegrees.size()==0)
                                noPossibleChord=1;
                            
                            if(!noPossibleChord){
                                
                                *beginHarmonySwitch = true; //------------------------>begin the process of fading out old harmony and fading in new harmony
                                *harmonySwitchFirstCycle = true; //------------------->set the first cycle of the harmony swap process ad true so that the necessary variables can be initialised
                                *chordDegree = degreeToNumber[currentScaleDegree]; //-->pass out the new scale degrees
                                
                                if(chat) std::cout << " MidiProcessor - Among these we chose for you " << currentScaleDegree <<"\n";
                                
                                if(chat) std::cout << "\n===================================================\n\n";
                                
                                int currentKeyCentreNumber = notesToNumber[currentKeyCentre];
                                int currentScaleDegreeNumber = degreeToNumber[currentScaleDegree];
                                
                                //generate the current major scale formula
                                if (chat) std::cout << "Midi Processor - your new major scale formula is: \n";
                                
                                for(int i = 0; i < majorScaleFormulaSize; i++)
                                {
                                    currentMajorScaleFormula[i]= (majorScaleFormula[i] + currentKeyCentreNumber) % 12;
                                    if(chat) std::cout << numberToNotes[currentMajorScaleFormula [i]] << "\n";
                                }
                                
                                
                                //generate the current harmony formula
                                for(int i = 0; i < genericHarmonyFormulaSize; i++)
                                {
                                    currentHarmonyFormula[i] = ( genericHarmonyFormula[i] + currentScaleDegreeNumber ) % 7;
                                }
                                
                                if (console) std::cout << "\n\n===Next harmony will be " << numberToNotes[currentMajorScaleFormula[degreeToNumber[currentScaleDegree]]] <<" "<< degreeToQuality[currentScaleDegree] <<"===\n\n";
                                

                                
                                if(debug)std::cout << "here is what you have got: \n";
                                
                                //generate the current harmony actual notes via the current harmony formula
                                for(int i = 0; i < genericHarmonyFormulaSize; i++)
                                {
                                    currentHarmony[i] = ( currentMajorScaleFormula[currentHarmonyFormula[i]]);
                                    if(chat) std::cout << numberToNotes[currentHarmony[i]] <<   "\n";
                                }
                            }else
                            {
                                if(console)std::cout << "no possible chord was found for that key centre with the notes you have input";
                                //return 0;
                            }
                        }else
                        {
                            if(console)std::cout << "system was unable to find a new key centre, sticking with the old one\n\n";
                            currentKeyCentre = possibleMajorScales[0];
                            //return 0;
                        }
                        
                        //clear the notes collection vector
                        newNotesCollection.clear();
                        
                         if(debug) std::cout << "cleared vector; Vector length = " << newNotesCollection.size() << "\n";
                    }
                }
            }
        }
    }
    
    
    void setPrioritizeKeyChange (bool b)
    {
        prioritizeKeyChange = b;
    }
    
    
private:
    
    bool console = 1;//messages that can be useful during performance, while a decent interface is not available
    bool debug = 0; //lower level debug
    bool chat = 0;  //higher level debug
    
    
    bool enterHarmonyMode = 0;
    int maxHarmonyInput = 4;
    
    bool prioritizeKeyChange = true;
    
    std::vector<int> newNotesCollection;
    std::string currentKeyCentre = "C";
    std::vector<int> storedNotesCollection;
    HarmonyResolver hr;
    

    int currentMajorScaleFormula[7];
  
    int currentHarmonyFormula[7];
    
    int currentHarmony[7];

    

    juce::Random random;
    
};
