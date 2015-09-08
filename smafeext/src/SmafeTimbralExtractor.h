///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// SmafeTimbralExtractor.h
//
// Extracts timbral features
// Layout SPECTRAL: | Spectral Centroid | Spectral Spread | Spectral Flux | Spectral Rolloff
// Layout TIMEDOM:  | Zerocrossing rate | RMS
// ------------------------------------------------------------------------
//
// $Id: smafeRPExtractor.h 237 2009-06-12 12:10:37Z ewald $
//
//
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////


/** Feature extraction from raw audio signal: ZCRS */
#pragma once

#include "smafeutil.h"
#include "smafeopt.h"
#include "tAudioformat.h"
#include "smafeExtractorUtil.h"
#include "smafeNumericFeatureVector.h"
#include "smafeExtractor.h"

#include <math.h>
#include "ipp.h"
#include <string>
#include <boost/shared_ptr.hpp>
#include <vector>



/** struct for segments' features (basically an array of ... with array length */
struct tTimbralFeatures4Segs{
	/** number of segments */
	int iNumSegs;
	/** length of each segment */
	int iLenSeg;
	/** array of tRPFeatures structs */
	//	tTimbralFeatures* features;
	/** start (offset) of segments */
	int* aiOffsets;

	/** zero crossings rate */
	double* zcr;
	/** root mean square */
	double* rms;
	/** Spectral Centroid */
	double* spc;
	/** Spectral Spread */
	double* sps;
	/** Spectral Flux */
	double* spf;
	/** Spectral Rolloffpoint */
	double* spr;
};


//------------------------------------------------
//
/** Feature extraction from audio data vector */
class DLLEXPORT SmafeTimbralExtractor : public SmafeExtractor
{
public:
	// --- consts
	// feature vector types offered by this extractor
	/*
	// individual
	static const std::string FVTYPE_NAME_ZEROCROSSINGRATE;
	static const std::string FVTYPE_NAME_RMS;
	static const std::string FVTYPE_NAME_SPECTRALCENTROID;
	static const std::string FVTYPE_NAME_SPECTRALSPREAD;
	static const std::string FVTYPE_NAME_SPECTRALFLUX;
	static const std::string FVTYPE_NAME_SPECTRALROLLOFF;
	 */
	// aggregated
	static const std::string FVTYPE_NAME_TIMEDOMAINFEATURES;
	static const std::string FVTYPE_NAME_SPECTRALFEATURES;
	static const std::string EXTRACTORNAME;


	/** rolloff point for spectral rolloff */
	static const double ROLLOFFPOINT = 0.85;

	/** columns for time domain features (currently zerocrossings and rms */
	static const long COLS_TIMEDOMAIN = 2;

	/** columns for spectral features (currently spectral centroid, .. spread, .. flux and .. rolloff */
	static const long COLS_SPECTRAL= 4;


	// --- methods

	SmafeTimbralExtractor(void) {
		vFvTypes.push_back(SmafeTimbralExtractor::FVTYPE_NAME_TIMEDOMAINFEATURES);
		vFvTypes.push_back(SmafeTimbralExtractor::FVTYPE_NAME_SPECTRALFEATURES);
	}

	virtual ~SmafeTimbralExtractor(void) {}

	virtual std::string getName() {
		return SmafeTimbralExtractor::EXTRACTORNAME;
	}

	virtual void getCapabilities(SmafeFVType_Ptr_map &fvts) {
        for(std::vector<std::string>::iterator iter = vFvTypes.begin(); iter < SmafeExtractor::vFvTypes.end(); ++iter) {
           addCapability(fvts, *iter, CLASS_ID_NUMERIC);
        } // end of iterator
	}



