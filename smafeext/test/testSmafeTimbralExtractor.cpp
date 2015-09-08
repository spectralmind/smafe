///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// TestSmafeTimbralExtractor
//
// unit tests of SmafeTimbralExtractor functions
// ------------------------------------------------------------------------
//
// $Id: testSmafeRPExtractor.cpp 278 2009-07-02 17:16:35Z ewald $
//
//
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////

// inline header, you can also use a separate header if you want to ;-)



#ifndef TESTSMAFETIMBRALEXTRACTOR_H_
#define TESTSMAFETIMBRALEXTRACTOR_H_


// needed by cppunit
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

// includes for the test
#include "smafeLogger.h"
#include "smafeopt.h"
#include "tAudioformat.h"
#include "SmafeTimbralExtractor.h"
#include "math.h"


// TODO: ? not nice, do you know a better way?
// include input wav file as char array
// #include <test/resources/inputWavRPExtract.txt>
// include result als global varibale
#include <test/resources/featuresResultArraysTimbralExtract.txt>

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
class TestSmafeTimbralExtractor : public CPPUNIT_NS :: TestFixture
{
	// declare the suite by passing the class name to the macro
    CPPUNIT_TEST_SUITE ( TestSmafeTimbralExtractor );

    // declare each test case of the fixture
    //CPPUNIT_TEST (testExtractionWithSilence); do not use this test for now
    CPPUNIT_TEST (testExtractionWithWave);

    // at last end the declaration
    CPPUNIT_TEST_SUITE_END ();

    public:
    	TestSmafeTimbralExtractor() :
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
		std::string theFvType;
		std::string theFeaturePosition;


		tAudioformat af;
		Smafeopt so;
		// the extractor
		SmafeTimbralExtractor smafeExt;
		// featureVectorTypes
		SmafeFVType_Ptr_map fvts;
		SmafeFVType* fv;
		SmafeFVType_Ptr fv_ptr;


};



#endif /* TESTSMAFETIMBRALEXTRACTOR_H_ */


// register the suite in the test factory registry:
CPPUNIT_TEST_SUITE_REGISTRATION ( TestSmafeTimbralExtractor );

// source


/** Description for function */
void TestSmafeTimbralExtractor :: setUp (void)
{
	// init Test
	std::cout << " setup TestSmafeTimbralExtractor: \n";
	initParameters();

}

/** Description for function */
void TestSmafeTimbralExtractor :: tearDown (void)
{
    // clean Test
	// undo changes
	std::cout << " tear down TestSmafeTimbralExtractor: ";
	std::cout << "\n Name of current FV: " + theFvType;
	std::cout << "\n Current position in FV (beginning from 1): " + theFeaturePosition;


	std::cout << "\n last allowed delta was: " + stringify(maxdelta);
	std::cout << "\n last delta: " + stringify(delta);
	std::cout << "\n last extracted: " + stringify(extracted);
	std::cout << "\n last reference: " + stringify(reference);


	std::cout << "\n the max delta factor was: " + stringify(maxdeltafactor);
	std::cout << "\n the median delta was: " + stringify(mediandelta);
	std::cout << "\n the max delta was: " + stringify(actualmaxdelta);

}

/** Check if extracted Values are euqal to a reference output */
// currently not used (not all features are 0: spectral roll off of silence is 1 ?
void TestSmafeTimbralExtractor :: testExtractionWithSilence (void)
{
	std::cout << " - running testExtractionWithSilence - \n";

	// ------- output params, declarations
	std::vector< SmafeAbstractFeatureVector_Ptr > fvs; // array of feature vectors returned
	double *referenceArray;

	// ------- processing
	double *inputWAV = createSilence();
	(void) smafeExt.getFeatures(inputWAV,
			&af,
			&so,
			&fvts,
			fvs);

	// By now, the vector fvs contains one or more instances of SmafeAbstractFeatureVector,
	// which is in fact of the dynamic type SmafeNumericFeatureVector.

/*
	// code used to output a reference result
		std::ofstream outfile ("unittest_getFeatures_result.txt");
		outfile.precision(20);
		for(std::vector<SmafeAbstractFeatureVector_Ptr>::iterator iter = fvs.begin(); iter < fvs.end(); iter++) {
			SmafeAbstractFeatureVector* theFv = iter->get();
			SmafeNumericFeatureVector *snfv1;
			snfv1 = dynamic_cast<SmafeNumericFeatureVector*>(theFv);

			writeArrayAsCode(outfile, theFv->fvtype->name, snfv1->buffer, snfv1->buflen);
		} // end of iterator
		outfile.close();
*/

	// Testing
	for(std::vector<SmafeAbstractFeatureVector_Ptr>::iterator iter = fvs.begin(); iter < fvs.end(); iter++) {
		SmafeAbstractFeatureVector* theFv;
		theFv = iter->get();
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
			theFeaturePosition = stringify(n+1);

			// actually check if the values are equal with respect to maxdelta
			CPPUNIT_ASSERT_DOUBLES_EQUAL(reference, extracted, maxdelta);
		}
	} // end of iterator
}

