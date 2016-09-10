//
//  ParameterLoader.cpp
//  
//
//  Created by Brian L Dorney on 10/09/16.
//
//

#include "ParameterLoader.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::pair;
using std::string;
using std::vector;

using QualityControl::Timing::getCharSeparatedList;
using QualityControl::Timing::getlineNoSpaces;
using QualityControl::Timing::stoiSafe;

using namespace QualityControl::Uniformity;

//Default Constructor
ParameterLoader::ParameterLoader(){
    strSecBegin_RunList = "[BEGIN_RUN_LIST]";
    strSecEnd_RunList   = "[END_RUN_LIST]";
}

//Maps a run number, found in an input filename, to an input run found in the input config file
//Only those input files having a run number will be returned
//  ->First is run number
//  ->Second is run name
std::vector<std::pair<int, string> > ParameterLoader::getPairedRunList(ifstream &file_Input, bool bVerboseMode){
    //Variable Declaration
    bool bHeaderEnd = false;
    
    int iNum_Run = -1;
    
    string strLine = "";
    
    vector<std::pair<int, string> > vec_retPairedRuns;
    
    //Loop Through Data File
    //Check for faults immediately afterward
    //------------------------------------------------------
    while ( getlineNoSpaces(file_Input, strLine )  ) { //Loop over input file
        //Skip commented lines
        if (strLine.compare(0,1,"#") == 0) continue;
        
        //Check for start of run list header
        if ( strLine.compare( strSecBegin_RunList ) == 0 ) { //Case: Run list header
            
            while ( getlineNoSpaces(file_Input, strLine) ) { //Loop through run list header
                //Skip commented lines
                if (strLine.compare(0,1,"#") == 0 ) continue;
                
                //Has the header ended?
                if ( strLine.compare( strSecEnd_RunList ) == 0 ) { //Case: End of run list header
                    if (bVerboseMode) { //Case: User Requested Verbose Input/Output
                        cout<<"ParameterLoader::getRunMap(): End of run list header reached!\n";
                        cout<<"ParameterLoader::getRunMap(): The following runs will be analyzed:\n";
                        
                        for (auto iterPairedRuns = vec_retPairedRuns.begin(); iterPairedRuns != vec_retPairedRuns.end(); ++iterPairedRuns) { //Loop over stored runs
                            cout<<"\t"<<"Run"<<(*iterPairedRuns).first<<"\t"<<(*iterPairedRuns).second<<endl;
                        } //End Loop over stored runs
                    } //End Case: User Requested Verbose Input/Output
                    
                    bHeaderEnd = true;
                    break;
                } //End Case: End of run list header
                
                //Get the run number from the file name
                iNum_Run = getRunNumber(strLine);
                
                //Set the run or inform the user of a problem
                if (iNum_Run > -1 ) { //Case: Success, Run Set!
                    vec_retPairedRuns.push_back(std::make_pair(iNum_Run, strLine ) );
                } //End Case: Success, Run Set!
                else{ //End Case: Input not understood/not parsed correctly
                    cout<<"ParameterLoader::getRunMap() - ERROR!!! input file:\n";
                    cout<<"\t"<<strLine.c_str()<<endl;
                    
                    cout<<"ParameterLoader::getRunMap(): Run number not found/parsed from filename!\n";
                    cout<<"ParameterLoader::getRunMap(): you must have the field 'RunX' in the filename, separated by underscores '_', for some unique integer X\n";
                    cout<<"ParameterLoader::getRunMap(): This file will be skipped!!! Please cross-check input file!!!\n";
                    
                    continue;
                } //Case: Input not understood/not parsed correctly
            } //End loop through run list header
        } //End Case: Run list header
        
        if (bHeaderEnd) break;
    } //End Loop over input file
    if ( file_Input.bad() && bVerboseMode) {
        perror( "getRunList(): error while reading config file" );
        Timing::printStreamStatus(file_Input);
    }
    
    //Do not close the input file, it will be used elsewhere
    
    return vec_retPairedRuns;
} //End ParameterLoader::getPairedRunList()

