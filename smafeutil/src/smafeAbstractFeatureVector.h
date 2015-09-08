///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafeAbstractFeatureVector.h
//
// Abstract class for one feature vector type
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
//
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "smafeExportDefs.h"
#include "smafeFVType.h"
#include <sstream>
#include <fstream>
#include <string>


// http://lists.boost.org/boost-users/2007/08/30324.php
//#include <boost/serialization/export.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

//#include <boost/serialization/export.hpp> - moved to subclass
//#include <boost/serialization/base_object.hpp>

// these are not needed anymore because we removed the .._ABSTRACT macro
// ues with boost 1.35
//#include <boost/serialization/is_abstract.hpp>
// use with boost 1.38 and greater
//#include <boost/serialization/assume_abstract.hpp>




// ---- Constants for class_id field --------
/** dummy class */
const std::string CLASS_ID_NOTSET = std::string("NOT_SET");
/** samfeNumericFeatureVector */
const std::string CLASS_ID_NUMERIC = std::string("NUMERIC");
/** samfeTestFeatureVector */
const std::string CLASS_ID_TEST = std::string("TEST");


/** Abstract class for one feature vector type */
class DLLEXPORT SmafeAbstractFeatureVector
{
public:
	/** feature vector type */
	SmafeFVType* fvtype;

	/** filename */
	std::string file_uri;

	/** foreign key track_id (db) */
	long track_id;
	/** foreign key file_id (db) */
	long file_id;

	// These numbers are used for segment feature vectors only
	/** segment number in track */
	long lSegmentnr;
	/** start sample */
	long lStartsample;
	/** segment length */
	long lLength;

	/** factory method */
	static SmafeAbstractFeatureVector* getInstance(const std::string class_id);

	SmafeAbstractFeatureVector(void);
	virtual ~SmafeAbstractFeatureVector(void);

	/** Returns memory size of this instance */
	virtual long sizeOf() const = 0;
	virtual void writeSomlibFileHeader(std::ofstream &outfile, size_t vector_size) = 0;
	virtual void writeSomlibFileEntry(std::ofstream &outfile) = 0;





private:

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int /* file_version */){
        //throw string("S11n of abstract class SmafeAbstractFeatureVector not implemented!");
		//SMAFELOG_FUNC(INFO, "in SmafeAbstractFeatureVector.serialize()")
		//std::cout << "in SmafeAbstractFeatureVector.serialize()" << std::endl;
    }




};


typedef boost::shared_ptr<SmafeAbstractFeatureVector> SmafeAbstractFeatureVector_Ptr;

//BOOST_IS_ABSTRACT(SmafeAbstractFeatureVector)

//BOOST_SERIALIZATION_ASSUME_ABSTRACT(SmafeAbstractFeatureVector)
//BOOST_CLASS_EXPORT_GUID(SmafeAbstractFeatureVector, "SmafeAbstractFeatureVector")