	virtual void setFVTProperties(SmafeFVType_Ptr_map &fvts, const Smafeopt opt) {
		std::string fvt_name;

		/*
		fvt_name = FVTYPE_NAME_ZEROCROSSINGRATE;
		fvts[fvt_name].get()->setProperties(1, 1, opt.serialize(fvt_name));

		fvt_name = FVTYPE_NAME_RMS;
		fvts[fvt_name].get()->setProperties(1, 1, opt.serialize(fvt_name));

		fvt_name = std::string(FVTYPE_NAME_SPECTRALCENTROID);
		fvts[fvt_name].get()->setProperties(1, 1, opt.serialize(fvt_name) );

		fvt_name = std::string(FVTYPE_NAME_SPECTRALSPREAD);
		fvts[fvt_name].get()->setProperties(1, 1, opt.serialize(fvt_name) );

		fvt_name = std::string(FVTYPE_NAME_SPECTRALFLUX);
		fvts[fvt_name].get()->setProperties(1, 1, opt.serialize(fvt_name) );

		fvt_name = std::string(FVTYPE_NAME_SPECTRALROLLOFF);
		fvts[fvt_name].get()->setProperties(1, 1, opt.serialize(fvt_name) );
		 */


		fvt_name = std::string(SmafeTimbralExtractor::FVTYPE_NAME_TIMEDOMAINFEATURES);
		fvts[fvt_name].get()->setProperties(COLS_TIMEDOMAIN, SSD_LEN, this->serializeConfig(fvt_name));

		fvt_name = std::string(SmafeTimbralExtractor::FVTYPE_NAME_SPECTRALFEATURES);
		fvts[fvt_name].get()->setProperties(COLS_SPECTRAL, SSD_LEN, this->serializeConfig(fvt_name));

	}

	virtual std::string serializeConfig(std::string type) {
		// disregard type
		return "(none)";
	}

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

			// fft size is set here: (idempotent function, has probably already been called)
			opt->setSongDependentOpts(audiodata->iSamplerate);


			const int LEN = opt->iFFTSize;
			const int NUMFRAMES = audiodata->ulNumSamples / opt->iFFTSize;

			// check for at least one frame
			if (NUMFRAMES == 0)
				throw std::string("File too short (0 frames)");

			double* wav; // pointer to correct offset
			//tTimbralFeatures* feat; // feature record for current frame
			int iFFTOrder = (int)floor(log(double(opt->iFFTSize)) / log(double(2))); // log_2(fftsize)

			// Array of pointers to features for each segment
			tTimbralFeatures4Segs* feats = new tTimbralFeatures4Segs();
			//feats->features = new tTimbralFeatures[NUMFRAMES];
			feats->iNumSegs = NUMFRAMES;
			feats->aiOffsets = new int[NUMFRAMES];
			feats->iLenSeg = LEN;
			// allocate mem for individual features
			feats->zcr = new double[NUMFRAMES];
			feats->rms = new double[NUMFRAMES];
			feats->spc = new double[NUMFRAMES];
			feats->sps = new double[NUMFRAMES];
			feats->spf = new double[NUMFRAMES];
			feats->spr = new double[NUMFRAMES];

			Ipp64f* win = new Ipp64f[LEN];
			SmafeLogger::smlog->log_alloc(NUMFRAMES*LEN*sizeof(Ipp64f));
			Ipp64f* powspec_matrix = new Ipp64f[NUMFRAMES*LEN];
			IppsFFTSpec_R_64f* ctx;


			double *powspec = NULL;
			double *powspec_prev = NULL;
			// interim numbers that can be useful for more than one feature
			double powspecframe_sum = -1;


			// Create hann window (one time)
			ippsSet_64f( 1, win, LEN );
			ippsWinHann_64f_I( win, LEN ); // -- 2009ok

			// change hint for IPP algorithm: fast or accurate
			//ippsFFTInitAlloc_R_64f(&ctx, iFFTOrder, IPP_FFT_DIV_FWD_BY_N, ippAlgHintFast);
			//ippsFFTInitAlloc_R_64f(&ctx, iFFTOrder, IPP_FFT_DIV_FWD_BY_N, ippAlgHintAccurate);
			ippsFFTInitAlloc_R_64f(&ctx, iFFTOrder, IPP_FFT_NODIV_BY_ANY, ippAlgHintAccurate);


