///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafeAbstractOutput.h
//
// Abstract base class for non-database output of feature vectors
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

#include <vector>
#include "smafeAbstractFeatureVector.h"
#include "smafeNumericFeatureVector.h"
#include "smafeLogger.h"


/** Abstract class for output of feature vector */
class SmafeAbstractOutput
{
public:
	SmafeAbstractOutput(void) {}
	virtual ~SmafeAbstractOutput(void) {}

	/** Writes the feature vectors to the output medium (depending on concrete subclass) */
	virtual bool output(std::vector< SmafeAbstractFeatureVector_Ptr > fvs, std::string filename) = 0;

};
