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
                        
                        possibleMajorScales = hr.findNewMajScale(storedNotesCollection);
                        
                        
                        std::string nextHarmony = findNearestHarmony(currentHarmony, possibleMajorScales);
                        
                        if(debug) std::cout << "Old harmony was " << currentHarmony << "\n";
                        if(debug) std::cout << "Next harmony is " << nextHarmony << "\n";
                        currentHarmony = nextHarmony;
                        if(debug) std::cout << "New harmony is " << currentHarmony << "\n";


                        
                        //clear the notes collection vector
                        newNotesCollection.clear();
                        
                         if(debug) std::cout << "cleared vector; Vector length = " << newNotesCollection.size() << "\n";
                    }
                   //  if(debug) std::cout << "\n\n";
                }
            }
        }
    }
    
    
    
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
        
        
        //check wich value in the harmonyDistances vector is the smallest and obtainging its index
        int smallestIndex = 0;
        for(int i = 1; i < harmonyDistances.size() ; i++)
        {
            if (harmonyDistances[i] < harmonyDistances[smallestIndex])
                smallestIndex = i;
        }
        std::cout << "the closest harmony is " << possibleHarmonies[smallestIndex] <<"\n\n";
        
        return possibleHarmonies[smallestIndex];
    }
    
    std::vector<int> getsStoredNotesCollection()
    {
        return storedNotesCollection;
    }
    
    
private:
    bool debug = 1;
    bool enterHarmonyMode = 1;
    int maxHarmonyInput = 4;
    std::vector<int> newNotesCollection;
    std::string currentHarmony = "C";
    std::vector<int> storedNotesCollection;

    std::map<std::string, int> accidentals = {
        {"C",0}, {"F",-1}, {"G",1}, {"Bb",-2}, {"D",2}, {"Eb",-3}, {"A",3}, {"Ab",-4}, {"E",4}, {"Db",-5}, {"B",5}, {"Gb",-6}
    };
    HarmonyResolver hr;
    
    

};