//Gets the list of input runs from the input config file
vector<string> ParameterLoader::getRunList(ifstream &file_Input, bool bVerboseMode){
    //Variable Declaration
    bool bHeaderEnd = false;
    
    string strLine = "";
    
    vector<string> vec_strRetRuns;
    
    //Loop Through Data File
    //Check for faults immediately afterward
    //------------------------------------------------------
    while ( getlineNoSpaces(file_Input, strLine )  ) { //Loop over input file
        //Skip commented lines
        if (strLine.compare(0,1,"#") == 0) continue;
        
        //Check for start of run list header
        if ( strLine.compare( strSecBegin_RunList ) == 0 ) { //Case: Run list header
            
            while ( getlineNoSpaces(file_Input, strLine) ) { //Loop through run list header
                //Skip commented lines
                if (strLine.compare(0,1,"#") == 0 ) continue;
                
                //Has the header ended?
                if ( strLine.compare( strSecEnd_RunList ) == 0 ) { //Case: End of run list header
                    if (bVerboseMode) { //Case: User Requested Verbose Input/Output
                        cout<<"ParameterLoader::getRunList(): End of run list header reached!\n";
                        cout<<"ParameterLoader::getRunList(): The following runs will be analyzed:\n";
                        
                        for (int i=0; i < vec_strRetRuns.size(); ++i) { //Loop over stored runs
                            cout<<"\t"<<vec_strRetRuns[i]<<endl;
                        } //End Loop over stored runs
                    } //End Case: User Requested Verbose Input/Output
                    
                    bHeaderEnd = true;
                    break;
                } //End Case: End of run list header
                
                vec_strRetRuns.push_back(strLine);
            } //End loop through run list header
        } //End Case: Run list header
        
        if (bHeaderEnd) break;
    } //End Loop over input file
    if ( file_Input.bad() && bVerboseMode) {
        perror( "getRunList(): error while reading config file" );
        Timing::printStreamStatus(file_Input);
    }
    
    //Do not close the input file, it will be used elsewhere
    
    return vec_strRetRuns;
} //End ParameterLoader::getRunList()

int ParameterLoader::getRunNumber(std::string strRunName){
    //Variable Declaration
    int iRetVal;
    
    //cout<<"getRunNumber() strRunName = " << strRunName.c_str() << endl;
    
    std::transform(strRunName.begin(), strRunName.end(), strRunName.begin(), toupper);
    
    //cout<<"getRunNumber() post transfer strRunName = " << strRunName.c_str() << endl;
    
    vector<string> vec_strParsedName = getCharSeparatedList(strRunName, '_');
    
    /*cout<<"i\tParsedInput\tContains\n";
     for(int i=0; i < vec_strParsedName.size(); ++i){
     contains cont("RUN");
     cout<<i<<"\t"<<vec_strParsedName[i]<<"\t"<<cont(vec_strParsedName[i])<<endl;
     }*/
    
    auto iterStr = std::find_if(vec_strParsedName.begin(), vec_strParsedName.end(), QualityControl::Uniformity::contains("RUN") );
    
    //cout<<"getRunNumber() - iterStr =
    
    //Input file name did not contain the phrase "RUN"
    if ( iterStr == vec_strParsedName.end() ) {
        return -1;
    }
    
    //cout<<"getRunNumber() - iterStr = " << (*iterStr).c_str() << endl;
    
    //(*iterStr).erase(remove((*iterStr).begin(), (*iterStr).end(), "RUN"), (*iterStr).end() );
    
    //cout<<"getRunNumber() - (*iterStr).find('RUN') = " << (*iterStr).find("RUN") << endl;
    
    (*iterStr).erase( (*iterStr).find("RUN"), 3);
    iRetVal = stoiSafe( (*iterStr) );
    
    return iRetVal;
} //End ParameterLoader::getRunNumber()