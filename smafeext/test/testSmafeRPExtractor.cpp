///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// TestSmafeRPExtractor
//
// unit tests of SmafeRPExtractor functions
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

// inline header, you can also use a separate header if you want to ;-)

#ifndef TESTSMAFERPEXTRACTOR_H_
#define TESTSMAFERPEXTRACTOR_H_

// needed by cppunit
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

// includes for the test
#include "smafeLogger.h"
#include "smafeopt.h"
#include "tAudioformat.h"
#include "smafeRPExtractor.h"
#include "math.h"


using namespace std;

// TODO: ? not nice, do you know a better way?
// include input wav file as char array
// #include <test/resources/inputWavRPExtract.txt>
// include result als global varibale
#include <test/resources/featuresResultArraysRPExtract.txt>

/**
 * <p>Performs unit test of public method
 * <code>void* smafeRPExtractor::getFeatures(char* buf, tAudioformat audiodata, Smafeopt* so, SmafeFVType_Ptr_map* fvts, std::vector< SmafeAbstractFeatureVector_Ptr > &fvs, tRPFeatures4Segs* &feats)</code>
 * in class <code>smafeRPExtractor</code>.</p>
 * <p>The function declares input and output parameters and sets initial values for input params,
 * as well as calls the method to be tested.</p>
 *
 * If you need to initialize multiple tests with the same values or objects
 * you should work with fixtures:
 * - http://cppunit.sourceforge.net/doc/lastest/class_test_fixture.html
 * - http://cppunit.sourceforge.net/doc/lastest/cppunit_cookbook.html#fixture
 *
 * */
class TestSmafeRPExtractor : public CPPUNIT_NS :: TestFixture
{
	// declare the suite by passing the class name to the macro
    CPPUNIT_TEST_SUITE ( TestSmafeRPExtractor );

    // declare each test case of the fixture
    CPPUNIT_TEST (testExtractionWithSilence);
    CPPUNIT_TEST (testExtractionWithWave);

    // at last end the declaration
    CPPUNIT_TEST_SUITE_END ();

    public:
    	TestSmafeRPExtractor() :
    		maxdeltafactor(1E-5), // multiplied with the reference value -> defines the max delta allowed when comparing the reference to the extracted value
    		maxdelta(0),
    		mediandelta(0),
    		actualmaxdelta(0)
			{}
    	// setUp and tearDown need to be public
        void setUp (void);
        void tearDown (void);

    // each test case needs to be declared protected
    protected:
        void testExtractionWithSilence (void);
        void testExtractionWithWave (void);


	// all the rest

    // variables used by test
    private:
    	void initParameters();
    	double* createSilence();
    	double* createWave();

		double maxdeltafactor;
		double extracted;
		double reference;
		double maxdelta;
		double mediandelta;
		double actualmaxdelta;
		double delta;
		tAudioformat af;
		Smafeopt so;
		// the extractor
		smafeRPExtractor smafeRPExt;
		// featureVectorTypes
		SmafeFVType_Ptr_map fvts;
		SmafeFVType* fv;
		SmafeFVType_Ptr fv_ptr;


};

#endif /* TESTSMAFERPEXTRACTOR_H_ */

// register the suite in the test factory registry:
CPPUNIT_TEST_SUITE_REGISTRATION ( TestSmafeRPExtractor );

// source


/** Description for function */
void TestSmafeRPExtractor :: setUp (void)
{
	// init Test
	cout << " setup TestSmafeRPExtractor: \n";
	initParameters();

}

/** Description for function */
void TestSmafeRPExtractor :: tearDown (void)
{
    // clean Test
	// undo changes
	cout << " tear down TestSmafeRPExtractor: ";

	cout << "\n last allowed delta was: " + stringify(maxdelta);
	cout << "\n last delta: " + stringify(delta);
	cout << "\n last extracted: " + stringify(extracted);
	cout << "\n last reference: " + stringify(reference);


	cout << "\n the max delta factor was: " + stringify(maxdeltafactor);
	cout << "\n the median delta was: " + stringify(mediandelta);
	cout << "\n the max delta was: " + stringify(actualmaxdelta);

}

