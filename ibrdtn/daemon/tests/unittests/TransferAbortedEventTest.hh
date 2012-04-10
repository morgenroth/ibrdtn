/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        TransferAbortedEventTest.hh
/// @brief       CPPUnit-Tests for class TransferAbortedEvent
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef TRANSFERABORTEDEVENTTEST_HH
#define TRANSFERABORTEDEVENTTEST_HH
class TransferAbortedEventTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'TransferAbortedEvent' ===*/
		void testGetName();
		void testToString();
		void testRaise();
		void testGetPeer();
		void testGetBundleID();
		/*=== END   tests for class 'TransferAbortedEvent' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(TransferAbortedEventTest);
			CPPUNIT_TEST(testGetName);
			CPPUNIT_TEST(testToString);
			CPPUNIT_TEST(testRaise);
			CPPUNIT_TEST(testGetPeer);
			CPPUNIT_TEST(testGetBundleID);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* TRANSFERABORTEDEVENTTEST_HH */
