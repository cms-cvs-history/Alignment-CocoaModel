#include "Alignment/CocoaModel/interface/CocoaDaqReaderText.h"
#include "Alignment/CocoaModel/interface/Measurement.h"
#include "Alignment/CocoaModel/interface/Model.h"
#include "Alignment/CocoaUtilities/interface/ALIUtils.h"
#include "Alignment/CocoaUtilities/interface/GlobalOptionMgr.h"
#include "CondFormats/OptAlignObjects/interface/OpticalAlignMeasurements.h"


#include <iostream>

//----------------------------------------------------------------------
CocoaDaqReaderText::CocoaDaqReaderText(const std::string& fileName )
{

 if(ALIUtils::debug >= 5) std::cout << " CocoaDaqReaderText::CocoaDaqReaderText from file " << fileName << std::endl;

  CocoaDaqReader::SetDaqReader( this );

  theFilein = ALIFileIn::getInstance( fileName );

}

//----------------------------------------------------------------------
CocoaDaqReaderText::~CocoaDaqReaderText()
{
}

//----------------------------------------------------------------------
bool CocoaDaqReaderText::ReadNextEvent()
{
  std::vector<ALIstring> wordlist;
  //---------- read date
  //  ALIint retfil = filein.getWordsInLine(wordlist);
  // std::cout << "@@@@@@@@@@@@@@@ RETFIL " << retfil << std::endl;
  //if( retfil == 0 ) {
  if( theFilein.getWordsInLine(wordlist) == 0 ) {
    if(ALIUtils::debug>=4 ) std::cout << "@@@@ No more measurements left" << std::endl;
    return 0; 
  }

  ////--- Transform to time_t format and save it 
  //  struct tm tim;
  //t Model::setMeasurementsTime( tim );

  //set date and time of current measurement
  if( wordlist[0] == "DATE:" ) {
    Measurement::setCurrentDate( wordlist ); 
  } 

  //---------- loop measurements
  ALIint nMeas = Model::MeasurementList().size();
  if(ALIUtils::debug >= 4) {
    std::cout << " Reading " << nMeas << " measurements from file " << theFilein.name() 
    	 << " DATE: " << wordlist[1] << " " << wordlist[1] << std::endl;
  }
  ALIint ii;
  for(ii = 0; ii < nMeas; ii++) {
  if(ALIUtils::debug >= 4) std::cout << " reading meas  " << ii << " out of " <<  nMeas << std::endl;    
    theFilein.getWordsInLine(wordlist);  
    if( wordlist[0] == ALIstring("SENSOR2D") || wordlist[0] == ALIstring("TILTMETER") || wordlist[0] == ALIstring("DISTANCEMETER")  || wordlist[0] == ALIstring("DISTANCEMETER1DIM")  || wordlist[0] == ALIstring("COPS") ) {
      if( wordlist.size() != 2 ) {
	std::cerr << "!!!EXITING Model::readMeasurementsFromFile. number of words should be 2 instead of " << wordlist.size() << std::endl;
	ALIUtils::dumpVS( wordlist, " " );
	exit(1);
      }
      std::vector< Measurement* >::const_iterator vmcite;
      for( vmcite = Model::MeasurementList().begin();  vmcite != Model::MeasurementList().end(); vmcite++ ) {
	//-------- Measurement found, fill data
	/*	ALIint last_slash =  (*vmcite)->name().rfind('/');
	ALIstring oname = (*vmcite)->name();
	if( last_slash != -1 ) {
	  oname = oname.substr(last_slash+1, (*vmcite)->name().size()-1);
	  } 
	*/
	ALIint fcolon = (*vmcite)->name().find(':');
	ALIstring oname = (*vmcite)->name();
	oname = oname.substr(fcolon+1,oname.length());
	//-    std::cout << " measurement name " << (*vmcite)->name() << " short " << oname << std::endl;
	if( oname == wordlist[1] ) {
	  //-   std::cout << " measurement name found " << oname << std::endl;
	  if( (*vmcite)->type() != wordlist[0] ) {
	    std::cerr << "!!! Reading measurement from file: type in file is " 
		 << wordlist[0] << " and should be " << (*vmcite)->type() << std::endl;
	    exit(1);
	  }
	  Measurement* meastemp = *vmcite;
	  
	  GlobalOptionMgr* gomgr = GlobalOptionMgr::getInstance();
	  ALIbool sigmaFF = gomgr->GlobalOptions()["measurementErrorFromFile"];
	  //---------- Read the data 
	  for ( uint jj=0; jj < meastemp->dim(); jj++){
	    if(ALIUtils::debug >= 4) std::cout << " reading meas dim " << jj << " out of " <<  meastemp->dim() << std::endl;
	    theFilein.getWordsInLine( wordlist );
            ALIdouble sigma = 0.;
            if( !sigmaFF ) { 
	      // keep the sigma, do not read it from file 
	      const ALIdouble* sigmav = meastemp->sigma();
	      sigma = sigmav[jj];
	    }
	    //---- Check measurement value type is OK
	    if( meastemp->valueType(jj) != wordlist[0] ) {
	      theFilein.ErrorInLine();
	      std::cerr << "!!!FATAL ERROR: Measurement value type is " << wordlist[0] << " while in setup definition was " << meastemp->valueType(jj) << std::endl;
	      exit(1);
	    }
	    meastemp->fillData( jj, wordlist );
            if( !sigmaFF ) { 
	      meastemp->setSigma( jj, sigma );
	    }
	  }
	  meastemp->correctValueAndSigma();
	  break;
	}
      }
      if( vmcite == Model::MeasurementList().end() ) {
	for( vmcite = Model::MeasurementList().begin(); vmcite != Model::MeasurementList().end(); vmcite++ ) {
	  std::cerr << "MEAS: " << (*vmcite)->name() << " " << (*vmcite)->type() << std::endl;
	}
	std::cerr << "!!! Reading measurement from file: measurement not found in list: type in file is "  << wordlist[1]  << std::endl;
	exit(1);
      }
    } else {
      std::cerr << " wrong type of measurement: " << wordlist[0] << std::endl
	   << " Available types are SENSOR2D, TILTMETER, DISTANCEMETER, DISTANCEMETER1DIM, COPS" << std::endl;
      exit(1);
    }
  }
  //-  std::cout << " returning readmeasff" << std::endl;

  if( theFilein.eof() ) {
    return 0;
  } else {
    return 1;
  }
}

//----------------------------------------------------------------------
void CocoaDaqReaderText::BuildMeasurementsFromOptAlign( std::vector<OpticalAlignMeasurementInfo>& measList )
{

}
