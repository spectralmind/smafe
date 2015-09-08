///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2009 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// SmafeSomlibfileExtractor.h
//
// Extracts features from somlib vector files
// ------------------------------------------------------------------------
//
// $Id$
//
//
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////


#pragma once

#include "smafeutil.h"
#include "smafeopt.h"
//#include "tAudioformat.h"
#include "smafeExtractorUtil.h"
#include "smafeNumericFeatureVector.h"
#include "smafeExtractor.h"
#include "SmafeSomlibParser.h"

//#include <math.h>
//#include "ipp.h"
#include <string>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <map>





//------------------------------------------------
//
/** Feature extraction from audio data vector */
class DLLEXPORT SmafeSomlibfileExtractor : public SmafeExtractor
{
public:
	// typedefs


	// --- consts
	static const std::string FVTYPE_NAME_SOMLIB;
	static const std::string EXTRACTORNAME;

	// --- properties
	SmafeSomlibParser* somparser;
	tSomlibFVMap fvmap;


	// --- methods

	SmafeSomlibfileExtractor(void) {
		vFvTypes.push_back(SmafeSomlibfileExtractor::FVTYPE_NAME_SOMLIB);
	}

	virtual ~SmafeSomlibfileExtractor(void) {}

	virtual std::string getName() {
		return SmafeSomlibfileExtractor::EXTRACTORNAME;
	}

	virtual void getCapabilities(SmafeFVType_Ptr_map &fvts) {
		for(std::vector<std::string>::iterator iter = vFvTypes.begin(); iter < SmafeExtractor::vFvTypes.end(); ++iter) {
			addCapability(fvts, *iter, CLASS_ID_NUMERIC);
		} // end of iterator
	}



	virtual void setFVTProperties(SmafeFVType_Ptr_map &fvts, const Smafeopt opt) {

		// check if set
		if (opt.sSomlibfile != "") {

			SMAFELOG_FUNC(SMAFELOG_INFO, "parsing somlib file: " + opt.sSomlibfile);

			SmafeFVType* fvt_cur = fvts[SmafeSomlibfileExtractor::FVTYPE_NAME_SOMLIB].get();

			somparser = new SmafeSomlibParser(opt.sSomlibfile);

			// parse somlib header
			somparser->parseHeader();
			fvt_cur->setProperties(somparser->uiFvSize, 1, this->serializeConfig(SmafeSomlibfileExtractor::FVTYPE_NAME_SOMLIB) + "---" + somparser->sFvName);

			// change name: this is necessary in order to be able to distinguish various somlib imported types
			// NOTE: it is safe to change the name here, because in the map SmafeFVType_Ptr_map &fvts,
			// the key is still "Somlib". This key / string is then also used in the smafeopt->mapExtractForSong
			fvt_cur->name += "-" + somparser->sFvName;

			// parse somlib main section
			somparser->parseMain(fvmap, fvt_cur);

			delete somparser;
		} else {
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "opt.sSomlibfile is empty");
		}
	}

	virtual std::string serializeConfig(std::string type) {
		// disregard the type
		return opt.sSomlibfile;
	}


	/** This implementation of getFeatures does need neither the audio buffer nor the audioformat nor the segmentfvs.
	 * It uses only the sFilename from Smafeopt and the internal fv map
	 */
	virtual bool getFeatures(double* buf, tAudioformat *audiodata,  Smafeopt* opt,  SmafeFVType_Ptr_map* fvts,
				std::vector< SmafeAbstractFeatureVector_Ptr > &fvs, std::vector< SmafeAbstractFeatureVector_Ptr > &segmentfvs, std::string sFilename) {

		// check if this extractor is required (any features relevant for this extractor to be extracted?)
		bool bThisExtractorDoesSomething = false;

		for(std::vector<std::string>::iterator iter = vFvTypes.begin(); iter < SmafeExtractor::vFvTypes.end(); ++iter) {
			bThisExtractorDoesSomething = bThisExtractorDoesSomething || opt->mapExtractForSong[*iter];
		} // end of iterator

		// test exception
		//		throw std::string("test");

		if (bThisExtractorDoesSomething) {

			tSomlibFVMap::iterator iter;
			SmafeNumericFeatureVector_Ptr fvp;

			iter=fvmap.find(sFilename);
			if (iter != fvmap.end()) {
				SMAFELOG_FUNC(SMAFELOG_DEBUG, "Found in map: " + sFilename);
				// insert in vector
				fvs.push_back(iter->second);
				// remove from map
				fvmap.erase(iter);
			} else {
				SMAFELOG_FUNC(SMAFELOG_WARNING, "NOT found in map: " + sFilename);
			}
			return true;
		} else {
			// nothing to do for this extractor...
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Nothing to do for SmafeSomlibfileExtractor...");
			return false;
		}
	}

	/** generates a dummy filelist with the labels of the somlib file */
	void generateFilelistFromSomlibfile(std::stringstream &ssFiles) {
		ssFiles.str(std::string());

		for(tSomlibFVMap::iterator iter = fvmap.begin(); iter != fvmap.end(); iter++) {
			ssFiles << iter->first << std::endl;
		} // end of iterator
	}


private:


	// --- properties

};

