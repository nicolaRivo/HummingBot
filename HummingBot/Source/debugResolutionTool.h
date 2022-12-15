/*
  ==============================================================================

    debugResolutionTool.h
    Created: 7 Nov 2022 12:56:00pm
    Author:  Nicola Rivosecchi

 
 I am aware it should be called DebugResolutionTool.h but I accidentally got it wrong and not sure now how to rename it safely.
  ==============================================================================
*/

#pragma once

/**
 I made this class for when i need to print out values that are processed for each sample (i.e. an LFO.process() value) so that it will print every nth message and avoid engulfing the program
 */
class DebugResolutionTool
{
public:
    
    /**updates the inner counter of the debug resolution tool. if the tool has compleated a whole cycle, it will return a debug message
     @param msg textual unchanging content of the debug message
     @param val float  dynamic content of the debug message
    */
    std::string nextStep(std::string msg = "", float val = 0.0f)
    {
        //get the remainder of the division between nuber of cycle and resolution so that you'd get a 1 every -resolution- number of cycles
        int resolutionCondition = cycle % resolution;
        
        if ( resolutionCondition == 1) {
            //initialize the variable tht will hold the converted float number
            std::string valStr;
            
            //turns the numeric value into a string
            valStr = std::to_string(val);
            
            //concatenates the textual message and the numeric value, plus a new line operator
            std::string finalMsg = nameTag + " || " + msg + " " + valStr + " \n";
            
            cycle++;
            
            return finalMsg;
        }
        
        cycle++;
        return "";
    }
    
    ///resets the counter
    void reset()
    {
        cycle = 1;
    }
    
    ///sets how often the debug messages should show up
    void setResolution(int r)
    {
        resolution = r;
    }
    
    ///sets name tag to trace back debug messages
    void setNameTag (std::string nt)
    {
        nameTag = nt;
    }

    
private:

    std::string message;
    int resolution = 200;
    int cycle = 1;
    
    // name tag to trace back debug messages
    std::string nameTag = "default debug resolution tool";
    
};