			// iterate through frames
			int numSeg, countSeg;
			for (countSeg = 0, numSeg = 0; countSeg < feats->iNumSegs; countSeg++, numSeg+=1) {
				SMAFELOG_FUNC(SMAFELOG_DEBUG, "Processing frame "+stringify(numSeg)+" (samples "+
						stringify(numSeg*LEN)+" to "+stringify(numSeg*LEN+LEN-1)+")...");
				feats->aiOffsets[countSeg] = numSeg*LEN;

				// set pointer to current offset
				wav = buf+numSeg*LEN;
				//feat = &(feats->features[countSeg]);
				//powspec = feats->features[countSeg].ps;
				powspec = &(powspec_matrix[INDEX2DARRAY(0, countSeg, LEN)]);
				// also set pointer to previous one if possible
				if (countSeg > 0) {
					//powspec_prev = feats->features[countSeg-1].ps;
					powspec_prev = &(powspec_matrix[INDEX2DARRAY(0, countSeg-1, LEN)]);
				} else {
					powspec_prev = NULL;
				}



				// --------------- Time domain features
				if (opt->mapExtractForSong[SmafeTimbralExtractor::FVTYPE_NAME_TIMEDOMAINFEATURES]) {

					//if (opt->mapExtractForSong[FVTYPE_NAME_ZEROCROSSINGRATE]) {
					long z = 0;
					for (int i = 0; i < LEN - 1; i++) {
						if ((wav[i] > 0.0 && wav[i + 1] < 0.0) ||
								(wav[i] < 0.0 && wav[i + 1] > 0.0) ||
								(wav[i] == 0.0 && wav[i + 1] != 0.0))
							z++;
					}
					feats->zcr[countSeg] = double(z) / double(LEN);
					//}
					//if (opt->mapExtractForSong[FVTYPE_NAME_RMS]) {
					double sum = 0;
					for (int i = 0; i < LEN - 1; i++) {
						sum += wav[i]*wav[i];
					}
					feats->rms[countSeg] = sqrt(sum / double(LEN));
					//}
				} // time domain






				// --------------- Freq domain features
				if (opt->mapExtractForSong[SmafeTimbralExtractor::FVTYPE_NAME_SPECTRALFEATURES]) {

					// power spectrum
					SmafeExtractorUtil::computePowerspectrum(LEN,
							wav,
							//					&(powspec[INDEX2DARRAY(0, numSeg, LEN)]),
							powspec,
							win,
							ctx);

					//writeArrayAsCode(std::cout, std::string("powspec") + stringify(numSeg), (double*)powspec, LEN);

					// sets powspecframe_sum
					//				if (opt->mapExtractForSong[FVTYPE_NAME_SPECTRALCENTROID]) {
					double weighted_total = 0.0;
					powspecframe_sum = 0.0;

					for (int i = 0; i < LEN/2+1; i++) {
						weighted_total += i * powspec[i];
						powspecframe_sum += powspec[i];
					}

					if (powspecframe_sum != 0.0 && !my_isnan(weighted_total / powspecframe_sum)) {
						feats->spc[countSeg] = weighted_total / powspecframe_sum / (LEN/2+1);
					} else {
						feats->spc[countSeg] = 0.0;
					}
					//				} // spectral centroid


					//				if (opt->mapExtractForSong[FVTYPE_NAME_SPECTRALSPREAD]) {
					ippsStdDev_64f(powspec, LEN/2+1, &(feats->sps[countSeg]));
					//				} // spectral spread


					// Flux: calculated from frame #1 to last (#0: defined as 0)
					//				if (opt->mapExtractForSong[FVTYPE_NAME_SPECTRALFLUX]) {
					// for frame #0: 0
					if (powspec_prev == NULL) {
						feats->spf[countSeg] =0;
					} else {
						// we have a previous frame
						double sum_square_diff  = 0.0;
						double diff = 0.0;

						for (int i = 0; i < LEN/2+1; i++) {
							diff = powspec[i] - powspec_prev[i];
							sum_square_diff += diff * diff;
						}
						feats->spf[countSeg] = sum_square_diff;
					}
					//				} // spectral flux


					// may use powspecframe_sum
					//				if (opt->mapExtractForSong[FVTYPE_NAME_SPECTRALROLLOFF]) {
					// calc sum if it does not already exist
					if (powspecframe_sum < 0) {
						for (int i = 0; i < LEN/2+1; i++) {
							powspecframe_sum += powspec[i];
						}
					}

					double ro_threshold  = powspecframe_sum * SmafeTimbralExtractor::ROLLOFFPOINT;
					double sum = 0.0;
					double dVal = 1.0;

					for (int i = 0; i < LEN/2+1; i++) {
						sum += powspec[i];
						if (sum > ro_threshold) {
							dVal = double(i) / double(LEN/2+1);
							// break loop
							i = LEN;
						}
					}
					feats->spr[countSeg] = dVal;
					//				} // spectral rolloffpoint

				} // spectral features

			} // iterate through frames



