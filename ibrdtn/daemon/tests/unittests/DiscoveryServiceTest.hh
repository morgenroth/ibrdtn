/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        DiscoveryServiceTest.hh
/// @brief       CPPUnit-Tests for class DiscoveryService
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef DISCOVERYSERVICETEST_HH
#define DISCOVERYSERVICETEST_HH
class DiscoveryServiceTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'DiscoveryService' ===*/
		void testGetLength();
		void testGetName();
		void testGetParameters();
		void testUpdate();
		void testOnInterface();
		void testOperatorShiftLeft();
		void testOperatorShiftRight();
		/*=== END   tests for class 'DiscoveryService' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(DiscoveryServiceTest);
			CPPUNIT_TEST(testGetLength);
			CPPUNIT_TEST(testGetName);
			CPPUNIT_TEST(testGetParameters);
			CPPUNIT_TEST(testUpdate);
			CPPUNIT_TEST(testOnInterface);
			CPPUNIT_TEST(testOperatorShiftLeft);
			CPPUNIT_TEST(testOperatorShiftRight);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* DISCOVERYSERVICETEST_HH */
