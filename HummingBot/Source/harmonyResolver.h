/*
  ==============================================================================
    harmonyResolver.h
  ==============================================================================
*/

#pragma once
#include <vector>
class HarmonyResolver {
    
public:
    
    void findNewMajScale (std::vector<int> noteInput)
    {
        int majorScaleSize = *(&majorScale + 1) - majorScale;
        
        if(debug) std::cout << "DEBUG " << nameTag << " - Entered the function findNewMajScale " << "\n";

        
        for(int majorScaleNumber = 0; majorScaleNumber < existingMajorScales; majorScaleNumber++ ){
            
            if(debug) std::cout << "DEBUG " << nameTag << " - Entered the Loop that will cycle through al possible major scales " << "\n";
            
            int currentMajorScale[7];
            
            for (int i = 0; i < majorScaleSize; i++)
            {
                if(debug && i == 0) std::cout << "DEBUG " << nameTag << " - Entered the Loop that will generate the formula opf the next major scale \n Current major scale formula is " << "\n";

                currentMajorScale[i] = (majorScale[i] + majorScaleNumber +12) % 12;
                
                if(debug) std::cout << currentMajorScale[i]  << "\n";
            }
            
            if(debug) std::cout << "DEBUG " << nameTag << " - exit the loop that will generate the formula opf the next major scale " << "\n\n\n";

            
            for(int i = 0; i < noteInput.size(); i++)
            {
                if(debug) std::cout << "DEBUG " << nameTag << " - Entered the loop that will pick each note input and search it within the major scale \n Currently searching for note ";
                
                if(debug) std::cout << noteInput[i]  << "\n\n";

                
                for (int j = 0 ; j < majorScaleSize ; j++)
                {
                    if(debug && j == 0) std::cout << "DEBUG " << nameTag << " - Entered the loop that will compare current input note with each note of the current major scale \n\n";
                    
                    if(debug) std::cout << "DEBUG " << nameTag << " - comparing with note number "<< j <<" of the scale"<<"\n\n";
                    
                    bool found = 0;
                    
                    if(noteInput[i] == currentMajorScale[j])
                    {
                        if(debug) std::cout << "DEBUG " << nameTag << " note "<< i <<" found in major scale " << majorScaleNumber <<" on position " << j << "\n";
                        isThisTheNextScale = true;
                        j = majorScaleSize; //exit the for loop
                        found = 1;
                    }else{
                        if(debug) std::cout << "DEBUG " << nameTag << " note "<< i <<" is different than note in the " << j << " position of the major scale \n";

                    }
                    
                    if (j == majorScaleSize - 1 && found == 0){
                        if(debug) std::cout << "DEBUG " << nameTag << " note "<< i <<" I couldn't find note " << noteInput[i] << " within the major scale" << majorScaleNumber << "\n Proceding with next major scale\n\n";
                        isThisTheNextScale = false;
                    }
                    
                    //if(debug) std::cout << "DEBUG " << nameTag << " iterator step is " << j << "\n";
                }
                
                if(isThisTheNextScale == false){
                    if(debug) std::cout << "DEBUG " << nameTag << " note "<< i <<" not found in major scale, extiting loop \n";
                    i = noteInput.size(); //exit the for loop
                }
                if(debug) std::cout << "DEBUG " << nameTag << " the result of this calculation is that isThisTheNextScale for major Scale number "<< majorScaleNumber <<" is " << isThisTheNextScale << "\n\n";
            }
            if (isThisTheNextScale) majorScaleNumber = 12;
        }
    }
    
    
    
    
private:
    bool debug = 1;
    std::string nameTag = "harmonyResolver";
    int currentMajorScale = 0;
    int majorScale[7] = {0,2,4,5,7,9,11};
    int existingMajorScales = 12;
    bool isThisTheNextScale = false;
    
};
