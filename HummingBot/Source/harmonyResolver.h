/*
  ==============================================================================
    harmonyResolver.h
  ==============================================================================
*/

#pragma once
#include <vector>
class HarmonyResolver {
    
public:
    
   
    std::string numberToNote (int note) {
        
        std::string noteName;
        note = (note + 12) % 12;

        switch(note) {
                
            case 0:
                noteName = "C";
                break;
            case 1:
                noteName = "Db";
                break;
            case 2:
                noteName = "D";
                break;
            case 3:
                noteName = "Eb";
                break;
            case 4:
                noteName = "E";
                break;
            case 5:
                noteName = "F";
                break;
            case 6:
                noteName = "Gb";
                break;
            case 7:
                noteName = "G";
                break;
            case 8:
                noteName = "Ab";
                break;
            case 9:
                noteName = "A";
                break;
            case 10:
                noteName = "Bb";
                break;
            case 11:
                noteName = "B";
                break;
        }
        return noteName;
    }
    
/**
 this class will resolve a 1-4 note input with all the possible major scales they coulud belong to
 */
    std::vector<std::string> findNewMajScale (std::vector<int> noteInput)
    {
        std::vector<std::string> possibleMajorScales;
        int majorScaleSize = *(&majorScale + 1) - majorScale; //size of the array majorScale
        
        if(debug) std::cout << "DEBUG " << nameTag << " - Entered the function findNewMajScale " << "\n";

        //this first loop will cycle through all the 12 possible major scales to find one or more that contain all the notes the user has entered
        for(int majorScaleNumber = 0; majorScaleNumber < existingMajorScales; majorScaleNumber++ ){
            
            if(debug) std::cout << "DEBUG " << nameTag << " - Entered the Loop that will cycle through al possible major scales " << "\n";
            
            int currentMajorScale[7]; //create an array that will contain all the note numbers of the currently checked major scale (C = 0, Db = 1, D=2,etc)
            
            //this loop will generate the formula for the next major scale (C major is 0,2,4,5,7,9,11; Db major is obtained by adding 1 to each element and wrapping around when you get to 12: 1,3,5,6,8,10,0)
            for (int i = 0; i < majorScaleSize; i++)
            {
                if(debug && i == 0) std::cout << "DEBUG " << nameTag << " - Entered the Loop that will generate the formula opf the next major scale \n Current major scale formula is " << "\n";

                currentMajorScale[i] = (majorScale[i] + majorScaleNumber +12) % 12; //adding i to eah element and wrapping arond when reach 12
                
                if(debug) std::cout << currentMajorScale[i]  << "\n";
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
                    
                    if(noteInput[i] == currentMajorScale[j])
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
                if(debug) std::cout << "DEBUG " << nameTag << " - couldn't find any possible major scale.\n\n";
        }
        
        for(int i = 0; i < possibleMajorScales.size(); i++)
            std::cout << "possible major scale number "<< i <<" is: "<<possibleMajorScales[i]<<"\n";
        
        int unCazzoDiNulla;
        return possibleMajorScales;
    }
    
    
    
    
private:
    bool debug = 1;
    std::string nameTag = "harmonyResolver";
    int currentMajorScale = 0;
    int majorScale[7] = {0,2,4,5,7,9,11};
    int existingMajorScales = 12;
    bool isThisTheNextScale = false;
    
};
