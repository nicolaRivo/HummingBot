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
    
    void process(juce::MidiBuffer& midiMessages)
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
            
            
            //if the note associated with enterHarmonyMode is pressed, we enter the harmony mode
            if (currentMessage.getNoteNumber() == 36 && currentMessage.getFloatVelocity()!=0 )
            {
                enterHarmonyMode = 1;
                 if(debug) std::cout << "enterHarmonyMode = " << enterHarmonyMode<< "\n";
            }
            
            //if the note associated with enterHarmonyMode is releas3ed, we exit the harmony mode
            if (currentMessage.getNoteNumber() == 36 && currentMessage.getFloatVelocity()==0 )
            {
                enterHarmonyMode = 0;
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
                            
                            std::cout << "New Key Centre is " << currentKeyCentre << "\n";
                            hr.setCurrentMajorScale(currentKeyCentre);
                            
                            std::vector<std::string> possibleNewChordDegrees = hr.findNewPossibleChordDegrees(newNotesCollection, currentKeyCentre);
                            
                            
                            std::cout << "possible new chords degrees are:\n";
                            for (int i = 0; i< possibleNewChordDegrees.size(); i++)
                            {
                                std::cout << possibleNewChordDegrees[i] <<"\n";
                            }
                            std::cout << "\n===================================================\n\n";
                            
                        }else
                        {
                            std::cout << "system was unable to find a new key centre, sticking with the old one\n\n";
                            currentKeyCentre = possibleMajorScales[0];
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
    
    
private:
    bool debug = 0;
    bool enterHarmonyMode = 1;
    int maxHarmonyInput = 4;
    std::vector<int> newNotesCollection;
    std::string currentKeyCentre = "C";
    std::vector<int> storedNotesCollection;
    HarmonyResolver hr;
    
    

};