/** Check if extracted Values are euqal to a reference output */
void TestSmafeRPExtractor :: testExtractionWithSilence (void)
{
	cout << " - running testExtractionWithSilence - \n";

	// ------- output params, declarations
	std::vector< SmafeAbstractFeatureVector_Ptr > fvs; // array of feature vectors returned
	std::vector< SmafeAbstractFeatureVector_Ptr > segsfvs; // for segments
	//tRPFeatures4Segs* feats; // not used
	double *referenceArray;

	// ------- processing
	double *inputWAV = createSilence();
	(void) smafeRPExt.getFeatures(inputWAV,
			&af,
			&so,
			&fvts,
			fvs, segsfvs, "<no filename>");

	// By now, the vector fvs contains one or more instances of SmafeAbstractFeatureVector,
	// which is in fact of the dynamic type SmafeNumericFeatureVector.

	// Testing
	for(std::vector<SmafeAbstractFeatureVector_Ptr>::iterator iter = fvs.begin(); iter < fvs.end(); iter++) {
		SmafeAbstractFeatureVector* theFv = iter->get();
		SmafeNumericFeatureVector *snfv1;
		snfv1 = dynamic_cast<SmafeNumericFeatureVector*>(theFv);

		// all results are 0 for the silence input
		reference = 0;
		for( int n = 0; n < snfv1->buflen; ++n ) {
			// prepare the result, create stats
			extracted = snfv1->buffer[n];
			maxdelta = fabs(reference * maxdeltafactor);
			delta = fabs(extracted - reference);
			if (delta > actualmaxdelta) actualmaxdelta = delta;
			mediandelta = (delta + mediandelta) /2;

			// actually check if the values are equal with respect to maxdelta
			CPPUNIT_ASSERT_DOUBLES_EQUAL(reference, extracted, maxdelta);
		}
	} // end of iterator
}

/** Check if extracted Values are euqal to a reference output */
void TestSmafeRPExtractor :: testExtractionWithWave (void)
{
	cout << " - running testExtractionWithWave - \n";

	// ------- output params, declarations
	std::vector< SmafeAbstractFeatureVector_Ptr > fvs; // array of feature vectors returned
	std::vector< SmafeAbstractFeatureVector_Ptr > segsfvs; // for segments
	//tRPFeatures4Segs* feats; // not used
	double *referenceArray;

	// ------- processing
	double *inputWAV = createWave();
	(void) smafeRPExt.getFeatures(inputWAV,
			&af,
			&so,
			&fvts,
			fvs, segsfvs, "<no filename>");

	// By now, the vector fvs contains one or more instances of SmafeAbstractFeatureVector,
	// which is in fact of the dynamic type SmafeNumericFeatureVector.

	// code used to output a reference result
//		std::ofstream outfile ("unittest_getFeatures_result.txt");
//		outfile.precision(20);
//		for(std::vector<SmafeAbstractFeatureVector_Ptr>::iterator iter = fvs.begin(); iter < fvs.end(); iter++) {
//			SmafeAbstractFeatureVector* theFv = iter->get();
//			SmafeNumericFeatureVector *snfv1;
//			snfv1 = dynamic_cast<SmafeNumericFeatureVector*>(theFv);
//
//			writeArrayAsCode(outfile, theFv->fvtype->name, snfv1->buffer, snfv1->buflen);
//		} // end of iterator
//		outfile.close();

	// Testing
	for(std::vector<SmafeAbstractFeatureVector_Ptr>::iterator iter = fvs.begin(); iter < fvs.end(); iter++) {
		SmafeAbstractFeatureVector* theFv = iter->get();
		SmafeNumericFeatureVector *snfv1;
		snfv1 = dynamic_cast<SmafeNumericFeatureVector*>(theFv);


		if (theFv->fvtype->name.compare("RP") == 0) {
			referenceArray = RP;
		} else if (theFv->fvtype->name.compare("RH") == 0) {
			referenceArray = RH;
		} else if(theFv->fvtype->name.compare("SSD") == 0) {
			referenceArray = SSD;
		} else if(theFv->fvtype->name.compare("TRP") == 0) {
			referenceArray = TRP;
		} else if(theFv->fvtype->name.compare("TRH") == 0) {
			referenceArray = TRH;
		} else if(theFv->fvtype->name.compare("TSSD") == 0) {
			referenceArray = TSSD;
		}

		for( int n = 0; n < snfv1->buflen; ++n ) {
			// prepare the result, create stats
			extracted = snfv1->buffer[n];
			reference = referenceArray[n];
			maxdelta = fabs(reference * maxdeltafactor);
			delta = fabs(extracted - reference);
			if (delta > actualmaxdelta) actualmaxdelta = delta;
			mediandelta = (delta + mediandelta) /2;

			// actually check if the values are equal with respect to maxdelta
			CPPUNIT_ASSERT_DOUBLES_EQUAL(reference, extracted, maxdelta);
		}
	} // end of iterator
}


/** Create input for the extration
 * in this case complete silence ;)
 */
double* TestSmafeRPExtractor :: createSilence(){
	double* a = NULL;   // Pointer to int, initialize to nothing.
	long n=10000000;           // Size needed for array
	a = new double[n];  // Allocate n ints and save ptr in a.
	for (long i=0; i<n; i++) {
	    a[i] = 0;    // Initialize all elements to zero.
	}

	return a;
}

