/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        BundleReceivedEventTest.hh
/// @brief       CPPUnit-Tests for class BundleReceivedEvent
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef BUNDLERECEIVEDEVENTTEST_HH
#define BUNDLERECEIVEDEVENTTEST_HH
class BundleReceivedEventTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'BundleReceivedEvent' ===*/
		void testGetName();
		void testToString();
		void testRaise();
		/*=== END   tests for class 'BundleReceivedEvent' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(BundleReceivedEventTest);
			CPPUNIT_TEST(testGetName);
			CPPUNIT_TEST(testToString);
			CPPUNIT_TEST(testRaise);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* BUNDLERECEIVEDEVENTTEST_HH */
