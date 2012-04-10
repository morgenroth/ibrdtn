/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        NetInterfaceTest.hh
/// @brief       CPPUnit-Tests for class NetInterface
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef NETINTERFACETEST_HH
#define NETINTERFACETEST_HH
class NetInterfaceTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'NetInterface' ===*/
		void testGetAddressList();
		void testGetBroadcastList();
		void testGetAddress();
		void testGetBroadcast();
		void testOperatorLessThan();
		void testOperatorEqual();
		void testToString();
		/*=== END   tests for class 'NetInterface' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(NetInterfaceTest);
			CPPUNIT_TEST(testGetAddressList);
			CPPUNIT_TEST(testGetBroadcastList);
			CPPUNIT_TEST(testGetAddress);
			CPPUNIT_TEST(testGetBroadcast);
			CPPUNIT_TEST(testOperatorLessThan);
			CPPUNIT_TEST(testOperatorEqual);
			CPPUNIT_TEST(testToString);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* NETINTERFACETEST_HH */
