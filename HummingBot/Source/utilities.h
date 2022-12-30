/*
  ==============================================================================

    utilities.h

  ==============================================================================
*/

  //=============//
 //~~FUNCTIONS~~//
//=============//

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



std::string numberToChord (int chordDegree) {
    
    std::string romanNumeral;
    chordDegree = (chordDegree + 7) % 7;

    switch(chordDegree) {
            
        case 0:
            romanNumeral = "I";
            break;
        case 1:
            romanNumeral = "II";
            break;
        case 2:
            romanNumeral = "III";
            break;
        case 3:
            romanNumeral = "IV";
            break;
        case 4:
            romanNumeral = "V";
            break;
        case 5:
            romanNumeral = "VI";
            break;
        case 6:
            romanNumeral = "VII";
            break;
    }
    return romanNumeral;
}


  //============//
 //~~~~MAPS~~~~//
//============//

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


std::map<std::string, int> accidentals =
{
    {"C",0}, {"F",-1}, {"G",1}, {"Bb",-2}, {"D",2}, {"Eb",-3}, {"A",3}, {"Ab",-4}, {"E",4}, {"Db",-5}, {"B",5}, {"Gb",-6}
};

  //=====================//
 //~~Consts and Arrays~~//
//=====================//


const int genericHarmonyFormula[7] = {0,2,4,6,1,3,5}; //basic formula to generate any added seventh chord plus extension starting from a major scale formula
const int genericHarmonyFormulaSize = *(&genericHarmonyFormula + 1) - genericHarmonyFormula; //size of the array majorScaleFormula

const int majorScaleFormula[7] = {0,2,4,5,7,9,11}; //    basic formula of any major scale
const int majorScaleFormulaSize = *(&majorScaleFormula + 1) - majorScaleFormula; //size of the array majorScaleFormula

const int majorScale[7] = {0,2,4,5,7,9,11}; //    basic formula of any major scale
const int add7thChord[4] = {0,2,4,6}; //          basic formula of any added seventh chord within a major scale
const int majorScaleSize = *(&majorScale + 1) - majorScale; //size of the array majorScale
const int add7thChordSize = *(&add7thChord + 1) - add7thChord; //size of the array add7thChord

const int existingMajorScales = 12; //number of existing major scales, useful for iterations
const int existingChordDegrees = 7; //number of existing chords in a major scale, useful for iterations

#pragma once
