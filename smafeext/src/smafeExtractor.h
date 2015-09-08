///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafeExtractor.h
//
// Feature extraction interface (abstract class)
// ------------------------------------------------------------------------
//
// $Id: smafeRPExtractor.h 134 2009-02-20 09:39:35Z ewald $
//
//
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////


/** Feature extraction interface (abstract class)  */
#pragma once


#include <map>

#include "smafeFVType.h"
#include "smafeopt.h"
#include "smafeLogger.h"
#include "smafeExtractorUtil.h"
#include "tAudioformat.h"







class SmafeExtractor {
public:
	// convenience Zuweisung
	static const int SSD_LEN = SmafeExtractorUtil::SSD_LEN;

	// ------------------------ constants
	/** maximum number of samples that can be processed
	 * This is to prevent memory allocation attempts of some GBs
	 *
	 * Currently: limit of 1.5 GB, i.e.201326592 samples
	 */
	static const long MAX_SAMPLES = 0xc000000;

	// ------------------------ properties
	Smafeopt opt;


	// ------------------------ methods
	SmafeExtractor(void) {}
	virtual ~SmafeExtractor(void) {}


	/** adds FV type instances that can be calculated with this extractor class to the container
	 * @param fvs IN/OUT container of instances of SmafeFVType */
	virtual void getCapabilities(SmafeFVType_Ptr_map &fvts) = 0;


	/** sets additional properties in FV type instances (like dimension_x, etc)
	 * The following props must be set:
	 * <ul>
	 * <li>dimension_x</li>
	 * <li>dimension_y></li>
	 * <li></li>
	 * <li></li>
	 * <li></li>
	 * <li></li>
	 * </ul>
	 * @param fvs IN/OUT container of instances of SmafeFVType */
	virtual void setFVTProperties(SmafeFVType_Ptr_map &fvts, const Smafeopt opt) = 0;

	/** Returns number of features to be extracted, depending on command line arguments
	 * @param fvs IN/OUT container of instances of SmafeFVType */
	int  getNumberOfFeaturesToBeExtracted(SmafeFVType_Ptr_map &fvts, Smafeopt *opt) {
		int n=0;
		for(SmafeFVType_Ptr_map::iterator iter = fvts.begin(); iter != fvts.end(); iter++) {
			if (opt->mapExtractGenerally[iter->first]) {
				n++;
			}
		} // end of iterator
		return n;
	}


	/** get features for one song
		@param buf the audio data
		@param audiodata the specifics of the audio file
		@param opt options struct
		@param fvts IN map of feature vector types
		@param fvs OUT vector of feature vectors. Need not be empty. Calculated fvs will be added at end of vector
		@param segmentfvs OUT vector of feature vectors of segments. Need not be empty. Calculated fvs will be added at end of vector (only if bReturnSegmentFeatures is true)
		@param sFilename filename
		@return true if any features have been extracted, or false if this extractor did not do anything

	 */
	virtual bool getFeatures(double* buf, tAudioformat *audiodata,  Smafeopt* opt,  SmafeFVType_Ptr_map* fvts,
			std::vector< SmafeAbstractFeatureVector_Ptr > &fvs, std::vector< SmafeAbstractFeatureVector_Ptr > &segmentfvs, std::string sFilename) = 0;


	/** Returns version of feature extraction algorithm.
		<p>The version number is, e.g., stored in the database
		@return version number as integer
	 */
	virtual int getVersion() {
		return 1;
	}

	/** returns string with all options */
	virtual std::string serializeConfig(std::string type)=0;

	/** Returns name of feature extractor
		@return string name
	 */
	virtual std::string getName()=0;

protected:
	/** vector of keys for feature vectors for this extractor */
	std::vector<std::string> vFvTypes;

	/** Adds a feature vector that can be extracted useing this extractor, to a map */
	void addCapability(SmafeFVType_Ptr_map &fvts, std::string name, std::string class_str) {
		SmafeFVType* fv;
		SmafeFVType_Ptr fv_ptr;

		fv = new SmafeFVType();
		fv->name = std::string(name);
		fv->version = getVersion();
		fv->class_id = std::string(class_str);
		fv_ptr.reset(fv);
		fvts[fv->name] = fv_ptr;
	}


};



typedef boost::shared_ptr<SmafeExtractor> SmafeExtractor_Ptr;
/** vector of extractors smart pointers */
typedef std::vector<SmafeExtractor_Ptr> SmafeExtractor_Ptr_vector;
/** vector of extractors*/
typedef std::vector<SmafeExtractor> SmafeExtractor_vector;
