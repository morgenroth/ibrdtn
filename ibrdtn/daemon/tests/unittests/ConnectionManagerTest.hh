/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        ConnectionManagerTest.hh
/// @brief       CPPUnit-Tests for class ConnectionManager
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef CONNECTIONMANAGERTEST_HH
#define CONNECTIONMANAGERTEST_HH
class ConnectionManagerTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'ConnectionManager' ===*/
		void testAddConnection();
		void testAddConvergenceLayer();
		void testQueue();
		void testRaiseEvent();
		void testGetNeighbors();
		void testIsNeighbor();
		void testDiscovered();
		void testCheck_discovered();
		/*=== END   tests for class 'ConnectionManager' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(ConnectionManagerTest);
			CPPUNIT_TEST(testAddConnection);
			CPPUNIT_TEST(testAddConvergenceLayer);
			CPPUNIT_TEST(testQueue);
			CPPUNIT_TEST(testRaiseEvent);
			CPPUNIT_TEST(testGetNeighbors);
			CPPUNIT_TEST(testIsNeighbor);
			CPPUNIT_TEST(testDiscovered);
			CPPUNIT_TEST(testCheck_discovered);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* CONNECTIONMANAGERTEST_HH */
