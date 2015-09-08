///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2010 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafeDistancesCalc.cpp
//
// Class for distance calculation
// ------------------------------------------------------------------------
// $Id: smafeDistancesCalc.h 338 2009-09-23 18:07:10Z ewald $
//
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////


#include "smafeDistancesCalc.h"

// Static initialization
// (note that distancetype ids start counting from 1!!)
SmafeDistancesCalc::tDistfunc
		SmafeDistancesCalc::distfunc_mapping[SmafeDistancesCalc::DISTFUNC_MAPPING_ARRAY_ELEMENTS] = {
				NULL,
				&SmafeDistancesCalc::getDistance_L2,
				&SmafeDistancesCalc::getDistance_L1,
				&SmafeDistancesCalc::getDistance_LINF,
				&SmafeDistancesCalc::getDistance_LN,
				&SmafeDistancesCalc::getDistance_cosine };

