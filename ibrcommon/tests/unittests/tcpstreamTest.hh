/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        tcpstreamTest.hh
/// @brief       CPPUnit-Tests for class tcpstream
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef TCPSTREAMTEST_HH
#define TCPSTREAMTEST_HH
class tcpstreamTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'tcpstream' ===*/
		void testGetAddress();
		void testGetPort();
		void testClose();
		void testEnableKeepalive();
		void testEnableLinger();
		void testEnableNoDelay();
		/*=== END   tests for class 'tcpstream' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(tcpstreamTest);
			CPPUNIT_TEST(testGetAddress);
			CPPUNIT_TEST(testGetPort);
			CPPUNIT_TEST(testClose);
			CPPUNIT_TEST(testEnableKeepalive);
			CPPUNIT_TEST(testEnableLinger);
			CPPUNIT_TEST(testEnableNoDelay);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* TCPSTREAMTEST_HH */
