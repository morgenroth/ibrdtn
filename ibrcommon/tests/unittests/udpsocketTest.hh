/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        udpsocketTest.hh
/// @brief       CPPUnit-Tests for class udpsocket
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef UDPSOCKETTEST_HH
#define UDPSOCKETTEST_HH
class udpsocketTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'udpsocket' ===*/
		/*=== BEGIN tests for class 'peer' ===*/
		void testSend();
		/*=== END   tests for class 'peer' ===*/

		void testReceive();
		void testGetPeer();
		/*=== END   tests for class 'udpsocket' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(udpsocketTest);
			CPPUNIT_TEST(testSend);
			CPPUNIT_TEST(testReceive);
			CPPUNIT_TEST(testGetPeer);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* UDPSOCKETTEST_HH */