/** Create input for the extration
 * in this case a sine
 */
double* TestSmafeRPExtractor :: createWave(){
	double* a = NULL;   // Pointer to int, initialize to nothing.
	long n=10000000;           // Size needed for array
	a = new double[n];  // Allocate n ints and save ptr in a.
	for (long i=0; i<n; i++) {
		// the sin needs to be modulated or the current version #238 of smafe will generate nan values for the SSD / TSSD vectors
	    a[i] = sin(2000*3.14159265*i/22050) * sin(100*3.14159265*i/22050);

	}

	return a;
}

/**
 * ------- input params, declarations and definitions
 *
 * */
void TestSmafeRPExtractor::initParameters(void){
	// audio format
	af.iChannels = 1;
	af.iSamplerate = 22050;
	af.ulNumSamples = 1000000;
	af.label = std::string("generatedtest.wav");
	af.encoding = std::string("PCM");

	// options
	//so.sFilename = std::string("generatedtest.wav");
	so.uiSkipin = 3;
	so.uiSkipout = 3;
	so.uiStepwidth = 1;
	so.bReturnSegmentFeatures = false;
	so.bNormalizeFFTEnergy = false;
	so.bTransformDecibel = true;
	so.bTransformSone = true;
	so.uiModAmplLimit = 60;
	so.bIncludeDC = false;
	so.shNumBarkBands = 24;
	so.bFluctuationStrengthWeighting = true;
	so.bBlurring1 = false;
	so.bBlurring2 = false;
	//so.sCollectionName = "";
	// song dep opts
	so.uiFeatFrom = 1;
	so.uiFeatTo = so.uiModAmplLimit;
	so.rh_len = so.uiFeatTo-so.uiFeatFrom+1;
	so.iFFTSize = 512;
	// are set only in loop
	so.mapExtractForSong[smafeRPExtractor::FVTYPE_NAME_RP] =
		so.mapExtractForSong[smafeRPExtractor::FVTYPE_NAME_RH] =
		so.mapExtractForSong[smafeRPExtractor::FVTYPE_NAME_SSD] =
		so.mapExtractForSong[smafeRPExtractor::FVTYPE_NAME_TRP] =
		so.mapExtractForSong[smafeRPExtractor::FVTYPE_NAME_TRH] =
		so.mapExtractForSong[smafeRPExtractor::FVTYPE_NAME_TSSD] = true;


	// RP
	fv = new SmafeFVType();
	fv->name = std::string(smafeRPExtractor::FVTYPE_NAME_RP);
	//fv->version = smafeRPExtractor::RP_VERSION;
	fv->class_id = std::string(CLASS_ID_NUMERIC);
	fv_ptr.reset(fv);
	fvts[fv->name] = fv_ptr;
	// RH
	fv = new SmafeFVType();
	fv->name = std::string(smafeRPExtractor::FVTYPE_NAME_RH);
	//fv->version = smafeRPExtractor::RP_VERSION;
	fv->class_id = std::string(CLASS_ID_NUMERIC);
	fv_ptr.reset(fv);
	fvts[fv->name] = fv_ptr;
	// SSD
	fv = new SmafeFVType();
	fv->name = std::string(smafeRPExtractor::FVTYPE_NAME_SSD);
	//fv->version = smafeRPExtractor::RP_VERSION;
	fv->class_id = std::string(CLASS_ID_NUMERIC);
	fv_ptr.reset(fv);
	fvts[fv->name] = fv_ptr;
	// TRP
	fv = new SmafeFVType();
	fv->name = std::string(smafeRPExtractor::FVTYPE_NAME_TRP);
	//fv->version = smafeRPExtractor::RP_VERSION;
	fv->class_id = std::string(CLASS_ID_NUMERIC);
	fv_ptr.reset(fv);
	fvts[fv->name] = fv_ptr;
	// TRH
	fv = new SmafeFVType();
	fv->name = std::string(smafeRPExtractor::FVTYPE_NAME_TRH);
	//fv->version = smafeRPExtractor::RP_VERSION;
	fv->class_id = std::string(CLASS_ID_NUMERIC);
	fv_ptr.reset(fv);
	fvts[fv->name] = fv_ptr;
	// TSSD
	fv = new SmafeFVType();
	fv->name = std::string(smafeRPExtractor::FVTYPE_NAME_TSSD);
	//fv->version = smafeRPExtractor::RP_VERSION;
	fv->class_id = std::string(CLASS_ID_NUMERIC);
	fv_ptr.reset(fv);
	fvts[fv->name] = fv_ptr;
	// set FVTProperties
	smafeRPExt.setFVTProperties(fvts, so);
}
