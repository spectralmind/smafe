///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafeRPExtractor.h
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


/** Feature extraction from raw audio signal: RP, RH, SSD, TRP, TRH, TSSD */
#pragma once

#include "smafeutil.h"
#include "smafeopt.h"
#include "tAudioformat.h"
#include "smafeNumericFeatureVector.h"
#include "smafeExtractor.h"

#include <math.h>
#include "ipp.h"
#include <string>
#include <boost/shared_ptr.hpp>
#include <vector>







//------------------------------------------------
/** struct for features */
struct tRPFeatures{
	/** Statistical spectrum descriptor */
	double* ssd;
	/** Rhythm Patterns RP */
	double* rp;
	/** Rhythm Histogram */
	double* rh;
	/** temporal statistical features over RP features
					( not relevant for segment features but only for whole song features) */
	double* trp;
	/** temporal statistical features over RH features
					( not relevant for segment features but only for whole song features) */
	double* trh;
	/** temporal statistical features over SSD features
					( not relevant for segment features but only for whole song features) */
	double* tssd;
	/** version number */
	int version;
};

/** struct for segments' features (basically an array of tRPFeatures with array length */
struct tRPFeatures4Segs{
	/** number of segments */
	int iNumSegs;
	/** length of each segment */
	int iLenSeg;
	/** array of tRPFeatures structs */
	tRPFeatures* features;
	/** start (offset) of segments */
	int* aiOffsets;
};

//------------------------------------------------
//
/** Feature extraction from audio data vector */
class DLLEXPORT smafeRPExtractor : public SmafeExtractor
{
public:
	// --- consts
	// feature vector types offered by this extractor
	// !NOTE: contants are used as keys for maps as well as for filenames!
	static const std::string FVTYPE_NAME_RP; // "RP", set in definition list in constructor
	static const std::string FVTYPE_NAME_RH; //  = "RH";
	static const std::string FVTYPE_NAME_SSD; //  = "SSD";
	static const std::string FVTYPE_NAME_TRP; // = "TRP";
	static const std::string FVTYPE_NAME_TRH; // = "TRH";
	static const std::string FVTYPE_NAME_TSSD; // = "TSSD";
	static const std::string EXTRACTORNAME;



	// --- methods

	smafeRPExtractor(void);
	virtual ~smafeRPExtractor(void);

	virtual std::string getName() {
		return smafeRPExtractor::EXTRACTORNAME;
	}

	virtual void getCapabilities(SmafeFVType_Ptr_map &fvts);

	virtual void setFVTProperties(SmafeFVType_Ptr_map &fvts, const Smafeopt opt);


	virtual bool getFeatures(double* buf, tAudioformat *audiodata,  Smafeopt* opt,  SmafeFVType_Ptr_map* fvts,
				std::vector< SmafeAbstractFeatureVector_Ptr > &fvs, std::vector< SmafeAbstractFeatureVector_Ptr > &segmentfvs, std::string sFilename);



	/** Returns version of feature extraction algorithm.
		<p>The version number is, e.g., stored in the database
		@return version number as integer
	*/
	virtual int getVersion();

private:


	/** the version of the features. Must be integer!*/
	static const int RP_VERSION = 1;



	//------------------------------------------------
	// ------- constants


	/** length of precalculated fluctuation vector */
	static const int FLUCT_CURVE_LEN = 257;





	/** bark band limits */
	static const double BARK_LIMITS[]; // = {100, 200, 300, 400, 510, 630, 770, 920, 1080, 1270, 1480, 1720, 2000, /* = [12] */
			//2320, 2700, 3150, 3700, 4400, 5300, 6400, 7700, 9500, 12000, 15500, 22050};
	/** sample frequency / fftsize# */
	static const double BARK_BIN_SIZE = 43.06640625;
	/** length of half of blur kernel */
	static const int BLUR_FILTER_LEN = 4;
	/** blur kernel for filtering */
	static const double BLUR_FILTER[]; // = {0.05, 0.1, 0.25, 0.5, 1, 0.5, 0.25, 0.1, 0.05};



	// --- properties

	/** precalculation for fluctuation strength weighting */
	double fluct_curve[FLUCT_CURVE_LEN];





	/** convenience mirror flags for extraction of feature vector types
	 * <p>Will be set at the beginning of getFeatures.</p>
	 */
	bool bExtractRP;
	bool bExtractRH;
	bool bExtractSSD;
	bool bExtractTRP;
	bool bExtractTRH;
	bool bExtractTSSD;


