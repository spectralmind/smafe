///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafeRPExtractor.cpp
//
// Feature extraction from audio data vector
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
// NOTE:
// Comments in the form of // MATLAB-REFIMP:893
// refer to the corresponding line in the Matlab reference implementation v0.63

#include "smafeRPExtractor.h"



// ---- Const static members inits
// initialization of static member ARRAY constants (arrays can't be used in init list, and also not at declaration time :-(
const double smafeRPExtractor::BARK_LIMITS[] = {100, 200, 300, 400, 510, 630, 770, 920, 1080, 1270, 1480, 1720, 2000, /* = [12] */
		2320, 2700, 3150, 3700, 4400, 5300, 6400, 7700, 9500, 12000, 15500, 22050};
const double smafeRPExtractor::BLUR_FILTER[] = {0.05, 0.1, 0.25, 0.5, 1, 0.5, 0.25, 0.1, 0.05};

const std::string smafeRPExtractor::FVTYPE_NAME_RP = "RP";
const std::string smafeRPExtractor::FVTYPE_NAME_RH  = "RH";
const std::string smafeRPExtractor::FVTYPE_NAME_SSD  = "SSD";
const std::string smafeRPExtractor::FVTYPE_NAME_TRP = "TRP";
const std::string smafeRPExtractor::FVTYPE_NAME_TRH = "TRH";
const std::string smafeRPExtractor::FVTYPE_NAME_TSSD = "TSSD";
const std::string smafeRPExtractor::EXTRACTORNAME = "RPExtractorEx";


smafeRPExtractor::smafeRPExtractor(void): blur1(NULL), blur2(NULL)
{
	vFvTypes.push_back(smafeRPExtractor::FVTYPE_NAME_RP);
	vFvTypes.push_back(smafeRPExtractor::FVTYPE_NAME_RH);
	vFvTypes.push_back(smafeRPExtractor::FVTYPE_NAME_SSD);
	vFvTypes.push_back(smafeRPExtractor::FVTYPE_NAME_TRP);
	vFvTypes.push_back(smafeRPExtractor::FVTYPE_NAME_TRH);
	vFvTypes.push_back(smafeRPExtractor::FVTYPE_NAME_TSSD);

	// private vars
	blurMatricesInit1 = blurMatricesInit2 = false;
}



smafeRPExtractor::~smafeRPExtractor(void)
{
	if (blurMatricesInit1) {
		delete[] blur1;
	}
	if (blurMatricesInit2) {
		delete[] blur2;
	}
}



void smafeRPExtractor::getCapabilities(SmafeFVType_Ptr_map &fvts) {
	for(std::vector<std::string>::iterator iter = vFvTypes.begin(); iter < SmafeExtractor::vFvTypes.end(); ++iter) {
		addCapability(fvts, *iter, CLASS_ID_NUMERIC);
	} // end of iterator

}

void smafeRPExtractor::setFVTProperties(SmafeFVType_Ptr_map &fvts, const Smafeopt opt) {
	// check validity of options
	// (out of range)

	if (opt.uiStepwidth < 1) throw std::string("uiStepwidth must be >= 1");
	if (opt.uiSkipin < 0) throw std::string("uiSkipin must be >= 0");
	if (opt.uiSkipout < 0) throw std::string("uiSkipout must be >= 0");
	if (opt.uiStepwidth > 1000) throw std::string("uiStepwidth must be <= 1000");
	if (opt.uiSkipin > 1000) throw std::string("uiSkipin must be <= 1000");
	if (opt.uiSkipout > 1000) throw std::string("uiSkipout must be <= 1000");


	std::string fvt_name;

	fvt_name = smafeRPExtractor::FVTYPE_NAME_RP;
	fvts[fvt_name].get()->setProperties(opt.shNumBarkBands, opt.uiModAmplLimit, this->serializeConfig(fvt_name));

	fvt_name = smafeRPExtractor::FVTYPE_NAME_RH;
	fvts[fvt_name].get()->setProperties(opt.rh_len, 1, this->serializeConfig(fvt_name));

	fvt_name = smafeRPExtractor::FVTYPE_NAME_SSD;
	fvts[fvt_name].get()->setProperties(SSD_LEN, opt.shNumBarkBands, this->serializeConfig(fvt_name));

	fvt_name = smafeRPExtractor::FVTYPE_NAME_TRP;
	fvts[fvt_name].get()->setProperties(SSD_LEN, opt.uiModAmplLimit * opt.shNumBarkBands, this->serializeConfig(fvt_name));

	fvt_name = smafeRPExtractor::FVTYPE_NAME_TRH;
	fvts[fvt_name].get()->setProperties(SSD_LEN, opt.rh_len * 1, this->serializeConfig(fvt_name));

	fvt_name = smafeRPExtractor::FVTYPE_NAME_TSSD;
	fvts[fvt_name].get()->setProperties(SSD_LEN, SSD_LEN * opt.shNumBarkBands, this->serializeConfig(fvt_name));
}



