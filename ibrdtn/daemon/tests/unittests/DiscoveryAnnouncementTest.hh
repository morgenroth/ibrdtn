/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        DiscoveryAnnouncementTest.hh
/// @brief       CPPUnit-Tests for class DiscoveryAnnouncement
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef DISCOVERYANNOUNCEMENTTEST_HH
#define DISCOVERYANNOUNCEMENTTEST_HH
class DiscoveryAnnouncementTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'DiscoveryAnnouncement' ===*/
		void testGetEID();
		void testGetServices();
		void testClearServices();
		void testAddService();
		void testGetService();
		void testToString();
		void testSetSequencenumber();
		void testIsShort();
		void testOperatorShiftLeft();
		void testOperatorShiftRight();
		/*=== END   tests for class 'DiscoveryAnnouncement' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(DiscoveryAnnouncementTest);
			CPPUNIT_TEST(testGetEID);
			CPPUNIT_TEST(testGetServices);
			CPPUNIT_TEST(testClearServices);
			CPPUNIT_TEST(testAddService);
			CPPUNIT_TEST(testGetService);
			CPPUNIT_TEST(testToString);
			CPPUNIT_TEST(testSetSequencenumber);
			CPPUNIT_TEST(testIsShort);
			CPPUNIT_TEST(testOperatorShiftLeft);
			CPPUNIT_TEST(testOperatorShiftRight);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* DISCOVERYANNOUNCEMENTTEST_HH */
