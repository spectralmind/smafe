///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafeDistancesCalc.h
//
// Class for distance calculation
// ------------------------------------------------------------------------
// $Id$
//
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////


#pragma once

// include autotools configuration
#include "config.h"

/** if defined IPP may be used for distance calculation
 * <p>Note that some distances may currently not provide implementations for both cases
 * (with IPP and without IPP)
 */
#define USE_IPP_FOR_DISTCALC 1

#include "smafeutil.h"
#include "smafeLogger.h"

#include <cassert>

#ifdef USE_IPP_FOR_DISTCALC
#include "ipp.h"
#endif





/** Class for distances calculation  */
class SmafeDistancesCalc
{
public:
	/** distance calc function pointer type */
	typedef double (*tDistfunc)(double, double*, int, double*, int);

	/** number of array elements (note that distancetype ids start counting from 1!) */
	static const long DISTFUNC_MAPPING_ARRAY_ELEMENTS = 6;

	/** mapping of distance type id to function */
	static tDistfunc distfunc_mapping[SmafeDistancesCalc::DISTFUNC_MAPPING_ARRAY_ELEMENTS];

	/** Calculate cosine distance
	 * @return the distance as a positive double, or a number < 0 if no distance can be calculated */
	static double getDistance_cosine(double p, double* arg0, int arg0len, double* arg1, int arg1len )  {
		// length of vectors must be the same
		if (arg0len != arg1len) {
			throw std::string("Feature vector lengths do not match: " + stringify(arg0len) + " vs " + stringify(arg1len));
		}


		double dist;

#ifdef USE_IPP_FOR_DISTCALC
		IppStatus status;

		// dot product
		double dp;
		status = ippsDotProd_64f(arg0, arg1, arg0len, &dp); // arg0len == arg1len!
		if (ippStsNoErr != status)
			throw std::string(ippGetStatusString(status));

		// magnitude (= L2 norm)
		double mg0, mg1;
		status = ippsNorm_L2_64f(arg0, arg0len, &mg0); // arg0len == arg1len!
		if (ippStsNoErr != status)
			throw std::string(ippGetStatusString(status));

		status = ippsNorm_L2_64f(arg1, arg0len, &mg1); // arg0len == arg1len!
		if (ippStsNoErr != status)
			throw std::string(ippGetStatusString(status));

		/*
		SMAFELOG_FUNC(DEBUG2, "dp: "  + stringify(dp));
		SMAFELOG_FUNC(DEBUG2, "mg0: "  + stringify(mg0));
		SMAFELOG_FUNC(DEBUG2, "mg1: "  + stringify(mg1));
		 */

		if (mg0 != 0.0 && mg1 != 0.0 && !my_isnan(dp / mg0 / mg1)) {
			// dot-product / (magnitude0 * magnitude1)
			dist = dp / mg0 / mg1;
		} else {
			if (mg0 == 0)
				SMAFELOG_FUNC(SMAFELOG_WARNING, "Magnitude of vector is 0. Cannot calculate cosine distance.");
			else
				SMAFELOG_FUNC(SMAFELOG_WARNING, "Magnitude of vector is 0. Cannot calculate cosine distance.");
			dist = -1;
		}

#else

		throw std::string("Not implemented without IPP. Define USE_IPP_FOR_DISTCALC to use this distance measure.");

#endif

		return dist;
	}




	/** Calculate L1 (Manhatten) distance */
	static double getDistance_L1(double p, double* arg0, int arg0len, double* arg1, int arg1len )  {
		// length of vectors must be the same

		if (arg0len != arg1len) {
			throw std::string("Feature vector lengths do not match: " + stringify(arg0len) + " vs " + stringify(arg1len));
		}

		double dist;

#ifdef USE_IPP_FOR_DISTCALC

		IppStatus status;


		status = ippsNormDiff_L1_64f(arg0, arg1, arg0len, &dist); // arg0len == arg1len!
		if (ippStsNoErr != status)
			throw std::string(ippGetStatusString(status));

		//SMAFELOG_FUNC(INFO, "ipp!");

#else

		throw std::string("Not implemented without IPP. Define USE_IPP_FOR_DISTCALC to use this distance measure.");


#endif

		return dist;
	}




	/** Calculate L2 (Euclidean) distance */
	static double getDistance_L2(double p, double* arg0, int arg0len, double* arg1, int arg1len )  {
		// length of vectors must be the same
		if (arg0len != arg1len) {
			throw std::string("Feature vector lengths do not match: " + stringify(arg0len) + " vs " + stringify(arg1len));
		}


		double dist;

#ifdef USE_IPP_FOR_DISTCALC

		IppStatus status;


		status = ippsNormDiff_L2_64f(arg0, arg1, arg0len, &dist); // arg0len == arg1len!
		if (ippStsNoErr != status)
			throw std::string(ippGetStatusString(status));

		//SMAFELOG_FUNC(INFO, "ipp!");

#else

		double sum = 0;
		double diff;
		//char tmp[100];
		for (int i = 0; i < arg0len; i++) {
			//sum += pow(arg0[i] - arg1[0], 2.0); <- buggy. Who finds the problem? ;-)
			diff = arg0[i] - arg1[i];
			sum += diff * diff;
			/*
			sprintf(tmp, "(%i) sum:0 %f", i, sum);
			elog(INFO, tmp);
			 */
		}
		dist = sqrt(sum);

		//SMAFELOG_FUNC(INFO, "normal!");


#endif

		return dist;
	}






	/** Calculate L-INF distance */
	static double getDistance_LINF(double p, double* arg0, int arg0len, double* arg1, int arg1len )  {
		// length of vectors must be the same
		if (arg0len != arg1len) {
			throw std::string("Feature vector lengths do not match: " + stringify(arg0len) + " vs " + stringify(arg1len));
		}


		double dist;

#ifdef USE_IPP_FOR_DISTCALC

		IppStatus status;


		status = ippsNormDiff_Inf_64f(arg0, arg1, arg0len, &dist); // arg0len == arg1len!
		if (ippStsNoErr != status)
			throw std::string(ippGetStatusString(status));

		//SMAFELOG_FUNC(INFO, "ipp!");

#else

		throw std::string("Not implemented without IPP. Define USE_IPP_FOR_DISTCALC to use this distance measure.");


#endif

		return dist;
	}






	/** Calculate generic L norm */
	static double getDistance_LN(double p, double* arg0, int arg0len, double* arg1, int arg1len )  {
		// length of vectors must be the same
		if (arg0len != arg1len) {
			throw std::string("Feature vector lengths do not match: " + stringify(arg0len) + " vs " + stringify(arg1len));
		}

		// p must not be 0
		if (! (p > 0)) {
			throw std::string("p must be > 0");
		}


		double dist;


		double sum = 0;
		double abs_diff;
		for (int i = 0; i < arg0len; i++) {

			abs_diff = abs(arg0[i] - arg1[i]);
			sum += pow(abs_diff, p);

		}
		dist = pow(sum, 1/p);




		return dist;
	}



};
