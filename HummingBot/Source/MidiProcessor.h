/*
  ==============================================================================
    MidiProcessor.h
  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include <vector>
#include "harmonyResolver.h"

class MidiProcessor
{
public:
    
    void process(juce::MidiBuffer& midiMessages, int* currentHarmony, int* chordDegree, bool* allowSynthNotes, bool* beginHarmonySwitch)
    {
        juce::MidiBuffer::Iterator it(midiMessages);
        juce::MidiMessage currentMessage;
        int samplePos;
        std::vector<std::string> possibleMajorScales;
        
        while (it.getNextEvent(currentMessage, samplePos))
        {
//             if(debug) std::cout << currentMessage.getDescription() << "\n";
//             if(debug) std::cout << currentMessage.getNoteNumber() << "\n";
//             if(debug) std::cout << currentMessage.getFloatVelocity() << "\n";
            
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
                        
                        
                        


                        if(possibleMajorScales[1]!="0")
                        {
                            std::string nextKeyCentre = hr.findNearestKeyCentre(currentKeyCentre, possibleMajorScales);

                            std::cout << "Old key Centre was " << currentKeyCentre << "\n";
                            std::cout << "Next key Centre is " << nextKeyCentre << "\n";
                            currentKeyCentre = nextKeyCentre;
                            
                            std::cout << "New Key Centre is " << currentKeyCentre << "\n\n";
                            hr.setCurrentMajorScale(currentKeyCentre);
                            
                            
                            
                            
                            std::vector<std::string> possibleNewChordDegrees = hr.findNewPossibleChordDegrees(newNotesCollection, currentKeyCentre);
                            
                            
                            std::cout << "possible new chords degrees are:\n";
                            for (int i = 0; i< possibleNewChordDegrees.size(); i++)
                            {
                                std::cout << possibleNewChordDegrees[i] <<"\n";
                            }
                            std::cout << "\n";

                            std::string currentScaleDegree; //this will hold the new scale degree
                            
                            bool noPossibleChord=0; //this will become true if no chord was found in the harmony and will skip the rest of the code without returning anything
                            
                            //if you have multiple chord options, randomly choose one chord out of the ones you have
                            if(possibleNewChordDegrees.size()>1)
                            {
                                currentScaleDegree = possibleNewChordDegrees[random.nextInt(possibleNewChordDegrees.size())];}
                            else if(possibleNewChordDegrees.size()==1)
                                 currentScaleDegree = possibleNewChordDegrees[0];
                            else if(possibleNewChordDegrees.size()==0)
                                noPossibleChord=1;
                            
                            if(!noPossibleChord){
                                
                                *beginHarmonySwitch = true; //begin the process of fading out old harmony and fading in new harmony
                                *chordDegree = degreeToNumber[currentScaleDegree]; //pass out the new scale degree
                                
                                std::cout << "among these we chose for you " << currentScaleDegree <<"\n";
                                
                                std::cout << "\n===================================================\n\n";
                                
                                int currentKeyCentreNumber = notesToNumber[currentKeyCentre];
                                int currentScaleDegreeNumber = degreeToNumber[currentScaleDegree];
                                
                                //generate the current major scale formula
                                std::cout << "your new major scale formula is: \n";
                                
                                for(int i = 0; i < majorScaleFormulaSize; i++)
                                {
                                    
                                    currentMajorScaleFormula[i]= (majorScaleFormula[i] + currentKeyCentreNumber) % 12;
                                    
                                    std::cout << numberToNotes[currentMajorScaleFormula [i]] << "\n";
                                    
                                }
                                
                                
                                //generate the current harmony formula
                                for(int i = 0; i < genericHarmonyFormulaSize; i++)
                                {
                                    currentHarmonyFormula[i] = ( genericHarmonyFormula[i] + currentScaleDegreeNumber ) % 7;
                                }
                                
                                std::cout << "you want a " << numberToNotes[currentMajorScaleFormula[degreeToNumber[currentScaleDegree]]] <<" "<< degreeToQuality[currentScaleDegree] <<"\n";
                                
                                std::cout << "here is what you have got: \n";
                                //generate the current harmony
                                for(int i = 0; i < genericHarmonyFormulaSize; i++)
                                {
                                    
                                    
                                    currentHarmony[i] = ( currentMajorScaleFormula[currentHarmonyFormula[i]]);
                                    
                                    std::cout << numberToNotes[currentHarmony[i]] <<   "\n";
                                    
                                }
                                
                                //return currentHarmony;
                            }else
                            {
                                std::cout << "no possible chord was found for that key centre with the notes you have input";
                                //return 0;
                            }
                        }else
                        {
                            std::cout << "system was unable to find a new key centre, sticking with the old one\n\n";
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
    
    
    
    std::vector<int> getsStoredNotesCollection()
    {
        return storedNotesCollection;
    }
    
    void setPrioritizeKeyChange (bool b)
    {
        prioritizeKeyChange = b;
    }
    
    
private:
    bool debug = 0;
    bool enterHarmonyMode = 0;
    int maxHarmonyInput = 4;
    
    bool prioritizeKeyChange = true;
    
    std::vector<int> newNotesCollection;
    std::string currentKeyCentre = "C";
    std::vector<int> storedNotesCollection;
    HarmonyResolver hr;
    
    int majorScaleFormula[7] = {0,2,4,5,7,9,11}; //    basic formula of any major scale
    int majorScaleFormulaSize = *(&majorScaleFormula + 1) - majorScaleFormula; //size of the array majorScaleFormula
    int currentMajorScaleFormula[7];
    
    int genericHarmonyFormula[7] = {0,2,4,6,1,3,5}; //basic formula to generate any added seventh chord plus extension starting from a major scale formula
    int genericHarmonyFormulaSize = *(&genericHarmonyFormula + 1) - genericHarmonyFormula; //size of the array majorScaleFormula
    int currentHarmonyFormula[7];
    
    int currentHarmony[7];

    
    std::map<std::string, int> notesToNumber =
    {
        {"C",0}, {"Db",1}, {"D",2}, {"Eb",3}, {"E",4}, {"F",5}, {"Gb",6}, {"G",7}, {"Ab",8}, {"A",9}, {"Bb",10}, {"B",11}
    };
    
    std::map<int, std::string> numberToNotes =
    {
        {0,"C"}, {1,"Db"}, {2,"D"}, {3,"Eb"}, {4,"E"}, {5,"F"}, {6,"Gb"}, {7,"G"}, {8,"Ab"}, {9,"A"}, {10,"Bb"}, {11,"B"}
    };
    
    std::map<std::string, int> degreeToNumber =
    {
        {"I",0},{"II",1},{"III",2},{"IV",3},{"V",4},{"VI",5},{"VII",6}
    };
    
    std::map<std::string, std::string> degreeToQuality =
    {
        {"I","Maj7-Ion"},{"II","Min7-Dor"},{"III","Min7-Phr"},{"IV","Maj7-Lyd"},{"V","Dom7-Mix"},{"VI","Min7-Aeo"},{"VII","Min7b5-Loc"}
    };
    juce::Random random;
    
};
