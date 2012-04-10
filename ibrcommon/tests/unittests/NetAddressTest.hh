/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        NetAddressTest.hh
/// @brief       CPPUnit-Tests for class NetAddress
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef NETADDRESSTEST_HH
#define NETADDRESSTEST_HH
class NetAddressTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'NetAddress' ===*/
		void testToString();
		/*=== END   tests for class 'NetAddress' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(NetAddressTest);
			CPPUNIT_TEST(testToString);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* NETADDRESSTEST_HH */
