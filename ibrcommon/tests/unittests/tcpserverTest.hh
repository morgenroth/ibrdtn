/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        tcpserverTest.hh
/// @brief       CPPUnit-Tests for class tcpserver
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef TCPSERVERTEST_HH
#define TCPSERVERTEST_HH
class tcpserverTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'tcpserver' ===*/
		void testAccept();
		void testClose();
		void testShutdown();
		/*=== END   tests for class 'tcpserver' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(tcpserverTest);
			CPPUNIT_TEST(testAccept);
			CPPUNIT_TEST(testClose);
			CPPUNIT_TEST(testShutdown);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* TCPSERVERTEST_HH */
