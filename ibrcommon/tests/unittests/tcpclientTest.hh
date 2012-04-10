/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        tcpclientTest.hh
/// @brief       CPPUnit-Tests for class tcpclient
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef TCPCLIENTTEST_HH
#define TCPCLIENTTEST_HH
class tcpclientTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'tcpclient' ===*/
		void testOpen();
		/*=== END   tests for class 'tcpclient' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(tcpclientTest);
			CPPUNIT_TEST(testOpen);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* TCPCLIENTTEST_HH */