// extracts features of a 6 sec segment
// !Note that data types double and Ipp64f are equivalent!
void smafeRPExtractor::getFeaturesForSegment(double* wav, int iBufOffset, int iBufLen, Smafeopt* opt, tRPFeatures* feat) {

	// set all members of struct to NULL (in case they never get filled, a delete[] does not make problems
	feat->rp = NULL;
	feat->rh = NULL;
	feat->ssd = NULL;
	feat->trp = NULL;
	feat->trh = NULL;
	feat->tssd = NULL;

	//adjust hearing threshold -- 2009OK
	// MATLAB-REFIMP:972
	for (int i=iBufOffset; i< iBufOffset+iBufLen; i++) {
		wav[i] = 0.0875*wav[i]*32768; // *2^15
	}

	// [S1]
	// of iterations with 50 % overlap
	int iFFTOrder = (int)floor(log(double(opt->iFFTSize)) / log(double(2))); // log_2(fftsize)
	int iOffset = 0;
	const int LEN = opt->iFFTSize;
	const int NUMFRAMES = iBufLen / opt->iFFTSize * 2 -1;

	IppStatus status;
	Ipp64f* win = new Ipp64f[LEN];
	Ipp64f* x = new Ipp64f[LEN];
	Ipp64fc* xc = new Ipp64fc[LEN];
	SmafeLogger::smlog->log_alloc(NUMFRAMES*LEN*sizeof(Ipp64f));
	Ipp64f* specgram = new Ipp64f[NUMFRAMES*LEN];
	SmafeLogger::smlog->log_alloc(NUMFRAMES*opt->shNumBarkBands*sizeof(double));
	double* barkSpec = new double[NUMFRAMES*opt->shNumBarkBands];
	IppsFFTSpec_R_64f* ctx;

	// Create hann window (one time)
	ippsSet_64f( 1, win, LEN );
	ippsWinHann_64f_I( win, LEN ); // -- 2009ok

	// change hint for IPP algorithm: fast or accurate
	//ippsFFTInitAlloc_R_64f(&ctx, iFFTOrder, IPP_FFT_DIV_FWD_BY_N, ippAlgHintFast);
	//ippsFFTInitAlloc_R_64f(&ctx, iFFTOrder, IPP_FFT_DIV_FWD_BY_N, ippAlgHintAccurate);
	ippsFFTInitAlloc_R_64f(&ctx, iFFTOrder, IPP_FFT_NODIV_BY_ANY, ippAlgHintAccurate);

	// for each frame with 50 % overlap
	for (int i=0; i<NUMFRAMES; i++, iOffset+=opt->iFFTSize/2) {
		// power spectrum
		SmafeExtractorUtil::computePowerspectrum(LEN,
				&(wav[iBufOffset+iOffset]),
				&(specgram[INDEX2DARRAY(0, i, LEN)]),
				win,
				ctx);

		//writeArrayAsCode(std::cout, std::string("specgram") + stringify(i), (double*)&(specgram[INDEX2DARRAY(0, i, LEN)]), 100);

		// MATLAB-REFIMP:980 / 1076
		// Matlab's "matrix" ... barkSpec
		int iBarkBand = 0;  // bark band index


		barkSpec[INDEX2DARRAY(iBarkBand, i, opt->shNumBarkBands)] = 0;
		//f ... frequency band, curFreq ... frequency represented by band f
		// iBarkBand must be lower than number of Bark Bands
		for (int f = 0; (f < LEN/2+1) && (iBarkBand < opt->iBarkBandsUsed); f++)
		{
			double curFreq = f * BARK_BIN_SIZE;
			while (curFreq > BARK_LIMITS[iBarkBand] && iBarkBand < opt->shNumBarkBands) {
				iBarkBand++;
				if (iBarkBand <  opt->shNumBarkBands)
					barkSpec[INDEX2DARRAY(iBarkBand, i, opt->shNumBarkBands)]=0;
			}
			if (iBarkBand <  opt->shNumBarkBands) {
				if (iBarkBand < opt->iBarkBandsUsed) {
					// use spectral information
					barkSpec[INDEX2DARRAY(iBarkBand, i, opt->shNumBarkBands)] += specgram[INDEX2DARRAY(f, i, LEN)];
				}
				else {
					// pad with 0
					barkSpec[INDEX2DARRAY(iBarkBand, i, opt->shNumBarkBands)] = 0;
				}
			}
		}
		for (int b = opt->iBarkBandsUsed; b < opt->shNumBarkBands; b++)
			barkSpec[INDEX2DARRAY(b, i, opt->shNumBarkBands)] = 0;


	} // // for each frame with 50 % overlap

	// normalization of "fft energy", according to Gabriel's experiments
	/*
		 %[N] Normalization
		function matrix = normal(matrix, wavsegment);
		  facdb=10^9;  %90 dB
		    matrix=matrix./max(matrix(:));
		    matrix=matrix*facdb;
	 */

	if (opt->bNormalizeFFTEnergy) {
		double maxval = 0;
		const double FACDB = 1e+09; // 10^9;

		//writeArrayAsCode(std::cout, std::string("barkSpec"), barkSpec, NUMFRAMES*opt->shNumBarkBands);

		// find max value in matrix
		for (int i=0; i < NUMFRAMES * opt->shNumBarkBands; i++) {
			//std::cout << barkSpec[i] << '\n';
			if (barkSpec[i] > maxval) maxval = barkSpec[i];
		}
		//std::cout << maxval << '\n'; // same as in Matlab reference
		if (maxval > 0) {
			// divide by max value and multiply with db factor
			for (int i=0; i < NUMFRAMES * opt->shNumBarkBands; i++) {
				//std::cout << barkSpec[i] << " " << barkSpec[i] / maxval << " " << barkSpec[i] * FACDB << " " << (barkSpec[i] / maxval) * FACDB << " " << FACDB << std::endl;
				barkSpec[i] = barkSpec[i] / maxval * FACDB;
			}
		}
		//writeArrayAsCode(std::cout, std::string("barkSpec"), barkSpec, 100);

	}


	// for each frame with 50 % overlap
	for (int i=0; i<NUMFRAMES; i++, iOffset+=opt->iFFTSize/2) {

		// SKIP spectral masking for now

		// transform db -ok
		if (opt->bTransformDecibel) {
			// MATLAB-REFIMP:1095
			double dtmp;
			for (int b = 0; b < opt->shNumBarkBands; b++)
			{
				dtmp = barkSpec[INDEX2DARRAY(b, i, opt->shNumBarkBands)];
				if (dtmp < 1)
					dtmp = 1.0;
				barkSpec[INDEX2DARRAY(b, i, opt->shNumBarkBands)] = 10.0 * log10(dtmp);
			}
		}

		// SKIP transform phon for now


		// transform sone -ok
		if (opt->bTransformSone) {
			// MATLAB-REFIMP:1149
			double dtmp;
			for (int b = 0; b < opt->shNumBarkBands; b++)
			{
				dtmp = barkSpec[INDEX2DARRAY(b, i, opt->shNumBarkBands)];
				if (dtmp >= 40)
					barkSpec[INDEX2DARRAY(b, i, opt->shNumBarkBands)] =
							pow(2.0, (dtmp-40.0)/10.0);
				else
					barkSpec[INDEX2DARRAY(b, i, opt->shNumBarkBands)] =
							pow(dtmp/40.0, 2.642);
			}
		}



	} // for each frame with 50 % overlap
	// release fft spec structure
	ippsFFTFree_R_64f(ctx);	


	//cout << "featurefor segment. barkSpec[0] = " << barkSpec[0] << endl;



	// debug: set barkspec to specified value for testing

	//for (int j = 0; j < opt->shNumBarkBands * NUMFRAMES; j++)
	//barkSpec[j] = j;



	// -- SSD -----------------------------------------------
	// MATLAB-REFIMP:1004/1191
	if (bExtractSSD || bExtractTSSD) {
		SmafeExtractorUtil::getSSD(barkSpec, opt->shNumBarkBands, NUMFRAMES, feat->ssd);
	} // if (opt->bExtractSSD || opt->bExtractTSSD)

	if (bExtractRP  || bExtractRH || bExtractTRH || bExtractTRP) {
		// MATLAB-REFIMP:1020/1157

		// buffer for complex fft result
		SmafeLogger::smlog->log_alloc(opt->iFFTSize_rp * opt->shNumBarkBands*sizeof(Ipp64fc));
		Ipp64fc* rpc = new Ipp64fc[opt->iFFTSize_rp * opt->shNumBarkBands];
		// init and alloc fft spec structure (IPP)
		// change hint for IPP algorithm: fast or accurate
		//ippsFFTInitAlloc_R_64f(&ctx, opt->iFFTOrder_rp, IPP_FFT_NODIV_BY_ANY, ippAlgHintFast);
		ippsFFTInitAlloc_R_64f(&ctx, opt->iFFTOrder_rp, IPP_FFT_NODIV_BY_ANY, ippAlgHintAccurate);

		// reserve memory to build barkspec's frame values of one band
		// in barkspec, the data is ordered by band, then frame
		// now, we need all frames of one band in one chunk
		Ipp64f* bandchunk = new Ipp64f[opt->iFFTSize_rp];
		//Ipp64f* fft_packed = new Ipp64f[opt->iFFTSize_rp];
		Ipp64f* fft_packed = new Ipp64f[opt->iFFTSize_rp];
		// for each band
		for (int b=0; b < opt->shNumBarkBands; b++) {
			// build chunk for this band
			for (int i=0; i < opt->iFFTSize_rp; i++) {
				// fill the first NUMFRAMES with the data from barkSpec
				// and pad the rest with 0
				if (i < NUMFRAMES)
					bandchunk[i] = barkSpec[INDEX2DARRAY(b, i, opt->shNumBarkBands)];
				else
					bandchunk[i] = 0;
			}


			status = ippsFFTFwd_RToPack_64f(bandchunk, fft_packed, ctx, 0); // no buffer used: 0
			if (ippStsNoErr != status)
				throw std::string(ippGetStatusString(status));
			// get complex vector from packed format
			int tmp1 = INDEX2DARRAY(0, b, opt->iFFTSize_rp);
			status = ippsConjPack_64fc(fft_packed, &rpc[tmp1], opt->iFFTSize_rp);
			if (ippStsNoErr != status)
				throw std::string(ippGetStatusString(status));
		} // for each band
		// free
		delete[] bandchunk;
		delete[] fft_packed;


		// MATLAB-REFIMP:1020
		// divide by 256
		Ipp64fc fctmp;
		fctmp.re = 256.0;
		fctmp.im = 0;
		ippsDivC_64fc_I(fctmp, rpc, opt->iFFTSize_rp * opt->shNumBarkBands);


		// create transposed rpc
		SmafeLogger::smlog->log_alloc(opt->iFFTSize_rp * opt->shNumBarkBands*sizeof(Ipp64fc));
		Ipp64fc* rpct = new Ipp64fc[opt->iFFTSize_rp * opt->shNumBarkBands];
		for (int b=0; b < opt->shNumBarkBands; b++) {
			for (int i=0; i < opt->iFFTSize_rp; i++) {
				int tmp1 = INDEX2DARRAY(b, i, opt->shNumBarkBands);
				int tmp2 = INDEX2DARRAY(i, b, opt->iFFTSize_rp);
				rpct[tmp1] = rpc[tmp2];
			}
		}


		// Determine which part of features to take
		// MATLAB-REFIMP:1014-1016
		// uiFeatFrom_rh is the offset used at feat->rp array
		// for determining which features we use for rh.
		unsigned int uiFeatFrom, uiFeatTo, uiFeatFrom_rh, rh_len;
		if (opt->bIncludeDC) {
			uiFeatFrom = 0;
			uiFeatTo = opt->uiModAmplLimit-1;
			uiFeatFrom_rh = 1; // feat->rp includes DC, never use DC for rh features
			rh_len = uiFeatTo-uiFeatFrom;
		} else {
			uiFeatFrom = 1;
			uiFeatTo = opt->uiModAmplLimit;
			uiFeatFrom_rh = 0; // feat->rp does NOT include DC, so start from 0
			rh_len = uiFeatTo-uiFeatFrom+1;
		}
		// also save in options
		opt->uiFeatFrom = uiFeatFrom;
		opt->uiFeatTo = uiFeatTo;
		opt->rh_len = rh_len;
		// Calculat RP also if only RH is required
		// becuase RH calculation uses RP
		// in this implementation -epei
		// same for TRP and TRH
		//if (opt->bExtractRH || opt->bExtractRP || opt->bExtractTRP || || opt->bExtractTRH) { // -ok
		// MATLAB-REFIMP:1024
		SmafeLogger::smlog->log_alloc((uiFeatTo-uiFeatFrom+1)*opt->shNumBarkBands*sizeof(double));
		feat->rp = new double[(uiFeatTo-uiFeatFrom+1)*opt->shNumBarkBands];
		ippsMagnitude_64fc(&(rpct[INDEX2DARRAY(0, uiFeatFrom, opt->shNumBarkBands)]),
				feat->rp, (uiFeatTo-uiFeatFrom+1)*opt->shNumBarkBands);
		// do not compute fluctuation strength weighting etc here, because
		// we need the "raw" rp value for rh computation
		//}

		if (bExtractRH || bExtractTRH) { // -ok
			// MATLAB-REFIMP:1038
			// always skip DC:
			//		Since we do not want to compute magnitude from rpc
			//		a second time we use feat->rp
			//		NOTE: That means that it is possible that feat->rh may only be based on
			//		opt->uiModAmplLimit - 1 values (in case that rp includes DC value)
			//		This shouldn't be a big deal anyway   -epei
			// sum over bark bands
			feat->rh = new double[rh_len];
			for (unsigned int i= 0; i < rh_len; i++) {
				feat->rh[i] = 0;
				for (int b= 0; b < opt->shNumBarkBands; b++) {
					// depending on whether feat->rp includes the DC component (index 0)
					// we have to start index 1 or not
					// uiFeatFrom_rh is set to correct value (0 or 1)
					feat->rh[i] += feat->rp[INDEX2DARRAY(b, uiFeatFrom_rh+i, opt->shNumBarkBands)];
				}
			}
		} // (opt->bExtractRH)
		if (opt->bFluctuationStrengthWeighting) { // -ok
			// security check
			// fluct_curve is only 256 elements long!
			if (uiFeatTo > FLUCT_CURVE_LEN-1)
				throw std::string("bFluctuationStrengthWeighting is set but uiFeatTo is larger then FLUCT_CURVE_LEN");
			// MATLAB-REFIMP:1043/1171
			for (int b= 0; b < opt->shNumBarkBands; b++) {
				for (unsigned int i= 0; i < (uiFeatTo-uiFeatFrom+1); i++) {
					feat->rp[INDEX2DARRAY(b, i, opt->shNumBarkBands)] *=
							fluct_curve[uiFeatFrom+i];
				}
			}
		}
		if (opt->bBlurring1 || opt->bBlurring2) { // -ok
			// MATLAB-REFIMP:1049/1182
			// calc gradients
			// start  at frame #1 (second frame)
			for (unsigned int i = 1; i < opt->uiModAmplLimit; i++) {
				for (int b = 0; b < opt->shNumBarkBands; b++) {
					feat->rp[INDEX2DARRAY(b, i-1, opt->shNumBarkBands)] =
							fabs(feat->rp[INDEX2DARRAY(b, i, opt->shNumBarkBands)] -
									feat->rp[INDEX2DARRAY(b, i-1, opt->shNumBarkBands)]);
				}
			}
			double* c = new double[opt->shNumBarkBands * opt->uiModAmplLimit];
			if (opt->bBlurring1) {
				// MATLAB-REFIMP:1187
				// 1st matrix multiplication blur1*rp
				// store interim result in temp buffer c
				// c is of size shNumBarkBands columns by uiModAmplLimit rows (eg 24x60)
				for( int i=0; i<opt->shNumBarkBands ; ++i ) {
					for( unsigned int j=0; j<opt->uiModAmplLimit; ++j ) {
						double sum = 0;
						for( int k=0; k< opt->shNumBarkBands; ++k )
							sum += getBlur1()[INDEX2DARRAY(k, i, opt->shNumBarkBands)] *
							feat->rp[INDEX2DARRAY(k, j, opt->shNumBarkBands)];
						c[INDEX2DARRAY(i, j, opt->shNumBarkBands)] = sum;
					}
				}
			} else {
				assert(opt->bBlurring2 == true);
				// no blur1, so copy feat->rp to c as the "interim result"
				memcpy(c, feat->rp, opt->shNumBarkBands * opt->uiModAmplLimit * sizeof(double));
			}
			if (opt->bBlurring2) {
				// 2nd matrix multiplication rp*blur2
				for( unsigned int i=0; i<opt->uiModAmplLimit ; ++i ) {
					for( int j=0; j<opt->shNumBarkBands; ++j ) {
						double sum = 0;
						for( unsigned int k=0; k< opt->uiModAmplLimit; ++k )
							sum += c[INDEX2DARRAY(j, k, opt->shNumBarkBands)] *
							getBlur2()[INDEX2DARRAY(i, k, opt->uiModAmplLimit)];
						feat->rp[INDEX2DARRAY(j, i, opt->shNumBarkBands)] = sum;
					}
				}
			} else {
				assert(opt->bBlurring1 == true);
				// no blur2, so copy c (result of first blur op back to rp)
				memcpy(feat->rp, c, opt->shNumBarkBands * opt->uiModAmplLimit * sizeof(double));
			}
			// free c
			delete[] c;
		} //  (opt->bBlurring1 || opt->bBlurring2)




		delete[] rpct;
		delete[] rpc;
		ippsFFTFree_R_64f(ctx);
	} //(opt->bExtractRP  || opt->bExtractRH || opt->bExtractTRH || opt->bExtractTRP)



	// free mem (maybe outsource to main routine?)

	delete[] win;
	delete[] x;
	delete[] xc;
	delete[] specgram;
	delete[] barkSpec;
}