/** Check if extracted Values are euqal to a reference output */
void TestSmafeTimbralExtractor :: testExtractionWithWave (void)
{
	std::cout << " - running testExtractionWithWave - \n";

	// ------- output params, declarations
	std::vector< SmafeAbstractFeatureVector_Ptr > fvs; // array of feature vectors returned
	double *referenceArray;

	// ------- processing
	double *inputWAV = createWave();
	(void) smafeExt.getFeatures(inputWAV,
			&af,
			&so,
			&fvts,
			fvs);

	// By now, the vector fvs contains one or more instances of SmafeAbstractFeatureVector,
	// which is in fact of the dynamic type SmafeNumericFeatureVector.

/*
	// code used to output a reference result
		std::ofstream outfile ("unittest_getFeatures_result.txt");
		outfile.precision(20);
		for(std::vector<SmafeAbstractFeatureVector_Ptr>::iterator iter = fvs.begin(); iter < fvs.end(); iter++) {
			SmafeAbstractFeatureVector* theFv = iter->get();
			SmafeNumericFeatureVector *snfv1;
			snfv1 = dynamic_cast<SmafeNumericFeatureVector*>(theFv);

			writeArrayAsCode(outfile, theFv->fvtype->name, snfv1->buffer, snfv1->buflen);
		} // end of iterator
		outfile.close();
*/
	// Testing
	for(std::vector<SmafeAbstractFeatureVector_Ptr>::iterator iter = fvs.begin(); iter < fvs.end(); iter++) {
		SmafeAbstractFeatureVector* theFv;
		theFv = iter->get();
		SmafeNumericFeatureVector *snfv1;
		snfv1 = dynamic_cast<SmafeNumericFeatureVector*>(theFv);

		theFvType = theFv->fvtype->name;
		if (theFv->fvtype->name.compare(SmafeTimbralExtractor::FVTYPE_NAME_TIMEDOMAINFEATURES) == 0) {
			referenceArray = TIMEDOM;
		} else if (theFv->fvtype->name.compare(SmafeTimbralExtractor::FVTYPE_NAME_SPECTRALFEATURES) == 0) {
			referenceArray = SPECTRAL;
		} else {
			CPPUNIT_FAIL("Not a valid fv type.");
		}

		for( int n = 0; n < snfv1->buflen; ++n ) {
			// prepare the result, create stats
			extracted = snfv1->buffer[n];
			reference = referenceArray[n];
			maxdelta = fabs(reference * maxdeltafactor);
			delta = fabs(extracted - reference);
			if (delta > actualmaxdelta) actualmaxdelta = delta;
			mediandelta = (delta + mediandelta) /2;
			theFeaturePosition = stringify(n+1);

			// actually check if the values are equal with respect to maxdelta
			CPPUNIT_ASSERT_DOUBLES_EQUAL(reference, extracted, maxdelta);
		}
	} // end of iterator
}


/** Create input for the extration
 * in this case complete silence ;)
 */
double* TestSmafeTimbralExtractor :: createSilence(){
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
double* TestSmafeTimbralExtractor :: createWave(){
	double* a = NULL;   // Pointer to int, initialize to nothing.
	long n=10000000;           // Size needed for array
	a = new double[n];  // Allocate n ints and save ptr in a.
	for (long i=0; i<n; i++) {
		// the sin needs to be modulated or the current version #238 of smafe will generate nan values for the SSD / TSSD vectors
	    a[i] = sin(2000*3.14159265*i/22050) * sin(100*3.14159265*i/22050) + double(i)/double(n)*2-1;
	}

	return a;
}

/**
 * ------- input params, declarations and definitions
 *
 * */
void TestSmafeTimbralExtractor::initParameters(void){
	// audio format
	af.iChannels = 1;
	af.iSamplerate = 22050;
	af.ulNumSamples = 1000000;
	af.label = std::string("generatedtest.wav");
	af.encoding = std::string("PCM");

	// options
	so.sFilename = std::string("generatedtest.wav");
	so.sCollectionName = "";
	// song dep opts
	so.iFFTSize = 512;
	// are set only in loop
	so.mapExtractForSong[SmafeTimbralExtractor::FVTYPE_NAME_TIMEDOMAINFEATURES] =
		so.mapExtractForSong[SmafeTimbralExtractor::FVTYPE_NAME_SPECTRALFEATURES] = true;



	fv = new SmafeFVType();
	fv->name = std::string(SmafeTimbralExtractor::FVTYPE_NAME_TIMEDOMAINFEATURES);
	fv->version = 0; // todo: this is not used, is it?
	fv->class_id = CLASS_ID_NUMERIC;
	fv_ptr.reset(fv);
	fvts[fv->name] = fv_ptr;

	fv = new SmafeFVType();
	fv->name = std::string(SmafeTimbralExtractor::FVTYPE_NAME_SPECTRALFEATURES);
	fv->version = 0;
	fv->class_id = CLASS_ID_NUMERIC;
	fv_ptr.reset(fv);
	fvts[fv->name] = fv_ptr;


	// set FVTProperties
	smafeExt.setFVTProperties(fvts, so);
}
