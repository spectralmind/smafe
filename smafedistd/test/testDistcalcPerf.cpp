///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// testDistcalcPerf.cpp
//
// Performance test for distance calculation (fv are in memory)
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

#ifndef TESTDISTCALCPERF_H_
#define TESTDISTCALCPERF_H_

// needed by cppunit
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

// includes for the test
#include <stdlib.h>
//#include <map>

// for OpenMP
#include <omp.h>

// for pid
#include <sys/types.h>
#include <unistd.h>

#include "smafeFeatureVectorClasses.h"
#include "smafeutil.h"
#include "smafeFVType.h"
#include "smafeDistancesCalc.h"


/**
 * <p>Performs performance test for live distance calculation.</p>
 * <p>The follow steps are performed for each n-subtest:</p>
 * <ol>
 * <li>Generate n feature vectors with FVLEN random numbers between 0 and MAXVAL</li>
 * <li>Calculate L1 distanes between the first fv and each other fvs.</li>
 * <li>Sort according to distances (with quicksort)</li>
 * <li>Print out top 20 "fv"</li>
 * </ol>
 * <p>Points 2 and 3 are measured with both clock() and time() functions.</p>
 * <p><b>Memory requirements</b> are prined for edsfach n-subtest.</p>
 * <p><b>Details:</b>Currently, two c arrays (one double[], one long[]) are used to store distance values and
 * "track_id", respectively. Dealing with STL container and algo seemed to be both to cumbersum and slow.</p>
 *
 * */
class TestDistcalcPerf : public CPPUNIT_NS :: TestFixture
{
	// declare the suite by passing the class name to the macro
	CPPUNIT_TEST_SUITE ( TestDistcalcPerf );

	// declare each test case of the fixture
	CPPUNIT_TEST (tests);

	// at last end the declaration
	CPPUNIT_TEST_SUITE_END ();

public:
	TestDistcalcPerf() : fvs(NULL), FVLEN(168), MAXVAL(1), TOPK(1000), fvs_len(0), ds(NULL), track_ids(NULL) {}
	// setUp and tearDown need to be public
	void setUp (void);
	void tearDown (void);

	/** type for map where you can store pairs of <track_id, dist> pairs */
	//	typedef std::map<long, double> tDsMap;
	/** type for mutlipamp where you can store pairs of <dist, track_id> pairs */
	//	typedef std::map<double, long> tDsMultimap;



	// each test case needs to be declared protected
protected:
	void tests (void);


	// all the rest

	// variables used by test
private:
	/** assumed length of feature vector */
	const int FVLEN;
	/** max value for feature vectorr elements */
	const int MAXVAL;
	/** top k */
	const int TOPK;
	/** array of tracks to calculate distance to */
	SmafeNumericFeatureVector **fvs;
	/** length of array */
	size_t fvs_len;
	char strings_in_c_suck[100];
	/** dummy fv type */
	SmafeFVType fvtype;
	/** the calc'd distance */
	//tDsMap ds;
	//	tDsMultimap ds;
	double* ds;
	/** the corresponding track ids */
	long* track_ids;

	void tearDownTest (void);
	void testn(size_t n);
	void populate_fvs(size_t n);

};

#endif /* include guard */

// register the suite in the test factory registry:
CPPUNIT_TEST_SUITE_REGISTRATION ( TestDistcalcPerf );

// source


/** Description for function */
void TestDistcalcPerf :: setUp (void)
{
	// init Test
	std::cout << " setup TestDistcalcPerf: \n";

	srand(666);	 //initialize random number generator
}


void TestDistcalcPerf :: tearDownTest (void)
{
	// clean Test
	for (size_t i = 0; i < fvs_len; i++) {
		delete fvs[i];
	}
	fvs_len=0;
	delete[] fvs;
	fvs=NULL;
	delete[] ds;
	ds=NULL;
	delete[] track_ids;
	track_ids=NULL;
}

void TestDistcalcPerf :: tearDown (void)
{
	tearDownTest();
	std::cout << " tear down TestDistcalcPerf: ";
}


// ------------------------------------------
// qsort for two arrays
// ------------------------------------------
// where one array is used for sorting and the other
// array tracks the original order
// (=track_ids)
void swap_pairs(double l[],  long k[], size_t i, size_t j) {
	double dummy;
	dummy=l[j];
	l[j]=l[i];
	l[i]=dummy;

	long dummy2;
	dummy2=k[j];
	k[j]=k[i];
	k[i]=dummy2;
}

size_t partions_pairs(double l[], long k[], size_t low, size_t high)
{
	double prvotkey=l[low];
	while (low<high)
	{
		while (low<high && l[high]>=prvotkey)
			--high;
		swap_pairs(l, k, high, low);
		while (low<high && l[low]<=prvotkey)
			++low;
		swap_pairs(l, k, high, low);
	}

	return low;
}


/** Sort the elements of a between indexes left and right inclusive
 *
 */
void qsort_pairs(double l[], long k[], size_t left, size_t right) {
	//std::cout << "quicksort_" << left << " " << right << "\n";
	size_t pivotIdx, newPivotIdx;
    if (right > left) {
        //Choose a pivot element a[pivotIdx] with left <= pivotIdx <= right
    	//pivotIdx = left;
        // Here, partition() returns the new index of the pivot element
        newPivotIdx = partions_pairs(l, k, left, right);
        //std::cout << "___newpi" << newPivotIdx << "\n";
        qsort_pairs(l, k, left, newPivotIdx);
        qsort_pairs(l, k, newPivotIdx+1, right);
    }
}


/** Rearrange the elements so that the k smallest elements appear at the beginning
 *
 */
