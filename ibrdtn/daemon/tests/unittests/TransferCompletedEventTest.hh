/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        TransferCompletedEventTest.hh
/// @brief       CPPUnit-Tests for class TransferCompletedEvent
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef TRANSFERCOMPLETEDEVENTTEST_HH
#define TRANSFERCOMPLETEDEVENTTEST_HH
class TransferCompletedEventTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'TransferCompletedEvent' ===*/
		void testGetName();
		void testToString();
		void testRaise();
		void testGetPeer();
		void testGetBundle();
		/*=== END   tests for class 'TransferCompletedEvent' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(TransferCompletedEventTest);
			CPPUNIT_TEST(testGetName);
			CPPUNIT_TEST(testToString);
			CPPUNIT_TEST(testRaise);
			CPPUNIT_TEST(testGetPeer);
			CPPUNIT_TEST(testGetBundle);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* TRANSFERCOMPLETEDEVENTTEST_HH */
