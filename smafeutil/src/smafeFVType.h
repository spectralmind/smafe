///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafeFVType.h
//
// Mapping class for a record of table FeatureVectorType
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

//#include "smafeExtractor.h"
#include "smafeAbstractFeatureVector.h"

#include <string>
#include <map>
#include <boost/shared_ptr.hpp>




class SmafeFVType {
	public:

	/** default constructor
	 * <p>initializes values to dummy values like -1 or "not set"</p>
	 */
	SmafeFVType(void) : id(-1), name("notset"), version(-1), dimension_x(-1), dimension_y(-1), parameters("(not set)"), class_id("NOTSET") {}
	/** copy constructor
	 * <p>Copys all values from the src to the new instance</p>
	 */
	//SmafeFVType(const SmafeFVType &src) : id(src.id), name("notset"), version(-1), dimension_x(-1), dimension_y(-1), parameters("(not set)"), class_id("NOTSET") {}

	virtual ~SmafeFVType(void) {}

	void setProperties(long dimx, long dimy, std::string params) {
		dimension_x = dimx;
		dimension_y = dimy;
		parameters = params;
	}

	/** comparison operator: objects are equal if all fields are equal, except for id */
	bool operator==(const SmafeFVType &other) const {
		return name == other.name && version == other.version && dimension_x == other.dimension_x && dimension_y == other.dimension_y && parameters == other.parameters;
	}

	/** comparison operator:  */
	bool operator!=(const SmafeFVType &other) const {
		 return !(*this == other);
	}


	// --- mapping of database table record
	/** primary key */
	long id;
	std::string name;
	long version;
	long dimension_x;
	long dimension_y;
	std::string parameters;
	std::string class_id;

	// --- program internal use
	/** which extractor class provides this fv type? */
	//SmafeExtractor provider;



};

typedef boost::shared_ptr<SmafeFVType> SmafeFVType_Ptr;
typedef std::map<std::string,  SmafeFVType > SmafeFVType_map;
typedef std::map<std::string,  SmafeFVType_Ptr > SmafeFVType_Ptr_map;
