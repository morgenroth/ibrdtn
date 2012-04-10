/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        UnicastSocketLowpanTest.hh
/// @brief       CPPUnit-Tests for class UnicastSocketLowpan
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef UNICASTSOCKETLOWPANTEST_HH
#define UNICASTSOCKETLOWPANTEST_HH
class UnicastSocketLowpanTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'UnicastSocketLowpan' ===*/
		void testBind();
		/*=== END   tests for class 'UnicastSocketLowpan' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(UnicastSocketLowpanTest);
			CPPUNIT_TEST(testBind);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* UNICASTSOCKETLOWPANTEST_HH */