void smafeRPExtractor::buildBlurMatrix(int size, double* &blur) {
	int ix;
	double colsum;
	blur = new double[size * size];
	// set matrix to 0s
	for (int i = 0; i < size * size; i++)
		blur[i] = 0;
	// iterate trough main diagonal
	for (int pos = 0; pos < size; pos++) {
		colsum = 0;
		// iterate through blur filter vector (the constant)
		for (int x = BLUR_FILTER_LEN*-1; x < BLUR_FILTER_LEN+1; x++) {
			// real x position in matrix
			ix = pos+x;
			// if not out of bounds
			if (ix >= 0 && ix < size) {
				// set value
				blur[INDEX2DARRAY(ix, pos, size)] = BLUR_FILTER[x+BLUR_FILTER_LEN];
				colsum += BLUR_FILTER[x+BLUR_FILTER_LEN];
			}
		} // for (int x = BLUR_FILTER_LEN*-1; x < BLUR_FILTER_LEN+1; x++)
		for (int x = 0; x < size; x++) {
			blur[INDEX2DARRAY(x, pos, size)] /= colsum;
		}
	} // for (int pos = 0; pos < opt->shNumBarkBands; pos++)
}


// is private!
void* smafeRPExtractor::getFeatures(double* buf, tAudioformat *audiodata, Smafeopt* opt,
		tRPFeatures* &songFeat, tRPFeatures4Segs* &feats)
{
	int iSegmentSize;
	int iNumSegmentsWholeSong;
	int iNumSegments;
	char tmplogmsg[100];

	unsigned int uiSkipin4song = opt->uiSkipin;
	unsigned int uiSkipout4song = opt->uiSkipout;
	unsigned int uiStepwidth4song = opt->uiStepwidth;



	// check max samples (so that not too much memory must be allocated
	// This is necessary, otherwise we get a "undefined reference error" in --enable-debug mode (?why?)
	long lTmp = SmafeExtractor::MAX_SAMPLES;
	if (audiodata->ulNumSamples > lTmp)
		throw std::string("Audio stream exceeds maximum number of samples allowed:  ") + stringify(audiodata->ulNumSamples) +
		std::string(" > ") + stringify(lTmp);

	// check validity of options
	// (out of range)

	if (uiStepwidth4song < 1) throw std::string("uiStepwidth must be >= 1");
	if (uiSkipin4song < 0) throw std::string("uiSkipin4song must be >= 0");
	if (uiSkipout4song < 0) throw std::string("uiSkipout4song must be >= 0");

	// segment_size should always be around 6 secs, fft_window_size should
	// always be around 23ms
	// MATLAB-REFIMP:848

	// fft size is set here: (idempotent function, has probably already been called)
	opt->setSongDependentOpts(audiodata->iSamplerate);

	// get number of bark bands really used (rest is padding with 0)
	opt->iBarkBandsUsed = 0;
	while ((opt->iBarkBandsUsed < sizeof(BARK_LIMITS) / sizeof(BARK_LIMITS[0])) &&
			(BARK_LIMITS[opt->iBarkBandsUsed] <=  audiodata->iSamplerate / 2)) {
		opt->iBarkBandsUsed++;
	}

	// also check if enough bark bands have been specified
	// if not issue warning
	if (opt->shNumBarkBands < opt->iBarkBandsUsed)
		//		SMAFELOG_FUNC(SMAFELOG_WARNING, std::string("Sample rate ") +
		// is debug message now (not to show "internal" to extern people
		SMAFELOG_FUNC(SMAFELOG_DEBUG, std::string("Sample rate ") +
				stringify(audiodata->iSamplerate) +
				std::string(" would allow ") +
				stringify(opt->iBarkBandsUsed) +
				std::string(" bark bands but only ") +
				stringify(opt->shNumBarkBands) +
				std::string(" have been been requested. Rest of data will be truncated."));

	if (audiodata->iSamplerate <= 11025) {
		iSegmentSize = 65536; // 2^16
	} else if (audiodata->iSamplerate <= 22050) {
		iSegmentSize = 131072; // 2^17;
	} else { // assume 44100
		iSegmentSize = 262144; // 2^18
	}
	// check if too many bark bounds
	int maxBB= sizeof(BARK_LIMITS) / sizeof(BARK_LIMITS[0]);
	if (opt->shNumBarkBands > maxBB)
		throw std::string("Maximum number of Bark bands is ") + stringify(maxBB);


	// MATLAB-REFIMP:1160
	// find efficient fft size (power of 2) for rp calculatoin
	int numframes = iSegmentSize / opt->iFFTSize * 2 -1;
	opt->iFFTOrder_rp = 1; // start with order 1 (2^1)
	// increase until appropriate order is found
	while (pow(2.0, opt->iFFTOrder_rp) < numframes) {opt->iFFTOrder_rp++;}
	opt->iFFTSize_rp = int(pow(2.0, opt->iFFTOrder_rp));


	// calculate fluctuation curve
	// MATLAB-REFIMP:859-863
	if (opt->bFluctuationStrengthWeighting) {
		double dModFreqRes = 1.0 / (double(iSegmentSize) / double(audiodata->iSamplerate));
		fluct_curve[0] = 0; // avoid div/0
		for (int i = 1; i < FLUCT_CURVE_LEN; i++) {
			fluct_curve[i] = 1.0 / (dModFreqRes*i/4.0 + 4.0/(dModFreqRes*i));
		}
	}
	// calculate blurring matrices
	// if not already done
	// MATLAB-REFIMP:866-871
	if (opt->bBlurring1 && getBlur1() == NULL) {
		double *b1;
		buildBlurMatrix(opt->shNumBarkBands, b1);
		setBlur1(b1);
	} // (opt->bBlurring1)
	if (opt->bBlurring2 && getBlur2() == NULL) {
		double *b2;
		buildBlurMatrix(opt->uiModAmplLimit, b2);
		setBlur2(b2);
	} // (opt->bBlurring2)


	// check validity of leadin/out and stepwith options
	iNumSegmentsWholeSong = audiodata->ulNumSamples / iSegmentSize;
	if (iNumSegmentsWholeSong - int(uiSkipin4song) - int(uiSkipout4song) < 1) {
		SMAFELOG_FUNC(SMAFELOG_WARNING, "skipin/skipout parameters lead to no segments. Setting them to 0, and stepwidth to 1 for this audio file.");
		uiSkipin4song = 0;
		uiSkipout4song = 0;
		uiStepwidth4song = 1;
	}

	// MATLAB-REFIMP:893
	iNumSegments = int(floor((floor(float((audiodata->ulNumSamples-(uiSkipin4song+uiSkipout4song)*iSegmentSize)) / float(iSegmentSize))-1)/float(uiStepwidth4song))+1);

	if (iNumSegments <= 0)
		throw std::string("Audio file is too short (0 segments)");

	sprintf(tmplogmsg, "# samples: %lu, # segments: %d",audiodata->ulNumSamples,  iNumSegmentsWholeSong);
	SMAFELOG_FUNC(SMAFELOG_DEBUG, std::string(tmplogmsg));
	sprintf(tmplogmsg, "# segments taken for feature extraction: %d", iNumSegments);
	SMAFELOG_FUNC(SMAFELOG_DEBUG, std::string(tmplogmsg));

	// Array of pointers to features for each segment
	feats = new tRPFeatures4Segs();
	feats->features = new tRPFeatures[iNumSegments];
	feats->iNumSegs = iNumSegments;
	feats->aiOffsets = new int[iNumSegments];
	feats->iLenSeg = iSegmentSize;

	// iterate through segments
	// MATLAB-REFIMP:900
	int numSeg, countSeg;
	// note: countSeg starts from 0, and is increased by 1
	// 		numSeg corresponds to "real" segment number , starting at the first after skipIn
	//				and being increased by stepwidth
	for (countSeg = 0, numSeg = uiSkipin4song; countSeg < iNumSegments; countSeg++, numSeg+=uiStepwidth4song) {
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "Processing segment "+stringify(numSeg)+" (samples "+
				stringify(numSeg*iSegmentSize)+" to "+stringify(numSeg*iSegmentSize+iSegmentSize-1)+")...");
		feats->aiOffsets[countSeg] = numSeg*iSegmentSize;
		(void) getFeaturesForSegment(buf, numSeg*iSegmentSize, iSegmentSize, opt, &(feats->features[countSeg]));
		// set tssd, trh and trp to NULL
		feats->features[countSeg].trp = NULL;
		feats->features[countSeg].trh = NULL;
		feats->features[countSeg].tssd = NULL;
	}

	// build summarization of all segments
	// MATLAB-REFIMP:944
	// A little c++ overhead: we have to find the median / mean for rp, rh and ssd
	// in Matlab that's very much more elegant.
	// Here, however, we have to prepare a buffer with relevant data and
	// calculate the mean (<- for each element of the final feature vector)
	// We use IPP's sort
	Ipp64f* buf2 = new Ipp64f[iNumSegments];
	double dVal;
	songFeat = new tRPFeatures();

	// start with RP		-ok
	if (bExtractRP || bExtractTRP) {
		int iRPBufLen = opt->shNumBarkBands * (opt->uiFeatTo-opt->uiFeatFrom+1);
		songFeat->rp = new double[iRPBufLen];
		// iterate linearly through 2D feature data
		for (int f=0; f < iRPBufLen; f++) {
			// go through segments
			for (int s=0; s<iNumSegments; s++) {
				// save feature datum from each segment in buffer
				dVal = feats->features[s].rp[f];
				if (!my_isnan(dVal))
					buf2[s] = dVal;
				else
					buf2[s] = 0.0;
			}
			// compute median over segment values and save in final feature struct
			songFeat->rp[f] = SmafeExtractorUtil::median(buf2, iNumSegments);
		}
	} // (opt->bExtractRP || opt->bExtractTRP)
	else
		songFeat->rp = NULL;

	// RH -ok
	if (bExtractRH || bExtractTRH) {
		int iRHBufLen = opt->rh_len;
		songFeat->rh = new double[iRHBufLen];
		// iterate through feature data
		for (int f=0; f < iRHBufLen; f++) {
			// go through segments
			for (int s=0; s<iNumSegments; s++) {
				// save feature datum from each segment in buffer
				dVal = feats->features[s].rh[f];
				if (!my_isnan(dVal))
					buf2[s] = dVal;
				else
					buf2[s] = 0.0;
			}
			// compute median over segment values and save in final feature struct
			songFeat->rh[f] = SmafeExtractorUtil::median(buf2, iNumSegments);
		}
	} //opt->bExtractRH || opt->bExtractTRH)
	else
		songFeat->rh = NULL;

	// SSD -ok
	if (bExtractSSD || bExtractTSSD) {
		songFeat->ssd = new double[SSD_LEN*opt->shNumBarkBands];
		// iterate linearly through 2D feature data
		for (int f=0; f < SSD_LEN*opt->shNumBarkBands; f++) {
			// use first element of buffer for mean  calculation
			buf2[0] = 0;
			// go through segments
			for (int s=0; s<iNumSegments; s++) {
				// save feature datum from each segment in buffer
				dVal = feats->features[s].ssd[f];
				if (!my_isnan(dVal))
					buf2[0] += dVal;
				else
					buf2[0] += 0.0; // do not change value
			}
			// compute MEAN (!) over segment values and save in final feature struct
			songFeat->ssd[f] = buf2[0] / double(iNumSegments);
		}
	} // (opt->bExtractSSD || opt->bExtractTSSD)
	else
		songFeat->ssd = NULL;

	// TRP
	if (bExtractTRP) {
		int iRPBufLen = opt->shNumBarkBands * (opt->uiFeatTo-opt->uiFeatFrom+1);
		SmafeLogger::smlog->log_alloc(iNumSegments * iRPBufLen*sizeof(double));
		double* adTmpRP = new double[iNumSegments * iRPBufLen];
		// iterate linearly through 2D feature data
		for (int f=0; f < iRPBufLen; f++) {
			// go through segments
			for (int s=0; s<iNumSegments; s++) {
				// fill temporary matrix
				dVal = feats->features[s].rp[f];
				if (!my_isnan(dVal))
					adTmpRP[INDEX2DARRAY(f, s, iRPBufLen)] = dVal;
				else
					adTmpRP[INDEX2DARRAY(f, s, iRPBufLen)] = 0.0;
			}
		}
		SmafeExtractorUtil::getSSD(adTmpRP, iRPBufLen, iNumSegments, songFeat->trp);
		delete[] adTmpRP;
	} else
		songFeat->trp = NULL;

	// TRH
	if (bExtractTRH) {
		int iRHBufLen = opt->rh_len;
		double* adTmpRH = new double[iNumSegments * iRHBufLen];
		// iterate linearly through 2D feature data
		for (int f=0; f < iRHBufLen; f++) {
			// go through segments
			for (int s=0; s<iNumSegments; s++) {
				// fill temporary matrix
				dVal = feats->features[s].rh[f];
				if (!my_isnan(dVal))
					adTmpRH[INDEX2DARRAY(f, s, iRHBufLen)] = dVal;
				else
					adTmpRH[INDEX2DARRAY(f, s, iRHBufLen)] = 0.0;
			}
		}
		SmafeExtractorUtil::getSSD(adTmpRH, iRHBufLen, iNumSegments, songFeat->trh);
		delete[] adTmpRH;
	} else
		songFeat->trh = NULL;

	// TSSD
	if (bExtractTSSD) {
		double* adTmpSSD = new double[iNumSegments * SSD_LEN*opt->shNumBarkBands];
		// iterate linearly through 2D feature data
		for (int f=0; f < SSD_LEN*opt->shNumBarkBands; f++) {
			// go through segments
			for (int s=0; s<iNumSegments; s++) {
				// fill temporary matrix (corresponds to Matlab's feat_ssd_list in line 950)
				dVal = feats->features[s].ssd[f];
				if (!my_isnan(dVal))
					adTmpSSD[INDEX2DARRAY(f, s, SSD_LEN*opt->shNumBarkBands)] = dVal;
				else
					adTmpSSD[INDEX2DARRAY(f, s, SSD_LEN*opt->shNumBarkBands)] = 0.0;
			}
		}
		SmafeExtractorUtil::getSSD(adTmpSSD, SSD_LEN*opt->shNumBarkBands, iNumSegments, songFeat->tssd);
		delete[] adTmpSSD;
	} else
		songFeat->tssd = NULL;



	// version
	songFeat->version = getVersion();



	// delete
	delete[] buf2;
	if (!opt->bReturnSegmentFeatures) {
		for (int i = 0; i < iNumSegments; i++) {
			delete[] feats->features[i].rp;
			delete[] feats->features[i].rh;
			delete[] feats->features[i].ssd;
		}
		delete[] feats->aiOffsets;
		delete[] feats->features;
		delete feats;
	}

	return NULL;
}




