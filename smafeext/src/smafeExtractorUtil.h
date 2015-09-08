///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafeExtractorUtil.h
//
// Utility class for feature extractor classes
// ------------------------------------------------------------------------
//
// $Id$
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////

#pragma once


#include "ipp.h"
#include "smafeExportDefs.h"
#include "smafeLogger.h"
#include <math.h>


/** just PI */
const double PI = 3.14159265358979323846;



class DLLEXPORT SmafeExtractorUtil  {
public:
	//------------------------------------------------
	// ------- constants

	/** length of ssd features */
	static const int SSD_LEN = 7;



	//------------------------------------------------
	// ------- methods

	/** computes the median<br>
		<b>NOTE</b> that this function changes (ie., sorts) the buffer!
	*/
	static Ipp64f median(Ipp64f* buf, int len) {
		ippsSortAscend_64f_I(buf, len);
		if (len % 2 == 0) {
			// even
			return (buf[(len / 2)-1] + buf[len / 2 + 1-1]) / 2.0;
		} else {
			// odd
			return buf[((len+1) / 2)-1];
		}
	}

	/** Computes power spectrum for one frame
	 * @param LEN Length of frame (of wav buffer), also FFT size.
	 * @param wav time domain signal, of length LEN
	 * @param dest OUT: allocated buffer for <b>two-sided</b> power spectrum as return by Matlab's periodogram function
	 * @param win window to be applied, length LEN
	 * @param ctx IPP FFT info structure, must be created by host (caller)
	 *
	 * */
	static void computePowerspectrum(int LEN, Ipp64f* wav, Ipp64f* dest, Ipp64f* win, IppsFFTSpec_R_64f* ctx) {
		IppStatus status;
		Ipp64f* x = new Ipp64f[LEN];
		Ipp64fc* xc = new Ipp64fc[LEN];
		Ipp64f dotprod_win;

		// calc dotproduct to "compensates for the power of the window"
		// cmp to Matlab's computeperiodogram.m, line 61
		ippsDotProd_64f(win, win, LEN, &dotprod_win);

		// apply hann window to data
		ippsMul_64f( win, wav, x, LEN );  // --2009ok

		status = ippsFFTFwd_RToPack_64f_I(x, ctx, 0); // no buffer used: 0
		if (ippStsNoErr != status)
			throw std::string(ippGetStatusString(status));
		// get complex vector from packed format
		status = ippsConjPack_64fc(x, xc, LEN); // -- xc: 2009ok
		//printf_64fc("xc: complex result of fft?: (first 30 elems):\n", xc, 30, status);
		if (ippStsNoErr != status)
			throw std::string(ippGetStatusString(status));
		status = ippsPowerSpectr_64fc(xc, dest, LEN);
		if (ippStsNoErr != status)
			throw std::string(ippGetStatusString(status));

		// scale to same value range as Matlab
		// in Matlab this is split into functions
		// - computeperiodogram.m, line 69
		// - computepsd.m, lines 45/49 and 60
		// NB: Don't double unique Nyquist point

		dest[0] /= (2 * dotprod_win * PI);
		for (int j = 1; j < LEN-1; j++)
			dest[j] /= (dotprod_win * PI);
		dest[LEN-1] /= (2 * dotprod_win * PI);


		// release mem
		delete[] x;
		delete[] xc;
	}




