///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafeAbstractFeatureVector.cpp
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
///////////////////////////////////////////////////////////////////////////


#include "smafeAbstractFeatureVector.h"

#include "smafeNumericFeatureVector.h"




SmafeAbstractFeatureVector::SmafeAbstractFeatureVector() {
}

SmafeAbstractFeatureVector::~SmafeAbstractFeatureVector()
{
}




SmafeAbstractFeatureVector* SmafeAbstractFeatureVector::getInstance(const std::string class_id) {

	if (class_id == CLASS_ID_NUMERIC) {
		return new SmafeNumericFeatureVector();
	}

	throw std::string("Class id not recognized: ") + class_id;
}



