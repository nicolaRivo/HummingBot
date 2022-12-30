/*
  ==============================================================================
    harmonyResolver.h
  ==============================================================================
*/

#pragma once
#include <vector>

#include "utilities.h"


/**
 this class has all the logic to transform a midi notes collection into a major scale and/or a added 7th chord that contains all of those.
 It holds the information about the current keyCentre in its private variables and can use it to favour a key change or a key conservation.
 NB that in the description I use the terms "Key Centre" and "Major Scale" interchangeably, to account for the possibility that at some point could be used also to work with Melodic/Harmonic minor or more exotic scale kinds
 */

class HarmonyResolver {
    
public:
    
    /**
     findNewPossibleChordDegrees
     this function takes in a vector of notes -noteInput- and a key centre -currentKeyCentre-.  the function will find added 7th chords within the key centre. A vector containing all the possible chords is then returned. If no
     
     @param noteInput its an array of names of notes (i.e. D, F, Bb)
     @param currentKeyCentre is a key centre name (i.e. Bb)
     @return an vector of possible chord degrees that have those notes in such key centre (i.e. I, VI )
     */
    std::vector<std::string> findNewPossibleChordDegrees(std::vector<int> noteInput, std::string currentKeyCentre)
    {
        std::vector<std::string> possibleChordDegrees;
        
        std::string currentKeyCentreName = currentKeyCentre; //save the name of the current major scale to be able to have clearer debug
        int currentKeyCentreNumber =  notesToNumber[currentKeyCentre]; //save the number of the current major scale to be able to do calculations
        int currentMajorScaleFormula[7];
        
        
        //calculating the major scale formula for the current key centre
        for (int i = 0; i < majorScaleSize; i++)
        {
            currentMajorScaleFormula[i] = (majorScale[i] + currentKeyCentreNumber +12) % 12; //adding i to each element and wrapping arond when reach 12
        }
        
        
        //this first loop will cycle through all the 7 possible chord degrees to find one or more that contain all the notes the user has entered
        for(int chordDegreeNumber = 0; chordDegreeNumber < existingChordDegrees; chordDegreeNumber++ ){
            
            if(debug) std::cout << "DEBUG " << nameTag << " - Entered the Loop that will cycle through al possible chord degrees " << "\n";
            
            int currentChord[4]; //create an array that will contain all the note numbers of the currently checked add7th chord (root = 0, 3rd = 2, 5th = 4, 7th = 6)
            
            //this loop will generate the formula for the next add7th chord (I 0,2,4,6; II  is obtained by adding 1 to each element and wrapping around when you get to 7: 1,3,5,0)
            for (int i = 0; i < add7thChordSize; i++)
            {
                if(debug && i == 0) std::cout << "DEBUG " << nameTag << " - Entered the Loop that will generate the formula for the next chord degree \n Current chord degree formula is " << "\n";

                currentChord[i] = (add7thChord[i] + chordDegreeNumber + 7) % 7; //adding i to each element and wrapping arond when reach 7;

                if(debug) std::cout << "position " << currentChord[i] << " note " << numberToNote (currentMajorScaleFormula[currentChord[i]])  << "\n";
            }
            
            if(debug) std::cout << "DEBUG " << nameTag << " - exit the loop that will generate the formula opf the next chord degree " << "\n\n\n";

            //once the next chord degree has been calculated,  this loop will pick one by one each note entered by the user and check if they all exisit within this major scale
            for(int i = 0; i < noteInput.size(); i++)
            {
                if(debug) std::cout << "DEBUG " << nameTag << " - Entered the loop that will pick each note input and search it within the chord degree \n Currently searching for note " << numberToNote(noteInput[i])  << "\n\n";
                //this loop will compare the currently picked note with all the notes within the chord degree. if a note doesn't exsts in the chord, it will exit the loop and move on to the next chord degree
                for (int j = 0 ; j < add7thChordSize ; j++)
                {
                    if(debug && j == 0) std::cout << "DEBUG " << nameTag << " - Entered the loop that will compare current input note with each note of the current chord degree \n\n";
                    if(debug) std::cout << "DEBUG " << nameTag << " - comparing with note number "<< j <<" of the chord"<<"\n\n";
                    
                    bool found = 0;
                    
                    if(debug) std::cout << "DEBUG " << nameTag << " - I am comparing imput note note " << numberToNote(noteInput[i])<< " with chord note " << numberToNote(currentMajorScaleFormula[currentChord[j]]) << ".\n If they are equal i will enter the next if statement\n\n";
                        
                    if(noteInput[i] == currentMajorScaleFormula[currentChord[j]])
                    {
                        if(debug) std::cout << "DEBUG " << nameTag << " note "<< i <<" found in chord " << chordDegreeNumber <<" on position " << j << "\n";
                        isThisTheNextChord = true;  //if the note was found in the chord
                        j = add7thChordSize;         //obtain the condition necessary to exit the loop
                        found = 1;                  //report the information that we found the note
                    }else{
                        if(debug) std::cout << "DEBUG " << nameTag << " note "<< i <<" is different than note in the " << j << " position of the chord degree \n";
                    }
                    //if we cycled through all the notes of this major scale and we havent found a match with the current user input note, we move on to the next scale
                    if (j == add7thChordSize - 1 && found == 0){
                        if(debug) std::cout << "DEBUG " << nameTag << " note "<< i <<" I couldn't find note " << noteInput[i] << " within the  " << numberToChord(chordDegreeNumber) << " chord degree. \n Proceding with next chord degree.\n\n";
                        isThisTheNextChord = false;  //report the information that we this isn't the scale that fits our purposes
                    }
                }
                
                //if a single note was not found within this scale, there is no need to check the other notes and we can move on to the next major scale
                if(isThisTheNextChord == false)
                {
                    if(debug) std::cout << "DEBUG " << nameTag << " note "<< i <<" not found in chord degree, extiting loop. \n";
                    
                    i = noteInput.size(); //generate the condition that will allow us tho leave the for loop for this scale
                }
                
                if(debug) std::cout << "DEBUG " << nameTag << " the result of this calculation is that isThisTheNextScale for major Scale number "<< numberToChord(chordDegreeNumber) <<" is " << isThisTheNextChord << ".\n\n";
            }
            if (isThisTheNextChord)
                possibleChordDegrees.push_back(numberToChord(chordDegreeNumber));
            
            if (possibleChordDegrees.size()==0 && chordDegreeNumber == 6)
                if(debug) std::cout << "DEBUG " << nameTag << " - couldn't find any possible chord degree.\n\n";

        }
        
        
        return possibleChordDegrees;
    }
    
    
    
/**
 this function takes in a vector of notes -noteInput- and finds one or more major scales that contain all those notes
 
 @param noteInput its an array of names of notes (i.e. D, F, Bb)
 @return a vector of possible chord degrees that have those notes in such key centre (i.e. F, Bb, Eb). if no major scales could be find, the vector will just contain a 0 in character form.
 */
    std::vector<std::string> findNewPossibleMajorScales (std::vector<int> noteInput)
    {
        std::vector<std::string> possibleMajorScales;
        
        if(debug) std::cout << "DEBUG " << nameTag << " - Entered the function findNewPossibleMajorScales " << "\n";

        //this first loop will cycle through all the 12 possible major scales to find one or more that contain all the notes the user has entered
        for(int majorScaleNumber = 0; majorScaleNumber < existingMajorScales; majorScaleNumber++ ){
            
            if(debug) std::cout << "DEBUG " << nameTag << " - Entered the Loop that will cycle through al possible major scales " << "\n";
            
            int currentlyCheckedMajorScale[7]; //create an array that will contain all the note numbers of the currently checked major scale (C = 0, Db = 1, D=2,etc)
            
            //this loop will generate the formula for the next major scale (C major is 0,2,4,5,7,9,11; Db major is obtained by adding 1 to each element and wrapping around when you get to 12: 1,3,5,6,8,10,0)
            for (int i = 0; i < majorScaleSize; i++)
            {
                if(debug && i == 0) std::cout << "DEBUG " << nameTag << " - Entered the Loop that will generate the formula opf the next major scale \n Current major scale formula is " << "\n";

                currentlyCheckedMajorScale[i] = (majorScale[i] + majorScaleNumber +12) % 12; //adding i to eah element and wrapping arond when reach 12
                
                if(debug) std::cout << currentlyCheckedMajorScale[i]  << "\n";
            }
            
            if(debug) std::cout << "DEBUG " << nameTag << " - exit the loop that will generate the formula opf the next major scale " << "\n\n\n";

            //once the next major scale has been calculated,  this loop will pick one by one each note entered by the user and check if they all exisit within this major scale
            for(int i = 0; i < noteInput.size(); i++)
            {
                if(debug) std::cout << "DEBUG " << nameTag << " - Entered the loop that will pick each note input and search it within the major scale \n Currently searching for note ";
                if(debug) std::cout << noteInput[i]  << "\n\n";

                //this loop will compare the currently picked note with all the notes within the major scale. if a note doesn't exsts in the scale, it will exit the loop and move on to the next major scale
                for (int j = 0 ; j < majorScaleSize ; j++)
                {
                    if(debug && j == 0) std::cout << "DEBUG " << nameTag << " - Entered the loop that will compare current input note with each note of the current major scale \n\n";
                    if(debug) std::cout << "DEBUG " << nameTag << " - comparing with note number "<< j <<" of the scale"<<"\n\n";
                    
                    bool found = 0;
                    
                    if(noteInput[i] == currentlyCheckedMajorScale[j])
                    {
                        if(debug) std::cout << "DEBUG " << nameTag << " note "<< i <<" found in major scale " << majorScaleNumber <<" on position " << j << "\n";
                        isThisTheNextScale = true;  //if the note was found in the
                        j = majorScaleSize;         //obtain the condition necessary to exit the loop
                        found = 1;                  //report the information that we found the note
                    }else{
                        if(debug) std::cout << "DEBUG " << nameTag << " note "<< i <<" is different than note in the " << j << " position of the major scale \n";
                    }
                    //if we cycled through all the not4s of this major scale and we havent found a match with pthe current user input note, we move on to the next scale
                    if (j == majorScaleSize - 1 && found == 0){
                        if(debug) std::cout << "DEBUG " << nameTag << " note "<< i <<" I couldn't find note " << noteInput[i] << " within the  " << numberToNote(majorScaleNumber) << " major scale. \n Proceding with next major scale.\n\n";
                        isThisTheNextScale = false;  //report the information that we this isn't the scale that fits our purposes
                    }
                    
                    //if(debug) std::cout << "DEBUG " << nameTag << " iterator step is " << j << "\n";
                }
                
                //if a single note was not found within this scale, there is no need to check the other notes and we can move on to the next major scale
                if(isThisTheNextScale == false)
                {
                    if(debug) std::cout << "DEBUG " << nameTag << " note "<< i <<" not found in major scale, extiting loop. \n";
                    i = noteInput.size(); //generate the condition that will allow us tho leave the for loop for this scale
                }
                
                if(debug) std::cout << "DEBUG " << nameTag << " the result of this calculation is that isThisTheNextScale for major Scale number "<< numberToNote(majorScaleNumber) <<" is " << isThisTheNextScale << ".\n\n";
            }
            if (isThisTheNextScale) possibleMajorScales.push_back(numberToNote(majorScaleNumber));
            if (possibleMajorScales.size()==0 && majorScaleNumber == 11)
            {
                if(debug) std::cout << "DEBUG " << nameTag << " - couldn't find any possible major scale.\n\n";
                possibleMajorScales.clear();
                possibleMajorScales.push_back(currentMajorScale);
                possibleMajorScales.push_back("0");

                return possibleMajorScales;
            }
        }
        
        for(int i = 0; i < possibleMajorScales.size(); i++)
            if(chat) std::cout << "possible major scale number "<< i <<" is: "<<possibleMajorScales[i]<<"\n";

        //if the setting is to prioritize a key change and more than one possible major scale is found, exclude the current major scale
        if (prioritizeKeyChange && possibleMajorScales.size() > 1)
        {
            std::vector<std::string> newPossibleMajorScales;
            //cycle thorugh all the possible major scales and exclude the currently adopted one
            for(int i = 0 ; i < possibleMajorScales.size(); i++)
            {
                if(possibleMajorScales[i] != currentMajorScale)
                    newPossibleMajorScales.push_back(possibleMajorScales[i]);
            }
            possibleMajorScales = newPossibleMajorScales;
        }

        return possibleMajorScales;
    }
    
    
    /**
     this function will figure out wich key centre to adopt from a given vector of possible choices. eventually the user will be able to input the modality of this choice, but at the moment the choice will be the key centre that requires less steps around the cycle of fifths.
     @param currentHarmony string holding the name of the  harmony we are starting from
     @param possibleHarmonies vector of strings of the possible harmonies we will be leading to
     @param mode this is just a placeholder for a possible future update where the user could choose between different ways of selecting the new harmony
     */
    std::string findNearestKeyCentre(std::string currentHarmony, std::vector<std::string> possibleHarmonies, std::string mode = "nearest")
    {
        std::vector<int> harmonyDistances;//this vector will collect all the result of the harmonic distances calculations
        int currentHarmonyAccidentals = accidentals[currentHarmony]; //how many accidentals does my current harmony have?
        
        if (chat) std::cout << "my current harmony is " << currentHarmony <<"\n\n";
        
        for (int i = 0; i < possibleHarmonies.size(); i++)
        {
            if (chat) std::cout << "checking distance with " << possibleHarmonies[i] <<"\n\n";

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
        if(chat) std::cout << "the closest harmony is " << possibleHarmonies[smallestIndex] <<"\n\n";
        
        return possibleHarmonies[smallestIndex];
        
    }
    
    ///this will set the option to prioritise a key change or a key preservation
    void setPrioritizeKeyChange (bool b)
    {
        prioritizeKeyChange = b;
    }
    
    ///sets the current major scale private variable
    void setCurrentMajorScale (std::string _currentMajorScale)
    {
        currentMajorScale = _currentMajorScale;
    }

    ///gets the current major scale private variable
    std::string getCurrentMajorScale ()
    {
        return currentMajorScale;
    }
    
    
private:
    bool debug = 0; //lower level debug
    bool chat = 0;  //higher level debug
    
    
    std::string nameTag = "harmonyResolver";
    std::string currentMajorScale = "C";
    std::string currentChords = "I"; // this is not really used at the moment, the next chord is randomly selected from the array of possible chords, but in the future a logic similar to findNearestKeyCentre function can easily be implemented


    
    bool isThisTheNextScale = false;
    bool isThisTheNextChord = false;

    
    bool prioritizeKeyChange = true;
    


};
