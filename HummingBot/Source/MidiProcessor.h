/*
  ==============================================================================
    MidiProcessor.h
  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include <vector>

class MidiProcessor
{
public:
    
    void process(juce::MidiBuffer& midiMessages)
    {
        juce::MidiBuffer::Iterator it(midiMessages);
        juce::MidiMessage currentMessage;
        int samplePos;
        
        while (it.getNextEvent(currentMessage, samplePos))
        {
//            std::cout << currentMessage.getDescription() << "\n";
//            std::cout << currentMessage.getNoteNumber() << "\n";
//            std::cout << currentMessage.getFloatVelocity() << "\n";
            
            
            //if the note associated with enterHarmonyMode is pressed, we enter the harmony mode
            if (currentMessage.getNoteNumber() == 36 && currentMessage.getFloatVelocity()!=0 )
            {
                enterHarmonyMode = 1;
                std::cout << "enterHarmonyMode = " << enterHarmonyMode<< "\n";
            }
            
            //if the note associated with enterHarmonyMode is releas3ed, we exit the harmony mode
            if (currentMessage.getNoteNumber() == 36 && currentMessage.getFloatVelocity()==0 )
            {
                enterHarmonyMode = 0;
                std::cout << "enterHarmonyMode = " << enterHarmonyMode<< "\n";
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
                        
                        //check wether the newly received note already exists within the notesCollection
                        if(notesCollection.size()>0){
                            for (int i=0; i< notesCollection.size(); i++){
                                if (notesCollection[i] == newNote)
                                    isNewNote = 0;
                            }
                        }

                        //if the new note already exists within the notesCollection, it will be ignored

                        if(isNewNote)
                            notesCollection.push_back(newNote);
                        std::cout << "Vector length = " << notesCollection.size() << "\n";


                        if ( notesCollection.size() > maxHarmonyInput )
                            notesCollection.erase(notesCollection.begin() );


                    }
                    
                    
                    std::cout << "Vector length = " << notesCollection.size() << "\n";
                }else
                {
                    //if there are notes in the notes collection vector, and one of these is released, the whole vector is cleared and the notes will be sent out to update the harmonic content of the music generator.
                    if(notesCollection.size() !=0)
                    {
                        std::cout << "note content is ";
                        
                        for(int i = 0; i < notesCollection.size() ; i++)
                        {
                            std::cout <<notesCollection[i] << " ";
                        }
                        
                        std::cout << "\n";
                        
                        //clear the notes collection vector
                        notesCollection.clear();
                        
                        std::cout << "cleared vector; Vector length = " << notesCollection.size() << "\n";
                    }
                   // std::cout << "\n\n";
                }
            }
        }
    }
    
private:
    bool enterHarmonyMode = 1;
    int maxHarmonyInput = 4;
    std::vector<int> notesCollection;

};