			// ------------ convert to SNFVs
			// ie., aggregate
			SmafeNumericFeatureVector_Ptr fvp;
			/*
			 * used to calculate the mean
			 *
			 *
		// used to store the sum
		tTimbralFeatures feat4mean = {0};
		Ipp64f* buf2 = new Ipp64f[NUMFRAMES];

		// go through segments
		for (int s=0; s<NUMFRAMES; s++) {
			feat4mean.zcr += feats->features[s].zcr;
			feat4mean.rms += feats->features[s].rms;
			feat4mean.spc += feats->features[s].spc;
			feat4mean.sps += feats->features[s].sps;
			feat4mean.spf += feats->features[s].spf;
			feat4mean.spr += feats->features[s].spr;
		}

		if (opt->mapExtractForSong[FVTYPE_NAME_ZEROCROSSINGRATE]) {
			double tmp_array[1] = {feat4mean.zcr / double(NUMFRAMES)};

			SmafeNumericFeatureVector *snfv = new SmafeNumericFeatureVector(tmp_array, (*fvts)[SmafeTimbralExtractor::FVTYPE_NAME_ZEROCROSSINGRATE].get(), true);
			snfv->file_uri = opt->sFilename;
			fvp.reset(snfv);
			fvs.push_back(fvp);
		}
		if (opt->mapExtractForSong[FVTYPE_NAME_RMS]) {
			double tmp_array[1] = {feat4mean.rms / double(NUMFRAMES)};

			SmafeNumericFeatureVector *snfv = new SmafeNumericFeatureVector(tmp_array, (*fvts)[SmafeTimbralExtractor::FVTYPE_NAME_RMS].get(), true);
			snfv->file_uri = opt->sFilename;
			fvp.reset(snfv);
			fvs.push_back(fvp);
		}
		if (opt->mapExtractForSong[FVTYPE_NAME_SPECTRALCENTROID]) {
			double tmp_array[1] = {feat4mean.spc / double(NUMFRAMES)};

			SmafeNumericFeatureVector *snfv = new SmafeNumericFeatureVector(tmp_array, (*fvts)[SmafeTimbralExtractor::FVTYPE_NAME_SPECTRALCENTROID].get(), true);
			snfv->file_uri = opt->sFilename;
			fvp.reset(snfv);
			fvs.push_back(fvp);
		}
		if (opt->mapExtractForSong[FVTYPE_NAME_SPECTRALSPREAD]) {
			double tmp_array[1] = {feat4mean.sps / double(NUMFRAMES)};

			SmafeNumericFeatureVector *snfv = new SmafeNumericFeatureVector(tmp_array, (*fvts)[SmafeTimbralExtractor::FVTYPE_NAME_SPECTRALSPREAD].get(), true);
			snfv->file_uri = opt->sFilename;
			fvp.reset(snfv);
			fvs.push_back(fvp);
		}
		if (opt->mapExtractForSong[FVTYPE_NAME_SPECTRALFLUX]) {
			double tmp_array[1] = {feat4mean.spf / double(NUMFRAMES)};

			SmafeNumericFeatureVector *snfv = new SmafeNumericFeatureVector(tmp_array, (*fvts)[SmafeTimbralExtractor::FVTYPE_NAME_SPECTRALFLUX].get(), true);
			snfv->file_uri = opt->sFilename;
			fvp.reset(snfv);
			fvs.push_back(fvp);
		}
		if (opt->mapExtractForSong[FVTYPE_NAME_SPECTRALROLLOFF]) {
			double tmp_array[1] = {feat4mean.spr / double(NUMFRAMES)};

			SmafeNumericFeatureVector *snfv = new SmafeNumericFeatureVector(tmp_array, (*fvts)[SmafeTimbralExtractor::FVTYPE_NAME_SPECTRALROLLOFF].get(), true);
			snfv->file_uri = opt->sFilename;
			fvp.reset(snfv);
			fvs.push_back(fvp);
		}
			 */


