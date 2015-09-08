/*
 * TestRunner.cpp
 *
 *  Created on: Feb 12, 2009
 *      Author: nig
 *
 *  Created from cppunit-1.12.1/examples/simple/main.cpp
 *  and cppunit-1.12.1/examples/qt/main.cpp
 */

#include <stdlib.h>
#include <vector>
#include <config.h>
using namespace std;


#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>

// check dependencies with qt & exclude includes & source ??
// in Order to enable the QtTestRunner compile with option -DQT_TEST_RUNNER
// you need the library qttestrunner to run QtTestRunner
#ifdef QT_TEST_RUNNER
#include <cppunit/ui/qt/QtTestRunner.h>		// needed only for Qt UI
#include <qt3/qapplication.h>				// needed only for Qt UI
#endif



int
main( int argc, char* argv[] )
{

    vector<string> args(argv, argv + argc);
    // Loop over command-line args
    for (vector<string>::iterator i = args.begin(); i != args.end(); ++i) {
        if (*i == "-h" || *i == "--help") {
        	// print help on Test Runner
            cout << "CppUNIT Test Runner:" << endl;
			#ifdef QT_TEST_RUNNER
				cout << "optional argument <-qt> will render QtTestRunner" << endl;
			#else
				cout << "no arguments supported - compile with QtTestRunner support (-DQT_TEST_RUNNER) to enable Qt GUI." << endl;
			#endif
            exit( EXIT_SUCCESS );
        }
		#ifdef QT_TEST_RUNNER
			else if (*i == "-qt") {
			QApplication app( argc, argv );
			CPPUNIT_NS::QtTestRunner runner;
			runner.addTest( CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest() );
			runner.run( true );
			exit( EXIT_SUCCESS );
			}
		#endif
    }

    // if no option was found execute regular Test Runner

	  // Create the event manager and test controller
	  CPPUNIT_NS::TestResult controller;

	  // Add a listener that collects test result
	  CPPUNIT_NS::TestResultCollector result;
	  controller.addListener( &result );

	  // Add a listener that print dots as test run.
	  CPPUNIT_NS::BriefTestProgressListener progress;
	  controller.addListener( &progress );

	  // Add the top suite to the test runner
	  CPPUNIT_NS::TestRunner runner;
	  runner.addTest( CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest() );
	  runner.run( controller );

	  // Print test in a compiler compatible format.
	  CPPUNIT_NS::CompilerOutputter outputter( &result, CPPUNIT_NS::stdCOut() );
	  outputter.setLocationFormat("%p:%l: "); 	// needed that eclipse/gcc recognizes the format
	  outputter.write();

	  result.wasSuccessful() ? exit( EXIT_SUCCESS ) : exit( EXIT_FAILURE );  // if successful exit with EXIT_SUCCESS

}
