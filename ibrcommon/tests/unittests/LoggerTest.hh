/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        LoggerTest.hh
/// @brief       CPPUnit-Tests for class Logger
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef LOGGERTEST_HH
#define LOGGERTEST_HH
class LoggerTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'Logger' ===*/
		void testEmergency();
		void testAlert();
		void testCritical();
		void testError();
		void testWarning();
		void testNotice();
		void testInfo();
		void testDebug();
		void testGetSetVerbosity();
		void testAddStream();
		void testEnableSyslog();
		void testFlush();
		void testEnableAsync();
		void testStop();
		/*=== END   tests for class 'Logger' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(LoggerTest);
			CPPUNIT_TEST(testEmergency);
			CPPUNIT_TEST(testAlert);
			CPPUNIT_TEST(testCritical);
			CPPUNIT_TEST(testError);
			CPPUNIT_TEST(testWarning);
			CPPUNIT_TEST(testNotice);
			CPPUNIT_TEST(testInfo);
			CPPUNIT_TEST(testDebug);
			CPPUNIT_TEST(testGetSetVerbosity);
			CPPUNIT_TEST(testAddStream);
			CPPUNIT_TEST(testEnableSyslog);
			CPPUNIT_TEST(testFlush);
			CPPUNIT_TEST(testEnableAsync);
			CPPUNIT_TEST(testStop);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* LOGGERTEST_HH */