void qsort_pairs_topk(double l[], long k[], size_t left, size_t right, long maxk) {
	size_t pivotIdx, newPivotIdx;
    while (right > left) {
        //Choose a pivot element a[pivotIdx] with left <= pivotIdx <= right
    	//pivotIdx = left;
        newPivotIdx = partions_pairs(l, k, left, right);
        //std::cout << "newpi" << newPivotIdx << "\n";
        if (maxk <= newPivotIdx) {
            right = newPivotIdx - 1;
        } else if (newPivotIdx - left > right - newPivotIdx) {
            qsort_pairs(l, k, newPivotIdx+1, right);
            right = newPivotIdx - 1;
        } else {
            qsort_pairs(l, k, left, newPivotIdx-1);
            left = newPivotIdx + 1;
        }
    }
}


// ------------------------------------------


void TestDistcalcPerf :: tests (void) {
	// run all tests
	testn(10e3);
	testn(100e3);
	testn(500e3);
	//testn(1e6); // temporarily disabled
	//testn(5e6);
}

void TestDistcalcPerf :: testn (size_t n)
{
	clock_t end_clock;
	time_t end_time;


	std::cout << " - running TestDistcalcPerf with " <<  n << " feature vectors of length "+stringify(FVLEN)+" - \n";
	size_t memreq = n * (sizeof(double) * FVLEN + sizeof(long));
	std::cout << " - estimated memory consumption is " << memreq/1000000 << " MB- \n";

	// create fvs
	std::cout << " - generating feature vectors - \n";
	populate_fvs(n);

	std::cout << "PID = " << getpid() << std::endl;


//	char t;
//	std::cout << "CHECK1 Press a key then press enter: ";
//	std::cin  >> t;

	// res mem
	ds = new double[fvs_len-1];
	track_ids = new long[fvs_len-1];

//	std::cout << "CHECK2 Press a key then press enter: ";
//	std::cin  >> t;

	// for "benchmark"
	std::cout << " - calculating distances - \n";
	clock_t begin_clock=clock();
	time_t begin_time = time (NULL);


	// at least two vecs;
	assert(fvs_len > 2);


	// do it
	#pragma omp parallel for
	for (long i = 1; i < fvs_len; i++) {
		if (i == 1)
			std::cout << "Hello from thread " <<omp_get_thread_num() << ", nthreads " <<  omp_get_num_threads() << std::endl;

		// the 0 is a dummy value: all distance calc functions have the same signature!
		double d = SmafeDistancesCalc::getDistance_L1(0, fvs[0]->buffer, fvs[0]->buflen, fvs[i]->buffer, fvs[i]->buflen);

		//std::cout << d << '\n';
		// insert in <dist, id> multimap
		// at each insert, map must be sorted again
		// -> inefficient
		//		ds.insert(std::pair<double, size_t>(d, i));
		// insert it in <id, dist> map
		// effiecent, since position will alwyas be end of map because track-ids are sorted here
		//		ds.insert(ds.end(), std::pair<size_t, double>(i, d)); // insert this new distance vlaue right after last element
		//		ds.insert(std::pair<size_t, double>(i, d)); // insert this new distance vlaue right after last element
		// -> effiecent
		// http://www.cplusplus.com/reference/stl/map/insert/;


		ds[i-1] = d;
		track_ids[i-1] = i;
	}


	end_clock=clock();
	end_time = time (NULL);
	sprintf(strings_in_c_suck, "\n==Execution time: %.0f s (wall clock time) / %.0f ms (usr time) .==\n\n", difftime (end_time,begin_time) , diffclock(end_clock,begin_clock));
	std::cout << strings_in_c_suck;

	std::cout << " - sorting - \n";

	// sorting
	//	MySet s( ds.begin(), ds.end() );
	//	MySet::iterator it = s.begin();
	//	size_t i = 0;
	//	while ( it != s.end() && i < 100 ) {
	//		std::
	//		cout << i << ": " << *it++ << '\n';
	//	}
	// sorting variant with two arrays (c style)
	qsort_pairs_topk(ds, track_ids, 0, fvs_len - 1 - 1, TOPK); // highest index, not number of elementsHERE
	//qsort_pairs(ds, track_ids, 0, fvs_len - 1 - 1); // highest index, not number of elements



	// execution time
	end_clock=clock();
	end_time = time (NULL);
	sprintf(strings_in_c_suck, "\n==Execution time: %.0f s (wall clock time) / %.0f ms (usr time) .==\n\n", difftime (end_time,begin_time) , diffclock(end_clock,begin_clock));
	std::cout << strings_in_c_suck;

	// output
	//std::cout.precision(2);
	// output variant multimap <double, long>
	//	tDsMultimap::iterator it = ds.begin();
	//	for (int i=0; i < 20 &&  it != ds.end(); i++, it++) {
	//		std::cout << (i+1) << ". Track_id=" << it->second << ", dist=" << it->first << std::endl;
	//	}

	// output variant two arrays c style
	for (size_t i=0; i < 20; i++) {
		std::cout << (i+1) << ". Track_id=" << track_ids[i] << ", dist=" << ds[i] << std::endl;
	}


//	std::cout << "CHECK3 Press a key then press enter: ";
//	std::cin  >> t;

	// make ready for next test;
	tearDownTest();
}

void TestDistcalcPerf ::  populate_fvs(size_t n) {
	fvs = new SmafeNumericFeatureVector*[n];
	double buf[FVLEN];

	fvtype.setProperties(FVLEN, 1, "(dummy)");
	for (fvs_len = 0; fvs_len < n; fvs_len++) {
		for (int i = 0; i < FVLEN; i++) {
			double r = (   (double)rand() / ((double)(RAND_MAX)+(double)(1)) );
			double x = (r * MAXVAL);
			buf[i] = x;
		}
		fvs[fvs_len] = new SmafeNumericFeatureVector(buf, &fvtype, true);
	}
}