	/** get features for one song<br>
		This method will normalize the audio data and join stereo channels to mono
		if applicable.
		@param buf the raw audio data
		@param opt options struct
		@param feat <b>destination</b> for features: (uninitialized) feature struct
		@param feats (unitialized) struct containing all segments' features
			<p>This struct is only allocated and filled if bReturnSegmentFeatures in options is true

	*/
	void* getFeatures(double* buf, tAudioformat *audiodata, Smafeopt* opt,
			tRPFeatures* &songFeat, tRPFeatures4Segs* &feats);

	/** get features for 6 s segment
		@param wav wav buffer (can be whole song)
		@param iBufOffset start feature extraction at this offset (number of samples)
		@param iBufLen length of segment in samples
		@param opt options struct
		@param ad audio format information (sample frequence etc)
		@param feat OUT <b>destination</b> for features: (uninitialized) feature struct
	*/
	void getFeaturesForSegment(double* wav, int iBufOffset, int iBufLen, Smafeopt* opt, tRPFeatures* feat);


	/** calculate SSD features<br>
		used for both "original" SSD and temporal evolution of other features (TSSD, TRP, TRH)
		@param matrix 2D matrix with Bark bands as columns and frames as rows
		@param cols number of matrix' columns
		@param rows number of matrix' rows
		@param dest ssd, trp or tssd member of a feature struct. <br>
			Should <b>not</b> be initialized (will be allocated in method)
	*/
	void getSSD(double* matrix, int cols, int rows, double* &dest);

	/** precalculates blur matrix with given size
		@param size size of matrix (both rows and columns)
		@param blur destination of matrix
	*/
	void buildBlurMatrix(int size, double* &blur);

	void setBlur1(double* b);
	void setBlur2(double* b);
	double* getBlur1();
	double* getBlur2();

	virtual std::string serializeConfig(std::string type) {
			if (type == smafeRPExtractor::FVTYPE_NAME_RP || type == smafeRPExtractor::FVTYPE_NAME_TRP) {
				std::string s = "uiSkipin="+stringify(opt.uiSkipin)+", uiSkipout="+stringify(opt.uiSkipout)+
						", uiStepwidth="+stringify(opt.uiStepwidth)+
						", bSpectralmasking=NA, bNormalizeFFTEnergy="+stringify(opt.bNormalizeFFTEnergy)+", bTransformDecibel="+stringify(opt.bTransformDecibel)+
						", bTransformPhon=NA, bTransformSone="+stringify(opt.bTransformSone)+
						", uiModAmplLimit="+stringify(opt.uiModAmplLimit)+
						", bIncludeDC="+stringify(opt.bIncludeDC)+", shNumBarkBands="+stringify(opt.shNumBarkBands)+
						",  bFluctuationStrengthWeighting="+stringify(opt.bFluctuationStrengthWeighting)+
						", bBlurring1="+stringify(opt.bBlurring1)+
						", bBlurring2="+stringify(opt.bBlurring2);
				return std::string(s);
			}
			// not rp and not trp
			std::string s = "uiSkipin="+stringify(opt.uiSkipin)+", uiSkipout="+stringify(opt.uiSkipout)+
							", uiStepwidth="+stringify(opt.uiStepwidth)+
							", bSpectralmasking=NA, bNormalizeFFTEnergy="+stringify(opt.bNormalizeFFTEnergy)+", bTransformDecibel="+stringify(opt.bTransformDecibel)+
							", bTransformPhon=NA, bTransformSone="+stringify(opt.bTransformSone)+
							", uiModAmplLimit="+stringify(opt.uiModAmplLimit)+
							", bIncludeDC="+stringify(opt.bIncludeDC)+", shNumBarkBands="+stringify(opt.shNumBarkBands)+
							",  bFluctuationStrengthWeighting="+stringify(opt.bFluctuationStrengthWeighting);
					return std::string(s);
	}

private:
	/** blur matrix 1 initialized? */
	bool blurMatricesInit1;
	/** blur matrix 2 initialized? */
	bool blurMatricesInit2;
	/** matrices for precalculation for blurring filter<br>
	size will be number of bark bands*/
	double* blur1; //
	/** matrices for precalculation for blurring filter<br>
	size will be uiModAmplLimit*/
	double* blur2; //

};