			// ----------- statistical descs
			if (opt->mapExtractForSong[SmafeTimbralExtractor::FVTYPE_NAME_TIMEDOMAINFEATURES]) {
				double* adDest1; // allocated in method
				double* adDest2; // allocated in method
				SmafeFVType* fvt_cur = (*fvts)[SmafeTimbralExtractor::FVTYPE_NAME_TIMEDOMAINFEATURES].get();
				double* adDestMerged = new double[fvt_cur->dimension_x * fvt_cur->dimension_y];
				//double* adDestMerged = new double[COLS_TIMEDOMAIN * SSD_LEN];

				SmafeExtractorUtil::getSSD(feats->zcr, 1, NUMFRAMES, adDest1);
				SmafeExtractorUtil::getSSD(feats->rms, 1, NUMFRAMES, adDest2);

				// copy to one array
				memcpy(adDestMerged, adDest1, SSD_LEN*sizeof(double));
				memcpy(&(adDestMerged[SSD_LEN]), adDest2, SSD_LEN*sizeof(double));

				SmafeNumericFeatureVector *snfv = new SmafeNumericFeatureVector(adDestMerged, fvt_cur, true);
				snfv->file_uri = sFilename;
				fvp.reset(snfv);
				fvs.push_back(fvp);

				delete[] adDest1;
				delete[] adDest2;
				delete[] adDestMerged;
			}
			if (opt->mapExtractForSong[SmafeTimbralExtractor::FVTYPE_NAME_SPECTRALFEATURES]) {
				double* adDest1; // allocated in method
				SmafeFVType* fvt_cur = (*fvts)[SmafeTimbralExtractor::FVTYPE_NAME_SPECTRALFEATURES].get();
				double* adDestMerged = new double[fvt_cur->dimension_x * fvt_cur->dimension_y];

				SmafeExtractorUtil::getSSD(feats->spc, 1, NUMFRAMES, adDest1);
				memcpy(adDestMerged, adDest1, SSD_LEN*sizeof(double));
				delete[] adDest1;

				SmafeExtractorUtil::getSSD(feats->sps, 1, NUMFRAMES, adDest1);
				memcpy(&(adDestMerged[SSD_LEN]), adDest1, SSD_LEN*sizeof(double));
				delete[] adDest1;

				SmafeExtractorUtil::getSSD(feats->spf, 1, NUMFRAMES, adDest1);
				memcpy(&(adDestMerged[SSD_LEN*2]), adDest1, SSD_LEN*sizeof(double));
				delete[] adDest1;

				SmafeExtractorUtil::getSSD(feats->spr, 1, NUMFRAMES, adDest1);
				memcpy(&(adDestMerged[SSD_LEN*3]), adDest1, SSD_LEN*sizeof(double));
				delete[] adDest1;

				SmafeNumericFeatureVector *snfv = new SmafeNumericFeatureVector(adDestMerged, fvt_cur, true);
				snfv->file_uri = sFilename;
				fvp.reset(snfv);
				fvs.push_back(fvp);

				delete[] adDestMerged;
			}

			/*
		//debug: mean of powspec
		double* psavg = new double[LEN];


		for (int l=0; l<LEN; l++) {
			sum=0.0;
			// go through segments
			for (int s=0; s<NUMFRAMES; s++) {
				sum += feats->features[s].ps[l];
			}
			psavg[l] = sum / double(NUMFRAMES);
		}
		writeArrayAsCode(std::cout, std::string("powspec_avg"), psavg, LEN);
			 */



			//writeArrayAsCode(std::cout, std::string("spf"), feats->spr, NUMFRAMES);
			//writeArrayAsCode(std::cout, std::string("spf"), feats->spf, NUMFRAMES);


			// release memory
			ippsFFTFree_R_64f(ctx);
			delete[] win;
			delete[] powspec_matrix;

			delete[] feats->zcr;
			delete[] feats->rms;
			delete[] feats->spc;
			delete[] feats->sps;
			delete[] feats->spf;
			delete[] feats->spr;
			delete[] feats->aiOffsets;
			delete feats;

			return true;
		} else {
			// nothing to do for this extractor...
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Nothing to do for SmafeTimbralExtractor...");
			return false;
		}
	}


private:


	// --- properties

};

