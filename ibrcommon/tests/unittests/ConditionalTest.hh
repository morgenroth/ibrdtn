/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        ConditionalTest.hh
/// @brief       CPPUnit-Tests for class Conditional
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef CONDITIONALTEST_HH
#define CONDITIONALTEST_HH
class ConditionalTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'Conditional' ===*/
		void testSignal();
		void testWait();
		void testAbort();
		void testReset();
		void testGettimeout();
		/*=== END   tests for class 'Conditional' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(ConditionalTest);
			CPPUNIT_TEST(testSignal);
			CPPUNIT_TEST(testWait);
			CPPUNIT_TEST(testAbort);
			CPPUNIT_TEST(testReset);
			CPPUNIT_TEST(testGettimeout);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* CONDITIONALTEST_HH */
