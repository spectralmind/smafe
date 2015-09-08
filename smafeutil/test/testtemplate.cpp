///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// testsmafeutil
//
// unit tests of Utiliy functions
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

#ifndef TESTSMAFEUTIL_H_
#define TESTSMAFEUTIL_H_

// needed by cppunit
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

// includes for the test
#include <time.h>

using namespace std;

/**
 * Description for class
 *
 * If you need to initialize multiple tests with the same values or objects
 * you should work with fixtures:
 * - http://cppunit.sourceforge.net/doc/lastest/class_test_fixture.html
 * - http://cppunit.sourceforge.net/doc/lastest/cppunit_cookbook.html#fixture
 *
 * */
class TestSmafeUtil : public CPPUNIT_NS :: TestFixture
{
	// declare the suite by passing the class name to the macro
    CPPUNIT_TEST_SUITE ( TestSmafeUtil );

    // declare each test case of the fixture
    CPPUNIT_TEST (firstTest);
    CPPUNIT_TEST (secondTest);

    // at last end the declaration
    CPPUNIT_TEST_SUITE_END ();

	// setUp and tearDown need to be public
    public:
        void setUp (void);
        void tearDown (void);

    // each test case needs to be declared protected
    protected:
        void firstTest (void);
        void secondTest (void);

	// all the rest
    private: int iValue;
};

#endif /* TESTSMAFEUTIL_H_ */

// register the suite in the test factory registry:
CPPUNIT_TEST_SUITE_REGISTRATION ( TestSmafeUtil );

// source


/** Description for function */
void TestSmafeUtil :: setUp (void)
{
	// init Test
	cout << " setup: ";
}

/** Description for function */
void TestSmafeUtil :: tearDown (void)
{
    // clean Test
	// undo changes
	cout << " tear down: ";
}

/** Description for function */
void TestSmafeUtil :: firstTest (void)
{
	cout << " - running firstTest - ";
	// examples how to use the assert macros: http://cppunit.sourceforge.net/doc/lastest/group___assertions.html
    // CPPUNIT_ASSERT (sText.compare("text"));
	// CPPUNIT_ASSERT_MESSAGE("oh no an error :-o", 2==1);
	// CPPUNIT_FAIL( "This is an intended Error. " );
    // CPPUNIT_ASSERT_EQUAL (iValue, 4);
	// CPPUNIT_ASSERT_EQUAL_MESSAGE("my message for the error", 2, 3);
	// CPPUNIT_ASSERT_DOUBLES_EQUAL(1.3, 1.2, 0.2);
	// CPPUNIT_ASSERT_THROW(throw 1, int);  // divide by zero does not work ?!?
	// CPPUNIT_ASSERT_NO_THROW(1==1);
	// CPPUNIT_ASSERT_ASSERTION_FAIL( CPPUNIT_ASSERT( 1==2 )); // if the assert fails the test succeeds
	// CPPUNIT_ASSERT_ASSERTION_PASS( CPPUNIT_ASSERT( 1==1 )); // same as simple assert ?


	// CPPUNIT_ASSERT_THROW(1/0, bad_exception);  // divide by zero does not work ?!?
	// the test will fail because div by zero is an error and can not be catched (is not anticipated)
	// all other test will perform as usual, also teardown will be executed after an error
	// for details see: http://cppunit.sourceforge.net/doc/1.8.0/cppunit_cookbook.html
	// ..
	// CppUnit distinguishes between failures and errors.
	// A failure is anticipated and checked for with assertions. Errors are unanticipated problems like division by zero and other exceptions thrown by the C++ runtime or your code.
	// ..

}

/** Description for function */
void TestSmafeUtil :: secondTest (void)
{
	cout << " - running secondTest - ";
    // CPPUNIT_ASSERT (4 == 5);						// Assert test fails
}
