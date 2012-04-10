/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        NeighborDatabaseTest.hh
/// @brief       CPPUnit-Tests for class NeighborDatabase
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef NEIGHBORDATABASETEST_HH
#define NEIGHBORDATABASETEST_HH
class NeighborDatabaseTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'NeighborDatabase' ===*/
		/*=== BEGIN tests for class 'NeighborEntry' ===*/
		void testNeighborEntryUpdateLastSeen();
		void testNeighborEntryUpdateBundles();
		/*=== END   tests for class 'NeighborEntry' ===*/

		void testUpdateLastSeen();
		void testUpdateBundles();
		void testKnowBundle();
		void testGetSetAvailable();
		void testSetUnavailable();
		void testGet();
		/*=== END   tests for class 'NeighborDatabase' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(NeighborDatabaseTest);
			CPPUNIT_TEST(testNeighborEntryUpdateLastSeen);
			CPPUNIT_TEST(testNeighborEntryUpdateBundles);
			CPPUNIT_TEST(testUpdateLastSeen);
			CPPUNIT_TEST(testUpdateBundles);
			CPPUNIT_TEST(testKnowBundle);
			CPPUNIT_TEST(testGetSetAvailable);
			CPPUNIT_TEST(testSetUnavailable);
			CPPUNIT_TEST(testGet);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* NEIGHBORDATABASETEST_HH */