	/** Returns statistical descriptors ("SSD") of vectors
	 * @param matrix Input vectors. The vectors are column vectors, ie., the statistical calculations
	 * are performed along each column (In case of bark band matrix: cols in matrix are Bark bands (basically opt->shNumBarkBands); rows in matrix are frames)
	 * @param cols number of columns
	 * @param rows number of rows
	 * @param dest OUT destination for values. dest is a SSD_LEN by "cols" matrix (matrix has "cols"  rows and SSD_LEN columns)
	 *
	 * calculate SSD features<br>
		used for both "original" SSD and temporal evolution of other features (TSSD, TRP, TRH)
		@param matrix 2D matrix with Bark bands as columns and frames as rows
		@param cols number of matrix' columns
		@param rows number of matrix' rows
		@param dest ssd, trp or tssd member of a feature struct. <br>
			Should <b>not</b> be initialized (will be allocated in method)
	 */
	static void getSSD(double* matrix, int cols, int rows, double* &dest) {
		// init ssd feature matrix
		SmafeLogger::smlog->log_alloc(SSD_LEN*cols*sizeof(double));
		dest = new double[SSD_LEN*cols];
		// set to 0
		for (int i = 0; i < SSD_LEN * cols; i++)
			dest[i] = 0;

		// sums along frames
		double* adSum = new double[cols];
		// squared sums along frames
		double* adSum2 = new double[cols];
		double* adMomentum3 = new double[cols];
		double* adMomentum4 = new double[cols];
		double* adSorted = new double[rows];
		double dMean, dVal, dVar; // tmp vars
		for (int b=0; b < cols; b++) {
			adSum[b] = 0;
			adSum2[b] = 0;
			adMomentum3[b] = 0;
			adMomentum4[b] = 0;
			for (int i=0; i<rows; i++) {
				dVal = matrix[INDEX2DARRAY(b, i, cols)];
				adSum[b] += dVal;
				adSum2[b] += dVal * dVal;
				adSorted[i] = dVal;
			}
			// mean -ok
			dMean = adSum[b] / double(rows);

			//std::cout << "vector " << b << ": ";

			for (int i=0; i<rows; i++) {
				dVal = matrix[INDEX2DARRAY(b, i, cols)];
				adMomentum3[b] += pow(dVal-dMean, 3);
				adMomentum4[b] += pow(dVal-dMean, 4);
//				std::cout << dVal << " ";
			}
//			std::cout << std::endl;
//			std::cout << "adMomentum4[b]: " << adMomentum4[b] << std::endl;

			adMomentum3[b] /= rows;
			adMomentum4[b] /= rows;


//			std::cout << "adMomentum4[b ] / rows: " << adMomentum4[b] << std::endl;


			// get sorted vector
			ippsSortAscend_64f_I(adSorted, rows);
			// mean -ok
			// always check for nan
			dest[INDEX2DARRAY(0, b, SSD_LEN)] = !my_isnan(dMean) ? dMean : 0;

			// var (formula from ipp documentation part 1 (DSP), p 8-142 (ie 663)
			//			-ok
			if (rows > 1)
				dVar = ((-1) * rows * dMean * dMean + adSum2[b]) / double(rows-1);
			else
				dVar = 0;
			dest[INDEX2DARRAY(1, b, SSD_LEN)] = !my_isnan(dVar) ? dVar : 0;


//			std::cout << "var: " << dest[INDEX2DARRAY(1, b, SSD_LEN)] << std::endl;


			// skewness (http://en.wikipedia.org/wiki/Skewness "sample skewness")
			// -ok
			if (dVar != 0.0 && !my_isnan(pow(dVar, 3.0 / 2.0)))
				dVal = adMomentum3[b] / pow(dVar, 3.0 / 2.0) ;
				//dest[INDEX2DARRAY(2, b, SSD_LEN)] = 99;
			else
				dVal = 0;
			dest[INDEX2DARRAY(2, b, SSD_LEN)] = !my_isnan(dVal) ? dVal : 0;

			// kurtosis (http://en.wikipedia.org/wiki/Kurtosis "sample kurtosis")
			// -ok
			if (dVar != 0.0 && !my_isnan(dVar * dVar))
				dVal = adMomentum4[b] / (dVar * dVar);
			else
				dVal = 0;
			dest[INDEX2DARRAY(3, b, SSD_LEN)] = !my_isnan(dVal) ? dVal : 0;


//			std::cout << "kurtosis: " << dest[INDEX2DARRAY(3, b, SSD_LEN)] << std::endl;

			// median  -ok
			if (rows % 2 == 0) {
				// even
				dVal = (adSorted[(rows / 2)-1] + adSorted[rows / 2 + 1-1]) / 2;
			} else {
				// odd
				dVal = adSorted[((rows+1) / 2)-1];
			}
			dest[INDEX2DARRAY(4, b, SSD_LEN)] = !my_isnan(dVal) ? dVal : 0;

			// min -ok
			dest[INDEX2DARRAY(5, b, SSD_LEN)] = !my_isnan(adSorted[0]) ? adSorted[0] : 0;
			// max -ok
			dest[INDEX2DARRAY(6, b, SSD_LEN)] = !my_isnan(adSorted[rows-1]) ? adSorted[rows-1] : 0;


		}
		// free mem
		delete[] adSum;
		delete[] adSum2;
		delete[] adMomentum3;
		delete[] adMomentum4;
		delete[] adSorted;
	}




};










/** the functions providing simple output of the result
	they are for real and complex data
	*/

#define genPRINT(TYPE,FMT) \
	void printf_##TYPE(const char* msg, Ipp##TYPE* buf, int len, IppStatus st ) { \
	int n; \
	if( st > ippStsNoErr ) \
	printf( "-- warning %d, %s\n", st, ippGetStatusString( st )); \
else if( st < ippStsNoErr ) \
	printf( "-- error %d, %s\n", st, ippGetStatusString( st )); \
	printf(" %s ", msg ); \
	for( n=0; n<len; ++n ) printf( FMT, buf[n] ); \
}
#define genPRINTcplx(TYPE,FMT) \
	void printf_##TYPE(const char* msg, Ipp##TYPE* buf, int len, IppStatus st ) { \
	int n; \
	if( st > ippStsNoErr ) \
	printf( "-- warning %d, %s\n", st, ippGetStatusString( st )); \
else if( st < ippStsNoErr ) \
	printf( "-- error %d, %s\n", st, ippGetStatusString( st )); \
	printf(" %s ", msg ); \
	for( n=0; n<len; ++n ) printf( FMT, buf[n].re, buf[n].im ); \
}