bool smafeRPExtractor::getFeatures(double* buf, tAudioformat *audiodata,  Smafeopt* so,  SmafeFVType_Ptr_map* fvts,
		std::vector< SmafeAbstractFeatureVector_Ptr > &fvs, std::vector< SmafeAbstractFeatureVector_Ptr > &segmentfvs, std::string sFilename) {
	tRPFeatures* feat;

	// set local bool flags
	bExtractRP = so->mapExtractForSong[FVTYPE_NAME_RP];
	bExtractRH = so->mapExtractForSong[FVTYPE_NAME_RH];
	bExtractSSD = so->mapExtractForSong[FVTYPE_NAME_SSD];
	bExtractTRP = so->mapExtractForSong[FVTYPE_NAME_TRP];
	bExtractTRH = so->mapExtractForSong[FVTYPE_NAME_TRH];
	bExtractTSSD = so->mapExtractForSong[FVTYPE_NAME_TSSD];

	// check if at least one feature set is to be extracted
	// check if this extractor is required (any features relevant for this extractor to be extracted?)
	bool bThisExtractorDoesSomething = false;

	for(std::vector<std::string>::iterator iter = vFvTypes.begin(); iter < SmafeExtractor::vFvTypes.end(); ++iter) {
		bThisExtractorDoesSomething = bThisExtractorDoesSomething || so->mapExtractForSong[*iter];
	} // end of iterator


	if (bThisExtractorDoesSomething) {


		tRPFeatures4Segs* feats;

		(void) getFeatures(buf, audiodata,
				so,
				feat,
				feats);



		//		throw std::string("test exception");

		//-- Transfer the data from the struct type to instances of our nice class
		SmafeNumericFeatureVector_Ptr fvp;

		// RP
		if (bExtractRP) {
			SmafeNumericFeatureVector *snfv = new SmafeNumericFeatureVector(feat->rp, (*fvts)[FVTYPE_NAME_RP].get(), true);
			snfv->file_uri = sFilename;

			fvp.reset(snfv);
			fvs.push_back(fvp);
		}
		// RH
		if (bExtractRH) {
			SmafeNumericFeatureVector *snfv = new SmafeNumericFeatureVector(feat->rh, (*fvts)[FVTYPE_NAME_RH].get(), true);
			snfv->file_uri = sFilename;

			fvp.reset(snfv);
			fvs.push_back(fvp);
		}

		// SSD
		if (bExtractSSD) {
			SmafeNumericFeatureVector *snfv = new SmafeNumericFeatureVector(feat->ssd, (*fvts)[FVTYPE_NAME_SSD].get(), true);
			snfv->file_uri = sFilename;

			fvp.reset(snfv);
			fvs.push_back(fvp);
		}
		// TRP
		if (bExtractTRP) {
			SmafeNumericFeatureVector *snfv = new SmafeNumericFeatureVector(feat->trp, (*fvts)[FVTYPE_NAME_TRP].get(), true);
			snfv->file_uri = sFilename;

			fvp.reset(snfv);
			fvs.push_back(fvp);
		}
		// TRH
		if (bExtractTRH) {
			SmafeNumericFeatureVector *snfv = new SmafeNumericFeatureVector(feat->trh, (*fvts)[FVTYPE_NAME_TRH].get(), true);
			snfv->file_uri = sFilename;

			fvp.reset(snfv);
			fvs.push_back(fvp);
		}
		// TSSD
		if (bExtractTSSD) {
			SmafeNumericFeatureVector *snfv = new SmafeNumericFeatureVector(feat->tssd, (*fvts)[FVTYPE_NAME_TSSD].get(), true);
			snfv->file_uri = sFilename;

			fvp.reset(snfv);
			fvs.push_back(fvp);
		}

		// Segments' features
		if (so->bReturnSegmentFeatures) {
			for (int i = 0; i < feats->iNumSegs; i++) {
				SMAFELOG_FUNC(SMAFELOG_DEBUG, std::string("Segment: ") + stringify(i));
				// RP
				if (bExtractRP) {
					SMAFELOG_FUNC(SMAFELOG_DEBUG, std::string("Writing segment feature vector record (RP)  to database..."));
					SmafeNumericFeatureVector *snfv = new SmafeNumericFeatureVector(feats->features[i].rp, (*fvts)[FVTYPE_NAME_RP].get(), true);
					snfv->file_uri = sFilename;
					snfv->lSegmentnr = i;
					snfv->lLength = feats->iLenSeg;
					snfv->lStartsample = feats->aiOffsets[i];
					fvp.reset(snfv);
					segmentfvs.push_back(fvp);
				}
				if (bExtractRH) {
					SMAFELOG_FUNC(SMAFELOG_DEBUG, std::string("Writing segment feature vector record (RH)  to database..."));
					SmafeNumericFeatureVector *snfv = new SmafeNumericFeatureVector(feats->features[i].rh, (*fvts)[FVTYPE_NAME_RH].get(), true);
					snfv->file_uri = sFilename;
					snfv->lSegmentnr = i;
					snfv->lLength = feats->iLenSeg;
					snfv->lStartsample = feats->aiOffsets[i];
					fvp.reset(snfv);
					segmentfvs.push_back(fvp);
				}
				if (bExtractSSD) {
					SMAFELOG_FUNC(SMAFELOG_DEBUG, std::string("Writing segment feature vector record (SSD)  to database..."));
					SmafeNumericFeatureVector *snfv = new SmafeNumericFeatureVector(feats->features[i].ssd, (*fvts)[FVTYPE_NAME_SSD].get(), true);
					snfv->file_uri = sFilename;
					snfv->lSegmentnr = i;
					snfv->lLength = feats->iLenSeg;
					snfv->lStartsample = feats->aiOffsets[i];
					fvp.reset(snfv);
					segmentfvs.push_back(fvp);
				}
			} // for (int i = 0; i < segfeat->iNumSegs; i++)

			// delete stuff
			for (int i = 0; i < feats->iNumSegs; i++) {
				delete[] feats->features[i].rp;
				delete[] feats->features[i].rh;
				delete[] feats->features[i].ssd;
			}
			delete[] feats->aiOffsets;
			delete[] feats->features;
			delete feats;
		}

		delete[] feat->rp;
		delete[] feat->rh;
		delete[] feat->ssd;
		delete[] feat->trp;
		delete[] feat->trh;
		delete[] feat->tssd;
		delete feat;

		return true;

	}  else { // if (bExtractRP || bExtractRH || bExtractSSD || bExtractTRP || bExtractTRH || bExtractTSSD)
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "No features to be calculated from SmafeRPExtractor");
		return false;
	}
}




int smafeRPExtractor::getVersion() {
	return RP_VERSION;
}



void smafeRPExtractor::setBlur1(double* b) {
	blurMatricesInit1 = true;
	blur1 = b;
}
void smafeRPExtractor::setBlur2(double* b) {
	blurMatricesInit2 = true;
	blur2 = b;
}
double* smafeRPExtractor::getBlur1() {
	return blur1;
}
double* smafeRPExtractor::getBlur2() {
	return blur2;
}
